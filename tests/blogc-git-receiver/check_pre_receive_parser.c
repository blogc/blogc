// SPDX-FileCopyrightText: 2014-2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
// SPDX-License-Identifier: BSD-3-Clause

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/blogc-git-receiver/pre-receive-parser.h"

#define _bgr_pre_receive_parse(a) bgr_pre_receive_parse(a, strlen(a))


static void
test_pre_receive_parse(void **state)
{
    assert_null(_bgr_pre_receive_parse(""));
    assert_null(_bgr_pre_receive_parse(
        "4f1f932f6ef6d6c9770266775c2db072964d7a62"));
    assert_null(_bgr_pre_receive_parse(
        "4f1f932f6ef6d6c9770266775c2db072964d7a62 "));
    assert_null(_bgr_pre_receive_parse(
        "4f1f932f6ef6d6c9770266775c2db072964d7a62 "
        "3fff4bb3172f77b292b0c913749e81bedd3545f3"));
    assert_null(_bgr_pre_receive_parse(
        "4f1f932f6ef6d6c9770266775c2db072964d7a62 "
        "3fff4bb3172f77b292b0c913749e81bedd3545f3 "));
    assert_null(_bgr_pre_receive_parse(
        "4f1f932f6ef6d6c9770266775c2db072964d7a62 "
        "3fff4bb3172f77b292b0c913749e81bedd3545f3 "
        "refs/heads/lol"));
    assert_null(_bgr_pre_receive_parse(
        "4f1f932f6ef6d6c9770266775c2db072964d7a62 "
        "3fff4bb3172f77b292b0c913749e81bedd3545f3 "
        "refs/heads/lol"));
    assert_null(_bgr_pre_receive_parse(
        "4f1f932f6ef6d6c9770266775c2db072964d7a62 "
        "3fff4bb3172f77b292b0c913749e81bedd3545f3 "
        "refs/heads/master"));
    assert_null(_bgr_pre_receive_parse(
        "4f1f932f6ef6d6c9770266775c2db072964d7a62 "
        "3fff4bb3172f77b292b0c913749e81bedd3545f3 "
        "adgfdgdfgfdgdfgdfgdfgdfgdfg\n"));

    bc_trie_t *t;
    t = _bgr_pre_receive_parse(
        "4f1f932f6ef6d6c9770266775c2db072964d7a62 "
        "3fff4bb3172f77b292b0c913749e81bedd3545f3 "
        "refs/heads/master asd\n");
    assert_non_null(t);
    assert_int_equal(bc_trie_size(t), 1);
    assert_string_equal(bc_trie_lookup(t, "master asd"),
        "3fff4bb3172f77b292b0c913749e81bedd3545f3");
    bc_trie_free(t);

    t = _bgr_pre_receive_parse(
        "4f1f932f6ef6d6c9770266775c2db072964d7a62 "
        "3fff4bb3172f77b292b0c913749e81bedd3545f3 "
        "refs/heads/master\n");
    assert_non_null(t);
    assert_int_equal(bc_trie_size(t), 1);
    assert_string_equal(bc_trie_lookup(t, "master"),
        "3fff4bb3172f77b292b0c913749e81bedd3545f3");
    bc_trie_free(t);

    t = _bgr_pre_receive_parse(
        "4f1f932f6ef6d6c9770266775c2db072964d7a62 "
        "3fff4bb3172f77b292b0c913749e81bedd3545fa "
        "refs/heads/master\n"
        "4f1f932f6ef6d6c9770266775c2db072964d7a63 "
        "3fff4bb3172f77b292b0c913749e81bedd3545f4 "
        "refs/heads/bola\n"
    );
    assert_non_null(t);
    assert_int_equal(bc_trie_size(t), 2);
    assert_string_equal(bc_trie_lookup(t, "master"),
        "3fff4bb3172f77b292b0c913749e81bedd3545fa");
    assert_string_equal(bc_trie_lookup(t, "bola"),
        "3fff4bb3172f77b292b0c913749e81bedd3545f4");
    bc_trie_free(t);

    t = _bgr_pre_receive_parse(
        "4f1f932f6ef6d6c9770266775c2db072964d7a63 "
        "3fff4bb3172f77b292b0c913749e81bedd3545f4 "
        "refs/heads/bola\n"
        "4f1f932f6ef6d6c9770266775c2db072964d7a62 "
        "3fff4bb3172f77b292b0c913749e81bedd3545fb "
        "refs/heads/master\n"
    );
    assert_non_null(t);
    assert_int_equal(bc_trie_size(t), 2);
    assert_string_equal(bc_trie_lookup(t, "bola"),
        "3fff4bb3172f77b292b0c913749e81bedd3545f4");
    assert_string_equal(bc_trie_lookup(t, "master"),
        "3fff4bb3172f77b292b0c913749e81bedd3545fb");
    bc_trie_free(t);

    t = _bgr_pre_receive_parse(
        "4f1f932f6ef6d6c9770266775c2db072964d7a63 "
        "3fff4bb3172f77b292b0c913749e81bedd3545f4 "
        "refs/heads/bola\n"
        "4f1f932f6ef6d6c9770266775c2db072964d7a62 "
        "3fff4bb3172f77b292b0c913749e81bedd3545fc "
        "refs/heads/master\n"
        "4f1f932f6ef6d6c9770266775c2db072964d7a64 "
        "3fff4bb3172f77b292b0c913749e81bedd3545f5 "
        "refs/heads/bolao\n"
    );
    assert_non_null(t);
    assert_int_equal(bc_trie_size(t), 3);
    assert_string_equal(bc_trie_lookup(t, "bola"),
        "3fff4bb3172f77b292b0c913749e81bedd3545f4");
    assert_string_equal(bc_trie_lookup(t, "master"),
        "3fff4bb3172f77b292b0c913749e81bedd3545fc");
    assert_string_equal(bc_trie_lookup(t, "bolao"),
        "3fff4bb3172f77b292b0c913749e81bedd3545f5");
    bc_trie_free(t);
}


int
main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_pre_receive_parse),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
