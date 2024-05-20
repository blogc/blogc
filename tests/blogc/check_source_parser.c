// SPDX-FileCopyrightText: 2014-2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
// SPDX-License-Identifier: BSD-3-Clause

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>
#include "../../src/common/error.h"
#include "../../src/common/utils.h"
#include "../../src/blogc/source-parser.h"


static void
test_source_parse(void **state)
{
    const char *a =
        "VAR1: asd asd\n"
        "VAR2: 123chunda\n"
        "----------\n"
        "# This is a test\n"
        "\n"
        "bola\n";
    bc_error_t *err = NULL;
    bc_trie_t *source = blogc_source_parse(a, strlen(a), 0, &err);
    assert_null(err);
    assert_non_null(source);
    assert_int_equal(bc_trie_size(source), 7);
    assert_string_equal(bc_trie_lookup(source, "VAR1"), "asd asd");
    assert_string_equal(bc_trie_lookup(source, "VAR2"), "123chunda");
    assert_string_equal(bc_trie_lookup(source, "EXCERPT"),
        "<h1 id=\"this-is-a-test\">This is a test</h1>\n"
        "<p>bola</p>\n");
    assert_string_equal(bc_trie_lookup(source, "CONTENT"),
        "<h1 id=\"this-is-a-test\">This is a test</h1>\n"
        "<p>bola</p>\n");
    assert_string_equal(bc_trie_lookup(source, "RAW_CONTENT"),
        "# This is a test\n"
        "\n"
        "bola\n");
    assert_string_equal(bc_trie_lookup(source, "FIRST_HEADER"), "This is a test");
    assert_string_equal(bc_trie_lookup(source, "DESCRIPTION"), "bola");
    assert_null(bc_trie_lookup(source, "TOCTREE"));
    bc_trie_free(source);
}


static void
test_source_parse_crlf(void **state)
{
    const char *a =
        "VAR1: asd asd\r\n"
        "VAR2: 123chunda\r\n"
        "----------\r\n"
        "# This is a test\r\n"
        "\r\n"
        "bola\r\n";
    bc_error_t *err = NULL;
    bc_trie_t *source = blogc_source_parse(a, strlen(a), 1, &err);
    assert_null(err);
    assert_non_null(source);
    assert_int_equal(bc_trie_size(source), 8);
    assert_string_equal(bc_trie_lookup(source, "VAR1"), "asd asd");
    assert_string_equal(bc_trie_lookup(source, "VAR2"), "123chunda");
    assert_string_equal(bc_trie_lookup(source, "EXCERPT"),
        "<h1 id=\"this-is-a-test\">This is a test</h1>\r\n"
        "<p>bola</p>\r\n");
    assert_string_equal(bc_trie_lookup(source, "CONTENT"),
        "<h1 id=\"this-is-a-test\">This is a test</h1>\r\n"
        "<p>bola</p>\r\n");
    assert_string_equal(bc_trie_lookup(source, "RAW_CONTENT"),
        "# This is a test\r\n"
        "\r\n"
        "bola\r\n");
    assert_string_equal(bc_trie_lookup(source, "FIRST_HEADER"), "This is a test");
    assert_string_equal(bc_trie_lookup(source, "DESCRIPTION"), "bola");
    assert_string_equal(bc_trie_lookup(source, "TOCTREE"),
        "<ul>\r\n"
        "    <li><a href=\"#this-is-a-test\">This is a test</a></li>\r\n"
        "</ul>\r\n");
    bc_trie_free(source);
}


