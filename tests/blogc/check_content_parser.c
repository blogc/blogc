/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2020 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include "../../src/blogc/toctree.h"
#include "../../src/blogc/content-parser.h"


static void
test_slugify(void **state)
{
    char *s = blogc_slugify(NULL);
    assert_null(s);
    s = blogc_slugify("bola");
    assert_string_equal(s, "bola");
    free(s);
    s = blogc_slugify("bola o");
    assert_string_equal(s, "bola-o");
    free(s);
    s = blogc_slugify("Bola O");
    assert_string_equal(s, "bola-o");
    free(s);
    s = blogc_slugify("bola 0");
    assert_string_equal(s, "bola-0");
    free(s);
    s = blogc_slugify("bola  () o");
    assert_string_equal(s, "bola-----o");
    free(s);
    s = blogc_slugify("Bola O");
    assert_string_equal(s, "bola-o");
    free(s);
    s = blogc_slugify("bolaço");
    assert_string_equal(s, "bola--o");
    free(s);
}


static void
test_htmlentities(void **state)
{
    char *s = blogc_htmlentities(NULL);
    assert_null(s);
    s = blogc_htmlentities("asdxcv & < > \" 'sfd/gf");
    assert_string_equal(s, "asdxcv &amp; &lt; &gt; &quot; &#x27;sfd&#x2F;gf");
    free(s);
}


static void
test_fix_description(void **state)
{
    assert_null(blogc_fix_description(NULL));
    char *s = blogc_fix_description("bola");
    assert_string_equal(s, "bola");
    free(s);
    s = blogc_fix_description("bola\n");
    assert_string_equal(s, "bola");
    free(s);
    s = blogc_fix_description("bola\r\n");
    assert_string_equal(s, "bola");
    free(s);
    s = blogc_fix_description("bola\nguda");
    assert_string_equal(s, "bola guda");
    free(s);
    s = blogc_fix_description("bola\nguda\n");
    assert_string_equal(s, "bola guda");
    free(s);
    s = blogc_fix_description("bola\r\nguda\r\n");
    assert_string_equal(s, "bola guda");
    free(s);

    s = blogc_fix_description("bola\n   guda   lol\n asd");
    assert_string_equal(s, "bola guda   lol asd");
    free(s);
    s = blogc_fix_description("bola\n   guda   lol\n asd\n");
    assert_string_equal(s, "bola guda   lol asd");
    free(s);
    s = blogc_fix_description("bola\r\n   guda   lol\r\n asd\r\n");
    assert_string_equal(s, "bola guda   lol asd");
    free(s);
    s = blogc_fix_description("  bola\n   guda   lol\n asd");
    assert_string_equal(s, "bola guda   lol asd");
    free(s);
    s = blogc_fix_description("  bola\n   guda   lol\n asd\n");
    assert_string_equal(s, "bola guda   lol asd");
    free(s);
    s = blogc_fix_description("  bola\r\n   guda   lol\r\n asd\r\n");
    assert_string_equal(s, "bola guda   lol asd");
    free(s);
    s = blogc_fix_description("b'o\"l<>a");
    assert_string_equal(s, "b&#x27;o&quot;l&lt;&gt;a");
    free(s);
}


static void
test_is_ordered_list_item(void **state)
{
    assert_true(blogc_is_ordered_list_item("1.bola", 2));
    assert_true(blogc_is_ordered_list_item("1. bola", 3));
    assert_true(blogc_is_ordered_list_item("12. bola", 4));
    assert_true(blogc_is_ordered_list_item("123. bola", 5));
    assert_true(blogc_is_ordered_list_item("1.    bola", 6));
    assert_true(blogc_is_ordered_list_item("1.    bola", 5));
    assert_false(blogc_is_ordered_list_item("1bola", 1));
    assert_false(blogc_is_ordered_list_item("12bola", 2));
    assert_false(blogc_is_ordered_list_item("1 .   bola", 6));
    assert_false(blogc_is_ordered_list_item("1.   bola", 6));
    assert_false(blogc_is_ordered_list_item("1.", 2));
    assert_false(blogc_is_ordered_list_item(NULL, 2));
}


static void
test_content_parse(void **state)
{
    size_t l = 0;
    char *t = NULL;
    char *d = NULL;
    char *n = NULL;
    bc_slist_t *h = NULL;
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
        "    <asd>bola</asd>\n"
        "     asd\n"
        "    qwewer\n"
        "\n"
        "+++\n"
        "1. chunda\n"
        "3. fuuuu\n"
        "\n"
        "-  chunda2\n"
        "-  fuuuu2\n"
        "\n"
        "<style>\n"
        "   chunda\n"
        "</style>\n"
        "\n"
        "guda\n"
        "yay\n"
        "\n"
        "**bola**\n"
        "-- foo-bar\n"
        "--- bar\n"
        "\n"
        "-- asd\n"
        "\n"
        "--- lol\n", &l, &t, &d, &n, &h);
    assert_non_null(html);
    assert_int_equal(l, 0);
    assert_non_null(t);
    assert_string_equal(t, "um");
    assert_non_null(d);
    assert_string_equal(d, "bola chunda");
    assert_non_null(n);
    assert_string_equal(n, "\n");
    assert_non_null(h);
    assert_int_equal(bc_slist_length(h), 6);
    assert_string_equal(html,
        "<h1 id=\"um\">um</h1>\n"
        "<h2 id=\"dois\">dois</h2>\n"
        "<h3 id=\"tres\">tres</h3>\n"
        "<h4 id=\"quatro\">quatro</h4>\n"
        "<h5 id=\"cinco\">cinco</h5>\n"
        "<h6 id=\"seis\">seis</h6>\n"
        "<p>bola\n"
        "chunda</p>\n"
        "<blockquote><p>bola<br />\n"
        "guda\n"
        "buga</p>\n"
        "<pre><code>asd</code></pre>\n"
        "</blockquote>\n"
        "<pre><code>&lt;asd&gt;bola&lt;&#x2F;asd&gt;\n"
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
        "<p><strong>bola</strong>\n"
        "&ndash; foo-bar\n"
        "&mdash; bar</p>\n"
        "<p>&ndash; asd</p>\n"
        "<p>&mdash; lol</p>\n");
    free(html);
    free(t);
    free(d);
    free(n);
    blogc_toctree_free(h);
}


static void
test_content_parse_crlf(void **state)
{
    size_t l = 0;
    char *t = NULL;
    char *d = NULL;
    char *n = NULL;
    bc_slist_t *h = NULL;
    char *html = blogc_content_parse(
        "# um\r\n"
        "## dois\r\n"
        "### tres\r\n"
        "#### quatro\r\n"
        "##### cinco\r\n"
        "###### seis\r\n"
        "\r\n"
        "bola\r\n"
        "chunda\r\n"
        "\r\n"
        ">  bola  \r\n"
        ">  guda\r\n"
        ">  buga\r\n"
        ">  \r\n"
        ">    asd\r\n"
        "\r\n"
        "    <asd>bola</asd>\r\n"
        "     asd\r\n"
        "    qwewer\r\n"
        "\r\n"
        "+++\r\n"
        "1. chunda\r\n"
        "3. fuuuu\r\n"
        "\r\n"
        "+  chunda2\r\n"
        "+  fuuuu2\r\n"
        "\r\n"
        "<style>\r\n"
        "   chunda\r\n"
        "</style>\r\n"
        "\r\n"
        "guda\r\n"
        "yay\r\n"
        "\r\n"
        "**bola**\r\n"
        "-- foo-bar\r\n"
        "--- bar\r\n"
        "\r\n"
        "-- asd\r\n"
        "\r\n"
        "--- lol\r\n", &l, &t, &d, &n, &h);
    assert_non_null(html);
    assert_int_equal(l, 0);
    assert_non_null(t);
    assert_string_equal(t, "um");
    assert_non_null(d);
    assert_string_equal(d, "bola chunda");
    assert_non_null(n);
    assert_string_equal(n, "\r\n");
    assert_non_null(h);
    assert_int_equal(bc_slist_length(h), 6);
    assert_string_equal(html,
        "<h1 id=\"um\">um</h1>\r\n"
        "<h2 id=\"dois\">dois</h2>\r\n"
        "<h3 id=\"tres\">tres</h3>\r\n"
        "<h4 id=\"quatro\">quatro</h4>\r\n"
        "<h5 id=\"cinco\">cinco</h5>\r\n"
        "<h6 id=\"seis\">seis</h6>\r\n"
        "<p>bola\r\n"
        "chunda</p>\r\n"
        "<blockquote><p>bola<br />\r\n"
        "guda\r\n"
        "buga</p>\r\n"
        "<pre><code>asd</code></pre>\r\n"
        "</blockquote>\r\n"
        "<pre><code>&lt;asd&gt;bola&lt;&#x2F;asd&gt;\r\n"
        " asd\r\n"
        "qwewer</code></pre>\r\n"
        "<hr />\r\n"
        "<ol>\r\n"
        "<li>chunda</li>\r\n"
        "<li>fuuuu</li>\r\n"
        "</ol>\r\n"
        "<ul>\r\n"
        "<li>chunda2</li>\r\n"
        "<li>fuuuu2</li>\r\n"
        "</ul>\r\n"
        "<style>\r\n"
        "   chunda\r\n"
        "</style>\r\n"
        "<p>guda\r\n"
        "yay</p>\r\n"
        "<p><strong>bola</strong>\r\n"
        "&ndash; foo-bar\r\n"
        "&mdash; bar</p>\r\n"
        "<p>&ndash; asd</p>\r\n"
        "<p>&mdash; lol</p>\r\n");
    free(html);
    free(t);
    free(d);
    free(n);
    blogc_toctree_free(h);
}


