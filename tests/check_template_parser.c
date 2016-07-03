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

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>
#include "../src/template-parser.h"
#include "../src/error.h"
#include "../src/utils.h"


static void
blogc_assert_template_stmt(sb_slist_t *l, const char *value,
    const blogc_template_stmt_type_t type)
{
    blogc_template_stmt_t *stmt = l->data;
    if (value == NULL)
        assert_null(stmt->value);
    else
        assert_string_equal(stmt->value, value);
    assert_int_equal(stmt->type, type);
}


static void
blogc_assert_template_if_stmt(sb_slist_t *l, const char *variable,
    blogc_template_stmt_operator_t operator, const char *operand)
{
    blogc_template_stmt_t *stmt = l->data;
    assert_string_equal(stmt->value, variable);
    assert_int_equal(stmt->op, operator);
    assert_string_equal(stmt->value2, operand);
    assert_int_equal(stmt->type, BLOGC_TEMPLATE_IF_STMT);
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
    blogc_error_t *err = NULL;
    sb_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
    assert_null(err);
    assert_non_null(stmts);
    blogc_assert_template_stmt(stmts, "Test",
        BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(stmts->next, "entry",
        BLOGC_TEMPLATE_BLOCK_STMT);
    blogc_assert_template_stmt(stmts->next->next, "",
        BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(stmts->next->next->next, "CHUNDA",
        BLOGC_TEMPLATE_IFDEF_STMT);
    blogc_assert_template_stmt(stmts->next->next->next->next, "\nbola\n",
        BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(stmts->next->next->next->next->next, NULL,
        BLOGC_TEMPLATE_ENDIF_STMT);
    blogc_assert_template_stmt(stmts->next->next->next->next->next->next, "\n",
        BLOGC_TEMPLATE_CONTENT_STMT);
    sb_slist_t *tmp = stmts->next->next->next->next->next->next->next;
    blogc_assert_template_stmt(tmp, "BOLA", BLOGC_TEMPLATE_IFNDEF_STMT);
    blogc_assert_template_stmt(tmp->next, "\nbolao", BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(tmp->next->next, NULL, BLOGC_TEMPLATE_ENDIF_STMT);
    blogc_assert_template_stmt(tmp->next->next->next, "\n",
        BLOGC_TEMPLATE_CONTENT_STMT);
    tmp = tmp->next->next->next->next;
    blogc_assert_template_stmt(tmp, NULL, BLOGC_TEMPLATE_ENDBLOCK_STMT);
    blogc_assert_template_stmt(tmp->next, "\n", BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(tmp->next->next, "listing",
        BLOGC_TEMPLATE_BLOCK_STMT);
    blogc_assert_template_stmt(tmp->next->next->next, "BOLA",
        BLOGC_TEMPLATE_VARIABLE_STMT);
    blogc_assert_template_stmt(tmp->next->next->next->next,
        NULL, BLOGC_TEMPLATE_ENDBLOCK_STMT);
    blogc_assert_template_stmt(tmp->next->next->next->next->next, "\n",
        BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(tmp->next->next->next->next->next->next,
        "listing_once", BLOGC_TEMPLATE_BLOCK_STMT);
    blogc_assert_template_stmt(tmp->next->next->next->next->next->next->next,
        "asd", BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(tmp->next->next->next->next->next->next->next->next,
        NULL, BLOGC_TEMPLATE_ENDBLOCK_STMT);
    blogc_assert_template_stmt(tmp->next->next->next->next->next->next->next->next->next,
        "", BLOGC_TEMPLATE_CONTENT_STMT);
    tmp = tmp->next->next->next->next->next->next->next->next->next->next;
    blogc_assert_template_stmt(tmp, "BOLA", BLOGC_TEMPLATE_FOREACH_STMT);
    blogc_assert_template_stmt(tmp->next, "hahaha",
        BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(tmp->next->next, NULL,
        BLOGC_TEMPLATE_ENDFOREACH_STMT);
    blogc_assert_template_stmt(tmp->next->next->next, "\n",
        BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_if_stmt(tmp->next->next->next->next, "BOLA",
        BLOGC_TEMPLATE_OP_EQ, "\"1\\\"0\"");
    blogc_assert_template_stmt(tmp->next->next->next->next->next, "aee",
        BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(tmp->next->next->next->next->next->next, NULL,
        BLOGC_TEMPLATE_ELSE_STMT);
    blogc_assert_template_stmt(tmp->next->next->next->next->next->next->next,
        "fffuuuuuuu", BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(tmp->next->next->next->next->next->next->next->next,
        NULL, BLOGC_TEMPLATE_ENDIF_STMT);
    assert_null(tmp->next->next->next->next->next->next->next->next->next);
    blogc_template_free_stmts(stmts);
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
    blogc_error_t *err = NULL;
    sb_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
    assert_null(err);
    assert_non_null(stmts);
    blogc_assert_template_stmt(stmts, "Test",
        BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(stmts->next, "entry",
        BLOGC_TEMPLATE_BLOCK_STMT);
    blogc_assert_template_stmt(stmts->next->next, "",
        BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(stmts->next->next->next, "CHUNDA",
        BLOGC_TEMPLATE_IFDEF_STMT);
    blogc_assert_template_stmt(stmts->next->next->next->next, "\r\nbola\r\n",
        BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(stmts->next->next->next->next->next, NULL,
        BLOGC_TEMPLATE_ENDIF_STMT);
    blogc_assert_template_stmt(stmts->next->next->next->next->next->next, "\r\n",
        BLOGC_TEMPLATE_CONTENT_STMT);
    sb_slist_t *tmp = stmts->next->next->next->next->next->next->next;
    blogc_assert_template_stmt(tmp, "BOLA", BLOGC_TEMPLATE_IFNDEF_STMT);
    blogc_assert_template_stmt(tmp->next, "\r\nbolao", BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(tmp->next->next, NULL, BLOGC_TEMPLATE_ENDIF_STMT);
    blogc_assert_template_stmt(tmp->next->next->next, "\r\n",
        BLOGC_TEMPLATE_CONTENT_STMT);
    tmp = tmp->next->next->next->next;
    blogc_assert_template_stmt(tmp, NULL, BLOGC_TEMPLATE_ENDBLOCK_STMT);
    blogc_assert_template_stmt(tmp->next, "\r\n", BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(tmp->next->next, "listing",
        BLOGC_TEMPLATE_BLOCK_STMT);
    blogc_assert_template_stmt(tmp->next->next->next, "BOLA",
        BLOGC_TEMPLATE_VARIABLE_STMT);
    blogc_assert_template_stmt(tmp->next->next->next->next,
        NULL, BLOGC_TEMPLATE_ENDBLOCK_STMT);
    blogc_assert_template_stmt(tmp->next->next->next->next->next, "\r\n",
        BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(tmp->next->next->next->next->next->next,
        "listing_once", BLOGC_TEMPLATE_BLOCK_STMT);
    blogc_assert_template_stmt(tmp->next->next->next->next->next->next->next,
        "asd", BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(tmp->next->next->next->next->next->next->next->next,
        NULL, BLOGC_TEMPLATE_ENDBLOCK_STMT);
    blogc_assert_template_stmt(tmp->next->next->next->next->next->next->next->next->next,
        "", BLOGC_TEMPLATE_CONTENT_STMT);
    tmp = tmp->next->next->next->next->next->next->next->next->next->next;
    blogc_assert_template_stmt(tmp, "BOLA", BLOGC_TEMPLATE_FOREACH_STMT);
    blogc_assert_template_stmt(tmp->next, "hahaha",
        BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(tmp->next->next, NULL,
        BLOGC_TEMPLATE_ENDFOREACH_STMT);
    blogc_assert_template_stmt(tmp->next->next->next, "\r\n",
        BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_if_stmt(tmp->next->next->next->next, "BOLA",
        BLOGC_TEMPLATE_OP_EQ, "\"1\\\"0\"");
    blogc_assert_template_stmt(tmp->next->next->next->next->next, "aee",
        BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(tmp->next->next->next->next->next->next, NULL,
        BLOGC_TEMPLATE_ELSE_STMT);
    blogc_assert_template_stmt(tmp->next->next->next->next->next->next->next,
        "fffuuuuuuu", BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(tmp->next->next->next->next->next->next->next->next,
        NULL, BLOGC_TEMPLATE_ENDIF_STMT);
    assert_null(tmp->next->next->next->next->next->next->next->next->next);
    blogc_template_free_stmts(stmts);
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
    blogc_error_t *err = NULL;
    sb_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
    assert_null(err);
    assert_non_null(stmts);
    blogc_assert_template_stmt(stmts, "<html>\n    <head>\n        ",
        BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(stmts->next, "entry",
        BLOGC_TEMPLATE_BLOCK_STMT);
    blogc_assert_template_stmt(stmts->next->next,
        "\n        <title>My cool blog >> ", BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(stmts->next->next->next, "TITLE",
        BLOGC_TEMPLATE_VARIABLE_STMT);
    blogc_assert_template_stmt(stmts->next->next->next->next,
        "</title>\n        ", BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(stmts->next->next->next->next->next, NULL,
        BLOGC_TEMPLATE_ENDBLOCK_STMT);
    blogc_assert_template_stmt(stmts->next->next->next->next->next->next,
        "\n        ", BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(stmts->next->next->next->next->next->next->next,
        "listing_once", BLOGC_TEMPLATE_BLOCK_STMT);
    sb_slist_t *tmp = stmts->next->next->next->next->next->next->next->next;
    blogc_assert_template_stmt(tmp,
        "\n        <title>My cool blog - Main page</title>\n        ",
        BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(tmp->next, NULL, BLOGC_TEMPLATE_ENDBLOCK_STMT);
    blogc_assert_template_stmt(tmp->next->next,
        "\n    </head>\n    <body>\n        <h1>My cool blog</h1>\n        ",
        BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(tmp->next->next->next, "entry",
        BLOGC_TEMPLATE_BLOCK_STMT);
    blogc_assert_template_stmt(tmp->next->next->next->next,
        "\n        <h2>", BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(tmp->next->next->next->next->next,
        "TITLE", BLOGC_TEMPLATE_VARIABLE_STMT);
    blogc_assert_template_stmt(tmp->next->next->next->next->next->next,
        "</h2>\n        ", BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(tmp->next->next->next->next->next->next->next,
        "DATE", BLOGC_TEMPLATE_IFDEF_STMT);
    tmp = tmp->next->next->next->next->next->next->next->next;
    blogc_assert_template_stmt(tmp, "<h4>Published in: ",
        BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(tmp->next, "DATE", BLOGC_TEMPLATE_VARIABLE_STMT);
    blogc_assert_template_stmt(tmp->next->next, "</h4>",
        BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(tmp->next->next->next, NULL,
        BLOGC_TEMPLATE_ENDIF_STMT);
    blogc_assert_template_stmt(tmp->next->next->next->next, "\n        <pre>",
        BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(tmp->next->next->next->next->next,
        "CONTENT", BLOGC_TEMPLATE_VARIABLE_STMT);
    blogc_assert_template_stmt(tmp->next->next->next->next->next->next,
        "</pre>\n        ", BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(tmp->next->next->next->next->next->next->next,
        NULL, BLOGC_TEMPLATE_ENDBLOCK_STMT);
    tmp = tmp->next->next->next->next->next->next->next->next;
    blogc_assert_template_stmt(tmp, "\n        ", BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(tmp->next, "listing_once",
        BLOGC_TEMPLATE_BLOCK_STMT);
    blogc_assert_template_stmt(tmp->next->next, "<ul>",
        BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(tmp->next->next->next, NULL,
        BLOGC_TEMPLATE_ENDBLOCK_STMT);
    blogc_assert_template_stmt(tmp->next->next->next->next, "\n        ",
        BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(tmp->next->next->next->next->next,
        "listing", BLOGC_TEMPLATE_BLOCK_STMT);
    blogc_assert_template_stmt(tmp->next->next->next->next->next->next,
        "<p><a href=\"", BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(tmp->next->next->next->next->next->next->next,
        "FILENAME", BLOGC_TEMPLATE_VARIABLE_STMT);
    tmp = tmp->next->next->next->next->next->next->next->next;
    blogc_assert_template_stmt(tmp, ".html\">", BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(tmp->next, "TITLE",
        BLOGC_TEMPLATE_VARIABLE_STMT);
    blogc_assert_template_stmt(tmp->next->next, "</a>",
        BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(tmp->next->next->next, "DATE",
        BLOGC_TEMPLATE_IFDEF_STMT);
    blogc_assert_template_stmt(tmp->next->next->next->next, " - ",
        BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(tmp->next->next->next->next->next, "DATE",
        BLOGC_TEMPLATE_VARIABLE_STMT);
    blogc_assert_template_stmt(tmp->next->next->next->next->next->next,
        NULL, BLOGC_TEMPLATE_ENDIF_STMT);
    blogc_assert_template_stmt(tmp->next->next->next->next->next->next->next,
        "</p>", BLOGC_TEMPLATE_CONTENT_STMT);
    tmp = tmp->next->next->next->next->next->next->next->next;
    blogc_assert_template_stmt(tmp, NULL, BLOGC_TEMPLATE_ENDBLOCK_STMT);
    blogc_assert_template_stmt(tmp->next, "\n        ",
        BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(tmp->next->next, "listing_once",
        BLOGC_TEMPLATE_BLOCK_STMT);
    blogc_assert_template_stmt(tmp->next->next->next, "</ul>",
        BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(tmp->next->next->next->next, NULL,
        BLOGC_TEMPLATE_ENDBLOCK_STMT);
    blogc_assert_template_stmt(tmp->next->next->next->next->next,
        "\n    </body>\n</html>\n", BLOGC_TEMPLATE_CONTENT_STMT);
    assert_null(tmp->next->next->next->next->next->next);
    blogc_template_free_stmts(stmts);
}


static void
test_template_parse_ifdef_and_var_outside_block(void **state)
{
    const char *a =
        "{% ifdef GUDA %}bola{% endif %}\n"
        "{{ BOLA }}\n"
        "{% ifndef CHUNDA %}{{ CHUNDA }}{% endif %}\n";
    blogc_error_t *err = NULL;
    sb_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
    assert_null(err);
    assert_non_null(stmts);
    blogc_assert_template_stmt(stmts, "GUDA", BLOGC_TEMPLATE_IFDEF_STMT);
    blogc_assert_template_stmt(stmts->next, "bola",
        BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(stmts->next->next, NULL,
        BLOGC_TEMPLATE_ENDIF_STMT);
    blogc_assert_template_stmt(stmts->next->next->next, "\n",
        BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(stmts->next->next->next->next, "BOLA",
        BLOGC_TEMPLATE_VARIABLE_STMT);
    blogc_assert_template_stmt(stmts->next->next->next->next->next, "\n",
        BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(stmts->next->next->next->next->next->next,
        "CHUNDA", BLOGC_TEMPLATE_IFNDEF_STMT);
    sb_slist_t *tmp = stmts->next->next->next->next->next->next->next;
    blogc_assert_template_stmt(tmp, "CHUNDA", BLOGC_TEMPLATE_VARIABLE_STMT);
    blogc_assert_template_stmt(tmp->next, NULL, BLOGC_TEMPLATE_ENDIF_STMT);
    blogc_assert_template_stmt(tmp->next->next, "\n",
        BLOGC_TEMPLATE_CONTENT_STMT);
    assert_null(tmp->next->next->next);
    blogc_template_free_stmts(stmts);
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
    blogc_error_t *err = NULL;
    sb_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
    assert_null(err);
    assert_non_null(stmts);
    blogc_assert_template_stmt(stmts, "GUDA", BLOGC_TEMPLATE_IFDEF_STMT);
    blogc_assert_template_stmt(stmts->next, "\n", BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(stmts->next->next, "BOLA", BLOGC_TEMPLATE_IFDEF_STMT);
    blogc_assert_template_stmt(stmts->next->next->next, "\nasd\n",
        BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(stmts->next->next->next->next, NULL,
        BLOGC_TEMPLATE_ELSE_STMT);
    blogc_assert_template_stmt(stmts->next->next->next->next->next, "\n",
        BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(stmts->next->next->next->next->next->next,
        "CHUNDA", BLOGC_TEMPLATE_IFDEF_STMT);
    blogc_assert_template_stmt(stmts->next->next->next->next->next->next->next,
        "\nqwe\n", BLOGC_TEMPLATE_CONTENT_STMT);
    sb_slist_t *tmp = stmts->next->next->next->next->next->next->next->next;
    blogc_assert_template_stmt(tmp, NULL, BLOGC_TEMPLATE_ELSE_STMT);
    blogc_assert_template_stmt(tmp->next, "\nrty\n", BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(tmp->next->next, NULL,
        BLOGC_TEMPLATE_ENDIF_STMT);
    blogc_assert_template_stmt(tmp->next->next->next, "\n",
        BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(tmp->next->next->next->next, NULL,
        BLOGC_TEMPLATE_ENDIF_STMT);
    blogc_assert_template_stmt(tmp->next->next->next->next->next, "\n",
        BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(tmp->next->next->next->next->next->next,
        "LOL", BLOGC_TEMPLATE_IFDEF_STMT);
    blogc_assert_template_stmt(tmp->next->next->next->next->next->next->next,
        "\nzxc\n", BLOGC_TEMPLATE_CONTENT_STMT);
    tmp = tmp->next->next->next->next->next->next->next->next;
    blogc_assert_template_stmt(tmp, NULL, BLOGC_TEMPLATE_ELSE_STMT);
    blogc_assert_template_stmt(tmp->next, "\nbnm\n", BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(tmp->next->next, NULL,
        BLOGC_TEMPLATE_ENDIF_STMT);
    blogc_assert_template_stmt(tmp->next->next->next, "\n",
        BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(tmp->next->next->next->next, NULL,
        BLOGC_TEMPLATE_ENDIF_STMT);
    blogc_assert_template_stmt(tmp->next->next->next->next->next, "\n",
        BLOGC_TEMPLATE_CONTENT_STMT);
    assert_null(tmp->next->next->next->next->next->next);
    blogc_template_free_stmts(stmts);
}


static void
test_template_parse_invalid_block_start(void **state)
{
    const char *a = "{% ASD %}\n";
    blogc_error_t *err = NULL;
    sb_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(stmts);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Invalid statement syntax. Must begin with lowercase letter.\n"
        "Error occurred near line 1, position 4: {% ASD %}");
    blogc_error_free(err);
    a = "{%-- block entry %}\n";
    err = NULL;
    stmts = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(stmts);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Invalid statement syntax. Duplicated whitespace cleaner before statement.\n"
        "Error occurred near line 1, position 4: {%-- block entry %}");
    blogc_error_free(err);
    a = "{% block entry --%}\n";
    err = NULL;
    stmts = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(stmts);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Invalid statement syntax. Duplicated whitespace cleaner after statement.\n"
        "Error occurred near line 1, position 17: {% block entry --%}");
    blogc_error_free(err);
}


static void
test_template_parse_invalid_block_nested(void **state)
{
    const char *a =
        "{% block entry %}\n"
        "{% block listing %}\n";
    blogc_error_t *err = NULL;
    sb_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(stmts);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Blocks can't be nested.\n"
        "Error occurred near line 2, position 9: {% block listing %}");
    blogc_error_free(err);
}


static void
test_template_parse_invalid_foreach_nested(void **state)
{
    const char *a =
        "{% foreach A %}\n"
        "{% foreach B %}\n";
    blogc_error_t *err = NULL;
    sb_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(stmts);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "'foreach' statements can't be nested.\n"
        "Error occurred near line 2, position 11: {% foreach B %}");
    blogc_error_free(err);
}


static void
test_template_parse_invalid_block_not_open(void **state)
{
    const char *a = "{% endblock %}\n";
    blogc_error_t *err = NULL;
    sb_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(stmts);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "'endblock' statement without an open 'block' statement.\n"
        "Error occurred near line 1, position 12: {% endblock %}");
    blogc_error_free(err);
}


static void
test_template_parse_invalid_endif_not_open(void **state)
{
    const char *a = "{% block listing %}{% endif %}{% endblock %}\n";
    blogc_error_t *err = NULL;
    sb_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(stmts);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "'endif' statement without an open 'if', 'ifdef' or 'ifndef' statement.\n"
        "Error occurred near line 1, position 28: "
        "{% block listing %}{% endif %}{% endblock %}");
    blogc_error_free(err);
}


static void
test_template_parse_invalid_endforeach_not_open(void **state)
{
    const char *a = "{% endforeach %}\n";
    blogc_error_t *err = NULL;
    sb_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(stmts);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "'endforeach' statement without an open 'foreach' statement.\n"
        "Error occurred near line 1, position 14: {% endforeach %}");
    blogc_error_free(err);
}


static void
test_template_parse_invalid_block_name(void **state)
{
    const char *a = "{% chunda %}\n";
    blogc_error_t *err = NULL;
    sb_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(stmts);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Invalid statement type: Allowed types are: 'block', 'endblock', 'ifdef', "
        "'ifndef', 'else', 'endif', 'foreach' and 'endforeach'.\n"
        "Error occurred near line 1, position 10: {% chunda %}");
    blogc_error_free(err);
}


static void
test_template_parse_invalid_block_type_start(void **state)
{
    const char *a = "{% block ENTRY %}\n";
    blogc_error_t *err = NULL;
    sb_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(stmts);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Invalid block syntax. Must begin with lowercase letter.\n"
        "Error occurred near line 1, position 10: {% block ENTRY %}");
    blogc_error_free(err);
}


static void
test_template_parse_invalid_block_type(void **state)
{
    const char *a = "{% block chunda %}\n";
    blogc_error_t *err = NULL;
    sb_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(stmts);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Invalid block type. Allowed types are: 'entry', 'listing' and 'listing_once'.\n"
        "Error occurred near line 1, position 16: {% block chunda %}");
    blogc_error_free(err);
}


static void
test_template_parse_invalid_ifdef_start(void **state)
{
    const char *a = "{% block entry %}{% ifdef guda %}\n";
    blogc_error_t *err = NULL;
    sb_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(stmts);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Invalid variable name. Must begin with uppercase letter.\n"
        "Error occurred near line 1, position 27: "
        "{% block entry %}{% ifdef guda %}");
    blogc_error_free(err);
}


static void
test_template_parse_invalid_foreach_start(void **state)
{
    const char *a = "{% block entry %}{% foreach guda %}\n";
    blogc_error_t *err = NULL;
    sb_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(stmts);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Invalid foreach variable name. Must begin with uppercase letter.\n"
        "Error occurred near line 1, position 29: "
        "{% block entry %}{% foreach guda %}");
    blogc_error_free(err);
}


static void
test_template_parse_invalid_ifdef_variable(void **state)
{
    const char *a = "{% block entry %}{% ifdef BoLA %}\n";
    blogc_error_t *err = NULL;
    sb_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(stmts);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Invalid variable name. Must be uppercase letter, number or '_'.\n"
        "Error occurred near line 1, position 28: "
        "{% block entry %}{% ifdef BoLA %}");
    blogc_error_free(err);
}


static void
test_template_parse_invalid_ifdef_variable2(void **state)
{
    const char *a = "{% block entry %}{% ifdef 0123 %}\n";
    blogc_error_t *err = NULL;
    sb_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(stmts);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Invalid variable name. Must begin with uppercase letter.\n"
        "Error occurred near line 1, position 27: "
        "{% block entry %}{% ifdef 0123 %}");
    blogc_error_free(err);
}


static void
test_template_parse_invalid_foreach_variable(void **state)
{
    const char *a = "{% block entry %}{% foreach BoLA %}\n";
    blogc_error_t *err = NULL;
    sb_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(stmts);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Invalid foreach variable name. Must be uppercase letter, number or '_'.\n"
        "Error occurred near line 1, position 30: "
        "{% block entry %}{% foreach BoLA %}");
    blogc_error_free(err);
}


static void
test_template_parse_invalid_foreach_variable2(void **state)
{
    const char *a = "{% block entry %}{% foreach 0123 %}\n";
    blogc_error_t *err = NULL;
    sb_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(stmts);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Invalid foreach variable name. Must begin with uppercase letter.\n"
        "Error occurred near line 1, position 29: {% block entry %}"
        "{% foreach 0123 %}");
    blogc_error_free(err);
}


static void
test_template_parse_invalid_if_operator(void **state)
{
    const char *a = "{% block entry %}{% if BOLA = \"asd\" %}\n";
    blogc_error_t *err = NULL;
    sb_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(stmts);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Invalid 'if' operator. Must be '<', '>', '<=', '>=', '==' or '!='.\n"
        "Error occurred near line 1, position 29: "
        "{% block entry %}{% if BOLA = \"asd\" %}");
    blogc_error_free(err);
}


static void
test_template_parse_invalid_if_operand(void **state)
{
    const char *a = "{% block entry %}{% if BOLA == asd %}\n";
    blogc_error_t *err = NULL;
    sb_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(stmts);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Invalid 'if' operand. Must be double-quoted static string or variable.\n"
        "Error occurred near line 1, position 32: "
        "{% block entry %}{% if BOLA == asd %}");
    blogc_error_free(err);
}


static void
test_template_parse_invalid_if_operand2(void **state)
{
    const char *a = "{% block entry %}{% if BOLA == \"asd %}\n";
    blogc_error_t *err = NULL;
    sb_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(stmts);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Found an open double-quoted string.\n"
        "Error occurred near line 1, position 32: "
        "{% block entry %}{% if BOLA == \"asd %}");
    blogc_error_free(err);
}


static void
test_template_parse_invalid_if_operand3(void **state)
{
    const char *a = "{% block entry %}{% if BOLA == 0123 %}\n";
    blogc_error_t *err = NULL;
    sb_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(stmts);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Invalid 'if' operand. Must be double-quoted static string or variable.\n"
        "Error occurred near line 1, position 32: "
        "{% block entry %}{% if BOLA == 0123 %}");
    blogc_error_free(err);
}


static void
test_template_parse_invalid_else1(void **state)
{
    const char *a = "{% else %}\n";
    blogc_error_t *err = NULL;
    sb_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(stmts);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "'else' statement without an open 'if', 'ifdef' or 'ifndef' statement.\n"
        "Error occurred near line 1, position 8: {% else %}");
    blogc_error_free(err);
}


static void
test_template_parse_invalid_else2(void **state)
{
    const char *a = "{% if BOLA == \"123\" %}{% if GUDA == \"1\" %}{% else %}{% else %}\n";
    blogc_error_t *err = NULL;
    sb_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(stmts);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "More than one 'else' statement for an open 'if', 'ifdef' or 'ifndef' "
        "statement.\nError occurred near line 1, position 60: {% if BOLA == \"123\" "
        "%}{% if GUDA == \"1\" %}{% else %}{% else %}");
    blogc_error_free(err);
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
    blogc_error_t *err = NULL;
    sb_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(stmts);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "More than one 'else' statement for an open 'if', 'ifdef' or 'ifndef' "
        "statement.\nError occurred near line 7, position 8: {% else %}");
    blogc_error_free(err);
}


static void
test_template_parse_invalid_block_end(void **state)
{
    const char *a = "{% block entry }}\n";
    blogc_error_t *err = NULL;
    sb_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(stmts);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Invalid statement syntax. Must end with '%}'.\n"
        "Error occurred near line 1, position 16: {% block entry }}");
    blogc_error_free(err);
}


static void
test_template_parse_invalid_variable_name(void **state)
{
    const char *a = "{% block entry %}{{ bola }}{% endblock %}\n";
    blogc_error_t *err = NULL;
    sb_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(stmts);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Invalid variable name. Must begin with uppercase letter.\n"
        "Error occurred near line 1, position 21: "
        "{% block entry %}{{ bola }}{% endblock %}");
    blogc_error_free(err);
}


static void
test_template_parse_invalid_variable_name2(void **state)
{
    const char *a = "{% block entry %}{{ Bola }}{% endblock %}\n";
    blogc_error_t *err = NULL;
    sb_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(stmts);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Invalid variable name. Must be uppercase letter, number or '_'.\n"
        "Error occurred near line 1, position 22: "
        "{% block entry %}{{ Bola }}{% endblock %}");
    blogc_error_free(err);
}


static void
test_template_parse_invalid_variable_name3(void **state)
{
    const char *a = "{% block entry %}{{ 0123 }}{% endblock %}\n";
    blogc_error_t *err = NULL;
    sb_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(stmts);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Invalid variable name. Must begin with uppercase letter.\n"
        "Error occurred near line 1, position 21: {% block entry %}{{ 0123 }}"
        "{% endblock %}");
    blogc_error_free(err);
}


static void
test_template_parse_invalid_variable_end(void **state)
{
    const char *a = "{% block entry %}{{ BOLA %}{% endblock %}\n";
    blogc_error_t *err = NULL;
    sb_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(stmts);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Invalid statement syntax. Must end with '}}'.\n"
        "Error occurred near line 1, position 26: "
        "{% block entry %}{{ BOLA %}{% endblock %}");
    blogc_error_free(err);
}


static void
test_template_parse_invalid_close(void **state)
{
    const char *a = "{% block entry %%\n";
    blogc_error_t *err = NULL;
    sb_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(stmts);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Invalid statement syntax. Must end with '}'.\n"
        "Error occurred near line 1, position 17: {% block entry %%");
    blogc_error_free(err);
}


static void
test_template_parse_invalid_close2(void **state)
{
    const char *a = "{% block entry %}{{ BOLA }%{% endblock %}\n";
    blogc_error_t *err = NULL;
    sb_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(stmts);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Invalid statement syntax. Must end with '}'.\n"
        "Error occurred near line 1, position 27: "
        "{% block entry %}{{ BOLA }%{% endblock %}");
    blogc_error_free(err);
}


static void
test_template_parse_invalid_if_not_closed(void **state)
{
    const char *a = "{% block entry %}{% ifdef BOLA %}{% endblock %}\n";
    blogc_error_t *err = NULL;
    sb_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(stmts);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg, "1 open 'ifdef' and/or 'ifndef' statements "
        "were not closed!");
    blogc_error_free(err);
}


static void
test_template_parse_invalid_block_not_closed(void **state)
{
    const char *a = "{% block entry %}\n";
    blogc_error_t *err = NULL;
    sb_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(stmts);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg, "An open block was not closed!");
    blogc_error_free(err);
}


static void
test_template_parse_invalid_foreach_not_closed(void **state)
{
    const char *a = "{% foreach ASD %}\n";
    blogc_error_t *err = NULL;
    sb_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(stmts);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg, "An open 'foreach' statement was not closed!");
    blogc_error_free(err);
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
        unit_test(test_template_parse_invalid_endforeach_not_open),
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
        unit_test(test_template_parse_invalid_if_not_closed),
        unit_test(test_template_parse_invalid_block_not_closed),
        unit_test(test_template_parse_invalid_foreach_not_closed),
    };
    return run_tests(tests);
}
