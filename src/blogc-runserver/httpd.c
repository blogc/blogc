/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2019 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#include <errno.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "../common/error.h"
#include "../common/file.h"
#include "../common/utils.h"
#include "mime.h"
#include "httpd-utils.h"

#define LISTEN_BACKLOG 100

typedef struct {
    pthread_t thread;
    bool initialized;
} thread_data_t;

typedef struct {
    size_t thread_id;
    int socket;
    char *ip;
    const char *docroot;
} request_data_t;


static void
error(int socket, int status_code, const char *error)
{
    char *str = bc_strdup_printf(
        "HTTP/1.0 %d %s\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n"
        "<h1>%s</h1>\n", status_code, error, strlen(error) + 10, error);
    size_t str_len = strlen(str);
    if (str_len != write(socket, str, str_len)) {
        fprintf(stderr, "warning: Failed to write full response header!\n");
    }
    free(str);
}


static void*
handle_request(void *arg)
{
    request_data_t *req = arg;
    size_t thread_id = req->thread_id;
    int client_socket = req->socket;
    char *ip = req->ip;
    const char *docroot = req->docroot;
    free(arg);

    char *conn_line = br_readline(client_socket);
    if (conn_line == NULL || conn_line[0] == '\0')
        goto point0;

    unsigned short status_code = 200;

    char **pieces = bc_str_split(conn_line, ' ', 3);
    if (bc_strv_length(pieces) != 3) {
        status_code = 400;
        error(client_socket, 400, "Bad Request");
        goto point1;
    }

    if (strcmp(pieces[0], "GET") != 0) {
        status_code = 405;
        error(client_socket, 405, "Method Not Allowed");
        goto point1;
    }

    char **pieces2 = bc_str_split(pieces[1], '?', 2);
    char *path = br_urldecode(pieces2[0]);
    bc_strv_free(pieces2);

    if (path == NULL) {
        status_code = 400;
        error(client_socket, 400, "Bad Request");
        goto point2;
    }

    char *abs_path = bc_strdup_printf("%s/%s", docroot, path);
    char *real_path = realpath(abs_path, NULL);
    free(abs_path);

    if (real_path == NULL) {
        if (errno == ENOENT) {
            status_code = 404;
            error(client_socket, 404, "Not Found");
        }
        else {
            status_code = 500;
            error(client_socket, 500, "Internal Server Error");
        }
        goto point2;
    }

    char *real_root = realpath(docroot, NULL);
    if (real_root == NULL) {
        status_code = 500;
        error(client_socket, 500, "Internal Server Error");
        goto point3;
    }

    if (0 != strncmp(real_root, real_path, strlen(real_root))) {
        status_code = 404;
        error(client_socket, 404, "Not Found");
        goto point4;
    }

    struct stat st;
    if (0 > stat(real_path, &st)) {
        status_code = 404;
        error(client_socket, 404, "Not Found");
        goto point4;
    }

    bool add_slash = false;

    if (S_ISDIR(st.st_mode)) {
        char *found = br_mime_guess_index(real_path);

        if (found == NULL) {
            status_code = 403;
            error(client_socket, 403, "Forbidden");
            goto point4;
        }

        size_t path_len = strlen(path);
        if (path_len > 0 && path[path_len - 1] != '/')
            add_slash = true;

        free(real_path);
        real_path = found;
    }

    if (0 != access(real_path, F_OK)) {
        status_code = 500;
        error(client_socket, 500, "Internal Server Error");
        goto point4;
    }

    if (add_slash) {
        // production webservers usually returns 301 in such cases, but 302 is
        // better for development/testing.
        char *tmp = bc_strdup_printf(
            "HTTP/1.0 302 Found\r\n"
            "Location: %s/\r\n"
            "Content-Length: 0\r\n"
            "Connection: close\r\n"
            "\r\n", path);
        status_code = 302;
        size_t tmp_len = strlen(tmp);
        if (tmp_len != write(client_socket, tmp, tmp_len)) {
            fprintf(stderr, "warning: Failed to write full response header!\n");
        }
        free(tmp);
        goto point4;
    }

    size_t len;
    bc_error_t *err = NULL;
    char* contents = bc_file_get_contents(real_path, false, &len, &err);
    if (err != NULL) {
        status_code = 500;
        error(client_socket, 500, "Internal Server Error");
        bc_error_free(err);
        goto point4;
    }

    char *out = bc_strdup_printf(
        "HTTP/1.0 200 OK\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n", br_mime_guess_content_type(real_path), len);
    size_t out_len = strlen(out);
    if (out_len != write(client_socket, out, out_len)) {
        fprintf(stderr, "warning: Failed to write full response header!\n");
    }
    free(out);

    if (len != write(client_socket, contents, len)) {
        fprintf(stderr, "warning: Failed to write full response body!\n");
    }
    free(contents);

point4:
    free(real_root);
point3:
    free(real_path);
point2:
    free(path);
point1:
    fprintf(stderr, "[Thread-%zu] %s - - \"%s\" %d\n", thread_id + 1,
        ip, conn_line, status_code);
    free(conn_line);
    bc_strv_free(pieces);
point0:
    free(ip);
    close(client_socket);
    return NULL;
}


