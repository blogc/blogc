/*
 * blogc: A blog compiler.
 * Copyright (C) 2015 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifndef _DIRECTIVES_H
#define _DIRECTIVES_H

#include "utils/utils.h"
#include "error.h"

char* blogc_directive_loader(const char *name, const char *argument,
    b_trie_t *params, blogc_error_t **err);

#endif /* _DIRECTIVES_H */
