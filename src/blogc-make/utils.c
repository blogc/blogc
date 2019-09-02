/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2019 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <squareball.h>

#include "utils.h"

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

    sb_string_t *rv = sb_string_new();

    if (dir != NULL && (have_prefix || have_fname || have_ext))
        sb_string_append(rv, dir);

    if ((have_prefix || have_fname || have_ext_noslash) && !is_index)
        sb_string_append_c(rv, '/');

    if (have_prefix)
        sb_string_append(rv, prefix);

    // with fname we have posts, pages and tags
    if (have_fname) {
        if (have_prefix && have_fname && fname[0] != '/')
            sb_string_append_c(rv, '/');
        if (!is_index)
            sb_string_append(rv, fname);
    }

    // no fname means index
    else if (have_ext_noslash) {
        if (have_fname)
            sb_string_append_c(rv, '/');
        if (!have_prefix)
            sb_string_append(rv, "index");
    }

    if (have_ext)
        sb_string_append(rv, ext);

    if (rv->len == 0) {
        sb_string_free(rv, true);
        return NULL;
    }

    return sb_string_free(rv, false);
}


char*
bm_generate_filename2(const char *dir, const char *prefix, const char *fname,
    const char *prefix2, const char *fname2, const char *ext)
{
    bool have_prefix = prefix != NULL && prefix[0] != '\0';
    bool have_fname = fname != NULL && fname[0] != '\0';
    bool have_prefix2 = prefix2 != NULL && prefix2[0] != '\0';

    sb_string_t *p = sb_string_new();

    if (have_prefix)
        sb_string_append(p, prefix);

    if (have_prefix && (have_fname || have_prefix2))
        sb_string_append_c(p, '/');

    if (have_fname)
        sb_string_append(p, fname);

    if (have_fname && have_prefix2)
        sb_string_append_c(p, '/');

    if (have_prefix2)
        sb_string_append(p, prefix2);

    char *rv = bm_generate_filename(dir, p->str, fname2, ext);
    sb_string_free(p, true);

    return rv;
}


char*
bm_abspath(const char *path, sb_error_t **err)
{
    if (err == NULL || *err != NULL)
        return NULL;

    if (path[0] == '/') {
        return sb_strdup(path);
    }

    char cwd[PATH_MAX];
    if (NULL == getcwd(cwd, sizeof(cwd))) {
        *err = sb_strerror_new_printf(
            "utils: Failed to detect absolute path (%s): %s", path, strerror(errno));
        return NULL;
    }

    if (cwd[0] != '/') {
        *err = sb_strerror_new_printf(
            "utils: Failed to get current working directory: %s", cwd);
        return NULL;
    }

    return sb_strdup_printf("%s/%s", cwd, path);
}
