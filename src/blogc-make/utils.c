// SPDX-FileCopyrightText: 2014-2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
// SPDX-License-Identifier: BSD-3-Clause

#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include "../common/error.h"
#include "../common/utils.h"

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif


char*
bm_generate_filename(const char *dir, const char *prefix, const char *fname,
    const char *ext)
{
    bool have_prefix = prefix != NULL && prefix[0] != '\0';
    bool have_fname = fname != NULL && fname[0] != '\0';
    bool have_ext = ext != NULL && ext[0] != '\0';
    bool have_ext_noslash = have_ext && ext[0] != '/';
    bool is_index = have_fname && have_ext && (
        (0 == strcmp(fname, "index")) && ext[0] == '/') && !have_prefix;

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


char*
bm_generate_filename2(const char *dir, const char *prefix, const char *fname,
    const char *prefix2, const char *fname2, const char *ext)
{
    bool have_prefix = prefix != NULL && prefix[0] != '\0';
    bool have_fname = fname != NULL && fname[0] != '\0';
    bool have_prefix2 = prefix2 != NULL && prefix2[0] != '\0';

    bc_string_t *p = bc_string_new();

    if (have_prefix)
        bc_string_append(p, prefix);

    if (have_prefix && (have_fname || have_prefix2))
        bc_string_append_c(p, '/');

    if (have_fname)
        bc_string_append(p, fname);

    if (have_fname && have_prefix2)
        bc_string_append_c(p, '/');

    if (have_prefix2)
        bc_string_append(p, prefix2);

    char *rv = bm_generate_filename(dir, p->str, fname2, ext);
    bc_string_free(p, true);

    return rv;
}


char*
bm_abspath(const char *path, bc_error_t **err)
{
    if (err == NULL || *err != NULL)
        return NULL;

    if (path[0] == '/') {
        return bc_strdup(path);
    }

    char cwd[PATH_MAX];
    if (NULL == getcwd(cwd, sizeof(cwd))) {
        *err = bc_error_new_printf(BLOGC_MAKE_ERROR_UTILS,
            "Failed to detect absolute path (%s): %s", path, strerror(errno));
        return NULL;
    }

    if (cwd[0] != '/') {
        *err = bc_error_new_printf(BLOGC_MAKE_ERROR_UTILS,
            "Failed to get current working directory: %s", cwd);
        return NULL;
    }

    return bc_strdup_printf("%s/%s", cwd, path);
}
