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
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "../common/utils.h"


// mime types with index should be in the begin of the list. first NULL
// index aborts the lookup, for optimization
static const struct content_type_map {
    const char *mimetype;
    const char *extension;
    const char *index;
} content_types[] = {

    // with index
    {"text/html", "html", "index.html"},
    {"text/html", "htm", "index.htm"},
    {"text/html", "shtml", "index.shtml"},
    {"text/xml", "xml", "index.xml"},
    {"text/plain", "txt", "index.txt"},
    {"application/xhtml+xml", "xhtml", "index.xhtml"},

    // without index
    {"text/css", "css", NULL},
    {"image/gif", "gif", NULL},
    {"image/jpeg", "jpeg", NULL},
    {"image/jpeg", "jpg", NULL},
    {"application/javascript", "js", NULL},
    {"application/atom+xml", "atom", NULL},
    {"application/rss+xml", "rss", NULL},
    {"text/mathml", "mml", NULL},
    {"text/vnd.sun.j2me.app-descriptor", "jad", NULL},
    {"text/vnd.wap.wml", "wml", NULL},
    {"text/x-component", "htc", NULL},
    {"image/png", "png", NULL},
    {"image/tiff", "tif", NULL},
    {"image/tiff", "tiff", NULL},
    {"image/vnd.wap.wbmp", "wbmp", NULL},
    {"image/x-icon", "ico", NULL},
    {"image/x-jng", "jng", NULL},
    {"image/x-ms-bmp", "bmp", NULL},
    {"image/svg+xml", "svg", NULL},
    {"image/svg+xml", "svgz", NULL},
    {"image/webp", "webp", NULL},
    {"application/font-woff", "woff", NULL},
    {"application/java-archive", "jar", NULL},
    {"application/java-archive", "war", NULL},
    {"application/java-archive", "ear", NULL},
    {"application/json", "json", NULL},
    {"application/mac-binhex40", "hqx", NULL},
    {"application/msword", "doc", NULL},
    {"application/pdf", "pdf", NULL},
    {"application/postscript", "ps", NULL},
    {"application/postscript", "eps", NULL},
    {"application/postscript", "ai", NULL},
    {"application/rtf", "rtf", NULL},
    {"application/vnd.apple.mpegurl", "m3u8", NULL},
    {"application/vnd.ms-excel", "xls", NULL},
    {"application/vnd.ms-fontobject", "eot", NULL},
    {"application/vnd.ms-powerpoint", "ppt", NULL},
    {"application/vnd.wap.wmlc", "wmlc", NULL},
    {"application/vnd.google-earth.kml+xml", "kml", NULL},
    {"application/vnd.google-earth.kmz", "kmz", NULL},
    {"application/x-7z-compressed", "7z", NULL},
    {"application/x-cocoa", "cco", NULL},
    {"application/x-java-archive-diff", "jardiff", NULL},
    {"application/x-java-jnlp-file", "jnlp", NULL},
    {"application/x-makeself", "run", NULL},
    {"application/x-perl", "pl", NULL},
    {"application/x-perl", "pm", NULL},
    {"application/x-pilot", "prc", NULL},
    {"application/x-pilot", "pdb", NULL},
    {"application/x-rar-compressed", "rar", NULL},
    {"application/x-redhat-package-manager", "rpm", NULL},
    {"application/x-sea", "sea", NULL},
    {"application/x-shockwave-flash", "swf", NULL},
    {"application/x-stuffit", "sit", NULL},
    {"application/x-tcl", "tcl", NULL},
    {"application/x-tcl", "tk", NULL},
    {"application/x-x509-ca-cert", "der", NULL},
    {"application/x-x509-ca-cert", "pem", NULL},
    {"application/x-x509-ca-cert", "crt", NULL},
    {"application/x-xpinstall", "xpi", NULL},
    {"application/xspf+xml", "xspf", NULL},
    {"application/zip", "zip", NULL},
    {"application/octet-stream", "bin", NULL},
    {"application/octet-stream", "exe", NULL},
    {"application/octet-stream", "dll", NULL},
    {"application/octet-stream", "deb", NULL},
    {"application/octet-stream", "dmg", NULL},
    {"application/octet-stream", "iso", NULL},
    {"application/octet-stream", "img", NULL},
    {"application/octet-stream", "msi", NULL},
    {"application/octet-stream", "msp", NULL},
    {"application/octet-stream", "msm", NULL},
    {"application/vnd.openxmlformats-officedocument.wordprocessingml.document", "docx", NULL},
    {"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet", "xlsx", NULL},
    {"application/vnd.openxmlformats-officedocument.presentationml.presentation", "pptx", NULL},
    {"audio/midi", "mid", NULL},
    {"audio/midi", "midi", NULL},
    {"audio/midi", "kar", NULL},
    {"audio/mpeg", "mp3", NULL},
    {"audio/ogg", "ogg", NULL},
    {"audio/x-m4a", "m4a", NULL},
    {"audio/x-realaudio", "ra", NULL},
    {"video/3gpp", "3gpp", NULL},
    {"video/3gpp", "3gp", NULL},
    {"video/mp2t", "ts", NULL},
    {"video/mp4", "mp4", NULL},
    {"video/mpeg", "mpeg", NULL},
    {"video/mpeg", "mpg", NULL},
    {"video/quicktime", "mov", NULL},
    {"video/webm", "webm", NULL},
    {"video/x-flv", "flv", NULL},
    {"video/x-m4v", "m4v", NULL},
    {"video/x-mng", "mng", NULL},
    {"video/x-ms-asf", "asx", NULL},
    {"video/x-ms-asf", "asf", NULL},
    {"video/x-ms-wmv", "wmv", NULL},
    {"video/x-msvideo", "avi", NULL},
    {NULL, NULL, NULL}
};


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


