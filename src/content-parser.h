/*
 * blogc: A blog compiler.
 * Copyright (C) 2015 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifndef _CONTENT_PARSER_H
#define _CONTENT_PARSER_H

#include <stdlib.h>
#include "error.h"

char* blogc_content_parse_inline(const char *src);
char* blogc_content_parse(const char *src);

#endif /* _CONTENT_PARSER_H */
