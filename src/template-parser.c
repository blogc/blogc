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
#include <string.h>

#include <squareball.h>
#include "template-parser.h"
#include "error.h"


typedef enum {
    TEMPLATE_START = 1,
    TEMPLATE_OPEN_BRACKET,
    TEMPLATE_BLOCK_START,
    TEMPLATE_BLOCK_START_WHITESPACE_CLEANER,
    TEMPLATE_BLOCK_TYPE,
    TEMPLATE_BLOCK_BLOCK_TYPE_START,
    TEMPLATE_BLOCK_BLOCK_TYPE,
    TEMPLATE_BLOCK_IF_START,
    TEMPLATE_BLOCK_IF_VARIABLE,
    TEMPLATE_BLOCK_IF_OPERATOR_START,
    TEMPLATE_BLOCK_IF_OPERATOR,
    TEMPLATE_BLOCK_IF_OPERAND_START,
    TEMPLATE_BLOCK_IF_STRING_OPERAND,
    TEMPLATE_BLOCK_IF_VARIABLE_OPERAND,
    TEMPLATE_BLOCK_FOREACH_START,
    TEMPLATE_BLOCK_FOREACH_VARIABLE,
    TEMPLATE_BLOCK_END_WHITESPACE_CLEANER,
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


sb_slist_t*
blogc_template_parse(const char *src, size_t src_len, sb_error_t **err)
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

    blogc_template_stmt_operator_t tmp_op = 0;

    unsigned int if_count = 0;
    bool foreach_open = false;

    sb_slist_t *stmts = NULL;
    blogc_template_stmt_t *stmt = NULL;

    /*
     * this is a reference to the content of previous node in the singly-linked
     * list. The "correct" solution here would be implement a doubly-linked
     * list, but here are a few reasons to avoid it:
     *
     * - i'm too tired to implement it :P
     * - template parser never walk backwards, then the list itself does not
     *   need to know its previous node.
     */
    blogc_template_stmt_t *previous = NULL;

    bool lstrip_next = false;
    char *tmp = NULL;

    blogc_template_parser_state_t state = TEMPLATE_START;
    blogc_template_parser_block_state_t block_state = BLOCK_CLOSED;
    blogc_template_stmt_type_t type = BLOGC_TEMPLATE_CONTENT_STMT;

    while (current < src_len) {
        char c = src[current];
        bool last = current == src_len - 1;

        switch (state) {

            case TEMPLATE_START:
                if (last) {
                    stmt = sb_malloc(sizeof(blogc_template_stmt_t));
                    stmt->type = type;
                    if (lstrip_next) {
                        tmp = sb_strndup(src + start, src_len - start);
                        stmt->value = sb_strdup(sb_str_lstrip(tmp));
                        free(tmp);
                        tmp = NULL;
                        lstrip_next = false;
                    }
                    else {
                        stmt->value = sb_strndup(src + start, src_len - start);
                    }
                    stmt->op = 0;
                    stmt->value2 = NULL;
                    stmts = sb_slist_append(stmts, stmt);
                    previous = stmt;
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
                        state = TEMPLATE_BLOCK_START_WHITESPACE_CLEANER;
                    else
                        state = TEMPLATE_VARIABLE_START;
                    if (end > start) {
                        stmt = sb_malloc(sizeof(blogc_template_stmt_t));
                        stmt->type = type;
                        if (lstrip_next) {
                            tmp = sb_strndup(src + start, end - start);
                            stmt->value = sb_strdup(sb_str_lstrip(tmp));
                            free(tmp);
                            tmp = NULL;
                            lstrip_next = false;
                        }
                        else {
                            stmt->value = sb_strndup(src + start, end - start);
                        }
                        stmt->op = 0;
                        stmt->value2 = NULL;
                        stmts = sb_slist_append(stmts, stmt);
                        previous = stmt;
                        stmt = NULL;
                    }
                    break;
                }
                state = TEMPLATE_START;
                break;

            case TEMPLATE_BLOCK_START_WHITESPACE_CLEANER:
                if (c == '-') {
                    if ((previous != NULL) &&
                        (previous->type == BLOGC_TEMPLATE_CONTENT_STMT))
                    {
                        previous->value = sb_str_rstrip(previous->value);  // does not need copy
                    }
                    state = TEMPLATE_BLOCK_START;
                    break;
                }
                state = TEMPLATE_BLOCK_START;

            case TEMPLATE_BLOCK_START:
                if (c == ' ')
                    break;
                if (c >= 'a' && c <= 'z') {
                    state = TEMPLATE_BLOCK_TYPE;
                    start = current;
                    break;
                }
                if (c == '-') {
                    *err = blogc_error_parser(BLOGC_ERROR_TEMPLATE_PARSER, src,
                        src_len, current,
                        "Invalid statement syntax. Duplicated whitespace "
                        "cleaner before statement.");
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
                        (0 == strncmp("block", src + start, 5)))
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
                        (0 == strncmp("endblock", src + start, 8)))
                    {
                        if (block_state != BLOCK_CLOSED) {
                            state = TEMPLATE_BLOCK_END_WHITESPACE_CLEANER;
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
                        (0 == strncmp("ifdef", src + start, 5)))
                    {
                        state = TEMPLATE_BLOCK_IF_START;
                        type = BLOGC_TEMPLATE_IFDEF_STMT;
                        start = current;
                        if_count++;
                        break;
                    }
                    else if ((current - start == 6) &&
                        (0 == strncmp("ifndef", src + start, 6)))
                    {
                        state = TEMPLATE_BLOCK_IF_START;
                        type = BLOGC_TEMPLATE_IFNDEF_STMT;
                        start = current;
                        if_count++;
                        break;
                    }
                    else if ((current - start == 2) &&
                        (0 == strncmp("if", src + start, 2)))
                    {
                        state = TEMPLATE_BLOCK_IF_START;
                        type = BLOGC_TEMPLATE_IF_STMT;
                        start = current;
                        if_count++;
                        break;
                    }
                    else if ((current - start == 5) &&
                        (0 == strncmp("endif", src + start, 5)))
                    {
                        if (if_count > 0) {
                            state = TEMPLATE_BLOCK_END_WHITESPACE_CLEANER;
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
                    else if ((current - start == 7) &&
                        (0 == strncmp("foreach", src + start, 7)))
                    {
                        if (!foreach_open) {
                            state = TEMPLATE_BLOCK_FOREACH_START;
                            type = BLOGC_TEMPLATE_FOREACH_STMT;
                            start = current;
                            foreach_open = true;
                            break;
                        }
                        *err = blogc_error_parser(BLOGC_ERROR_TEMPLATE_PARSER,
                            src, src_len, current, "'foreach' statements can't "
                            "be nested.");
                        break;
                    }
                    else if ((current - start == 10) &&
                        (0 == strncmp("endforeach", src + start, 10)))
                    {
                        if (foreach_open) {
                            state = TEMPLATE_BLOCK_END_WHITESPACE_CLEANER;
                            type = BLOGC_TEMPLATE_ENDFOREACH_STMT;
                            foreach_open = false;
                            break;
                        }
                        *err = blogc_error_parser(BLOGC_ERROR_TEMPLATE_PARSER,
                            src, src_len, current,
                            "'endforeach' statement without an open 'foreach' "
                            "statement.");
                        break;
                    }
                }
                *err = blogc_error_parser(BLOGC_ERROR_TEMPLATE_PARSER, src,
                    src_len, current,
                    "Invalid statement type: Allowed types are: 'block', "
                    "'endblock', 'ifdef', 'ifndef', 'endif', 'foreach' and "
                    "'endforeach'.");
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
                    if ((current - start == 5) &&
                        (0 == strncmp("entry", src + start, 5)))
                    {
                        block_state = BLOCK_ENTRY;
                        end = current;
                        state = TEMPLATE_BLOCK_END_WHITESPACE_CLEANER;
                        break;
                    }
                    else if ((current - start == 7) &&
                        (0 == strncmp("listing", src + start, 7)))
                    {
                        block_state = BLOCK_LISTING;
                        end = current;
                        state = TEMPLATE_BLOCK_END_WHITESPACE_CLEANER;
                        break;
                    }
                    else if ((current - start == 12) &&
                        (0 == strncmp("listing_once", src + start, 12)))
                    {
                        block_state = BLOCK_LISTING_ONCE;
                        end = current;
                        state = TEMPLATE_BLOCK_END_WHITESPACE_CLEANER;
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
                        state = TEMPLATE_BLOCK_END_WHITESPACE_CLEANER;
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
                if (c >= 'A' && c <= 'Z') {
                    state = TEMPLATE_BLOCK_IF_VARIABLE_OPERAND;
                    start2 = current;
                    break;
                }
                if (c == '"') {
                    state = TEMPLATE_BLOCK_IF_STRING_OPERAND;
                    start2 = current;
                    break;
                }
                op_start = 0;
                op_end = 0;
                *err = blogc_error_parser(BLOGC_ERROR_TEMPLATE_PARSER, src,
                    src_len, current,
                    "Invalid 'if' operand. Must be double-quoted static "
                    "string or variable.");
                break;

            case TEMPLATE_BLOCK_IF_STRING_OPERAND:
                if (c != '"')
                    break;
                if (c == '"' && src[current - 1] == '\\')
                    break;
                state = TEMPLATE_BLOCK_END_WHITESPACE_CLEANER;
                end2 = current + 1;
                break;

           case TEMPLATE_BLOCK_IF_VARIABLE_OPERAND:
                if ((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_')
                    break;
                state = TEMPLATE_BLOCK_END_WHITESPACE_CLEANER;
                end2 = current;
                break;

            case TEMPLATE_BLOCK_FOREACH_START:
                if (c == ' ')
                    break;
                if (c >= 'A' && c <= 'Z') {
                    state = TEMPLATE_BLOCK_FOREACH_VARIABLE;
                    start = current;
                    break;
                }
                *err = blogc_error_parser(BLOGC_ERROR_TEMPLATE_PARSER, src,
                    src_len, current,
                    "Invalid foreach variable name. Must begin with uppercase "
                    "letter.");
                break;

            case TEMPLATE_BLOCK_FOREACH_VARIABLE:
                if ((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_')
                    break;
                if (c == ' ') {
                    end = current;
                    state = TEMPLATE_BLOCK_END_WHITESPACE_CLEANER;
                    break;
                }
                *err = blogc_error_parser(BLOGC_ERROR_TEMPLATE_PARSER, src,
                    src_len, current,
                    "Invalid foreach variable name. Must be uppercase letter, "
                    "number or '_'.");
                break;

            case TEMPLATE_BLOCK_END_WHITESPACE_CLEANER:
                if (c == ' ')
                    break;
                if (c == '-') {
                    lstrip_next = true;
                    state = TEMPLATE_BLOCK_END;
                    break;
                }
                state = TEMPLATE_BLOCK_END;

            case TEMPLATE_BLOCK_END:
                if (c == '%') {
                    state = TEMPLATE_CLOSE_BRACKET;
                    break;
                }
                if (c == '-') {
                    *err = blogc_error_parser(BLOGC_ERROR_TEMPLATE_PARSER, src,
                        src_len, current,
                        "Invalid statement syntax. Duplicated whitespace "
                        "cleaner after statement.");
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
                    tmp_op = 0;
                    if (op_end > op_start) {
                        if (op_end - op_start == 1) {
                            if (0 == strncmp("<", src + op_start, 1))
                                tmp_op = BLOGC_TEMPLATE_OP_LT;
                            else if (0 == strncmp(">", src + op_start, 1))
                                tmp_op = BLOGC_TEMPLATE_OP_GT;
                        }
                        else if (op_end - op_start == 2) {
                            if (0 == strncmp("<=", src + op_start, 2))
                                tmp_op = BLOGC_TEMPLATE_OP_LT | BLOGC_TEMPLATE_OP_EQ;
                            else if (0 == strncmp(">=", src + op_start, 2))
                                tmp_op = BLOGC_TEMPLATE_OP_GT | BLOGC_TEMPLATE_OP_EQ;
                            else if (0 == strncmp("==", src + op_start, 2))
                                tmp_op = BLOGC_TEMPLATE_OP_EQ;
                            else if (0 == strncmp("!=", src + op_start, 2))
                                tmp_op = BLOGC_TEMPLATE_OP_NEQ;
                        }
                        if (tmp_op == 0) {
                            *err = blogc_error_parser(BLOGC_ERROR_TEMPLATE_PARSER,
                                src, src_len, op_start,
                                "Invalid 'if' operator. Must be '<', '>', "
                                "'<=', '>=', '==' or '!='.");
                            op_start = 0;
                            op_end = 0;
                            break;
                        }
                        op_start = 0;
                        op_end = 0;
                    }
                    stmt = sb_malloc(sizeof(blogc_template_stmt_t));
                    stmt->type = type;
                    stmt->value = NULL;
                    stmt->op = tmp_op;
                    stmt->value2 = NULL;
                    if (end > start)
                        stmt->value = sb_strndup(src + start, end - start);
                    if (end2 > start2) {
                        stmt->value2 = sb_strndup(src + start2, end2 - start2);
                        start2 = 0;
                        end2 = 0;
                    }
                    stmts = sb_slist_append(stmts, stmt);
                    previous = stmt;
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
        if (state == TEMPLATE_BLOCK_IF_STRING_OPERAND)
            *err = blogc_error_parser(BLOGC_ERROR_TEMPLATE_PARSER, src, src_len,
                start2 - 1, "Found an open double-quoted string.");
        else if (if_count != 0)
            *err = sb_error_new_printf(BLOGC_ERROR_TEMPLATE_PARSER,
                "%d open 'ifdef' and/or 'ifndef' statements were not closed!",
                if_count);
        else if (block_state != BLOCK_CLOSED)
            *err = sb_error_new(BLOGC_ERROR_TEMPLATE_PARSER,
                "An open block was not closed!");
        else if (foreach_open)
            *err = sb_error_new(BLOGC_ERROR_TEMPLATE_PARSER,
                "An open 'foreach' statement was not closed!");
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
blogc_template_free_stmts(sb_slist_t *stmts)
{
    for (sb_slist_t *tmp = stmts; tmp != NULL; tmp = tmp->next) {
        blogc_template_stmt_t *data = tmp->data;
        if (data == NULL)
            continue;
        free(data->value);
        free(data->value2);
        free(data);
    }
    sb_slist_free(stmts);
}
