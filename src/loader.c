/*
 * blogc: A blog compiler.
 * Copyright (C) 2015 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file COPYING.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "utils/utils.h"
#include "source-parser.h"
#include "template-parser.h"
#include "loader.h"
#include "error.h"


char*
blogc_file_get_contents(const char *path, size_t *len, blogc_error_t **err)
{
    if (err == NULL || *err != NULL)
        return NULL;

    *len = 0;
    FILE *fp = fopen(path, "r");

    if (fp == NULL) {
        int tmp_errno = errno;
        *err = blogc_error_new_printf(BLOGC_ERROR_LOADER,
            "Failed to open file (%s): %s", path, strerror(tmp_errno));
        return NULL;
    }

    b_string_t *str = b_string_new();
    char buffer[BLOGC_FILE_CHUNK_SIZE];

    while (!feof(fp)) {
        size_t read_len = fread(buffer, sizeof(char), BLOGC_FILE_CHUNK_SIZE, fp);
        *len += read_len;
        b_string_append_len(str, buffer, read_len);
    }
    fclose(fp);
    return b_string_free(str, false);
}


b_slist_t*
blogc_template_parse_from_file(const char *f, blogc_error_t **err)
{
    if (err == NULL || *err != NULL)
        return NULL;
    size_t len;
    char *s = blogc_file_get_contents(f, &len, err);
    if (s == NULL)
        return NULL;
    b_slist_t *rv = blogc_template_parse(s, len, err);
    free(s);
    return rv;
}


blogc_source_t*
blogc_source_parse_from_file(const char *f, blogc_error_t **err)
{
    if (err == NULL || *err != NULL)
        return NULL;
    size_t len;
    char *s = blogc_file_get_contents(f, &len, err);
    if (s == NULL)
        return NULL;
    blogc_source_t *rv = blogc_source_parse(s, len, err);
    free(s);
    return rv;
}


b_slist_t*
blogc_source_parse_from_files(b_slist_t *l, blogc_error_t **err)
{
    blogc_error_t *tmp_err = NULL;
    b_slist_t *rv = NULL;

    for (b_slist_t *tmp = l; tmp != NULL; tmp = tmp->next) {
        char *f = tmp->data;
        blogc_source_t *s = blogc_source_parse_from_file(f, &tmp_err);
        if (s == NULL) {
            *err = blogc_error_new_printf(BLOGC_ERROR_LOADER,
                "An error occurred while parsing source file: %s\n\n%s",
                f, tmp_err->msg);
            blogc_error_free(tmp_err);
            tmp_err = NULL;
            b_slist_free_full(rv, blogc_source_free);
            rv = NULL;
            break;
        }
        rv = b_slist_append(rv, s);
    }
    return rv;
}
