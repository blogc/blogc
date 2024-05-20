// SPDX-FileCopyrightText: 2014-2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <stdbool.h>
#include "../common/utils.h"

typedef void (*blogc_funcvars_func_t) (bc_trie_t*);

void blogc_funcvars_eval(bc_trie_t *global, const char *name);
