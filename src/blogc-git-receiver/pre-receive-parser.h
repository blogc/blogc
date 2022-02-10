/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2020 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifndef _PRE_RECEIVE_PARSER_H
#define _PRE_RECEIVE_PARSER_H

#include <stddef.h>
#include "../common/utils.h"

bc_trie_t* bgr_pre_receive_parse(const char *input, size_t input_len);

#endif /* _PRE_RECEIVE_PARSER_H */
