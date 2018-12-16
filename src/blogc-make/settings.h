/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2018 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifndef _MAKE_SETTINGS_H
#define _MAKE_SETTINGS_H

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

#endif /* _MAKE_SETTINGS_H */
