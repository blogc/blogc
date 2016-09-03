/*
 * blogc: A blog compiler.
 * Copyright (C) 2015-2016 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#include <stdio.h>
#include <stdlib.h>
#include "errors.h"
#include "../common/error.h"


void
blogc_error_print(bc_error_t *err)
{
    if (err == NULL)
        return;

    switch((blogc_error_type_t) err->type) {
        case BLOGC_ERROR_SOURCE_PARSER:
            fprintf(stderr, "blogc: error: source: %s\n", err->msg);
            break;
        case BLOGC_ERROR_TEMPLATE_PARSER:
            fprintf(stderr, "blogc: error: template: %s\n", err->msg);
            break;
        case BLOGC_ERROR_LOADER:
            fprintf(stderr, "blogc: error: loader: %s\n", err->msg);
            break;
        case BLOGC_ERROR_FILE:
            fprintf(stderr, "blogc: error: file: %s\n", err->msg);
            break;
        case BLOGC_WARNING_DATETIME_PARSER:
            fprintf(stderr, "blogc: warning: datetime: %s\n", err->msg);
            break;
        default:
            fprintf(stderr, "blogc: error: %s\n", err->msg);
    }
}
