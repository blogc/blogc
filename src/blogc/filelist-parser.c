/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2019 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#include "../common/utils.h"


typedef enum {
    LINE_START = 1,
    LINE,
} blogc_filelist_parser_state_t;


bc_slist_t*
blogc_filelist_parse(const char *src, size_t src_len)
{
    size_t current = 0;
    bc_slist_t *rv = NULL;
    bc_string_t *line = bc_string_new();
    blogc_filelist_parser_state_t state = LINE_START;

    while (current < src_len) {
        char c = src[current];
        bool is_last = current == src_len - 1;

        switch (state) {

            case LINE_START:
                if (c == '#') {
                    while (current < src_len) {
                        if (src[current] == '\r' || src[current] == '\n')
                            break;
                        current++;
                    }
                    break;
                }
                if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
                    break;
                state = LINE;
                continue;

            case LINE:
                if (c == '\r' || c == '\n' || is_last) {
                    if (is_last && c != '\r' && c != '\n')
                        bc_string_append_c(line, c);
                    rv = bc_slist_append(rv, bc_str_strip(line->str));
                    bc_string_free(line, false);
                    line = bc_string_new();
                    state = LINE_START;
                    break;
                }
                bc_string_append_c(line, c);
                break;

        }

        current++;
    }

    bc_string_free(line, true);

    return rv;
}
