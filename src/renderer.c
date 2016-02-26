/*
 * blogc: A blog compiler.
 * Copyright (C) 2015-2016 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <squareball.h>
#include "datetime-parser.h"
#include "error.h"
#include "loader.h"
#include "source-parser.h"
#include "template-parser.h"
#include "renderer.h"


const char*
blogc_get_variable(const char *name, sb_trie_t *global, sb_trie_t *local)
{
    const char *rv = NULL;
    if (local != NULL) {
        rv = sb_trie_lookup(local, name);
        if (rv != NULL)
            return rv;
    }
    if (global != NULL)
        rv = sb_trie_lookup(global, name);
    return rv;
}


char*
blogc_format_date(const char *date, sb_trie_t *global, sb_trie_t *local)
{
    const char *date_format = blogc_get_variable("DATE_FORMAT", global, local);
    if (date == NULL)
        return NULL;
    if (date_format == NULL)
        return sb_strdup(date);

    blogc_error_t *err = NULL;
    char *rv = blogc_convert_datetime(date, date_format, &err);
    if (err != NULL) {
        blogc_error_print(err);
        blogc_error_free(err);
        return sb_strdup(date);
    }
    return rv;
}


char*
blogc_format_variable(const char *name, sb_trie_t *global, sb_trie_t *local,
    sb_slist_t *foreach_var)
{
    if (0 == strcmp(name, "FOREACH_ITEM")) {
        if (foreach_var != NULL && foreach_var->data != NULL) {
            return sb_strdup(foreach_var->data);
        }
        return NULL;
    }

    char *var = NULL;
    bool must_format = false;
    if (sb_str_ends_with(name, "_FORMATTED")) {
        var = sb_strndup(name, strlen(name) - 10);
        must_format = true;
    }
    if (var == NULL)
        var = sb_strdup(name);

    const char *value = blogc_get_variable(var, global, local);
    free(var);

    if (value == NULL)
        return NULL;

    char *rv = NULL;
    if (must_format) {
        if (sb_str_starts_with(name, "DATE_")) {
            rv = blogc_format_date(value, global, local);
        }
    }

    if (rv == NULL)
        return sb_strdup(value);
    return rv;
}


sb_slist_t*
blogc_split_list_variable(const char *name, sb_trie_t *global, sb_trie_t *local)
{
    const char *value = blogc_get_variable(name, global, local);
    if (value == NULL)
        return NULL;

    sb_slist_t *rv = NULL;

    char **tmp = sb_str_split(value, ' ', 0);
    for (unsigned int i = 0; tmp[i] != NULL; i++) {
        if (tmp[i][0] != '\0')  // ignore empty strings
            rv = sb_slist_append(rv, tmp[i]);
        else
            free(tmp[i]);
    }
    free(tmp);

    return rv;
}


char*
blogc_render(sb_slist_t *tmpl, sb_slist_t *sources, sb_trie_t *config, bool listing)
{
    if (tmpl == NULL)
        return NULL;

    sb_slist_t *current_source = NULL;
    sb_slist_t *listing_start = NULL;

    sb_string_t *str = sb_string_new();

    sb_trie_t *tmp_source = NULL;
    char *config_value = NULL;
    char *defined = NULL;

    unsigned int if_count = 0;

    sb_slist_t *foreach_var = NULL;
    sb_slist_t *foreach_var_start = NULL;
    sb_slist_t *foreach_start = NULL;

    bool if_not = false;
    bool inside_block = false;
    bool evaluate = false;

    int cmp = 0;

    sb_slist_t *tmp = tmpl;
    while (tmp != NULL) {
        blogc_template_stmt_t *stmt = tmp->data;

        switch (stmt->type) {

            case BLOGC_TEMPLATE_CONTENT_STMT:
                if (stmt->value != NULL)
                    sb_string_append(str, stmt->value);
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
                    config_value = blogc_format_variable(stmt->value,
                        config, inside_block ? tmp_source : NULL, foreach_var);
                    if (config_value != NULL) {
                        sb_string_append(str, config_value);
                        free(config_value);
                        config_value = NULL;
                        break;
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
                if_count = 0;
                defined = NULL;
                if (stmt->value != NULL)
                    defined = blogc_format_variable(stmt->value, config,
                        inside_block ? tmp_source : NULL, foreach_var);
                evaluate = false;
                if (stmt->op != 0) {
                    // Strings that start with a '"' are actually strings, the
                    // others are meant to be looked up as a second variable
                    // check.
                    char *defined2 = NULL;
                    if (stmt->value2 != NULL) {
                        if ((strlen(stmt->value2) >= 2) &&
                            (stmt->value2[0] == '"') &&
                            (stmt->value2[strlen(stmt->value2) - 1] == '"'))
                        {
                            defined2 = sb_strndup(stmt->value2 + 1,
                                strlen(stmt->value2) - 2);
                        }
                        else {
                            defined2 = blogc_format_variable(stmt->value2,
                                config, inside_block ? tmp_source : NULL,
                                foreach_var);
                        }
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
                            if (if_count > 0) {
                                if_count--;
                                continue;
                            }
                            if (if_count == 0)
                                break;
                        }
                    }
                }
                free(defined);
                defined = NULL;
                if_not = false;
                break;

            case BLOGC_TEMPLATE_ENDIF_STMT:
                if (if_count > 0)
                    if_count--;
                break;

            case BLOGC_TEMPLATE_FOREACH_STMT:
                if (foreach_var_start == NULL) {
                    if (stmt->value != NULL)
                        foreach_var_start = blogc_split_list_variable(stmt->value,
                            config, inside_block ? tmp_source : NULL);

                    if (foreach_var_start != NULL) {
                        foreach_var = foreach_var_start;
                        foreach_start = tmp;
                    }
                    else {

                        // we can just skip anything and walk until the next
                        // 'endforeach'
                        while (stmt->type != BLOGC_TEMPLATE_ENDFOREACH_STMT) {
                            tmp = tmp->next;
                            stmt = tmp->data;
                        }
                        break;
                    }
                }

                if (foreach_var == NULL) {
                    foreach_start = tmp;
                    foreach_var = foreach_var_start;
                }
                break;

            case BLOGC_TEMPLATE_ENDFOREACH_STMT:
                if (foreach_start != NULL && foreach_var != NULL) {
                    foreach_var = foreach_var->next;
                    if (foreach_var != NULL) {
                        tmp = foreach_start;
                        continue;
                    }
                }
                foreach_start = NULL;
                sb_slist_free_full(foreach_var_start, free);
                foreach_var_start = NULL;
                break;
        }
        tmp = tmp->next;
    }

    // no need to free temporary variables here. the template parser makes sure
    // that templates are sane and statements are closed.

    return sb_string_free(str, false);
}
