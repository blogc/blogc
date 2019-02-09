/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2019 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifndef ___FUNCVARS_H
#define ___FUNCVARS_H

#include <stdbool.h>
#include "../common/utils.h"

typedef void (*blogc_funcvars_func_t) (bc_trie_t*);

void blogc_funcvars_eval(bc_trie_t *global, const char *name);

#endif /* ___FUNCVARS_H */
