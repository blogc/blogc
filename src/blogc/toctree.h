// SPDX-FileCopyrightText: 2014-2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "../common/utils.h"

typedef struct {
    size_t level;
    char *slug;
    char *text;
} blogc_toctree_header_t;

bc_slist_t* blogc_toctree_append(bc_slist_t *headers, size_t level,
    const char *slug, const char *text);
char* blogc_toctree_render(bc_slist_t *headers, int maxdepth,
    const char *endl);
void blogc_toctree_free(bc_slist_t *l);
