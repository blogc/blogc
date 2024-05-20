// SPDX-FileCopyrightText: 2014-2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
// SPDX-License-Identifier: BSD-3-Clause

#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include "../common/utils.h"
#include "httpd-utils.h"


char*
br_readline(int socket)
{
    bc_string_t *rv = bc_string_new();
    char buffer[READLINE_BUFFER_SIZE];
    ssize_t len;
    bool end = false;

    while ((len = read(socket, buffer, READLINE_BUFFER_SIZE)) > 0) {
        if (!end) {
            for (ssize_t i = 0; i < len; i++) {
                if (buffer[i] == '\r' || buffer[i] == '\n' || buffer[i] == '\0') {
                    // we finished "recording", but still need to exhaust
                    // request data.
                    end = true;
                    break;
                }
                bc_string_append_c(rv, buffer[i]);
            }
        }
        if (len < READLINE_BUFFER_SIZE) {
            break;
        }
    }

    return bc_string_free(rv, false);
}


int
br_hextoi(const char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return -1;
}


char*
br_urldecode(const char *str)
{
    bc_string_t *rv = bc_string_new();

    for (size_t i = 0; i < strlen(str); i++) {
        switch (str[i]) {
            case '%':
                if (i + 2 < strlen(str)) {
                    int p1 = br_hextoi(str[i + 1]) * 16;
                    int p2 = br_hextoi(str[i + 2]);
                    if (p1 >= 0 && p2 >= 0) {
                        bc_string_append_c(rv, p1 + p2);
                        i += 2;
                        continue;
                    }
                }
                bc_string_append_c(rv, '%');
                break;
            case '+':
                bc_string_append_c(rv, ' ');
                break;
            default:
                bc_string_append_c(rv, str[i]);
        }
    }

    return bc_string_free(rv, false);
}


const char*
br_get_extension(const char *filename)
{
    const char *ext = NULL;
    size_t i;
    for (i = strlen(filename); i > 0; i--) {
        if (filename[i] == '.') {
            ext = filename + i + 1;
            break;
        }
        if ((filename[i] == '/') || (filename[i] == '\\'))
            return NULL;
    }
    if (i == 0)
        return NULL;
    return ext;
}
