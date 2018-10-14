/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2018 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>
#include "../../src/common/error.h"
#include "../../src/common/utils.h"
#include "../../src/blogc/template-parser.h"


static void
blogc_assert_template_node(bc_slist_t *l, const char *data,
    const blogc_template_node_type_t type)
{
    blogc_template_node_t *node = l->data;
    if (data == NULL)
        assert_null(node->data[0]);
    else
        assert_string_equal(node->data[0], data);
    assert_int_equal(node->type, type);
}


static void
blogc_assert_template_if_node(bc_slist_t *l, const char *variable,
    blogc_template_operator_t operator, const char *operand)
{
    blogc_template_node_t *node = l->data;
    assert_string_equal(node->data[0], variable);
    assert_int_equal(node->op, operator);
    assert_string_equal(node->data[1], operand);
    assert_int_equal(node->type, BLOGC_TEMPLATE_NODE_IF);
}


static void
test_template_parse(void **state)
{
    const char *a =
        "Test\n"
        "\n"
        "    {%- block entry -%}\n"
        "{% ifdef CHUNDA %}\n"
        "bola\n"
        "{% endif %}\n"
        "{% ifndef BOLA %}\n"
        "bolao\n"
        "{%- endif %}\n"
        "{% endblock %}\n"
        "{% block listing %}{{ BOLA }}{% endblock %}\n"
        "{% block listing_once %}asd{% endblock %}\n"
        "{%- foreach BOLA %}hahaha{% endforeach %}\n"
        "{% if BOLA == \"1\\\"0\" %}aee{% else %}fffuuuuuuu{% endif %}";
    bc_error_t *err = NULL;
    bc_slist_t *ast = blogc_template_parse(a, strlen(a), &err);
    assert_null(err);
    assert_non_null(ast);
    blogc_assert_template_node(ast, "Test",
        BLOGC_TEMPLATE_NODE_CONTENT);
    blogc_assert_template_node(ast->next, "entry",
        BLOGC_TEMPLATE_NODE_BLOCK);
    blogc_assert_template_node(ast->next->next, "",
        BLOGC_TEMPLATE_NODE_CONTENT);
    blogc_assert_template_node(ast->next->next->next, "CHUNDA",
        BLOGC_TEMPLATE_NODE_IFDEF);
    blogc_assert_template_node(ast->next->next->next->next, "\nbola\n",
        BLOGC_TEMPLATE_NODE_CONTENT);
    blogc_assert_template_node(ast->next->next->next->next->next, NULL,
        BLOGC_TEMPLATE_NODE_ENDIF);
    blogc_assert_template_node(ast->next->next->next->next->next->next, "\n",
        BLOGC_TEMPLATE_NODE_CONTENT);
    bc_slist_t *tmp = ast->next->next->next->next->next->next->next;
    blogc_assert_template_node(tmp, "BOLA", BLOGC_TEMPLATE_NODE_IFNDEF);
    blogc_assert_template_node(tmp->next, "\nbolao", BLOGC_TEMPLATE_NODE_CONTENT);
    blogc_assert_template_node(tmp->next->next, NULL, BLOGC_TEMPLATE_NODE_ENDIF);
    blogc_assert_template_node(tmp->next->next->next, "\n",
        BLOGC_TEMPLATE_NODE_CONTENT);
    tmp = tmp->next->next->next->next;
    blogc_assert_template_node(tmp, NULL, BLOGC_TEMPLATE_NODE_ENDBLOCK);
    blogc_assert_template_node(tmp->next, "\n", BLOGC_TEMPLATE_NODE_CONTENT);
    blogc_assert_template_node(tmp->next->next, "listing",
        BLOGC_TEMPLATE_NODE_BLOCK);
    blogc_assert_template_node(tmp->next->next->next, "BOLA",
        BLOGC_TEMPLATE_NODE_VARIABLE);
    blogc_assert_template_node(tmp->next->next->next->next,
        NULL, BLOGC_TEMPLATE_NODE_ENDBLOCK);
    blogc_assert_template_node(tmp->next->next->next->next->next, "\n",
        BLOGC_TEMPLATE_NODE_CONTENT);
    blogc_assert_template_node(tmp->next->next->next->next->next->next,
        "listing_once", BLOGC_TEMPLATE_NODE_BLOCK);
    blogc_assert_template_node(tmp->next->next->next->next->next->next->next,
        "asd", BLOGC_TEMPLATE_NODE_CONTENT);
    blogc_assert_template_node(tmp->next->next->next->next->next->next->next->next,
        NULL, BLOGC_TEMPLATE_NODE_ENDBLOCK);
    blogc_assert_template_node(tmp->next->next->next->next->next->next->next->next->next,
        "", BLOGC_TEMPLATE_NODE_CONTENT);
    tmp = tmp->next->next->next->next->next->next->next->next->next->next;
    blogc_assert_template_node(tmp, "BOLA", BLOGC_TEMPLATE_NODE_FOREACH);
    blogc_assert_template_node(tmp->next, "hahaha",
        BLOGC_TEMPLATE_NODE_CONTENT);
    blogc_assert_template_node(tmp->next->next, NULL,
        BLOGC_TEMPLATE_NODE_ENDFOREACH);
    blogc_assert_template_node(tmp->next->next->next, "\n",
        BLOGC_TEMPLATE_NODE_CONTENT);
    blogc_assert_template_if_node(tmp->next->next->next->next, "BOLA",
        BLOGC_TEMPLATE_OP_EQ, "\"1\\\"0\"");
    blogc_assert_template_node(tmp->next->next->next->next->next, "aee",
        BLOGC_TEMPLATE_NODE_CONTENT);
    blogc_assert_template_node(tmp->next->next->next->next->next->next, NULL,
        BLOGC_TEMPLATE_NODE_ELSE);
    blogc_assert_template_node(tmp->next->next->next->next->next->next->next,
        "fffuuuuuuu", BLOGC_TEMPLATE_NODE_CONTENT);
    blogc_assert_template_node(tmp->next->next->next->next->next->next->next->next,
        NULL, BLOGC_TEMPLATE_NODE_ENDIF);
    assert_null(tmp->next->next->next->next->next->next->next->next->next);
    blogc_template_free_ast(ast);
}