static void
test_content_parse_with_excerpt(void **state)
{
    size_t l = 0;
    char *t = NULL;
    char *d = NULL;
    char *n = NULL;
    bc_slist_t *h = NULL;
    char *html = blogc_content_parse(
        "# test\n"
        "\n"
        "chunda\n"
        "\n"
        "..\n"
        "\n"
        "guda\n"
        "lol", &l, &t, &d, &n, &h);
    assert_non_null(html);
    assert_int_equal(l, 38);
    assert_non_null(t);
    assert_string_equal(t, "test");
    assert_non_null(d);
    assert_string_equal(d, "chunda");
    assert_non_null(n);
    assert_string_equal(n, "\n");
    assert_non_null(h);
    assert_int_equal(bc_slist_length(h), 1);
    assert_string_equal(html,
        "<h1 id=\"test\">test</h1>\n"
        "<p>chunda</p>\n"
        "<p>guda\n"
        "lol</p>\n");
    free(html);
    l = 0;
    free(t);
    free(d);
    free(n);
    blogc_toctree_free(h);
    t = NULL;
    d = NULL;
    n = NULL;
    h = NULL;
    html = blogc_content_parse(
        "# test\n"
        "\n"
        "chunda\n"
        "\n"
        "...\n"
        "\n"
        "guda\n"
        "lol", &l, &t, &d, &n, &h);
    assert_non_null(html);
    assert_int_equal(l, 38);
    assert_non_null(t);
    assert_string_equal(t, "test");
    assert_non_null(d);
    assert_string_equal(d, "chunda");
    assert_non_null(n);
    assert_string_equal(n, "\n");
    assert_non_null(h);
    assert_int_equal(bc_slist_length(h), 1);
    assert_string_equal(html,
        "<h1 id=\"test\">test</h1>\n"
        "<p>chunda</p>\n"
        "<p>guda\n"
        "lol</p>\n");
    free(html);
    free(t);
    free(d);
    free(n);
    blogc_toctree_free(h);
}


static void
test_content_parse_with_excerpt_crlf(void **state)
{
    size_t l = 0;
    char *t = NULL;
    char *d = NULL;
    char *n = NULL;
    bc_slist_t *h = NULL;
    char *html = blogc_content_parse(
        "# test\r\n"
        "\r\n"
        "chunda\r\n"
        "\r\n"
        "..\r\n"
        "\r\n"
        "guda\r\n"
        "lol", &l, &t, &d, &n, &h);
    assert_non_null(html);
    assert_int_equal(l, 40);
    assert_non_null(t);
    assert_string_equal(t, "test");
    assert_non_null(d);
    assert_string_equal(d, "chunda");
    assert_non_null(n);
    assert_string_equal(n, "\r\n");
    assert_non_null(h);
    assert_int_equal(bc_slist_length(h), 1);
    assert_string_equal(html,
        "<h1 id=\"test\">test</h1>\r\n"
        "<p>chunda</p>\r\n"
        "<p>guda\r\n"
        "lol</p>\r\n");
    free(html);
    l = 0;
    free(t);
    free(d);
    free(n);
    blogc_toctree_free(h);
    t = NULL;
    d = NULL;
    n = NULL;
    h = NULL;
    html = blogc_content_parse(
        "# test\r\n"
        "\r\n"
        "chunda\r\n"
        "\r\n"
        "...\r\n"
        "\r\n"
        "guda\r\n"
        "lol", &l, &t, &d, &n, &h);
    assert_non_null(html);
    assert_int_equal(l, 40);
    assert_non_null(t);
    assert_string_equal(t, "test");
    assert_non_null(d);
    assert_string_equal(d, "chunda");
    assert_non_null(n);
    assert_string_equal(n, "\r\n");
    assert_non_null(h);
    assert_int_equal(bc_slist_length(h), 1);
    assert_string_equal(html,
        "<h1 id=\"test\">test</h1>\r\n"
        "<p>chunda</p>\r\n"
        "<p>guda\r\n"
        "lol</p>\r\n");
    free(html);
    free(t);
    free(d);
    free(n);
    blogc_toctree_free(h);
}


static void
test_content_parse_header(void **state)
{
    char *html = blogc_content_parse("## bola", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html, "<h2 id=\"bola\">bola</h2>\n");
    free(html);
    html = blogc_content_parse("## bola\n", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html, "<h2 id=\"bola\">bola</h2>\n");
    free(html);
    html = blogc_content_parse(
        "bola\n"
        "\n"
        "## bola\n"
        "\n"
        "guda\n", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>bola</p>\n"
        "<h2 id=\"bola\">bola</h2>\n"
        "<p>guda</p>\n");
    free(html);
}


static void
test_content_parse_header_crlf(void **state)
{
    char *html = blogc_content_parse("## bola", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html, "<h2 id=\"bola\">bola</h2>\n");
    free(html);
    html = blogc_content_parse("## bola\r\n", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html, "<h2 id=\"bola\">bola</h2>\r\n");
    free(html);
    html = blogc_content_parse(
        "bola\r\n"
        "\r\n"
        "## bola\r\n"
        "\r\n"
        "guda\r\n", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>bola</p>\r\n"
        "<h2 id=\"bola\">bola</h2>\r\n"
        "<p>guda</p>\r\n");
    free(html);
}


static void
test_content_parse_html(void **state)
{
    char *html = blogc_content_parse("<div>\n</div>", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html, "<div>\n</div>\n");
    free(html);
    html = blogc_content_parse("<div>\n</div>\n", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html, "<div>\n</div>\n");
    free(html);
    html = blogc_content_parse(
        "bola\n"
        "\n"
        "<div>\n"
        "</div>\n"
        "\n"
        "chunda\n", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>bola</p>\n"
        "<div>\n</div>\n"
        "<p>chunda</p>\n");
    free(html);
}


static void
test_content_parse_html_crlf(void **state)
{
    char *html = blogc_content_parse("<div>\r\n</div>", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html, "<div>\r\n</div>\r\n");
    free(html);
    html = blogc_content_parse("<div>\r\n</div>\r\n", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html, "<div>\r\n</div>\r\n");
    free(html);
    html = blogc_content_parse(
        "bola\r\n"
        "\r\n"
        "<div>\r\n"
        "</div>\r\n"
        "\r\n"
        "chunda\r\n", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>bola</p>\r\n"
        "<div>\r\n</div>\r\n"
        "<p>chunda</p>\r\n");
    free(html);
}


static void
test_content_parse_blockquote(void **state)
{
    char *html = blogc_content_parse(">  bola\n>  guda", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<blockquote><p>bola\n"
        "guda</p>\n"
        "</blockquote>\n");
    free(html);
    html = blogc_content_parse(">  bola\n>  guda\n", NULL, NULL, NULL, NULL, NULL);
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
        "\n"
        "chunda\n", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>bola</p>\n"
        "<blockquote><p>bola</p>\n"
        "</blockquote>\n"
        "<p>chunda</p>\n");
    free(html);
    html = blogc_content_parse(
        "bola\n"
        "\n"
        ">   bola\n"
        ">   guda\n"
        "\n"
        "chunda\n", NULL, NULL, NULL, NULL, NULL);
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
test_content_parse_blockquote_crlf(void **state)
{
    char *html = blogc_content_parse(">  bola\r\n>  guda", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<blockquote><p>bola\r\n"
        "guda</p>\r\n"
        "</blockquote>\r\n");
    free(html);
    html = blogc_content_parse(">  bola\r\n>  guda\r\n", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<blockquote><p>bola\r\n"
        "guda</p>\r\n"
        "</blockquote>\r\n");
    free(html);
    html = blogc_content_parse(
        "bola\r\n"
        "\r\n"
        ">   bola\r\n"
        "\r\n"
        "chunda\r\n", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>bola</p>\r\n"
        "<blockquote><p>bola</p>\r\n"
        "</blockquote>\r\n"
        "<p>chunda</p>\r\n");
    free(html);
    html = blogc_content_parse(
        "bola\r\n"
        "\r\n"
        ">   bola\r\n"
        ">   guda\r\n"
        "\r\n"
        "chunda\r\n", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>bola</p>\r\n"
        "<blockquote><p>bola\r\n"
        "guda</p>\r\n"
        "</blockquote>\r\n"
        "<p>chunda</p>\r\n");
    free(html);
}


static void
test_content_parse_code(void **state)
{
    char *html = blogc_content_parse("  bola\n  guda", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<pre><code>bola\n"
        "guda</code></pre>\n");
    free(html);
    html = blogc_content_parse("  bola\n  guda\n", NULL, NULL, NULL, NULL, NULL);
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
        "chunda\n", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>bola</p>\n"
        "<pre><code>bola\n"
        "guda</code></pre>\n"
        "<p>chunda</p>\n");
    free(html);
}


