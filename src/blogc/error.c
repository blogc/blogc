/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2017 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#include <stdio.h>
#include <squareball.h>

#include "error.h"


void
blogc_error_print(sb_error_t *err)
{
    if (err == NULL)
        return;

    fprintf(stderr, "blogc: ");

    switch((blogc_error_type_t) err->code) {
        case BLOGC_ERROR_SOURCE_PARSER:
            fprintf(stderr, "error: source: %s\n", err->msg);
            break;
        case BLOGC_ERROR_TEMPLATE_PARSER:
            fprintf(stderr, "error: template: %s\n", err->msg);
            break;
        case BLOGC_ERROR_LOADER:
            fprintf(stderr, "error: loader: %s\n", err->msg);
            break;
        case BLOGC_WARNING_DATETIME_PARSER:
            fprintf(stderr, "warning: datetime: %s\n", err->msg);
            break;
        default:
            fprintf(stderr, "error: %s\n", err->msg);
    }
}
