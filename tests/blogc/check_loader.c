/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2019 Rafael G. Martins <rafael@rafaelmartins.eng.br>
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
#include <stdio.h>
#include "../../src/common/error.h"
#include "../../src/common/utils.h"
#include "../../src/blogc/template-parser.h"
#include "../../src/blogc/loader.h"


static void
test_get_filename(void **state)
{
    char *f = blogc_get_filename("/home/foo/asd/bola.txt");
    assert_string_equal(f, "bola");
    free(f);
    f = blogc_get_filename("/home/foo/asd/bola.guda.txt");
    assert_string_equal(f, "bola.guda");
    free(f);
    f = blogc_get_filename("bola.txt");
    assert_string_equal(f, "bola");
    free(f);
    f = blogc_get_filename("bola.guda.txt");
    assert_string_equal(f, "bola.guda");
    free(f);
    f = blogc_get_filename("/home/foo/asd/bola");
    assert_string_equal(f, "bola");
    free(f);
    f = blogc_get_filename("bola");
    assert_string_equal(f, "bola");
    free(f);
    f = blogc_get_filename("");
    assert_null(f);
    free(f);
    f = blogc_get_filename(NULL);
    assert_null(f);
    free(f);
}


char*
__wrap_bc_file_get_contents(const char *path, bool utf8, size_t *len, bc_error_t **err)
{
    assert_true(utf8);
    assert_null(*err);
    const char *_path = mock_type(const char*);
    if (_path != NULL)
        assert_string_equal(path, _path);
    char *rv = mock_type(char*);
    *len = 0;
    if (rv != NULL)
        *len = strlen(rv);
    return rv;
}


static void
test_template_parse_from_file(void **state)
{
    bc_error_t *err = NULL;
    will_return(__wrap_bc_file_get_contents, "bola");
    will_return(__wrap_bc_file_get_contents, bc_strdup("{{ BOLA }}\n"));
    bc_slist_t *l = blogc_template_parse_from_file("bola", &err);
    assert_null(err);
    assert_non_null(l);
    assert_int_equal(bc_slist_length(l), 2);
    blogc_template_free_ast(l);
}


static void
test_template_parse_from_file_null(void **state)
{
    bc_error_t *err = NULL;
    will_return(__wrap_bc_file_get_contents, "bola");
    will_return(__wrap_bc_file_get_contents, NULL);
    bc_slist_t *l = blogc_template_parse_from_file("bola", &err);
    assert_null(err);
    assert_null(l);
}


static void
test_source_parse_from_file(void **state)
{
    bc_error_t *err = NULL;
    will_return(__wrap_bc_file_get_contents, "bola.txt");
    will_return(__wrap_bc_file_get_contents, bc_strdup(
        "ASD: 123\n"
        "--------\n"
        "bola"));
    bc_trie_t *t = blogc_source_parse_from_file("bola.txt", &err);
    assert_null(err);
    assert_non_null(t);
    assert_int_equal(bc_trie_size(t), 6);
    assert_string_equal(bc_trie_lookup(t, "ASD"), "123");
    assert_string_equal(bc_trie_lookup(t, "FILENAME"), "bola");
    assert_string_equal(bc_trie_lookup(t, "EXCERPT"), "<p>bola</p>\n");
    assert_string_equal(bc_trie_lookup(t, "CONTENT"), "<p>bola</p>\n");
    assert_string_equal(bc_trie_lookup(t, "RAW_CONTENT"), "bola");
    assert_string_equal(bc_trie_lookup(t, "DESCRIPTION"), "bola");
    bc_trie_free(t);
}


static void
test_source_parse_from_file_null(void **state)
{
    bc_error_t *err = NULL;
    will_return(__wrap_bc_file_get_contents, "bola.txt");
    will_return(__wrap_bc_file_get_contents, NULL);
    bc_trie_t *t = blogc_source_parse_from_file("bola.txt", &err);
    assert_null(err);
    assert_null(t);
}


