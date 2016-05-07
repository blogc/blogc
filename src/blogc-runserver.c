/*
 * blogc: A blog compiler.
 * Copyright (C) 2015-2016 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/keyvalq_struct.h>
#include <magic.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "utils.h"


/**
 * this mapping is used to declare "supported" file types, that are forced over
 * whatever detected by libmagic, but we will still use the charset provided by
 * libmagic anyway. it also helps detecting index files when the client asks
 * for a directory.
 */
static const struct content_type_map {
    const char *mimetype;
    const char *extension;
    const char *index;
} content_types[] = {
    {"text/html", "html", "index.html"},
    {"text/html", "htm", "index.htm"},
    {"text/xml", "xml", "index.xml"},
    {"text/plain", "txt", "index.txt"},
    {"text/css", "css", NULL},
    {"application/javascript", "js", NULL},
    {NULL, NULL, NULL}
};


static magic_t magic_all = NULL;
static magic_t magic_charset = NULL;


static const char*
get_extension(const char *filename)
{
    const char *ext = NULL;
    unsigned int i;
    for (i = strlen(filename); i > 0; i--) {
        if (filename[i] == '.') {
            ext = filename + i + 1;
            break;
        }
    }
    if (i == 0)
        return NULL;
    return ext;
}


static char*
guess_content_type(const char *filename, int fd)
{
    int newfd;

    // try "supported" types first, and just use libmagic for charset
    const char *extension = get_extension(filename);
    if (extension == NULL)
        goto libmagic;
    const char *supported = NULL;
    for (unsigned int i = 0; content_types[i].extension != NULL; i++)
        if (0 == strcmp(content_types[i].extension, extension))
            supported = content_types[i].mimetype;
    if (supported != NULL) {
        newfd = dup(fd);
        if (-1 != newfd) {
            const char* charset = magic_descriptor(magic_charset, newfd);
            close(newfd);
            if (charset != NULL)
                return sb_strdup_printf("%s; charset=%s", supported, charset);
        }
        return sb_strdup(supported);
    }

libmagic:

    // fallback to use libmagic for everything
    newfd = dup(fd);
    if (-1 != newfd) {
        const char* content_type = magic_descriptor(magic_all, newfd);
        close(newfd);
        if (content_type != NULL)
            return sb_strdup(content_type);
    }
    return sb_strdup("application/octet-stream");
}


static void
handler(struct evhttp_request *request, void *ptr)
{
    const char *root = ptr;
    const char *uri = evhttp_request_get_uri(request);

    struct evhttp_uri *decoded_uri = evhttp_uri_parse(uri);
    if (decoded_uri == NULL) {
        evhttp_send_error(request, 400, "Bad request");
        return;
    }

    const char *path = evhttp_uri_get_path(decoded_uri);
    if (path == NULL)
        path = "/";

    char *decoded_path = evhttp_uridecode(path, 0, NULL);
    if (decoded_path == NULL) {
        evhttp_send_error(request, 400, "Bad request");
        goto point1;
    }

    char *abs_path = sb_strdup_printf("%s/%s", root, decoded_path);
    char *real_path = realpath(abs_path, NULL);
    free(abs_path);

    if (real_path == NULL) {
        evhttp_send_error(request, 404, "Not found");
        goto point2;
    }

    char *real_root = realpath(root, NULL);
    if (real_root == NULL) {
        evhttp_send_error(request, 500, "Internal server error");
        goto point3;
    }

    if (0 != strncmp(real_root, real_path, strlen(real_root))) {
        evhttp_send_error(request, 404, "Not found");
        goto point4;
    }

    struct stat st;
    if (0 > stat(real_path, &st)) {
        evhttp_send_error(request, 404, "Not found");
        goto point4;
    }

    bool add_slash = false;

    if (S_ISDIR(st.st_mode)) {
        char *found = NULL;

        for (unsigned int i = 0; content_types[i].mimetype != NULL; i++) {
            if (content_types[i].index == NULL)
                continue;
            char *f = sb_strdup_printf("%s/%s", real_path,
                content_types[i].index);
            if (0 == access(f, F_OK)) {
                found = sb_strdup(f);
                break;
            }
            free(f);
        }

        if (found == NULL) {
            evhttp_send_error(request, 403, "Forbidden");
            goto point4;
        }

        size_t path_len = strlen(path);
        if (path_len > 0 && path[path_len - 1] != '/')
            add_slash = true;

        free(real_path);
        real_path = found;
    }

    int fd;
    if ((fd = open(real_path, O_RDONLY)) < 0) {
        evhttp_send_error(request, 500, "Internal server error");
        goto point4;
    }

    char *type = guess_content_type(real_path, fd);

    if (fstat(fd, &st) < 0) {
        evhttp_send_error(request, 500, "Internal server error");
        goto point5;
    }

    struct evkeyvalq *headers = evhttp_request_get_output_headers(request);

    if (add_slash) {
        char *tmp = sb_strdup_printf("%s/", path);
        evhttp_add_header(headers, "Location", tmp);
        free(tmp);
        // production webservers usually returns 301 in such cases, but 302 is
        // better for development/testing.
        evhttp_send_reply(request, 302, "Found", NULL);
        goto point5;
    }

    evhttp_add_header(headers, "Content-Type", type);
    char *content_length = sb_strdup_printf("%zu", st.st_size);
    evhttp_add_header(headers, "Content-Length", content_length);
    free(content_length);

    struct evbuffer *evb = evbuffer_new();
    evbuffer_add_file(evb, fd, 0, st.st_size);
    evhttp_send_reply(request, 200, "OK", evb);

point5:
    free(type);
point4:
    free(real_root);
point3:
    free(real_path);
point2:
    free(decoded_path);
point1:
    evhttp_uri_free(decoded_uri);
}