static void
test_content_parse_code_crlf(void **state)
{
    char *html = blogc_content_parse("  bola\r\n  guda", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<pre><code>bola\r\n"
        "guda</code></pre>\r\n");
    free(html);
    html = blogc_content_parse("  bola\r\n  guda\r\n", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<pre><code>bola\r\n"
        "guda</code></pre>\r\n");
    free(html);
    html = blogc_content_parse(
        "bola\r\n"
        "\r\n"
        "   bola\r\n"
        "   guda\r\n"
        "\r\n"
        "chunda\r\n", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>bola</p>\r\n"
        "<pre><code>bola\r\n"
        "guda</code></pre>\r\n"
        "<p>chunda</p>\r\n");
    free(html);
}


static void
test_content_parse_horizontal_rule(void **state)
{
    char *html = blogc_content_parse("bola\nguda\n\n***", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>bola\n"
        "guda</p>\n"
        "<hr />\n");
    free(html);
    html = blogc_content_parse("bola\nguda\n\n++++", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>bola\n"
        "guda</p>\n"
        "<hr />\n");
    free(html);
    html = blogc_content_parse("bola\nguda\n\n---\n", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>bola\n"
        "guda</p>\n"
        "<hr />\n");
    free(html);
    html = blogc_content_parse("bola\nguda\n\n****\n", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>bola\n"
        "guda</p>\n"
        "<hr />\n");
    free(html);
    html = blogc_content_parse(
        "bola\n"
        "\n"
        "***\n"
        "\n"
        "chunda\n", NULL, NULL, NULL, NULL, NULL);
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
        "chunda\n", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>bola</p>\n"
        "<hr />\n"
        "<p>chunda</p>\n");
    free(html);
}


static void
test_content_parse_horizontal_rule_crlf(void **state)
{
    char *html = blogc_content_parse("bola\r\nguda\r\n\r\n***", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>bola\r\n"
        "guda</p>\r\n"
        "<hr />\r\n");
    free(html);
    html = blogc_content_parse("bola\r\nguda\r\n\r\n++++", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>bola\r\n"
        "guda</p>\r\n"
        "<hr />\r\n");
    free(html);
    html = blogc_content_parse("bola\r\nguda\r\n\r\n---\r\n", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>bola\r\n"
        "guda</p>\r\n"
        "<hr />\r\n");
    free(html);
    html = blogc_content_parse("bola\r\nguda\r\n\r\n****\r\n", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>bola\r\n"
        "guda</p>\r\n"
        "<hr />\r\n");
    free(html);
    html = blogc_content_parse(
        "bola\r\n"
        "\r\n"
        "***\r\n"
        "\r\n"
        "chunda\r\n", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>bola</p>\r\n"
        "<hr />\r\n"
        "<p>chunda</p>\r\n");
    free(html);
    html = blogc_content_parse(
        "bola\r\n"
        "\r\n"
        "----\r\n"
        "\r\n"
        "chunda\r\n", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>bola</p>\r\n"
        "<hr />\r\n"
        "<p>chunda</p>\r\n");
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
        "*  zxc", NULL, NULL, NULL, NULL, NULL);
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
        "*  zxc\n", NULL, NULL, NULL, NULL, NULL);
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
        "fuuuu\n", NULL, NULL, NULL, NULL, NULL);
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
    html = blogc_content_parse(
        "lol\n"
        "\n"
        "*  asd\n"
        "   cvb\n"
        "*  qwe\n"
        "*  zxc\n"
        "   1234\n"
        "\n"
        "fuuuu\n", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>lol</p>\n"
        "<ul>\n"
        "<li>asd\n"
        "cvb</li>\n"
        "<li>qwe</li>\n"
        "<li>zxc\n"
        "1234</li>\n"
        "</ul>\n"
        "<p>fuuuu</p>\n");
    free(html);
    html = blogc_content_parse(
        "*  asd\n"
        "*   qwe\n"
        "*    zxc", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<ul>\n"
        "<li>asd</li>\n"
        "<li> qwe</li>\n"
        "<li>  zxc</li>\n"
        "</ul>\n");
    free(html);
}


static void
test_content_parse_unordered_list_crlf(void **state)
{
    char *html = blogc_content_parse(
        "lol\r\n"
        "\r\n"
        "*  asd\r\n"
        "*  qwe\r\n"
        "*  zxc", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>lol</p>\r\n"
        "<ul>\r\n"
        "<li>asd</li>\r\n"
        "<li>qwe</li>\r\n"
        "<li>zxc</li>\r\n"
        "</ul>\r\n");
    free(html);
    html = blogc_content_parse(
        "lol\r\n"
        "\r\n"
        "*  asd\r\n"
        "*  qwe\r\n"
        "*  zxc\r\n", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>lol</p>\r\n"
        "<ul>\r\n"
        "<li>asd</li>\r\n"
        "<li>qwe</li>\r\n"
        "<li>zxc</li>\r\n"
        "</ul>\r\n");
    free(html);
    html = blogc_content_parse(
        "lol\r\n"
        "\r\n"
        "*  asd\r\n"
        "*  qwe\r\n"
        "*  zxc\r\n"
        "\r\n"
        "fuuuu\r\n", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>lol</p>\r\n"
        "<ul>\r\n"
        "<li>asd</li>\r\n"
        "<li>qwe</li>\r\n"
        "<li>zxc</li>\r\n"
        "</ul>\r\n"
        "<p>fuuuu</p>\r\n");
    free(html);
    html = blogc_content_parse(
        "lol\r\n"
        "\r\n"
        "*  asd\r\n"
        "   cvb\r\n"
        "*  qwe\r\n"
        "*  zxc\r\n"
        "   1234\r\n"
        "\r\n"
        "fuuuu\r\n", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>lol</p>\r\n"
        "<ul>\r\n"
        "<li>asd\r\n"
        "cvb</li>\r\n"
        "<li>qwe</li>\r\n"
        "<li>zxc\r\n"
        "1234</li>\r\n"
        "</ul>\r\n"
        "<p>fuuuu</p>\r\n");
    free(html);
    html = blogc_content_parse(
        "*  asd\r\n"
        "*   qwe\r\n"
        "*    zxc", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<ul>\r\n"
        "<li>asd</li>\r\n"
        "<li> qwe</li>\r\n"
        "<li>  zxc</li>\r\n"
        "</ul>\r\n");
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
        "3.  zxc", NULL, NULL, NULL, NULL, NULL);
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
        "3.  zxc\n", NULL, NULL, NULL, NULL, NULL);
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
        "fuuuu\n", NULL, NULL, NULL, NULL, NULL);
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
        "lol\n"
        "\n"
        "1.  asd\n"
        "    cvb\n"
        "2.  qwe\n"
        "3.  zxc\n"
        "    1234\n"
        "\n"
        "fuuuu\n", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>lol</p>\n"
        "<ol>\n"
        "<li>asd\n"
        "cvb</li>\n"
        "<li>qwe</li>\n"
        "<li>zxc\n"
        "1234</li>\n"
        "</ol>\n"
        "<p>fuuuu</p>\n");
    free(html);
    html = blogc_content_parse(
        "1.  asd\n"
        "2.   qwe\n"
        "3.    zxc", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<ol>\n"
        "<li>asd</li>\n"
        "<li> qwe</li>\n"
        "<li>  zxc</li>\n"
        "</ol>\n");
    free(html);
}


static void
test_content_parse_ordered_list_crlf(void **state)
{
    char *html = blogc_content_parse(
        "lol\r\n"
        "\r\n"
        "1.  asd\r\n"
        "2.  qwe\r\n"
        "3.  zxc", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>lol</p>\r\n"
        "<ol>\r\n"
        "<li>asd</li>\r\n"
        "<li>qwe</li>\r\n"
        "<li>zxc</li>\r\n"
        "</ol>\r\n");
    free(html);
    html = blogc_content_parse(
        "lol\r\n"
        "\r\n"
        "1.  asd\r\n"
        "2.  qwe\r\n"
        "3.  zxc\r\n", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>lol</p>\r\n"
        "<ol>\r\n"
        "<li>asd</li>\r\n"
        "<li>qwe</li>\r\n"
        "<li>zxc</li>\r\n"
        "</ol>\r\n");
    free(html);
    html = blogc_content_parse(
        "lol\r\n"
        "\r\n"
        "1.  asd\r\n"
        "2.  qwe\r\n"
        "3.  zxc\r\n"
        "\r\n"
        "fuuuu\r\n", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>lol</p>\r\n"
        "<ol>\r\n"
        "<li>asd</li>\r\n"
        "<li>qwe</li>\r\n"
        "<li>zxc</li>\r\n"
        "</ol>\r\n"
        "<p>fuuuu</p>\r\n");
    free(html);
    html = blogc_content_parse(
        "lol\r\n"
        "\r\n"
        "1.  asd\r\n"
        "    cvb\r\n"
        "2.  qwe\r\n"
        "3.  zxc\r\n"
        "    1234\r\n"
        "\r\n"
        "fuuuu\r\n", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>lol</p>\r\n"
        "<ol>\r\n"
        "<li>asd\r\n"
        "cvb</li>\r\n"
        "<li>qwe</li>\r\n"
        "<li>zxc\r\n"
        "1234</li>\r\n"
        "</ol>\r\n"
        "<p>fuuuu</p>\r\n");
    free(html);
    html = blogc_content_parse(
        "1.  asd\r\n"
        "2.   qwe\r\n"
        "3.    zxc", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<ol>\r\n"
        "<li>asd</li>\r\n"
        "<li> qwe</li>\r\n"
        "<li>  zxc</li>\r\n"
        "</ol>\r\n");
    free(html);
}


