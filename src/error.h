/*
 * blogc: A blog compiler.
 * Copyright (C) 2015-2016 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifndef _ERROR_H
#define _ERROR_H

#include <stdlib.h>
#include <stdarg.h>

typedef enum {
    BLOGC_ERROR_SOURCE_PARSER = 1,
    BLOGC_ERROR_TEMPLATE_PARSER,
    BLOGC_ERROR_LOADER,
    BLOGC_WARNING_DATETIME_PARSER,
} blogc_error_type_t;

typedef struct {
    char *msg;
    blogc_error_type_t type;
} blogc_error_t;

blogc_error_t* blogc_error_new(blogc_error_type_t type, const char *msg);
blogc_error_t* blogc_error_new_printf(blogc_error_type_t type, const char *format, ...);
blogc_error_t* blogc_error_parser(blogc_error_type_t type, const char *src,
    size_t src_len, size_t current, const char *format, ...);
void blogc_error_print(blogc_error_t *err);
void blogc_error_free(blogc_error_t *err);

#endif /* _ERROR_H */
