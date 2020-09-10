/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2020 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#include <stdlib.h>
#include "../common/utils.h"
#include "toctree.h"

bc_slist_t*
blogc_toctree_append(bc_slist_t *headers, size_t level, const char *slug, const char *text)
{
    if (level == 0)
        return headers;

    blogc_toctree_header_t *t = bc_malloc(sizeof(blogc_toctree_header_t));
    t->level = level;
    t->slug = bc_strdup(slug);
    t->text = bc_strdup(text);
    return bc_slist_append(headers, t);
}


char*
blogc_toctree_render(bc_slist_t *headers, int maxdepth, const char *endl)
{
    if (headers == NULL || maxdepth == 0)
        return NULL;

    // find lower level
    size_t lower_level = 0;
    for (bc_slist_t *l = headers; l != NULL; l = l->next) {
        size_t lv = ((blogc_toctree_header_t*) l->data)->level;
        if (lower_level == 0 || lower_level > lv) {
            lower_level = lv;
        }
    }

    if (lower_level == 0)
        return NULL;

    // render
    bc_string_t *rv = bc_string_new();
    bc_string_append_printf(rv, "<ul>%s", endl == NULL ? "\n" : endl);
    size_t spacing = 4;
    size_t current_level = lower_level;
    for (bc_slist_t *l = headers; l != NULL; l = l->next) {
        blogc_toctree_header_t *t = l->data;
        if (t->level - lower_level >= maxdepth) {
            continue;
        }
        while (current_level > t->level) {
            spacing -= 4;
            bc_string_append_printf(rv, "%*s</ul>%s", spacing, "",
                endl == NULL ? "\n" : endl);
            current_level--;
        }
        while (current_level < t->level) {
            bc_string_append_printf(rv, "%*s<ul>%s", spacing, "",
                endl == NULL ? "\n" : endl);
            current_level++;
            spacing += 4;
        }
        bc_string_append_printf(rv, "%*s<li>", spacing, "");
        if (t->slug != NULL) {
            bc_string_append_printf(rv, "<a href=\"#%s\">%s</a>", t->slug,
                t->text != NULL ? t->text : "");
        }
        else {
            bc_string_append(rv, t->text);
        }
        bc_string_append_printf(rv, "</li>%s", endl == NULL ? "\n" : endl);
    }

    // close leftovers
    while (current_level >= lower_level) {
        spacing -= 4;
        bc_string_append_printf(rv, "%*s</ul>%s", spacing, "",
            endl == NULL ? "\n" : endl);
        current_level--;
    }

    return bc_string_free(rv, false);
}


static void
free_header(blogc_toctree_header_t *h)
{
    free(h->slug);
    free(h->text);
    free(h);
}


void
blogc_toctree_free(bc_slist_t *l)
{
    bc_slist_free_full(l, (bc_free_func_t) free_header);
}
