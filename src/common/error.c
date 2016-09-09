/*
 * blogc: A blog compiler.
 * Copyright (C) 2015-2016 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "error.h"
#include "utils.h"


bc_error_t*
bc_error_new(bc_error_type_t type, const char *msg)
{
    bc_error_t *err = bc_malloc(sizeof(bc_error_t));
    err->type = type;
    err->msg = bc_strdup(msg);
    return err;
}


bc_error_t*
bc_error_new_printf(bc_error_type_t type, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    char *tmp = bc_strdup_vprintf(format, ap);
    va_end(ap);
    bc_error_t *rv = bc_error_new(type, tmp);
    free(tmp);
    return rv;
}


bc_error_t*
bc_error_parser(bc_error_type_t type, const char *src, size_t src_len,
    size_t current, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    char *msg = bc_strdup_vprintf(format, ap);
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

    char *line = bc_strndup(src + linestart, lineend - linestart);

    bc_error_t *rv = NULL;

    if (line[0] == '\0')  // "near" message isn't useful if line is empty
        rv = bc_error_new(type, msg);
    else
        rv = bc_error_new_printf(type,
            "%s\nError occurred near line %d, position %d: %s", msg, lineno,
            pos, line);

    free(msg);
    free(line);

    return rv;
}


// error handling is centralized here for the sake of simplicity :/
void
bc_error_print(bc_error_t *err)
{
    if (err == NULL)
        return;

    switch(err->type) {
        case BC_ERROR_CONFIG_PARSER:
            fprintf(stderr, "error: config-parser: %s\n", err->msg);
            break;
        case BLOGC_ERROR_SOURCE_PARSER:
            fprintf(stderr, "error: source: %s\n", err->msg);
            break;
        case BLOGC_ERROR_TEMPLATE_PARSER:
            fprintf(stderr, "error: template: %s\n", err->msg);
            break;
        case BLOGC_ERROR_LOADER:
            fprintf(stderr, "error: loader: %s\n", err->msg);
            break;
        case BLOGC_ERROR_FILE:
            fprintf(stderr, "error: file: %s\n", err->msg);
            break;
        case BLOGC_WARNING_DATETIME_PARSER:
            fprintf(stderr, "warning: datetime: %s\n", err->msg);
            break;
        default:
            fprintf(stderr, "blogc: error: %s\n", err->msg);
    }
}


void
bc_error_free(bc_error_t *err)
{
    if (err == NULL)
        return;
    free(err->msg);
    free(err);
}
