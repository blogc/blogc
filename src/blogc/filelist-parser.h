/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2019 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifndef _FILELIST_PARSER_H
#define _FILELIST_PARSER_H

#include "../common/utils.h"

bc_slist_t* blogc_filelist_parse(const char *src, size_t src_len);

#endif /* _FILELIST_PARSER_H */