static void
test_content_parse_first_header(void **state)
{
    char *t = NULL;
    char *html = blogc_content_parse("# foo", NULL, &t, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html, "<h1 id=\"foo\">foo</h1>\n");
    assert_non_null(t);
    assert_string_equal(t, "foo");
    free(html);
    free(t);
    t = NULL;
    html = blogc_content_parse("# foo\n", NULL, &t, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html, "<h1 id=\"foo\">foo</h1>\n");
    assert_non_null(t);
    assert_string_equal(t, "foo");
    free(html);
    free(t);
    t = NULL;
    html = blogc_content_parse(
        "# foo\n"
        "## bar\n"
        "### baz", NULL, &t, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<h1 id=\"foo\">foo</h1>\n"
        "<h2 id=\"bar\">bar</h2>\n"
        "<h3 id=\"baz\">baz</h3>\n");
    assert_non_null(t);
    assert_string_equal(t, "foo");
    free(html);
    free(t);
    t = NULL;
    html = blogc_content_parse(
        "# foo\n"
        "## bar\n"
        "### baz\n", NULL, &t, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<h1 id=\"foo\">foo</h1>\n"
        "<h2 id=\"bar\">bar</h2>\n"
        "<h3 id=\"baz\">baz</h3>\n");
    assert_non_null(t);
    assert_string_equal(t, "foo");
    free(html);
    free(t);
    t = NULL;
    html = blogc_content_parse(
        "## bar\n"
        "# foo\n"
        "### baz", NULL, &t, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<h2 id=\"bar\">bar</h2>\n"
        "<h1 id=\"foo\">foo</h1>\n"
        "<h3 id=\"baz\">baz</h3>\n");
    assert_non_null(t);
    assert_string_equal(t, "bar");
    free(html);
    free(t);
    t = NULL;
    html = blogc_content_parse(
        "## bar\n"
        "# foo\n"
        "### baz\n", NULL, &t, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<h2 id=\"bar\">bar</h2>\n"
        "<h1 id=\"foo\">foo</h1>\n"
        "<h3 id=\"baz\">baz</h3>\n");
    assert_non_null(t);
    assert_string_equal(t, "bar");
    free(html);
    free(t);
}


static void
test_content_parse_first_header_crlf(void **state)
{
    char *t = NULL;
    char *html = blogc_content_parse("# foo\r\n", NULL, &t, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html, "<h1 id=\"foo\">foo</h1>\r\n");
    assert_non_null(t);
    assert_string_equal(t, "foo");
    free(html);
    free(t);
    t = NULL;
    html = blogc_content_parse(
        "# foo\r\n"
        "## bar\r\n"
        "### baz", NULL, &t, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<h1 id=\"foo\">foo</h1>\r\n"
        "<h2 id=\"bar\">bar</h2>\r\n"
        "<h3 id=\"baz\">baz</h3>\r\n");
    assert_non_null(t);
    assert_string_equal(t, "foo");
    free(html);
    free(t);
    t = NULL;
    html = blogc_content_parse(
        "# foo\r\n"
        "## bar\r\n"
        "### baz\r\n", NULL, &t, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<h1 id=\"foo\">foo</h1>\r\n"
        "<h2 id=\"bar\">bar</h2>\r\n"
        "<h3 id=\"baz\">baz</h3>\r\n");
    assert_non_null(t);
    assert_string_equal(t, "foo");
    free(html);
    free(t);
    t = NULL;
    html = blogc_content_parse(
        "## bar\r\n"
        "# foo\r\n"
        "### baz", NULL, &t, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<h2 id=\"bar\">bar</h2>\r\n"
        "<h1 id=\"foo\">foo</h1>\r\n"
        "<h3 id=\"baz\">baz</h3>\r\n");
    assert_non_null(t);
    assert_string_equal(t, "bar");
    free(html);
    free(t);
    t = NULL;
    html = blogc_content_parse(
        "## bar\r\n"
        "# foo\r\n"
        "### baz\r\n", NULL, &t, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<h2 id=\"bar\">bar</h2>\r\n"
        "<h1 id=\"foo\">foo</h1>\r\n"
        "<h3 id=\"baz\">baz</h3>\r\n");
    assert_non_null(t);
    assert_string_equal(t, "bar");
    free(html);
    free(t);
}


static void
test_content_parse_headers(void **state)
{
    bc_slist_t *h = NULL;
    char *html = blogc_content_parse("# foo", NULL, NULL, NULL, NULL, &h);
    assert_non_null(html);
    assert_string_equal(html, "<h1 id=\"foo\">foo</h1>\n");
    assert_non_null(h);
    assert_int_equal(((blogc_toctree_header_t*) h->data)->level, 1);
    assert_string_equal(((blogc_toctree_header_t*) h->data)->slug, "foo");
    assert_string_equal(((blogc_toctree_header_t*) h->data)->text, "foo");
    assert_null(h->next);
    blogc_toctree_free(h);
    h = NULL;
    free(html);
    html = blogc_content_parse("# foo\n", NULL, NULL, NULL, NULL, &h);
    assert_non_null(html);
    assert_string_equal(html, "<h1 id=\"foo\">foo</h1>\n");
    assert_non_null(h);
    assert_int_equal(((blogc_toctree_header_t*) h->data)->level, 1);
    assert_string_equal(((blogc_toctree_header_t*) h->data)->slug, "foo");
    assert_string_equal(((blogc_toctree_header_t*) h->data)->text, "foo");
    assert_null(h->next);
    blogc_toctree_free(h);
    h = NULL;
    free(html);
    html = blogc_content_parse(
        "# foo\n"
        "## bar\n"
        "### baz", NULL, NULL, NULL, NULL, &h);
    assert_non_null(html);
    assert_string_equal(html,
        "<h1 id=\"foo\">foo</h1>\n"
        "<h2 id=\"bar\">bar</h2>\n"
        "<h3 id=\"baz\">baz</h3>\n");
    assert_non_null(h);
    assert_int_equal(((blogc_toctree_header_t*) h->data)->level, 1);
    assert_string_equal(((blogc_toctree_header_t*) h->data)->slug, "foo");
    assert_string_equal(((blogc_toctree_header_t*) h->data)->text, "foo");
    assert_non_null(h->next);
    assert_int_equal(((blogc_toctree_header_t*) h->next->data)->level, 2);
    assert_string_equal(((blogc_toctree_header_t*) h->next->data)->slug, "bar");
    assert_string_equal(((blogc_toctree_header_t*) h->next->data)->text, "bar");
    assert_non_null(h);
    assert_int_equal(((blogc_toctree_header_t*) h->next->next->data)->level, 3);
    assert_string_equal(((blogc_toctree_header_t*) h->next->next->data)->slug, "baz");
    assert_string_equal(((blogc_toctree_header_t*) h->next->next->data)->text, "baz");
    assert_null(h->next->next->next);
    blogc_toctree_free(h);
    h = NULL;
    free(html);
    html = blogc_content_parse(
        "# foo\n"
        "## bar\n"
        "### baz\n", NULL, NULL, NULL, NULL, &h);
    assert_non_null(html);
    assert_string_equal(html,
        "<h1 id=\"foo\">foo</h1>\n"
        "<h2 id=\"bar\">bar</h2>\n"
        "<h3 id=\"baz\">baz</h3>\n");
    assert_non_null(h);
    assert_int_equal(((blogc_toctree_header_t*) h->data)->level, 1);
    assert_string_equal(((blogc_toctree_header_t*) h->data)->slug, "foo");
    assert_string_equal(((blogc_toctree_header_t*) h->data)->text, "foo");
    assert_non_null(h->next);
    assert_int_equal(((blogc_toctree_header_t*) h->next->data)->level, 2);
    assert_string_equal(((blogc_toctree_header_t*) h->next->data)->slug, "bar");
    assert_string_equal(((blogc_toctree_header_t*) h->next->data)->text, "bar");
    assert_non_null(h);
    assert_int_equal(((blogc_toctree_header_t*) h->next->next->data)->level, 3);
    assert_string_equal(((blogc_toctree_header_t*) h->next->next->data)->slug, "baz");
    assert_string_equal(((blogc_toctree_header_t*) h->next->next->data)->text, "baz");
    assert_null(h->next->next->next);
    blogc_toctree_free(h);
    h = NULL;
    free(html);
    html = blogc_content_parse(
        "## bar\n"
        "# foo\n"
        "### baz", NULL, NULL, NULL, NULL, &h);
    assert_non_null(html);
    assert_string_equal(html,
        "<h2 id=\"bar\">bar</h2>\n"
        "<h1 id=\"foo\">foo</h1>\n"
        "<h3 id=\"baz\">baz</h3>\n");
    assert_non_null(h);
    assert_int_equal(((blogc_toctree_header_t*) h->data)->level, 2);
    assert_string_equal(((blogc_toctree_header_t*) h->data)->slug, "bar");
    assert_string_equal(((blogc_toctree_header_t*) h->data)->text, "bar");
    assert_non_null(h->next);
    assert_int_equal(((blogc_toctree_header_t*) h->next->data)->level, 1);
    assert_string_equal(((blogc_toctree_header_t*) h->next->data)->slug, "foo");
    assert_string_equal(((blogc_toctree_header_t*) h->next->data)->text, "foo");
    assert_non_null(h);
    assert_int_equal(((blogc_toctree_header_t*) h->next->next->data)->level, 3);
    assert_string_equal(((blogc_toctree_header_t*) h->next->next->data)->slug, "baz");
    assert_string_equal(((blogc_toctree_header_t*) h->next->next->data)->text, "baz");
    assert_null(h->next->next->next);
    blogc_toctree_free(h);
    h = NULL;
    free(html);
    html = blogc_content_parse(
        "## bar\n"
        "# foo\n"
        "### baz\n", NULL, NULL, NULL, NULL, &h);
    assert_non_null(html);
    assert_string_equal(html,
        "<h2 id=\"bar\">bar</h2>\n"
        "<h1 id=\"foo\">foo</h1>\n"
        "<h3 id=\"baz\">baz</h3>\n");
    assert_non_null(h);
    assert_int_equal(((blogc_toctree_header_t*) h->data)->level, 2);
    assert_string_equal(((blogc_toctree_header_t*) h->data)->slug, "bar");
    assert_string_equal(((blogc_toctree_header_t*) h->data)->text, "bar");
    assert_non_null(h->next);
    assert_int_equal(((blogc_toctree_header_t*) h->next->data)->level, 1);
    assert_string_equal(((blogc_toctree_header_t*) h->next->data)->slug, "foo");
    assert_string_equal(((blogc_toctree_header_t*) h->next->data)->text, "foo");
    assert_non_null(h);
    assert_int_equal(((blogc_toctree_header_t*) h->next->next->data)->level, 3);
    assert_string_equal(((blogc_toctree_header_t*) h->next->next->data)->slug, "baz");
    assert_string_equal(((blogc_toctree_header_t*) h->next->next->data)->text, "baz");
    assert_null(h->next->next->next);
    blogc_toctree_free(h);
    free(html);
}


