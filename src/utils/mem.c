/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2015 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file COPYING.
 */

#include <stdlib.h>
#include <stdio.h>
#include "utils.h"


void*
b_malloc(size_t size)
{
    // simple things simple!
    void *rv = malloc(size);
    if (rv == NULL) {
        fprintf(stderr, "fatal error: Failed to allocate memory!\n");
        exit(1);
    }
    return rv;
}
