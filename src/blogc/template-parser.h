/*
 * blogc: A blog compiler.
 * Copyright (C) 2015-2016 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifndef _TEMPLATE_PARSER_H
#define _TEMPLATE_PARSER_H

#include <stddef.h>
#include "error.h"
#include "../common/utils.h"

/*
 * note: whitespace cleaners are NOT added to ast. we fix strings right during
 * template parsing. renderer does not need to care about it, for the sake of
 * simplicity.
 */
typedef enum {
    BLOGC_TEMPLATE_IFDEF_STMT = 1,
    BLOGC_TEMPLATE_IFNDEF_STMT,
    BLOGC_TEMPLATE_IF_STMT,
    BLOGC_TEMPLATE_ELSE_STMT,
    BLOGC_TEMPLATE_ENDIF_STMT,
    BLOGC_TEMPLATE_FOREACH_STMT,
    BLOGC_TEMPLATE_ENDFOREACH_STMT,
    BLOGC_TEMPLATE_BLOCK_STMT,
    BLOGC_TEMPLATE_ENDBLOCK_STMT,
    BLOGC_TEMPLATE_VARIABLE_STMT,
    BLOGC_TEMPLATE_CONTENT_STMT,
} blogc_template_stmt_type_t;

typedef enum {
    BLOGC_TEMPLATE_OP_NEQ = 1 << 0,
    BLOGC_TEMPLATE_OP_EQ  = 1 << 1,
    BLOGC_TEMPLATE_OP_LT  = 1 << 2,
    BLOGC_TEMPLATE_OP_GT  = 1 << 3,
} blogc_template_stmt_operator_t;

typedef struct {
    blogc_template_stmt_type_t type;
    char *value;
    char *value2;
    blogc_template_stmt_operator_t op;
} blogc_template_stmt_t;

bc_slist_t* blogc_template_parse(const char *src, size_t src_len,
    blogc_error_t **err);
void blogc_template_free_stmts(bc_slist_t *stmts);

#endif /* _TEMPLATE_PARSER_H */
