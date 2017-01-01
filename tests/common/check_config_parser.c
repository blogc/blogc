/*
 * blogc: A blog compiler.
 * Copyright (C) 2015-2016 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <stdlib.h>
#include <string.h>

#include "../../src/common/config-parser.h"
#include "../../src/common/error.h"
#include "../../src/common/utils.h"


static void
test_config_empty(void **state)
{
    const char *a = "";
    bc_error_t *err = NULL;
    bc_config_t *c = bc_config_parse(a, strlen(a), NULL, &err);
    assert_null(err);
    assert_non_null(c);
    assert_non_null(c->root);
    assert_int_equal(bc_trie_size(c->root), 0);
    assert_string_equal(bc_config_get_with_default(c, "bola", "foo", "bar"), "bar");
    bc_config_free(c);
}


static void
test_config_section_empty(void **state)
{
    const char *a = "[foo]";
    bc_error_t *err = NULL;
    bc_config_t *c = bc_config_parse(a, strlen(a), NULL, &err);
    assert_null(err);
    assert_non_null(c);
    assert_non_null(c->root);
    assert_int_equal(bc_trie_size(c->root), 1);
    char **s = bc_config_list_sections(c);
    assert_non_null(s);
    assert_int_equal(bc_strv_length(s), 1);
    assert_string_equal(s[0], "foo");
    assert_null(s[1]);
    bc_strv_free(s);
    char **k = bc_config_list_keys(c, "foo");
    assert_non_null(k);
    assert_int_equal(bc_strv_length(k), 0);
    assert_null(k[0]);
    bc_strv_free(k);
    bc_config_free(c);
}


static void
test_config_section(void **state)
{
    const char *a =
        "[foo]\n"
        "asd = zxc";
    bc_error_t *err = NULL;
    bc_config_t *c = bc_config_parse(a, strlen(a), NULL, &err);
    assert_null(err);
    assert_non_null(c);
    assert_non_null(c->root);
    assert_int_equal(bc_trie_size(c->root), 1);
    char **s = bc_config_list_sections(c);
    assert_non_null(s);
    assert_int_equal(bc_strv_length(s), 1);
    assert_string_equal(s[0], "foo");
    assert_null(s[1]);
    bc_strv_free(s);
    assert_string_equal(bc_config_get(c, "foo", "asd"), "zxc");
    char **k = bc_config_list_keys(c, "foo");
    assert_non_null(k);
    assert_int_equal(bc_strv_length(k), 1);
    assert_string_equal(k[0], "asd");
    assert_null(k[1]);
    bc_strv_free(k);
    bc_config_free(c);

    a =
        "[foo]\n"
        "asd = zxc\n";
    err = NULL;
    c = bc_config_parse(a, strlen(a), NULL, &err);
    assert_null(err);
    assert_non_null(c);
    assert_non_null(c->root);
    assert_int_equal(bc_trie_size(c->root), 1);
    s = bc_config_list_sections(c);
    assert_non_null(s);
    assert_int_equal(bc_strv_length(s), 1);
    assert_string_equal(s[0], "foo");
    assert_null(s[1]);
    bc_strv_free(s);
    assert_string_equal(bc_config_get(c, "foo", "asd"), "zxc");
    k = bc_config_list_keys(c, "foo");
    assert_non_null(k);
    assert_int_equal(bc_strv_length(k), 1);
    assert_string_equal(k[0], "asd");
    assert_null(k[1]);
    bc_strv_free(k);
    bc_config_free(c);

    a =
        "[foo]\r\n"
        "asd = zxc\r\n";
    err = NULL;
    c = bc_config_parse(a, strlen(a), NULL, &err);
    assert_null(err);
    assert_non_null(c);
    assert_non_null(c->root);
    assert_int_equal(bc_trie_size(c->root), 1);
    s = bc_config_list_sections(c);
    assert_non_null(s);
    assert_int_equal(bc_strv_length(s), 1);
    assert_string_equal(s[0], "foo");
    assert_null(s[1]);
    bc_strv_free(s);
    assert_string_equal(bc_config_get(c, "foo", "asd"), "zxc");
    k = bc_config_list_keys(c, "foo");
    assert_non_null(k);
    assert_int_equal(bc_strv_length(k), 1);
    assert_string_equal(k[0], "asd");
    assert_null(k[1]);
    bc_strv_free(k);
    bc_config_free(c);
}


static void
test_config_section_multiple_keys(void **state)
{
    const char *a =
        "[foo]\n"
        "asd = zxc\n"
        "qwe =   rty  \n"
        "zxc = vbn";
    bc_error_t *err = NULL;
    bc_config_t *c = bc_config_parse(a, strlen(a), NULL, &err);
    assert_null(err);
    assert_non_null(c);
    assert_non_null(c->root);
    assert_int_equal(bc_trie_size(c->root), 1);
    char **s = bc_config_list_sections(c);
    assert_non_null(s);
    assert_int_equal(bc_strv_length(s), 1);
    assert_string_equal(s[0], "foo");
    assert_null(s[1]);
    bc_strv_free(s);
    assert_string_equal(bc_config_get(c, "foo", "asd"), "zxc");
    assert_string_equal(bc_config_get(c, "foo", "qwe"), "rty");
    assert_string_equal(bc_config_get(c, "foo", "zxc"), "vbn");
    char **k = bc_config_list_keys(c, "foo");
    assert_non_null(k);
    assert_int_equal(bc_strv_length(k), 3);
    assert_string_equal(k[0], "asd");
    assert_string_equal(k[1], "qwe");
    assert_string_equal(k[2], "zxc");
    assert_null(k[3]);
    bc_strv_free(k);
    bc_config_free(c);

    a =
        "[foo]\n"
        "asd = zxc\n"
        "qwe =   rty  \n"
        "zxc = vbn\n";
    err = NULL;
    c = bc_config_parse(a, strlen(a), NULL, &err);
    assert_null(err);
    assert_non_null(c);
    assert_non_null(c->root);
    assert_int_equal(bc_trie_size(c->root), 1);
    s = bc_config_list_sections(c);
    assert_non_null(s);
    assert_int_equal(bc_strv_length(s), 1);
    assert_string_equal(s[0], "foo");
    assert_null(s[1]);
    bc_strv_free(s);
    assert_string_equal(bc_config_get(c, "foo", "asd"), "zxc");
    assert_string_equal(bc_config_get(c, "foo", "qwe"), "rty");
    assert_string_equal(bc_config_get(c, "foo", "zxc"), "vbn");
    k = bc_config_list_keys(c, "foo");
    assert_non_null(k);
    assert_int_equal(bc_strv_length(k), 3);
    assert_string_equal(k[0], "asd");
    assert_string_equal(k[1], "qwe");
    assert_string_equal(k[2], "zxc");
    assert_null(k[3]);
    bc_strv_free(k);
    bc_config_free(c);

    a =
        "[foo]\r\n"
        "asd = zxc\r\n"
        "qwe =   rty  \r\n"
        "zxc = vbn\r\n";
    err = NULL;
    c = bc_config_parse(a, strlen(a), NULL, &err);
    assert_null(err);
    assert_non_null(c);
    assert_non_null(c->root);
    assert_int_equal(bc_trie_size(c->root), 1);
    s = bc_config_list_sections(c);
    assert_non_null(s);
    assert_int_equal(bc_strv_length(s), 1);
    assert_string_equal(s[0], "foo");
    assert_null(s[1]);
    bc_strv_free(s);
    assert_string_equal(bc_config_get(c, "foo", "asd"), "zxc");
    assert_string_equal(bc_config_get(c, "foo", "qwe"), "rty");
    assert_string_equal(bc_config_get(c, "foo", "zxc"), "vbn");
    k = bc_config_list_keys(c, "foo");
    assert_non_null(k);
    assert_int_equal(bc_strv_length(k), 3);
    assert_string_equal(k[0], "asd");
    assert_string_equal(k[1], "qwe");
    assert_string_equal(k[2], "zxc");
    assert_null(k[3]);
    bc_strv_free(k);
    bc_config_free(c);
}


static void
test_config_section_multiple_sections(void **state)
{
    const char *a =
        "[foo]\n"
        "asd = zxc\n"
        "qwe = rty\n"
        "zxc = vbn\n"
        "\n"
        "[bar]\n"
        "lol = hehe";
    bc_error_t *err = NULL;
    bc_config_t *c = bc_config_parse(a, strlen(a), NULL, &err);
    assert_null(err);
    assert_non_null(c);
    assert_non_null(c->root);
    assert_int_equal(bc_trie_size(c->root), 2);
    char **s = bc_config_list_sections(c);
    assert_non_null(s);
    assert_int_equal(bc_strv_length(s), 2);
    assert_string_equal(s[0], "foo");
    assert_string_equal(s[1], "bar");
    assert_null(s[2]);
    bc_strv_free(s);
    assert_string_equal(bc_config_get(c, "foo", "asd"), "zxc");
    assert_string_equal(bc_config_get(c, "foo", "qwe"), "rty");
    assert_string_equal(bc_config_get(c, "foo", "zxc"), "vbn");
    assert_string_equal(bc_config_get(c, "bar", "lol"), "hehe");
    char **k = bc_config_list_keys(c, "foo");
    assert_non_null(k);
    assert_int_equal(bc_strv_length(k), 3);
    assert_string_equal(k[0], "asd");
    assert_string_equal(k[1], "qwe");
    assert_string_equal(k[2], "zxc");
    assert_null(k[3]);
    bc_strv_free(k);
    k = bc_config_list_keys(c, "bar");
    assert_non_null(k);
    assert_int_equal(bc_strv_length(k), 1);
    assert_string_equal(k[0], "lol");
    assert_null(k[1]);
    bc_strv_free(k);
    bc_config_free(c);

    a =
        "[foo]\n"
        "asd = zxc\n"
        "qwe = rty\n"
        "zxc = vbn\n"
        "\n"
        "[bar]\n"
        "lol = hehe\n";
    err = NULL;
    c = bc_config_parse(a, strlen(a), NULL, &err);
    assert_null(err);
    assert_non_null(c);
    assert_non_null(c->root);
    assert_int_equal(bc_trie_size(c->root), 2);
    s = bc_config_list_sections(c);
    assert_non_null(s);
    assert_int_equal(bc_strv_length(s), 2);
    assert_string_equal(s[0], "foo");
    assert_string_equal(s[1], "bar");
    assert_null(s[2]);
    bc_strv_free(s);
    assert_string_equal(bc_config_get(c, "foo", "asd"), "zxc");
    assert_string_equal(bc_config_get(c, "foo", "qwe"), "rty");
    assert_string_equal(bc_config_get(c, "foo", "zxc"), "vbn");
    assert_string_equal(bc_config_get(c, "bar", "lol"), "hehe");
    k = bc_config_list_keys(c, "foo");
    assert_non_null(k);
    assert_int_equal(bc_strv_length(k), 3);
    assert_string_equal(k[0], "asd");
    assert_string_equal(k[1], "qwe");
    assert_string_equal(k[2], "zxc");
    assert_null(k[3]);
    bc_strv_free(k);
    k = bc_config_list_keys(c, "bar");
    assert_non_null(k);
    assert_int_equal(bc_strv_length(k), 1);
    assert_string_equal(k[0], "lol");
    assert_null(k[1]);
    bc_strv_free(k);
    bc_config_free(c);

    a =
        "[foo]\r\n"
        "asd = zxc\r\n"
        "qwe = rty\r\n"
        "zxc = vbn\r\n"
        "\r\n"
        "[bar]\r\n"
        "lol = hehe\r\n";
    err = NULL;
    c = bc_config_parse(a, strlen(a), NULL, &err);
    assert_null(err);
    assert_non_null(c);
    assert_non_null(c->root);
    assert_int_equal(bc_trie_size(c->root), 2);
    s = bc_config_list_sections(c);
    assert_non_null(s);
    assert_int_equal(bc_strv_length(s), 2);
    assert_string_equal(s[0], "foo");
    assert_string_equal(s[1], "bar");
    assert_null(s[2]);
    bc_strv_free(s);
    assert_string_equal(bc_config_get(c, "foo", "asd"), "zxc");
    assert_string_equal(bc_config_get(c, "foo", "qwe"), "rty");
    assert_string_equal(bc_config_get(c, "foo", "zxc"), "vbn");
    assert_string_equal(bc_config_get(c, "bar", "lol"), "hehe");
    k = bc_config_list_keys(c, "foo");
    assert_non_null(k);
    assert_int_equal(bc_strv_length(k), 3);
    assert_string_equal(k[0], "asd");
    assert_string_equal(k[1], "qwe");
    assert_string_equal(k[2], "zxc");
    assert_null(k[3]);
    bc_strv_free(k);
    k = bc_config_list_keys(c, "bar");
    assert_non_null(k);
    assert_int_equal(bc_strv_length(k), 1);
    assert_string_equal(k[0], "lol");
    assert_null(k[1]);
    bc_strv_free(k);
    bc_config_free(c);
}


static void
test_config_section_list(void **state)
{
    const char *a =
        "[foo]\n"
        "asd = zxc\n"
        "qwe = rty\n"
        "zxc = vbn\n"
        "\n"
        "[bar]\n"
        "lol = hehe\n"
        "  asdasdadssad  ";
    bc_error_t *err = NULL;
    const char *sections[] = {"bar", NULL};
    bc_config_t *c = bc_config_parse(a, strlen(a), sections, &err);
    assert_null(err);
    assert_non_null(c);
    assert_non_null(c->root);
    assert_int_equal(bc_trie_size(c->root), 2);
    char **s = bc_config_list_sections(c);
    assert_non_null(s);
    assert_int_equal(bc_strv_length(s), 2);
    assert_string_equal(s[0], "foo");
    assert_string_equal(s[1], "bar");
    assert_null(s[2]);
    bc_strv_free(s);
    assert_string_equal(bc_config_get(c, "foo", "asd"), "zxc");
    assert_string_equal(bc_config_get(c, "foo", "qwe"), "rty");
    assert_string_equal(bc_config_get(c, "foo", "zxc"), "vbn");
    char **bar = bc_config_get_list(c, "bar");
    assert_non_null(bar);
    assert_string_equal(bar[0], "lol = hehe");
    assert_string_equal(bar[1], "asdasdadssad");
    assert_null(bar[2]);
    bc_strv_free(bar);
    char **k = bc_config_list_keys(c, "foo");
    assert_non_null(k);
    assert_int_equal(bc_strv_length(k), 3);
    assert_string_equal(k[0], "asd");
    assert_string_equal(k[1], "qwe");
    assert_string_equal(k[2], "zxc");
    assert_null(k[3]);
    bc_strv_free(k);
    k = bc_config_list_keys(c, "bar");
    assert_null(k);
    bc_config_free(c);

    a =
        "[foo]\n"
        "asd = zxc\n"
        "qwe = rty\n"
        "zxc = vbn\n"
        "\n"
        "[bar]\n"
        "lol = hehe\n"
        "asdasdadssad\n";
    err = NULL;
    c = bc_config_parse(a, strlen(a), sections, &err);
    assert_null(err);
    assert_non_null(c);
    assert_non_null(c->root);
    assert_int_equal(bc_trie_size(c->root), 2);
    s = bc_config_list_sections(c);
    assert_non_null(s);
    assert_int_equal(bc_strv_length(s), 2);
    assert_string_equal(s[0], "foo");
    assert_string_equal(s[1], "bar");
    assert_null(s[2]);
    bc_strv_free(s);
    assert_string_equal(bc_config_get(c, "foo", "asd"), "zxc");
    assert_string_equal(bc_config_get(c, "foo", "qwe"), "rty");
    assert_string_equal(bc_config_get(c, "foo", "zxc"), "vbn");
    bar = bc_config_get_list(c, "bar");
    assert_non_null(bar);
    assert_string_equal(bar[0], "lol = hehe");
    assert_string_equal(bar[1], "asdasdadssad");
    assert_null(bar[2]);
    bc_strv_free(bar);
    k = bc_config_list_keys(c, "foo");
    assert_non_null(k);
    assert_int_equal(bc_strv_length(k), 3);
    assert_string_equal(k[0], "asd");
    assert_string_equal(k[1], "qwe");
    assert_string_equal(k[2], "zxc");
    assert_null(k[3]);
    bc_strv_free(k);
    k = bc_config_list_keys(c, "bar");
    assert_null(k);
    bc_config_free(c);

    a =
        "[foo]\r\n"
        "asd = zxc\r\n"
        "qwe = rty\r\n"
        "zxc = vbn\r\n"
        "\r\n"
        "[bar]\r\n"
        "lol = hehe\r\n"
        "asdasdadssad\r\n";
    err = NULL;
    c = bc_config_parse(a, strlen(a), sections, &err);
    assert_null(err);
    assert_non_null(c);
    assert_non_null(c->root);
    assert_int_equal(bc_trie_size(c->root), 2);
    s = bc_config_list_sections(c);
    assert_non_null(s);
    assert_int_equal(bc_strv_length(s), 2);
    assert_string_equal(s[0], "foo");
    assert_string_equal(s[1], "bar");
    assert_null(s[2]);
    bc_strv_free(s);
    assert_string_equal(bc_config_get(c, "foo", "asd"), "zxc");
    assert_string_equal(bc_config_get(c, "foo", "qwe"), "rty");
    assert_string_equal(bc_config_get(c, "foo", "zxc"), "vbn");
    bar = bc_config_get_list(c, "bar");
    assert_non_null(bar);
    assert_string_equal(bar[0], "lol = hehe");
    assert_string_equal(bar[1], "asdasdadssad");
    assert_null(bar[2]);
    bc_strv_free(bar);
    k = bc_config_list_keys(c, "foo");
    assert_non_null(k);
    assert_int_equal(bc_strv_length(k), 3);
    assert_string_equal(k[0], "asd");
    assert_string_equal(k[1], "qwe");
    assert_string_equal(k[2], "zxc");
    assert_null(k[3]);
    bc_strv_free(k);
    k = bc_config_list_keys(c, "bar");
    assert_null(k);
    bc_config_free(c);
}


static void
test_config_quoted_values(void **state)
{
    const char *a =
        "[foo]\n"
        "a = \"lol\"\n"
        "b = \"lo\\\"l\"\n"
        "c = \"lo'l\"\n"
        "d = 'lol'\n"
        "e = 'lo\\'l'\n"
        "f = 'lo\"l'\n"
        "g = \\\\asd\n"
        "h = \"\\\\asd\"\n"
        "i = '\\\\asd'\n";
    bc_error_t *err = NULL;
    bc_config_t *c = bc_config_parse(a, strlen(a), NULL, &err);
    assert_null(err);
    assert_non_null(c);
    assert_non_null(c->root);
    assert_int_equal(bc_trie_size(c->root), 1);
    char **s = bc_config_list_sections(c);
    assert_non_null(s);
    assert_int_equal(bc_strv_length(s), 1);
    assert_string_equal(s[0], "foo");
    assert_null(s[1]);
    bc_strv_free(s);
    char **k = bc_config_list_keys(c, "foo");
    assert_non_null(k);
    assert_int_equal(bc_strv_length(k), 9);
    assert_string_equal(k[0], "a");
    assert_string_equal(k[1], "b");
    assert_string_equal(k[2], "c");
    assert_string_equal(k[3], "d");
    assert_string_equal(k[4], "e");
    assert_string_equal(k[5], "f");
    assert_string_equal(k[6], "g");
    assert_string_equal(k[7], "h");
    assert_string_equal(k[8], "i");
    assert_null(k[9]);
    bc_strv_free(k);
    assert_string_equal(bc_config_get(c, "foo", "a"), "lol");
    assert_string_equal(bc_config_get(c, "foo", "b"), "lo\"l");
    assert_string_equal(bc_config_get(c, "foo", "c"), "lo'l");
    assert_string_equal(bc_config_get(c, "foo", "d"), "'lol'");
    assert_string_equal(bc_config_get(c, "foo", "e"), "'lo'l'");
    assert_string_equal(bc_config_get(c, "foo", "f"), "'lo\"l'");
    assert_string_equal(bc_config_get(c, "foo", "g"), "\\asd");
    assert_string_equal(bc_config_get(c, "foo", "h"), "\\asd");
    assert_string_equal(bc_config_get(c, "foo", "i"), "'\\asd'");
    bc_config_free(c);

    a =
        "[foo]\n"
        "\"lol\"\n"
        "\"lo\\\"l\"\n"
        "\"lo'l\"\n"
        "'lol'\n"
        "'lo\\'l'\n"
        "'lo\"l'\n"
        "\\\\asd\n"
        "\"\\\\asd\"\n"
        "'\\\\asd'\n"
        "\n"
        "[bar]\n"
        "'lol = hehe'\n"
        "\"  asdasdadssad  \"";
    err = NULL;
    const char *sections[] = {"foo", "bar", NULL};
    c = bc_config_parse(a, strlen(a), sections, &err);
    assert_null(err);
    assert_non_null(c);
    assert_non_null(c->root);
    assert_int_equal(bc_trie_size(c->root), 2);
    s = bc_config_list_sections(c);
    assert_non_null(s);
    assert_int_equal(bc_strv_length(s), 2);
    assert_string_equal(s[0], "foo");
    assert_string_equal(s[1], "bar");
    assert_null(s[2]);
    bc_strv_free(s);
    char **bar = bc_config_get_list(c, "foo");
    assert_string_equal(bar[0], "lol");
    assert_string_equal(bar[1], "lo\"l");
    assert_string_equal(bar[2], "lo'l");
    assert_string_equal(bar[3], "'lol'");
    assert_string_equal(bar[4], "'lo'l'");
    assert_string_equal(bar[5], "'lo\"l'");
    assert_string_equal(bar[6], "\\asd");
    assert_string_equal(bar[7], "\\asd");
    assert_string_equal(bar[8], "'\\asd'");
    assert_null(bar[9]);
    bc_strv_free(bar);
    bar = bc_config_get_list(c, "bar");
    assert_non_null(bar);
    assert_string_equal(bar[0], "'lol = hehe'");
    assert_string_equal(bar[1], "  asdasdadssad  ");
    assert_null(bar[2]);
    bc_strv_free(bar);
    k = bc_config_list_keys(c, "foo");
    assert_null(k);
    k = bc_config_list_keys(c, "bar");
    assert_null(k);
    bc_config_free(c);
}


static void
test_config_error_start(void **state)
{
    const char *a =
        "asd\n"
        "[foo]";
    bc_error_t *err = NULL;
    bc_config_t *c = bc_config_parse(a, strlen(a), NULL, &err);
    assert_non_null(err);
    assert_null(c);
    assert_int_equal(err->type, BC_ERROR_CONFIG_PARSER);
    assert_string_equal(err->msg,
        "File must start with section.\n"
        "Error occurred near line 1, position 1: asd");
    bc_error_free(err);
}


static void
test_config_error_section_with_newline(void **state)
{
    const char *a =
        "[foo\nbar]";
    bc_error_t *err = NULL;
    bc_config_t *c = bc_config_parse(a, strlen(a), NULL, &err);
    assert_non_null(err);
    assert_null(c);
    assert_int_equal(err->type, BC_ERROR_CONFIG_PARSER);
    assert_string_equal(err->msg,
        "Section names can't have new lines.\n"
        "Error occurred near line 1, position 5: [foo");
    bc_error_free(err);
}


static void
test_config_error_key_without_value(void **state)
{
    const char *a =
        "[foobar]\n"
        "asd = 12\n"
        "foo";
    bc_error_t *err = NULL;
    bc_config_t *c = bc_config_parse(a, strlen(a), NULL, &err);
    assert_non_null(err);
    assert_null(c);
    assert_int_equal(err->type, BC_ERROR_CONFIG_PARSER);
    assert_string_equal(err->msg,
        "Key without value: foo.\n"
        "Error occurred near line 3, position 3: foo");
    bc_error_free(err);
    a =
        "[foobar]\n"
        "asd = 12\n"
        "foo\n";
    err = NULL;
    c = bc_config_parse(a, strlen(a), NULL, &err);
    assert_non_null(err);
    assert_null(c);
    assert_int_equal(err->type, BC_ERROR_CONFIG_PARSER);
    assert_string_equal(err->msg,
        "Key without value: foo.\n"
        "Error occurred near line 3, position 4: foo");
    bc_error_free(err);
}


int
main(void)
{
    const UnitTest tests[] = {
        unit_test(test_config_empty),
        unit_test(test_config_section_empty),
        unit_test(test_config_section),
        unit_test(test_config_section_multiple_keys),
        unit_test(test_config_section_multiple_sections),
        unit_test(test_config_section_list),
        unit_test(test_config_quoted_values),
        unit_test(test_config_error_start),
        unit_test(test_config_error_section_with_newline),
        unit_test(test_config_error_key_without_value),
    };
    return run_tests(tests);
}
