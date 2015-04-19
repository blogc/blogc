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

#include <string.h>
#include "utils/utils.h"
#include "source-parser.h"
#include "template-parser.h"
#include "renderer.h"


char*
blogc_render(b_slist_t *tmpl, b_slist_t *sources)
{
    if (tmpl == NULL || sources == NULL)
        return NULL;

    bool single_source = sources->next == NULL;

    b_slist_t *current_source = NULL;
    b_slist_t *multiple_sources_start = NULL;

    b_string_t *str = b_string_new();

    blogc_source_t *tmp_source = NULL;
    char *config_value = NULL;

    unsigned int if_count = 0;
    unsigned int if_skip = 0;

    b_slist_t *tmp = tmpl;
    while (tmp != NULL) {
        blogc_template_stmt_t *stmt = tmp->data;

        switch (stmt->type) {

            case BLOGC_TEMPLATE_CONTENT_STMT:
                if (stmt->value != NULL)
                    b_string_append(str, stmt->value);
                break;

            case BLOGC_TEMPLATE_BLOCK_STMT:
                if_count = 0;
                if (0 == strcmp("single_source", stmt->value)) {
                    if (!single_source) {

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
                else if ((0 == strcmp("multiple_sources", stmt->value)) ||
                         (0 == strcmp("multiple_sources_once", stmt->value))) {
                    if (single_source) {

                        // we can just skip anything and walk until the next
                        // 'endblock'
                        while (stmt->type != BLOGC_TEMPLATE_ENDBLOCK_STMT) {
                            tmp = tmp->next;
                            stmt = tmp->data;
                        }
                        break;
                    }
                }
                if (0 == strcmp("multiple_sources", stmt->value)) {
                    if (current_source == NULL) {
                        multiple_sources_start = tmp;
                        current_source = sources;
                    }
                    tmp_source = current_source->data;
                }
                break;

            case BLOGC_TEMPLATE_VARIABLE_STMT:
                if (stmt->value != NULL && tmp_source != NULL) {
                    config_value = b_trie_lookup(tmp_source->config, stmt->value);
                    if (config_value != NULL)
                        b_string_append(str, config_value);
                    break;
                }
                break;

            case BLOGC_TEMPLATE_ENDBLOCK_STMT:
                if (multiple_sources_start != NULL && current_source != NULL) {
                    current_source = current_source->next;
                    if (current_source != NULL) {
                        tmp = multiple_sources_start;
                        continue;
                    }
                    else
                        multiple_sources_start = NULL;
                }
                break;

            case BLOGC_TEMPLATE_IF_STMT:
                if (stmt->value != NULL && tmp_source != NULL) {
                    if (b_trie_lookup(tmp_source->config, stmt->value) == NULL) {
                        if_skip = if_count;

                        // at this point we can just skip anything, counting the
                        // number of 'if's, to know how many 'endif's we need to
                        // skip as well.
                        while (1) {
                            tmp = tmp->next;
                            stmt = tmp->data;
                            if (stmt->type == BLOGC_TEMPLATE_IF_STMT) {
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
                }
                break;

            case BLOGC_TEMPLATE_ENDIF_STMT:
                if_count--;
                break;
        }
        tmp = tmp->next;
    }

    return b_string_free(str, false);
}
