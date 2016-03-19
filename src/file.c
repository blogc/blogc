/*
 * blogc: A blog compiler.
 * Copyright (C) 2015-2016 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdarg.h>
#include <stdio.h>
#include <squareball.h>
#include "file.h"
#include "error.h"

// this would belong to loader.c, but we need it in a separated file to be
// able to mock it when unit testing the loader functions.


char*
blogc_file_get_contents(const char *path, size_t *len, sb_error_t **err)
{
    return sb_file_get_contents(path, len, err);
}


int
blogc_fprintf(FILE *stream, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    int rv = vfprintf(stream, format, ap);
    va_end(ap);
    return rv;
}