static void
test_source_parse_with_spaces(void **state)
{
    const char *a =
        "\n  \n"
        "VAR1: chunda     \t   \n"
        "\n\n"
        "BOLA: guda\n"
        "----------\n"
        "# This is a test\n"
        "\n"
        "bola\n";
    bc_error_t *err = NULL;
    bc_trie_t *source = blogc_source_parse(a, strlen(a), -1, &err);
    assert_null(err);
    assert_non_null(source);
    assert_int_equal(bc_trie_size(source), 8);
    assert_string_equal(bc_trie_lookup(source, "VAR1"), "chunda");
    assert_string_equal(bc_trie_lookup(source, "BOLA"), "guda");
    assert_string_equal(bc_trie_lookup(source, "EXCERPT"),
        "<h1 id=\"this-is-a-test\">This is a test</h1>\n"
        "<p>bola</p>\n");
    assert_string_equal(bc_trie_lookup(source, "CONTENT"),
        "<h1 id=\"this-is-a-test\">This is a test</h1>\n"
        "<p>bola</p>\n");
    assert_string_equal(bc_trie_lookup(source, "RAW_CONTENT"),
        "# This is a test\n"
        "\n"
        "bola\n");
    assert_string_equal(bc_trie_lookup(source, "FIRST_HEADER"), "This is a test");
    assert_string_equal(bc_trie_lookup(source, "DESCRIPTION"), "bola");
    assert_string_equal(bc_trie_lookup(source, "TOCTREE"),
        "<ul>\n"
        "    <li><a href=\"#this-is-a-test\">This is a test</a></li>\n"
        "</ul>\n");
    bc_trie_free(source);
}


static void
test_source_parse_with_excerpt(void **state)
{
    const char *a =
        "VAR1: asd asd\n"
        "VAR2: 123chunda\n"
        "----------\n"
        "# This is a test\n"
        "\n"
        "bola\n"
        "\n"
        "...\n"
        "\n"
        "guda\n"
        "yay";
    bc_error_t *err = NULL;
    bc_trie_t *source = blogc_source_parse(a, strlen(a), 0, &err);
    assert_null(err);
    assert_non_null(source);
    assert_int_equal(bc_trie_size(source), 7);
    assert_string_equal(bc_trie_lookup(source, "VAR1"), "asd asd");
    assert_string_equal(bc_trie_lookup(source, "VAR2"), "123chunda");
    assert_string_equal(bc_trie_lookup(source, "EXCERPT"),
        "<h1 id=\"this-is-a-test\">This is a test</h1>\n"
        "<p>bola</p>\n");
    assert_string_equal(bc_trie_lookup(source, "CONTENT"),
        "<h1 id=\"this-is-a-test\">This is a test</h1>\n"
        "<p>bola</p>\n"
        "<p>guda\n"
        "yay</p>\n");
    assert_string_equal(bc_trie_lookup(source, "RAW_CONTENT"),
        "# This is a test\n"
        "\n"
        "bola\n"
        "\n"
        "...\n"
        "\n"
        "guda\n"
        "yay");
    assert_string_equal(bc_trie_lookup(source, "FIRST_HEADER"), "This is a test");
    assert_string_equal(bc_trie_lookup(source, "DESCRIPTION"), "bola");
    assert_null(bc_trie_lookup(source, "TOCTREE"));
    bc_trie_free(source);
}


static void
test_source_parse_with_first_header(void **state)
{
    const char *a =
        "VAR1: asd asd\n"
        "VAR2: 123chunda\n"
        "FIRST_HEADER: THIS IS CHUNDA!\n"
        "----------\n"
        "# This is a test\n"
        "\n"
        "bola\n";
    bc_error_t *err = NULL;
    bc_trie_t *source = blogc_source_parse(a, strlen(a), 0, &err);
    assert_null(err);
    assert_non_null(source);
    assert_int_equal(bc_trie_size(source), 7);
    assert_string_equal(bc_trie_lookup(source, "VAR1"), "asd asd");
    assert_string_equal(bc_trie_lookup(source, "VAR2"), "123chunda");
    assert_string_equal(bc_trie_lookup(source, "EXCERPT"),
        "<h1 id=\"this-is-a-test\">This is a test</h1>\n"
        "<p>bola</p>\n");
    assert_string_equal(bc_trie_lookup(source, "CONTENT"),
        "<h1 id=\"this-is-a-test\">This is a test</h1>\n"
        "<p>bola</p>\n");
    assert_string_equal(bc_trie_lookup(source, "RAW_CONTENT"),
        "# This is a test\n"
        "\n"
        "bola\n");
    assert_string_equal(bc_trie_lookup(source, "FIRST_HEADER"), "THIS IS CHUNDA!");
    assert_string_equal(bc_trie_lookup(source, "DESCRIPTION"), "bola");
    assert_null(bc_trie_lookup(source, "TOCTREE"));
    bc_trie_free(source);
}


