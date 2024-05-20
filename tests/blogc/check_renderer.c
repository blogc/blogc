// SPDX-FileCopyrightText: 2014-2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
// SPDX-License-Identifier: BSD-3-Clause

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/common/error.h"
#include "../../src/common/utils.h"
#include "../../src/blogc/renderer.h"
#include "../../src/blogc/source-parser.h"
#include "../../src/blogc/template-parser.h"


static bc_slist_t*
create_sources(size_t count)
{
    const char *s[] = {
        "BOLA: asd\n"
        "GUDA: zxc\n"
        "GUDA2: zxc\n"
        "DATE: 2015-01-02 03:04:05\n"
        "DATE_FORMAT: %R\n"
        "TAGS: foo   bar baz\n"
        "-----\n"
        "ahahahahahahahaha",
        "BOLA: asd2\n"
        "GUDA: zxc2\n"
        "DATE: 2014-02-03 04:05:06\n"
        "-----\n"
        "ahahahahahahahaha2",
        "BOLA: asd3\n"
        "GUDA: zxc3\n"
        "DATE: 2013-01-02 03:04:05\n"
        "-----\n"
        "ahahahahahahahaha3",
    };
    assert_false(count > 3);
    bc_error_t *err = NULL;
    bc_slist_t *l = NULL;
    for (size_t i = 0; i < count; i++) {
        l = bc_slist_append(l, blogc_source_parse(s[i], strlen(s[i]), -1, &err));
        assert_null(err);
    }
    assert_int_equal(bc_slist_length(l), count);
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
        "{% ifdef DATE_FORMATTED %}{{ DATE_FORMATTED }}{% endif %}\n"
        "{% ifdef GUDA %}{{ GUDA }}{% endif %}\n"
        "{% ifdef CHUNDA %}{{ CHUNDA }}{% endif %}\n"
        "{% endblock %}\n"
        "{% block listing %}lol{% endblock %}\n"
        "{% if GUDA == GUDA2 %}gudabola{% endif %}\n"
        "{% if GUDA == \"zxc\" %}LOL{% endif %}\n"
        "{% if GUDA != \"bola\" %}HEHE{% endif %}\n"
        "{% if GUDA < \"zxd\" %}LOL2{% endif %}\n"
        "{% if GUDA > \"zxd\" %}LOL3{% else %}ELSE{% endif %}\n"
        "{% if GUDA <= \"zxc\" %}LOL4{% endif %}\n"
        "{% foreach TAGS %}lol {{ FOREACH_ITEM }} haha {% endforeach %}\n"
        "{% foreach TAGS_ASD %}yay{% endforeach %}\n"
        "{% block listing_empty %}vazio{% endblock %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *l = blogc_template_parse(str, strlen(str), &err);
    assert_non_null(l);
    assert_null(err);
    bc_slist_t *s = create_sources(1);
    assert_non_null(s);
    char *out = blogc_render(l, s, NULL, NULL, false);
    assert_string_equal(out,
        "foo\n"
        "\n"
        "\n"
        "2015-01-02 03:04:05\n"
        "03:04\n"
        "zxc\n"
        "\n"
        "\n"
        "\n"
        "gudabola\n"
        "LOL\n"
        "HEHE\n"
        "LOL2\n"
        "ELSE\n"
        "LOL4\n"
        "lol foo haha lol bar haha lol baz haha \n"
        "\n"
        "\n");
    blogc_template_free_ast(l);
    bc_slist_free_full(s, (bc_free_func_t) bc_trie_free);
    free(out);
}


static void
test_render_listing(void **state)
{
    const char *str =
        "foo\n"
        "{% block listing_once %}fuuu{% endblock %}\n"
        "{% block entry %}\n"
        "{% ifdef GUDA %}{{ GUDA }}{% endif %}\n"
        "{% ifdef CHUNDA %}{{ CHUNDA }}{% endif %}\n"
        "{% endblock %}\n"
        "{% block listing %}\n"
        "{% ifdef DATE_FORMATTED %}{{ DATE_FORMATTED }}{% endif %}\n"
        "bola: {% ifdef BOLA %}{{ BOLA }}{% endif %}\n"
        "{% foreach TAGS %}lol {{ FOREACH_ITEM }} haha {% endforeach %}\n"
        "{% foreach TAGS_ASD %}yay{% endforeach %}\n"
        "{% endblock %}\n"
        "{% block listing_empty %}vazio{% endblock %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *l = blogc_template_parse(str, strlen(str), &err);
    assert_non_null(l);
    assert_null(err);
    bc_slist_t *s = create_sources(3);
    assert_non_null(s);
    char *out = blogc_render(l, s, NULL, NULL, true);
    assert_string_equal(out,
        "foo\n"
        "fuuu\n"
        "\n"
        "\n"
        "03:04\n"
        "bola: asd\n"
        "lol foo haha lol bar haha lol baz haha \n"
        "\n"
        "\n"
        "2014-02-03 04:05:06\n"
        "bola: asd2\n"
        "\n"
        "\n"
        "\n"
        "2013-01-02 03:04:05\n"
        "bola: asd3\n"
        "\n"
        "\n"
        "\n"
        "\n");
    blogc_template_free_ast(l);
    bc_slist_free_full(s, (bc_free_func_t) bc_trie_free);
    free(out);
}