static void
test_content_parse_headers_crlf(void **state)
{
    bc_slist_t *h = NULL;
    char *html = blogc_content_parse("# foo\r\n", NULL, NULL, NULL, NULL, &h);
    assert_non_null(html);
    assert_string_equal(html, "<h1 id=\"foo\">foo</h1>\r\n");
    assert_non_null(h);
    assert_int_equal(((blogc_toctree_header_t*) h->data)->level, 1);
    assert_string_equal(((blogc_toctree_header_t*) h->data)->slug, "foo");
    assert_string_equal(((blogc_toctree_header_t*) h->data)->text, "foo");
    assert_null(h->next);
    blogc_toctree_free(h);
    h = NULL;
    free(html);
    html = blogc_content_parse(
        "# foo\r\n"
        "## bar\r\n"
        "### baz", NULL, NULL, NULL, NULL, &h);
    assert_non_null(html);
    assert_string_equal(html,
        "<h1 id=\"foo\">foo</h1>\r\n"
        "<h2 id=\"bar\">bar</h2>\r\n"
        "<h3 id=\"baz\">baz</h3>\r\n");
    assert_non_null(h);
    assert_int_equal(((blogc_toctree_header_t*) h->data)->level, 1);
    assert_string_equal(((blogc_toctree_header_t*) h->data)->slug, "foo");
    assert_string_equal(((blogc_toctree_header_t*) h->data)->text, "foo");
    assert_non_null(h->next);
    assert_int_equal(((blogc_toctree_header_t*) h->next->data)->level, 2);
    assert_string_equal(((blogc_toctree_header_t*) h->next->data)->slug, "bar");
    assert_string_equal(((blogc_toctree_header_t*) h->next->data)->text, "bar");
    assert_non_null(h);
    assert_int_equal(((blogc_toctree_header_t*) h->next->next->data)->level, 3);
    assert_string_equal(((blogc_toctree_header_t*) h->next->next->data)->slug, "baz");
    assert_string_equal(((blogc_toctree_header_t*) h->next->next->data)->text, "baz");
    assert_null(h->next->next->next);
    blogc_toctree_free(h);
    h = NULL;
    free(html);
    html = blogc_content_parse(
        "# foo\r\n"
        "## bar\r\n"
        "### baz\r\n", NULL, NULL, NULL, NULL, &h);
    assert_non_null(html);
    assert_string_equal(html,
        "<h1 id=\"foo\">foo</h1>\r\n"
        "<h2 id=\"bar\">bar</h2>\r\n"
        "<h3 id=\"baz\">baz</h3>\r\n");
    assert_non_null(h);
    assert_int_equal(((blogc_toctree_header_t*) h->data)->level, 1);
    assert_string_equal(((blogc_toctree_header_t*) h->data)->slug, "foo");
    assert_string_equal(((blogc_toctree_header_t*) h->data)->text, "foo");
    assert_non_null(h->next);
    assert_int_equal(((blogc_toctree_header_t*) h->next->data)->level, 2);
    assert_string_equal(((blogc_toctree_header_t*) h->next->data)->slug, "bar");
    assert_string_equal(((blogc_toctree_header_t*) h->next->data)->text, "bar");
    assert_non_null(h);
    assert_int_equal(((blogc_toctree_header_t*) h->next->next->data)->level, 3);
    assert_string_equal(((blogc_toctree_header_t*) h->next->next->data)->slug, "baz");
    assert_string_equal(((blogc_toctree_header_t*) h->next->next->data)->text, "baz");
    assert_null(h->next->next->next);
    blogc_toctree_free(h);
    h = NULL;
    free(html);
    html = blogc_content_parse(
        "## bar\r\n"
        "# foo\r\n"
        "### baz", NULL, NULL, NULL, NULL, &h);
    assert_non_null(html);
    assert_string_equal(html,
        "<h2 id=\"bar\">bar</h2>\r\n"
        "<h1 id=\"foo\">foo</h1>\r\n"
        "<h3 id=\"baz\">baz</h3>\r\n");
    assert_non_null(h);
    assert_int_equal(((blogc_toctree_header_t*) h->data)->level, 2);
    assert_string_equal(((blogc_toctree_header_t*) h->data)->slug, "bar");
    assert_string_equal(((blogc_toctree_header_t*) h->data)->text, "bar");
    assert_non_null(h->next);
    assert_int_equal(((blogc_toctree_header_t*) h->next->data)->level, 1);
    assert_string_equal(((blogc_toctree_header_t*) h->next->data)->slug, "foo");
    assert_string_equal(((blogc_toctree_header_t*) h->next->data)->text, "foo");
    assert_non_null(h);
    assert_int_equal(((blogc_toctree_header_t*) h->next->next->data)->level, 3);
    assert_string_equal(((blogc_toctree_header_t*) h->next->next->data)->slug, "baz");
    assert_string_equal(((blogc_toctree_header_t*) h->next->next->data)->text, "baz");
    assert_null(h->next->next->next);
    blogc_toctree_free(h);
    h = NULL;
    free(html);
    html = blogc_content_parse(
        "## bar\r\n"
        "# foo\r\n"
        "### baz\r\n", NULL, NULL, NULL, NULL, &h);
    assert_non_null(html);
    assert_string_equal(html,
        "<h2 id=\"bar\">bar</h2>\r\n"
        "<h1 id=\"foo\">foo</h1>\r\n"
        "<h3 id=\"baz\">baz</h3>\r\n");
    assert_non_null(h);
    assert_int_equal(((blogc_toctree_header_t*) h->data)->level, 2);
    assert_string_equal(((blogc_toctree_header_t*) h->data)->slug, "bar");
    assert_string_equal(((blogc_toctree_header_t*) h->data)->text, "bar");
    assert_non_null(h->next);
    assert_int_equal(((blogc_toctree_header_t*) h->next->data)->level, 1);
    assert_string_equal(((blogc_toctree_header_t*) h->next->data)->slug, "foo");
    assert_string_equal(((blogc_toctree_header_t*) h->next->data)->text, "foo");
    assert_non_null(h);
    assert_int_equal(((blogc_toctree_header_t*) h->next->next->data)->level, 3);
    assert_string_equal(((blogc_toctree_header_t*) h->next->next->data)->slug, "baz");
    assert_string_equal(((blogc_toctree_header_t*) h->next->next->data)->text, "baz");
    assert_null(h->next->next->next);
    blogc_toctree_free(h);
    free(html);
}


static void
test_content_parse_description(void **state)
{
    char *d = NULL;
    char *html = blogc_content_parse(
        "# foo\n"
        "\n"
        "bar", NULL, NULL, &d, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<h1 id=\"foo\">foo</h1>\n"
        "<p>bar</p>\n");
    assert_non_null(d);
    assert_string_equal(d, "bar");
    free(html);
    free(d);
    d = NULL;
    html = blogc_content_parse(
        "# foo\n"
        "\n"
        "bar\n", NULL, NULL, &d, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<h1 id=\"foo\">foo</h1>\n"
        "<p>bar</p>\n");
    assert_non_null(d);
    assert_string_equal(d, "bar");
    free(html);
    free(d);
    d = NULL;
    html = blogc_content_parse(
        "# foo\n"
        "\n"
        "qwe\n"
        "bar\n", NULL, NULL, &d, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<h1 id=\"foo\">foo</h1>\n"
        "<p>qwe\n"
        "bar</p>\n");
    assert_non_null(d);
    assert_string_equal(d, "qwe bar");
    free(html);
    free(d);
    d = NULL;
    html = blogc_content_parse(
        "# foo\n"
        "\n"
        "> qwe\n"
        "\n"
        "bar\n", NULL, NULL, &d, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<h1 id=\"foo\">foo</h1>\n"
        "<blockquote><p>qwe</p>\n"
        "</blockquote>\n"
        "<p>bar</p>\n");
    assert_non_null(d);
    assert_string_equal(d, "bar");
    free(html);
    free(d);
    d = NULL;
    html = blogc_content_parse(
        "# foo\n"
        "\n"
        "> qwe\n"
        "> zxc\n"
        "\n"
        "bar\n", NULL, NULL, &d, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<h1 id=\"foo\">foo</h1>\n"
        "<blockquote><p>qwe\n"
        "zxc</p>\n"
        "</blockquote>\n"
        "<p>bar</p>\n");
    assert_non_null(d);
    assert_string_equal(d, "bar");
    free(html);
    free(d);
}