static void
test_source_parse_from_files(void **state)
{
    will_return(__wrap_bc_file_get_contents, "bola1.txt");
    will_return(__wrap_bc_file_get_contents, bc_strdup(
        "ASD: 123\n"
        "DATE: 2001-02-03 04:05:06\n"
        "--------\n"
        "bola"));
    will_return(__wrap_bc_file_get_contents, "bola2.txt");
    will_return(__wrap_bc_file_get_contents, bc_strdup(
        "ASD: 456\n"
        "DATE: 2002-02-03 04:05:06\n"
        "--------\n"
        "bola"));
    will_return(__wrap_bc_file_get_contents, "bola3.txt");
    will_return(__wrap_bc_file_get_contents, bc_strdup(
        "ASD: 789\n"
        "DATE: 2003-02-03 04:05:06\n"
        "--------\n"
        "bola"));
    bc_error_t *err = NULL;
    bc_slist_t *s = NULL;
    s = bc_slist_append(s, bc_strdup("bola1.txt"));
    s = bc_slist_append(s, bc_strdup("bola2.txt"));
    s = bc_slist_append(s, bc_strdup("bola3.txt"));
    bc_trie_t *c = bc_trie_new(free);
    bc_slist_t *t = blogc_source_parse_from_files(c, s, &err);
    assert_null(err);
    assert_non_null(t);
    assert_int_equal(bc_slist_length(t), 3);  // it is enough, no need to look at the items
    assert_int_equal(bc_trie_size(c), 4);
    assert_string_equal(bc_trie_lookup(c, "FILENAME_FIRST"), "bola1");
    assert_string_equal(bc_trie_lookup(c, "FILENAME_LAST"), "bola3");
    assert_string_equal(bc_trie_lookup(c, "DATE_FIRST"), "2001-02-03 04:05:06");
    assert_string_equal(bc_trie_lookup(c, "DATE_LAST"), "2003-02-03 04:05:06");
    bc_trie_free(c);
    bc_slist_free_full(s, free);
    bc_slist_free_full(t, (bc_free_func_t) bc_trie_free);
}


static void
test_source_parse_from_files_filter_reverse(void **state)
{
    will_return(__wrap_bc_file_get_contents, "bola3.txt");
    will_return(__wrap_bc_file_get_contents, bc_strdup(
        "ASD: 789\n"
        "DATE: 2003-02-03 04:05:06\n"
        "--------\n"
        "bola"));
    will_return(__wrap_bc_file_get_contents, "bola2.txt");
    will_return(__wrap_bc_file_get_contents, bc_strdup(
        "ASD: 456\n"
        "DATE: 2002-02-03 04:05:06\n"
        "TAGS: bola, chunda\n"
        "--------\n"
        "bola"));
    will_return(__wrap_bc_file_get_contents, "bola1.txt");
    will_return(__wrap_bc_file_get_contents, bc_strdup(
        "ASD: 123\n"
        "DATE: 2001-02-03 04:05:06\n"
        "TAGS: chunda\n"
        "--------\n"
        "bola"));
    bc_error_t *err = NULL;
    bc_slist_t *s = NULL;
    s = bc_slist_append(s, bc_strdup("bola1.txt"));
    s = bc_slist_append(s, bc_strdup("bola2.txt"));
    s = bc_slist_append(s, bc_strdup("bola3.txt"));
    bc_trie_t *c = bc_trie_new(free);
    bc_trie_insert(c, "FILTER_REVERSE", bc_strdup(""));
    bc_slist_t *t = blogc_source_parse_from_files(c, s, &err);
    assert_null(err);
    assert_non_null(t);
    assert_int_equal(bc_slist_length(t), 3);  // it is enough, no need to look at the items
    assert_int_equal(bc_trie_size(c), 5);
    assert_string_equal(bc_trie_lookup(c, "FILENAME_FIRST"), "bola3");
    assert_string_equal(bc_trie_lookup(c, "FILENAME_LAST"), "bola1");
    assert_string_equal(bc_trie_lookup(c, "DATE_FIRST"), "2003-02-03 04:05:06");
    assert_string_equal(bc_trie_lookup(c, "DATE_LAST"), "2001-02-03 04:05:06");
    assert_string_equal(bc_trie_lookup(c, "FILTER_REVERSE"), "");
    bc_trie_free(c);
    bc_slist_free_full(s, free);
    bc_slist_free_full(t, (bc_free_func_t) bc_trie_free);
}


static void
test_source_parse_from_files_filter_by_tag(void **state)
{
    will_return(__wrap_bc_file_get_contents, "bola1.txt");
    will_return(__wrap_bc_file_get_contents, bc_strdup(
        "ASD: 123\n"
        "DATE: 2001-02-03 04:05:06\n"
        "TAGS: chunda\n"
        "--------\n"
        "bola"));
    will_return(__wrap_bc_file_get_contents, "bola2.txt");
    will_return(__wrap_bc_file_get_contents, bc_strdup(
        "ASD: 456\n"
        "DATE: 2002-02-03 04:05:06\n"
        "TAGS: bola, chunda\n"
        "--------\n"
        "bola"));
    will_return(__wrap_bc_file_get_contents, "bola3.txt");
    will_return(__wrap_bc_file_get_contents, bc_strdup(
        "ASD: 789\n"
        "DATE: 2003-02-03 04:05:06\n"
        "--------\n"
        "bola"));
    bc_error_t *err = NULL;
    bc_slist_t *s = NULL;
    s = bc_slist_append(s, bc_strdup("bola1.txt"));
    s = bc_slist_append(s, bc_strdup("bola2.txt"));
    s = bc_slist_append(s, bc_strdup("bola3.txt"));
    bc_trie_t *c = bc_trie_new(free);
    bc_trie_insert(c, "FILTER_TAG", bc_strdup("chunda"));
    bc_slist_t *t = blogc_source_parse_from_files(c, s, &err);
    assert_null(err);
    assert_non_null(t);
    assert_int_equal(bc_slist_length(t), 2);  // it is enough, no need to look at the items
    assert_int_equal(bc_trie_size(c), 5);
    assert_string_equal(bc_trie_lookup(c, "FILENAME_FIRST"), "bola1");
    assert_string_equal(bc_trie_lookup(c, "FILENAME_LAST"), "bola2");
    assert_string_equal(bc_trie_lookup(c, "DATE_FIRST"), "2001-02-03 04:05:06");
    assert_string_equal(bc_trie_lookup(c, "DATE_LAST"), "2002-02-03 04:05:06");
    assert_string_equal(bc_trie_lookup(c, "FILTER_TAG"), "chunda");
    bc_trie_free(c);
    bc_slist_free_full(s, free);
    bc_slist_free_full(t, (bc_free_func_t) bc_trie_free);
}


