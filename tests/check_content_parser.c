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
#include "../src/content-parser.h"
#include "../src/error.h"
#include "../src/utils/utils.h"


static void
test_content_parse(void **state)
{
    const char *a =
        "# um\n"
        "## dois\n"
        "### tres\n"
        "#### quatro\n"
        "##### cinco\n"
        "###### seis\n"
        "\n"
        "bola\n"
        "chunda\n"
        "\n"
        ">  bola\n"
        ">  guda\n"
        ">  buga\n"
        ">  \n"
        ">    asd\n"
        "\n"
        "    bola\n"
        "     asd\n"
        "    qwewer\n"
        "\n"
        "+++\n"
        "1. chunda\n"
        "3. fuuuu\n"
        "\n"
        "+  chunda2\n"
        "+  fuuuu2\n"
        "\n"
        "<style>\n"
        "   chunda\n"
        "</style>\n"
        "\n"
        "guda\n"
        "yay";
    blogc_error_t *err = NULL;
    char *html = blogc_content_parse(a, strlen(a), &err);
    assert_null(err);
    assert_non_null(html);
    assert_string_equal(html,
        "<h1>um</h1>\n"
        "<h2>dois</h2>\n"
        "<h3>tres</h3>\n"
        "<h4>quatro</h4>\n"
        "<h5>cinco</h5>\n"
        "<h6>seis</h6>\n"
        "<p>bola\n"
        "chunda</p>\n"
        "<blockquote><p>bola\n"
        "guda\n"
        "buga</p>\n"
        "<pre><code>asd</code></pre>\n"
        "</blockquote>\n"
        "<pre><code>bola\n"
        " asd\n"
        "qwewer</code></pre>\n"
        "<hr />\n"
        "<ol>\n"
        "<li>chunda</li>\n"
        "<li>fuuuu</li>\n"
        "</ol>\n"
        "<ul>\n"
        "<li>chunda2</li>\n"
        "<li>fuuuu2</li>\n"
        "</ul>\n"
        "<style>\n"
        "   chunda\n"
        "</style>\n"
        "<p>guda\n"
        "yay</p>\n");
    free(html);
}


void
test_content_parse_header(void **state)
{
    const char *a = "## bola";
    blogc_error_t *err = NULL;
    char *html = blogc_content_parse(a, strlen(a), &err);
    assert_null(err);
    assert_non_null(html);
    assert_string_equal(html, "<h2>bola</h2>\n");
    free(html);
    a = "## bola\n";
    html = blogc_content_parse(a, strlen(a), &err);
    assert_null(err);
    assert_non_null(html);
    assert_string_equal(html, "<h2>bola</h2>\n");
    free(html);
    a =
        "bola\n"
        "\n"
        "## bola\n"
        "\n"
        "guda\n";
    html = blogc_content_parse(a, strlen(a), &err);
    assert_null(err);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>bola</p>\n"
        "<h2>bola</h2>\n"
        "<p>guda</p>\n");
    free(html);
}


void
test_content_parse_html(void **state)
{
    const char *a = "<div>\n</div>";
    blogc_error_t *err = NULL;
    char *html = blogc_content_parse(a, strlen(a), &err);
    assert_null(err);
    assert_non_null(html);
    assert_string_equal(html, "<div>\n</div>\n");
    free(html);
    a = "<div>\n</div>\n";
    html = blogc_content_parse(a, strlen(a), &err);
    assert_null(err);
    assert_non_null(html);
    assert_string_equal(html, "<div>\n</div>\n");
    free(html);
    a =
        "bola\n"
        "\n"
        "<div>\n"
        "</div>\n"
        "\n"
        "chunda\n";
    html = blogc_content_parse(a, strlen(a), &err);
    assert_null(err);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>bola</p>\n"
        "<div>\n</div>\n"
        "<p>chunda</p>\n");
    free(html);
}