static void
test_template_parse_crlf(void **state)
{
    const char *a =
        "Test\r\n"
        "\r\n"
        "    {%- block entry -%}\r\n"
        "{% ifdef CHUNDA %}\r\n"
        "bola\r\n"
        "{% endif %}\r\n"
        "{% ifndef BOLA %}\r\n"
        "bolao\r\n"
        "{%- endif %}\r\n"
        "{% endblock %}\r\n"
        "{% block listing %}{{ BOLA }}{% endblock %}\r\n"
        "{% block listing_once %}asd{% endblock %}\r\n"
        "{%- foreach BOLA %}hahaha{% endforeach %}\r\n"
        "{% if BOLA == \"1\\\"0\" %}aee{% else %}fffuuuuuuu{% endif %}";
    bc_error_t *err = NULL;
    bc_slist_t *ast = blogc_template_parse(a, strlen(a), &err);
    assert_null(err);
    assert_non_null(ast);
    blogc_assert_template_node(ast, "Test",
        BLOGC_TEMPLATE_NODE_CONTENT);
    blogc_assert_template_node(ast->next, "entry",
        BLOGC_TEMPLATE_NODE_BLOCK);
    blogc_assert_template_node(ast->next->next, "",
        BLOGC_TEMPLATE_NODE_CONTENT);
    blogc_assert_template_node(ast->next->next->next, "CHUNDA",
        BLOGC_TEMPLATE_NODE_IFDEF);
    blogc_assert_template_node(ast->next->next->next->next, "\r\nbola\r\n",
        BLOGC_TEMPLATE_NODE_CONTENT);
    blogc_assert_template_node(ast->next->next->next->next->next, NULL,
        BLOGC_TEMPLATE_NODE_ENDIF);
    blogc_assert_template_node(ast->next->next->next->next->next->next, "\r\n",
        BLOGC_TEMPLATE_NODE_CONTENT);
    bc_slist_t *tmp = ast->next->next->next->next->next->next->next;
    blogc_assert_template_node(tmp, "BOLA", BLOGC_TEMPLATE_NODE_IFNDEF);
    blogc_assert_template_node(tmp->next, "\r\nbolao", BLOGC_TEMPLATE_NODE_CONTENT);
    blogc_assert_template_node(tmp->next->next, NULL, BLOGC_TEMPLATE_NODE_ENDIF);
    blogc_assert_template_node(tmp->next->next->next, "\r\n",
        BLOGC_TEMPLATE_NODE_CONTENT);
    tmp = tmp->next->next->next->next;
    blogc_assert_template_node(tmp, NULL, BLOGC_TEMPLATE_NODE_ENDBLOCK);
    blogc_assert_template_node(tmp->next, "\r\n", BLOGC_TEMPLATE_NODE_CONTENT);
    blogc_assert_template_node(tmp->next->next, "listing",
        BLOGC_TEMPLATE_NODE_BLOCK);
    blogc_assert_template_node(tmp->next->next->next, "BOLA",
        BLOGC_TEMPLATE_NODE_VARIABLE);
    blogc_assert_template_node(tmp->next->next->next->next,
        NULL, BLOGC_TEMPLATE_NODE_ENDBLOCK);
    blogc_assert_template_node(tmp->next->next->next->next->next, "\r\n",
        BLOGC_TEMPLATE_NODE_CONTENT);
    blogc_assert_template_node(tmp->next->next->next->next->next->next,
        "listing_once", BLOGC_TEMPLATE_NODE_BLOCK);
    blogc_assert_template_node(tmp->next->next->next->next->next->next->next,
        "asd", BLOGC_TEMPLATE_NODE_CONTENT);
    blogc_assert_template_node(tmp->next->next->next->next->next->next->next->next,
        NULL, BLOGC_TEMPLATE_NODE_ENDBLOCK);
    blogc_assert_template_node(tmp->next->next->next->next->next->next->next->next->next,
        "", BLOGC_TEMPLATE_NODE_CONTENT);
    tmp = tmp->next->next->next->next->next->next->next->next->next->next;
    blogc_assert_template_node(tmp, "BOLA", BLOGC_TEMPLATE_NODE_FOREACH);
    blogc_assert_template_node(tmp->next, "hahaha",
        BLOGC_TEMPLATE_NODE_CONTENT);
    blogc_assert_template_node(tmp->next->next, NULL,
        BLOGC_TEMPLATE_NODE_ENDFOREACH);
    blogc_assert_template_node(tmp->next->next->next, "\r\n",
        BLOGC_TEMPLATE_NODE_CONTENT);
    blogc_assert_template_if_node(tmp->next->next->next->next, "BOLA",
        BLOGC_TEMPLATE_OP_EQ, "\"1\\\"0\"");
    blogc_assert_template_node(tmp->next->next->next->next->next, "aee",
        BLOGC_TEMPLATE_NODE_CONTENT);
    blogc_assert_template_node(tmp->next->next->next->next->next->next, NULL,
        BLOGC_TEMPLATE_NODE_ELSE);
    blogc_assert_template_node(tmp->next->next->next->next->next->next->next,
        "fffuuuuuuu", BLOGC_TEMPLATE_NODE_CONTENT);
    blogc_assert_template_node(tmp->next->next->next->next->next->next->next->next,
        NULL, BLOGC_TEMPLATE_NODE_ENDIF);
    assert_null(tmp->next->next->next->next->next->next->next->next->next);
    blogc_template_free_ast(ast);
}