static void
test_source_parse_from_files_filter_by_page(void **state)
{
    will_return(__wrap_bc_file_get_contents, "bola1.txt");
    will_return(__wrap_bc_file_get_contents, bc_strdup(
        "ASD: 123\n"
        "DATE: 2001-02-03 04:05:06\n"
        "--------\n"
        "bola"));
    will_return(__wrap_bc_file_get_contents, "bola2.txt");
    will_return(__wrap_bc_file_get_contents, bc_strdup(
        "ASD: 456\n"
        "DATE: 2002-02-03 04:05:06\n"
        "--------\n"
        "bola"));
    will_return(__wrap_bc_file_get_contents, "bola3.txt");
    will_return(__wrap_bc_file_get_contents, bc_strdup(
        "ASD: 789\n"
        "DATE: 2003-02-03 04:05:06\n"
        "--------\n"
        "bola"));
    will_return(__wrap_bc_file_get_contents, "bola4.txt");
    will_return(__wrap_bc_file_get_contents, bc_strdup(
        "ASD: 7891\n"
        "DATE: 2004-02-03 04:05:06\n"
        "--------\n"
        "bola"));
    will_return(__wrap_bc_file_get_contents, "bola5.txt");
    will_return(__wrap_bc_file_get_contents, bc_strdup(
        "ASD: 7892\n"
        "DATE: 2005-02-03 04:05:06\n"
        "--------\n"
        "bola"));
    will_return(__wrap_bc_file_get_contents, "bola6.txt");
    will_return(__wrap_bc_file_get_contents, bc_strdup(
        "ASD: 7893\n"
        "DATE: 2006-02-03 04:05:06\n"
        "--------\n"
        "bola"));
    will_return(__wrap_bc_file_get_contents, "bola7.txt");
    will_return(__wrap_bc_file_get_contents, bc_strdup(
        "ASD: 7894\n"
        "DATE: 2007-02-03 04:05:06\n"
        "--------\n"
        "bola"));
    bc_error_t *err = NULL;
    bc_slist_t *s = NULL;
    s = bc_slist_append(s, bc_strdup("bola1.txt"));
    s = bc_slist_append(s, bc_strdup("bola2.txt"));
    s = bc_slist_append(s, bc_strdup("bola3.txt"));
    s = bc_slist_append(s, bc_strdup("bola4.txt"));
    s = bc_slist_append(s, bc_strdup("bola5.txt"));
    s = bc_slist_append(s, bc_strdup("bola6.txt"));
    s = bc_slist_append(s, bc_strdup("bola7.txt"));
    bc_trie_t *c = bc_trie_new(free);
    bc_trie_insert(c, "FILTER_PAGE", bc_strdup("1"));
    bc_trie_insert(c, "FILTER_PER_PAGE", bc_strdup("2"));
    bc_slist_t *t = blogc_source_parse_from_files(c, s, &err);
    assert_null(err);
    assert_non_null(t);
    assert_int_equal(bc_slist_length(t), 2);  // it is enough, no need to look at the items
    assert_int_equal(bc_trie_size(c), 10);
    assert_string_equal(bc_trie_lookup(c, "FILENAME_FIRST"), "bola1");
    assert_string_equal(bc_trie_lookup(c, "FILENAME_LAST"), "bola2");
    assert_string_equal(bc_trie_lookup(c, "DATE_FIRST"), "2001-02-03 04:05:06");
    assert_string_equal(bc_trie_lookup(c, "DATE_LAST"), "2002-02-03 04:05:06");
    assert_string_equal(bc_trie_lookup(c, "FILTER_PAGE"), "1");
    assert_string_equal(bc_trie_lookup(c, "FILTER_PER_PAGE"), "2");
    assert_string_equal(bc_trie_lookup(c, "CURRENT_PAGE"), "1");
    assert_string_equal(bc_trie_lookup(c, "NEXT_PAGE"), "2");
    assert_string_equal(bc_trie_lookup(c, "FIRST_PAGE"), "1");
    assert_string_equal(bc_trie_lookup(c, "LAST_PAGE"), "4");
    bc_trie_free(c);
    bc_slist_free_full(s, free);
    bc_slist_free_full(t, (bc_free_func_t) bc_trie_free);
}