static void
test_source_parse_with_description(void **state)
{
    const char *a =
        "VAR1: asd asd\n"
        "VAR2: 123chunda\n"
        "DESCRIPTION: huehuehuebrbr\n"
        "----------\n"
        "# This is a test\n"
        "\n"
        "bola\n";
    bc_error_t *err = NULL;
    bc_trie_t *source = blogc_source_parse(a, strlen(a), 0, &err);
    assert_null(err);
    assert_non_null(source);
    assert_int_equal(bc_trie_size(source), 7);
    assert_string_equal(bc_trie_lookup(source, "VAR1"), "asd asd");
    assert_string_equal(bc_trie_lookup(source, "VAR2"), "123chunda");
    assert_string_equal(bc_trie_lookup(source, "EXCERPT"),
        "<h1 id=\"this-is-a-test\">This is a test</h1>\n"
        "<p>bola</p>\n");
    assert_string_equal(bc_trie_lookup(source, "CONTENT"),
        "<h1 id=\"this-is-a-test\">This is a test</h1>\n"
        "<p>bola</p>\n");
    assert_string_equal(bc_trie_lookup(source, "RAW_CONTENT"),
        "# This is a test\n"
        "\n"
        "bola\n");
    assert_string_equal(bc_trie_lookup(source, "FIRST_HEADER"), "This is a test");
    assert_string_equal(bc_trie_lookup(source, "DESCRIPTION"), "huehuehuebrbr");
    assert_null(bc_trie_lookup(source, "TOCTREE"));
    bc_trie_free(source);
}


static void
test_source_parse_with_toctree(void **state)
{
    const char *a =
        "VAR1: asd asd\n"
        "VAR2: 123chunda\n"
        "----------\n"
        "### asd\n"
        "### qwe\n"
        "## zxc\n"
        "### rty\n"
        "#### bnm\n"
        "### asd\n";
    bc_error_t *err = NULL;
    bc_trie_t *source = blogc_source_parse(a, strlen(a), -1, &err);
    assert_null(err);
    assert_non_null(source);
    assert_int_equal(bc_trie_size(source), 7);
    assert_string_equal(bc_trie_lookup(source, "VAR1"), "asd asd");
    assert_string_equal(bc_trie_lookup(source, "VAR2"), "123chunda");
    assert_string_equal(bc_trie_lookup(source, "EXCERPT"),
        "<h3 id=\"asd\">asd</h3>\n"
        "<h3 id=\"qwe\">qwe</h3>\n"
        "<h2 id=\"zxc\">zxc</h2>\n"
        "<h3 id=\"rty\">rty</h3>\n"
        "<h4 id=\"bnm\">bnm</h4>\n"
        "<h3 id=\"asd\">asd</h3>\n");
    assert_string_equal(bc_trie_lookup(source, "CONTENT"),
        "<h3 id=\"asd\">asd</h3>\n"
        "<h3 id=\"qwe\">qwe</h3>\n"
        "<h2 id=\"zxc\">zxc</h2>\n"
        "<h3 id=\"rty\">rty</h3>\n"
        "<h4 id=\"bnm\">bnm</h4>\n"
        "<h3 id=\"asd\">asd</h3>\n");
    assert_string_equal(bc_trie_lookup(source, "RAW_CONTENT"),
        "### asd\n"
        "### qwe\n"
        "## zxc\n"
        "### rty\n"
        "#### bnm\n"
        "### asd\n");
    assert_string_equal(bc_trie_lookup(source, "FIRST_HEADER"), "asd");
    assert_string_equal(bc_trie_lookup(source, "TOCTREE"),
        "<ul>\n"
        "    <ul>\n"
        "        <li><a href=\"#asd\">asd</a></li>\n"
        "        <li><a href=\"#qwe\">qwe</a></li>\n"
        "    </ul>\n"
        "    <li><a href=\"#zxc\">zxc</a></li>\n"
        "    <ul>\n"
        "        <li><a href=\"#rty\">rty</a></li>\n"
        "        <ul>\n"
        "            <li><a href=\"#bnm\">bnm</a></li>\n"
        "        </ul>\n"
        "        <li><a href=\"#asd\">asd</a></li>\n"
        "    </ul>\n"
        "</ul>\n");
    bc_trie_free(source);
}


