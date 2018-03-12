/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2017 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifndef _MAKE_RELOADER_H
#define _MAKE_RELOADER_H

#include <stdbool.h>
#include "../common/utils.h"
#include "ctx.h"
#include "rules.h"

typedef struct {
    bm_ctx_t *ctx;
    bm_rule_exec_func_t rule_exec;
    bc_slist_t *outputs;
    bc_trie_t *args;
    bool running;
} bm_reloader_t;

bm_reloader_t* bm_reloader_new(bm_ctx_t *ctx, bm_rule_exec_func_t rule_exec,
    bc_slist_t *outputs, bc_trie_t *args);
void bm_reloader_stop(bm_reloader_t *reloader);

#endif /* _MAKE_RELOADER_H */