static void
test_source_parse_from_files_filter_by_page2(void **state)
{
    will_return(__wrap_bc_file_get_contents, "bola1.txt");
    will_return(__wrap_bc_file_get_contents, bc_strdup(
        "ASD: 123\n"
        "DATE: 2001-02-03 04:05:06\n"
        "--------\n"
        "bola"));
    will_return(__wrap_bc_file_get_contents, "bola2.txt");
    will_return(__wrap_bc_file_get_contents, bc_strdup(
        "ASD: 456\n"
        "DATE: 2002-02-03 04:05:06\n"
        "--------\n"
        "bola"));
    will_return(__wrap_bc_file_get_contents, "bola3.txt");
    will_return(__wrap_bc_file_get_contents, bc_strdup(
        "ASD: 789\n"
        "DATE: 2003-02-03 04:05:06\n"
        "--------\n"
        "bola"));
    will_return(__wrap_bc_file_get_contents, "bola4.txt");
    will_return(__wrap_bc_file_get_contents, bc_strdup(
        "ASD: 7891\n"
        "DATE: 2004-02-03 04:05:06\n"
        "--------\n"
        "bola"));
    will_return(__wrap_bc_file_get_contents, "bola5.txt");
    will_return(__wrap_bc_file_get_contents, bc_strdup(
        "ASD: 7892\n"
        "DATE: 2005-02-03 04:05:06\n"
        "--------\n"
        "bola"));
    will_return(__wrap_bc_file_get_contents, "bola6.txt");
    will_return(__wrap_bc_file_get_contents, bc_strdup(
        "ASD: 7893\n"
        "DATE: 2006-02-03 04:05:06\n"
        "--------\n"
        "bola"));
    will_return(__wrap_bc_file_get_contents, "bola7.txt");
    will_return(__wrap_bc_file_get_contents, bc_strdup(
        "ASD: 7894\n"
        "DATE: 2007-02-03 04:05:06\n"
        "--------\n"
        "bola"));
    bc_error_t *err = NULL;
    bc_slist_t *s = NULL;
    s = bc_slist_append(s, bc_strdup("bola1.txt"));
    s = bc_slist_append(s, bc_strdup("bola2.txt"));
    s = bc_slist_append(s, bc_strdup("bola3.txt"));
    s = bc_slist_append(s, bc_strdup("bola4.txt"));
    s = bc_slist_append(s, bc_strdup("bola5.txt"));
    s = bc_slist_append(s, bc_strdup("bola6.txt"));
    s = bc_slist_append(s, bc_strdup("bola7.txt"));
    bc_trie_t *c = bc_trie_new(free);
    bc_trie_insert(c, "FILTER_PAGE", bc_strdup("3"));
    bc_trie_insert(c, "FILTER_PER_PAGE", bc_strdup("2"));
    bc_slist_t *t = blogc_source_parse_from_files(c, s, &err);
    assert_null(err);
    assert_non_null(t);
    assert_int_equal(bc_slist_length(t), 2);  // it is enough, no need to look at the items
    assert_int_equal(bc_trie_size(c), 11);
    assert_string_equal(bc_trie_lookup(c, "FILENAME_FIRST"), "bola5");
    assert_string_equal(bc_trie_lookup(c, "FILENAME_LAST"), "bola6");
    assert_string_equal(bc_trie_lookup(c, "DATE_FIRST"), "2005-02-03 04:05:06");
    assert_string_equal(bc_trie_lookup(c, "DATE_LAST"), "2006-02-03 04:05:06");
    assert_string_equal(bc_trie_lookup(c, "FILTER_PAGE"), "3");
    assert_string_equal(bc_trie_lookup(c, "FILTER_PER_PAGE"), "2");
    assert_string_equal(bc_trie_lookup(c, "CURRENT_PAGE"), "3");
    assert_string_equal(bc_trie_lookup(c, "PREVIOUS_PAGE"), "2");
    assert_string_equal(bc_trie_lookup(c, "NEXT_PAGE"), "4");
    assert_string_equal(bc_trie_lookup(c, "FIRST_PAGE"), "1");
    assert_string_equal(bc_trie_lookup(c, "LAST_PAGE"), "4");
    bc_trie_free(c);
    bc_slist_free_full(s, free);
    bc_slist_free_full(t, (bc_free_func_t) bc_trie_free);
}


