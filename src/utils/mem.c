/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2015 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

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


void*
b_realloc(void *ptr, size_t size)
{
    // simple things even simpler :P
    void *rv = realloc(ptr, size);
    if (rv == NULL && size != 0) {
        fprintf(stderr, "fatal error: Failed to reallocate memory!\n");
        free(ptr);
        exit(1);
    }
    return rv;
}
