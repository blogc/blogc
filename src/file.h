/*
 * blogc: A blog compiler.
 * Copyright (C) 2015-2016 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifndef _FILE_H
#define _FILE_H

#include <stdio.h>
#include <stdlib.h>
#include "error.h"

#define BLOGC_FILE_CHUNK_SIZE 1024

char* blogc_file_get_contents(const char *path, size_t *len, blogc_error_t **err);
int blogc_fprintf(FILE *stream, const char *format, ...);

#endif /* _FILE_H */
