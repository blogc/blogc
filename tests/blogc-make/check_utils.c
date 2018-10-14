/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2018 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <stdlib.h>

#include "../../src/blogc-make/utils.h"
#include "../../src/common/utils.h"


static void
test_generate_filename(void **state)
{
    char *rv;

    assert_null(bm_generate_filename(NULL, NULL, NULL, NULL));
    assert_null(bm_generate_filename(NULL, "", "", ""));
    assert_null(bm_generate_filename("_build", NULL, NULL, NULL));
    assert_null(bm_generate_filename("_build", "", "", ""));

    rv = bm_generate_filename(NULL, NULL, NULL, ".html");
    assert_string_equal(rv, "/index.html");
    free(rv);

    rv = bm_generate_filename(NULL, NULL, NULL, "/index.html");
    assert_string_equal(rv, "/index.html");
    free(rv);

    rv = bm_generate_filename(NULL, "lol", NULL, ".html");
    assert_string_equal(rv, "/lol.html");
    free(rv);

    rv = bm_generate_filename(NULL, "lol", NULL, "/index.html");
    assert_string_equal(rv, "/lol/index.html");
    free(rv);

    rv = bm_generate_filename(NULL, NULL, "foo", ".html");
    assert_string_equal(rv, "/foo.html");
    free(rv);

    rv = bm_generate_filename(NULL, NULL, "foo", "/index.html");
    assert_string_equal(rv, "/foo/index.html");
    free(rv);

    rv = bm_generate_filename(NULL, NULL, "index", ".html");
    assert_string_equal(rv, "/index.html");
    free(rv);

    rv = bm_generate_filename(NULL, NULL, "index", "/index.html");
    assert_string_equal(rv, "/index.html");
    free(rv);

    rv = bm_generate_filename(NULL, "bar", "foo", ".html");
    assert_string_equal(rv, "/bar/foo.html");
    free(rv);

    rv = bm_generate_filename(NULL, "bar", "foo", "/index.html");
    assert_string_equal(rv, "/bar/foo/index.html");
    free(rv);

    rv = bm_generate_filename("_build", NULL, NULL, ".html");
    assert_string_equal(rv, "_build/index.html");
    free(rv);

    rv = bm_generate_filename("_build", NULL, NULL, "/index.html");
    assert_string_equal(rv, "_build/index.html");
    free(rv);

    rv = bm_generate_filename("_build", "lol", NULL, ".html");
    assert_string_equal(rv, "_build/lol.html");
    free(rv);

    rv = bm_generate_filename("_build", "lol", NULL, "/index.html");
    assert_string_equal(rv, "_build/lol/index.html");
    free(rv);

    rv = bm_generate_filename("_build", NULL, "foo", ".html");
    assert_string_equal(rv, "_build/foo.html");
    free(rv);

    rv = bm_generate_filename("_build", NULL, "foo", "/index.html");
    assert_string_equal(rv, "_build/foo/index.html");
    free(rv);

    rv = bm_generate_filename("_build", NULL, "index", ".html");
    assert_string_equal(rv, "_build/index.html");
    free(rv);

    rv = bm_generate_filename("_build", NULL, "index", "/index.html");
    assert_string_equal(rv, "_build/index.html");
    free(rv);

    rv = bm_generate_filename("_build", "bar", "foo", ".html");
    assert_string_equal(rv, "_build/bar/foo.html");
    free(rv);

    rv = bm_generate_filename("_build", "bar", "foo", "/index.html");
    assert_string_equal(rv, "_build/bar/foo/index.html");
    free(rv);
}


int
main(void)
{
    const UnitTest tests[] = {
        unit_test(test_generate_filename),
    };
    return run_tests(tests);
}
