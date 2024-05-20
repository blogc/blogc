// SPDX-FileCopyrightText: 2014-2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
// SPDX-License-Identifier: BSD-3-Clause

#include <stdbool.h>
#include "utils.h"
#include "sort.h"


bc_slist_t*
bc_slist_sort(bc_slist_t *l, bc_sort_func_t cmp)
{
    if (l == NULL) {
        return NULL;
    }

    bool swapped = false;
    bc_slist_t *lptr = NULL;
    bc_slist_t *rptr = NULL;

    do {
        swapped = false;
        lptr = l;

        while (lptr->next != rptr) {
            if (0 < cmp(lptr->data, lptr->next->data)) {
                void *tmp = lptr->data;
                lptr->data = lptr->next->data;
                lptr->next->data = tmp;
                swapped = true;
            }

            lptr = lptr->next;
        }

        rptr = lptr;
    } while(swapped);

    return l;
}
