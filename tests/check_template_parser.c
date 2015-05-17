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
#include <string.h>
#include "../src/template-parser.h"
#include "../src/error.h"
#include "../src/utils/utils.h"


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
    const char *a =
        "Test\n"
        "\n"
        "    {% block entry %}\n"
        "{% ifdef CHUNDA %}\n"
        "bola\n"
        "{% endif %}\n"
        "{% ifndef BOLA %}\n"
        "bolao\n"
        "{% endif %}\n"
        "{% endblock %}\n"
        "{% block listing %}{{ BOLA }}{% endblock %}\n"
        "{% block listing_once %}asd{% endblock %}\n";
    blogc_error_t *err = NULL;
    b_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
    assert_null(err);
    assert_non_null(stmts);
    blogc_assert_template_stmt(stmts, "Test\n\n    ",
        BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(stmts->next, "entry",
        BLOGC_TEMPLATE_BLOCK_STMT);
    blogc_assert_template_stmt(stmts->next->next, "\n",
        BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(stmts->next->next->next, "CHUNDA",
        BLOGC_TEMPLATE_IFDEF_STMT);
    blogc_assert_template_stmt(stmts->next->next->next->next, "\nbola\n",
        BLOGC_TEMPLATE_CONTENT_STMT);
    blogc_assert_template_stmt(stmts->next->next->next->next->next, NULL,
        BLOGC_TEMPLATE_ENDIF_STMT);
    blogc_assert_template_stmt(stmts->next->next->next->next->next->next, "\n",
        BLOGC_TEMPLATE_CONTENT_STMT);
    b_slist_t *tmp = stmts->next->next->next->next->next->next->next;
    blogc_assert_template_stmt(tmp, "BOLA", BLOGC_TEMPLATE_IFNDEF_STMT);
    blogc_assert_template_stmt(tmp->next, "\nbolao\n", BLOGC_TEMPLATE_CONTENT_STMT);
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
        "\n", BLOGC_TEMPLATE_CONTENT_STMT);
    assert_null(tmp->next->next->next->next->next->next->next->next->next->next);
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
    b_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
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
    b_slist_t *tmp = stmts->next->next->next->next->next->next->next->next;
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
    b_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
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
    b_slist_t *tmp = stmts->next->next->next->next->next->next->next;
    blogc_assert_template_stmt(tmp, "CHUNDA", BLOGC_TEMPLATE_VARIABLE_STMT);
    blogc_assert_template_stmt(tmp->next, NULL, BLOGC_TEMPLATE_ENDIF_STMT);
    blogc_assert_template_stmt(tmp->next->next, "\n",
        BLOGC_TEMPLATE_CONTENT_STMT);
    assert_null(tmp->next->next->next);
    blogc_template_free_stmts(stmts);
}


static void
test_template_parse_invalid_block_start(void **state)
{
    const char *a = "{% ASD %}\n";
    blogc_error_t *err = NULL;
    b_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(stmts);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Invalid statement syntax. Must begin with lowercase letter.\n"
        "Error occurred near to 'ASD %}'");
    blogc_error_free(err);
}


static void
test_template_parse_invalid_block_nested(void **state)
{
    const char *a =
        "{% block entry %}\n"
        "{% block listing %}\n";
    blogc_error_t *err = NULL;
    b_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(stmts);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Blocks can't be nested.\n"
        "Error occurred near to ' listing %}'");
    blogc_error_free(err);
}


static void
test_template_parse_invalid_block_not_open(void **state)
{
    const char *a = "{% endblock %}\n";
    blogc_error_t *err = NULL;
    b_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(stmts);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "'endblock' statement without an open 'block' statement.\n"
        "Error occurred near to ' %}'");
    blogc_error_free(err);
}


static void
test_template_parse_invalid_endif_not_open(void **state)
{
    const char *a = "{% block listing %}{% endif %}{% endblock %}\n";
    blogc_error_t *err = NULL;
    b_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(stmts);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "'endif' statement without an open 'ifdef' or 'ifndef' statement.\n"
        "Error occurred near to ' %}{% endblock %}'");
    blogc_error_free(err);
}


static void
test_template_parse_invalid_block_name(void **state)
{
    const char *a = "{% chunda %}\n";
    blogc_error_t *err = NULL;
    b_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(stmts);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Invalid statement type: Allowed types are: 'block', 'endblock', 'ifdef', "
        "'ifndef' and 'endif'.\nError occurred near to ' %}'");
    blogc_error_free(err);
}


static void
test_template_parse_invalid_block_type_start(void **state)
{
    const char *a = "{% block ENTRY %}\n";
    blogc_error_t *err = NULL;
    b_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(stmts);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Invalid block syntax. Must begin with lowercase letter.\n"
        "Error occurred near to 'ENTRY %}'");
    blogc_error_free(err);
}