static void
test_source_parse_from_files_filter_by_page3(void **state)
{
    will_return(__wrap_bc_file_get_contents, "bola1.txt");
    will_return(__wrap_bc_file_get_contents, bc_strdup(
        "ASD: 123\n"
        "DATE: 2001-02-03 04:05:06\n"
        "--------\n"
        "bola"));
    will_return(__wrap_bc_file_get_contents, "bola2.txt");
    will_return(__wrap_bc_file_get_contents, bc_strdup(
        "ASD: 456\n"
        "DATE: 2002-02-03 04:05:06\n"
        "--------\n"
        "bola"));
    will_return(__wrap_bc_file_get_contents, "bola3.txt");
    will_return(__wrap_bc_file_get_contents, bc_strdup(
        "ASD: 789\n"
        "DATE: 2003-02-03 04:05:06\n"
        "--------\n"
        "bola"));
    will_return(__wrap_bc_file_get_contents, "bola4.txt");
    will_return(__wrap_bc_file_get_contents, bc_strdup(
        "ASD: 7891\n"
        "DATE: 2004-02-03 04:05:06\n"
        "--------\n"
        "bola"));
    will_return(__wrap_bc_file_get_contents, "bola5.txt");
    will_return(__wrap_bc_file_get_contents, bc_strdup(
        "ASD: 7892\n"
        "DATE: 2005-02-03 04:05:06\n"
        "--------\n"
        "bola"));
    will_return(__wrap_bc_file_get_contents, "bola6.txt");
    will_return(__wrap_bc_file_get_contents, bc_strdup(
        "ASD: 7893\n"
        "DATE: 2006-02-03 04:05:06\n"
        "--------\n"
        "bola"));
    will_return(__wrap_bc_file_get_contents, "bola7.txt");
    will_return(__wrap_bc_file_get_contents, bc_strdup(
        "ASD: 7894\n"
        "DATE: 2007-02-03 04:05:06\n"
        "--------\n"
        "bola"));
    bc_error_t *err = NULL;
    bc_slist_t *s = NULL;
    s = bc_slist_append(s, bc_strdup("bola1.txt"));
    s = bc_slist_append(s, bc_strdup("bola2.txt"));
    s = bc_slist_append(s, bc_strdup("bola3.txt"));
    s = bc_slist_append(s, bc_strdup("bola4.txt"));
    s = bc_slist_append(s, bc_strdup("bola5.txt"));
    s = bc_slist_append(s, bc_strdup("bola6.txt"));
    s = bc_slist_append(s, bc_strdup("bola7.txt"));
    bc_trie_t *c = bc_trie_new(free);
    bc_trie_insert(c, "FILTER_PAGE", bc_strdup("1"));
    bc_trie_insert(c, "FILTER_PER_PAGE", bc_strdup("2"));
    bc_slist_t *t = blogc_source_parse_from_files(c, s, &err);
    assert_null(err);
    assert_non_null(t);
    assert_int_equal(bc_slist_length(t), 2);  // it is enough, no need to look at the items
    assert_int_equal(bc_trie_size(c), 10);
    assert_string_equal(bc_trie_lookup(c, "FILENAME_FIRST"), "bola1");
    assert_string_equal(bc_trie_lookup(c, "FILENAME_LAST"), "bola2");
    assert_string_equal(bc_trie_lookup(c, "DATE_FIRST"), "2001-02-03 04:05:06");
    assert_string_equal(bc_trie_lookup(c, "DATE_LAST"), "2002-02-03 04:05:06");
    assert_string_equal(bc_trie_lookup(c, "FILTER_PAGE"), "1");
    assert_string_equal(bc_trie_lookup(c, "FILTER_PER_PAGE"), "2");
    assert_string_equal(bc_trie_lookup(c, "CURRENT_PAGE"), "1");
    assert_string_equal(bc_trie_lookup(c, "NEXT_PAGE"), "2");
    assert_string_equal(bc_trie_lookup(c, "FIRST_PAGE"), "1");
    assert_string_equal(bc_trie_lookup(c, "LAST_PAGE"), "4");
    bc_trie_free(c);
    bc_slist_free_full(s, free);
    bc_slist_free_full(t, (bc_free_func_t) bc_trie_free);
}