static void
test_template_parse_html(void **state)
{
    const char *a =
        "<html>\n"
        "    <head>\n"
        "        {% block entry %}\n"
        "        <title>My cool blog >> {{ TITLE }}</title>\n"
        "        {% endblock %}\n"
        "        {% block listing_once %}\n"
        "        <title>My cool blog - Main page</title>\n"
        "        {% endblock %}\n"
        "    </head>\n"
        "    <body>\n"
        "        <h1>My cool blog</h1>\n"
        "        {% block entry %}\n"
        "        <h2>{{ TITLE }}</h2>\n"
        "        {% ifdef DATE %}<h4>Published in: {{ DATE }}</h4>{% endif %}\n"
        "        <pre>{{ CONTENT }}</pre>\n"
        "        {% endblock %}\n"
        "        {% block listing_once %}<ul>{% endblock %}\n"
        "        {% block listing %}<p><a href=\"{{ FILENAME }}.html\">"
        "{{ TITLE }}</a>{% ifdef DATE %} - {{ DATE }}{% endif %}</p>{% endblock %}\n"
        "        {% block listing_once %}</ul>{% endblock %}\n"
        "    </body>\n"
        "</html>\n";
    bc_error_t *err = NULL;
    bc_slist_t *ast = blogc_template_parse(a, strlen(a), &err);
    assert_null(err);
    assert_non_null(ast);
    blogc_assert_template_node(ast, "<html>\n    <head>\n        ",
        BLOGC_TEMPLATE_NODE_CONTENT);
    blogc_assert_template_node(ast->next, "entry",
        BLOGC_TEMPLATE_NODE_BLOCK);
    blogc_assert_template_node(ast->next->next,
        "\n        <title>My cool blog >> ", BLOGC_TEMPLATE_NODE_CONTENT);
    blogc_assert_template_node(ast->next->next->next, "TITLE",
        BLOGC_TEMPLATE_NODE_VARIABLE);
    blogc_assert_template_node(ast->next->next->next->next,
        "</title>\n        ", BLOGC_TEMPLATE_NODE_CONTENT);
    blogc_assert_template_node(ast->next->next->next->next->next, NULL,
        BLOGC_TEMPLATE_NODE_ENDBLOCK);
    blogc_assert_template_node(ast->next->next->next->next->next->next,
        "\n        ", BLOGC_TEMPLATE_NODE_CONTENT);
    blogc_assert_template_node(ast->next->next->next->next->next->next->next,
        "listing_once", BLOGC_TEMPLATE_NODE_BLOCK);
    bc_slist_t *tmp = ast->next->next->next->next->next->next->next->next;
    blogc_assert_template_node(tmp,
        "\n        <title>My cool blog - Main page</title>\n        ",
        BLOGC_TEMPLATE_NODE_CONTENT);
    blogc_assert_template_node(tmp->next, NULL, BLOGC_TEMPLATE_NODE_ENDBLOCK);
    blogc_assert_template_node(tmp->next->next,
        "\n    </head>\n    <body>\n        <h1>My cool blog</h1>\n        ",
        BLOGC_TEMPLATE_NODE_CONTENT);
    blogc_assert_template_node(tmp->next->next->next, "entry",
        BLOGC_TEMPLATE_NODE_BLOCK);
    blogc_assert_template_node(tmp->next->next->next->next,
        "\n        <h2>", BLOGC_TEMPLATE_NODE_CONTENT);
    blogc_assert_template_node(tmp->next->next->next->next->next,
        "TITLE", BLOGC_TEMPLATE_NODE_VARIABLE);
    blogc_assert_template_node(tmp->next->next->next->next->next->next,
        "</h2>\n        ", BLOGC_TEMPLATE_NODE_CONTENT);
    blogc_assert_template_node(tmp->next->next->next->next->next->next->next,
        "DATE", BLOGC_TEMPLATE_NODE_IFDEF);
    tmp = tmp->next->next->next->next->next->next->next->next;
    blogc_assert_template_node(tmp, "<h4>Published in: ",
        BLOGC_TEMPLATE_NODE_CONTENT);
    blogc_assert_template_node(tmp->next, "DATE", BLOGC_TEMPLATE_NODE_VARIABLE);
    blogc_assert_template_node(tmp->next->next, "</h4>",
        BLOGC_TEMPLATE_NODE_CONTENT);
    blogc_assert_template_node(tmp->next->next->next, NULL,
        BLOGC_TEMPLATE_NODE_ENDIF);
    blogc_assert_template_node(tmp->next->next->next->next, "\n        <pre>",
        BLOGC_TEMPLATE_NODE_CONTENT);
    blogc_assert_template_node(tmp->next->next->next->next->next,
        "CONTENT", BLOGC_TEMPLATE_NODE_VARIABLE);
    blogc_assert_template_node(tmp->next->next->next->next->next->next,
        "</pre>\n        ", BLOGC_TEMPLATE_NODE_CONTENT);
    blogc_assert_template_node(tmp->next->next->next->next->next->next->next,
        NULL, BLOGC_TEMPLATE_NODE_ENDBLOCK);
    tmp = tmp->next->next->next->next->next->next->next->next;
    blogc_assert_template_node(tmp, "\n        ", BLOGC_TEMPLATE_NODE_CONTENT);
    blogc_assert_template_node(tmp->next, "listing_once",
        BLOGC_TEMPLATE_NODE_BLOCK);
    blogc_assert_template_node(tmp->next->next, "<ul>",
        BLOGC_TEMPLATE_NODE_CONTENT);
    blogc_assert_template_node(tmp->next->next->next, NULL,
        BLOGC_TEMPLATE_NODE_ENDBLOCK);
    blogc_assert_template_node(tmp->next->next->next->next, "\n        ",
        BLOGC_TEMPLATE_NODE_CONTENT);
    blogc_assert_template_node(tmp->next->next->next->next->next,
        "listing", BLOGC_TEMPLATE_NODE_BLOCK);
    blogc_assert_template_node(tmp->next->next->next->next->next->next,
        "<p><a href=\"", BLOGC_TEMPLATE_NODE_CONTENT);
    blogc_assert_template_node(tmp->next->next->next->next->next->next->next,
        "FILENAME", BLOGC_TEMPLATE_NODE_VARIABLE);
    tmp = tmp->next->next->next->next->next->next->next->next;
    blogc_assert_template_node(tmp, ".html\">", BLOGC_TEMPLATE_NODE_CONTENT);
    blogc_assert_template_node(tmp->next, "TITLE",
        BLOGC_TEMPLATE_NODE_VARIABLE);
    blogc_assert_template_node(tmp->next->next, "</a>",
        BLOGC_TEMPLATE_NODE_CONTENT);
    blogc_assert_template_node(tmp->next->next->next, "DATE",
        BLOGC_TEMPLATE_NODE_IFDEF);
    blogc_assert_template_node(tmp->next->next->next->next, " - ",
        BLOGC_TEMPLATE_NODE_CONTENT);
    blogc_assert_template_node(tmp->next->next->next->next->next, "DATE",
        BLOGC_TEMPLATE_NODE_VARIABLE);
    blogc_assert_template_node(tmp->next->next->next->next->next->next,
        NULL, BLOGC_TEMPLATE_NODE_ENDIF);
    blogc_assert_template_node(tmp->next->next->next->next->next->next->next,
        "</p>", BLOGC_TEMPLATE_NODE_CONTENT);
    tmp = tmp->next->next->next->next->next->next->next->next;
    blogc_assert_template_node(tmp, NULL, BLOGC_TEMPLATE_NODE_ENDBLOCK);
    blogc_assert_template_node(tmp->next, "\n        ",
        BLOGC_TEMPLATE_NODE_CONTENT);
    blogc_assert_template_node(tmp->next->next, "listing_once",
        BLOGC_TEMPLATE_NODE_BLOCK);
    blogc_assert_template_node(tmp->next->next->next, "</ul>",
        BLOGC_TEMPLATE_NODE_CONTENT);
    blogc_assert_template_node(tmp->next->next->next->next, NULL,
        BLOGC_TEMPLATE_NODE_ENDBLOCK);
    blogc_assert_template_node(tmp->next->next->next->next->next,
        "\n    </body>\n</html>\n", BLOGC_TEMPLATE_NODE_CONTENT);
    assert_null(tmp->next->next->next->next->next->next);
    blogc_template_free_ast(ast);
}