static void
test_template_parse_invalid_block_type(void **state)
{
    const char *a = "{% block chunda %}\n";
    blogc_error_t *err = NULL;
    b_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(stmts);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Invalid block type. Allowed types are: 'entry', 'listing' and 'listing_once'.\n"
        "Error occurred near to ' %}'");
    blogc_error_free(err);
}


static void
test_template_parse_invalid_ifdef_start(void **state)
{
    const char *a = "{% block entry %}{% ifdef guda %}\n";
    blogc_error_t *err = NULL;
    b_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(stmts);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Invalid variable name. Must begin with uppercase letter.\n"
        "Error occurred near to 'guda %}'");
    blogc_error_free(err);
}


static void
test_template_parse_invalid_ifdef_variable(void **state)
{
    const char *a = "{% block entry %}{% ifdef BoLA %}\n";
    blogc_error_t *err = NULL;
    b_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(stmts);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Invalid variable name. Must be uppercase letter, number or '_'.\n"
        "Error occurred near to 'oLA %}'");
    blogc_error_free(err);
}


static void
test_template_parse_invalid_block_end(void **state)
{
    const char *a = "{% block entry }}\n";
    blogc_error_t *err = NULL;
    b_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(stmts);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Invalid statement syntax. Must end with '%}'.\n"
        "Error occurred near to '}}'");
    blogc_error_free(err);
}


static void
test_template_parse_invalid_variable_name(void **state)
{
    const char *a = "{% block entry %}{{ bola }}{% endblock %}\n";
    blogc_error_t *err = NULL;
    b_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(stmts);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Invalid variable name. Must begin with uppercase letter.\n"
        "Error occurred near to 'bola }}{% endblock %}'");
    blogc_error_free(err);
}


static void
test_template_parse_invalid_variable_name2(void **state)
{
    const char *a = "{% block entry %}{{ Bola }}{% endblock %}\n";
    blogc_error_t *err = NULL;
    b_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(stmts);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Invalid variable name. Must be uppercase letter, number or '_'.\n"
        "Error occurred near to 'ola }}{% endblock %}'");
    blogc_error_free(err);
}


static void
test_template_parse_invalid_variable_end(void **state)
{
    const char *a = "{% block entry %}{{ BOLA %}{% endblock %}\n";
    blogc_error_t *err = NULL;
    b_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(stmts);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Invalid statement syntax. Must end with '}}'.\n"
        "Error occurred near to '%}{% endblock %}'");
    blogc_error_free(err);
}


static void
test_template_parse_invalid_close(void **state)
{
    const char *a = "{% block entry %%\n";
    blogc_error_t *err = NULL;
    b_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(stmts);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Invalid statement syntax. Must end with '}'.\n"
        "Error occurred near to '%'");
    blogc_error_free(err);
}


static void
test_template_parse_invalid_close2(void **state)
{
    const char *a = "{% block entry %}{{ BOLA }%{% endblock %}\n";
    blogc_error_t *err = NULL;
    b_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(stmts);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg,
        "Invalid statement syntax. Must end with '}'.\n"
        "Error occurred near to '%{% endblock %}'");
    blogc_error_free(err);
}


static void
test_template_parse_invalid_if_not_closed(void **state)
{
    const char *a = "{% block entry %}{% if BOLA %}{% endblock %}\n";
    blogc_error_t *err = NULL;
    b_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
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
    b_slist_t *stmts = blogc_template_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(stmts);
    assert_int_equal(err->type, BLOGC_ERROR_TEMPLATE_PARSER);
    assert_string_equal(err->msg, "An open block was not closed!");
    blogc_error_free(err);
}


int
main(void)
{
    const UnitTest tests[] = {
        unit_test(test_template_parse),
        unit_test(test_template_parse_html),
        unit_test(test_template_parse_ifdef_and_var_outside_block),
        unit_test(test_template_parse_invalid_block_start),
        unit_test(test_template_parse_invalid_block_nested),
        unit_test(test_template_parse_invalid_block_not_open),
        unit_test(test_template_parse_invalid_endif_not_open),
        unit_test(test_template_parse_invalid_block_name),
        unit_test(test_template_parse_invalid_block_type_start),
        unit_test(test_template_parse_invalid_block_type),
        unit_test(test_template_parse_invalid_ifdef_start),
        unit_test(test_template_parse_invalid_ifdef_variable),
        unit_test(test_template_parse_invalid_block_end),
        unit_test(test_template_parse_invalid_variable_name),
        unit_test(test_template_parse_invalid_variable_name2),
        unit_test(test_template_parse_invalid_variable_end),
        unit_test(test_template_parse_invalid_close),
        unit_test(test_template_parse_invalid_close2),
        unit_test(test_template_parse_invalid_if_not_closed),
        unit_test(test_template_parse_invalid_block_not_closed),
    };
    return run_tests(tests);
}
