/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2017 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#include <stdio.h>

#include "template-parser.h"
#include "../common/utils.h"
#include "debug.h"


static const char*
get_operator(blogc_template_stmt_operator_t op)
{
    if (op & BLOGC_TEMPLATE_OP_NEQ)
        return "!=";
    if (op & BLOGC_TEMPLATE_OP_EQ) {
        if (op & BLOGC_TEMPLATE_OP_LT)
            return "<=";
        else if (op & BLOGC_TEMPLATE_OP_GT)
            return ">=";
        return "==";
    }
    if (op & BLOGC_TEMPLATE_OP_LT)
        return "<";
    else if (op & BLOGC_TEMPLATE_OP_GT)
        return ">";
    return "";
}


void
blogc_debug_template(bc_slist_t *stmts)
{
    for (bc_slist_t *tmp = stmts; tmp != NULL; tmp = tmp->next) {
        blogc_template_stmt_t *data = tmp->data;
        fprintf(stderr, "DEBUG: <TEMPLATE ");
        switch (data->type) {
            case BLOGC_TEMPLATE_IFDEF_STMT:
                fprintf(stderr, "IFDEF: %s", data->value);
                break;
            case BLOGC_TEMPLATE_IFNDEF_STMT:
                fprintf(stderr, "IFNDEF: %s", data->value);
                break;
            case BLOGC_TEMPLATE_IF_STMT:
                fprintf(stderr, "IF: %s %s %s", data->value,
                    get_operator(data->op), data->value2);
                break;
            case BLOGC_TEMPLATE_ELSE_STMT:
                fprintf(stderr, "ELSE");
                break;
            case BLOGC_TEMPLATE_ENDIF_STMT:
                fprintf(stderr, "ENDIF");
                break;
            case BLOGC_TEMPLATE_FOREACH_STMT:
                fprintf(stderr, "FOREACH: %s", data->value);
                break;
            case BLOGC_TEMPLATE_ENDFOREACH_STMT:
                fprintf(stderr, "ENDFOREACH");
                break;
            case BLOGC_TEMPLATE_BLOCK_STMT:
                fprintf(stderr, "BLOCK: %s", data->value);
                break;
            case BLOGC_TEMPLATE_ENDBLOCK_STMT:
                fprintf(stderr, "ENDBLOCK");
                break;
            case BLOGC_TEMPLATE_VARIABLE_STMT:
                fprintf(stderr, "VARIABLE: %s", data->value);
                break;
            case BLOGC_TEMPLATE_CONTENT_STMT:
                fprintf(stderr, "CONTENT: `%s`", data->value);
                break;
        }
        fprintf(stderr, ">\n");
    }
}
