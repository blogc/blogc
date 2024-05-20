// SPDX-FileCopyrightText: 2014-2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "../common/error.h"
#include "../common/utils.h"

char* blogc_get_filename(const char *f);
bc_slist_t* blogc_template_parse_from_file(const char *f, bc_error_t **err);
bc_trie_t* blogc_source_parse_from_file(bc_trie_t *conf, const char *f,
    bc_error_t **err);
bc_slist_t* blogc_source_parse_from_files(bc_trie_t *conf, bc_slist_t *l,
    bc_error_t **err);
