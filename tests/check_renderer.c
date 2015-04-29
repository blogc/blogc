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
#include "../src/error.h"
#include "../src/renderer.h"
#include "../src/source-parser.h"
#include "../src/template-parser.h"
#include "../src/utils/utils.h"


static b_slist_t*
create_sources(unsigned int count)
{
    const char *s[] = {
        "BOLA: asd\n"
        "GUDA: zxc\n"
        "DATE: 2015-01-02 03:04:05\n"
        "DATE_FORMAT: %R\n"
        "-----\n"
        "ahahahahahahahaha",
        "BOLA: asd2\n"
        "GUDA: zxc2\n"
        "DATE: 2014-02-03 04:05:06\n"
        "-----\n"
        "ahahahahahahahaha2",
        "BOLA: asd3\n"
        "GUDA: zxc3\n"
        "-----\n"
        "ahahahahahahahaha3",
    };
    assert_false(count > 3);
    blogc_error_t *err = NULL;
    b_slist_t *l = NULL;
    for (unsigned int i = 0; i < count; i++) {
        l = b_slist_append(l, blogc_source_parse(s[i], strlen(s[i]), &err));
        assert_null(err);
    }
    assert_int_equal(b_slist_length(l), count);
    return l;
}


static void
test_render_entry(void **state)
{
    const char *str =
        "foo\n"
        "{% block listing_once %}fuuu{% endblock %}\n"
        "{% block entry %}\n"
        "{{ DATE }}\n"
        "{% if DATE_FORMATTED %}{{ DATE_FORMATTED }}{% endif %}\n"
        "{% if GUDA %}{{ GUDA }}{% endif %}\n"
        "{% if CHUNDA %}{{ CHUNDA }}{% endif %}\n"
        "{% endblock %}\n"
        "{% block listing %}lol{% endblock %}\n";
    blogc_error_t *err = NULL;
    b_slist_t *l = blogc_template_parse(str, strlen(str), &err);
    assert_non_null(l);
    assert_null(err);
    b_slist_t *s = create_sources(1);
    assert_non_null(s);
    char *out = blogc_render(l, s, NULL, false);
    assert_string_equal(out,
        "foo\n"
        "\n"
        "\n"
        "2015-01-02 03:04:05\n"
        "03:04\n"
        "zxc\n"
        "\n"
        "\n"
        "\n");
    blogc_template_free_stmts(l);
    b_slist_free_full(s, (b_free_func_t) b_trie_free);
    free(out);
}


static void
test_render_listing(void **state)
{
    const char *str =
        "foo\n"
        "{% block listing_once %}fuuu{% endblock %}\n"
        "{% block entry %}\n"
        "{% if GUDA %}{{ GUDA }}{% endif %}\n"
        "{% if CHUNDA %}{{ CHUNDA }}{% endif %}\n"
        "{% endblock %}\n"
        "{% block listing %}\n"
        "{% if DATE_FORMATTED %}{{ DATE_FORMATTED }}{% endif %}\n"
        "bola: {% if BOLA %}{{ BOLA }}{% endif %}\n"
        "{% endblock %}\n";
    blogc_error_t *err = NULL;
    b_slist_t *l = blogc_template_parse(str, strlen(str), &err);
    assert_non_null(l);
    assert_null(err);
    b_slist_t *s = create_sources(3);
    assert_non_null(s);
    char *out = blogc_render(l, s, NULL, true);
    assert_string_equal(out,
        "foo\n"
        "fuuu\n"
        "\n"
        "\n"
        "03:04\n"
        "bola: asd\n"
        "\n"
        "2014-02-03 04:05:06\n"
        "bola: asd2\n"
        "\n"
        "\n"
        "bola: asd3\n"
        "\n");
    blogc_template_free_stmts(l);
    b_slist_free_full(s, (b_free_func_t) b_trie_free);
    free(out);
}


static void
test_render_if(void **state)
{
    const char *str =
        "{% block entry %}\n"
        "{% if CHUNDA %}chunda\n"
        "{% if GUDA %}guda\n"
        "{% if BOLA %}bola\n"
        "{% endif %}\n"
        "{% endif %}\n"
        "{% endif %}\n"
        "{% endblock %}\n";
    blogc_error_t *err = NULL;
    b_slist_t *l = blogc_template_parse(str, strlen(str), &err);
    assert_non_null(l);
    assert_null(err);
    b_slist_t *s = create_sources(1);
    assert_non_null(s);
    char *out = blogc_render(l, s, NULL, false);
    assert_string_equal(out,
        "\n"
        "\n"
        "\n");
    blogc_template_free_stmts(l);
    b_slist_free_full(s, (b_free_func_t) b_trie_free);
    free(out);
}