static void
test_template_parse_ifdef_and_var_outside_block(void **state)
{
    const char *a =
        "{% ifdef GUDA %}bola{% endif %}\n"
        "{{ BOLA }}\n"
        "{% ifndef CHUNDA %}{{ CHUNDA }}{% endif %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *ast = blogc_template_parse(a, strlen(a), &err);
    assert_null(err);
    assert_non_null(ast);
    blogc_assert_template_node(ast, "GUDA", BLOGC_TEMPLATE_NODE_IFDEF);
    blogc_assert_template_node(ast->next, "bola",
        BLOGC_TEMPLATE_NODE_CONTENT);
    blogc_assert_template_node(ast->next->next, NULL,
        BLOGC_TEMPLATE_NODE_ENDIF);
    blogc_assert_template_node(ast->next->next->next, "\n",
        BLOGC_TEMPLATE_NODE_CONTENT);
    blogc_assert_template_node(ast->next->next->next->next, "BOLA",
        BLOGC_TEMPLATE_NODE_VARIABLE);
    blogc_assert_template_node(ast->next->next->next->next->next, "\n",
        BLOGC_TEMPLATE_NODE_CONTENT);
    blogc_assert_template_node(ast->next->next->next->next->next->next,
        "CHUNDA", BLOGC_TEMPLATE_NODE_IFNDEF);
    bc_slist_t *tmp = ast->next->next->next->next->next->next->next;
    blogc_assert_template_node(tmp, "CHUNDA", BLOGC_TEMPLATE_NODE_VARIABLE);
    blogc_assert_template_node(tmp->next, NULL, BLOGC_TEMPLATE_NODE_ENDIF);
    blogc_assert_template_node(tmp->next->next, "\n",
        BLOGC_TEMPLATE_NODE_CONTENT);
    assert_null(tmp->next->next->next);
    blogc_template_free_ast(ast);
}


