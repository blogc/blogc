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
    TEMPLATE_BLOCK_IF_START,
    TEMPLATE_BLOCK_IF_VARIABLE,
    TEMPLATE_BLOCK_IF_OPERATOR_START,
    TEMPLATE_BLOCK_IF_OPERATOR,
    TEMPLATE_BLOCK_IF_OPERAND_START,
    TEMPLATE_BLOCK_IF_OPERAND,
    TEMPLATE_BLOCK_END,
    TEMPLATE_VARIABLE_START,
    TEMPLATE_VARIABLE,
    TEMPLATE_VARIABLE_END,
    TEMPLATE_CLOSE_BRACKET,
} blogc_template_parser_state_t;


typedef enum {
    BLOCK_CLOSED = 1,
    BLOCK_ENTRY,
    BLOCK_LISTING,
    BLOCK_LISTING_ONCE,
} blogc_template_parser_block_state_t;


b_slist_t*
blogc_template_parse(const char *src, size_t src_len, blogc_error_t **err)
{
    if (err == NULL || *err != NULL)
        return NULL;

    size_t current = 0;
    size_t start = 0;
    size_t end = 0;
    size_t op_start = 0;
    size_t op_end = 0;
    size_t start2 = 0;
    size_t end2 = 0;

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
                    stmt = b_malloc(sizeof(blogc_template_stmt_t));
                    stmt->type = type;
                    stmt->value = b_strndup(src + start, src_len - start);
                    stmt->op = NULL;
                    stmt->value2 = NULL;
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
                        stmt = b_malloc(sizeof(blogc_template_stmt_t));
                        stmt->type = type;
                        stmt->value = b_strndup(src + start, end - start);
                        stmt->op = NULL;
                        stmt->value2 = NULL;
                        stmts = b_slist_append(stmts, stmt);
                        stmt = NULL;
                    }
                    break;
                }
                state = TEMPLATE_START;
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
                    "Invalid statement syntax. Must begin with lowercase letter.");
                break;

            case TEMPLATE_BLOCK_TYPE:
                if (c >= 'a' && c <= 'z')
                    break;
                if (c == ' ') {
                    if ((current - start == 5) &&
                        (0 == strncmp("block", src + start, current - start)))
                    {
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
                    else if ((current - start == 8) &&
                        (0 == strncmp("endblock", src + start, current - start)))
                    {
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
                    else if ((current - start == 5) &&
                        (0 == strncmp("ifdef", src + start, current - start)))
                    {
                        state = TEMPLATE_BLOCK_IF_START;
                        type = BLOGC_TEMPLATE_IFDEF_STMT;
                        start = current;
                        if_count++;
                        break;
                    }
                    else if ((current - start == 6) &&
                        (0 == strncmp("ifndef", src + start, current - start)))
                    {
                        state = TEMPLATE_BLOCK_IF_START;
                        type = BLOGC_TEMPLATE_IFNDEF_STMT;
                        start = current;
                        if_count++;
                        break;
                    }
                    else if ((current - start == 2) &&
                        (0 == strncmp("if", src + start, current - start)))
                    {
                        state = TEMPLATE_BLOCK_IF_START;
                        type = BLOGC_TEMPLATE_IF_STMT;
                        start = current;
                        if_count++;
                        break;
                    }
                    else if ((current - start == 5) &&
                        (0 == strncmp("endif", src + start, current - start)))
                    {
                        if (if_count > 0) {
                            state = TEMPLATE_BLOCK_END;
                            type = BLOGC_TEMPLATE_ENDIF_STMT;
                            if_count--;
                            break;
                        }
                        *err = blogc_error_parser(BLOGC_ERROR_TEMPLATE_PARSER,
                            src, src_len, current,
                            "'endif' statement without an open 'ifdef' or 'ifndef' "
                            "statement.");
                        break;
                    }
                }
                *err = blogc_error_parser(BLOGC_ERROR_TEMPLATE_PARSER, src,
                    src_len, current,
                    "Invalid statement type: Allowed types are: 'block', "
                    "'endblock', 'ifdef', 'ifndef' and 'endif'.");
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
                    if (0 == strncmp("entry", src + start, current - start)) {
                        block_state = BLOCK_ENTRY;
                        end = current;
                        state = TEMPLATE_BLOCK_END;
                        break;
                    }
                    else if (0 == strncmp("listing", src + start, current - start)) {
                        block_state = BLOCK_LISTING;
                        end = current;
                        state = TEMPLATE_BLOCK_END;
                        break;
                    }
                    else if (0 == strncmp("listing_once", src + start, current - start)) {
                        block_state = BLOCK_LISTING_ONCE;
                        end = current;
                        state = TEMPLATE_BLOCK_END;
                        break;
                    }
                }
                *err = blogc_error_parser(BLOGC_ERROR_TEMPLATE_PARSER, src,
                    src_len, current,
                    "Invalid block type. Allowed types are: 'entry', 'listing' "
                    "and 'listing_once'.");
                break;

            case TEMPLATE_BLOCK_IF_START:
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
                    if (type == BLOGC_TEMPLATE_IF_STMT)
                        state = TEMPLATE_BLOCK_IF_OPERATOR_START;
                    else
                        state = TEMPLATE_BLOCK_END;
                    break;
                }
                *err = blogc_error_parser(BLOGC_ERROR_TEMPLATE_PARSER, src,
                    src_len, current,
                    "Invalid variable name. Must be uppercase letter, number "
                    "or '_'.");
                break;

            case TEMPLATE_BLOCK_IF_OPERATOR_START:
                if (c == ' ') {
                    break;
                }
                state = TEMPLATE_BLOCK_IF_OPERATOR;
                op_start = current;
                break;

            case TEMPLATE_BLOCK_IF_OPERATOR:
                if (c != ' ')
                    break;
                state = TEMPLATE_BLOCK_IF_OPERAND_START;
                op_end = current;
                break;

            case TEMPLATE_BLOCK_IF_OPERAND_START:
                if (c == ' ')
                    break;
                if (c != '"') {
                    op_start = 0;
                    op_end = 0;
                    *err = blogc_error_parser(BLOGC_ERROR_TEMPLATE_PARSER, src,
                        src_len, current,
                        "Invalid 'if' operand. Must be double-quoted static string.");
                    break;
                }
                state = TEMPLATE_BLOCK_IF_OPERAND;
                start2 = current + 1;
                break;

            case TEMPLATE_BLOCK_IF_OPERAND:
                if (c != '"')
                    break;
                if (c == '"' && src[current - 1] == '\\')
                    break;
                state = TEMPLATE_BLOCK_END;
                end2 = current;
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
                    "Invalid statement syntax. Must end with '%%}'.");
                break;

            case TEMPLATE_VARIABLE_START:
                if (c == ' ')
                    break;
                if (c >= 'A' && c <= 'Z') {
                    state = TEMPLATE_VARIABLE;
                    type = BLOGC_TEMPLATE_VARIABLE_STMT;
                    start = current;
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
                    "Invalid variable name. Must be uppercase letter, number "
                    "or '_'.");
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
                    if (op_end > op_start) {
                        if ((!((op_end - op_start == 1) &&
                               (0 == strncmp("<", src + op_start, op_end - op_start)))) &&
                            (!((op_end - op_start == 1) &&
                               (0 == strncmp(">", src + op_start, op_end - op_start)))) &&
                            (!((op_end - op_start == 2) &&
                               (0 == strncmp("<=", src + op_start, op_end - op_start)))) &&
                            (!((op_end - op_start == 2) &&
                               (0 == strncmp(">=", src + op_start, op_end - op_start)))) &&
                            (!((op_end - op_start == 2) &&
                               (0 == strncmp("==", src + op_start, op_end - op_start)))) &&
                            (!((op_end - op_start == 2) &&
                               (0 == strncmp("!=", src + op_start, op_end - op_start)))))
                        {
                            *err = blogc_error_parser(BLOGC_ERROR_TEMPLATE_PARSER,
                                src, src_len, op_start,
                                "Invalid 'if' operator. Must be '<', '>', "
                                "'<=', '>=', '==' or '!='.");
                            break;
                        }
                    }
                    stmt = b_malloc(sizeof(blogc_template_stmt_t));
                    stmt->type = type;
                    stmt->value = NULL;
                    stmt->op = NULL;
                    stmt->value2 = NULL;
                    if (op_end > op_start) {
                        stmt->op = b_strndup(src + op_start, op_end - op_start);
                        op_start = 0;
                        op_end = 0;
                    }
                    if (end > start)
                        stmt->value = b_strndup(src + start, end - start);
                    if (end2 > start2) {
                        stmt->value2 = b_strndup(src + start2, end2 - start2);
                        start2 = 0;
                        end2 = 0;
                    }
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
                "%d open 'ifdef' and/or 'ifndef' statements were not closed!",
                if_count);
        else if (block_state != BLOCK_CLOSED)
            *err = blogc_error_new(BLOGC_ERROR_TEMPLATE_PARSER,
                "An open block was not closed!");
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
        if (data == NULL)
            continue;
        free(data->value);
        free(data->op);
        free(data->value2);
        free(data);
    }
    b_slist_free(stmts);
}
