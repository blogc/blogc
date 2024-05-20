// SPDX-FileCopyrightText: 2014-2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "utils.h"

typedef int (*bc_sort_func_t) (const void *a, const void *b);

bc_slist_t* bc_slist_sort(bc_slist_t *l, bc_sort_func_t cmp);
