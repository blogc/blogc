// SPDX-FileCopyrightText: 2014-2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <stddef.h>
#include "../common/error.h"
#include "../common/utils.h"

/*
 * note: whitespace cleaners are NOT added to AST. we fix strings right during
 * template parsing. renderer does not need to care about it, for the sake of
 * simplicity.
 *
 * another note: technically this is not an AST, because it is not a tree. duh!
 */
typedef enum {
    BLOGC_TEMPLATE_NODE_IFDEF = 1,
    BLOGC_TEMPLATE_NODE_IFNDEF,
    BLOGC_TEMPLATE_NODE_IF,
    BLOGC_TEMPLATE_NODE_ELSE,
    BLOGC_TEMPLATE_NODE_ENDIF,
    BLOGC_TEMPLATE_NODE_FOREACH,
    BLOGC_TEMPLATE_NODE_ENDFOREACH,
    BLOGC_TEMPLATE_NODE_BLOCK,
    BLOGC_TEMPLATE_NODE_ENDBLOCK,
    BLOGC_TEMPLATE_NODE_VARIABLE,
    BLOGC_TEMPLATE_NODE_CONTENT,
} blogc_template_node_type_t;

typedef enum {
    BLOGC_TEMPLATE_OP_NEQ = 1 << 0,
    BLOGC_TEMPLATE_OP_EQ  = 1 << 1,
    BLOGC_TEMPLATE_OP_LT  = 1 << 2,
    BLOGC_TEMPLATE_OP_GT  = 1 << 3,
} blogc_template_operator_t;

typedef struct {
    blogc_template_node_type_t type;
    blogc_template_operator_t op;

    // 2 slots to store node data.
    char *data[2];

    bc_slist_t *childs;
} blogc_template_node_t;

bc_slist_t* blogc_template_parse(const char *src, size_t src_len,
    bc_error_t **err);
void blogc_template_free_ast(bc_slist_t *ast);
