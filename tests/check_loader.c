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
#include "../src/loader.h"


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


int
main(void)
{
    const UnitTest tests[] = {
        unit_test(test_get_filename),
    };
    return run_tests(tests);
}