void
test_content_parse_blockquote(void **state)
{
    const char *a = ">  bola\n>  guda";
    blogc_error_t *err = NULL;
    char *html = blogc_content_parse(a, strlen(a), &err);
    assert_null(err);
    assert_non_null(html);
    assert_string_equal(html,
        "<blockquote><p>bola\n"
        "guda</p>\n"
        "</blockquote>\n");
    free(html);
    a = ">  bola\n>  guda\n";
    html = blogc_content_parse(a, strlen(a), &err);
    assert_null(err);
    assert_non_null(html);
    assert_string_equal(html,
        "<blockquote><p>bola\n"
        "guda</p>\n"
        "</blockquote>\n");
    free(html);
    a =
        "bola\n"
        "\n"
        ">   bola\n"
        ">   guda\n"
        "\n"
        "chunda\n";
    html = blogc_content_parse(a, strlen(a), &err);
    assert_null(err);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>bola</p>\n"
        "<blockquote><p>bola\n"
        "guda</p>\n"
        "</blockquote>\n"
        "<p>chunda</p>\n");
    free(html);
}


void
test_content_parse_code(void **state)
{
    const char *a = "  bola\n  guda";
    blogc_error_t *err = NULL;
    char *html = blogc_content_parse(a, strlen(a), &err);
    assert_null(err);
    assert_non_null(html);
    assert_string_equal(html,
        "<pre><code>bola\n"
        "guda</code></pre>\n");
    free(html);
    a = "  bola\n  guda\n";
    html = blogc_content_parse(a, strlen(a), &err);
    assert_null(err);
    assert_non_null(html);
    assert_string_equal(html,
        "<pre><code>bola\n"
        "guda</code></pre>\n");
    free(html);
    a =
        "bola\n"
        "\n"
        "   bola\n"
        "   guda\n"
        "\n"
        "chunda\n";
    html = blogc_content_parse(a, strlen(a), &err);
    assert_null(err);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>bola</p>\n"
        "<pre><code>bola\n"
        "guda</code></pre>\n"
        "<p>chunda</p>\n");
    free(html);
}


void
test_content_parse_invalid_header(void **state)
{
    const char *a =
        "asd\n"
        "\n"
        "##bola\n";
    blogc_error_t *err = NULL;
    char *html = blogc_content_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(html);
    assert_int_equal(err->type, BLOGC_ERROR_CONTENT_PARSER);
    assert_string_equal(err->msg,
        "Malformed header, no space or tab after '#'\n"
        "Error occurred near to 'bola'");
    blogc_error_free(err);
}


void
test_content_parse_invalid_header_empty(void **state)
{
    const char *a =
        "asd\n"
        "\n"
        "##\n"
        "\n"
        "qwe\n";
    blogc_error_t *err = NULL;
    char *html = blogc_content_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(html);
    assert_int_equal(err->type, BLOGC_ERROR_CONTENT_PARSER);
    assert_string_equal(err->msg,
        "Malformed header, no space or tab after '#'");
    blogc_error_free(err);
}


void
test_content_parse_invalid_blockquote(void **state)
{
    const char *a =
        ">   asd\n"
        "> bola\n"
        ">   foo\n";
    blogc_error_t *err = NULL;
    char *html = blogc_content_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(html);
    assert_int_equal(err->type, BLOGC_ERROR_CONTENT_PARSER);
    assert_string_equal(err->msg,
        "Malformed blockquote, must use same prefix as previous line(s): >   ");
    blogc_error_free(err);
}


void
test_content_parse_invalid_code(void **state)
{
    const char *a =
        "    asd\n"
        "  bola\n"
        "    foo\n";
    blogc_error_t *err = NULL;
    char *html = blogc_content_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(html);
    assert_int_equal(err->type, BLOGC_ERROR_CONTENT_PARSER);
    assert_string_equal(err->msg,
        "Malformed code block, must use same prefix as previous line(s): '    '");
    blogc_error_free(err);
}


int
main(void)
{
    const UnitTest tests[] = {
        unit_test(test_content_parse),
        unit_test(test_content_parse_header),
        unit_test(test_content_parse_html),
        unit_test(test_content_parse_blockquote),
        unit_test(test_content_parse_code),
        unit_test(test_content_parse_invalid_header),
        unit_test(test_content_parse_invalid_header_empty),
        unit_test(test_content_parse_invalid_blockquote),
        unit_test(test_content_parse_invalid_code),
    };
    return run_tests(tests);
}
