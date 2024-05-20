// SPDX-FileCopyrightText: 2014-2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
// SPDX-License-Identifier: BSD-3-Clause

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <stdlib.h>
#include <string.h>

#include "../../src/blogc-make/settings.h"
#include "../../src/common/error.h"
#include "../../src/common/utils.h"


static void
test_settings_empty(void **state)
{
    const char *a = "";
    bc_error_t *err = NULL;
    bm_settings_t *s = bm_settings_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(s);
    assert_int_equal(err->type, BLOGC_MAKE_ERROR_SETTINGS);
    assert_string_equal(err->msg,
        "[global] key required but not found or empty: AUTHOR_NAME");
    bc_error_free(err);
}


static void
test_settings(void **state)
{
    const char *a =
        "[settings]\n"
        "content_dir = guda\n"
        "main_template = foo.tmpl\n"
        "\n"
        "[global]\n"
        "BOLA = asd\n"
        "GUDA = qwe\n";
    bc_error_t *err = NULL;
    bm_settings_t *s = bm_settings_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(s);
    assert_int_equal(err->type, BLOGC_MAKE_ERROR_SETTINGS);
    assert_string_equal(err->msg,
        "[global] key required but not found or empty: AUTHOR_NAME");
    bc_error_free(err);

    a =
        "[settings]\n"
        "content_dir = guda\n"
        "main_template = foo.tmpl\n"
        "\n"
        "[global]\n"
        "bOLA = asd\n"
        "GUDA = qwe\n";
    err = NULL;
    s = bm_settings_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(s);
    assert_int_equal(err->type, BLOGC_MAKE_ERROR_SETTINGS);
    assert_string_equal(err->msg,
        "Invalid [global] key (first character must be uppercase): bOLA");
    bc_error_free(err);

    a =
        "[settings]\n"
        "content_dir = guda\n"
        "main_template = foo.tmpl\n"
        "\n"
        "[global]\n"
        "BOLA = asd\n"
        "GUDa = qwe\n";
    err = NULL;
    s = bm_settings_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(s);
    assert_int_equal(err->type, BLOGC_MAKE_ERROR_SETTINGS);
    assert_string_equal(err->msg,
        "Invalid [global] key (must be uppercase with '_' and digits after first character): GUDa");
    bc_error_free(err);
}


static void
test_settings_env(void **state)
{
    const char *a =
        "[settings]\n"
        "content_dir = guda\n"
        "main_template = foo.tmpl\n"
        "\n"
        "[environment]\n"
        "BOLA = asd\n"
        "GUDA = qwe\n";
    bc_error_t *err = NULL;
    bm_settings_t *s = bm_settings_parse(a, strlen(a), &err);
    assert_non_null(err);
    assert_null(s);
    assert_int_equal(err->type, BLOGC_MAKE_ERROR_SETTINGS);
    assert_string_equal(err->msg,
        "[environment] key required but not found or empty: AUTHOR_NAME");
    bc_error_free(err);
}


