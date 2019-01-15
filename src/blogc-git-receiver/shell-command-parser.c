/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2019 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "../common/utils.h"
#include "shell-command-parser.h"

typedef enum {
    START_COMMAND = 1,
    START_REPO,
    START_REPO2,
    REPO,
    START_ESCAPED,
} command_state_t;


char*
bgr_shell_command_parse(const char *command)
{
    command_state_t state = START_COMMAND;
    size_t start = 0;
    size_t command_len = strlen(command);

    bc_string_t *rv = bc_string_new();

    for (size_t current = 0; current < command_len; current++) {

        char c = command[current];

        switch (state) {
            case START_COMMAND:
                if (c == ' ') {
                    if (((current == 16) &&
                         (0 == strncmp("git-receive-pack", command, 16))) ||
                        ((current == 15) &&
                         (0 == strncmp("git-upload-pack", command, 15))) ||
                        ((current == 18) &&
                         (0 == strncmp("git-upload-archive", command, 18))))
                    {
                        state = START_REPO;
                        break;
                    }
                    goto error;
                }
                break;

            case START_REPO:
                if (c == '\'') {  // never saw git using double-quotes
                    state = START_REPO2;
                    break;
                }
                if (c == '\\') {  // escaped ! or '
                    state = START_ESCAPED;
                    break;
                }
                goto error;

            case START_REPO2:
                if (c == '\'') {
                    state = START_REPO;
                    break;
                }
                start = current;
                if (rv->len == 0 && c == '/') {  // no absolute urls
                    start = current + 1;
                }
                state = REPO;
                break;

            case START_ESCAPED:
                if (c == '!' || c == '\'') {
                    bc_string_append_c(rv, c);
                    state = START_REPO;
                    break;
                }
                goto error;

            case REPO:
                if (c == '\'') {
                    bc_string_append_len(rv, command + start, current - start);
                    state = START_REPO;
                    break;
                }
                break;
        }
    }

    if (rv->len > 0)
        return bc_string_free(rv, false);

error:
    bc_string_free(rv, true);
    return NULL;
}