static void
test_render_listing_entry(void **state)
{
    const char *str =
        "foo\n"
        "{% block listing_once %}fuuu{% endblock %}\n"
        "{% block entry %}\n"
        "{% ifdef GUDA %}{{ GUDA }}{% endif %}\n"
        "{% ifdef CHUNDA %}{{ CHUNDA }}{% endif %}\n"
        "{% endblock %}\n"
        "{% block listing_entry %}asd{% endblock %}\n"
        "{% block listing %}\n"
        "{% ifdef DATE_FORMATTED %}{{ DATE_FORMATTED }}{% endif %}\n"
        "bola: {% ifdef BOLA %}{{ BOLA }}{% endif %}\n"
        "{% foreach TAGS %}lol {{ FOREACH_ITEM }} haha {% endforeach %}\n"
        "{% foreach TAGS_ASD %}yay{% endforeach %}\n"
        "{% endblock %}\n"
        "{% block listing_empty %}vazio{% endblock %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *l = blogc_template_parse(str, strlen(str), &err);
    assert_non_null(l);
    assert_null(err);
    bc_slist_t *s = create_sources(3);
    assert_non_null(s);
    char *out = blogc_render(l, s, NULL, NULL, true);
    assert_string_equal(out,
        "foo\n"
        "fuuu\n"
        "\n"
        "\n"
        "\n"
        "03:04\n"
        "bola: asd\n"
        "lol foo haha lol bar haha lol baz haha \n"
        "\n"
        "\n"
        "2014-02-03 04:05:06\n"
        "bola: asd2\n"
        "\n"
        "\n"
        "\n"
        "2013-01-02 03:04:05\n"
        "bola: asd3\n"
        "\n"
        "\n"
        "\n"
        "\n");
    blogc_template_free_ast(l);
    bc_slist_free_full(s, (bc_free_func_t) bc_trie_free);
    free(out);
}


static void
test_render_listing_entry2(void **state)
{
    const char *str =
        "foo\n"
        "{% block listing_once %}fuuu{% endblock %}\n"
        "{% block entry %}\n"
        "{% ifdef GUDA %}{{ GUDA }}{% endif %}\n"
        "{% ifdef CHUNDA %}{{ CHUNDA }}{% endif %}\n"
        "{% endblock %}\n"
        "{% block listing_entry %}{{ FUUUUU }}{% endblock %}\n"
        "{% block listing_entry %}{{ BAAAAA }}{% endblock %}\n"
        "{% block listing %}\n"
        "{% ifdef DATE_FORMATTED %}{{ DATE_FORMATTED }}{% endif %}\n"
        "bola: {% ifdef BOLA %}{{ BOLA }}{% endif %}\n"
        "{% foreach TAGS %}lol {{ FOREACH_ITEM }} haha {% endforeach %}\n"
        "{% foreach TAGS_ASD %}yay{% endforeach %}\n"
        "{% endblock %}\n"
        "{% block listing_empty %}vazio{% endblock %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *l = blogc_template_parse(str, strlen(str), &err);
    assert_non_null(l);
    assert_null(err);
    bc_slist_t *s = create_sources(3);
    assert_non_null(s);
    bc_trie_t *entry = bc_trie_new(free);
    bc_trie_insert(entry, "FUUUUU", bc_strdup("XD"));
    bc_trie_insert(entry, "BAAAAA", bc_strdup(":p"));
    bc_slist_t *e = NULL;
    e = bc_slist_append(e, entry);
    char *out = blogc_render(l, s, e, NULL, true);
    bc_slist_free_full(e, (bc_free_func_t) bc_trie_free);
    assert_string_equal(out,
        "foo\n"
        "fuuu\n"
        "\n"
        "XD\n"
        "\n"
        "\n"
        "03:04\n"
        "bola: asd\n"
        "lol foo haha lol bar haha lol baz haha \n"
        "\n"
        "\n"
        "2014-02-03 04:05:06\n"
        "bola: asd2\n"
        "\n"
        "\n"
        "\n"
        "2013-01-02 03:04:05\n"
        "bola: asd3\n"
        "\n"
        "\n"
        "\n"
        "\n");
    blogc_template_free_ast(l);
    bc_slist_free_full(s, (bc_free_func_t) bc_trie_free);
    free(out);
}


static void
test_render_listing_entry3(void **state)
{
    const char *str =
        "foo\n"
        "{% block listing_once %}fuuu{% endblock %}\n"
        "{% block entry %}\n"
        "{% ifdef GUDA %}{{ GUDA }}{% endif %}\n"
        "{% ifdef CHUNDA %}{{ CHUNDA }}{% endif %}\n"
        "{% endblock %}\n"
        "{% block listing_entry %}{{ FUUUUU }}{% endblock %}\n"
        "{% block listing_entry %}{{ BAAAAA }}{% endblock %}\n"
        "{% block listing %}\n"
        "{% ifdef DATE_FORMATTED %}{{ DATE_FORMATTED }}{% endif %}\n"
        "bola: {% ifdef BOLA %}{{ BOLA }}{% endif %}\n"
        "{% foreach TAGS %}lol {{ FOREACH_ITEM }} haha {% endforeach %}\n"
        "{% foreach TAGS_ASD %}yay{% endforeach %}\n"
        "{% endblock %}\n"
        "{% block listing_empty %}vazio{% endblock %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *l = blogc_template_parse(str, strlen(str), &err);
    assert_non_null(l);
    assert_null(err);
    bc_slist_t *s = create_sources(3);
    assert_non_null(s);
    bc_trie_t *entry = bc_trie_new(free);
    bc_trie_insert(entry, "FUUUUU", bc_strdup("XD"));
    bc_trie_insert(entry, "BAAAAA", bc_strdup(":p"));
    bc_slist_t *e = NULL;
    e = bc_slist_append(e, NULL);
    e = bc_slist_append(e, entry);
    char *out = blogc_render(l, s, e, NULL, true);
    bc_slist_free_full(e, (bc_free_func_t) bc_trie_free);
    assert_string_equal(out,
        "foo\n"
        "fuuu\n"
        "\n"
        "\n"
        ":p\n"
        "\n"
        "03:04\n"
        "bola: asd\n"
        "lol foo haha lol bar haha lol baz haha \n"
        "\n"
        "\n"
        "2014-02-03 04:05:06\n"
        "bola: asd2\n"
        "\n"
        "\n"
        "\n"
        "2013-01-02 03:04:05\n"
        "bola: asd3\n"
        "\n"
        "\n"
        "\n"
        "\n");
    blogc_template_free_ast(l);
    bc_slist_free_full(s, (bc_free_func_t) bc_trie_free);
    free(out);
}