static void
test_settings2(void **state)
{
    const char *a =
        "[settings]\n"
        "content_dir = guda\n"
        "main_template = foo.tmpl\n"
        "\n"
        "[global]\n"
        "BOLA = asd\n"
        "GUDA = qwe\n"
        "AUTHOR_NAME = chunda\n"
        "AUTHOR_EMAIL = chunda@example.com\n"
        "SITE_TITLE = Fuuuuuuuuu\n"
        "SITE_TAGLINE = My cool tagline\n"
        "BASE_DOMAIN = http://example.com\n"
        "\n"
        "[posts]\n"
        "\n"
        "aaaa\n"
        "bbbb\n"
        "cccc\n"
        "[pages]\n"
        "  dddd\n"
        "eeee\n"
        "ffff\n"
        "[tags]\n"
        "gggg\n"
        "\n"
        "  hhhh\n"
        "iiii\n"
        "[copy]\n"
        "jjjj\n"
        "kkkk\n"
        "llll\n";
    bc_error_t *err = NULL;
    bm_settings_t *s = bm_settings_parse(a, strlen(a), &err);
    assert_null(err);
    assert_non_null(s);
    assert_int_equal(bc_trie_size(s->global), 7);
    assert_string_equal(bc_trie_lookup(s->global, "BOLA"), "asd");
    assert_string_equal(bc_trie_lookup(s->global, "GUDA"), "qwe");
    assert_string_equal(bc_trie_lookup(s->global, "AUTHOR_NAME"), "chunda");
    assert_string_equal(bc_trie_lookup(s->global, "AUTHOR_EMAIL"), "chunda@example.com");
    assert_string_equal(bc_trie_lookup(s->global, "SITE_TITLE"), "Fuuuuuuuuu");
    assert_string_equal(bc_trie_lookup(s->global, "SITE_TAGLINE"), "My cool tagline");
    assert_string_equal(bc_trie_lookup(s->global, "BASE_DOMAIN"), "http://example.com");
    assert_int_equal(bc_trie_size(s->settings), 16);
    assert_string_equal(bc_trie_lookup(s->settings, "source_ext"), ".txt");
    assert_string_equal(bc_trie_lookup(s->settings, "html_ext"), "/index.html");
    assert_string_equal(bc_trie_lookup(s->settings, "content_dir"), "guda");
    assert_string_equal(bc_trie_lookup(s->settings, "template_dir"), "templates");
    assert_string_equal(bc_trie_lookup(s->settings, "main_template"), "foo.tmpl");
    assert_string_equal(bc_trie_lookup(s->settings, "date_format"),
        "%b %d, %Y, %I:%M %p GMT");
    assert_string_equal(bc_trie_lookup(s->settings, "posts_per_page"), "10");
    assert_string_equal(bc_trie_lookup(s->settings, "atom_prefix"), "atom");
    assert_string_equal(bc_trie_lookup(s->settings, "atom_ext"), ".xml");
    assert_string_equal(bc_trie_lookup(s->settings, "atom_posts_per_page"), "10");
    assert_string_equal(bc_trie_lookup(s->settings, "pagination_prefix"), "page");
    assert_string_equal(bc_trie_lookup(s->settings, "post_prefix"), "post");
    assert_string_equal(bc_trie_lookup(s->settings, "tag_prefix"), "tag");
    assert_string_equal(bc_trie_lookup(s->settings, "html_order"), "DESC");
    assert_string_equal(bc_trie_lookup(s->settings, "atom_order"), "DESC");
    assert_string_equal(bc_trie_lookup(s->settings, "index_prefix"), "");
    assert_non_null(s->posts);
    assert_string_equal(s->posts[0], "aaaa");
    assert_string_equal(s->posts[1], "bbbb");
    assert_string_equal(s->posts[2], "cccc");
    assert_null(s->posts[3]);
    assert_non_null(s->pages);
    assert_string_equal(s->pages[0], "dddd");
    assert_string_equal(s->pages[1], "eeee");
    assert_string_equal(s->pages[2], "ffff");
    assert_null(s->pages[3]);
    assert_non_null(s->copy);
    assert_string_equal(s->copy[0], "jjjj");
    assert_string_equal(s->copy[1], "kkkk");
    assert_string_equal(s->copy[2], "llll");
    assert_null(s->copy[3]);
    assert_non_null(s->tags);
    assert_string_equal(s->tags[0], "gggg");
    assert_string_equal(s->tags[1], "hhhh");
    assert_string_equal(s->tags[2], "iiii");
    assert_null(s->tags[3]);
    bm_settings_free(s);
}


