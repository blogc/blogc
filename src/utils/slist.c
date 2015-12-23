/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2015 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include "utils.h"


b_slist_t*
b_slist_append(b_slist_t *l, void *data)
{
    b_slist_t *node = malloc(sizeof(b_slist_t));
    if (node == NULL) {
        l = NULL;
        return l;
    }
    node->data = data;
    node->next = NULL;
    if (l == NULL)
        l = node;
    else {
        b_slist_t *tmp;
        for (tmp = l; tmp->next != NULL; tmp = tmp->next);
        tmp->next = node;
    }
    return l;
}


void
b_slist_free_full(b_slist_t *l, b_free_func_t free_func)
{
    while (l != NULL) {
        b_slist_t *tmp = l->next;
        free_func(l->data);
        free(l);
        l = tmp;
    }
}


void
b_slist_free(b_slist_t *l)
{
    while (l != NULL) {
        b_slist_t *tmp = l->next;
        free(l);
        l = tmp;
    }
}


unsigned int
b_slist_length(b_slist_t *l)
{
    unsigned int i;
    b_slist_t *tmp;
    for (tmp = l, i = 0; tmp != NULL; tmp = tmp->next, i++);
    return i;
}
