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
        "-----\n"
        "ahahahahahahahaha",
        "BOLA: asd2\n"
        "GUDA: zxc2\n"
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
    char *out = blogc_render(l, s, false);
    assert_string_equal(out,
        "foo\n"
        "\n"
        "\n"
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
        "bola: {% if BOLA %}{{ BOLA }}{% endif %}\n"
        "{% endblock %}\n";
    blogc_error_t *err = NULL;
    b_slist_t *l = blogc_template_parse(str, strlen(str), &err);
    assert_non_null(l);
    assert_null(err);
    b_slist_t *s = create_sources(3);
    assert_non_null(s);
    char *out = blogc_render(l, s, true);
    assert_string_equal(out,
        "foo\n"
        "fuuu\n"
        "\n"
        "\n"
        "bola: asd\n"
        "\n"
        "bola: asd2\n"
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
    char *out = blogc_render(l, s, false);
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
    char *out = blogc_render(l, s, false);
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
    char *out = blogc_render(l, s, false);
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
test_render_null(void **state)
{
    assert_null(blogc_render(NULL, NULL, false));
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
        unit_test(test_render_null),
    };
    return run_tests(tests);
}
