/*
 * blogc: A blog compiler.
 * Copyright (C) 2015-2016 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifndef _ERRORS_H
#define _ERRORS_H

#include "../common/error.h"

typedef enum {
    BLOGC_ERROR_SOURCE_PARSER = 1,
    BLOGC_ERROR_TEMPLATE_PARSER,
    BLOGC_ERROR_LOADER,
    BLOGC_ERROR_FILE,
    BLOGC_WARNING_DATETIME_PARSER,
} blogc_error_type_t;

void blogc_error_print(bc_error_t *err);

#endif /* _ERRORS_H */
