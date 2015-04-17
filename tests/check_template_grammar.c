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

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "../src/template-grammar.h"


static void
blogc_assert_template_stmt(b_slist_t *l, const char *value,
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
test_template_parse(void **state)
{
    b_slist_t *stmts = blogc_template_parse(
        "Test\n"
        "\n"
        "    {% block single_source %}\n"
        "{% if CHUNDA %}\n"
        "bola\n"
        "{% else %}\n"
        "guda\n"
        "{% endif %}\n"
        "{% endblock %}\n"
        "{% block multiple_sources %}{{ BOLA }}{% endblock %}\n"
        "{% block multiple_sources_once %}asd{% endblock %}\n");
    assert_non_null(stmts);
    blogc_assert_template_stmt(stmts, "Test\n\n    ",
        BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(stmts->next, "single_source",
        BLOGC_TEMPLATE_BLOCK_STMT);
    blogc_assert_template_stmt(stmts->next->next, "\n",
        BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(stmts->next->next->next, "CHUNDA",
        BLOGC_TEMPLATE_IF_STMT);
    blogc_assert_template_stmt(stmts->next->next->next->next, "\nbola\n",
        BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(stmts->next->next->next->next->next, NULL,
        BLOGC_TEMPLATE_ELSE_STMT);
    blogc_assert_template_stmt(stmts->next->next->next->next->next->next,
        "\nguda\n", BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(stmts->next->next->next->next->next->next->next,
        NULL, BLOGC_TEMPLATE_ENDIF_STMT);
    blogc_assert_template_stmt(stmts->next->next->next->next->next->next->next->next,
        "\n", BLOGC_TEMPLATE_CONTENT_STMT);
    b_slist_t *tmp = stmts->next->next->next->next->next->next->next->next->next;
    blogc_assert_template_stmt(tmp, NULL, BLOGC_TEMPLATE_ENDBLOCK_STMT);
    blogc_assert_template_stmt(tmp->next, "\n", BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(tmp->next->next, "multiple_sources",
        BLOGC_TEMPLATE_BLOCK_STMT);
    blogc_assert_template_stmt(tmp->next->next->next, "BOLA",
        BLOGC_TEMPLATE_VARIABLE_STMT);
    blogc_assert_template_stmt(tmp->next->next->next->next, NULL,
        BLOGC_TEMPLATE_ENDBLOCK_STMT);
    blogc_assert_template_stmt(tmp->next->next->next->next->next, "\n",
        BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(tmp->next->next->next->next->next->next,
        "multiple_sources_once", BLOGC_TEMPLATE_BLOCK_STMT);
    blogc_assert_template_stmt(tmp->next->next->next->next->next->next->next,
        "asd", BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(tmp->next->next->next->next->next->next->next->next,
        NULL, BLOGC_TEMPLATE_ENDBLOCK_STMT);
    blogc_assert_template_stmt(tmp->next->next->next->next->next->next->next->next->next,
        "\n", BLOGC_TEMPLATE_CONTENT_STMT);
    assert_null(tmp->next->next->next->next->next->next->next->next->next->next);
    blogc_template_free_stmts(stmts);
}


static void
test_template_parse_html(void **state)
{
    b_slist_t *stmts = blogc_template_parse(
        "<html>\n"
        "    <head>\n"
        "        {% block single_source %}\n"
        "        <title>My cool blog >> {{ TITLE }}</title>\n"
        "        {% endblock %}\n"
        "        {% block multiple_sources %}\n"
        "        <title>My cool blog - Main page</title>\n"
        "        {% endblock %}\n"
        "    </head>\n"
        "    <body>\n"
        "        <h1>My cool blog</h1>\n"
        "        {% block single_source %}\n"
        "        <h2>{{ TITLE }}</h2>\n"
        "        {% if DATE %}<h4>Published in: {{ DATE }}</h4>{% endif %}\n"
        "        <pre>{{ CONTENT }}</pre>\n"
        "        {% endblock %}\n"
        "        {% block multiple_sources_once %}<ul>{% endblock %}\n"
        "        {% block multiple_sources %}<p><a href=\"{{ FILENAME }}.html\">"
        "{{ TITLE }}</a>{% if DATE %} - {{ DATE }}{% endif %}</p>{% endblock %}\n"
        "        {% block multiple_sources_once %}</ul>{% endblock %}\n"
        "    </body>\n"
        "</html>\n");
    assert_non_null(stmts);
    blogc_assert_template_stmt(stmts, "<html>\n    <head>\n        ",
        BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(stmts->next, "single_source",
        BLOGC_TEMPLATE_BLOCK_STMT);
    blogc_assert_template_stmt(stmts->next->next,
        "\n        <title>My cool blog >> ", BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(stmts->next->next->next, "TITLE",
        BLOGC_TEMPLATE_VARIABLE_STMT);
    blogc_assert_template_stmt(stmts->next->next->next->next,
        "</title>\n        ", BLOGC_TEMPLATE_CONTENT_STMT);





    blogc_template_free_stmts(stmts);
}


int
main(void)
{
    const UnitTest tests[] = {
        unit_test(test_template_parse),
        unit_test(test_template_parse_html),
    };
    return run_tests(tests);
}