static void
test_settings_env2(void **state)
{
    const char *a =
        "[settings]\n"
        "content_dir = guda\n"
        "main_template = foo.tmpl\n"
        "\n"
        "[environment]\n"
        "BOLA = asd\n"
        "GUDA = qwe\n"
        "AUTHOR_NAME = chunda\n"
        "AUTHOR_EMAIL = chunda@example.com\n"
        "SITE_TITLE = Fuuuuuuuuu\n"
        "SITE_TAGLINE = My cool tagline\n"
        "BASE_DOMAIN = http://example.com\n"
        "\n"
        "[posts]\n"
        "\n"
        "aaaa\n"
        "bbbb\n"
        "cccc\n"
        "[pages]\n"
        "  dddd\n"
        "eeee\n"
        "ffff\n"
        "[tags]\n"
        "gggg\n"
        "\n"
        "  hhhh\n"
        "iiii\n"
        "[copy]\n"
        "jjjj\n"
        "kkkk\n"
        "llll\n";
    bc_error_t *err = NULL;
    bm_settings_t *s = bm_settings_parse(a, strlen(a), &err);
    assert_null(err);
    assert_non_null(s);
    assert_int_equal(bc_trie_size(s->global), 7);
    assert_string_equal(bc_trie_lookup(s->global, "BOLA"), "asd");
    assert_string_equal(bc_trie_lookup(s->global, "GUDA"), "qwe");
    assert_string_equal(bc_trie_lookup(s->global, "AUTHOR_NAME"), "chunda");
    assert_string_equal(bc_trie_lookup(s->global, "AUTHOR_EMAIL"), "chunda@example.com");
    assert_string_equal(bc_trie_lookup(s->global, "SITE_TITLE"), "Fuuuuuuuuu");
    assert_string_equal(bc_trie_lookup(s->global, "SITE_TAGLINE"), "My cool tagline");
    assert_string_equal(bc_trie_lookup(s->global, "BASE_DOMAIN"), "http://example.com");
    assert_int_equal(bc_trie_size(s->settings), 16);
    assert_string_equal(bc_trie_lookup(s->settings, "source_ext"), ".txt");
    assert_string_equal(bc_trie_lookup(s->settings, "html_ext"), "/index.html");
    assert_string_equal(bc_trie_lookup(s->settings, "content_dir"), "guda");
    assert_string_equal(bc_trie_lookup(s->settings, "template_dir"), "templates");
    assert_string_equal(bc_trie_lookup(s->settings, "main_template"), "foo.tmpl");
    assert_string_equal(bc_trie_lookup(s->settings, "date_format"),
        "%b %d, %Y, %I:%M %p GMT");
    assert_string_equal(bc_trie_lookup(s->settings, "posts_per_page"), "10");
    assert_string_equal(bc_trie_lookup(s->settings, "atom_prefix"), "atom");
    assert_string_equal(bc_trie_lookup(s->settings, "atom_ext"), ".xml");
    assert_string_equal(bc_trie_lookup(s->settings, "atom_posts_per_page"), "10");
    assert_string_equal(bc_trie_lookup(s->settings, "pagination_prefix"), "page");
    assert_string_equal(bc_trie_lookup(s->settings, "post_prefix"), "post");
    assert_string_equal(bc_trie_lookup(s->settings, "tag_prefix"), "tag");
    assert_string_equal(bc_trie_lookup(s->settings, "html_order"), "DESC");
    assert_string_equal(bc_trie_lookup(s->settings, "atom_order"), "DESC");
    assert_string_equal(bc_trie_lookup(s->settings, "index_prefix"), "");
    assert_non_null(s->posts);
    assert_string_equal(s->posts[0], "aaaa");
    assert_string_equal(s->posts[1], "bbbb");
    assert_string_equal(s->posts[2], "cccc");
    assert_null(s->posts[3]);
    assert_non_null(s->pages);
    assert_string_equal(s->pages[0], "dddd");
    assert_string_equal(s->pages[1], "eeee");
    assert_string_equal(s->pages[2], "ffff");
    assert_null(s->pages[3]);
    assert_non_null(s->copy);
    assert_string_equal(s->copy[0], "jjjj");
    assert_string_equal(s->copy[1], "kkkk");
    assert_string_equal(s->copy[2], "llll");
    assert_null(s->copy[3]);
    assert_non_null(s->tags);
    assert_string_equal(s->tags[0], "gggg");
    assert_string_equal(s->tags[1], "hhhh");
    assert_string_equal(s->tags[2], "iiii");
    assert_null(s->tags[3]);
    bm_settings_free(s);
}


