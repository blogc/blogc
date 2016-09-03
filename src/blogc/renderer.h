/*
 * blogc: A blog compiler.
 * Copyright (C) 2015-2016 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifndef _RENDERER_H
#define _RENDERER_H

#include <stdbool.h>
#include "../common/utils.h"

const char* blogc_get_variable(const char *name, bc_trie_t *global, bc_trie_t *local);
char* blogc_format_date(const char *date, bc_trie_t *global, bc_trie_t *local);
char* blogc_format_variable(const char *name, bc_trie_t *global, bc_trie_t *local,
    bc_slist_t *foreach_var);
bc_slist_t* blogc_split_list_variable(const char *name, bc_trie_t *global,
    bc_trie_t *local);
char* blogc_render(bc_slist_t *tmpl, bc_slist_t *sources, bc_trie_t *config,
    bool listing);

#endif /* _RENDERER_H */