static char*
br_httpd_get_ip(int af, const struct sockaddr *addr)
{
    char host[INET6_ADDRSTRLEN];
    if (af == AF_INET6) {
        struct sockaddr_in6 *a = (struct sockaddr_in6*) addr;
        inet_ntop(af, &(a->sin6_addr), host, INET6_ADDRSTRLEN);
    }
    else {
        struct sockaddr_in *a = (struct sockaddr_in*) addr;
        inet_ntop(af, &(a->sin_addr), host, INET6_ADDRSTRLEN);
    }
    return bc_strdup(host);
}


static u_int16_t
br_httpd_get_port(int af, const struct sockaddr *addr)
{
    in_port_t port = 0;
    if (af == AF_INET6) {
        struct sockaddr_in6 *a = (struct sockaddr_in6*) addr;
        port = a->sin6_port;
    }
    else {
        struct sockaddr_in *a = (struct sockaddr_in*) addr;
        port = a->sin_port;
    }
    return ntohs(port);
}


int
br_httpd_run(const char *host, const char *port, const char *docroot,
    size_t max_threads)
{
    int err;
    struct addrinfo *result;
    struct addrinfo hints = {
        .ai_family = AF_UNSPEC,
        .ai_socktype = SOCK_STREAM,
        .ai_flags = AI_PASSIVE,
        .ai_protocol = 0,
        .ai_canonname = NULL,
        .ai_addr = NULL,
        .ai_next = NULL,
    };
    if (0 != (err = getaddrinfo(host, port, &hints, &result))) {
        fprintf(stderr, "Failed to get host:port info: %s\n",
            gai_strerror(err));
        return 3;
    }

    thread_data_t threads[max_threads];
    for (size_t i = 0; i < max_threads; i++)
        threads[i].initialized = false;

    int rv = 0;

    struct addrinfo *rp;
    int server_socket = 0;

    int ai_family = 0;
    char *final_host = NULL;
    u_int16_t final_port = 0;

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        final_host = br_httpd_get_ip(rp->ai_family, rp->ai_addr);
        final_port = br_httpd_get_port(rp->ai_family, rp->ai_addr);
        server_socket = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (server_socket == -1) {
            if (rp->ai_next == NULL) {
                fprintf(stderr, "Failed to open server socket (%s:%d): %s\n",
                    final_host, final_port, strerror(errno));
                rv = 3;
                goto cleanup0;
            }
            continue;
        }
        int value = 1;
        if (0 > setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &value,
            sizeof(int)))
        {
            if (rp->ai_next == NULL) {
                fprintf(stderr, "Failed to set socket option (%s:%d): %s\n",
                    final_host, final_port, strerror(errno));
                rv = 3;
                goto cleanup;
            }
            close(server_socket);
            continue;
        }
        if (0 == bind(server_socket, rp->ai_addr, rp->ai_addrlen)) {
            ai_family = rp->ai_family;
            break;
        }
        else {
            if (rp->ai_next == NULL) {
                fprintf(stderr, "Failed to bind to server socket (%s:%d): %s\n",
                    final_host, final_port, strerror(errno));
                rv = 3;
                goto cleanup;
            }
        }
        free(final_host);
        close(server_socket);
    }

    if (-1 == listen(server_socket, LISTEN_BACKLOG)) {
        fprintf(stderr, "Failed to listen to server socket (%s:%d): %s\n",
            final_host, final_port, strerror(errno));
        rv = 3;
        goto cleanup;
    }

    fprintf(stderr, " * Running on http://");
    if (ai_family == AF_INET6)
        fprintf(stderr, "[%s]", final_host);
    else
        fprintf(stderr, "%s", final_host);
    if (final_port != 80)
        fprintf(stderr, ":%d", final_port);
    fprintf(stderr, "/ (max threads: %zu)\n"
        "\n"
        "WARNING!!! This is a development server, DO NOT RUN IT IN PRODUCTION!\n"
        "\n", max_threads);

    size_t current_thread = 0;

    while (1) {
        struct sockaddr_in6 addr6;
        struct sockaddr_in addr;

        socklen_t addrlen;
        struct sockaddr *client_addr = NULL;

        if (ai_family == AF_INET6) {
            addrlen = sizeof(addr6);
            client_addr = (struct sockaddr*) &addr6;
        }
        else {
            addrlen = sizeof(addr);
            client_addr = (struct sockaddr*) &addr;
        }

        int client_socket = accept(server_socket, client_addr, &addrlen);
        if (client_socket == -1) {
            fprintf(stderr, "Failed to accept connection: %s\n", strerror(errno));
            rv = 3;
            goto cleanup;
        }

        request_data_t *arg = malloc(sizeof(request_data_t));
        arg->thread_id = current_thread;
        arg->socket = client_socket;
        arg->ip = br_httpd_get_ip(ai_family, client_addr);
        arg->docroot = docroot;

        if (threads[current_thread].initialized) {
            if (pthread_join(threads[current_thread].thread, NULL) != 0) {
                fprintf(stderr, "Failed to join thread\n");
                free(arg->ip);
                free(arg);
                rv = 3;
                goto cleanup;
            }
        }

        if (pthread_create(&(threads[current_thread].thread), NULL,
            handle_request, arg) != 0)
        {
            fprintf(stderr, "Failed to create thread\n");
            rv = 3;
            goto cleanup;
        }

        threads[current_thread++].initialized = true;

        if (current_thread >= max_threads)
            current_thread = 0;
    }

cleanup:
    close(server_socket);

cleanup0:
    free(final_host);
    freeaddrinfo(result);
    return rv;
}
