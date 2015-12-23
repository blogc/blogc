/*
 * blogc: A blog compiler.
 * Copyright (C) 2015 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <squareball.h>
#include "file.h"
#include "source-parser.h"
#include "template-parser.h"
#include "loader.h"
#include "error.h"


char*
blogc_get_filename(const char *f)
{
    if (f == NULL)
        return NULL;

    if (strlen(f) == 0)
        return NULL;

    char *filename = sb_strdup(f);

    // keep a pointer to original string
    char *tmp = filename;

    bool removed_dot = false;
    for (int i = strlen(tmp); i >= 0 ; i--) {

        // remove last extension
        if (!removed_dot && tmp[i] == '.') {
            tmp[i] = '\0';
            removed_dot = true;
            continue;
        }

        if (tmp[i] == '/' || tmp[i] == '\\') {
            tmp += i + 1;
            break;
        }
    }

    char *final_filename = sb_strdup(tmp);
    free(filename);

    return final_filename;
}


sb_slist_t*
blogc_template_parse_from_file(const char *f, blogc_error_t **err)
{
    if (err == NULL || *err != NULL)
        return NULL;
    size_t len;
    char *s = blogc_file_get_contents(f, &len, err);
    if (s == NULL)
        return NULL;
    sb_slist_t *rv = blogc_template_parse(s, len, err);
    free(s);
    return rv;
}


sb_trie_t*
blogc_source_parse_from_file(const char *f, blogc_error_t **err)
{
    if (err == NULL || *err != NULL)
        return NULL;
    size_t len;
    char *s = blogc_file_get_contents(f, &len, err);
    if (s == NULL)
        return NULL;
    sb_trie_t *rv = blogc_source_parse(s, len, err);

    // set FILENAME variable
    if (rv != NULL) {
        char *filename = blogc_get_filename(f);
        if (filename != NULL)
            sb_trie_insert(rv, "FILENAME", filename);
    }

    free(s);
    return rv;
}


sb_slist_t*
blogc_source_parse_from_files(sb_trie_t *conf, sb_slist_t *l, blogc_error_t **err)
{
    blogc_error_t *tmp_err = NULL;
    sb_slist_t *rv = NULL;
    unsigned int with_date = 0;

    const char *filter_tag = sb_trie_lookup(conf, "FILTER_TAG");
    const char *filter_page = sb_trie_lookup(conf, "FILTER_PAGE");
    const char *filter_per_page = sb_trie_lookup(conf, "FILTER_PER_PAGE");

    long page = strtol(filter_page != NULL ? filter_page : "", NULL, 10);
    if (page <= 0)
        page = 1;
    long per_page = strtol(filter_per_page != NULL ? filter_per_page : "10",
        NULL, 10);
    if (per_page <= 0)
        per_page = 10;

    // poor man's pagination
    unsigned int start = (page - 1) * per_page;
    unsigned int end = start + per_page;
    unsigned int counter = 0;

    for (sb_slist_t *tmp = l; tmp != NULL; tmp = tmp->next) {
        char *f = tmp->data;
        sb_trie_t *s = blogc_source_parse_from_file(f, &tmp_err);
        if (s == NULL) {
            *err = blogc_error_new_printf(BLOGC_ERROR_LOADER,
                "An error occurred while parsing source file: %s\n\n%s",
                f, tmp_err->msg);
            blogc_error_free(tmp_err);
            tmp_err = NULL;
            sb_slist_free_full(rv, (sb_free_func_t) sb_trie_free);
            rv = NULL;
            break;
        }
        if (filter_tag != NULL) {
            const char *tags_str = sb_trie_lookup(s, "TAGS");
            // if user wants to filter by tag and no tag is provided, skip it
            if (tags_str == NULL) {
                sb_trie_free(s);
                continue;
            }
            char **tags = sb_str_split(tags_str, ',', 0);
            bool found = false;
            for (unsigned int i = 0; tags[i] != NULL; i++)
                if (0 == strcmp(sb_str_strip(tags[i]), filter_tag))
                    found = true;
            sb_strv_free(tags);
            if (!found) {
                sb_trie_free(s);
                continue;
            }
        }
        if (filter_page != NULL) {
            if (counter < start || counter >= end) {
                counter++;
                sb_trie_free(s);
                continue;
            }
            counter++;
        }
        if (sb_trie_lookup(s, "DATE") != NULL)
            with_date++;
        rv = sb_slist_append(rv, s);
    }

    if (with_date > 0 && with_date < sb_slist_length(rv))
        // fatal error, maybe?
        blogc_fprintf(stderr,
            "blogc: warning: 'DATE' variable provided for at least one source "
            "file, but not for all source files. This means that you may get "
            "wrong values for 'DATE_FIRST' and 'DATE_LAST' variables.\n");

    bool first = true;
    for (sb_slist_t *tmp = rv; tmp != NULL; tmp = tmp->next) {
        sb_trie_t *s = tmp->data;
        if (first) {
            const char *val = sb_trie_lookup(s, "DATE");
            if (val != NULL)
                sb_trie_insert(conf, "DATE_FIRST", sb_strdup(val));
            val = sb_trie_lookup(s, "FILENAME");
            if (val != NULL)
                sb_trie_insert(conf, "FILENAME_FIRST", sb_strdup(val));
            first = false;
        }
        if (tmp->next == NULL) {  // last
            const char *val = sb_trie_lookup(s, "DATE");
            if (val != NULL)
                sb_trie_insert(conf, "DATE_LAST", sb_strdup(val));
            val = sb_trie_lookup(s, "FILENAME");
            if (val != NULL)
                sb_trie_insert(conf, "FILENAME_LAST", sb_strdup(val));
        }
    }

    if (filter_page != NULL) {
        unsigned int last_page = ceilf(((float) counter) / per_page);
        sb_trie_insert(conf, "CURRENT_PAGE", sb_strdup_printf("%ld", page));
        if (page > 1)
            sb_trie_insert(conf, "PREVIOUS_PAGE", sb_strdup_printf("%ld", page - 1));
        if (page < last_page)
            sb_trie_insert(conf, "NEXT_PAGE", sb_strdup_printf("%ld", page + 1));
        if (sb_slist_length(rv) > 0)
            sb_trie_insert(conf, "FIRST_PAGE", sb_strdup("1"));
        if (last_page > 0)
            sb_trie_insert(conf, "LAST_PAGE", sb_strdup_printf("%d", last_page));
    }

    return rv;
}