static void
test_template_parse_nested_else(void **state)
{
    const char *a =
        "{% ifdef GUDA %}\n"
        "{% ifdef BOLA %}\n"
        "asd\n"
        "{% else %}\n"
        "{% ifdef CHUNDA %}\n"
        "qwe\n"
        "{% else %}\n"
        "rty\n"
        "{% endif %}\n"
        "{% endif %}\n"
        "{% ifdef LOL %}\n"
        "zxc\n"
        "{% else %}\n"
        "bnm\n"
        "{% endif %}\n"
        "{% endif %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *ast = blogc_template_parse(a, strlen(a), &err);
    assert_null(err);
    assert_non_null(ast);
    blogc_assert_template_node(ast, "GUDA", BLOGC_TEMPLATE_NODE_IFDEF);
    blogc_assert_template_node(ast->next, "\n", BLOGC_TEMPLATE_NODE_CONTENT);
    blogc_assert_template_node(ast->next->next, "BOLA", BLOGC_TEMPLATE_NODE_IFDEF);
    blogc_assert_template_node(ast->next->next->next, "\nasd\n",
        BLOGC_TEMPLATE_NODE_CONTENT);
    blogc_assert_template_node(ast->next->next->next->next, NULL,
        BLOGC_TEMPLATE_NODE_ELSE);
    blogc_assert_template_node(ast->next->next->next->next->next, "\n",
        BLOGC_TEMPLATE_NODE_CONTENT);
    blogc_assert_template_node(ast->next->next->next->next->next->next,
        "CHUNDA", BLOGC_TEMPLATE_NODE_IFDEF);
    blogc_assert_template_node(ast->next->next->next->next->next->next->next,
        "\nqwe\n", BLOGC_TEMPLATE_NODE_CONTENT);
    bc_slist_t *tmp = ast->next->next->next->next->next->next->next->next;
    blogc_assert_template_node(tmp, NULL, BLOGC_TEMPLATE_NODE_ELSE);
    blogc_assert_template_node(tmp->next, "\nrty\n", BLOGC_TEMPLATE_NODE_CONTENT);
    blogc_assert_template_node(tmp->next->next, NULL,
        BLOGC_TEMPLATE_NODE_ENDIF);
    blogc_assert_template_node(tmp->next->next->next, "\n",
        BLOGC_TEMPLATE_NODE_CONTENT);
    blogc_assert_template_node(tmp->next->next->next->next, NULL,
        BLOGC_TEMPLATE_NODE_ENDIF);
    blogc_assert_template_node(tmp->next->next->next->next->next, "\n",
        BLOGC_TEMPLATE_NODE_CONTENT);
    blogc_assert_template_node(tmp->next->next->next->next->next->next,
        "LOL", BLOGC_TEMPLATE_NODE_IFDEF);
    blogc_assert_template_node(tmp->next->next->next->next->next->next->next,
        "\nzxc\n", BLOGC_TEMPLATE_NODE_CONTENT);
    tmp = tmp->next->next->next->next->next->next->next->next;
    blogc_assert_template_node(tmp, NULL, BLOGC_TEMPLATE_NODE_ELSE);
    blogc_assert_template_node(tmp->next, "\nbnm\n", BLOGC_TEMPLATE_NODE_CONTENT);
    blogc_assert_template_node(tmp->next->next, NULL,
        BLOGC_TEMPLATE_NODE_ENDIF);
    blogc_assert_template_node(tmp->next->next->next, "\n",
        BLOGC_TEMPLATE_NODE_CONTENT);
    blogc_assert_template_node(tmp->next->next->next->next, NULL,
        BLOGC_TEMPLATE_NODE_ENDIF);
    blogc_assert_template_node(tmp->next->next->next->next->next, "\n",
        BLOGC_TEMPLATE_NODE_CONTENT);
    assert_null(tmp->next->next->next->next->next->next);
    blogc_template_free_ast(ast);
}


static void
test_template_parse_invalid_block_start(void **state)
{
    const char *a = "{% ASD %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *ast = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(ast);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Invalid statement syntax. Must begin with lowercase letter.\n"
        "Error occurred near line 1, position 4: {% ASD %}");
    bc_error_free(err);
    a = "{%-- block entry %}\n";
    err = NULL;
    ast = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(ast);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Invalid statement syntax. Duplicated whitespace cleaner before statement.\n"
        "Error occurred near line 1, position 4: {%-- block entry %}");
    bc_error_free(err);
    a = "{% block entry --%}\n";
    err = NULL;
    ast = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(ast);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Invalid statement syntax. Duplicated whitespace cleaner after statement.\n"
        "Error occurred near line 1, position 17: {% block entry --%}");
    bc_error_free(err);
}


static void
test_template_parse_invalid_block_nested(void **state)
{
    const char *a =
        "{% block entry %}\n"
        "{% block listing %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *ast = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(ast);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Blocks can't be nested.\n"
        "Error occurred near line 2, position 9: {% block listing %}");
    bc_error_free(err);
}


static void
test_template_parse_invalid_foreach_nested(void **state)
{
    const char *a =
        "{% foreach A %}\n"
        "{% foreach B %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *ast = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(ast);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "'foreach' statements can't be nested.\n"
        "Error occurred near line 2, position 11: {% foreach B %}");
    bc_error_free(err);
}


static void
test_template_parse_invalid_block_not_open(void **state)
{
    const char *a = "{% endblock %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *ast = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(ast);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "'endblock' statement without an open 'block' statement.\n"
        "Error occurred near line 1, position 12: {% endblock %}");
    bc_error_free(err);
}


