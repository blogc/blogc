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


int
main(void)
{
    const UnitTest tests[] = {
        unit_test(test_template_parse),
    };
    return run_tests(tests);
}
