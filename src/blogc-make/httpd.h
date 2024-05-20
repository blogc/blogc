// SPDX-FileCopyrightText: 2014-2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "../common/utils.h"
#include "ctx.h"
#include "rules.h"

int bm_httpd_run(bm_ctx_t **ctx, bm_rule_exec_func_t rule_exec, bc_slist_t *outputs,
    bc_trie_t *args);