static void
test_render_listing_entry4(void **state)
{
    const char *str =
        "foo\n"
        "{% block listing_once %}fuuu{% endblock %}\n"
        "{% block entry %}\n"
        "{% ifdef GUDA %}{{ GUDA }}{% endif %}\n"
        "{% ifdef CHUNDA %}{{ CHUNDA }}{% endif %}\n"
        "{% endblock %}\n"
        "{% block listing_entry %}{{ FUUUUU }}{% endblock %}\n"
        "{% block listing_entry %}{{ DDDDDD }}{% endblock %}\n"
        "{% block listing %}\n"
        "{% ifdef DATE_FORMATTED %}{{ DATE_FORMATTED }}{% endif %}\n"
        "bola: {% ifdef BOLA %}{{ BOLA }}{% endif %}\n"
        "{% foreach TAGS %}lol {{ FOREACH_ITEM }} haha {% endforeach %}\n"
        "{% foreach TAGS_ASD %}yay{% endforeach %}\n"
        "{% endblock %}\n"
        "{% block listing_empty %}vazio{% endblock %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *l = blogc_template_parse(str, strlen(str), &err);
    assert_non_null(l);
    assert_null(err);
    bc_slist_t *s = create_sources(3);
    assert_non_null(s);
    bc_trie_t *entry1 = bc_trie_new(free);
    bc_trie_insert(entry1, "FUUUUU", bc_strdup("XD"));
    bc_trie_insert(entry1, "BAAAAA", bc_strdup(":p"));
    bc_trie_t *entry2 = bc_trie_new(free);
    bc_trie_insert(entry2, "CCCCCC", bc_strdup("er"));
    bc_trie_insert(entry2, "DDDDDD", bc_strdup("ty"));
    bc_slist_t *e = NULL;
    e = bc_slist_append(e, entry1);
    e = bc_slist_append(e, entry2);
    char *out = blogc_render(l, s, e, NULL, true);
    bc_slist_free_full(e, (bc_free_func_t) bc_trie_free);
    assert_string_equal(out,
        "foo\n"
        "fuuu\n"
        "\n"
        "XD\n"
        "ty\n"
        "\n"
        "03:04\n"
        "bola: asd\n"
        "lol foo haha lol bar haha lol baz haha \n"
        "\n"
        "\n"
        "2014-02-03 04:05:06\n"
        "bola: asd2\n"
        "\n"
        "\n"
        "\n"
        "2013-01-02 03:04:05\n"
        "bola: asd3\n"
        "\n"
        "\n"
        "\n"
        "\n");
    blogc_template_free_ast(l);
    bc_slist_free_full(s, (bc_free_func_t) bc_trie_free);
    free(out);
}


static void
test_render_listing_empty(void **state)
{
    const char *str =
        "foo\n"
        "{% block listing_once %}fuuu{% endblock %}\n"
        "{% block entry %}\n"
        "{% ifdef GUDA %}{{ GUDA }}{% endif %}\n"
        "{% ifdef CHUNDA %}{{ CHUNDA }}{% endif %}\n"
        "{% endblock %}\n"
        "{% block listing %}\n"
        "{% ifdef DATE_FORMATTED %}{{ DATE_FORMATTED }}{% endif %}\n"
        "bola: {% ifdef BOLA %}{{ BOLA }}{% endif %}\n"
        "{% foreach TAGS %}lol {{ FOREACH_ITEM }} haha {% endforeach %}\n"
        "{% endblock %}\n"
        "{% block listing_empty %}vazio{% endblock %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *l = blogc_template_parse(str, strlen(str), &err);
    assert_non_null(l);
    assert_null(err);
    char *out = blogc_render(l, NULL, NULL, NULL, true);
    assert_string_equal(out,
        "foo\n"
        "fuuu\n"
        "\n"
        "\n"
        "vazio\n");
    blogc_template_free_ast(l);
    free(out);
}


static void
test_render_ifdef(void **state)
{
    const char *str =
        "{% block entry %}\n"
        "{% ifdef CHUNDA %}chunda\n"
        "{% ifdef GUDA %}guda\n"
        "{% ifdef BOLA %}bola\n"
        "{% endif %}\n"
        "{% endif %}\n"
        "{% endif %}\n"
        "{% endblock %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *l = blogc_template_parse(str, strlen(str), &err);
    assert_non_null(l);
    assert_null(err);
    bc_slist_t *s = create_sources(1);
    assert_non_null(s);
    char *out = blogc_render(l, s, NULL, NULL, false);
    assert_string_equal(out,
        "\n"
        "\n"
        "\n");
    blogc_template_free_ast(l);
    bc_slist_free_full(s, (bc_free_func_t) bc_trie_free);
    free(out);
}


static void
test_render_ifdef2(void **state)
{
    const char *str =
        "{% block entry %}\n"
        "{% ifdef GUDA %}guda\n"
        "{% ifdef CHUNDA %}chunda\n"
        "{% ifdef BOLA %}bola\n"
        "{% endif %}\n"
        "{% endif %}\n"
        "{% endif %}\n"
        "{% endblock %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *l = blogc_template_parse(str, strlen(str), &err);
    assert_non_null(l);
    assert_null(err);
    bc_slist_t *s = create_sources(1);
    assert_non_null(s);
    char *out = blogc_render(l, s, NULL, NULL, false);
    assert_string_equal(out,
        "\n"
        "guda\n"
        "\n"
        "\n"
        "\n");
    blogc_template_free_ast(l);
    bc_slist_free_full(s, (bc_free_func_t) bc_trie_free);
    free(out);
}


static void
test_render_ifdef3(void **state)
{
    const char *str =
        "{% block entry %}\n"
        "{% ifdef GUDA %}guda\n"
        "{% ifdef BOLA %}bola\n"
        "{% ifdef CHUNDA %}chunda\n"
        "{% endif %}\n"
        "{% endif %}\n"
        "{% endif %}\n"
        "{% endblock %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *l = blogc_template_parse(str, strlen(str), &err);
    assert_non_null(l);
    assert_null(err);
    bc_slist_t *s = create_sources(1);
    assert_non_null(s);
    char *out = blogc_render(l, s, NULL, NULL, false);
    assert_string_equal(out,
        "\n"
        "guda\n"
        "bola\n"
        "\n"
        "\n"
        "\n"
        "\n");
    blogc_template_free_ast(l);
    bc_slist_free_full(s, (bc_free_func_t) bc_trie_free);
    free(out);
}