static void
test_source_parse_with_toctree_noheader(void **state)
{
    const char *a =
        "VAR1: asd asd\n"
        "VAR2: 123chunda\n"
        "----------\n"
        "asd\n";
    bc_error_t *err = NULL;
    bc_trie_t *source = blogc_source_parse(a, strlen(a), -1, &err);
    assert_null(err);
    assert_non_null(source);
    assert_int_equal(bc_trie_size(source), 6);
    assert_string_equal(bc_trie_lookup(source, "VAR1"), "asd asd");
    assert_string_equal(bc_trie_lookup(source, "VAR2"), "123chunda");
    assert_string_equal(bc_trie_lookup(source, "EXCERPT"),
        "<p>asd</p>\n");
    assert_string_equal(bc_trie_lookup(source, "CONTENT"),
        "<p>asd</p>\n");
    assert_string_equal(bc_trie_lookup(source, "RAW_CONTENT"),
        "asd\n");
    assert_null(bc_trie_lookup(source, "FIRST_HEADER"));
    assert_null(bc_trie_lookup(source, "TOCTREE"));
    bc_trie_free(source);
}


static void
test_source_parse_with_toctree_maxdepth1(void **state)
{
    const char *a =
        "VAR1: asd asd\n"
        "VAR2: 123chunda\n"
        "----------\n"
        "### asd\n"
        "### qwe\n"
        "## zxc\n"
        "### rty\n"
        "#### bnm\n"
        "### asd\n";
    bc_error_t *err = NULL;
    bc_trie_t *source = blogc_source_parse(a, strlen(a), 1, &err);
    assert_null(err);
    assert_non_null(source);
    assert_int_equal(bc_trie_size(source), 7);
    assert_string_equal(bc_trie_lookup(source, "VAR1"), "asd asd");
    assert_string_equal(bc_trie_lookup(source, "VAR2"), "123chunda");
    assert_string_equal(bc_trie_lookup(source, "EXCERPT"),
        "<h3 id=\"asd\">asd</h3>\n"
        "<h3 id=\"qwe\">qwe</h3>\n"
        "<h2 id=\"zxc\">zxc</h2>\n"
        "<h3 id=\"rty\">rty</h3>\n"
        "<h4 id=\"bnm\">bnm</h4>\n"
        "<h3 id=\"asd\">asd</h3>\n");
    assert_string_equal(bc_trie_lookup(source, "CONTENT"),
        "<h3 id=\"asd\">asd</h3>\n"
        "<h3 id=\"qwe\">qwe</h3>\n"
        "<h2 id=\"zxc\">zxc</h2>\n"
        "<h3 id=\"rty\">rty</h3>\n"
        "<h4 id=\"bnm\">bnm</h4>\n"
        "<h3 id=\"asd\">asd</h3>\n");
    assert_string_equal(bc_trie_lookup(source, "RAW_CONTENT"),
        "### asd\n"
        "### qwe\n"
        "## zxc\n"
        "### rty\n"
        "#### bnm\n"
        "### asd\n");
    assert_string_equal(bc_trie_lookup(source, "FIRST_HEADER"), "asd");
    assert_string_equal(bc_trie_lookup(source, "TOCTREE"),
        "<ul>\n"
        "    <li><a href=\"#zxc\">zxc</a></li>\n"
        "</ul>\n");
    bc_trie_free(source);
}


static void
test_source_parse_with_toctree_maxdepth_invalid(void **state)
{
    const char *a =
        "VAR1: asd asd\n"
        "VAR2: 123chunda\n"
        "TOCTREE_MAXDEPTH: bola\n"
        "----------\n"
        "### asd\n"
        "### qwe\n"
        "## zxc\n"
        "### rty\n"
        "#### bnm\n"
        "### asd\n";
    bc_error_t *err = NULL;
    bc_trie_t *source = blogc_source_parse(a, strlen(a), 1, &err);
    assert_non_null(err);
    assert_null(source);
    assert_int_equal(err->type, BLOGC_ERROR_SOURCE_PARSER);
    assert_string_equal(err->msg,
        "Invalid value for 'TOCTREE_MAXDEPTH' variable: bola.\n"
        "Error occurred near line 10, position 8: ### asd");
    bc_error_free(err);
    bc_trie_free(source);
}


