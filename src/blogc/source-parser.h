// SPDX-FileCopyrightText: 2014-2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <stddef.h>
#include "../common/error.h"
#include "../common/utils.h"

bc_trie_t* blogc_source_parse(const char *src, size_t src_len, int toctree_maxdepth,
    bc_error_t **err);