static void
test_template_parse_invalid_endif_not_open(void **state)
{
    const char *a = "{% block listing %}{% endif %}{% endblock %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *ast = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(ast);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "'endif' statement without an open 'if', 'ifdef' or 'ifndef' statement.\n"
        "Error occurred near line 1, position 28: "
        "{% block listing %}{% endif %}{% endblock %}");
    bc_error_free(err);
}


static void
test_template_parse_invalid_endif_not_open_inside_block(void **state)
{
    const char *a = "{% ifdef BOLA %}{% block listing %}{% endif %}{% endblock %}";
    bc_error_t *err = NULL;
    bc_slist_t *ast = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(ast);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "'endif' statement without an open 'if', 'ifdef' or 'ifndef' statement.\n"
        "Error occurred near line 1, position 44: {% ifdef BOLA %}{% block "
        "listing %}{% endif %}{% endblock %}");
    bc_error_free(err);
}


static void
test_template_parse_invalid_else_not_open_inside_block(void **state)
{
    const char *a = "{% ifdef BOLA %}{% block listing %}{% else %}{% endif %}{% endblock %}";
    bc_error_t *err = NULL;
    bc_slist_t *ast = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(ast);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "'else' statement without an open 'if', 'ifdef' or 'ifndef' statement.\n"
        "Error occurred near line 1, position 43: {% ifdef BOLA %}"
        "{% block listing %}{% else %}{% endif %}{% endblock %}");
    bc_error_free(err);
}


static void
test_template_parse_invalid_endforeach_not_open(void **state)
{
    const char *a = "{% endforeach %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *ast = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(ast);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "'endforeach' statement without an open 'foreach' statement.\n"
        "Error occurred near line 1, position 14: {% endforeach %}");
    bc_error_free(err);
}


static void
test_template_parse_invalid_endforeach_not_open_inside_block(void **state)
{
    const char *a = "{% foreach TAGS %}{% block entry %}{% endforeach %}"
        "{% endblock %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *ast = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(ast);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "'endforeach' statement without an open 'foreach' statement.\n"
        "Error occurred near line 1, position 49: {% foreach TAGS %}"
        "{% block entry %}{% endforeach %}{% endblock %}");
    bc_error_free(err);
}


static void
test_template_parse_invalid_endforeach_not_open_inside_block2(void **state)
{
    const char *a = "{% block entry %}{% foreach TAGS %}"
        "{% endforeach %}{% endforeach %}{% endblock %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *ast = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(ast);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "'endforeach' statement without an open 'foreach' statement.\n"
        "Error occurred near line 1, position 65: {% block entry %}"
        "{% foreach TAGS %}{% endforeach %}{% endforeach %}{% endblock %}");
    bc_error_free(err);
}


static void
test_template_parse_invalid_endforeach_not_closed_inside_block(void **state)
{
    const char *a = "{% block entry %}{% foreach TAGS %}{% endblock %}"
        "{% endforeach %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *ast = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(ast);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "An open 'foreach' statement was not closed inside a 'entry' block!");
    bc_error_free(err);
}


static void
test_template_parse_invalid_endforeach_not_closed_inside_block2(void **state)
{
    const char *a = "{% block entry %}{% foreach TAGS %}{% endforeach %}"
        "{% foreach TAGS %}{% endblock %}{% endforeach %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *ast = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(ast);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "An open 'foreach' statement was not closed inside a 'entry' block!");
    bc_error_free(err);
}


static void
test_template_parse_invalid_block_name(void **state)
{
    const char *a = "{% chunda %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *ast = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(ast);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Invalid statement type: Allowed types are: 'block', 'endblock', 'if', "
        "'ifdef', 'ifndef', 'else', 'endif', 'foreach' and 'endforeach'.\n"
        "Error occurred near line 1, position 10: {% chunda %}");
    bc_error_free(err);
}


static void
test_template_parse_invalid_block_type_start(void **state)
{
    const char *a = "{% block ENTRY %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *ast = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(ast);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Invalid block syntax. Must begin with lowercase letter.\n"
        "Error occurred near line 1, position 10: {% block ENTRY %}");
    bc_error_free(err);
}


static void
test_template_parse_invalid_block_type(void **state)
{
    const char *a = "{% block chunda %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *ast = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(ast);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Invalid block type. Allowed types are: 'entry', 'listing' and 'listing_once'.\n"
        "Error occurred near line 1, position 16: {% block chunda %}");
    bc_error_free(err);
}


static void
test_template_parse_invalid_ifdef_start(void **state)
{
    const char *a = "{% block entry %}{% ifdef guda %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *ast = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(ast);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Invalid variable name. Must begin with uppercase letter.\n"
        "Error occurred near line 1, position 27: "
        "{% block entry %}{% ifdef guda %}");
    bc_error_free(err);
}


static void
test_template_parse_invalid_foreach_start(void **state)
{
    const char *a = "{% block entry %}{% foreach guda %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *ast = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(ast);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Invalid foreach variable name. Must begin with uppercase letter.\n"
        "Error occurred near line 1, position 29: "
        "{% block entry %}{% foreach guda %}");
    bc_error_free(err);
}


static void
test_template_parse_invalid_ifdef_variable(void **state)
{
    const char *a = "{% block entry %}{% ifdef BoLA %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *ast = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(ast);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Invalid variable name. Must be uppercase letter, number or '_'.\n"
        "Error occurred near line 1, position 28: "
        "{% block entry %}{% ifdef BoLA %}");
    bc_error_free(err);
}