static void
test_source_parse_from_files_filter_by_page_and_tag(void **state)
{
    will_return(__wrap_bc_file_get_contents, "bola1.txt");
    will_return(__wrap_bc_file_get_contents, bc_strdup(
        "ASD: 123\n"
        "DATE: 2001-02-03 04:05:06\n"
        "--------\n"
        "bola"));
    will_return(__wrap_bc_file_get_contents, "bola2.txt");
    will_return(__wrap_bc_file_get_contents, bc_strdup(
        "ASD: 456\n"
        "DATE: 2002-02-03 04:05:06\n"
        "TAGS: chunda\n"
        "--------\n"
        "bola"));
    will_return(__wrap_bc_file_get_contents, "bola3.txt");
    will_return(__wrap_bc_file_get_contents, bc_strdup(
        "ASD: 789\n"
        "DATE: 2003-02-03 04:05:06\n"
        "TAGS: chunda bola\n"
        "--------\n"
        "bola"));
    will_return(__wrap_bc_file_get_contents, "bola4.txt");
    will_return(__wrap_bc_file_get_contents, bc_strdup(
        "ASD: 7891\n"
        "DATE: 2004-02-03 04:05:06\n"
        "TAGS: bola\n"
        "--------\n"
        "bola"));
    will_return(__wrap_bc_file_get_contents, "bola5.txt");
    will_return(__wrap_bc_file_get_contents, bc_strdup(
        "ASD: 7892\n"
        "DATE: 2005-02-03 04:05:06\n"
        "TAGS: chunda\n"
        "--------\n"
        "bola"));
    will_return(__wrap_bc_file_get_contents, "bola6.txt");
    will_return(__wrap_bc_file_get_contents, bc_strdup(
        "ASD: 7893\n"
        "DATE: 2006-02-03 04:05:06\n"
        "--------\n"
        "bola"));
    will_return(__wrap_bc_file_get_contents, "bola7.txt");
    will_return(__wrap_bc_file_get_contents, bc_strdup(
        "ASD: 7894\n"
        "DATE: 2007-02-03 04:05:06\n"
        "TAGS: yay chunda\n"
        "--------\n"
        "bola"));
    bc_error_t *err = NULL;
    bc_slist_t *s = NULL;
    s = bc_slist_append(s, bc_strdup("bola1.txt"));
    s = bc_slist_append(s, bc_strdup("bola2.txt"));
    s = bc_slist_append(s, bc_strdup("bola3.txt"));
    s = bc_slist_append(s, bc_strdup("bola4.txt"));
    s = bc_slist_append(s, bc_strdup("bola5.txt"));
    s = bc_slist_append(s, bc_strdup("bola6.txt"));
    s = bc_slist_append(s, bc_strdup("bola7.txt"));
    bc_trie_t *c = bc_trie_new(free);
    bc_trie_insert(c, "FILTER_TAG", bc_strdup("chunda"));
    bc_trie_insert(c, "FILTER_PAGE", bc_strdup("2"));
    bc_trie_insert(c, "FILTER_PER_PAGE", bc_strdup("2"));
    bc_slist_t *t = blogc_source_parse_from_files(c, s, &err);
    assert_null(err);
    assert_non_null(t);
    assert_int_equal(bc_slist_length(t), 2);  // it is enough, no need to look at the items
    assert_int_equal(bc_trie_size(c), 11);
    assert_string_equal(bc_trie_lookup(c, "FILENAME_FIRST"), "bola5");
    assert_string_equal(bc_trie_lookup(c, "FILENAME_LAST"), "bola7");
    assert_string_equal(bc_trie_lookup(c, "DATE_FIRST"), "2005-02-03 04:05:06");
    assert_string_equal(bc_trie_lookup(c, "DATE_LAST"), "2007-02-03 04:05:06");
    assert_string_equal(bc_trie_lookup(c, "FILTER_TAG"), "chunda");
    assert_string_equal(bc_trie_lookup(c, "FILTER_PAGE"), "2");
    assert_string_equal(bc_trie_lookup(c, "FILTER_PER_PAGE"), "2");
    assert_string_equal(bc_trie_lookup(c, "CURRENT_PAGE"), "2");
    assert_string_equal(bc_trie_lookup(c, "PREVIOUS_PAGE"), "1");
    assert_string_equal(bc_trie_lookup(c, "FIRST_PAGE"), "1");
    assert_string_equal(bc_trie_lookup(c, "LAST_PAGE"), "2");
    bc_trie_free(c);
    bc_slist_free_full(s, free);
    bc_slist_free_full(t, (bc_free_func_t) bc_trie_free);
}


