/*
 * blogc: A blog compiler.
 * Copyright (C) 2015-2016 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#include <errno.h>
#include <stdio.h>
#include <stdbool.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
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
    if (write(socket, str, strlen(str)) == -1) {
        // do nothing, just avoid warnig
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
        goto point1;
    }

    char *abs_path = bc_strdup_printf("%s/%s", docroot, path);
    char *real_path = realpath(abs_path, NULL);
    free(abs_path);

    if (real_path == NULL) {
        status_code = 404;
        error(client_socket, 404, "Not Found");
        goto point1;
    }

    char *real_root = realpath(docroot, NULL);
    if (real_root == NULL) {
        status_code = 500;
        error(client_socket, 500, "Internal Server Error");
        goto point2;
    }

    if (0 != strncmp(real_root, real_path, strlen(real_root))) {
        status_code = 404;
        error(client_socket, 404, "Not Found");
        goto point3;
    }

    struct stat st;
    if (0 > stat(real_path, &st)) {
        status_code = 404;
        error(client_socket, 404, "Not Found");
        goto point3;
    }

    bool add_slash = false;

    if (S_ISDIR(st.st_mode)) {
        char *found = br_mime_guess_index(real_path);

        if (found == NULL) {
            status_code = 403;
            error(client_socket, 403, "Forbidden");
            goto point3;
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
        goto point3;
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
        if (write(client_socket, tmp, strlen(tmp)) == -1) {
            // do nothing, just avoid warnig
        }
        free(tmp);
        goto point3;
    }

    size_t len;
    bc_error_t *err = NULL;
    char* contents = bc_file_get_contents(real_path, false, &len, &err);
    if (err != NULL) {
        status_code = 500;
        error(client_socket, 500, "Internal Server Error");
        bc_error_free(err);
        goto point3;
    }

    char *out = bc_strdup_printf(
        "HTTP/1.0 200 OK\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n", br_mime_guess_content_type(real_path), len);
    if (write(client_socket, out, strlen(out)) == -1) {
        // do nothing, just avoid warnig
    }
    free(out);

    if (write(client_socket, contents, len) == -1) {
        // do nothing, just avoid warnig
    }

point3:
    free(real_root);
point2:
    free(real_path);
point1:
    fprintf(stderr, "[Thread-%zu] %s - - \"%s\" %d\n", thread_id + 1,
        ip, conn_line, status_code);
    bc_strv_free(pieces);
point0:
    free(ip);
    close(client_socket);
    return NULL;
}


int
br_httpd_run(const char *host, unsigned short port, const char *docroot,
    size_t max_threads)
{
    if (port == 0) {
        fprintf(stderr, "Invalid port: 0\n");
        return 1;
    }

    thread_data_t threads[max_threads];
    for (size_t i = 0; i < max_threads; i++)
        threads[i].initialized = false;

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        fprintf(stderr, "Failed to open server socket: %s\n", strerror(errno));
        return 1;
    }

    int rv = 0;

    int value = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(int)) < 0) {
        fprintf(stderr, "Failed to set socket option: %s\n", strerror(errno));
        rv = 1;
        goto cleanup;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if ((server_addr.sin_addr.s_addr = inet_addr(host)) == -1) {
        fprintf(stderr, "Invalid server listen address: %s\n", host);
        rv = 1;
        goto cleanup;
    }

    if ((bind(server_socket, (struct sockaddr*) &server_addr,
        sizeof(struct sockaddr_in))) == -1)
    {
        fprintf(stderr, "Failed to bind to server socket (%s:%d): %s\n",
            host, port, strerror(errno));
        rv = 1;
        goto cleanup;
    }

    if (listen(server_socket, LISTEN_BACKLOG) == -1) {
        fprintf(stderr, "Failed to listen to server socket: %s\n", strerror(errno));
        rv = 1;
        goto cleanup;
    }

    fprintf(stderr,
        " * Running on http://%s:%d/ (max threads: %zu)\n"
        "\n"
        "WARNING!!! This is a development server, DO NOT RUN IT IN PRODUCTION!\n"
        "\n", host, port, max_threads);

    socklen_t len = sizeof(struct sockaddr_in);

    size_t current_thread = 0;

    while (1) {
        struct sockaddr_in client_addr;
        int client_socket = accept(server_socket,
            (struct sockaddr *) &client_addr, &len);
        if (client_socket == -1) {
            fprintf(stderr, "Failed to accept connection: %s\n", strerror(errno));
            rv = 1;
            goto cleanup;
        }

        request_data_t *arg = malloc(sizeof(request_data_t));
        arg->thread_id = current_thread;
        arg->socket = client_socket;
        arg->ip = bc_strdup(inet_ntoa(client_addr.sin_addr));
        arg->docroot = docroot;

        if (threads[current_thread].initialized) {
            if (pthread_join(threads[current_thread].thread, NULL) != 0) {
                fprintf(stderr, "Failed to join thread\n");
                rv = 1;
                goto cleanup;
            }
        }

        if (pthread_create(&(threads[current_thread].thread), NULL, handle_request, arg) != 0) {
            fprintf(stderr, "Failed to create thread\n");
            rv = 1;
            goto cleanup;
        }

        threads[current_thread++].initialized = true;

        if (current_thread >= max_threads)
            current_thread = 0;
    }

cleanup:
    close(server_socket);
    return rv;
}
