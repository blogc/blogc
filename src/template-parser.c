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
#include "error.h"


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


typedef enum {
    BLOCK_CLOSED = 1,
    BLOCK_SINGLE_SOURCE,
    BLOCK_MULTIPLE_SOURCES,
    BLOCK_MULTIPLE_SOURCES_ONCE,
} blogc_template_parser_block_state_t;


b_slist_t*
blogc_template_parse(const char *src, size_t src_len, blogc_error_t **err)
{
    if (err == NULL || *err != NULL)
        return NULL;

    size_t current = 0;
    size_t start = 0;
    size_t end = 0;

    unsigned int if_count = 0;

    b_slist_t *stmts = NULL;
    blogc_template_stmt_t *stmt = NULL;

    blogc_template_parser_state_t state = TEMPLATE_START;
    blogc_template_parser_block_state_t block_state = BLOCK_CLOSED;
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
                    break;
                }
                *err = blogc_error_parser(BLOGC_ERROR_TEMPLATE_PARSER, src,
                    src_len, current,
                    "Invalid statement syntax. Must be '%%' or '{'.");
                break;

            case TEMPLATE_BLOCK_START:
                if (c == ' ')
                    break;
                if (c >= 'a' && c <= 'z') {
                    state = TEMPLATE_BLOCK_TYPE;
                    start = current;
                    break;
                }
                *err = blogc_error_parser(BLOGC_ERROR_TEMPLATE_PARSER, src,
                    src_len, current,
                    "Invalid statement syntax. Must begin lowercase letter.");
                break;

            case TEMPLATE_BLOCK_TYPE:
                if (c >= 'a' && c <= 'z')
                    break;
                if (c == ' ') {
                    if (0 == strncmp("block", src + start, current - start)) {
                        if (block_state == BLOCK_CLOSED) {
                            state = TEMPLATE_BLOCK_BLOCK_TYPE_START;
                            type = BLOGC_TEMPLATE_BLOCK_STMT;
                            start = current;
                            break;
                        }
                        *err = blogc_error_parser(BLOGC_ERROR_TEMPLATE_PARSER,
                            src, src_len, current, "Blocks can't be nested.");
                        break;
                    }
                    else if (0 == strncmp("endblock", src + start, current - start)) {
                        if (block_state != BLOCK_CLOSED) {
                            state = TEMPLATE_BLOCK_END;
                            type = BLOGC_TEMPLATE_ENDBLOCK_STMT;
                            block_state = BLOCK_CLOSED;
                            break;
                        }
                        *err = blogc_error_parser(BLOGC_ERROR_TEMPLATE_PARSER,
                            src, src_len, current,
                            "'endblock' statement without an open 'block' statement.");
                        break;
                    }
                    else if (0 == strncmp("if", src + start, current - start)) {
                        if (block_state == BLOCK_SINGLE_SOURCE || block_state == BLOCK_MULTIPLE_SOURCES) {
                            state = TEMPLATE_BLOCK_IF_VARIABLE_START;
                            type = BLOGC_TEMPLATE_IF_STMT;
                            start = current;
                            if_count++;
                            break;
                        }
                        *err = blogc_error_parser(BLOGC_ERROR_TEMPLATE_PARSER,
                            src, src_len, current,
                            "'if' statements only allowed inside 'single_source' "
                            "and 'multiple_sources' blocks.");
                        break;
                    }
                    else if (0 == strncmp("endif", src + start, current - start)) {
                        if (block_state == BLOCK_SINGLE_SOURCE || block_state == BLOCK_MULTIPLE_SOURCES) {
                            if (if_count > 0) {
                                state = TEMPLATE_BLOCK_END;
                                type = BLOGC_TEMPLATE_ENDIF_STMT;
                                if_count--;
                                break;
                            }
                            *err = blogc_error_parser(BLOGC_ERROR_TEMPLATE_PARSER,
                                src, src_len, current,
                                "'endif' statement without an open 'if' statement.");
                            break;
                        }
                        *err = blogc_error_parser(BLOGC_ERROR_TEMPLATE_PARSER,
                            src, src_len, current,
                            "'endif' statements only allowed inside 'single_source' "
                            "and 'multiple_sources' blocks.");
                        break;
                    }
                }
                *err = blogc_error_parser(BLOGC_ERROR_TEMPLATE_PARSER, src,
                    src_len, current,
                    "Invalid statement type: Allowed types are: block, endblock, if, endif.");
                break;

            case TEMPLATE_BLOCK_BLOCK_TYPE_START:
                if (c == ' ')
                    break;
                if (c >= 'a' && c <= 'z') {
                    state = TEMPLATE_BLOCK_BLOCK_TYPE;
                    start = current;
                    break;
                }
                *err = blogc_error_parser(BLOGC_ERROR_TEMPLATE_PARSER, src,
                    src_len, current,
                    "Invalid block syntax. Must begin with lowercase letter.");
                break;

            case TEMPLATE_BLOCK_BLOCK_TYPE:
                if ((c >= 'a' && c <= 'z') || c == '_')
                    break;
                if (c == ' ') {
                    if (0 == strncmp("single_source", src + start, current - start)) {
                        block_state = BLOCK_SINGLE_SOURCE;
                        end = current;
                        state = TEMPLATE_BLOCK_END;
                        break;
                    }
                    else if (0 == strncmp("multiple_sources", src + start, current - start)) {
                        block_state = BLOCK_MULTIPLE_SOURCES;
                        end = current;
                        state = TEMPLATE_BLOCK_END;
                        break;
                    }
                    else if (0 == strncmp("multiple_sources_once", src + start, current - start)) {
                        block_state = BLOCK_MULTIPLE_SOURCES_ONCE;
                        end = current;
                        state = TEMPLATE_BLOCK_END;
                        break;
                    }
                }
                *err = blogc_error_parser(BLOGC_ERROR_TEMPLATE_PARSER, src,
                    src_len, current,
                    "Invalid block type. Allowed types are: single_source, multiple_sources, multiple_sources_once.");
                break;

            case TEMPLATE_BLOCK_IF_VARIABLE_START:
                if (c == ' ')
                    break;
                if (c >= 'A' && c <= 'Z') {
                    state = TEMPLATE_BLOCK_IF_VARIABLE;
                    start = current;
                    break;
                }
                *err = blogc_error_parser(BLOGC_ERROR_TEMPLATE_PARSER, src,
                    src_len, current,
                    "Invalid variable name. Must begin with uppercase letter.");
                break;

            case TEMPLATE_BLOCK_IF_VARIABLE:
                if ((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_')
                    break;
                if (c == ' ') {
                    end = current;
                    state = TEMPLATE_BLOCK_END;
                    break;
                }
                *err = blogc_error_parser(BLOGC_ERROR_TEMPLATE_PARSER, src,
                    src_len, current,
                    "Invalid variable name. Must be uppercase letter, number or '_'.");
                break;

            case TEMPLATE_BLOCK_END:
                if (c == ' ')
                    break;
                if (c == '%') {
                    state = TEMPLATE_CLOSE_BRACKET;
                    break;
                }
                *err = blogc_error_parser(BLOGC_ERROR_TEMPLATE_PARSER, src,
                    src_len, current,
                    "Invalid statement syntax. Must end with '%}'.");
                break;

            case TEMPLATE_VARIABLE_START:
                if (c == ' ')
                    break;
                if (c >= 'A' && c <= 'Z') {
                    if (block_state == BLOCK_SINGLE_SOURCE || block_state == BLOCK_MULTIPLE_SOURCES) {
                        state = TEMPLATE_VARIABLE;
                        type = BLOGC_TEMPLATE_VARIABLE_STMT;
                        start = current;
                        break;
                    }
                    *err = blogc_error_parser(BLOGC_ERROR_TEMPLATE_PARSER,
                        src, src_len, current,
                        "variable statements only allowed inside 'single_source' "
                        "and 'multiple_sources' blocks.");
                    break;
                }
                *err = blogc_error_parser(BLOGC_ERROR_TEMPLATE_PARSER, src,
                    src_len, current,
                    "Invalid variable name. Must begin with uppercase letter.");
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
                *err = blogc_error_parser(BLOGC_ERROR_TEMPLATE_PARSER, src,
                    src_len, current,
                    "Invalid variable name. Must be uppercase letter, number or '_'.");
                break;

            case TEMPLATE_VARIABLE_END:
                if (c == ' ')
                    break;
                if (c == '}') {
                    state = TEMPLATE_CLOSE_BRACKET;
                    break;
                }
                *err = blogc_error_parser(BLOGC_ERROR_TEMPLATE_PARSER, src,
                    src_len, current,
                    "Invalid statement syntax. Must end with '}}'.");
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
                *err = blogc_error_parser(BLOGC_ERROR_TEMPLATE_PARSER, src,
                    src_len, current,
                    "Invalid statement syntax. Must end with '}'.");
                break;

        }

        if (*err != NULL)
            break;

        current++;
    }

    if (*err == NULL) {
        if (if_count != 0)
            *err = blogc_error_new_printf(BLOGC_ERROR_TEMPLATE_PARSER,
                "%d 'if' statements were not closed!", if_count);
        else if (block_state != BLOCK_CLOSED)
            *err = blogc_error_new(BLOGC_ERROR_TEMPLATE_PARSER,
                "A block was not closed!");
    }

    if (*err != NULL) {
        if (stmt != NULL) {
            free(stmt->value);
            free(stmt);
        }
        blogc_template_free_stmts(stmts);
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