static void
test_render_if2(void **state)
{
    const char *str =
        "{% block entry %}\n"
        "{% if GUDA %}guda\n"
        "{% if CHUNDA %}chunda\n"
        "{% if BOLA %}bola\n"
        "{% endif %}\n"
        "{% endif %}\n"
        "{% endif %}\n"
        "{% endblock %}\n";
    blogc_error_t *err = NULL;
    b_slist_t *l = blogc_template_parse(str, strlen(str), &err);
    assert_non_null(l);
    assert_null(err);
    b_slist_t *s = create_sources(1);
    assert_non_null(s);
    char *out = blogc_render(l, s, NULL, false);
    assert_string_equal(out,
        "\n"
        "guda\n"
        "\n"
        "\n"
        "\n");
    blogc_template_free_stmts(l);
    b_slist_free_full(s, (b_free_func_t) b_trie_free);
    free(out);
}


static void
test_render_if3(void **state)
{
    const char *str =
        "{% block entry %}\n"
        "{% if GUDA %}guda\n"
        "{% if BOLA %}bola\n"
        "{% if CHUNDA %}chunda\n"
        "{% endif %}\n"
        "{% endif %}\n"
        "{% endif %}\n"
        "{% endblock %}\n";
    blogc_error_t *err = NULL;
    b_slist_t *l = blogc_template_parse(str, strlen(str), &err);
    assert_non_null(l);
    assert_null(err);
    b_slist_t *s = create_sources(1);
    assert_non_null(s);
    char *out = blogc_render(l, s, NULL, false);
    assert_string_equal(out,
        "\n"
        "guda\n"
        "bola\n"
        "\n"
        "\n"
        "\n"
        "\n");
    blogc_template_free_stmts(l);
    b_slist_free_full(s, (b_free_func_t) b_trie_free);
    free(out);
}


static void
test_render_if_not(void **state)
{
    const char *str =
        "{% block entry %}\n"
        "{% if not CHUNDA %}chunda\n"
        "{% if GUDA %}guda\n"
        "{% if not BOLA %}bola\n"
        "{% endif %}\n"
        "{% endif %}\n"
        "{% endif %}\n"
        "{% endblock %}\n";
    blogc_error_t *err = NULL;
    b_slist_t *l = blogc_template_parse(str, strlen(str), &err);
    assert_non_null(l);
    assert_null(err);
    b_slist_t *s = create_sources(1);
    assert_non_null(s);
    char *out = blogc_render(l, s, NULL, false);
    assert_string_equal(out,
        "\n"
        "chunda\n"
        "guda\n"
        "\n"
        "\n"
        "\n"
        "\n");
    blogc_template_free_stmts(l);
    b_slist_free_full(s, (b_free_func_t) b_trie_free);
    free(out);
}


static void
test_render_null(void **state)
{
    assert_null(blogc_render(NULL, NULL, NULL, false));
}


static void
test_render_outside_block(void **state)
{
    const char *str =
        "{% if GUDA %}bola{% endif %}\n"
        "{{ BOLA }}\n"
        "{% if not CHUNDA %}lol{% endif %}\n";
    blogc_error_t *err = NULL;
    b_slist_t *l = blogc_template_parse(str, strlen(str), &err);
    assert_non_null(l);
    assert_null(err);
    b_slist_t *s = create_sources(1);
    assert_non_null(s);
    b_trie_t *c = b_trie_new(free);
    b_trie_insert(c, "GUDA", b_strdup("asd"));
    char *out = blogc_render(l, s, c, false);
    assert_string_equal(out,
        "bola\n"
        "\n"
        "lol\n");
    b_trie_free(c);
    blogc_template_free_stmts(l);
    b_slist_free_full(s, (b_free_func_t) b_trie_free);
    free(out);
}


static void
test_render_prefer_local_variable(void **state)
{
    const char *str =
        "{% block entry %}\n"
        "{% if LOL %}{{ LOL }}{% endif %}\n"
        "{% if not CHUNDA %}chunda\n"
        "{% if GUDA %}{{ GUDA }}\n"
        "{% if not BOLA %}bola\n"
        "{% endif %}\n"
        "{% endif %}\n"
        "{% endif %}\n"
        "{% endblock %}\n";
    blogc_error_t *err = NULL;
    b_slist_t *l = blogc_template_parse(str, strlen(str), &err);
    assert_non_null(l);
    assert_null(err);
    b_slist_t *s = create_sources(1);
    assert_non_null(s);
    b_trie_t *c = b_trie_new(free);
    b_trie_insert(c, "GUDA", b_strdup("hehe"));
    b_trie_insert(c, "LOL", b_strdup("hmm"));
    char *out = blogc_render(l, s, c, false);
    assert_string_equal(out,
        "\n"
        "hmm\n"
        "chunda\n"
        "zxc\n"
        "\n"
        "\n"
        "\n"
        "\n");
    b_trie_free(c);
    blogc_template_free_stmts(l);
    b_slist_free_full(s, (b_free_func_t) b_trie_free);
    free(out);
}