static void
test_render_ifdef4(void **state)
{
    const char *str =
        "{% block entry %}\n"
        "{% ifdef GUDA %}guda\n"
        "{% ifdef BOLA %}bola\n"
        "{% ifdef CHUNDA %}chunda\n"
        "{% else %}else\n"
        "{% endif %}\n"
        "{% endif %}\n"
        "{% else %}lol\n"
        "{% endif %}\n"
        "{% endblock %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *l = blogc_template_parse(str, strlen(str), &err);
    assert_non_null(l);
    assert_null(err);
    bc_slist_t *s = create_sources(1);
    assert_non_null(s);
    char *out = blogc_render(l, s, NULL, NULL, false);
    assert_string_equal(out,
        "\n"
        "guda\n"
        "bola\n"
        "else\n"
        "\n"
        "\n"
        "\n"
        "\n");
    blogc_template_free_ast(l);
    bc_slist_free_full(s, (bc_free_func_t) bc_trie_free);
    free(out);
}


static void
test_render_ifdef5(void **state)
{
    const char *str =
        "{% block entry %}\n"
        "{% ifdef GUDA %}guda\n"
        "{% ifdef CHUNDA %}chunda\n"
        "{% ifdef BOLA %}bola\n"
        "{% endif %}\n"
        "{% else %}else\n"
        "{% endif %}\n"
        "{% else %}lol\n"
        "{% endif %}\n"
        "{% endblock %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *l = blogc_template_parse(str, strlen(str), &err);
    assert_non_null(l);
    assert_null(err);
    bc_slist_t *s = create_sources(1);
    assert_non_null(s);
    char *out = blogc_render(l, s, NULL, NULL, false);
    assert_string_equal(out,
        "\n"
        "guda\n"
        "else\n"
        "\n"
        "\n"
        "\n");
    blogc_template_free_ast(l);
    bc_slist_free_full(s, (bc_free_func_t) bc_trie_free);
    free(out);
}


static void
test_render_ifdef6(void **state)
{
    const char *str =
        "{% block entry %}\n"
        "{% ifdef CHUNDA %}chunda\n"
        "{% ifdef GUDA %}guda\n"
        "{% ifdef BOLA %}bola\n"
        "{% endif %}\n"
        "{% else %}else\n"
        "{% endif %}\n"
        "{% else %}lol\n"
        "{% endif %}\n"
        "{% endblock %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *l = blogc_template_parse(str, strlen(str), &err);
    assert_non_null(l);
    assert_null(err);
    bc_slist_t *s = create_sources(1);
    assert_non_null(s);
    char *out = blogc_render(l, s, NULL, NULL, false);
    assert_string_equal(out,
        "\n"
        "lol\n"
        "\n"
        "\n");
    blogc_template_free_ast(l);
    bc_slist_free_full(s, (bc_free_func_t) bc_trie_free);
    free(out);
}


static void
test_render_ifdef7(void **state)
{
    const char *str =
        "{% block entry %}\n"
        "{% ifdef GUDA %}guda\n"
        "{% ifdef BOLA %}bola\n"
        "{% ifdef CHUNDA %}chunda\n"
        "{% endif %}\n"
        "{% endif %}\n"
        "{% ifdef CHUNDA %}ch\n"
        "{% else %}else\n"
        "{% endif %}\n"
        "{% endif %}\n"
        "{% endblock %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *l = blogc_template_parse(str, strlen(str), &err);
    assert_non_null(l);
    assert_null(err);
    bc_slist_t *s = create_sources(1);
    assert_non_null(s);
    char *out = blogc_render(l, s, NULL, NULL, false);
    assert_string_equal(out,
        "\n"
        "guda\n"
        "bola\n"
        "\n"
        "\n"
        "else\n"
        "\n"
        "\n"
        "\n");
    blogc_template_free_ast(l);
    bc_slist_free_full(s, (bc_free_func_t) bc_trie_free);
    free(out);
}


static void
test_render_ifndef(void **state)
{
    const char *str =
        "{% block entry %}\n"
        "{% ifndef CHUNDA %}chunda\n"
        "{% ifdef GUDA %}guda\n"
        "{% ifndef BOLA %}bola\n"
        "{% else %}else\n"
        "{% endif %}\n"
        "{% endif %}\n"
        "{% endif %}\n"
        "{% endblock %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *l = blogc_template_parse(str, strlen(str), &err);
    assert_non_null(l);
    assert_null(err);
    bc_slist_t *s = create_sources(1);
    assert_non_null(s);
    char *out = blogc_render(l, s, NULL, NULL, false);
    assert_string_equal(out,
        "\n"
        "chunda\n"
        "guda\n"
        "else\n"
        "\n"
        "\n"
        "\n"
        "\n");
    blogc_template_free_ast(l);
    bc_slist_free_full(s, (bc_free_func_t) bc_trie_free);
    free(out);
}


static void
test_render_if_eq(void **state)
{
    const char *str =
        "{% block entry %}\n"
        "{% if GUDA == GUDA2 %}gudabola{% endif %}\n"
        "{% if GUDA == \"zxc\" %}guda\n"
        "{% ifdef BOLA %}bola\n"
        "{% if GUDA > \"zxc\" %}asd\n"
        "{% else %}else\n"
        "{% endif %}\n"
        "{% endif %}\n"
        "{% endif %}\n"
        "{% endblock %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *l = blogc_template_parse(str, strlen(str), &err);
    assert_non_null(l);
    assert_null(err);
    bc_slist_t *s = create_sources(1);
    assert_non_null(s);
    char *out = blogc_render(l, s, NULL, NULL, false);
    assert_string_equal(out,
        "\n"
        "gudabola\n"
        "guda\n"
        "bola\n"
        "else\n"
        "\n"
        "\n"
        "\n"
        "\n");
    blogc_template_free_ast(l);
    bc_slist_free_full(s, (bc_free_func_t) bc_trie_free);
    free(out);
}