static void
test_source_parse_config_empty(void **state)
{
    const char *a = "";
    bc_error_t *err = NULL;
    bc_trie_t *source = blogc_source_parse(a, strlen(a), 0, &err);
    assert_null(source);
    assert_non_null(err);
    assert_int_equal(err->type, BLOGC_ERROR_SOURCE_PARSER);
    assert_string_equal(err->msg, "Your source file is empty.");
    bc_error_free(err);
    bc_trie_free(source);
}


static void
test_source_parse_config_invalid_key(void **state)
{
    const char *a = "bola: guda";
    bc_error_t *err = NULL;
    bc_trie_t *source = blogc_source_parse(a, strlen(a), 0, &err);
    assert_non_null(err);
    assert_int_equal(err->type, BLOGC_ERROR_SOURCE_PARSER);
    assert_string_equal(err->msg,
        "Can't find a configuration key or the content separator.\n"
        "Error occurred near line 1, position 1: bola: guda");
    bc_error_free(err);
    bc_trie_free(source);
}


static void
test_source_parse_config_no_key(void **state)
{
    const char *a = "BOLa";
    bc_error_t *err = NULL;
    bc_trie_t *source = blogc_source_parse(a, strlen(a), 0, &err);
    assert_non_null(err);
    assert_int_equal(err->type, BLOGC_ERROR_SOURCE_PARSER);
    assert_string_equal(err->msg,
        "Invalid configuration key.\n"
        "Error occurred near line 1, position 4: BOLa");
    bc_error_free(err);
    bc_trie_free(source);
}


static void
test_source_parse_config_no_key2(void **state)
{
    const char *a = "BOLA";
    bc_error_t *err = NULL;
    bc_trie_t *source = blogc_source_parse(a, strlen(a), 0, &err);
    assert_non_null(err);
    assert_int_equal(err->type, BLOGC_ERROR_SOURCE_PARSER);
    assert_string_equal(err->msg,
        "Your last configuration key is missing ':' and the value\n"
        "Error occurred near line 1, position 5: BOLA");
    bc_error_free(err);
    bc_trie_free(source);
}


static void
test_source_parse_config_no_value(void **state)
{
    // this is a special case, not an error
    const char *a = "BOLA:\r\n";
    bc_error_t *err = NULL;
    bc_trie_t *source = blogc_source_parse(a, strlen(a), 0, &err);
    assert_non_null(source);
    assert_null(err);
    assert_string_equal(bc_trie_lookup(source, "BOLA"), "");
    assert_null(bc_trie_lookup(source, "TOCTREE"));
    bc_trie_free(source);
}


static void
test_source_parse_config_no_value2(void **state)
{
    const char *a = "BOLA:";
    bc_error_t *err = NULL;
    bc_trie_t *source = blogc_source_parse(a, strlen(a), 0, &err);
    assert_null(source);
    assert_non_null(err);
    assert_int_equal(err->type, BLOGC_ERROR_SOURCE_PARSER);
    assert_string_equal(err->msg,
        "Configuration value not provided for 'BOLA'.\n"
        "Error occurred near line 1, position 6: BOLA:");
    bc_error_free(err);
    bc_trie_free(source);
}


static void
test_source_parse_config_reserved_name(void **state)
{
    const char *a = "FILENAME: asd\r\n";
    bc_error_t *err = NULL;
    bc_trie_t *source = blogc_source_parse(a, strlen(a), 0, &err);
    assert_null(source);
    assert_non_null(err);
    assert_int_equal(err->type, BLOGC_ERROR_SOURCE_PARSER);
    assert_string_equal(err->msg,
        "'FILENAME' variable is forbidden in source files. It will be set "
        "for you by the compiler.");
    bc_error_free(err);
    bc_trie_free(source);
}


static void
test_source_parse_config_reserved_name2(void **state)
{
    const char *a = "CONTENT: asd\r\n";
    bc_error_t *err = NULL;
    bc_trie_t *source = blogc_source_parse(a, strlen(a), 0, &err);
    assert_null(source);
    assert_non_null(err);
    assert_int_equal(err->type, BLOGC_ERROR_SOURCE_PARSER);
    assert_string_equal(err->msg,
        "'CONTENT' variable is forbidden in source files. It will be set "
        "for you by the compiler.");
    bc_error_free(err);
    bc_trie_free(source);
}


