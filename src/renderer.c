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

#include <stdio.h>
#include <string.h>
#include "utils/utils.h"
#include "datetime-parser.h"
#include "error.h"
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

    blogc_error_t *err = NULL;
    char *rv = blogc_convert_datetime(date, date_format, &err);
    if (err != NULL) {
        blogc_error_print(err);
        blogc_error_free(err);
        return b_strdup(date);
    }
    return rv;
}


char*
blogc_render(b_slist_t *tmpl, b_slist_t *sources, b_trie_t *config, bool listing)
{
    if (tmpl == NULL)
        return NULL;

    b_slist_t *current_source = NULL;
    b_slist_t *listing_start = NULL;

    b_string_t *str = b_string_new();

    b_trie_t *tmp_source = NULL;
    const char *config_value = NULL;
    const char *config_var = NULL;
    char *config_value2 = NULL;
    char *defined = NULL;

    unsigned int if_count = 0;
    unsigned int if_skip = 0;

    bool if_not = false;
    bool inside_block = false;
    bool evaluate = false;

    int cmp = 0;

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
                    if (sources == NULL) {

                        // we can just skip anything and walk until the next
                        // 'endblock'
                        while (stmt->type != BLOGC_TEMPLATE_ENDBLOCK_STMT) {
                            tmp = tmp->next;
                            stmt = tmp->data;
                        }
                        break;
                    }
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

            case BLOGC_TEMPLATE_IF_STMT:
            case BLOGC_TEMPLATE_IFDEF_STMT:
                defined = NULL;
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
                            defined = config_value2;
                            config_value2 = NULL;
                        }
                    }
                    else
                        defined = b_strdup(blogc_get_variable(stmt->value,
                            config, inside_block ? tmp_source : NULL));
                }
                evaluate = false;
                if (stmt->op != 0) {
                    // Strings that start with a '"' are actually strings, the
                    // others are meant to be looked up as a second variable
                    // check.
                    char *defined2 = NULL;
                    if (stmt->value2[0] != '"') {
                        defined2 =
                            b_strdup(blogc_get_variable(stmt->value2,
                                                        config,
                                                        inside_block ? tmp_source
                                                                     : NULL)
                            );
                    } else {
                        defined2 = b_strndup(stmt->value2 + 1,
                                             strlen(stmt->value2) - 2);
                    }

                    if (defined != NULL && defined2 != NULL) {
                        cmp = strcmp(defined, defined2);
                        if (cmp != 0 && stmt->op & BLOGC_TEMPLATE_OP_NEQ)
                            evaluate = true;
                        else if (cmp == 0 && stmt->op & BLOGC_TEMPLATE_OP_EQ)
                            evaluate = true;
                        else if (cmp < 0 && stmt->op & BLOGC_TEMPLATE_OP_LT)
                            evaluate = true;
                        else if (cmp > 0 && stmt->op & BLOGC_TEMPLATE_OP_GT)
                            evaluate = true;
                    }

                    free(defined2);
                }
                else {
                    if (if_not && defined == NULL)
                        evaluate = true;
                    if (!if_not && defined != NULL)
                        evaluate = true;
                }
                if (!evaluate) {
                    if_skip = if_count;

                    // at this point we can just skip anything, counting the
                    // number of 'if's, to know how many 'endif's we need to
                    // skip as well.
                    while (1) {
                        tmp = tmp->next;
                        stmt = tmp->data;
                        if ((stmt->type == BLOGC_TEMPLATE_IF_STMT) ||
                            (stmt->type == BLOGC_TEMPLATE_IFDEF_STMT) ||
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
                free(defined);
                defined = NULL;
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
