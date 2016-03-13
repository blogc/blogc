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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <squareball.h>
#include "error.h"


sb_error_t*
blogc_error_parser(blogc_error_code_t type, const char *src, size_t src_len,
    size_t current, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    char *msg = sb_strdup_vprintf(format, ap);
    va_end(ap);

    size_t lineno = 1;
    size_t linestart = 0;
    size_t lineend = 0;
    size_t pos = 1;

    for (size_t i = 0; i < src_len; i++) {
        char c = src[i];
        if (i < current) {
            if ((i + 1) < src_len) {
                if ((c == '\n' && src[i + 1] == '\r') ||
                    (c == '\r' && src[i + 1] == '\n'))
                {
                    lineno++;
                    i++;
                    pos = 1;
                    if ((i + 1) < src_len)
                        linestart = i + 1;
                    continue;
                }
            }
            if (c == '\n' || c == '\r') {
                lineno++;
                pos = 1;
                if ((i + 1) < src_len)
                    linestart = i + 1;
                continue;
            }
            pos++;
        }
        else if (c == '\n' || c == '\r') {
            lineend = i;
            break;
        }
    }

    if (lineend <= linestart && src_len >= linestart)
        lineend = src_len;

    char *line = sb_strndup(src + linestart, lineend - linestart);

    sb_error_t *rv = NULL;

    if (line[0] == '\0')  // "near" message isn't useful if line is empty
        rv = sb_error_new(type, msg);
    else
        rv = sb_error_new_printf(type,
            "%s\nError occurred near line %d, position %d: %s", msg, lineno,
            pos, line);

    free(msg);
    free(line);

    return rv;
}


void
blogc_error_print(sb_error_t *err)
{
    if (err == NULL)
        return;

    switch(err->code) {
        case BLOGC_ERROR_SOURCE_PARSER:
            fprintf(stderr, "blogc: error: source: %s\n", err->msg);
            break;
        case BLOGC_ERROR_TEMPLATE_PARSER:
            fprintf(stderr, "blogc: error: template: %s\n", err->msg);
            break;
        case BLOGC_ERROR_LOADER:
            fprintf(stderr, "blogc: error: loader: %s\n", err->msg);
            break;
        case BLOGC_WARNING_DATETIME_PARSER:
            fprintf(stderr, "blogc: warning: datetime: %s\n", err->msg);
            break;
        default:
            fprintf(stderr, "blogc: error: %s\n", err->msg);
    }
}