static void
test_source_parse_config_reserved_name3(void **state)
{
    const char *a = "DATE_FORMATTED: asd\r\n";
    bc_error_t *err = NULL;
    bc_trie_t *source = blogc_source_parse(a, strlen(a), 0, &err);
    assert_null(source);
    assert_non_null(err);
    assert_int_equal(err->type, BLOGC_ERROR_SOURCE_PARSER);
    assert_string_equal(err->msg,
        "'DATE_FORMATTED' variable is forbidden in source files. It will be set "
        "for you by the compiler.");
    bc_error_free(err);
    bc_trie_free(source);
}


static void
test_source_parse_config_reserved_name4(void **state)
{
    const char *a = "DATE_FIRST_FORMATTED: asd\r\n";
    bc_error_t *err = NULL;
    bc_trie_t *source = blogc_source_parse(a, strlen(a), 0, &err);
    assert_null(source);
    assert_non_null(err);
    assert_int_equal(err->type, BLOGC_ERROR_SOURCE_PARSER);
    assert_string_equal(err->msg,
        "'DATE_FIRST_FORMATTED' variable is forbidden in source files. It will be set "
        "for you by the compiler.");
    bc_error_free(err);
    bc_trie_free(source);
}


static void
test_source_parse_config_reserved_name5(void **state)
{
    const char *a = "DATE_LAST_FORMATTED: asd\r\n";
    bc_error_t *err = NULL;
    bc_trie_t *source = blogc_source_parse(a, strlen(a), 0, &err);
    assert_null(source);
    assert_non_null(err);
    assert_int_equal(err->type, BLOGC_ERROR_SOURCE_PARSER);
    assert_string_equal(err->msg,
        "'DATE_LAST_FORMATTED' variable is forbidden in source files. It will be set "
        "for you by the compiler.");
    bc_error_free(err);
    bc_trie_free(source);
}


static void
test_source_parse_config_reserved_name6(void **state)
{
    const char *a = "PAGE_FIRST: asd\r\n";
    bc_error_t *err = NULL;
    bc_trie_t *source = blogc_source_parse(a, strlen(a), 0, &err);
    assert_null(source);
    assert_non_null(err);
    assert_int_equal(err->type, BLOGC_ERROR_SOURCE_PARSER);
    assert_string_equal(err->msg,
        "'PAGE_FIRST' variable is forbidden in source files. It will be set "
        "for you by the compiler.");
    bc_error_free(err);
    bc_trie_free(source);
}


static void
test_source_parse_config_reserved_name7(void **state)
{
    const char *a = "PAGE_PREVIOUS: asd\r\n";
    bc_error_t *err = NULL;
    bc_trie_t *source = blogc_source_parse(a, strlen(a), 0, &err);
    assert_null(source);
    assert_non_null(err);
    assert_int_equal(err->type, BLOGC_ERROR_SOURCE_PARSER);
    assert_string_equal(err->msg,
        "'PAGE_PREVIOUS' variable is forbidden in source files. It will be set "
        "for you by the compiler.");
    bc_error_free(err);
    bc_trie_free(source);
}


static void
test_source_parse_config_reserved_name8(void **state)
{
    const char *a = "PAGE_CURRENT: asd\r\n";
    bc_error_t *err = NULL;
    bc_trie_t *source = blogc_source_parse(a, strlen(a), 0, &err);
    assert_null(source);
    assert_non_null(err);
    assert_int_equal(err->type, BLOGC_ERROR_SOURCE_PARSER);
    assert_string_equal(err->msg,
        "'PAGE_CURRENT' variable is forbidden in source files. It will be set "
        "for you by the compiler.");
    bc_error_free(err);
    bc_trie_free(source);
}


static void
test_source_parse_config_reserved_name9(void **state)
{
    const char *a = "PAGE_NEXT: asd\r\n";
    bc_error_t *err = NULL;
    bc_trie_t *source = blogc_source_parse(a, strlen(a), 0, &err);
    assert_null(source);
    assert_non_null(err);
    assert_int_equal(err->type, BLOGC_ERROR_SOURCE_PARSER);
    assert_string_equal(err->msg,
        "'PAGE_NEXT' variable is forbidden in source files. It will be set "
        "for you by the compiler.");
    bc_error_free(err);
    bc_trie_free(source);
}


