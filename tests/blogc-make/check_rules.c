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
#include <squareball.h>

#include "../../src/blogc-make/rules.h"


static void
test_rule_parse_args(void **state)
{
    sb_trie_t *t = bm_rule_parse_args("bola:foo=" + 4);
    assert_non_null(t);
    assert_int_equal(sb_trie_size(t), 1);
    assert_string_equal(sb_trie_lookup(t, "foo"), "");
    sb_trie_free(t);
    t = bm_rule_parse_args("bola:foo=bar" + 4);
    assert_non_null(t);
    assert_int_equal(sb_trie_size(t), 1);
    assert_string_equal(sb_trie_lookup(t, "foo"), "bar");
    sb_trie_free(t);
    t = bm_rule_parse_args("bola:foo=,baz=lol" + 4);
    assert_non_null(t);
    assert_int_equal(sb_trie_size(t), 2);
    assert_string_equal(sb_trie_lookup(t, "foo"), "");
    assert_string_equal(sb_trie_lookup(t, "baz"), "lol");
    sb_trie_free(t);
    t = bm_rule_parse_args("bola:foo=bar,baz=" + 4);
    assert_non_null(t);
    assert_int_equal(sb_trie_size(t), 2);
    assert_string_equal(sb_trie_lookup(t, "foo"), "bar");
    assert_string_equal(sb_trie_lookup(t, "baz"), "");
    sb_trie_free(t);
    t = bm_rule_parse_args("bola:foo=bar,baz=lol" + 4);
    assert_non_null(t);
    assert_int_equal(sb_trie_size(t), 2);
    assert_string_equal(sb_trie_lookup(t, "foo"), "bar");
    assert_string_equal(sb_trie_lookup(t, "baz"), "lol");
    sb_trie_free(t);
    t = bm_rule_parse_args("bola:foo=,baz=lol,asd=qwe" + 4);
    assert_non_null(t);
    assert_int_equal(sb_trie_size(t), 3);
    assert_string_equal(sb_trie_lookup(t, "foo"), "");
    assert_string_equal(sb_trie_lookup(t, "baz"), "lol");
    assert_string_equal(sb_trie_lookup(t, "asd"), "qwe");
    sb_trie_free(t);
    t = bm_rule_parse_args("bola:foo=bar,baz=,asd=qwe" + 4);
    assert_non_null(t);
    assert_int_equal(sb_trie_size(t), 3);
    assert_string_equal(sb_trie_lookup(t, "foo"), "bar");
    assert_string_equal(sb_trie_lookup(t, "baz"), "");
    assert_string_equal(sb_trie_lookup(t, "asd"), "qwe");
    sb_trie_free(t);
    t = bm_rule_parse_args("bola:foo=bar,baz=lol,asd=" + 4);
    assert_non_null(t);
    assert_int_equal(sb_trie_size(t), 3);
    assert_string_equal(sb_trie_lookup(t, "foo"), "bar");
    assert_string_equal(sb_trie_lookup(t, "baz"), "lol");
    assert_string_equal(sb_trie_lookup(t, "asd"), "");
    sb_trie_free(t);
    t = bm_rule_parse_args("bola:foo=bar,baz=lol,asd=qwe" + 4);
    assert_non_null(t);
    assert_int_equal(sb_trie_size(t), 3);
    assert_string_equal(sb_trie_lookup(t, "foo"), "bar");
    assert_string_equal(sb_trie_lookup(t, "baz"), "lol");
    assert_string_equal(sb_trie_lookup(t, "asd"), "qwe");
    sb_trie_free(t);
}


static void
test_rule_parse_args_error(void **state)
{
    assert_null(bm_rule_parse_args(NULL));
    assert_null(bm_rule_parse_args("bola" + 4));
    assert_null(bm_rule_parse_args("bola:" + 4));
    assert_null(bm_rule_parse_args("bola:asd" + 4));
    assert_null(bm_rule_parse_args("bola:asd=foo,lol" + 4));
    assert_null(bm_rule_parse_args("bola:asd=foo,qwe=bar,lol" + 4));
    assert_null(bm_rule_parse_args("bolaasd" + 4));
}


int
main(void)
{
    const UnitTest tests[] = {
        unit_test(test_rule_parse_args),
        unit_test(test_rule_parse_args_error),
    };
    return run_tests(tests);
}
