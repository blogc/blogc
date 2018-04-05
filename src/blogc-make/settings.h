/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2017 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifndef _MAKE_SETTINGS_H
#define _MAKE_SETTINGS_H

#include <stddef.h>
#include <squareball.h>

typedef struct {
    char *root_dir;
    sb_trie_t *global;
    sb_trie_t *settings;
    char **posts;
    char **pages;
    char **copy;
    char **tags;
} bm_settings_t;

bm_settings_t* bm_settings_parse(const char *content, size_t content_len,
    sb_error_t **err);
bm_settings_t* bm_settings_parse_file(const char *filename, sb_error_t **err);
void bm_settings_free(bm_settings_t *settings);

#endif /* _MAKE_SETTINGS_H */
