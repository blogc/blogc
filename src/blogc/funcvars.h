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
#include <squareball.h>

typedef void (*blogc_funcvars_func_t) (sb_trie_t*);

void blogc_funcvars_eval(sb_trie_t *global, const char *name);

#endif /* ___FUNCVARS_H */
