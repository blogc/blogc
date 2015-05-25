/*
 * blogc: A blog compiler.
 * Copyright (C) 2015 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
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
#include "../src/utils/utils.h"


static void
test_error_new(void **state)
{
    blogc_error_t *error = blogc_error_new(1, "bola %s");
    assert_non_null(error);
    assert_int_equal(error->type, 1);
    assert_string_equal(error->msg, "bola %s");
    blogc_error_free(error);
}


static void
test_error_new_printf(void **state)
{
    blogc_error_t *error = blogc_error_new_printf(2, "bola %s", "guda");
    assert_non_null(error);
    assert_int_equal(error->type, 2);
    assert_string_equal(error->msg, "bola guda");
    blogc_error_free(error);
}


static void
test_error_parser(void **state)
{
    const char *a = "bola\nguda\nchunda\n";
    blogc_error_t *error = blogc_error_parser(1, a, strlen(a), 11, "asd %d", 10);
    assert_non_null(error);
    assert_int_equal(error->type, 1);
    assert_string_equal(error->msg, "asd 10\nError occurred near to 'hunda'");
    blogc_error_free(error);
}


int
main(void)
{
    const UnitTest tests[] = {
        unit_test(test_error_new),
        unit_test(test_error_new_printf),
        unit_test(test_error_parser),
    };
    return run_tests(tests);
}