static void
test_render_if_neq(void **state)
{
    const char *str =
        "{% block entry %}\n"
        "{% if GUDA != BOLA %}gudabola{% endif %}\n"
        "{% if GUDA != \"zxa\" %}guda\n"
        "{% ifdef BOLA %}bola\n"
        "{% if GUDA > \"zxc\" %}asd\n"
        "{% else %}else\n"
        "{% endif %}\n"
        "{% endif %}\n"
        "{% endif %}\n"
        "{% endblock %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *l = blogc_template_parse(str, strlen(str), &err);
    assert_non_null(l);
    assert_null(err);
    bc_slist_t *s = create_sources(1);
    assert_non_null(s);
    char *out = blogc_render(l, s, NULL, NULL, false);
    assert_string_equal(out,
        "\n"
        "gudabola\n"
        "guda\n"
        "bola\n"
        "else\n"
        "\n"
        "\n"
        "\n"
        "\n");
    blogc_template_free_ast(l);
    bc_slist_free_full(s, (bc_free_func_t) bc_trie_free);
    free(out);
}


static void
test_render_if_lt(void **state)
{
    const char *str =
        "{% block entry %}\n"
        "{% if BOLA < GUDA %}gudabola{% endif %}\n"
        "{% if GUDA < \"zxe\" %}guda\n"
        "{% ifdef BOLA %}bola\n"
        "{% if GUDA > \"zxc\" %}asd\n"
        "{% else %}else\n"
        "{% endif %}\n"
        "{% endif %}\n"
        "{% endif %}\n"
        "{% endblock %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *l = blogc_template_parse(str, strlen(str), &err);
    assert_non_null(l);
    assert_null(err);
    bc_slist_t *s = create_sources(1);
    assert_non_null(s);
    char *out = blogc_render(l, s, NULL, NULL, false);
    assert_string_equal(out,
        "\n"
        "gudabola\n"
        "guda\n"
        "bola\n"
        "else\n"
        "\n"
        "\n"
        "\n"
        "\n");
    blogc_template_free_ast(l);
    bc_slist_free_full(s, (bc_free_func_t) bc_trie_free);
    free(out);
}


static void
test_render_if_gt(void **state)
{
    const char *str =
        "{% block entry %}\n"
        "{% if GUDA > BOLA %}gudabola{% endif %}\n"
        "{% if GUDA > \"zxa\" %}guda\n"
        "{% ifdef BOLA %}bola\n"
        "{% if GUDA > \"zxc\" %}asd\n"
        "{% else %}else\n"
        "{% endif %}\n"
        "{% endif %}\n"
        "{% endif %}\n"
        "{% endblock %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *l = blogc_template_parse(str, strlen(str), &err);
    assert_non_null(l);
    assert_null(err);
    bc_slist_t *s = create_sources(1);
    assert_non_null(s);
    char *out = blogc_render(l, s, NULL, NULL, false);
    assert_string_equal(out,
        "\n"
        "gudabola\n"
        "guda\n"
        "bola\n"
        "else\n"
        "\n"
        "\n"
        "\n"
        "\n");
    blogc_template_free_ast(l);
    bc_slist_free_full(s, (bc_free_func_t) bc_trie_free);
    free(out);
}


static void
test_render_if_lt_eq(void **state)
{
    const char *str =
        "{% block entry %}\n"
        "{% if BOLA <= GUDA %}gudabola{% endif %}\n"
        "{% if GUDA <= \"zxc\" %}guda\n"
        "{% if GUDA <= \"zxe\" %}guda2\n"
        "{% ifdef BOLA %}bola\n"
        "{% if GUDA > \"zxc\" %}asd\n"
        "{% else %}else\n"
        "{% endif %}\n"
        "{% endif %}\n"
        "{% endif %}\n"
        "{% endif %}\n"
        "{% endblock %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *l = blogc_template_parse(str, strlen(str), &err);
    assert_non_null(l);
    assert_null(err);
    bc_slist_t *s = create_sources(1);
    assert_non_null(s);
    char *out = blogc_render(l, s, NULL, NULL, false);
    assert_string_equal(out,
        "\n"
        "gudabola\n"
        "guda\n"
        "guda2\n"
        "bola\n"
        "else\n"
        "\n"
        "\n"
        "\n"
        "\n"
        "\n");
    blogc_template_free_ast(l);
    bc_slist_free_full(s, (bc_free_func_t) bc_trie_free);
    free(out);
}


static void
test_render_if_gt_eq(void **state)
{
    const char *str =
        "{% block entry %}\n"
        "{% if GUDA >= BOLA %}gudabola{% endif %}\n"
        "{% if GUDA >= \"zxc\" %}guda\n"
        "{% if GUDA >= \"zxa\" %}guda2\n"
        "{% ifdef BOLA %}bola\n"
        "{% if GUDA > \"zxc\" %}asd\n"
        "{% else %}else\n"
        "{% endif %}\n"
        "{% endif %}\n"
        "{% endif %}\n"
        "{% endif %}\n"
        "{% endblock %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *l = blogc_template_parse(str, strlen(str), &err);
    assert_non_null(l);
    assert_null(err);
    bc_slist_t *s = create_sources(1);
    assert_non_null(s);
    char *out = blogc_render(l, s, NULL, NULL, false);
    assert_string_equal(out,
        "\n"
        "gudabola\n"
        "guda\n"
        "guda2\n"
        "bola\n"
        "else\n"
        "\n"
        "\n"
        "\n"
        "\n"
        "\n");
    blogc_template_free_ast(l);
    bc_slist_free_full(s, (bc_free_func_t) bc_trie_free);
    free(out);
}


static void
test_render_foreach(void **state)
{
    const char *str =
        "{% block entry %}\n"
        "{% foreach TAGS %} {{ FOREACH_ITEM }} {% endforeach %}\n"
        "{% endblock %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *l = blogc_template_parse(str, strlen(str), &err);
    assert_non_null(l);
    assert_null(err);
    bc_slist_t *s = create_sources(1);
    assert_non_null(s);
    char *out = blogc_render(l, s, NULL, NULL, false);
    assert_string_equal(out,
        "\n"
        " foo  bar  baz \n"
        "\n");
    blogc_template_free_ast(l);
    bc_slist_free_full(s, (bc_free_func_t) bc_trie_free);
    free(out);
}


