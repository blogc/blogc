// SPDX-FileCopyrightText: 2014-2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

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
