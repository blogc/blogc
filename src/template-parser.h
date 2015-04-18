/*
 * blogc: A blog compiler.
 * Copyright (C) 2015 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file COPYING.
 */

#ifndef _TEMPLATE_PARSER_H
#define _TEMPLATE_PARSER_H

#include "utils/utils.h"
#include "error.h"

typedef enum {
    BLOGC_TEMPLATE_IF_STMT = 1,
    BLOGC_TEMPLATE_ENDIF_STMT,
    BLOGC_TEMPLATE_BLOCK_STMT,
    BLOGC_TEMPLATE_ENDBLOCK_STMT,
    BLOGC_TEMPLATE_VARIABLE_STMT,
    BLOGC_TEMPLATE_CONTENT_STMT,
} blogc_template_stmt_type_t;

typedef struct {
    blogc_template_stmt_type_t type;
    char *value;
} blogc_template_stmt_t;

b_slist_t* blogc_template_parse(const char *src, size_t src_len,
    blogc_error_t **err);
void blogc_template_free_stmts(b_slist_t *stmts);

#endif /* _TEMPLATE_GRAMMAR_H */