static void
test_template_parse_invalid_ifdef_variable2(void **state)
{
    const char *a = "{% block entry %}{% ifdef 0123 %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *ast = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(ast);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Invalid variable name. Must begin with uppercase letter.\n"
        "Error occurred near line 1, position 27: "
        "{% block entry %}{% ifdef 0123 %}");
    bc_error_free(err);
}


static void
test_template_parse_invalid_foreach_variable(void **state)
{
    const char *a = "{% block entry %}{% foreach BoLA %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *ast = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(ast);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Invalid foreach variable name. Must be uppercase letter, number or '_'.\n"
        "Error occurred near line 1, position 30: "
        "{% block entry %}{% foreach BoLA %}");
    bc_error_free(err);
}


static void
test_template_parse_invalid_foreach_variable2(void **state)
{
    const char *a = "{% block entry %}{% foreach 0123 %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *ast = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(ast);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Invalid foreach variable name. Must begin with uppercase letter.\n"
        "Error occurred near line 1, position 29: {% block entry %}"
        "{% foreach 0123 %}");
    bc_error_free(err);
}


static void
test_template_parse_invalid_if_operator(void **state)
{
    const char *a = "{% block entry %}{% if BOLA = \"asd\" %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *ast = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(ast);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Invalid 'if' operator. Must be '<', '>', '<=', '>=', '==' or '!='.\n"
        "Error occurred near line 1, position 29: "
        "{% block entry %}{% if BOLA = \"asd\" %}");
    bc_error_free(err);
}


static void
test_template_parse_invalid_if_operand(void **state)
{
    const char *a = "{% block entry %}{% if BOLA == asd %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *ast = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(ast);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Invalid 'if' operand. Must be double-quoted static string or variable.\n"
        "Error occurred near line 1, position 32: "
        "{% block entry %}{% if BOLA == asd %}");
    bc_error_free(err);
}


static void
test_template_parse_invalid_if_operand2(void **state)
{
    const char *a = "{% block entry %}{% if BOLA == \"asd %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *ast = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(ast);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Found an open double-quoted string.\n"
        "Error occurred near line 1, position 32: "
        "{% block entry %}{% if BOLA == \"asd %}");
    bc_error_free(err);
}


static void
test_template_parse_invalid_if_operand3(void **state)
{
    const char *a = "{% block entry %}{% if BOLA == 0123 %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *ast = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(ast);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Invalid 'if' operand. Must be double-quoted static string or variable.\n"
        "Error occurred near line 1, position 32: "
        "{% block entry %}{% if BOLA == 0123 %}");
    bc_error_free(err);
}


static void
test_template_parse_invalid_else1(void **state)
{
    const char *a = "{% else %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *ast = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(ast);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "'else' statement without an open 'if', 'ifdef' or 'ifndef' statement.\n"
        "Error occurred near line 1, position 8: {% else %}");
    bc_error_free(err);
}


static void
test_template_parse_invalid_else2(void **state)
{
    const char *a = "{% if BOLA == \"123\" %}{% if GUDA == \"1\" %}{% else %}{% else %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *ast = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(ast);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "More than one 'else' statement for an open 'if', 'ifdef' or 'ifndef' "
        "statement.\nError occurred near line 1, position 60: {% if BOLA == \"123\" "
        "%}{% if GUDA == \"1\" %}{% else %}{% else %}");
    bc_error_free(err);
}


static void
test_template_parse_invalid_else3(void **state)
{
    const char *a =
        "{% if BOLA == \"123\" %}\n"
        "{% if GUDA == \"1\" %}\n"
        "{% else %}\n"
        "asd\n"
        "{% endif %}\n"
        "{% else %}\n"
        "{% else %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *ast = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(ast);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "More than one 'else' statement for an open 'if', 'ifdef' or 'ifndef' "
        "statement.\nError occurred near line 7, position 8: {% else %}");
    bc_error_free(err);
}


static void
test_template_parse_invalid_block_end(void **state)
{
    const char *a = "{% block entry }}\n";
    bc_error_t *err = NULL;
    bc_slist_t *ast = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(ast);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Invalid statement syntax. Must end with '%}'.\n"
        "Error occurred near line 1, position 16: {% block entry }}");
    bc_error_free(err);
}


static void
test_template_parse_invalid_variable_name(void **state)
{
    const char *a = "{% block entry %}{{ bola }}{% endblock %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *ast = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(ast);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Invalid variable name. Must begin with uppercase letter.\n"
        "Error occurred near line 1, position 21: "
        "{% block entry %}{{ bola }}{% endblock %}");
    bc_error_free(err);
}


static void
test_template_parse_invalid_variable_name2(void **state)
{
    const char *a = "{% block entry %}{{ Bola }}{% endblock %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *ast = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(ast);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Invalid variable name. Must be uppercase letter, number or '_'.\n"
        "Error occurred near line 1, position 22: "
        "{% block entry %}{{ Bola }}{% endblock %}");
    bc_error_free(err);
}


static void
test_template_parse_invalid_variable_name3(void **state)
{
    const char *a = "{% block entry %}{{ 0123 }}{% endblock %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *ast = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(ast);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Invalid variable name. Must begin with uppercase letter.\n"
        "Error occurred near line 1, position 21: {% block entry %}{{ 0123 }}"
        "{% endblock %}");
    bc_error_free(err);
}


