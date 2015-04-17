/*
 * blogc: A blog compiler.
 * Copyright (C) 2015 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the LGPL-2 License.
 * See the file COPYING.
 */

#ifndef _OUTPUT_H
#define _OUTPUT_H

#include <stdlib.h>

void blogc_parser_syntax_error(const char *name, const char *src,
    size_t src_len, size_t current);

#endif /* _OUTPUT_H */
