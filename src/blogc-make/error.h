/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2017 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifndef _MAKE_ERROR_H
#define _MAKE_ERROR_H

typedef enum {
    BLOGC_MAKE_ERROR_SETTINGS = 100,
    BLOGC_MAKE_ERROR_EXEC,
    BLOGC_MAKE_ERROR_ATOM,
} bm_error_type_t;

void bm_error_print(sb_error_t *err);

#endif /* _MAKE_ERROR_H */