static void
test_template_parse_invalid_variable_end(void **state)
{
    const char *a = "{% block entry %}{{ BOLA %}{% endblock %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *ast = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(ast);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Invalid statement syntax. Must end with '}}'.\n"
        "Error occurred near line 1, position 26: "
        "{% block entry %}{{ BOLA %}{% endblock %}");
    bc_error_free(err);
}


static void
test_template_parse_invalid_close(void **state)
{
    const char *a = "{% block entry %%\n";
    bc_error_t *err = NULL;
    bc_slist_t *ast = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(ast);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Invalid statement syntax. Must end with '}'.\n"
        "Error occurred near line 1, position 17: {% block entry %%");
    bc_error_free(err);
}


static void
test_template_parse_invalid_close2(void **state)
{
    const char *a = "{% block entry %}{{ BOLA }%{% endblock %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *ast = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(ast);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Invalid statement syntax. Must end with '}'.\n"
        "Error occurred near line 1, position 27: "
        "{% block entry %}{{ BOLA }%{% endblock %}");
    bc_error_free(err);
}


static void
test_template_parse_invalid_endif_not_closed(void **state)
{
    const char *a = "{% block entry %}{% endblock %}{% ifdef BOLA %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *ast = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(ast);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg, "1 open 'if', 'ifdef' and/or 'ifndef' statements "
        "were not closed!");
    bc_error_free(err);
}


static void
test_template_parse_invalid_endif_not_closed_inside_block(void **state)
{
    const char *a = "{% block listing %}{% ifdef BOLA %}{% endblock %}{% endif %}";
    bc_error_t *err = NULL;
    bc_slist_t *ast = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(ast);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "1 open 'if', 'ifdef' and/or 'ifndef' statements were not closed inside "
        "a 'listing' block!");
    bc_error_free(err);
}


static void
test_template_parse_invalid_else_not_closed_inside_block(void **state)
{
    const char *a = "{% block listing %}{% ifdef BOLA %}{% else %}{% endblock %}{% endif %}";
    bc_error_t *err = NULL;
    bc_slist_t *ast = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(ast);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "1 open 'if', 'ifdef' and/or 'ifndef' statements were not closed inside "
        "a 'listing' block!");
    bc_error_free(err);
}


static void
test_template_parse_invalid_block_not_closed(void **state)
{
    const char *a = "{% block entry %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *ast = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(ast);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg, "An open block was not closed!");
    bc_error_free(err);
}


static void
test_template_parse_invalid_foreach_not_closed(void **state)
{
    const char *a = "{% foreach ASD %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *ast = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(ast);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg, "An open 'foreach' statement was not closed!");
    bc_error_free(err);
}


int
main(void)
{
    const UnitTest tests[] = {
        unit_test(test_template_parse),
        unit_test(test_template_parse_crlf),
        unit_test(test_template_parse_html),
        unit_test(test_template_parse_ifdef_and_var_outside_block),
        unit_test(test_template_parse_nested_else),
        unit_test(test_template_parse_invalid_block_start),
        unit_test(test_template_parse_invalid_block_nested),
        unit_test(test_template_parse_invalid_foreach_nested),
        unit_test(test_template_parse_invalid_block_not_open),
        unit_test(test_template_parse_invalid_endif_not_open),
        unit_test(test_template_parse_invalid_endif_not_open_inside_block),
        unit_test(test_template_parse_invalid_else_not_open_inside_block),
        unit_test(test_template_parse_invalid_endforeach_not_open),
        unit_test(test_template_parse_invalid_endforeach_not_open_inside_block),
        unit_test(test_template_parse_invalid_endforeach_not_open_inside_block2),
        unit_test(test_template_parse_invalid_endforeach_not_closed_inside_block),
        unit_test(test_template_parse_invalid_endforeach_not_closed_inside_block2),
        unit_test(test_template_parse_invalid_block_name),
        unit_test(test_template_parse_invalid_block_type_start),
        unit_test(test_template_parse_invalid_block_type),
        unit_test(test_template_parse_invalid_ifdef_start),
        unit_test(test_template_parse_invalid_foreach_start),
        unit_test(test_template_parse_invalid_ifdef_variable),
        unit_test(test_template_parse_invalid_ifdef_variable2),
        unit_test(test_template_parse_invalid_foreach_variable),
        unit_test(test_template_parse_invalid_foreach_variable2),
        unit_test(test_template_parse_invalid_if_operator),
        unit_test(test_template_parse_invalid_if_operand),
        unit_test(test_template_parse_invalid_if_operand2),
        unit_test(test_template_parse_invalid_if_operand3),
        unit_test(test_template_parse_invalid_else1),
        unit_test(test_template_parse_invalid_else2),
        unit_test(test_template_parse_invalid_else3),
        unit_test(test_template_parse_invalid_block_end),
        unit_test(test_template_parse_invalid_variable_name),
        unit_test(test_template_parse_invalid_variable_name2),
        unit_test(test_template_parse_invalid_variable_name3),
        unit_test(test_template_parse_invalid_variable_end),
        unit_test(test_template_parse_invalid_close),
        unit_test(test_template_parse_invalid_close2),
        unit_test(test_template_parse_invalid_endif_not_closed),
        unit_test(test_template_parse_invalid_endif_not_closed_inside_block),
        unit_test(test_template_parse_invalid_else_not_closed_inside_block),
        unit_test(test_template_parse_invalid_block_not_closed),
        unit_test(test_template_parse_invalid_foreach_not_closed),
    };
    return run_tests(tests);
}
