/*
 * blogc: A blog compiler.
 * Copyright (C) 2015 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file COPYING.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include "utils/utils.h"
#include "output.h"


void
blogc_parser_syntax_error(const char *name, const char *src, size_t src_len,
    size_t current)
{
    b_string_t *msg = b_string_new();

    while (current < src_len) {
        char c = src[current];

        if (c == '\r' || c == '\n')
            break;

        b_string_append_c(msg, c);

        current++;
    }

    fprintf(stderr, "%s parser error: syntax error near \"%s\"\n", name,
        msg->str);

    b_string_free(msg, true);
}
