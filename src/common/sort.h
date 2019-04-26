/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2019 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifndef _SORT_H
#define _SORT_H

#include "utils.h"

typedef int (*bc_sort_func_t) (const void *a, const void *b);

bc_slist_t* bc_slist_sort(bc_slist_t *l, bc_sort_func_t cmp);

#endif /* _SORT_H */
