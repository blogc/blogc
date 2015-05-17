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

#ifdef HAVE_TIME_H
#include <time.h>
#endif /* HAVE_TIME_H */

#include <stdio.h>
#include <string.h>
#include "utils/utils.h"
#include "loader.h"
#include "source-parser.h"
#include "template-parser.h"
#include "renderer.h"


const char*
blogc_get_variable(const char *name, b_trie_t *global, b_trie_t *local)
{
    const char *rv = NULL;
    if (local != NULL) {
        rv = b_trie_lookup(local, name);
        if (rv != NULL)
            return rv;
    }
    if (global != NULL)
        rv = b_trie_lookup(global, name);
    return rv;
}


char*
blogc_format_date(const char *date, b_trie_t *global, b_trie_t *local)
{
    const char *date_format = blogc_get_variable("DATE_FORMAT", global, local);
    if (date == NULL)
        return NULL;
    if (date_format == NULL)
        return b_strdup(date);
#ifdef HAVE_TIME_H
    struct tm tm;
    memset(&tm, 0, sizeof(struct tm));
    if (NULL == strptime(date, "%Y-%m-%d %H:%M:%S", &tm)) {
        fprintf(stderr, "blogc: warning: Failed to parse DATE variable: %s\n",
            date);
        return b_strdup(date);
    }
    char tmp[1024];
    if (0 == strftime(tmp, sizeof(tmp), date_format, &tm)) {
        fprintf(stderr, "blogc: warning: Failed to format DATE variable, "
            "FORMAT is too long: %s\n", date_format);
        return b_strdup(date);
    }
    return b_strdup(tmp);
#else
    fprintf(stderr, "blogc: warning: Can't pre-process DATE variable.\n");
    return NULL;
#endif
}


char*
blogc_render(b_slist_t *tmpl, b_slist_t *sources, b_trie_t *config, bool listing)
{
    if (tmpl == NULL || sources == NULL)
        return NULL;

    b_slist_t *current_source = NULL;
    b_slist_t *listing_start = NULL;

    b_string_t *str = b_string_new();

    b_trie_t *tmp_source = NULL;
    const char *config_value = NULL;
    const char *config_var = NULL;
    char *config_value2 = NULL;

    unsigned int if_count = 0;
    unsigned int if_skip = 0;

    bool if_not = false;
    bool defined = false;
    bool inside_block = false;

    b_slist_t *tmp = tmpl;
    while (tmp != NULL) {
        blogc_template_stmt_t *stmt = tmp->data;

        switch (stmt->type) {

            case BLOGC_TEMPLATE_CONTENT_STMT:
                if (stmt->value != NULL)
                    b_string_append(str, stmt->value);
                break;

            case BLOGC_TEMPLATE_BLOCK_STMT:
                inside_block = true;
                if_count = 0;
                if (0 == strcmp("entry", stmt->value)) {
                    if (listing) {

                        // we can just skip anything and walk until the next
                        // 'endblock'
                        while (stmt->type != BLOGC_TEMPLATE_ENDBLOCK_STMT) {
                            tmp = tmp->next;
                            stmt = tmp->data;
                        }
                        break;
                    }
                    current_source = sources;
                    tmp_source = current_source->data;
                }
                else if ((0 == strcmp("listing", stmt->value)) ||
                         (0 == strcmp("listing_once", stmt->value))) {
                    if (!listing) {

                        // we can just skip anything and walk until the next
                        // 'endblock'
                        while (stmt->type != BLOGC_TEMPLATE_ENDBLOCK_STMT) {
                            tmp = tmp->next;
                            stmt = tmp->data;
                        }
                        break;
                    }
                }
                if (0 == strcmp("listing", stmt->value)) {
                    if (current_source == NULL) {
                        listing_start = tmp;
                        current_source = sources;
                    }
                    tmp_source = current_source->data;
                }
                break;

            case BLOGC_TEMPLATE_VARIABLE_STMT:
                if (stmt->value != NULL) {
                    config_var = NULL;
                    if (0 == strcmp(stmt->value, "DATE_FORMATTED"))
                        config_var = "DATE";
                    else if (0 == strcmp(stmt->value, "DATE_FIRST_FORMATTED"))
                        config_var = "DATE_FIRST";
                    else if (0 == strcmp(stmt->value, "DATE_LAST_FORMATTED"))
                        config_var = "DATE_LAST";
                    if (config_var != NULL) {
                        config_value2 = blogc_format_date(
                            blogc_get_variable(config_var, config,
                                inside_block ? tmp_source : NULL),
                            config, inside_block ? tmp_source : NULL);
                        if (config_value2 != NULL) {
                            b_string_append(str, config_value2);
                            free(config_value2);
                            config_value2 = NULL;
                            break;
                        }
                    }
                    else {
                        config_value = blogc_get_variable(stmt->value, config,
                            inside_block ? tmp_source : NULL);
                        if (config_value != NULL)
                            b_string_append(str, config_value);
                    }
                }
                break;

            case BLOGC_TEMPLATE_ENDBLOCK_STMT:
                inside_block = false;
                if (listing_start != NULL && current_source != NULL) {
                    current_source = current_source->next;
                    if (current_source != NULL) {
                        tmp = listing_start;
                        continue;
                    }
                    else
                        listing_start = NULL;
                }
                break;

            case BLOGC_TEMPLATE_IFNDEF_STMT:
                if_not = true;

            case BLOGC_TEMPLATE_IFDEF_STMT:
                defined = false;
                if (stmt->value != NULL) {
                    config_var = NULL;
                    if (0 == strcmp(stmt->value, "DATE_FORMATTED"))
                        config_var = "DATE";
                    else if (0 == strcmp(stmt->value, "DATE_FIRST_FORMATTED"))
                        config_var = "DATE_FIRST";
                    else if (0 == strcmp(stmt->value, "DATE_LAST_FORMATTED"))
                        config_var = "DATE_LAST";
                    if (config_var != NULL) {
                        config_value2 = blogc_format_date(
                            blogc_get_variable(config_var, config,
                                inside_block ? tmp_source : NULL),
                            config, inside_block ? tmp_source : NULL);
                        if (config_value2 != NULL) {
                            defined = true;
                            free(config_value2);
                            config_value2 = NULL;
                        }
                    }
                    else
                        defined = blogc_get_variable(stmt->value, config,
                            inside_block ? tmp_source : NULL) != NULL;
                }
                if ((!if_not && !defined) || (if_not && defined)) {
                    if_skip = if_count;

                    // at this point we can just skip anything, counting the
                    // number of 'if's, to know how many 'endif's we need to
                    // skip as well.
                    while (1) {
                        tmp = tmp->next;
                        stmt = tmp->data;
                        if ((stmt->type == BLOGC_TEMPLATE_IFDEF_STMT) ||
                            (stmt->type == BLOGC_TEMPLATE_IFNDEF_STMT))
                        {
                            if_count++;
                            continue;
                        }
                        if (stmt->type == BLOGC_TEMPLATE_ENDIF_STMT) {
                            if (if_count > if_skip) {
                                if_count--;
                                continue;
                            }
                            if (if_count == if_skip)
                                break;
                        }
                    }
                }
                if_not = false;
                break;

            case BLOGC_TEMPLATE_ENDIF_STMT:
                if_count--;
                break;
        }
        tmp = tmp->next;
    }

    return b_string_free(str, false);
}
