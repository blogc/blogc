/*
 * blogc: A blog compiler.
 * Copyright (C) 2015-2016 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifndef _ERROR_H
#define _ERROR_H

#include <stddef.h>

typedef struct {
    char *msg;
    int type;
} bc_error_t;

bc_error_t* bc_error_new(int type, const char *msg);
bc_error_t* bc_error_new_printf(int type, const char *format, ...);
bc_error_t* bc_error_parser(int type, const char *src, size_t src_len,
    size_t current, const char *format, ...);
void bc_error_print(bc_error_t *err);
void bc_error_free(bc_error_t *err);

#endif /* _ERROR_H */