static const char*
guess_content_type(const char *filename)
{
    const char *extension = get_extension(filename);
    if (extension == NULL)
        goto default_type;
    for (unsigned int i = 0; content_types[i].extension != NULL; i++) {
        if (0 == strcmp(content_types[i].extension, extension)) {
            return content_types[i].mimetype;
        }
    }
default_type:
    return "application/octet-stream";
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

    char *abs_path = bc_strdup_printf("%s/%s", root, decoded_path);
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

        for (unsigned int i = 0; content_types[i].index != NULL; i++) {
            char *f = bc_strdup_printf("%s/%s", real_path,
                content_types[i].index);
            if (0 == access(f, F_OK)) {
                found = f;
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

    const char *type = guess_content_type(real_path);

    if (fstat(fd, &st) < 0) {
        evhttp_send_error(request, 500, "Internal server error");
        goto point4;
    }

    struct evkeyvalq *headers = evhttp_request_get_output_headers(request);

    if (add_slash) {
        char *tmp = bc_strdup_printf("%s/", path);
        evhttp_add_header(headers, "Location", tmp);
        free(tmp);
        // production webservers usually returns 301 in such cases, but 302 is
        // better for development/testing.
        evhttp_send_reply(request, 302, "Found", NULL);
        goto point4;
    }

    evhttp_add_header(headers, "Content-Type", type);
    char *content_length = bc_strdup_printf("%zu", st.st_size);
    evhttp_add_header(headers, "Content-Length", content_length);
    free(content_length);

    struct evbuffer *evb = evbuffer_new();
    evbuffer_add_file(evb, fd, 0, st.st_size);
    evhttp_send_reply(request, 200, "OK", evb);

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
                        host = bc_strdup(argv[i] + 2);
                    else
                        host = bc_strdup(argv[++i]);
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
            docroot = bc_strdup(argv[i]);
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
        host = bc_strdup("127.0.0.1");

    rv = runserver(host, port, docroot);

cleanup:
    free(host);
    free(docroot);

    return rv;
}