static void
test_source_parse_config_reserved_name10(void **state)
{
    const char *a = "PAGE_LAST: asd\r\n";
    bc_error_t *err = NULL;
    bc_trie_t *source = blogc_source_parse(a, strlen(a), 0, &err);
    assert_null(source);
    assert_non_null(err);
    assert_int_equal(err->type, BLOGC_ERROR_SOURCE_PARSER);
    assert_string_equal(err->msg,
        "'PAGE_LAST' variable is forbidden in source files. It will be set "
        "for you by the compiler.");
    bc_error_free(err);
    bc_trie_free(source);
}


static void
test_source_parse_config_reserved_name11(void **state)
{
    const char *a = "BLOGC_VERSION: 1.0\r\n";
    bc_error_t *err = NULL;
    bc_trie_t *source = blogc_source_parse(a, strlen(a), 0, &err);
    assert_null(source);
    assert_non_null(err);
    assert_int_equal(err->type, BLOGC_ERROR_SOURCE_PARSER);
    assert_string_equal(err->msg,
        "'BLOGC_VERSION' variable is forbidden in source files. It will be set "
        "for you by the compiler.");
    bc_error_free(err);
    bc_trie_free(source);
}


static void
test_source_parse_config_value_no_line_ending(void **state)
{
    const char *a = "BOLA: asd";
    bc_error_t *err = NULL;
    bc_trie_t *source = blogc_source_parse(a, strlen(a), 0, &err);
    assert_null(source);
    assert_non_null(err);
    assert_int_equal(err->type, BLOGC_ERROR_SOURCE_PARSER);
    assert_string_equal(err->msg,
        "No line ending after the configuration value for 'BOLA'.\n"
        "Error occurred near line 1, position 10: BOLA: asd");
    bc_error_free(err);
    bc_trie_free(source);
}


static void
test_source_parse_invalid_separator(void **state)
{
    const char *a = "BOLA: asd\n---#";
    bc_error_t *err = NULL;
    bc_trie_t *source = blogc_source_parse(a, strlen(a), 0, &err);
    assert_null(source);
    assert_non_null(err);
    assert_int_equal(err->type, BLOGC_ERROR_SOURCE_PARSER);
    assert_string_equal(err->msg,
        "Invalid content separator. Must be more than one '-' characters.\n"
        "Error occurred near line 2, position 4: ---#");
    bc_error_free(err);
    bc_trie_free(source);
}


int
main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_source_parse),
        cmocka_unit_test(test_source_parse_crlf),
        cmocka_unit_test(test_source_parse_with_spaces),
        cmocka_unit_test(test_source_parse_with_excerpt),
        cmocka_unit_test(test_source_parse_with_first_header),
        cmocka_unit_test(test_source_parse_with_description),
        cmocka_unit_test(test_source_parse_with_toctree),
        cmocka_unit_test(test_source_parse_with_toctree_noheader),
        cmocka_unit_test(test_source_parse_with_toctree_maxdepth1),
        cmocka_unit_test(test_source_parse_with_toctree_maxdepth_invalid),
        cmocka_unit_test(test_source_parse_config_empty),
        cmocka_unit_test(test_source_parse_config_invalid_key),
        cmocka_unit_test(test_source_parse_config_no_key),
        cmocka_unit_test(test_source_parse_config_no_key2),
        cmocka_unit_test(test_source_parse_config_no_value),
        cmocka_unit_test(test_source_parse_config_no_value2),
        cmocka_unit_test(test_source_parse_config_reserved_name),
        cmocka_unit_test(test_source_parse_config_reserved_name2),
        cmocka_unit_test(test_source_parse_config_reserved_name3),
        cmocka_unit_test(test_source_parse_config_reserved_name4),
        cmocka_unit_test(test_source_parse_config_reserved_name5),
        cmocka_unit_test(test_source_parse_config_reserved_name6),
        cmocka_unit_test(test_source_parse_config_reserved_name7),
        cmocka_unit_test(test_source_parse_config_reserved_name8),
        cmocka_unit_test(test_source_parse_config_reserved_name9),
        cmocka_unit_test(test_source_parse_config_reserved_name10),
        cmocka_unit_test(test_source_parse_config_reserved_name11),
        cmocka_unit_test(test_source_parse_config_value_no_line_ending),
        cmocka_unit_test(test_source_parse_invalid_separator),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
