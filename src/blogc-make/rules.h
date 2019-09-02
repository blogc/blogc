/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2019 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifndef _MAKE_RULES_H
#define _MAKE_RULES_H

#include <stdbool.h>
#include <squareball.h>

#include "ctx.h"

typedef sb_slist_t* (*bm_rule_outputlist_func_t) (bm_ctx_t *ctx);
typedef int (*bm_rule_exec_func_t) (bm_ctx_t *ctx, sb_slist_t *outputs,
    sb_trie_t *args);

typedef struct {
    const char *name;
    const char *help;
    bm_rule_outputlist_func_t outputlist_func;
    bm_rule_exec_func_t exec_func;
    bool generate_files;
} bm_rule_t;

sb_trie_t* bm_rule_parse_args(const char *sep);
int bm_rule_executor(bm_ctx_t *ctx, sb_slist_t *rule_list);
int bm_rule_execute(bm_ctx_t *ctx, const bm_rule_t *rule, sb_trie_t *args);
bool bm_rule_need_rebuild(sb_slist_t *sources, bm_filectx_t *settings,
    bm_filectx_t *listing_entry, bm_filectx_t *template, bm_filectx_t *output,
    bool only_first_source);
sb_slist_t* bm_rule_list_built_files(bm_ctx_t *ctx);
void bm_rule_print_help(void);

#endif /* _MAKE_RULES_H */
