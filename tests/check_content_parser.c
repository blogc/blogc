/*
 * blogc: A blog compiler.
 * Copyright (C) 2015 Rafael G. Martins <rafael@rafaelmartins.eng.br>
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
#include "../src/content-parser.h"
#include "../src/utils/utils.h"


static void
test_content_parse(void **state)
{
    size_t l = 0;
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
        "**bola**\n", &l);
    assert_non_null(html);
    assert_int_equal(l, 0);
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


static void
test_content_parse_with_excerpt(void **state)
{
    size_t l = 0;
    char *html = blogc_content_parse(
        "# test\n"
        "\n"
        "chunda\n"
        "\n"
        "..\n"
        "\n"
        "guda\n"
        "lol", &l);
    assert_non_null(html);
    assert_int_equal(l, 28);
    assert_string_equal(html,
        "<h1>test</h1>\n"
        "<p>chunda</p>\n"
        "<p>guda\n"
        "lol</p>\n");
    free(html);
    l = 0;
    html = blogc_content_parse(
        "# test\n"
        "\n"
        "chunda\n"
        "\n"
        "...\n"
        "\n"
        "guda\n"
        "lol", &l);
    assert_non_null(html);
    assert_int_equal(l, 28);
    assert_string_equal(html,
        "<h1>test</h1>\n"
        "<p>chunda</p>\n"
        "<p>guda\n"
        "lol</p>\n");
    free(html);
}


static void
test_content_parse_header(void **state)
{
    char *html = blogc_content_parse("## bola", NULL);
    assert_non_null(html);
    assert_string_equal(html, "<h2>bola</h2>\n");
    free(html);
    html = blogc_content_parse("## bola\n", NULL);
    assert_non_null(html);
    assert_string_equal(html, "<h2>bola</h2>\n");
    free(html);
    html = blogc_content_parse(
        "bola\n"
        "\n"
        "## bola\n"
        "\n"
        "guda\n", NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>bola</p>\n"
        "<h2>bola</h2>\n"
        "<p>guda</p>\n");
    free(html);
}


static void
test_content_parse_html(void **state)
{
    char *html = blogc_content_parse("<div>\n</div>", NULL);
    assert_non_null(html);
    assert_string_equal(html, "<div>\n</div>\n");
    free(html);
    html = blogc_content_parse("<div>\n</div>\n", NULL);
    assert_non_null(html);
    assert_string_equal(html, "<div>\n</div>\n");
    free(html);
    html = blogc_content_parse(
        "bola\n"
        "\n"
        "<div>\n"
        "</div>\n"
        "\n"
        "chunda\n", NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>bola</p>\n"
        "<div>\n</div>\n"
        "<p>chunda</p>\n");
    free(html);
}


static void
test_content_parse_blockquote(void **state)
{
    char *html = blogc_content_parse(">  bola\n>  guda", NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<blockquote><p>bola\n"
        "guda</p>\n"
        "</blockquote>\n");
    free(html);
    html = blogc_content_parse(">  bola\n>  guda\n", NULL);
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
        "chunda\n", NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>bola</p>\n"
        "<blockquote><p>bola\n"
        "guda</p>\n"
        "</blockquote>\n"
        "<p>chunda</p>\n");
    free(html);
}


static void
test_content_parse_code(void **state)
{
    char *html = blogc_content_parse("  bola\n  guda", NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<pre><code>bola\n"
        "guda</code></pre>\n");
    free(html);
    html = blogc_content_parse("  bola\n  guda\n", NULL);
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
        "chunda\n", NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>bola</p>\n"
        "<pre><code>bola\n"
        "guda</code></pre>\n"
        "<p>chunda</p>\n");
    free(html);
}


static void
test_content_parse_horizontal_rule(void **state)
{
    char *html = blogc_content_parse("bola\nguda\n\n**", NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>bola\n"
        "guda</p>\n"
        "<hr />\n");
    free(html);
    html = blogc_content_parse("bola\nguda\n\n++++", NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>bola\n"
        "guda</p>\n"
        "<hr />\n");
    free(html);
    html = blogc_content_parse("bola\nguda\n\n--\n", NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>bola\n"
        "guda</p>\n"
        "<hr />\n");
    free(html);
    html = blogc_content_parse("bola\nguda\n\n****\n", NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>bola\n"
        "guda</p>\n"
        "<hr />\n");
    free(html);
    html = blogc_content_parse(
        "bola\n"
        "\n"
        "**\n"
        "\n"
        "chunda\n", NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>bola</p>\n"
        "<hr />\n"
        "<p>chunda</p>\n");
    free(html);
    html = blogc_content_parse(
        "bola\n"
        "\n"
        "----\n"
        "\n"
        "chunda\n", NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>bola</p>\n"
        "<hr />\n"
        "<p>chunda</p>\n");
    free(html);
}


static void
test_content_parse_unordered_list(void **state)
{
    char *html = blogc_content_parse(
        "lol\n"
        "\n"
        "*  asd\n"
        "*  qwe\n"
        "*  zxc", NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>lol</p>\n"
        "<ul>\n"
        "<li>asd</li>\n"
        "<li>qwe</li>\n"
        "<li>zxc</li>\n"
        "</ul>\n");
    free(html);
    html = blogc_content_parse(
        "lol\n"
        "\n"
        "*  asd\n"
        "*  qwe\n"
        "*  zxc\n", NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>lol</p>\n"
        "<ul>\n"
        "<li>asd</li>\n"
        "<li>qwe</li>\n"
        "<li>zxc</li>\n"
        "</ul>\n");
    free(html);
    html = blogc_content_parse(
        "lol\n"
        "\n"
        "*  asd\n"
        "*  qwe\n"
        "*  zxc\n"
        "\n"
        "fuuuu\n", NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>lol</p>\n"
        "<ul>\n"
        "<li>asd</li>\n"
        "<li>qwe</li>\n"
        "<li>zxc</li>\n"
        "</ul>\n"
        "<p>fuuuu</p>\n");
    free(html);
}


static void
test_content_parse_ordered_list(void **state)
{
    char *html = blogc_content_parse(
        "lol\n"
        "\n"
        "1.  asd\n"
        "2.  qwe\n"
        "3.  zxc", NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>lol</p>\n"
        "<ol>\n"
        "<li>asd</li>\n"
        "<li>qwe</li>\n"
        "<li>zxc</li>\n"
        "</ol>\n");
    free(html);
    html = blogc_content_parse(
        "lol\n"
        "\n"
        "1.  asd\n"
        "2.  qwe\n"
        "3.  zxc\n", NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>lol</p>\n"
        "<ol>\n"
        "<li>asd</li>\n"
        "<li>qwe</li>\n"
        "<li>zxc</li>\n"
        "</ol>\n");
    free(html);
    html = blogc_content_parse(
        "lol\n"
        "\n"
        "1.  asd\n"
        "2.  qwe\n"
        "3.  zxc\n"
        "\n"
        "fuuuu\n", NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>lol</p>\n"
        "<ol>\n"
        "<li>asd</li>\n"
        "<li>qwe</li>\n"
        "<li>zxc</li>\n"
        "</ol>\n"
        "<p>fuuuu</p>\n");
    free(html);
    html = blogc_content_parse(
        "1.\nasd\n"
        "2. qwe\n", NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>1.\n"
        "asd</p>\n"
        "<ol>\n"
        "<li>qwe</li>\n"
        "</ol>\n");
    free(html);
    html = blogc_content_parse("1.\n", NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<ol>\n"
        "<li></li>\n"
        "</ol>\n");
    free(html);
}


static void
test_content_parse_invalid_excerpt(void **state)
{
    size_t l = 0;
    char *html = blogc_content_parse(
        "# test\n"
        "\n"
        "chunda\n"
        "..\n"
        "\n"
        "guda\n"
        "lol", &l);
    assert_non_null(html);
    assert_int_equal(l, 0);
    assert_string_equal(html,
        "<h1>test</h1>\n"
        "<p>chunda\n"
        "..</p>\n"
        "<p>guda\n"
        "lol</p>\n");
    free(html);
    l = 0;
    html = blogc_content_parse(
        "# test\n"
        "\n"
        "chunda\n"
        "\n"
        "...\n"
        "guda\n"
        "lol", &l);
    assert_non_null(html);
    assert_int_equal(l, 0);
    assert_string_equal(html,
        "<h1>test</h1>\n"
        "<p>chunda</p>\n"
        "<p>...\n"
        "guda\n"
        "lol</p>\n");
    free(html);
    l = 0;
    html = blogc_content_parse(
        "# test\n"
        "\n"
        "chunda..\n"
        "\n"
        "guda\n"
        "lol", &l);
    assert_non_null(html);
    assert_int_equal(l, 0);
    assert_string_equal(html,
        "<h1>test</h1>\n"
        "<p>chunda..</p>\n"
        "<p>guda\n"
        "lol</p>\n");
    free(html);
    l = 0;
    html = blogc_content_parse(
        "# test\n"
        "\n"
        "chunda\n"
        "\n"
        "...guda\n"
        "lol", &l);
    assert_non_null(html);
    assert_int_equal(l, 0);
    assert_string_equal(html,
        "<h1>test</h1>\n"
        "<p>chunda</p>\n"
        "<p>...guda\n"
        "lol</p>\n");
    free(html);
}


static void
test_content_parse_invalid_header(void **state)
{
    char *html = blogc_content_parse(
        "asd\n"
        "\n"
        "##bola\n", NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>asd</p>\n"
        "<p>##bola</p>\n");
    free(html);
}


static void
test_content_parse_invalid_header_empty(void **state)
{
    char *html = blogc_content_parse(
        "asd\n"
        "\n"
        "##\n"
        "\n"
        "qwe\n", NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>asd</p>\n"
        "<p>##\n"
        "\n"
        "qwe</p>\n");
    free(html);
}


static void
test_content_parse_invalid_blockquote(void **state)
{
    char *html = blogc_content_parse(
        ">   asd\n"
        "> bola\n"
        ">   foo\n", NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>&gt;   asd\n"
        "&gt; bola\n"
        "&gt;   foo</p>\n");
    free(html);
}


static void
test_content_parse_invalid_code(void **state)
{
    char *html = blogc_content_parse(
        "    asd\n"
        "  bola\n"
        "    foo\n", NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>    asd\n"
        "  bola\n"
        "    foo</p>\n");
    free(html);
}


static void
test_content_parse_invalid_horizontal_rule(void **state)
{
    // this generates invalid html, but...
    char *html = blogc_content_parse("** asd", NULL);
    assert_non_null(html);
    assert_string_equal(html, "<p><strong> asd</p>\n");
    free(html);
    html = blogc_content_parse("** asd\n", NULL);
    assert_non_null(html);
    assert_string_equal(html, "<p><strong> asd</p>\n");
    free(html);
}


static void
test_content_parse_invalid_unordered_list(void **state)
{
    // more invalid html
    char *html = blogc_content_parse(
        "*  asd\n"
        "1. qwe", NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p><em>  asd\n"
        "1. qwe</p>\n");
    free(html);
    html = blogc_content_parse(
        "*  asd\n"
        "1. qwe\n"
        "\n", NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p><em>  asd\n"
        "1. qwe</p>\n");
    free(html);
    html = blogc_content_parse(
        "*  asd\n"
        "1. qwe\n", NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p><em>  asd\n"
        "1. qwe</p>\n");
    free(html);
    html = blogc_content_parse(
        "* asd\n"
        "1. qwe\n", NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p><em> asd\n"
        "1. qwe</p>\n");
    free(html);
    html = blogc_content_parse(
        "chunda\n"
        "\n"
        "* asd\n"
        "1. qwe\n"
        "\n"
        "poi\n", NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>chunda</p>\n"
        "<p><em> asd\n"
        "1. qwe</p>\n"
        "<p>poi</p>\n");
    free(html);
}


static void
test_content_parse_invalid_ordered_list(void **state)
{
    // more invalid html
    char *html = blogc_content_parse(
        "1. asd\n"
        "*  qwe", NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>1. asd\n"
        "<em>  qwe</p>\n");
    free(html);
    html = blogc_content_parse(
        "1. asd\n"
        "*  qwe\n"
        "\n", NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>1. asd\n"
        "<em>  qwe</p>\n");
    free(html);
    html = blogc_content_parse(
        "1. asd\n"
        "*  qwe\n", NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>1. asd\n"
        "<em>  qwe</p>\n");
    free(html);
    html = blogc_content_parse(
        "1. asd\n"
        "*  qwe\n", NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>1. asd\n"
        "<em>  qwe</p>\n");
    free(html);
    html = blogc_content_parse(
        "chunda\n"
        "\n"
        "1. asd\n"
        "*  qwe\n"
        "\n"
        "poi\n", NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>chunda</p>\n"
        "<p>1. asd\n"
        "<em>  qwe</p>\n"
        "<p>poi</p>\n");
    free(html);
    html = blogc_content_parse(
        "1 asd\n"
        "* qwe\n", NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>1 asd\n"
        "<em> qwe</p>\n");
    free(html);
    html = blogc_content_parse(
        "a. asd\n"
        "2. qwe\n", NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>a. asd\n"
        "2. qwe</p>\n");
    free(html);
}


static void
test_content_parse_inline(void **state)
{
    char *html = blogc_content_parse_inline(
        "**bola***asd* [![lol](http://google.com/lol.png) **lol** "
        "\\[asd\\]\\(qwe\\)](http://google.com) ``chunda`` [[bola]] chunda[9]");
    assert_non_null(html);
    assert_string_equal(html,
        "<strong>bola</strong><em>asd</em> "
        "<a href=\"http://google.com\"><img src=\"http://google.com/lol.png\" "
        "alt=\"lol\"> <strong>lol</strong> [asd](qwe)</a> "
        "<code>chunda</code> <a href=\"bola\">bola</a> chunda[9]");
    free(html);
    html = blogc_content_parse_inline("*bola*");
    assert_non_null(html);
    assert_string_equal(html, "<em>bola</em>");
    free(html);
}


static void
test_content_parse_inline_em(void **state)
{
    char *html = blogc_content_parse_inline("*bola*");
    assert_non_null(html);
    assert_string_equal(html, "<em>bola</em>");
    free(html);
    html = blogc_content_parse_inline("*bola*\n");
    assert_non_null(html);
    assert_string_equal(html, "<em>bola</em>\n");
    free(html);
    html = blogc_content_parse_inline("_bola_");
    assert_non_null(html);
    assert_string_equal(html, "<em>bola</em>");
    free(html);
    html = blogc_content_parse_inline("_bola_\n");
    assert_non_null(html);
    assert_string_equal(html, "<em>bola</em>\n");
    free(html);
    html = blogc_content_parse_inline("_**bola**_\n");
    assert_non_null(html);
    assert_string_equal(html, "<em><strong>bola</strong></em>\n");
    free(html);
    // this is not really valid
    html = blogc_content_parse_inline("_**bola\n");
    assert_non_null(html);
    assert_string_equal(html, "<em><strong>bola\n");
    free(html);
}


static void
test_content_parse_inline_strong(void **state)
{
    char *html = blogc_content_parse_inline("**bola**");
    assert_non_null(html);
    assert_string_equal(html, "<strong>bola</strong>");
    free(html);
    html = blogc_content_parse_inline("**bola**\n");
    assert_non_null(html);
    assert_string_equal(html, "<strong>bola</strong>\n");
    free(html);
    html = blogc_content_parse_inline("__bola__");
    assert_non_null(html);
    assert_string_equal(html, "<strong>bola</strong>");
    free(html);
    html = blogc_content_parse_inline("__bola__\n");
    assert_non_null(html);
    assert_string_equal(html, "<strong>bola</strong>\n");
    free(html);
    html = blogc_content_parse_inline("__*bola*__\n");
    assert_non_null(html);
    assert_string_equal(html, "<strong><em>bola</em></strong>\n");
    free(html);
    // this is not really valid
    html = blogc_content_parse_inline("__*bola\n");
    assert_non_null(html);
    assert_string_equal(html, "<strong><em>bola\n");
    free(html);
}


static void
test_content_parse_inline_code(void **state)
{
    char *html = blogc_content_parse_inline("``bola``");
    assert_non_null(html);
    assert_string_equal(html, "<code>bola</code>");
    free(html);
    html = blogc_content_parse_inline("``bola``\n");
    assert_non_null(html);
    assert_string_equal(html, "<code>bola</code>\n");
    free(html);
    html = blogc_content_parse_inline("`bola`");
    assert_non_null(html);
    assert_string_equal(html, "<code>bola</code>");
    free(html);
    html = blogc_content_parse_inline("`bola`\n");
    assert_non_null(html);
    assert_string_equal(html, "<code>bola</code>\n");
    free(html);
    html = blogc_content_parse_inline("``bo*la``\n");
    assert_non_null(html);
    assert_string_equal(html, "<code>bo*la</code>\n");
    free(html);
    // invalid
    html = blogc_content_parse_inline("``bola\n");
    assert_non_null(html);
    assert_string_equal(html, "<code>bola\n");
    free(html);
    html = blogc_content_parse_inline("`bola\n");
    assert_non_null(html);
    assert_string_equal(html, "<code>bola\n");
    free(html);
    html = blogc_content_parse_inline("``bola`\n");
    assert_non_null(html);
    assert_string_equal(html, "<code>bola<code>\n");
    free(html);
}


static void
test_content_parse_inline_link(void **state)
{
    char *html = blogc_content_parse_inline("[bola](http://example.org/)");
    assert_non_null(html);
    assert_string_equal(html, "<a href=\"http://example.org/\">bola</a>");
    free(html);
    html = blogc_content_parse_inline("[bola](http://example.org/)\n");
    assert_non_null(html);
    assert_string_equal(html, "<a href=\"http://example.org/\">bola</a>\n");
    free(html);
    html = blogc_content_parse_inline("[bola]\n(http://example.org/)\n");
    assert_non_null(html);
    assert_string_equal(html, "<a href=\"http://example.org/\">bola</a>\n");
    free(html);
    html = blogc_content_parse_inline("[bo\nla](http://example.org/)\n");
    assert_non_null(html);
    assert_string_equal(html, "<a href=\"http://example.org/\">bo\nla</a>\n");
    free(html);
    html = blogc_content_parse_inline("[``bola``](http://example.org/)\n");
    assert_non_null(html);
    assert_string_equal(html, "<a href=\"http://example.org/\"><code>bola</code></a>\n");
    free(html);
    html = blogc_content_parse_inline("[``bola(2)[3]**!\\``](http://example.org/)\n");
    assert_non_null(html);
    assert_string_equal(html, "<a href=\"http://example.org/\"><code>bola(2)[3]**!\\</code></a>\n");
    free(html);
    // "invalid"
    html = blogc_content_parse_inline("[bola](\nhttp://example.org/)\n");
    assert_non_null(html);
    assert_string_equal(html, "<a href=\"\nhttp://example.org/\">bola</a>\n");
    free(html);
    html = blogc_content_parse_inline("[bola](http://example.org/\n");
    assert_non_null(html);
    assert_string_equal(html, "[bola](http:&#x2F;&#x2F;example.org&#x2F;\n");
    free(html);
    html = blogc_content_parse_inline("[");
    assert_non_null(html);
    assert_string_equal(html, "[");
    free(html);
    html = blogc_content_parse_inline("[\n");
    assert_non_null(html);
    assert_string_equal(html, "[\n");
    free(html);
    html = blogc_content_parse_inline("[a");
    assert_non_null(html);
    assert_string_equal(html, "[a");
    free(html);
    html = blogc_content_parse_inline("[a\n");
    assert_non_null(html);
    assert_string_equal(html, "[a\n");
    free(html);
    html = blogc_content_parse_inline("[a]");
    assert_non_null(html);
    assert_string_equal(html, "[a]");
    free(html);
    html = blogc_content_parse_inline("[a]\n");
    assert_non_null(html);
    assert_string_equal(html, "[a]\n");
    free(html);
}


static void
test_content_parse_inline_link_auto(void **state)
{
    char *html = blogc_content_parse_inline("[[guda]]");
    assert_non_null(html);
    assert_string_equal(html, "<a href=\"guda\">guda</a>");
    free(html);
    html = blogc_content_parse_inline("[[guda]]\n");
    assert_non_null(html);
    assert_string_equal(html, "<a href=\"guda\">guda</a>\n");
    free(html);
    html = blogc_content_parse_inline("[[guda]asd]");
    assert_non_null(html);
    assert_string_equal(html, "<a href=\"guda\">guda</a>");
    free(html);
    html = blogc_content_parse_inline("[[guda]asd]\n");
    assert_non_null(html);
    assert_string_equal(html, "<a href=\"guda\">guda</a>\n");
    free(html);
    html = blogc_content_parse_inline("[[guda]asd");
    assert_non_null(html);
    assert_string_equal(html, "[[guda]asd");
    free(html);
    html = blogc_content_parse_inline("[[guda]asd\n");
    assert_non_null(html);
    assert_string_equal(html, "[[guda]asd\n");
    free(html);
}


static void
test_content_parse_inline_image(void **state)
{
    char *html = blogc_content_parse_inline("![bola](http://example.org/)");
    assert_non_null(html);
    assert_string_equal(html, "<img src=\"http://example.org/\" alt=\"bola\">");
    free(html);
    html = blogc_content_parse_inline("![bola](http://example.org/)\n");
    assert_non_null(html);
    assert_string_equal(html, "<img src=\"http://example.org/\" alt=\"bola\">\n");
    free(html);
    html = blogc_content_parse_inline("![bola]\n(http://example.org/)\n");
    assert_non_null(html);
    assert_string_equal(html, "<img src=\"http://example.org/\" alt=\"bola\">\n");
    free(html);
    // "invalid"
    html = blogc_content_parse_inline("![bo\nla](http://example.org/)\n");
    assert_non_null(html);
    assert_string_equal(html, "<img src=\"http://example.org/\" alt=\"bo\nla\">\n");
    free(html);
    html = blogc_content_parse_inline("![bola](\nhttp://example.org/)\n");
    assert_non_null(html);
    assert_string_equal(html, "<img src=\"\nhttp://example.org/\" alt=\"bola\">\n");
    free(html);
    html = blogc_content_parse_inline("![bola](http://example.org/\n");
    assert_non_null(html);
    assert_string_equal(html, "![bola](http:&#x2F;&#x2F;example.org&#x2F;\n");
    free(html);
    html = blogc_content_parse_inline("!");
    assert_non_null(html);
    assert_string_equal(html, "!");
    free(html);
    html = blogc_content_parse_inline("![");
    assert_non_null(html);
    assert_string_equal(html, "![");
    free(html);
    html = blogc_content_parse_inline("!\n");
    assert_non_null(html);
    assert_string_equal(html, "!\n");
    free(html);
    html = blogc_content_parse_inline("![\n");
    assert_non_null(html);
    assert_string_equal(html, "![\n");
    free(html);
    html = blogc_content_parse_inline("![a");
    assert_non_null(html);
    assert_string_equal(html, "![a");
    free(html);
    html = blogc_content_parse_inline("!a\n");
    assert_non_null(html);
    assert_string_equal(html, "!a\n");
    free(html);
    html = blogc_content_parse_inline("![a\n");
    assert_non_null(html);
    assert_string_equal(html, "![a\n");
    free(html);
    html = blogc_content_parse_inline("![a]");
    assert_non_null(html);
    assert_string_equal(html, "![a]");
    free(html);
    html = blogc_content_parse_inline("!a]\n");
    assert_non_null(html);
    assert_string_equal(html, "!a]\n");
    free(html);
    html = blogc_content_parse_inline("![a]\n");
    assert_non_null(html);
    assert_string_equal(html, "![a]\n");
    free(html);
}


static void
test_content_parse_inline_line_break(void **state)
{
    char *html = blogc_content_parse_inline("asd  \n");
    assert_non_null(html);
    assert_string_equal(html, "asd  <br />\n");
    free(html);
    html = blogc_content_parse_inline("asd  ");
    assert_non_null(html);
    assert_string_equal(html, "asd  <br />\n");
    free(html);
    html = blogc_content_parse_inline("asd   ");
    assert_non_null(html);
    assert_string_equal(html, "asd   <br />\n");
    free(html);
    // invalid
    html = blogc_content_parse_inline("asd ");
    assert_non_null(html);
    assert_string_equal(html, "asd ");
    free(html);
    html = blogc_content_parse_inline("asd \n");
    assert_non_null(html);
    assert_string_equal(html, "asd \n");
    free(html);
}


int
main(void)
{
    const UnitTest tests[] = {
        unit_test(test_content_parse),
        unit_test(test_content_parse_with_excerpt),
        unit_test(test_content_parse_header),
        unit_test(test_content_parse_html),
        unit_test(test_content_parse_blockquote),
        unit_test(test_content_parse_code),
        unit_test(test_content_parse_horizontal_rule),
        unit_test(test_content_parse_unordered_list),
        unit_test(test_content_parse_ordered_list),
        unit_test(test_content_parse_invalid_excerpt),
        unit_test(test_content_parse_invalid_header),
        unit_test(test_content_parse_invalid_header_empty),
        unit_test(test_content_parse_invalid_blockquote),
        unit_test(test_content_parse_invalid_code),
        unit_test(test_content_parse_invalid_horizontal_rule),
        unit_test(test_content_parse_invalid_unordered_list),
        unit_test(test_content_parse_invalid_ordered_list),
        unit_test(test_content_parse_inline),
        unit_test(test_content_parse_inline_em),
        unit_test(test_content_parse_inline_strong),
        unit_test(test_content_parse_inline_code),
        unit_test(test_content_parse_inline_link),
        unit_test(test_content_parse_inline_link_auto),
        unit_test(test_content_parse_inline_image),
        unit_test(test_content_parse_inline_line_break),
    };
    return run_tests(tests);
}
