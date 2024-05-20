// SPDX-FileCopyrightText: 2014-2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <stdbool.h>
#include "ctx.h"
#include "../common/utils.h"

typedef bc_slist_t* (*bm_rule_outputlist_func_t) (bm_ctx_t *ctx);
typedef int (*bm_rule_exec_func_t) (bm_ctx_t *ctx, bc_slist_t *outputs,
    bc_trie_t *args);

typedef struct {
    const char *name;
    const char *help;
    bm_rule_outputlist_func_t outputlist_func;
    bm_rule_exec_func_t exec_func;
} bm_rule_t;

bc_trie_t* bm_rule_parse_args(const char *sep);
int bm_rule_executor(bm_ctx_t *ctx, bc_slist_t *rule_list);
int bm_rule_execute(bm_ctx_t *ctx, const bm_rule_t *rule, bc_trie_t *args);
bool bm_rule_need_rebuild(bc_slist_t *sources, bm_filectx_t *settings,
    bm_filectx_t *listing_entry, bm_filectx_t *template, bm_filectx_t *output,
    bool only_first_source);
bc_slist_t* bm_rule_list_built_files(bm_ctx_t *ctx);
void bm_rule_print_help(void);
