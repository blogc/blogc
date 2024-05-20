// SPDX-FileCopyrightText: 2014-2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <stdbool.h>
#include "../common/utils.h"

const char* blogc_get_variable(const char *name, bc_trie_t *global, bc_trie_t *local);
char* blogc_format_date(const char *date, bc_trie_t *global, bc_trie_t *local);
char* blogc_format_variable(const char *name, bc_trie_t *global, bc_trie_t *local,
    const char *foreach_name, bc_slist_t *foreach_var);
bc_slist_t* blogc_split_list_variable(const char *name, bc_trie_t *global,
    bc_trie_t *local);
char* blogc_render(bc_slist_t *tmpl, bc_slist_t *sources, bc_slist_t *listing_entries,
    bc_trie_t *config, bool listing);
