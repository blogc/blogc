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
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "utils/utils.h"
#include "error.h"


blogc_error_t*
blogc_error_new(blogc_error_type_t type, const char *msg)
{
    blogc_error_t *err = b_malloc(sizeof(blogc_error_t));
    err->type = type;
    err->msg = b_strdup(msg);
    return err;
}


blogc_error_t*
blogc_error_new_printf(blogc_error_type_t type, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    char *tmp = b_strdup_vprintf(format, ap);
    va_end(ap);
    blogc_error_t *rv = blogc_error_new(type, tmp);
    free(tmp);
    return rv;
}


blogc_error_t*
blogc_error_parser(blogc_error_type_t type, const char *src, size_t src_len,
    size_t current, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    char *msg = b_strdup_vprintf(format, ap);
    va_end(ap);

    b_string_t *str = b_string_new();
    while (current < src_len) {
        char c = src[current];

        if (c == '\r' || c == '\n')
            break;

        b_string_append_c(str, c);

        current++;
    }
    char *line = b_string_free(str, false);

    blogc_error_t *rv = NULL;

    if (strlen(line) == 0)  // "near to" message isn't useful if line is empty
        rv = blogc_error_new(type, msg);
    else
        rv = blogc_error_new_printf(type,
            "%s\nError occurred near to '%s'", msg, line);

    free(msg);
    free(line);

    return rv;
}


void
blogc_error_print(blogc_error_t *err)
{
    if (err == NULL)
        return;

    char *tmp = NULL;

    switch(err->type) {
        case BLOGC_ERROR_SOURCE_PARSER:
            tmp = b_strdup("Source parser error");
            break;
        case BLOGC_ERROR_TEMPLATE_PARSER:
            tmp = b_strdup("Template parser error");
            break;
        case BLOGC_ERROR_LOADER:
            tmp = b_strdup("Loader error");
            break;
        default:
            tmp = b_strdup("Unknown error");
    }

    fprintf(stderr, "%s: %s\n", tmp, err->msg);

    free(tmp);
}


void
blogc_error_free(blogc_error_t *err)
{
    if (err == NULL)
        return;
    free(err->msg);
    free(err);
}
