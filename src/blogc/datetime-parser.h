/*
 * blogc: A blog compiler.
 * Copyright (C) 2015-2016 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifndef _DATETIME_H
#define _DATETIME_H

#include "error.h"

char* blogc_convert_datetime(const char *orig, const char *format,
    blogc_error_t **err);

#endif /* _DATETIME_H */
