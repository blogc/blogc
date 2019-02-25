/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2019 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifndef _MAKE_SASS_H
#define _MAKE_SASS_H

#include "ctx.h"
#include "../common/utils.h"

bc_slist_t* bm_sass_outputlist(bm_ctx_t *ctx);
int bm_sass_exec(bm_ctx_t *ctx, bc_slist_t *outputs, bc_trie_t *args);

#endif /* _MAKE_SASS_H */
