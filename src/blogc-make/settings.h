// SPDX-FileCopyrightText: 2014-2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <stddef.h>
#include "../common/error.h"
#include "../common/utils.h"

typedef struct {
    bc_trie_t *global;
    bc_trie_t *settings;
    char **posts;
    char **pages;
    char **copy;
    char **tags;
} bm_settings_t;

bm_settings_t* bm_settings_parse(const char *content, size_t content_len,
    bc_error_t **err);
void bm_settings_free(bm_settings_t *settings);
