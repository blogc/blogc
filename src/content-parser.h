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
#include <stdbool.h>

char* blogc_slugify(const char *str);
char* blogc_content_parse_inline(const char *src);
bool blogc_is_ordered_list_item(const char *str, size_t prefix_len);
char* blogc_content_parse(const char *src, size_t *end_excerpt);

#endif /* _CONTENT_PARSER_H */
