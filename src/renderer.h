/*
 * blogc: A blog compiler.
 * Copyright (C) 2015 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file COPYING.
 */

#ifndef _RENDERER_H
#define _RENDERER_H

#include <stdbool.h>
#include "utils/utils.h"

const char* blogc_get_variable(const char *name, b_trie_t *global, b_trie_t *local);
char* blogc_format_date(b_trie_t *global, b_trie_t *local);
char* blogc_render(b_slist_t *tmpl, b_slist_t *sources, b_trie_t *config,
    bool listing);

#endif /* _RENDERER_H */