static void
test_render_foreach_if(void **state)
{
    const char *str =
        "{% block entry %}\n"
        "{% foreach TAGS %} {% if FOREACH_ITEM == \"bar\" %}{{ FOREACH_ITEM }}"
        "{% endif %} {% endforeach %}\n"
        "{% endblock %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *l = blogc_template_parse(str, strlen(str), &err);
    assert_non_null(l);
    assert_null(err);
    bc_slist_t *s = create_sources(1);
    assert_non_null(s);
    char *out = blogc_render(l, s, NULL, NULL, false);
    assert_string_equal(out,
        "\n"
        "   bar   \n"
        "\n");
    blogc_template_free_ast(l);
    bc_slist_free_full(s, (bc_free_func_t) bc_trie_free);
    free(out);
}


static void
test_render_foreach_if_else(void **state)
{
    const char *str =
        "{% block entry %}\n"
        "{% foreach TAGS %}{% if FOREACH_ITEM == \"bar\" %}yay"
        "{% else %}{{ FOREACH_ITEM }}"
        "{% endif %} {% endforeach %}\n"
        "{% endblock %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *l = blogc_template_parse(str, strlen(str), &err);
    assert_non_null(l);
    assert_null(err);
    bc_slist_t *s = create_sources(1);
    assert_non_null(s);
    char *out = blogc_render(l, s, NULL, NULL, false);
    assert_string_equal(out,
        "\n"
        "foo yay baz \n"
        "\n");
    blogc_template_free_ast(l);
    bc_slist_free_full(s, (bc_free_func_t) bc_trie_free);
    free(out);
}


static void
test_render_null(void **state)
{
    assert_null(blogc_render(NULL, NULL, NULL, NULL, false));
}


static void
test_render_outside_block(void **state)
{
    const char *str =
        "{% ifdef GUDA %}bola{% endif %}\n"
        "{{ BOLA }}\n"
        "{% ifndef CHUNDA %}lol{% endif %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *l = blogc_template_parse(str, strlen(str), &err);
    assert_non_null(l);
    assert_null(err);
    bc_slist_t *s = create_sources(1);
    assert_non_null(s);
    bc_trie_t *c = bc_trie_new(free);
    bc_trie_insert(c, "GUDA", bc_strdup("asd"));
    char *out = blogc_render(l, s, NULL, c, false);
    assert_string_equal(out,
        "bola\n"
        "\n"
        "lol\n");
    bc_trie_free(c);
    blogc_template_free_ast(l);
    bc_slist_free_full(s, (bc_free_func_t) bc_trie_free);
    free(out);
}


static void
test_render_prefer_local_variable(void **state)
{
    const char *str =
        "{% block entry %}\n"
        "{% ifdef LOL %}{{ LOL }}{% endif %}\n"
        "{% ifndef CHUNDA %}chunda\n"
        "{% ifdef GUDA %}{{ GUDA }}\n"
        "{% ifndef BOLA %}bola\n"
        "{% endif %}\n"
        "{% endif %}\n"
        "{% endif %}\n"
        "{% endblock %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *l = blogc_template_parse(str, strlen(str), &err);
    assert_non_null(l);
    assert_null(err);
    bc_slist_t *s = create_sources(1);
    assert_non_null(s);
    bc_trie_t *c = bc_trie_new(free);
    bc_trie_insert(c, "GUDA", bc_strdup("hehe"));
    bc_trie_insert(c, "LOL", bc_strdup("hmm"));
    char *out = blogc_render(l, s, NULL, c, false);
    assert_string_equal(out,
        "\n"
        "hmm\n"
        "chunda\n"
        "zxc\n"
        "\n"
        "\n"
        "\n"
        "\n");
    bc_trie_free(c);
    blogc_template_free_ast(l);
    bc_slist_free_full(s, (bc_free_func_t) bc_trie_free);
    free(out);
}


static void
test_render_respect_variable_scope(void **state)
{
    const char *str =
        "{{ LOL }}\n"
        "{{ BOLA }}\n"
        "{% block entry %}\n"
        "{% ifdef LOL %}{{ LOL }}{% endif %}\n"
        "{% ifdef BOLA %}{{ BOLA }}{% endif %}\n"
        "{% endblock %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *l = blogc_template_parse(str, strlen(str), &err);
    assert_non_null(l);
    assert_null(err);
    bc_slist_t *s = create_sources(1);
    assert_non_null(s);
    bc_trie_t *c = bc_trie_new(free);
    char *out = blogc_render(l, s, NULL, c, false);
    assert_string_equal(out,
        "\n"
        "\n"
        "\n"
        "\n"
        "asd\n"
        "\n");
    bc_trie_free(c);
    blogc_template_free_ast(l);
    bc_slist_free_full(s, (bc_free_func_t) bc_trie_free);
    free(out);
}


static void
test_render_ifcount_bug(void **state)
{
    const char *str =
        "{% block entry %}\n"
        "{% ifdef TITLE %}<h3>{{ TITLE }}</h3>{% endif %}\n"
        "{% ifdef IS_POST %}\n"
        "{% ifdef ASD %}ASD{% endif %}\n"
        "{% endif %}\n"
        "{% endblock %}\n";
    bc_error_t *err = NULL;
    bc_slist_t *l = blogc_template_parse(str, strlen(str), &err);
    assert_non_null(l);
    assert_null(err);
    bc_slist_t *s = NULL;
    s = bc_slist_append(s, bc_trie_new(free));
    bc_trie_insert(s->data, "TITLE", bc_strdup("bola"));
    bc_trie_t *c = bc_trie_new(free);
    char *out = blogc_render(l, s, NULL, c, false);
    assert_string_equal(out,
        "\n"
        "<h3>bola</h3>\n"
        "\n"
        "\n");
    bc_trie_free(c);
    blogc_template_free_ast(l);
    bc_slist_free_full(s, (bc_free_func_t) bc_trie_free);
    free(out);
}