static void
test_source_parse_from_files_filter_by_page_invalid(void **state)
{
    will_return(__wrap_bc_file_get_contents, "bola1.txt");
    will_return(__wrap_bc_file_get_contents, bc_strdup(
        "ASD: 123\n"
        "DATE: 2001-02-03 04:05:06\n"
        "--------\n"
        "bola"));
    will_return(__wrap_bc_file_get_contents, "bola2.txt");
    will_return(__wrap_bc_file_get_contents, bc_strdup(
        "ASD: 456\n"
        "DATE: 2002-02-03 04:05:06\n"
        "--------\n"
        "bola"));
    will_return(__wrap_bc_file_get_contents, "bola3.txt");
    will_return(__wrap_bc_file_get_contents, bc_strdup(
        "ASD: 789\n"
        "DATE: 2003-02-03 04:05:06\n"
        "--------\n"
        "bola"));
    will_return(__wrap_bc_file_get_contents, "bola4.txt");
    will_return(__wrap_bc_file_get_contents, bc_strdup(
        "ASD: 7891\n"
        "DATE: 2004-02-03 04:05:06\n"
        "--------\n"
        "bola"));
    will_return(__wrap_bc_file_get_contents, "bola5.txt");
    will_return(__wrap_bc_file_get_contents, bc_strdup(
        "ASD: 7892\n"
        "DATE: 2005-02-03 04:05:06\n"
        "--------\n"
        "bola"));
    will_return(__wrap_bc_file_get_contents, "bola6.txt");
    will_return(__wrap_bc_file_get_contents, bc_strdup(
        "ASD: 7893\n"
        "DATE: 2006-02-03 04:05:06\n"
        "--------\n"
        "bola"));
    will_return(__wrap_bc_file_get_contents, "bola7.txt");
    will_return(__wrap_bc_file_get_contents, bc_strdup(
        "ASD: 7894\n"
        "DATE: 2007-02-03 04:05:06\n"
        "--------\n"
        "bola"));
    bc_error_t *err = NULL;
    bc_slist_t *s = NULL;
    s = bc_slist_append(s, bc_strdup("bola1.txt"));
    s = bc_slist_append(s, bc_strdup("bola2.txt"));
    s = bc_slist_append(s, bc_strdup("bola3.txt"));
    s = bc_slist_append(s, bc_strdup("bola4.txt"));
    s = bc_slist_append(s, bc_strdup("bola5.txt"));
    s = bc_slist_append(s, bc_strdup("bola6.txt"));
    s = bc_slist_append(s, bc_strdup("bola7.txt"));
    bc_trie_t *c = bc_trie_new(free);
    bc_trie_insert(c, "FILTER_PAGE", bc_strdup("-1"));
    bc_trie_insert(c, "FILTER_PER_PAGE", bc_strdup("2"));
    bc_slist_t *t = blogc_source_parse_from_files(c, s, &err);
    assert_null(err);
    assert_non_null(t);
    assert_int_equal(bc_slist_length(t), 2);  // it is enough, no need to look at the items
    assert_int_equal(bc_trie_size(c), 10);
    assert_string_equal(bc_trie_lookup(c, "FILENAME_FIRST"), "bola1");
    assert_string_equal(bc_trie_lookup(c, "FILENAME_LAST"), "bola2");
    assert_string_equal(bc_trie_lookup(c, "DATE_FIRST"), "2001-02-03 04:05:06");
    assert_string_equal(bc_trie_lookup(c, "DATE_LAST"), "2002-02-03 04:05:06");
    assert_string_equal(bc_trie_lookup(c, "FILTER_PAGE"), "-1");
    assert_string_equal(bc_trie_lookup(c, "FILTER_PER_PAGE"), "2");
    assert_string_equal(bc_trie_lookup(c, "CURRENT_PAGE"), "1");
    assert_string_equal(bc_trie_lookup(c, "NEXT_PAGE"), "2");
    assert_string_equal(bc_trie_lookup(c, "FIRST_PAGE"), "1");
    assert_string_equal(bc_trie_lookup(c, "LAST_PAGE"), "4");
    bc_trie_free(c);
    bc_slist_free_full(s, free);
    bc_slist_free_full(t, (bc_free_func_t) bc_trie_free);
}