static void
test_content_parse_description_crlf(void **state)
{
    char *d = NULL;
    char *html = blogc_content_parse(
        "# foo\r\n"
        "\r\n"
        "bar", NULL, NULL, &d, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<h1 id=\"foo\">foo</h1>\r\n"
        "<p>bar</p>\r\n");
    assert_non_null(d);
    assert_string_equal(d, "bar");
    free(html);
    free(d);
    d = NULL;
    html = blogc_content_parse(
        "# foo\r\n"
        "\r\n"
        "bar\r\n", NULL, NULL, &d, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<h1 id=\"foo\">foo</h1>\r\n"
        "<p>bar</p>\r\n");
    assert_non_null(d);
    assert_string_equal(d, "bar");
    free(html);
    free(d);
    d = NULL;
    html = blogc_content_parse(
        "# foo\r\n"
        "\r\n"
        "qwe\r\n"
        "bar\r\n", NULL, NULL, &d, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<h1 id=\"foo\">foo</h1>\r\n"
        "<p>qwe\r\n"
        "bar</p>\r\n");
    assert_non_null(d);
    assert_string_equal(d, "qwe bar");
    free(html);
    free(d);
    d = NULL;
    html = blogc_content_parse(
        "# foo\r\n"
        "\r\n"
        "> qwe\r\n"
        "\r\n"
        "bar\r\n", NULL, NULL, &d, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<h1 id=\"foo\">foo</h1>\r\n"
        "<blockquote><p>qwe</p>\r\n"
        "</blockquote>\r\n"
        "<p>bar</p>\r\n");
    assert_non_null(d);
    assert_string_equal(d, "bar");
    free(html);
    free(d);
    d = NULL;
    html = blogc_content_parse(
        "# foo\r\n"
        "\r\n"
        "> qwe\r\n"
        "> zxc\r\n"
        "\r\n"
        "bar\r\n", NULL, NULL, &d, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<h1 id=\"foo\">foo</h1>\r\n"
        "<blockquote><p>qwe\r\n"
        "zxc</p>\r\n"
        "</blockquote>\r\n"
        "<p>bar</p>\r\n");
    assert_non_null(d);
    assert_string_equal(d, "bar");
    free(html);
    free(d);
}


static void
test_content_parse_endl(void **state)
{
    char *n = NULL;
    char *html = blogc_content_parse("# test", NULL, NULL, NULL, &n, NULL);
    assert_non_null(html);
    assert_non_null(n);
    assert_string_equal(n, "\n");
    free(html);
    free(n);
    n = "\n";
    html = blogc_content_parse("# test", NULL, NULL, NULL, &n, NULL);
    assert_non_null(html);
    assert_non_null(n);
    assert_string_equal(n, "\n");
    free(html);
    n = NULL;
    html = blogc_content_parse(
        "# test\n", NULL, NULL, NULL, &n, NULL);
    assert_non_null(html);
    assert_non_null(n);
    assert_string_equal(n, "\n");
    free(html);
    free(n);
    n = NULL;
    html = blogc_content_parse(
        "# test\n"
        "foo", NULL, NULL, NULL, &n, NULL);
    assert_non_null(html);
    assert_non_null(n);
    assert_string_equal(n, "\n");
    free(html);
    free(n);
    n = NULL;
    html = blogc_content_parse(
        "# test\n"
        "foo\n", NULL, NULL, NULL, &n, NULL);
    assert_non_null(html);
    assert_non_null(n);
    assert_string_equal(n, "\n");
    free(html);
    free(n);
    n = NULL;
    html = blogc_content_parse(
        "# test\n"
        "foo\r\n", NULL, NULL, NULL, &n, NULL);
    assert_non_null(html);
    assert_non_null(n);
    assert_string_equal(n, "\n");
    free(html);
    free(n);
}


static void
test_content_parse_endl_crlf(void **state)
{
    char *n = "\r\n";
    char *html = blogc_content_parse("# test", NULL, NULL, NULL, &n, NULL);
    assert_non_null(html);
    assert_non_null(n);
    assert_string_equal(n, "\r\n");
    free(html);
    n = NULL;
    html = blogc_content_parse(
        "# test\r\n", NULL, NULL, NULL, &n, NULL);
    assert_non_null(html);
    assert_non_null(n);
    assert_string_equal(n, "\r\n");
    free(html);
    free(n);
    n = NULL;
    html = blogc_content_parse(
        "# test\r\n"
        "foo", NULL, NULL, NULL, &n, NULL);
    assert_non_null(html);
    assert_non_null(n);
    assert_string_equal(n, "\r\n");
    free(html);
    free(n);
    n = NULL;
    html = blogc_content_parse(
        "# test\r\n"
        "foo\r\n", NULL, NULL, NULL, &n, NULL);
    assert_non_null(html);
    assert_non_null(n);
    assert_string_equal(n, "\r\n");
    free(html);
    free(n);
    n = NULL;
    html = blogc_content_parse(
        "# test\r\n"
        "foo\n", NULL, NULL, NULL, &n, NULL);
    assert_non_null(html);
    assert_non_null(n);
    assert_string_equal(n, "\r\n");
    free(html);
    free(n);
}


static void
test_content_parse_invalid_excerpt(void **state)
{
    size_t l = 0;
    char *d = NULL;
    char *html = blogc_content_parse(
        "# test\n"
        "\n"
        "chunda\n"
        "..\n"
        "\n"
        "guda\n"
        "lol", &l, NULL, &d, NULL, NULL);
    assert_non_null(html);
    assert_int_equal(l, 0);
    assert_non_null(d);
    assert_string_equal(d, "chunda ..");
    assert_string_equal(html,
        "<h1 id=\"test\">test</h1>\n"
        "<p>chunda\n"
        "..</p>\n"
        "<p>guda\n"
        "lol</p>\n");
    free(html);
    l = 0;
    free(d);
    d = NULL;
    html = blogc_content_parse(
        "# test\n"
        "\n"
        "chunda\n"
        "\n"
        "...\n"
        "guda\n"
        "lol", &l, NULL, &d, NULL, NULL);
    assert_non_null(html);
    assert_int_equal(l, 0);
    assert_non_null(d);
    assert_string_equal(d, "chunda");
    assert_string_equal(html,
        "<h1 id=\"test\">test</h1>\n"
        "<p>chunda</p>\n"
        "<p>...\n"
        "guda\n"
        "lol</p>\n");
    free(html);
    l = 0;
    free(d);
    d = NULL;
    html = blogc_content_parse(
        "# test\n"
        "\n"
        "chunda..\n"
        "\n"
        "guda\n"
        "lol", &l, NULL, &d, NULL, NULL);
    assert_non_null(html);
    assert_int_equal(l, 0);
    assert_non_null(d);
    assert_string_equal(d, "chunda..");
    assert_string_equal(html,
        "<h1 id=\"test\">test</h1>\n"
        "<p>chunda..</p>\n"
        "<p>guda\n"
        "lol</p>\n");
    free(html);
    l = 0;
    free(d);
    d = NULL;
    html = blogc_content_parse(
        "# test\n"
        "\n"
        "chunda\n"
        "\n"
        "...guda\n"
        "lol", &l, NULL, &d, NULL, NULL);
    assert_non_null(html);
    assert_int_equal(l, 0);
    assert_non_null(d);
    assert_string_equal(d, "chunda");
    assert_string_equal(html,
        "<h1 id=\"test\">test</h1>\n"
        "<p>chunda</p>\n"
        "<p>...guda\n"
        "lol</p>\n");
    free(html);
    free(d);
}


static void
test_content_parse_invalid_header(void **state)
{
    char *html = blogc_content_parse(
        "asd\n"
        "\n"
        "##bola\n", NULL, NULL, NULL, NULL, NULL);
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
        "qwe\n", NULL, NULL, NULL, NULL, NULL);
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
        ">   foo\n", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>&gt;   asd\n"
        "&gt; bola\n"
        "&gt;   foo</p>\n");
    free(html);
    html = blogc_content_parse(
        ">   asd\n"
        "> bola", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>&gt;   asd\n"
        "&gt; bola</p>\n");
    free(html);
}


static void
test_content_parse_invalid_code(void **state)
{
    char *html = blogc_content_parse(
        "    asd\n"
        "  bola\n"
        "    foo\n", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>    asd\n"
        "  bola\n"
        "    foo</p>\n");
    free(html);
    html = blogc_content_parse(
        "    asd\n"
        "  bola\n"
        "    foo", NULL, NULL, NULL, NULL, NULL);
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
    char *html = blogc_content_parse("** asd", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html, "<p>** asd</p>\n");
    free(html);
    html = blogc_content_parse("** asd\n", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html, "<p>** asd</p>\n");
    free(html);
}