static void
test_get_variable(void **state)
{
    b_trie_t *g = b_trie_new(free);
    b_trie_insert(g, "NAME", b_strdup("bola"));
    b_trie_insert(g, "TITLE", b_strdup("bola2"));
    b_trie_t *l = b_trie_new(free);
    b_trie_insert(l, "NAME", b_strdup("chunda"));
    b_trie_insert(l, "TITLE", b_strdup("chunda2"));
    assert_string_equal(blogc_get_variable("NAME", g, l), "chunda");
    assert_string_equal(blogc_get_variable("TITLE", g, l), "chunda2");
    assert_null(blogc_get_variable("BOLA", g, l));
    b_trie_free(g);
    b_trie_free(l);
}


static void
test_get_variable_only_local(void **state)
{
    b_trie_t *g = NULL;
    b_trie_t *l = b_trie_new(free);
    b_trie_insert(l, "NAME", b_strdup("chunda"));
    b_trie_insert(l, "TITLE", b_strdup("chunda2"));
    assert_string_equal(blogc_get_variable("NAME", g, l), "chunda");
    assert_string_equal(blogc_get_variable("TITLE", g, l), "chunda2");
    assert_null(blogc_get_variable("BOLA", g, l));
    b_trie_free(l);
}


static void
test_get_variable_only_global(void **state)
{
    b_trie_t *g = b_trie_new(free);
    b_trie_insert(g, "NAME", b_strdup("bola"));
    b_trie_insert(g, "TITLE", b_strdup("bola2"));
    b_trie_t *l = NULL;
    assert_string_equal(blogc_get_variable("NAME", g, l), "bola");
    assert_string_equal(blogc_get_variable("TITLE", g, l), "bola2");
    assert_null(blogc_get_variable("BOLA", g, l));
    b_trie_free(g);
}


static void
test_format_date(void **state)
{
    b_trie_t *g = b_trie_new(free);
    b_trie_insert(g, "DATE_FORMAT", b_strdup("%H -- %M"));
    b_trie_t *l = b_trie_new(free);
    b_trie_insert(l, "DATE", b_strdup("2015-01-02 03:04:05"));
    b_trie_insert(l, "DATE_FORMAT", b_strdup("%R"));
    char *date = blogc_format_date(g, l);
    assert_string_equal(date, "03:04");
    free(date);
    b_trie_free(g);
    b_trie_free(l);
}


static void
test_format_date_with_global_format(void **state)
{
    b_trie_t *g = b_trie_new(free);
    b_trie_insert(g, "DATE_FORMAT", b_strdup("%H -- %M"));
    b_trie_t *l = b_trie_new(free);
    b_trie_insert(l, "DATE", b_strdup("2015-01-02 03:04:05"));
    char *date = blogc_format_date(g, l);
    assert_string_equal(date, "03 -- 04");
    free(date);
    b_trie_free(g);
    b_trie_free(l);
}


static void
test_format_date_without_format(void **state)
{
    b_trie_t *g = b_trie_new(free);
    b_trie_t *l = b_trie_new(free);
    b_trie_insert(l, "DATE", b_strdup("2015-01-02 03:04:05"));
    char *date = blogc_format_date(g, l);
    assert_string_equal(date, "2015-01-02 03:04:05");
    free(date);
    b_trie_free(g);
    b_trie_free(l);
}


static void
test_format_date_without_date(void **state)
{
    b_trie_t *g = b_trie_new(free);
    b_trie_t *l = b_trie_new(free);
    char *date = blogc_format_date(g, l);
    assert_null(date);
    free(date);
    b_trie_free(g);
    b_trie_free(l);
}


int
main(void)
{
    const UnitTest tests[] = {
        unit_test(test_render_entry),
        unit_test(test_render_listing),
        unit_test(test_render_if),
        unit_test(test_render_if2),
        unit_test(test_render_if3),
        unit_test(test_render_if_not),
        unit_test(test_render_outside_block),
        unit_test(test_render_null),
        unit_test(test_render_prefer_local_variable),
        unit_test(test_get_variable),
        unit_test(test_get_variable_only_local),
        unit_test(test_get_variable_only_global),
        unit_test(test_format_date),
        unit_test(test_format_date_with_global_format),
        unit_test(test_format_date_without_format),
        unit_test(test_format_date_without_date),
    };
    return run_tests(tests);
}
