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

typedef char* (*blogc_funcvars_func_t) (void);

char* blogc_funcvars_lookup(const char *name, bc_trie_t *global);

#endif /* ___FUNCVARS_H */