static int
runserver(const char *address, unsigned short port, const char *root)
{
    struct event_base *base = event_base_new();
    if (base == NULL) {
        fprintf(stderr, "error: failed to initialize event base\n");
        return 1;
    }

    struct evhttp *http = evhttp_new(base);
    if (http == NULL) {
        fprintf(stderr, "error: failed to initialize HTTP server\n");
        return 1;
    }

    evhttp_set_gencb(http, handler, (char*) root);

    evhttp_set_allowed_methods(http, EVHTTP_REQ_GET | EVHTTP_REQ_HEAD);

    if (0 != evhttp_bind_socket(http, address, port)) {
        fprintf(stderr, "error: failed to bind socket to %s:%d\n", address,
            port);
        return 1;
    }

    fprintf(stderr, " * Running on http://%s:%d/\n", address, port);

    event_base_dispatch(base);

    return 0;
}


static void
print_help(void)
{
    printf(
        "usage:\n"
        "    blogc-runserver [-h] [-v] [-t HOST] [-p PORT] DOCROOT\n"
        "                    - A simple HTTP server to test blogc websites.\n"
        "\n"
        "positional arguments:\n"
        "    DOCROOT       document root directory\n"
        "\n"
        "optional arguments:\n"
        "    -h            show this help message and exit\n"
        "    -v            show version and exit\n"
        "    -t HOST       set server listen address (default: 127.0.0.1)\n"
        "    -p PORT       set server listen port (default: 8080)\n");
}


static void
print_usage(void)
{
    printf("usage: blogc-runserver [-h] [-v] [-t HOST] [-p PORT] DOCROOT\n");
}


int
main(int argc, char **argv)
{
    signal(SIGPIPE, SIG_IGN);

    int rv = 0;
    char *host = NULL;
    char *docroot = NULL;
    unsigned short port = 8080;

    unsigned int args = 0;

    for (unsigned int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            switch (argv[i][1]) {
                case 'h':
                    print_help();
                    goto cleanup;
                case 'v':
                    printf("%s\n", PACKAGE_STRING);
                    goto cleanup;
                case 't':
                    if (argv[i][2] != '\0')
                        host = sb_strdup(argv[i] + 2);
                    else
                        host = sb_strdup(argv[++i]);
                    break;
                case 'p':
                    if (argv[i][2] != '\0')
                        port = strtoul(argv[i] + 2, NULL, 10);
                    else
                        port = strtoul(argv[++i], NULL, 10);
                    break;
                default:
                    print_usage();
                    fprintf(stderr, "blogc-runserver: error: invalid "
                        "argument: -%c\n", argv[i][1]);
                    rv = 2;
                    goto cleanup;
            }
        }
        else {
            if (args > 0) {
                print_usage();
                fprintf(stderr, "blogc-runserver: error: only one positional "
                    "argument allowed\n");
                rv = 2;
                goto cleanup;
            }
            args++;
            docroot = sb_strdup(argv[i]);
        }
    }

    if (docroot == NULL) {
        print_usage();
        fprintf(stderr, "blogc-runserver: error: document root directory "
            "required\n");
        rv = 2;
        goto cleanup;
    }

    if (host == NULL)
        host = sb_strdup("127.0.0.1");

    magic_all = magic_open(MAGIC_MIME);
    magic_charset = magic_open(MAGIC_MIME_ENCODING);
    if (magic_all == NULL || magic_charset == NULL) {
        fprintf(stderr, "error: failed to initialize libmagic\n");
        rv = 1;
        goto cleanup;
    }

    if ((0 != magic_load(magic_all, NULL)) ||
        (0 != magic_load(magic_charset, NULL)))
    {
        fprintf(stderr, "error: failed to load libmagic data\n");
        magic_close(magic_all);
        magic_close(magic_charset);
        rv = 1;
        goto cleanup;
    }

    rv = runserver(host, port, docroot);

    magic_close(magic_all);
    magic_close(magic_charset);

cleanup:
    free(host);
    free(docroot);

    return rv;
}
