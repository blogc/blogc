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
#include <string.h>
#include "../../src/common/error.h"


static void
test_error_new(void **state)
{
    bc_error_t *error = bc_error_new(1, "bola %s");
    assert_non_null(error);
    assert_int_equal(error->type, 1);
    assert_string_equal(error->msg, "bola %s");
    bc_error_free(error);
}


static void
test_error_new_printf(void **state)
{
    bc_error_t *error = bc_error_new_printf(2, "bola %s", "guda");
    assert_non_null(error);
    assert_int_equal(error->type, 2);
    assert_string_equal(error->msg, "bola guda");
    bc_error_free(error);
}


static void
test_error_parser(void **state)
{
    const char *a = "bola\nguda\nchunda\n";
    bc_error_t *error = bc_error_parser(1, a, strlen(a), 11, "asd %d", 10);
    assert_non_null(error);
    assert_int_equal(error->type, 1);
    assert_string_equal(error->msg,
        "asd 10\nError occurred near line 3, position 2: chunda");
    bc_error_free(error);
    a = "bola\nguda\nchunda";
    error = bc_error_parser(1, a, strlen(a), 11, "asd %d", 10);
    assert_non_null(error);
    assert_int_equal(error->type, 1);
    assert_string_equal(error->msg,
        "asd 10\nError occurred near line 3, position 2: chunda");
    bc_error_free(error);
    a = "bola\nguda\nchunda";
    error = bc_error_parser(1, a, strlen(a), 0, "asd %d", 10);
    assert_non_null(error);
    assert_int_equal(error->type, 1);
    assert_string_equal(error->msg,
        "asd 10\nError occurred near line 1, position 1: bola");
    bc_error_free(error);
    a = "";
    error = bc_error_parser(1, a, strlen(a), 0, "asd %d", 10);
    assert_non_null(error);
    assert_int_equal(error->type, 1);
    assert_string_equal(error->msg, "asd 10");
    bc_error_free(error);
}


static void
test_error_parser_crlf(void **state)
{
    const char *a = "bola\r\nguda\r\nchunda\r\n";
    bc_error_t *error = bc_error_parser(1, a, strlen(a), 13, "asd %d", 10);
    assert_non_null(error);
    assert_int_equal(error->type, 1);
    assert_string_equal(error->msg,
        "asd 10\nError occurred near line 3, position 2: chunda");
    bc_error_free(error);
    a = "bola\r\nguda\r\nchunda";
    error = bc_error_parser(1, a, strlen(a), 13, "asd %d", 10);
    assert_non_null(error);
    assert_int_equal(error->type, 1);
    assert_string_equal(error->msg,
        "asd 10\nError occurred near line 3, position 2: chunda");
    bc_error_free(error);
    a = "bola\r\nguda\r\nchunda";
    error = bc_error_parser(1, a, strlen(a), 0, "asd %d", 10);
    assert_non_null(error);
    assert_int_equal(error->type, 1);
    assert_string_equal(error->msg,
        "asd 10\nError occurred near line 1, position 1: bola");
    bc_error_free(error);
}


int
main(void)
{
    const UnitTest tests[] = {
        unit_test(test_error_new),
        unit_test(test_error_new_printf),
        unit_test(test_error_parser),
        unit_test(test_error_parser_crlf),
    };
    return run_tests(tests);
}
