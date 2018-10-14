/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2018 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#include <stdbool.h>
#include <string.h>
#include "../common/utils.h"


char*
bm_generate_filename(const char *dir, const char *prefix, const char *fname,
    const char *ext)
{
    bool have_prefix = prefix != NULL && prefix[0] != '\0';
    bool have_fname = fname != NULL && fname[0] != '\0';
    bool have_ext = ext != NULL && ext[0] != '\0';
    bool have_ext_noslash = have_ext && ext[0] != '/';
    bool is_index = have_fname && have_ext && (
        (0 == strcmp(fname, "index")) && ext[0] == '/');

    bc_string_t *rv = bc_string_new();

    if (dir != NULL && (have_prefix || have_fname || have_ext))
        bc_string_append(rv, dir);

    if ((have_prefix || have_fname || have_ext_noslash) && !is_index)
        bc_string_append_c(rv, '/');

    if (have_prefix)
        bc_string_append(rv, prefix);

    // with fname we have posts, pages and tags
    if (have_fname) {
        if (have_prefix && have_fname && fname[0] != '/')
            bc_string_append_c(rv, '/');
        if (!is_index)
            bc_string_append(rv, fname);
    }

    // no fname means index
    else if (have_ext_noslash) {
        if (have_fname)
            bc_string_append_c(rv, '/');
        if (!have_prefix)
            bc_string_append(rv, "index");
    }

    if (have_ext)
        bc_string_append(rv, ext);

    if (rv->len == 0) {
        bc_string_free(rv, true);
        return NULL;
    }

    return bc_string_free(rv, false);
}
