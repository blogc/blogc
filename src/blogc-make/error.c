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
bm_error_print(sb_error_t *err)
{
    if (err == NULL)
        return;

    fprintf(stderr, "blogc-make: ");

    switch((bm_error_type_t) err->code) {
        case BLOGC_MAKE_ERROR_SETTINGS:
            fprintf(stderr, "error: settings: %s\n", err->msg);
            break;
        case BLOGC_MAKE_ERROR_EXEC:
            fprintf(stderr, "error: exec: %s\n", err->msg);
            break;
        case BLOGC_MAKE_ERROR_ATOM:
            fprintf(stderr, "error: atom: %s\n", err->msg);
            break;
        default:
            fprintf(stderr, "error: %s\n", err->msg);
    }
}
