/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2016 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifndef _CONFIG_PARSER_H
#define _CONFIG_PARSER_H

#include <stddef.h>
#include "utils.h"
#include "error.h"

typedef struct {
    bc_trie_t *root;
} bc_config_t;

bc_config_t* bc_config_parse(const char *src, size_t src_len,
    const char *list_sections[], bc_error_t **err);
char** bc_config_list_sections(bc_config_t *config);
char** bc_config_list_keys(bc_config_t *config, const char *section);
const char* bc_config_get(bc_config_t *config, const char *section,
    const char *key);
const char* bc_config_get_with_default(bc_config_t *config, const char *section,
    const char *key, const char *default_);
char** bc_config_get_list(bc_config_t *config, const char *section);
void bc_config_free(bc_config_t *config);

#endif /* _CONFIG_PARSER_H */