static void
test_get_variable(void **state)
{
    bc_trie_t *g = bc_trie_new(free);
    bc_trie_insert(g, "NAME", bc_strdup("bola"));
    bc_trie_insert(g, "TITLE", bc_strdup("bola2"));
    bc_trie_t *l = bc_trie_new(free);
    bc_trie_insert(l, "NAME", bc_strdup("chunda"));
    bc_trie_insert(l, "TITLE", bc_strdup("chunda2"));
    assert_string_equal(blogc_get_variable("NAME", g, l), "chunda");
    assert_string_equal(blogc_get_variable("TITLE", g, l), "chunda2");
    assert_null(blogc_get_variable("BOLA", g, l));
    bc_trie_free(g);
    bc_trie_free(l);
}


static void
test_get_variable_only_local(void **state)
{
    bc_trie_t *g = NULL;
    bc_trie_t *l = bc_trie_new(free);
    bc_trie_insert(l, "NAME", bc_strdup("chunda"));
    bc_trie_insert(l, "TITLE", bc_strdup("chunda2"));
    assert_string_equal(blogc_get_variable("NAME", g, l), "chunda");
    assert_string_equal(blogc_get_variable("TITLE", g, l), "chunda2");
    assert_null(blogc_get_variable("BOLA", g, l));
    bc_trie_free(l);
}


static void
test_get_variable_only_global(void **state)
{
    bc_trie_t *g = bc_trie_new(free);
    bc_trie_insert(g, "NAME", bc_strdup("bola"));
    bc_trie_insert(g, "TITLE", bc_strdup("bola2"));
    bc_trie_t *l = NULL;
    assert_string_equal(blogc_get_variable("NAME", g, l), "bola");
    assert_string_equal(blogc_get_variable("TITLE", g, l), "bola2");
    assert_null(blogc_get_variable("BOLA", g, l));
    bc_trie_free(g);
}


static void
test_format_date(void **state)
{
    bc_trie_t *g = bc_trie_new(free);
    bc_trie_insert(g, "DATE_FORMAT", bc_strdup("%H -- %M"));
    bc_trie_t *l = bc_trie_new(free);
    bc_trie_insert(l, "DATE_FORMAT", bc_strdup("%R"));
    char *date = blogc_format_date("2015-01-02 03:04:05", g, l);
    assert_string_equal(date, "03:04");
    free(date);
    bc_trie_free(g);
    bc_trie_free(l);
}


static void
test_format_date_with_global_format(void **state)
{
    bc_trie_t *g = bc_trie_new(free);
    bc_trie_insert(g, "DATE_FORMAT", bc_strdup("%H -- %M"));
    bc_trie_t *l = bc_trie_new(free);
    char *date = blogc_format_date("2015-01-02 03:04:05", g, l);
    assert_string_equal(date, "03 -- 04");
    free(date);
    bc_trie_free(g);
    bc_trie_free(l);
}


static void
test_format_date_without_format(void **state)
{
    bc_trie_t *g = bc_trie_new(free);
    bc_trie_t *l = bc_trie_new(free);
    char *date = blogc_format_date("2015-01-02 03:04:05", g, l);
    assert_string_equal(date, "2015-01-02 03:04:05");
    free(date);
    bc_trie_free(g);
    bc_trie_free(l);
}


static void
test_format_date_without_date(void **state)
{
    bc_trie_t *g = bc_trie_new(free);
    bc_trie_t *l = bc_trie_new(free);
    char *date = blogc_format_date(NULL, g, l);
    assert_null(date);
    free(date);
    bc_trie_free(g);
    bc_trie_free(l);
}


static void
test_format_variable(void **state)
{
    // FIXME: test warnings
    bc_trie_t *g = bc_trie_new(free);
    bc_trie_insert(g, "NAME", bc_strdup("bola"));
    bc_trie_insert(g, "TITLE", bc_strdup("bola2"));
    bc_trie_t *l = bc_trie_new(free);
    bc_trie_insert(l, "NAME", bc_strdup("chunda"));
    bc_trie_insert(l, "TITLE", bc_strdup("chunda2"));
    bc_trie_insert(l, "SIZE", bc_strdup("1234567890987654321"));
    char *tmp = blogc_format_variable("NAME", g, l, NULL, NULL);
    assert_string_equal(tmp, "chunda");
    free(tmp);
    tmp = blogc_format_variable("TITLE", g, l, NULL, NULL);
    assert_string_equal(tmp, "chunda2");
    free(tmp);
    tmp = blogc_format_variable("TITLE_2", g, l, NULL, NULL);
    assert_string_equal(tmp, "ch");
    free(tmp);
    tmp = blogc_format_variable("SIZE_12", g, l, NULL, NULL);
    assert_string_equal(tmp, "123456789098");
    free(tmp);
    tmp = blogc_format_variable("SIZE_200", g, l, NULL, NULL);
    assert_string_equal(tmp, "1234567890987654321");
    free(tmp);
    assert_null(blogc_format_variable("SIZE_", g, l, NULL, NULL));
    assert_null(blogc_format_variable("BOLA", g, l, NULL, NULL));
    bc_trie_free(g);
    bc_trie_free(l);
}


static void
test_format_variable_with_date(void **state)
{
    bc_trie_t *g = bc_trie_new(free);
    bc_trie_insert(g, "DATE", bc_strdup("2010-11-12 13:14:15"));
    bc_trie_insert(g, "DATE_FORMAT", bc_strdup("%R"));
    bc_trie_t *l = bc_trie_new(free);
    bc_trie_insert(l, "DATE", bc_strdup("2011-12-13 14:15:16"));
    char *tmp = blogc_format_variable("DATE_FORMATTED", g, l, NULL, NULL);
    assert_string_equal(tmp, "14:15");
    free(tmp);
    tmp = blogc_format_variable("DATE_FORMATTED_3", g, l, NULL, NULL);
    assert_string_equal(tmp, "14:");
    free(tmp);
    tmp = blogc_format_variable("DATE_FORMATTED_10", g, l, NULL, NULL);
    assert_string_equal(tmp, "14:15");
    free(tmp);
    bc_trie_free(g);
    bc_trie_free(l);
}


