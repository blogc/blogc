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
#include "../../src/common/stdin.h"


int
__wrap_fgetc(FILE *stream)
{
    assert_int_equal(fileno(stream), fileno(stdin));
    return mock_type(int);
}


static void
test_read(void **state)
{
    assert_null(bc_stdin_read(NULL));
    will_return(__wrap_fgetc, EOF);
    size_t len;
    char *t = bc_stdin_read(&len);
    assert_non_null(t);
    assert_string_equal(t, "");
    assert_int_equal(len, 0);
    free(t);
    will_return(__wrap_fgetc, 'b');
    will_return(__wrap_fgetc, 'o');
    will_return(__wrap_fgetc, 'l');
    will_return(__wrap_fgetc, 'a');
    will_return(__wrap_fgetc, EOF);
    t = bc_stdin_read(&len);
    assert_non_null(t);
    assert_string_equal(t, "bola");
    assert_int_equal(len, 4);
    free(t);
}


int
main(void)
{
    const UnitTest tests[] = {
        unit_test(test_read),
    };
    return run_tests(tests);
}
