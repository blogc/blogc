/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2019 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#include <stdio.h>

#include "template-parser.h"
#include "../common/utils.h"
#include "debug.h"


static const char*
get_operator(blogc_template_operator_t op)
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
blogc_debug_template(bc_slist_t *ast)
{
    for (bc_slist_t *tmp = ast; tmp != NULL; tmp = tmp->next) {
        blogc_template_node_t *data = tmp->data;
        fprintf(stderr, "DEBUG: <TEMPLATE ");
        switch (data->type) {
            case BLOGC_TEMPLATE_NODE_IFDEF:
                fprintf(stderr, "IFDEF: %s", data->data[0]);
                break;
            case BLOGC_TEMPLATE_NODE_IFNDEF:
                fprintf(stderr, "IFNDEF: %s", data->data[0]);
                break;
            case BLOGC_TEMPLATE_NODE_IF:
                fprintf(stderr, "IF: %s %s %s", data->data[0],
                    get_operator(data->op), data->data[1]);
                break;
            case BLOGC_TEMPLATE_NODE_ELSE:
                fprintf(stderr, "ELSE");
                break;
            case BLOGC_TEMPLATE_NODE_ENDIF:
                fprintf(stderr, "ENDIF");
                break;
            case BLOGC_TEMPLATE_NODE_FOREACH:
                fprintf(stderr, "FOREACH: %s", data->data[0]);
                break;
            case BLOGC_TEMPLATE_NODE_ENDFOREACH:
                fprintf(stderr, "ENDFOREACH");
                break;
            case BLOGC_TEMPLATE_NODE_BLOCK:
                fprintf(stderr, "BLOCK: %s", data->data[0]);
                break;
            case BLOGC_TEMPLATE_NODE_ENDBLOCK:
                fprintf(stderr, "ENDBLOCK");
                break;
            case BLOGC_TEMPLATE_NODE_VARIABLE:
                fprintf(stderr, "VARIABLE: %s", data->data[0]);
                break;
            case BLOGC_TEMPLATE_NODE_CONTENT:
                fprintf(stderr, "CONTENT: `%s`", data->data[0]);
                break;
        }
        fprintf(stderr, ">\n");
    }
}