static void
test_content_parse_invalid_unordered_list(void **state)
{
    char *html = blogc_content_parse(
        "*  asd\n"
        "1. qwe", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>*  asd\n"
        "1. qwe</p>\n");
    free(html);
    html = blogc_content_parse(
        "*  asd\n"
        "1. qwe\n"
        "\n", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>*  asd\n"
        "1. qwe</p>\n");
    free(html);
    html = blogc_content_parse(
        "*  asd\n"
        "1. qwe\n", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>*  asd\n"
        "1. qwe"
        "</p>\n");
    free(html);
    html = blogc_content_parse(
        "* asd\n"
        "1. qwe\n", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>* asd\n"
        "1. qwe"
        "</p>\n");
    free(html);
    html = blogc_content_parse(
        "chunda\n"
        "\n"
        "* asd\n"
        "1. qwe\n"
        "\n"
        "poi\n", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>chunda</p>\n"
        "<p>* asd\n"
        "1. qwe</p>\n"
        "<p>poi</p>\n");
    free(html);
}


static void
test_content_parse_invalid_ordered_list(void **state)
{
    char *html = blogc_content_parse(
        "1. asd\n"
        "*  qwe", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>1. asd\n"
        "*  qwe</p>\n");
    free(html);
    html = blogc_content_parse(
        "1. asd\n"
        "*  qwe\n"
        "\n", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>1. asd\n"
        "*  qwe</p>\n");
    free(html);
    html = blogc_content_parse(
        "1. asd\n"
        "*  qwe\n", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>1. asd\n"
        "*  qwe"
        "</p>\n");
    free(html);
    html = blogc_content_parse(
        "1. asd\n"
        "*  qwe\n", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>1. asd\n"
        "*  qwe"
        "</p>\n");
    free(html);
    html = blogc_content_parse(
        "chunda\n"
        "\n"
        "1. asd\n"
        "*  qwe\n"
        "\n"
        "poi\n", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>chunda</p>\n"
        "<p>1. asd\n"
        "*  qwe</p>\n"
        "<p>poi</p>\n");
    free(html);
    html = blogc_content_parse(
        "1 asd\n"
        "* qwe\n", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>1 asd\n"
        "* qwe</p>\n");
    free(html);
    html = blogc_content_parse(
        "a. asd\n"
        "2. qwe\n", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>a. asd\n"
        "2. qwe</p>\n");
    free(html);
    html = blogc_content_parse(
        "1.\nasd\n"
        "2. qwe\n", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html,
        "<p>1.\n"
        "asd\n"
        "2. qwe</p>\n");
    free(html);
    html = blogc_content_parse("1.\n", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html, "<p>1.</p>\n");
    free(html);
    html = blogc_content_parse("1 ", NULL, NULL, NULL, NULL, NULL);
    assert_non_null(html);
    assert_string_equal(html, "<p>1 </p>\n");
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
    html = blogc_content_parse_inline("bola!");
    assert_non_null(html);
    assert_string_equal(html, "bola!");
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
    html = blogc_content_parse_inline("*bo\\*la*\n");
    assert_non_null(html);
    assert_string_equal(html, "<em>bo*la</em>\n");
    free(html);
    html = blogc_content_parse_inline("_bola_");
    assert_non_null(html);
    assert_string_equal(html, "<em>bola</em>");
    free(html);
    html = blogc_content_parse_inline("_bola_\n");
    assert_non_null(html);
    assert_string_equal(html, "<em>bola</em>\n");
    free(html);
    html = blogc_content_parse_inline("_bo\\_la_\n");
    assert_non_null(html);
    assert_string_equal(html, "<em>bo_la</em>\n");
    free(html);
    html = blogc_content_parse_inline("_**bola**_\n");
    assert_non_null(html);
    assert_string_equal(html, "<em><strong>bola</strong></em>\n");
    free(html);
    html = blogc_content_parse_inline("_**bo\\_\\*la**_\n");
    assert_non_null(html);
    assert_string_equal(html, "<em><strong>bo_*la</strong></em>\n");
    free(html);
    html = blogc_content_parse_inline("_**bola\n");
    assert_non_null(html);
    assert_string_equal(html, "_**bola\n");
    free(html);
    html = blogc_content_parse_inline("**_bola\\*\n");
    assert_non_null(html);
    assert_string_equal(html, "**_bola*\n");
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
    html = blogc_content_parse_inline("**bo\\*\\*la**\n");
    assert_non_null(html);
    assert_string_equal(html, "<strong>bo**la</strong>\n");
    free(html);
    html = blogc_content_parse_inline("__bola__");
    assert_non_null(html);
    assert_string_equal(html, "<strong>bola</strong>");
    free(html);
    html = blogc_content_parse_inline("__bola__\n");
    assert_non_null(html);
    assert_string_equal(html, "<strong>bola</strong>\n");
    free(html);
    html = blogc_content_parse_inline("__bo\\_\\_la__\n");
    assert_non_null(html);
    assert_string_equal(html, "<strong>bo__la</strong>\n");
    free(html);
    html = blogc_content_parse_inline("__*bola*__\n");
    assert_non_null(html);
    assert_string_equal(html, "<strong><em>bola</em></strong>\n");
    free(html);
    html = blogc_content_parse_inline("__*bo\\_\\*la*__\n");
    assert_non_null(html);
    assert_string_equal(html, "<strong><em>bo_*la</em></strong>\n");
    free(html);
    html = blogc_content_parse_inline("__*bola\n");
    assert_non_null(html);
    assert_string_equal(html, "__*bola\n");
    free(html);
    html = blogc_content_parse_inline("__*bola\\_\n");
    assert_non_null(html);
    assert_string_equal(html, "__*bola_\n");
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
    html = blogc_content_parse_inline("``bo<la``\n");
    assert_non_null(html);
    assert_string_equal(html, "<code>bo&lt;la</code>\n");
    free(html);
    html = blogc_content_parse_inline("`bo\\`\\`la`\n");
    assert_non_null(html);
    assert_string_equal(html, "<code>bo\\`\\`la</code>\n");
    free(html);
    html = blogc_content_parse_inline("``bo\\`\\`la``\n");
    assert_non_null(html);
    assert_string_equal(html, "<code>bo\\`\\`la</code>\n");
    free(html);
    html = blogc_content_parse_inline("``bo`la``\n");
    assert_non_null(html);
    assert_string_equal(html, "<code>bo`la</code>\n");
    free(html);
    html = blogc_content_parse_inline("``bola\n");
    assert_non_null(html);
    assert_string_equal(html, "``bola\n");
    free(html);
    html = blogc_content_parse_inline("`bola\n");
    assert_non_null(html);
    assert_string_equal(html, "`bola\n");
    free(html);
    html = blogc_content_parse_inline("``bola`\n");
    assert_non_null(html);
    assert_string_equal(html, "``bola`\n");
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
    html = blogc_content_parse_inline("[bola!](http://example.org/)\n");
    assert_non_null(html);
    assert_string_equal(html, "<a href=\"http://example.org/\">bola!</a>\n");
    free(html);
    html = blogc_content_parse_inline("[bola]\n(http://example.org/)\n");
    assert_non_null(html);
    assert_string_equal(html, "<a href=\"http://example.org/\">bola</a>\n");
    free(html);
    html = blogc_content_parse_inline("[bola]\r\n(http://example.org/)\n");
    assert_non_null(html);
    assert_string_equal(html, "<a href=\"http://example.org/\">bola</a>\n");
    free(html);
    html = blogc_content_parse_inline("[bola] \r\n (http://example.org/)\n");
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
    html = blogc_content_parse_inline("[``bola(2)[3]**!\\```](http://example.org/)\n");
    assert_non_null(html);
    assert_string_equal(html, "<a href=\"http://example.org/\"><code>bola(2)[3]**!\\`</code></a>\n");
    free(html);
    html = blogc_content_parse_inline("[``bola(2)[3]**!```](http://example.org/)\n");
    assert_non_null(html);
    assert_string_equal(html, "<a href=\"http://example.org/\"><code>bola(2)[3]**!</code>`</a>\n");
    free(html);
    html = blogc_content_parse_inline("test suite!)\n"
        "depends on [cmocka](http://cmocka.org/), though.\n");
    assert_non_null(html);
    assert_string_equal(html, "test suite!)\n"
        "depends on <a href=\"http://cmocka.org/\">cmocka</a>, though.\n");
    free(html);
    html = blogc_content_parse_inline("asd [bola]chunda(1234)");
    assert_non_null(html);
    assert_string_equal(html, "asd [bola]chunda(1234)");
    free(html);
    // "invalid"
    html = blogc_content_parse_inline("[bola](\nhttp://example.org/)\n");
    assert_non_null(html);
    assert_string_equal(html, "<a href=\"\nhttp://example.org/\">bola</a>\n");
    free(html);
    html = blogc_content_parse_inline("[bo[]\\[\\]()la](http://example.org/?\\(\\))\n");
    assert_non_null(html);
    assert_string_equal(html, "<a href=\"http://example.org/?()\">bo[][]()la</a>\n");
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
    html = blogc_content_parse_inline("[[http://example.org/?\\[\\]]]\n");
    assert_non_null(html);
    assert_string_equal(html, "<a href=\"http://example.org/?[]\">http://example.org/?[]</a>\n");
    free(html);
    html = blogc_content_parse_inline("[[http://example.org/?\\[\\]a]]\n");
    assert_non_null(html);
    assert_string_equal(html, "<a href=\"http://example.org/?[]a\">http://example.org/?[]a</a>\n");
    free(html);
    html = blogc_content_parse_inline("[[guda]asd]");
    assert_non_null(html);
    assert_string_equal(html, "[[guda]asd]");
    free(html);
    html = blogc_content_parse_inline("[[guda]asd]\n");
    assert_non_null(html);
    assert_string_equal(html, "[[guda]asd]\n");
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
    html = blogc_content_parse_inline("![bola]\r\n(http://example.org/)\n");
    assert_non_null(html);
    assert_string_equal(html, "<img src=\"http://example.org/\" alt=\"bola\">\n");
    free(html);
    html = blogc_content_parse_inline("![bola] \r\n (http://example.org/)\n");
    assert_non_null(html);
    assert_string_equal(html, "<img src=\"http://example.org/\" alt=\"bola\">\n");
    free(html);
    html = blogc_content_parse_inline(
        "[![This is the image alt text](picture.jpg)](https://blogc.rgm.io)");
    assert_non_null(html);
    assert_string_equal(html,
        "<a href=\"https://blogc.rgm.io\"><img src=\"picture.jpg\" "
        "alt=\"This is the image alt text\"></a>");
    free(html);
    html = blogc_content_parse_inline(
        "[![This is the image alt text]\n"
        "(picture.jpg)](https://blogc.rgm.io)");
    assert_non_null(html);
    assert_string_equal(html,
        "<a href=\"https://blogc.rgm.io\"><img src=\"picture.jpg\" "
        "alt=\"This is the image alt text\"></a>");
    free(html);
    html = blogc_content_parse_inline(
        "[![This is the image alt text]\n"
        "(picture.jpg)]\n"
        "(https://blogc.rgm.io)");
    assert_non_null(html);
    assert_string_equal(html,
        "<a href=\"https://blogc.rgm.io\"><img src=\"picture.jpg\" "
        "alt=\"This is the image alt text\"></a>");
    free(html);
    html = blogc_content_parse_inline("asd ![bola]chunda(1234)");
    assert_non_null(html);
    assert_string_equal(html, "asd ![bola]chunda(1234)");
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
    html = blogc_content_parse_inline("![bo\\[\\]()la](http://example.org/?\\(\\))\n");
    assert_non_null(html);
    assert_string_equal(html, "<img src=\"http://example.org/?()\" alt=\"bo[]()la\">\n");
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
    assert_string_equal(html, "asd<br />\n");
    free(html);
    html = blogc_content_parse_inline("asd  ");
    assert_non_null(html);
    assert_string_equal(html, "asd<br />");
    free(html);
    html = blogc_content_parse_inline("asd   ");
    assert_non_null(html);
    assert_string_equal(html, "asd<br />");
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
    html = blogc_content_parse_inline("asd  a\n");
    assert_non_null(html);
    assert_string_equal(html, "asd  a\n");
    free(html);
    html = blogc_content_parse_inline("asd    a\n");
    assert_non_null(html);
    assert_string_equal(html, "asd    a\n");
    free(html);
}


