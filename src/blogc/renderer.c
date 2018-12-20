/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2018 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "datetime-parser.h"
#include "template-parser.h"
#include "renderer.h"
#include "../common/error.h"
#include "../common/utils.h"


const char*
blogc_get_variable(const char *name, bc_trie_t *global, bc_trie_t *local)
{
    const char *rv = NULL;
    if (local != NULL) {
        rv = bc_trie_lookup(local, name);
        if (rv != NULL)
            return rv;
    }
    if (global != NULL)
        rv = bc_trie_lookup(global, name);
    return rv;
}


char*
blogc_format_date(const char *date, bc_trie_t *global, bc_trie_t *local)
{
    const char *date_format = blogc_get_variable("DATE_FORMAT", global, local);
    if (date == NULL)
        return NULL;
    if (date_format == NULL)
        return bc_strdup(date);

    bc_error_t *err = NULL;
    char *rv = blogc_convert_datetime(date, date_format, &err);
    if (err != NULL) {
        bc_error_print(err, "blogc");
        bc_error_free(err);
        return bc_strdup(date);
    }
    return rv;
}


char*
blogc_format_variable(const char *name, bc_trie_t *global, bc_trie_t *local,
    bc_slist_t *foreach_var)
{
    // if used asked for a variable that exists, just return it right away
    const char *value = blogc_get_variable(name, global, local);
    if (value != NULL)
        return bc_strdup(value);

    // do the same for special variable 'FOREACH_ITEM'
    if (0 == strcmp(name, "FOREACH_ITEM")) {
        if (foreach_var != NULL && foreach_var->data != NULL) {
            return bc_strdup(foreach_var->data);
        }
        return NULL;
    }

    char *var = bc_strdup(name);

    size_t i;
    size_t last = strlen(var);

    long int len = -1;

    // just walk till the last '_'
    for (i = last - 1; i > 0 && var[i] >= '0' && var[i] <= '9'; i--);

    if (var[i] == '_' && (i + 1) < last) {  // var ends with '_[0-9]+'
        char *endptr;
        len = strtol(var + i + 1, &endptr, 10);
        if (*endptr != '\0') {
            fprintf(stderr, "warning: invalid variable size for '%s', "
                "ignoring.\n", var);
            len = -1;
        }
        else {
            var[i] = '\0';
        }
    }

    bool must_format = false;

    if (bc_str_ends_with(var, "_FORMATTED")) {
        var[strlen(var) - 10] = '\0';
        must_format = true;
    }

    if ((0 == strcmp(var, "FOREACH_ITEM")) &&
        (foreach_var != NULL && foreach_var->data != NULL))
        value = foreach_var->data;
    else
        value = blogc_get_variable(var, global, local);

    if (value == NULL) {
        free(var);
        return NULL;
    }

    char *rv = NULL;

    if (must_format) {
        if (bc_str_starts_with(name, "DATE_")) {
            rv = blogc_format_date(value, global, local);
        }
        else {
            fprintf(stderr, "warning: no formatter found for '%s', "
                "ignoring.\n", var);
            rv = bc_strdup(value);
        }
    }
    else {
        rv = bc_strdup(value);
    }

    free(var);

    if (len > 0) {
        char *tmp = bc_strndup(rv, len);
        free(rv);
        rv = tmp;
    }

    return rv;
}


bc_slist_t*
blogc_split_list_variable(const char *name, bc_trie_t *global, bc_trie_t *local)
{
    const char *value = blogc_get_variable(name, global, local);
    if (value == NULL)
        return NULL;

    bc_slist_t *rv = NULL;

    char **tmp = bc_str_split(value, ' ', 0);
    for (size_t i = 0; tmp[i] != NULL; i++) {
        if (tmp[i][0] != '\0')  // ignore empty strings
            rv = bc_slist_append(rv, tmp[i]);
        else
            free(tmp[i]);
    }
    free(tmp);

    return rv;
}