static void
test_source_parse_from_files_filter_by_page_invalid2(void **state)
{
    will_return(__wrap_bc_file_get_contents, "bola1.txt");
    will_return(__wrap_bc_file_get_contents, bc_strdup(
        "ASD: 123\n"
        "DATE: 2001-02-03 04:05:06\n"
        "--------\n"
        "bola"));
    will_return(__wrap_bc_file_get_contents, "bola2.txt");
    will_return(__wrap_bc_file_get_contents, bc_strdup(
        "ASD: 456\n"
        "DATE: 2002-02-03 04:05:06\n"
        "--------\n"
        "bola"));
    will_return(__wrap_bc_file_get_contents, "bola3.txt");
    will_return(__wrap_bc_file_get_contents, bc_strdup(
        "ASD: 789\n"
        "DATE: 2003-02-03 04:05:06\n"
        "--------\n"
        "bola"));
    will_return(__wrap_bc_file_get_contents, "bola4.txt");
    will_return(__wrap_bc_file_get_contents, bc_strdup(
        "ASD: 7891\n"
        "DATE: 2004-02-03 04:05:06\n"
        "--------\n"
        "bola"));
    will_return(__wrap_bc_file_get_contents, "bola5.txt");
    will_return(__wrap_bc_file_get_contents, bc_strdup(
        "ASD: 7892\n"
        "DATE: 2005-02-03 04:05:06\n"
        "--------\n"
        "bola"));
    will_return(__wrap_bc_file_get_contents, "bola6.txt");
    will_return(__wrap_bc_file_get_contents, bc_strdup(
        "ASD: 7893\n"
        "DATE: 2006-02-03 04:05:06\n"
        "--------\n"
        "bola"));
    will_return(__wrap_bc_file_get_contents, "bola7.txt");
    will_return(__wrap_bc_file_get_contents, bc_strdup(
        "ASD: 7894\n"
        "DATE: 2007-02-03 04:05:06\n"
        "--------\n"
        "bola"));
    bc_error_t *err = NULL;
    bc_slist_t *s = NULL;
    s = bc_slist_append(s, bc_strdup("bola1.txt"));
    s = bc_slist_append(s, bc_strdup("bola2.txt"));
    s = bc_slist_append(s, bc_strdup("bola3.txt"));
    s = bc_slist_append(s, bc_strdup("bola4.txt"));
    s = bc_slist_append(s, bc_strdup("bola5.txt"));
    s = bc_slist_append(s, bc_strdup("bola6.txt"));
    s = bc_slist_append(s, bc_strdup("bola7.txt"));
    bc_trie_t *c = bc_trie_new(free);
    bc_trie_insert(c, "FILTER_PAGE", bc_strdup("5"));
    bc_trie_insert(c, "FILTER_PER_PAGE", bc_strdup("2"));
    bc_slist_t *t = blogc_source_parse_from_files(c, s, &err);
    assert_null(err);
    assert_null(t);
    bc_trie_free(c);
    bc_slist_free_full(s, free);
}


static void
test_source_parse_from_files_without_all_dates(void **state)
{
    will_return(__wrap_bc_file_get_contents, "bola1.txt");
    will_return(__wrap_bc_file_get_contents, bc_strdup(
        "ASD: 123\n"
        "--------\n"
        "bola"));
    will_return(__wrap_bc_file_get_contents, "bola2.txt");
    will_return(__wrap_bc_file_get_contents, bc_strdup(
        "ASD: 456\n"
        "DATE: 2002-02-03 04:05:06\n"
        "--------\n"
        "bola"));
    will_return(__wrap_bc_file_get_contents, "bola3.txt");
    will_return(__wrap_bc_file_get_contents, bc_strdup(
        "ASD: 789\n"
        "DATE: 2003-02-03 04:05:06\n"
        "--------\n"
        "bola"));
    bc_error_t *err = NULL;
    bc_slist_t *s = NULL;
    s = bc_slist_append(s, bc_strdup("bola1.txt"));
    s = bc_slist_append(s, bc_strdup("bola2.txt"));
    s = bc_slist_append(s, bc_strdup("bola3.txt"));
    bc_trie_t *c = bc_trie_new(free);
    bc_slist_t *t = blogc_source_parse_from_files(c, s, &err);
    assert_null(t);
    assert_non_null(err);
    assert_int_equal(err->type, BLOGC_ERROR_LOADER);
    assert_string_equal(err->msg,
        "'DATE' variable provided for at least one source file, but not for "
        "all source files. It must be provided for all files.\n");
    bc_error_free(err);
    assert_int_equal(bc_trie_size(c), 0);
    bc_trie_free(c);
    bc_slist_free_full(s, free);
}


static void
test_source_parse_from_files_null(void **state)
{
    bc_error_t *err = NULL;
    bc_slist_t *s = NULL;
    bc_trie_t *c = bc_trie_new(free);
    bc_slist_t *t = blogc_source_parse_from_files(c, s, &err);
    assert_null(err);
    assert_null(t);
    assert_int_equal(bc_slist_length(t), 0);
    assert_int_equal(bc_trie_size(c), 0);
    bc_trie_free(c);
    bc_slist_free_full(s, free);
    bc_slist_free_full(t, (bc_free_func_t) bc_trie_free);
}


int
main(void)
{
    const UnitTest tests[] = {
        unit_test(test_get_filename),
        unit_test(test_template_parse_from_file),
        unit_test(test_template_parse_from_file_null),
        unit_test(test_source_parse_from_file),
        unit_test(test_source_parse_from_file_null),
        unit_test(test_source_parse_from_files),
        unit_test(test_source_parse_from_files_filter_reverse),
        unit_test(test_source_parse_from_files_filter_by_tag),
        unit_test(test_source_parse_from_files_filter_by_page),
        unit_test(test_source_parse_from_files_filter_by_page2),
        unit_test(test_source_parse_from_files_filter_by_page3),
        unit_test(test_source_parse_from_files_filter_by_page_and_tag),
        unit_test(test_source_parse_from_files_filter_by_page_invalid),
        unit_test(test_source_parse_from_files_filter_by_page_invalid2),
        unit_test(test_source_parse_from_files_without_all_dates),
        unit_test(test_source_parse_from_files_null),
    };
    return run_tests(tests);
}