static void
test_settings_copy_files(void **state)
{
    const char *a =
        "[settings]\n"
        "content_dir = guda\n"
        "main_template = foo.tmpl\n"
        "\n"
        "[global]\n"
        "BOLA = asd\n"
        "GUDA = qwe\n"
        "AUTHOR_NAME = chunda\n"
        "AUTHOR_EMAIL = chunda@example.com\n"
        "SITE_TITLE = Fuuuuuuuuu\n"
        "SITE_TAGLINE = My cool tagline\n"
        "BASE_DOMAIN = http://example.com\n"
        "\n"
        "[posts]\n"
        "\n"
        "aaaa\n"
        "bbbb\n"
        "cccc\n"
        "[pages]\n"
        "  dddd\n"
        "eeee\n"
        "ffff\n"
        "[tags]\n"
        "gggg\n"
        "\n"
        "  hhhh\n"
        "iiii\n"
        "[copy_files]\n"
        "jjjj\n"
        "kkkk\n"
        "llll\n";
    bc_error_t *err = NULL;
    bm_settings_t *s = bm_settings_parse(a, strlen(a), &err);
    assert_null(err);
    assert_non_null(s);
    assert_int_equal(bc_trie_size(s->global), 7);
    assert_string_equal(bc_trie_lookup(s->global, "BOLA"), "asd");
    assert_string_equal(bc_trie_lookup(s->global, "GUDA"), "qwe");
    assert_string_equal(bc_trie_lookup(s->global, "AUTHOR_NAME"), "chunda");
    assert_string_equal(bc_trie_lookup(s->global, "AUTHOR_EMAIL"), "chunda@example.com");
    assert_string_equal(bc_trie_lookup(s->global, "SITE_TITLE"), "Fuuuuuuuuu");
    assert_string_equal(bc_trie_lookup(s->global, "SITE_TAGLINE"), "My cool tagline");
    assert_string_equal(bc_trie_lookup(s->global, "BASE_DOMAIN"), "http://example.com");
    assert_int_equal(bc_trie_size(s->settings), 16);
    assert_string_equal(bc_trie_lookup(s->settings, "source_ext"), ".txt");
    assert_string_equal(bc_trie_lookup(s->settings, "html_ext"), "/index.html");
    assert_string_equal(bc_trie_lookup(s->settings, "content_dir"), "guda");
    assert_string_equal(bc_trie_lookup(s->settings, "template_dir"), "templates");
    assert_string_equal(bc_trie_lookup(s->settings, "main_template"), "foo.tmpl");
    assert_string_equal(bc_trie_lookup(s->settings, "date_format"),
        "%b %d, %Y, %I:%M %p GMT");
    assert_string_equal(bc_trie_lookup(s->settings, "posts_per_page"), "10");
    assert_string_equal(bc_trie_lookup(s->settings, "atom_prefix"), "atom");
    assert_string_equal(bc_trie_lookup(s->settings, "atom_ext"), ".xml");
    assert_string_equal(bc_trie_lookup(s->settings, "atom_posts_per_page"), "10");
    assert_string_equal(bc_trie_lookup(s->settings, "pagination_prefix"), "page");
    assert_string_equal(bc_trie_lookup(s->settings, "post_prefix"), "post");
    assert_string_equal(bc_trie_lookup(s->settings, "tag_prefix"), "tag");
    assert_string_equal(bc_trie_lookup(s->settings, "html_order"), "DESC");
    assert_string_equal(bc_trie_lookup(s->settings, "atom_order"), "DESC");
    assert_string_equal(bc_trie_lookup(s->settings, "index_prefix"), "");
    assert_non_null(s->posts);
    assert_string_equal(s->posts[0], "aaaa");
    assert_string_equal(s->posts[1], "bbbb");
    assert_string_equal(s->posts[2], "cccc");
    assert_null(s->posts[3]);
    assert_non_null(s->pages);
    assert_string_equal(s->pages[0], "dddd");
    assert_string_equal(s->pages[1], "eeee");
    assert_string_equal(s->pages[2], "ffff");
    assert_null(s->pages[3]);
    assert_non_null(s->copy);
    assert_string_equal(s->copy[0], "jjjj");
    assert_string_equal(s->copy[1], "kkkk");
    assert_string_equal(s->copy[2], "llll");
    assert_null(s->copy[3]);
    assert_non_null(s->tags);
    assert_string_equal(s->tags[0], "gggg");
    assert_string_equal(s->tags[1], "hhhh");
    assert_string_equal(s->tags[2], "iiii");
    assert_null(s->tags[3]);
    bm_settings_free(s);
}


int
main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_settings_empty),
        cmocka_unit_test(test_settings),
        cmocka_unit_test(test_settings_env),
        cmocka_unit_test(test_settings2),
        cmocka_unit_test(test_settings_env2),
        cmocka_unit_test(test_settings_copy_files),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
