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

#include <stdbool.h>
#include <string.h>

#include "utils/utils.h"
#include "template-parser.h"
#include "output.h"


typedef enum {
    TEMPLATE_START = 1,
    TEMPLATE_OPEN_BRACKET,
    TEMPLATE_BLOCK_START,
    TEMPLATE_BLOCK_TYPE,
    TEMPLATE_BLOCK_BLOCK_TYPE_START,
    TEMPLATE_BLOCK_BLOCK_TYPE,
    TEMPLATE_BLOCK_IF_VARIABLE_START,
    TEMPLATE_BLOCK_IF_VARIABLE,
    TEMPLATE_BLOCK_END,
    TEMPLATE_VARIABLE_START,
    TEMPLATE_VARIABLE,
    TEMPLATE_VARIABLE_END,
    TEMPLATE_CLOSE_BRACKET,
} blogc_template_parser_state_t;


b_slist_t*
blogc_template_parse(const char *src, size_t src_len)
{
    size_t current = 0;
    size_t start = 0;
    size_t end = 0;

    bool error = false;
    char *tmp = NULL;

    bool open_block = false;
    unsigned int if_count = 0;

    b_slist_t *stmts = NULL;
    blogc_template_stmt_t *stmt = NULL;

    blogc_template_parser_state_t state = TEMPLATE_START;
    blogc_template_stmt_type_t type = BLOGC_TEMPLATE_CONTENT_STMT;

    while (current < src_len) {
        char c = src[current];
        bool last = current == src_len - 1;

        switch (state) {

            case TEMPLATE_START:
                if (last) {
                    stmt = malloc(sizeof(blogc_template_stmt_t));
                    stmt->type = type;
                    stmt->value = b_strndup(src + start, src_len - start);
                    stmts = b_slist_append(stmts, stmt);
                    stmt = NULL;
                }
                if (c == '{') {
                    end = current;
                    state = TEMPLATE_OPEN_BRACKET;
                }
                break;

            case TEMPLATE_OPEN_BRACKET:
                if (c == '%' || c == '{') {
                    if (c == '%')
                        state = TEMPLATE_BLOCK_START;
                    else
                        state = TEMPLATE_VARIABLE_START;
                    if (end > start) {
                        stmt = malloc(sizeof(blogc_template_stmt_t));
                        stmt->type = type;
                        stmt->value = b_strndup(src + start, end - start);
                        stmts = b_slist_append(stmts, stmt);
                        stmt = NULL;
                    }
                }
                break;

            case TEMPLATE_BLOCK_START:
                if (c == ' ')
                    break;
                if (c >= 'a' && c <= 'z') {
                    state = TEMPLATE_BLOCK_TYPE;
                    start = current;
                    break;
                }
                error = true;
                break;

            case TEMPLATE_BLOCK_TYPE:
                if (c >= 'a' && c <= 'z')
                    break;
                if (c == ' ') {
                    if (0 == strncmp("block", src + start, current - start)) {
                        if (!open_block) {
                            state = TEMPLATE_BLOCK_BLOCK_TYPE_START;
                            type = BLOGC_TEMPLATE_BLOCK_STMT;
                            start = current;
                            open_block = true;
                            break;
                        }
                    }
                    else if (0 == strncmp("endblock", src + start, current - start)) {
                        if (open_block) {
                            state = TEMPLATE_BLOCK_END;
                            type = BLOGC_TEMPLATE_ENDBLOCK_STMT;
                            open_block = false;
                            break;
                        }
                    }
                    else if (0 == strncmp("if", src + start, current - start)) {
                        if (open_block) {
                            state = TEMPLATE_BLOCK_IF_VARIABLE_START;
                            type = BLOGC_TEMPLATE_IF_STMT;
                            start = current;
                            if_count++;
                            break;
                        }
                    }
                    else if (0 == strncmp("endif", src + start, current - start)) {
                        if (open_block) {
                            if (if_count > 0) {
                                state = TEMPLATE_BLOCK_END;
                                type = BLOGC_TEMPLATE_ENDIF_STMT;
                                if_count--;
                                break;
                            }
                        }
                    }
                }
                error = true;
                break;

            case TEMPLATE_BLOCK_BLOCK_TYPE_START:
                if (c == ' ')
                    break;
                if (c >= 'a' && c <= 'z') {
                    state = TEMPLATE_BLOCK_BLOCK_TYPE;
                    start = current;
                    break;
                }
                error = true;
                break;

            case TEMPLATE_BLOCK_BLOCK_TYPE:
                if ((c >= 'a' && c <= 'z') || c == '_')
                    break;
                if (c == ' ') {
                    if ((0 == strncmp("single_source", src + start, current - start)) ||
                        (0 == strncmp("multiple_sources", src + start, current - start)) ||
                        (0 == strncmp("multiple_sources_once", src + start, current - start)))
                    {
                        end = current;
                        state = TEMPLATE_BLOCK_END;
                        break;
                    }
                }
                error = true;
                break;

            case TEMPLATE_BLOCK_IF_VARIABLE_START:
                if (c == ' ')
                    break;
                if (c >= 'A' && c <= 'Z') {
                    state = TEMPLATE_BLOCK_IF_VARIABLE;
                    start = current;
                    break;
                }
                error = true;
                break;

            case TEMPLATE_BLOCK_IF_VARIABLE:
                if ((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_')
                    break;
                if (c == ' ') {
                    end = current;
                    state = TEMPLATE_BLOCK_END;
                    break;
                }
                error = true;
                break;

            case TEMPLATE_BLOCK_END:
                if (c == ' ')
                    break;
                if (c == '%') {
                    state = TEMPLATE_CLOSE_BRACKET;
                    break;
                }
                error = true;
                break;

            case TEMPLATE_VARIABLE_START:
                if (c == ' ')
                    break;
                if (c >= 'A' && c <= 'Z') {
                    if (open_block) {
                        state = TEMPLATE_VARIABLE;
                        type = BLOGC_TEMPLATE_VARIABLE_STMT;
                        start = current;
                        break;
                    }
                }
                error = true;
                break;

            case TEMPLATE_VARIABLE:
                if ((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_')
                    break;
                if (c == ' ') {
                    end = current;
                    state = TEMPLATE_VARIABLE_END;
                    break;
                }
                if (c == '}') {
                    end = current;
                    state = TEMPLATE_CLOSE_BRACKET;
                    break;
                }
                error = true;
                break;

            case TEMPLATE_VARIABLE_END:
                if (c == ' ')
                    break;
                if (c == '}') {
                    state = TEMPLATE_CLOSE_BRACKET;
                    break;
                }
                error = true;
                break;

            case TEMPLATE_CLOSE_BRACKET:
                if (c == '}') {
                    stmt = malloc(sizeof(blogc_template_stmt_t));
                    stmt->type = type;
                    stmt->value = NULL;
                    if (end > start)
                        stmt->value = b_strndup(src + start, end - start);
                    stmts = b_slist_append(stmts, stmt);
                    stmt = NULL;
                    state = TEMPLATE_START;
                    type = BLOGC_TEMPLATE_CONTENT_STMT;
                    start = current + 1;
                    break;
                }
                error = true;
                break;

        }

        if (error)
            break;

        current++;
    }

    if (error) {
        if (stmt != NULL) {
            free(stmt->value);
            free(stmt);
        }
        blogc_template_free_stmts(stmts);
        blogc_parser_syntax_error("template", src, src_len, current);
        return NULL;
    }

    return stmts;
}


void
blogc_template_free_stmts(b_slist_t *stmts)
{
    for (b_slist_t *tmp = stmts; tmp != NULL; tmp = tmp->next) {
        blogc_template_stmt_t *data = tmp->data;
        free(data->value);
        free(data);
    }
    b_slist_free(stmts);
}
