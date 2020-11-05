/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2020 Rafael G. Martins <rafael@rafaelmartins.eng.br>
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
        "refs/heads/master asd\n"));
    char *t;
    t = _bgr_pre_receive_parse(
        "4f1f932f6ef6d6c9770266775c2db072964d7a62 "
        "3fff4bb3172f77b292b0c913749e81bedd3545f3 "
        "refs/heads/master\n");
    assert_non_null(t);
    assert_string_equal(t, "3fff4bb3172f77b292b0c913749e81bedd3545f3");
    free(t);
    t = _bgr_pre_receive_parse(
        "4f1f932f6ef6d6c9770266775c2db072964d7a62 "
        "3fff4bb3172f77b292b0c913749e81bedd3545fa "
        "refs/heads/master\n"
        "4f1f932f6ef6d6c9770266775c2db072964d7a63 "
        "3fff4bb3172f77b292b0c913749e81bedd3545f4 "
        "refs/heads/bola\n"
    );
    assert_non_null(t);
    assert_string_equal(t, "3fff4bb3172f77b292b0c913749e81bedd3545fa");
    free(t);
    t = _bgr_pre_receive_parse(
        "4f1f932f6ef6d6c9770266775c2db072964d7a63 "
        "3fff4bb3172f77b292b0c913749e81bedd3545f4 "
        "refs/heads/bola\n"
        "4f1f932f6ef6d6c9770266775c2db072964d7a62 "
        "3fff4bb3172f77b292b0c913749e81bedd3545fb "
        "refs/heads/master\n"
    );
    assert_non_null(t);
    assert_string_equal(t, "3fff4bb3172f77b292b0c913749e81bedd3545fb");
    free(t);
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
    assert_string_equal(t, "3fff4bb3172f77b292b0c913749e81bedd3545fc");
    free(t);
}


int
main(void)
{
    const UnitTest tests[] = {
        unit_test(test_pre_receive_parse),
    };
    return run_tests(tests);
}
