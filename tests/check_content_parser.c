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


int
main(void)
{
    const UnitTest tests[] = {
        unit_test(test_content_parse),
    };
    return run_tests(tests);
}