static void
test_content_parse_inline_line_break_crlf(void **state)
{
    char *html = blogc_content_parse_inline("asd  \r\n");
    assert_non_null(html);
    assert_string_equal(html, "asd<br />\r\n");
    free(html);
    html = blogc_content_parse_inline("asd \r\n");
    assert_non_null(html);
    assert_string_equal(html, "asd \r\n");
    free(html);
}


static void
test_content_parse_inline_endash_emdash(void **state)
{
    char *html = blogc_content_parse_inline("foo -- bar");
    assert_non_null(html);
    assert_string_equal(html, "foo &ndash; bar");
    free(html);
    html = blogc_content_parse_inline("foo --- bar");
    assert_non_null(html);
    assert_string_equal(html, "foo &mdash; bar");
    free(html);
    html = blogc_content_parse_inline("foo --");
    assert_non_null(html);
    assert_string_equal(html, "foo &ndash;");
    free(html);
    html = blogc_content_parse_inline("foo ---");
    assert_non_null(html);
    assert_string_equal(html, "foo &mdash;");
    free(html);
    html = blogc_content_parse_inline("foo \\-\\-");
    assert_non_null(html);
    assert_string_equal(html, "foo --");
    free(html);
    html = blogc_content_parse_inline("foo \\-\\-\\-");
    assert_non_null(html);
    assert_string_equal(html, "foo ---");
    free(html);
    html = blogc_content_parse_inline("foo \\---");
    assert_non_null(html);
    assert_string_equal(html, "foo -&ndash;");
    free(html);
    html = blogc_content_parse_inline("foo \\----");
    assert_non_null(html);
    assert_string_equal(html, "foo -&mdash;");
    free(html);
    html = blogc_content_parse_inline("foo \\-\\- bar");
    assert_non_null(html);
    assert_string_equal(html, "foo -- bar");
    free(html);
    html = blogc_content_parse_inline("foo \\-\\-\\- bar");
    assert_non_null(html);
    assert_string_equal(html, "foo --- bar");
    free(html);
    html = blogc_content_parse_inline("foo \\--- bar");
    assert_non_null(html);
    assert_string_equal(html, "foo -&ndash; bar");
    free(html);
    html = blogc_content_parse_inline("foo \\---- bar");
    assert_non_null(html);
    assert_string_equal(html, "foo -&mdash; bar");
    free(html);
    html = blogc_content_parse_inline("`foo -- bar`");
    assert_non_null(html);
    assert_string_equal(html, "<code>foo -- bar</code>");
    free(html);
    html = blogc_content_parse_inline("`foo --- bar`");
    assert_non_null(html);
    assert_string_equal(html, "<code>foo --- bar</code>");
    free(html);
    html = blogc_content_parse_inline("``foo -- bar``");
    assert_non_null(html);
    assert_string_equal(html, "<code>foo -- bar</code>");
    free(html);
    html = blogc_content_parse_inline("``foo --- bar``");
    assert_non_null(html);
    assert_string_equal(html, "<code>foo --- bar</code>");
    free(html);
}


int
main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_slugify),
        cmocka_unit_test(test_htmlentities),
        cmocka_unit_test(test_fix_description),
        cmocka_unit_test(test_is_ordered_list_item),
        cmocka_unit_test(test_content_parse),
        cmocka_unit_test(test_content_parse_crlf),
        cmocka_unit_test(test_content_parse_with_excerpt),
        cmocka_unit_test(test_content_parse_with_excerpt_crlf),
        cmocka_unit_test(test_content_parse_header),
        cmocka_unit_test(test_content_parse_header_crlf),
        cmocka_unit_test(test_content_parse_html),
        cmocka_unit_test(test_content_parse_html_crlf),
        cmocka_unit_test(test_content_parse_blockquote),
        cmocka_unit_test(test_content_parse_blockquote_crlf),
        cmocka_unit_test(test_content_parse_code),
        cmocka_unit_test(test_content_parse_code_crlf),
        cmocka_unit_test(test_content_parse_horizontal_rule),
        cmocka_unit_test(test_content_parse_horizontal_rule_crlf),
        cmocka_unit_test(test_content_parse_unordered_list),
        cmocka_unit_test(test_content_parse_unordered_list_crlf),
        cmocka_unit_test(test_content_parse_ordered_list),
        cmocka_unit_test(test_content_parse_ordered_list_crlf),
        cmocka_unit_test(test_content_parse_first_header),
        cmocka_unit_test(test_content_parse_first_header_crlf),
        cmocka_unit_test(test_content_parse_headers),
        cmocka_unit_test(test_content_parse_headers_crlf),
        cmocka_unit_test(test_content_parse_description),
        cmocka_unit_test(test_content_parse_description_crlf),
        cmocka_unit_test(test_content_parse_endl),
        cmocka_unit_test(test_content_parse_endl_crlf),
        cmocka_unit_test(test_content_parse_invalid_excerpt),
        cmocka_unit_test(test_content_parse_invalid_header),
        cmocka_unit_test(test_content_parse_invalid_header_empty),
        cmocka_unit_test(test_content_parse_invalid_blockquote),
        cmocka_unit_test(test_content_parse_invalid_code),
        cmocka_unit_test(test_content_parse_invalid_horizontal_rule),
        cmocka_unit_test(test_content_parse_invalid_unordered_list),
        cmocka_unit_test(test_content_parse_invalid_ordered_list),
        cmocka_unit_test(test_content_parse_inline),
        cmocka_unit_test(test_content_parse_inline_em),
        cmocka_unit_test(test_content_parse_inline_strong),
        cmocka_unit_test(test_content_parse_inline_code),
        cmocka_unit_test(test_content_parse_inline_link),
        cmocka_unit_test(test_content_parse_inline_link_auto),
        cmocka_unit_test(test_content_parse_inline_image),
        cmocka_unit_test(test_content_parse_inline_line_break),
        cmocka_unit_test(test_content_parse_inline_line_break_crlf),
        cmocka_unit_test(test_content_parse_inline_endash_emdash),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