static void
test_format_variable_foreach(void **state)
{
    bc_slist_t *l = NULL;
    l = bc_slist_append(l, bc_strdup("asd"));
    l = bc_slist_append(l, bc_strdup("qwe"));
    l = bc_slist_append(l, bc_strdup("zxcvbn"));
    char *tmp = blogc_format_variable("FOREACH_ITEM", NULL, NULL, NULL, l->next);
    assert_string_equal(tmp, "qwe");
    free(tmp);
    tmp = blogc_format_variable("FOREACH_ITEM_4", NULL, NULL, NULL, l->next->next);
    assert_string_equal(tmp, "zxcv");
    free(tmp);
    tmp = blogc_format_variable("FOREACH_ITEM_10", NULL, NULL, NULL, l->next->next);
    assert_string_equal(tmp, "zxcvbn");
    free(tmp);
    bc_slist_free_full(l, free);
}


static void
test_format_variable_foreach_value(void **state)
{
    bc_trie_t *gl = bc_trie_new(free);
    bc_trie_insert(gl, "FOO_BAR__QWE", bc_strdup("bnm"));
    bc_trie_t *loc = bc_trie_new(free);
    bc_trie_insert(loc, "BAR__ZXCVBN", bc_strdup("dfg"));
    bc_trie_insert(loc, "HUE__ZXCVBN", bc_strdup("xcv"));
    bc_slist_t *l = NULL;
    l = bc_slist_append(l, bc_strdup("asd"));
    l = bc_slist_append(l, bc_strdup("qwe"));
    l = bc_slist_append(l, bc_strdup("zxcvbn"));
    char *tmp = blogc_format_variable("FOREACH_VALUE", gl, loc, "foo-bar", l->next);
    assert_string_equal(tmp, "bnm");
    free(tmp);
    tmp = blogc_format_variable("FOREACH_VALUE_2", gl, loc, "bar", l->next->next);
    assert_string_equal(tmp, "df");
    free(tmp);
    tmp = blogc_format_variable("FOREACH_VALUE_4", gl, loc, "hue", l->next->next);
    assert_string_equal(tmp, "xcv");
    free(tmp);
    bc_trie_free(gl);
    bc_trie_free(loc);
    bc_slist_free_full(l, free);
}


static void
test_format_variable_foreach_empty(void **state)
{
    assert_null(blogc_format_variable("FOREACH_ITEM", NULL, NULL, NULL, NULL));
    assert_null(blogc_format_variable("FOREACH_ITEM_4", NULL, NULL, NULL, NULL));
}


static void
test_split_list_variable(void **state)
{
    bc_trie_t *g = bc_trie_new(free);
    bc_trie_insert(g, "TAGS", bc_strdup("asd  lol hehe"));
    bc_trie_t *l = bc_trie_new(free);
    bc_trie_insert(l, "TAGS", bc_strdup("asd  lol XD"));
    bc_slist_t *tmp = blogc_split_list_variable("TAGS", g, l);
    assert_string_equal(tmp->data, "asd");
    assert_string_equal(tmp->next->data, "lol");
    assert_string_equal(tmp->next->next->data, "XD");
    bc_slist_free_full(tmp, free);
    bc_trie_free(g);
    bc_trie_free(l);
}


static void
test_split_list_variable_not_found(void **state)
{
    bc_trie_t *g = bc_trie_new(free);
    bc_trie_insert(g, "TAGS", bc_strdup("asd  lol hehe"));
    bc_trie_t *l = bc_trie_new(free);
    bc_trie_insert(l, "TAGS", bc_strdup("asd  lol XD"));
    bc_slist_t *tmp = blogc_split_list_variable("TAG", g, l);
    assert_null(tmp);
    bc_trie_free(g);
    bc_trie_free(l);
}


int
main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_render_entry),
        cmocka_unit_test(test_render_listing),
        cmocka_unit_test(test_render_listing_entry),
        cmocka_unit_test(test_render_listing_entry2),
        cmocka_unit_test(test_render_listing_entry3),
        cmocka_unit_test(test_render_listing_entry4),
        cmocka_unit_test(test_render_listing_empty),
        cmocka_unit_test(test_render_ifdef),
        cmocka_unit_test(test_render_ifdef2),
        cmocka_unit_test(test_render_ifdef3),
        cmocka_unit_test(test_render_ifdef4),
        cmocka_unit_test(test_render_ifdef5),
        cmocka_unit_test(test_render_ifdef6),
        cmocka_unit_test(test_render_ifdef7),
        cmocka_unit_test(test_render_ifndef),
        cmocka_unit_test(test_render_if_eq),
        cmocka_unit_test(test_render_if_neq),
        cmocka_unit_test(test_render_if_lt),
        cmocka_unit_test(test_render_if_gt),
        cmocka_unit_test(test_render_if_lt_eq),
        cmocka_unit_test(test_render_if_gt_eq),
        cmocka_unit_test(test_render_foreach),
        cmocka_unit_test(test_render_foreach_if),
        cmocka_unit_test(test_render_foreach_if_else),
        cmocka_unit_test(test_render_null),
        cmocka_unit_test(test_render_outside_block),
        cmocka_unit_test(test_render_prefer_local_variable),
        cmocka_unit_test(test_render_respect_variable_scope),
        cmocka_unit_test(test_render_ifcount_bug),
        cmocka_unit_test(test_get_variable),
        cmocka_unit_test(test_get_variable_only_local),
        cmocka_unit_test(test_get_variable_only_global),
        cmocka_unit_test(test_format_date),
        cmocka_unit_test(test_format_date_with_global_format),
        cmocka_unit_test(test_format_date_without_format),
        cmocka_unit_test(test_format_date_without_date),
        cmocka_unit_test(test_format_variable),
        cmocka_unit_test(test_format_variable_with_date),
        cmocka_unit_test(test_format_variable_foreach),
        cmocka_unit_test(test_format_variable_foreach_value),
        cmocka_unit_test(test_format_variable_foreach_empty),
        cmocka_unit_test(test_split_list_variable),
        cmocka_unit_test(test_split_list_variable_not_found),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
