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
#include "../src/utils/utils.h"


static void
test_content_parse(void **state)
{
    char *html = blogc_content_parse(
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
        ">  bola  \n"
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
        "yay\n"
        "\n"
        "**bola**\n");
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
        "<blockquote><p>bola  <br />\n"
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
        "yay</p>\n"
        "<p><strong>bola</strong></p>\n");
    free(html);
}


void
test_content_parse_header(void **state)
{
    char *html = blogc_content_parse("## bola");
    assert_non_null(html);
    assert_string_equal(html, "<h2>bola</h2>\n");
    free(html);
    html = blogc_content_parse("## bola\n");
    assert_non_null(html);
    assert_string_equal(html, "<h2>bola</h2>\n");
    free(html);
    html = blogc_content_parse(
        "bola\n"
        "\n"
        "## bola\n"
        "\n"
        "guda\n");
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
    char *html = blogc_content_parse("<div>\n</div>");
    assert_non_null(html);
    assert_string_equal(html, "<div>\n</div>\n");
    free(html);
    html = blogc_content_parse("<div>\n</div>\n");
    assert_non_null(html);
    assert_string_equal(html, "<div>\n</div>\n");
    free(html);
    html = blogc_content_parse(
        "bola\n"
        "\n"
        "<div>\n"
        "</div>\n"
        "\n"
        "chunda\n");
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
    char *html = blogc_content_parse(">  bola\n>  guda");
    assert_non_null(html);
    assert_string_equal(html,
        "<blockquote><p>bola\n"
        "guda</p>\n"
        "</blockquote>\n");
    free(html);
    html = blogc_content_parse(">  bola\n>  guda\n");
    assert_non_null(html);
    assert_string_equal(html,
        "<blockquote><p>bola\n"
        "guda</p>\n"
        "</blockquote>\n");
    free(html);
    html = blogc_content_parse(
        "bola\n"
        "\n"
        ">   bola\n"
        ">   guda\n"
        "\n"
        "chunda\n");
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
    char *html = blogc_content_parse("  bola\n  guda");
    assert_non_null(html);
    assert_string_equal(html,
        "<pre><code>bola\n"
        "guda</code></pre>\n");
    free(html);
    html = blogc_content_parse("  bola\n  guda\n");
    assert_non_null(html);
    assert_string_equal(html,
        "<pre><code>bola\n"
        "guda</code></pre>\n");
    free(html);
    html = blogc_content_parse(
        "bola\n"
        "\n"
        "   bola\n"
        "   guda\n"
        "\n"
        "chunda\n");
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
    char *html = blogc_content_parse(
        "asd\n"
        "\n"
        "##bola\n");
    assert_non_null(html);
    assert_string_equal(html,
        "<p>asd</p>\n"
        "<p>##bola</p>\n");
    free(html);
}


void
test_content_parse_invalid_header_empty(void **state)
{
    char *html = blogc_content_parse(
        "asd\n"
        "\n"
        "##\n"
        "\n"
        "qwe\n");
    assert_non_null(html);
    assert_string_equal(html,
        "<p>asd</p>\n"
        "<p>##\n"
        "\n"
        "qwe</p>\n");
    free(html);
}


void
test_content_parse_invalid_blockquote(void **state)
{
    char *html = blogc_content_parse(
        ">   asd\n"
        "> bola\n"
        ">   foo\n");
    assert_non_null(html);
    assert_string_equal(html,
        "<p>&gt;   asd\n"
        "&gt; bola\n"
        "&gt;   foo</p>\n");
    free(html);
}


void
test_content_parse_invalid_code(void **state)
{
    char *html = blogc_content_parse(
        "    asd\n"
        "  bola\n"
        "    foo\n");
    assert_non_null(html);
    assert_string_equal(html,
        "<p>    asd\n"
        "  bola\n"
        "    foo</p>\n");
    free(html);
}


void
test_content_parse_inline(void **state)
{
    char *html = blogc_content_parse_inline(
        "**bola***asd* [![lol](http://google.com/lol.png) **lol** "
        "\\[asd\\]\\(qwe\\)](http://google.com) ``chunda``");
    assert_string_equal(html,
        "<strong>bola</strong><em>asd</em> "
        "<a href=\"http://google.com\"><img src=\"http://google.com/lol.png\" "
        "alt=\"lol\"> <strong>lol</strong> [asd](qwe)</a> "
        "<code>chunda</code>");
    free(html);
    html = blogc_content_parse_inline("*bola*");
    assert_string_equal(html, "<em>bola</em>");
    free(html);
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
        unit_test(test_content_parse_inline),
    };
    return run_tests(tests);
}