char*
blogc_render(bc_slist_t *tmpl, bc_slist_t *sources, bc_trie_t *config, bool listing)
{
    if (tmpl == NULL)
        return NULL;

    bc_slist_t *current_source = NULL;
    bc_slist_t *listing_start = NULL;

    bc_string_t *str = bc_string_new();

    bc_trie_t *tmp_source = NULL;
    char *config_value = NULL;
    char *defined = NULL;

    size_t if_count = 0;

    bc_slist_t *foreach_var = NULL;
    bc_slist_t *foreach_var_start = NULL;
    bc_slist_t *foreach_start = NULL;

    bool if_not = false;
    bool inside_block = false;
    bool evaluate = false;
    bool valid_else = false;

    int cmp = 0;

    bc_slist_t *tmp = tmpl;
    while (tmp != NULL) {
        blogc_template_node_t *node = tmp->data;

        switch (node->type) {

            case BLOGC_TEMPLATE_NODE_CONTENT:
                if (node->data[0] != NULL)
                    bc_string_append(str, node->data[0]);
                break;

            case BLOGC_TEMPLATE_NODE_BLOCK:
                inside_block = true;
                if_count = 0;
                if (0 == strcmp("entry", node->data[0])) {
                    if (listing) {

                        // we can just skip anything and walk until the next
                        // 'endblock'
                        while (node->type != BLOGC_TEMPLATE_NODE_ENDBLOCK) {
                            tmp = tmp->next;
                            node = tmp->data;
                        }
                        break;
                    }
                    current_source = sources;
                    tmp_source = current_source != NULL ? current_source->data : NULL;
                }
                else if ((0 == strcmp("listing", node->data[0])) ||
                         (0 == strcmp("listing_once", node->data[0]))) {
                    if (!listing) {

                        // we can just skip anything and walk until the next
                        // 'endblock'
                        while (node->type != BLOGC_TEMPLATE_NODE_ENDBLOCK) {
                            tmp = tmp->next;
                            node = tmp->data;
                        }
                        break;
                    }
                }
                if (0 == strcmp("listing", node->data[0])) {
                    if (sources == NULL) {

                        // we can just skip anything and walk until the next
                        // 'endblock'
                        while (node->type != BLOGC_TEMPLATE_NODE_ENDBLOCK) {
                            tmp = tmp->next;
                            node = tmp->data;
                        }
                        break;
                    }
                    if (current_source == NULL) {
                        listing_start = tmp;
                        current_source = sources;
                    }
                    tmp_source = current_source != NULL ? current_source->data : NULL;
                }
                break;

            case BLOGC_TEMPLATE_NODE_VARIABLE:
                if (node->data[0] != NULL) {
                    config_value = blogc_format_variable(node->data[0],
                        config, inside_block ? tmp_source : NULL, foreach_var);
                    if (config_value != NULL) {
                        bc_string_append(str, config_value);
                        free(config_value);
                        config_value = NULL;
                        break;
                    }
                }
                break;

            case BLOGC_TEMPLATE_NODE_ENDBLOCK:
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

            case BLOGC_TEMPLATE_NODE_IFNDEF:
                if_not = true;

            case BLOGC_TEMPLATE_NODE_IF:
            case BLOGC_TEMPLATE_NODE_IFDEF:
                if_count = 0;
                defined = NULL;
                if (node->data[0] != NULL)
                    defined = blogc_format_variable(node->data[0], config,
                        inside_block ? tmp_source : NULL, foreach_var);
                evaluate = false;
                if (node->op != 0) {
                    // Strings that start with a '"' are actually strings, the
                    // others are meant to be looked up as a second variable
                    // check.
                    char *defined2 = NULL;
                    if (node->data[1] != NULL) {
                        if ((strlen(node->data[1]) >= 2) &&
                            (node->data[1][0] == '"') &&
                            (node->data[1][strlen(node->data[1]) - 1] == '"'))
                        {
                            defined2 = bc_strndup(node->data[1] + 1,
                                strlen(node->data[1]) - 2);
                        }
                        else {
                            defined2 = blogc_format_variable(node->data[1],
                                config, inside_block ? tmp_source : NULL,
                                foreach_var);
                        }
                    }

                    if (defined != NULL && defined2 != NULL) {
                        cmp = strcmp(defined, defined2);
                        if (cmp != 0 && node->op & BLOGC_TEMPLATE_OP_NEQ)
                            evaluate = true;
                        else if (cmp == 0 && node->op & BLOGC_TEMPLATE_OP_EQ)
                            evaluate = true;
                        else if (cmp < 0 && node->op & BLOGC_TEMPLATE_OP_LT)
                            evaluate = true;
                        else if (cmp > 0 && node->op & BLOGC_TEMPLATE_OP_GT)
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
                        node = tmp->data;
                        if ((node->type == BLOGC_TEMPLATE_NODE_IF) ||
                            (node->type == BLOGC_TEMPLATE_NODE_IFDEF) ||
                            (node->type == BLOGC_TEMPLATE_NODE_IFNDEF))
                        {
                            if_count++;
                            continue;
                        }
                        if ((node->type == BLOGC_TEMPLATE_NODE_ELSE) &&
                            (if_count == 0))
                        {
                            // this is somewhat complex. only an else statement
                            // right after a non evaluated block should be considered
                            // valid, because all the inner conditionals were just
                            // skipped, and all the outter conditionals evaluated
                            // to true.
                            valid_else = true;
                            break;
                        }
                        if (node->type == BLOGC_TEMPLATE_NODE_ENDIF) {
                            if (if_count > 0) {
                                if_count--;
                                continue;
                            }
                            break;
                        }
                    }
                }
                else {
                    valid_else = false;
                }
                free(defined);
                defined = NULL;
                if_not = false;
                break;

            case BLOGC_TEMPLATE_NODE_ELSE:
                if_count = 0;
                if (!valid_else) {

                    // at this point we can just skip anything, counting the
                    // number of 'if's, to know how many 'endif's we need to
                    // skip as well.
                    while (1) {
                        tmp = tmp->next;
                        node = tmp->data;
                        if ((node->type == BLOGC_TEMPLATE_NODE_IF) ||
                            (node->type == BLOGC_TEMPLATE_NODE_IFDEF) ||
                            (node->type == BLOGC_TEMPLATE_NODE_IFNDEF))
                        {
                            if_count++;
                            continue;
                        }
                        // no need to handle else statements here, because every
                        // if should have an endif.
                        if (node->type == BLOGC_TEMPLATE_NODE_ENDIF) {
                            if (if_count > 0) {
                                if_count--;
                                continue;
                            }
                            break;
                        }
                    }
                }
                valid_else = false;
                break;

            case BLOGC_TEMPLATE_NODE_ENDIF:
                // any endif statement should invalidate valid_else, to avoid
                // propagation to outter conditionals.
                valid_else = false;
                if (if_count > 0)
                    if_count--;
                break;

            case BLOGC_TEMPLATE_NODE_FOREACH:
                if (foreach_var_start == NULL) {
                    if (node->data[0] != NULL)
                        foreach_var_start = blogc_split_list_variable(node->data[0],
                            config, inside_block ? tmp_source : NULL);

                    if (foreach_var_start != NULL) {
                        foreach_var = foreach_var_start;
                        foreach_start = tmp;
                    }
                    else {

                        // we can just skip anything and walk until the next
                        // 'endforeach'
                        while (node->type != BLOGC_TEMPLATE_NODE_ENDFOREACH) {
                            tmp = tmp->next;
                            node = tmp->data;
                        }
                        break;
                    }
                }

                if (foreach_var == NULL) {
                    foreach_start = tmp;
                    foreach_var = foreach_var_start;
                }
                break;

            case BLOGC_TEMPLATE_NODE_ENDFOREACH:
                if (foreach_start != NULL && foreach_var != NULL) {
                    foreach_var = foreach_var->next;
                    if (foreach_var != NULL) {
                        tmp = foreach_start;
                        continue;
                    }
                }
                foreach_start = NULL;
                bc_slist_free_full(foreach_var_start, free);
                foreach_var_start = NULL;
                break;
        }
        tmp = tmp->next;
    }

    // no need to free temporary variables here. the template parser makes sure
    // that templates are sane and statements are closed.

    return bc_string_free(str, false);
}
