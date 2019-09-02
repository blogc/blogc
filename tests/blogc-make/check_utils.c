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
#include <squareball.h>

#include "../../src/blogc-make/utils.h"


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

    rv = bm_generate_filename(NULL, "lol", "index", ".html");
    assert_string_equal(rv, "/lol/index.html");
    free(rv);

    rv = bm_generate_filename(NULL, NULL, "index", "/index.html");
    assert_string_equal(rv, "/index.html");
    free(rv);

    rv = bm_generate_filename(NULL, "lol", "index", "/index.html");
    assert_string_equal(rv, "/lol/index/index.html");
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

    rv = bm_generate_filename("_build", "lol", "index", ".html");
    assert_string_equal(rv, "_build/lol/index.html");
    free(rv);

    rv = bm_generate_filename("_build", NULL, "index", "/index.html");
    assert_string_equal(rv, "_build/index.html");
    free(rv);

    rv = bm_generate_filename("_build", "lol", "index", "/index.html");
    assert_string_equal(rv, "_build/lol/index/index.html");
    free(rv);

    rv = bm_generate_filename("_build", "bar", "foo", ".html");
    assert_string_equal(rv, "_build/bar/foo.html");
    free(rv);

    rv = bm_generate_filename("_build", "bar", "foo", "/index.html");
    assert_string_equal(rv, "_build/bar/foo/index.html");
    free(rv);
}


static void
test_generate_filename2(void **state)
{
    char *rv;

    assert_null(bm_generate_filename2(NULL, NULL, NULL, NULL, NULL, NULL));
    assert_null(bm_generate_filename2(NULL, "", "", "", "", ""));
    assert_null(bm_generate_filename2("_build", NULL, NULL, NULL, NULL, NULL));
    assert_null(bm_generate_filename2("_build", "", "", "", "", ""));

    rv = bm_generate_filename2(NULL, NULL, NULL, NULL, NULL, ".html");
    assert_string_equal(rv, "/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "p", NULL, NULL, NULL, ".html");
    assert_string_equal(rv, "/p.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, "q", NULL, NULL, ".html");
    assert_string_equal(rv, "/q.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "p", "q", NULL, NULL, ".html");
    assert_string_equal(rv, "/p/q.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, NULL, NULL, NULL, "/index.html");
    assert_string_equal(rv, "/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "p", NULL, NULL, NULL, "/index.html");
    assert_string_equal(rv, "/p/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, "q", NULL, NULL, "/index.html");
    assert_string_equal(rv, "/q/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "p", "q", NULL, NULL, "/index.html");
    assert_string_equal(rv, "/p/q/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, NULL, "lol", NULL, ".html");
    assert_string_equal(rv, "/lol.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "p", NULL, "lol", NULL, ".html");
    assert_string_equal(rv, "/p/lol.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, "q", "lol", NULL, ".html");
    assert_string_equal(rv, "/q/lol.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "p", "q", "lol", NULL, ".html");
    assert_string_equal(rv, "/p/q/lol.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, NULL, "lol", NULL, "/index.html");
    assert_string_equal(rv, "/lol/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "p", NULL, "lol", NULL, "/index.html");
    assert_string_equal(rv, "/p/lol/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, "q", "lol", NULL, "/index.html");
    assert_string_equal(rv, "/q/lol/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "p", "q", "lol", NULL, "/index.html");
    assert_string_equal(rv, "/p/q/lol/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, NULL, NULL, "foo", ".html");
    assert_string_equal(rv, "/foo.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "p", NULL, NULL, "foo", ".html");
    assert_string_equal(rv, "/p/foo.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, "q", NULL, "foo", ".html");
    assert_string_equal(rv, "/q/foo.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "p", "q", NULL, "foo", ".html");
    assert_string_equal(rv, "/p/q/foo.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, NULL, NULL, "foo", "/index.html");
    assert_string_equal(rv, "/foo/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "p", NULL, NULL, "foo", "/index.html");
    assert_string_equal(rv, "/p/foo/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, "q", NULL, "foo", "/index.html");
    assert_string_equal(rv, "/q/foo/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "p", "q", NULL, "foo", "/index.html");
    assert_string_equal(rv, "/p/q/foo/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, NULL, NULL, "index", ".html");
    assert_string_equal(rv, "/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "p", NULL, NULL, "index", ".html");
    assert_string_equal(rv, "/p/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, "q", NULL, "index", ".html");
    assert_string_equal(rv, "/q/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "p", "q", NULL, "index", ".html");
    assert_string_equal(rv, "/p/q/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, NULL, NULL, "index", "/index.html");
    assert_string_equal(rv, "/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "p", NULL, NULL, "index", "/index.html");
    assert_string_equal(rv, "/p/index/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, "q", NULL, "index", "/index.html");
    assert_string_equal(rv, "/q/index/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "p", "q", NULL, "index", "/index.html");
    assert_string_equal(rv, "/p/q/index/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, NULL, "bar", "foo", ".html");
    assert_string_equal(rv, "/bar/foo.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "p", NULL, "bar", "foo", ".html");
    assert_string_equal(rv, "/p/bar/foo.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, "q", "bar", "foo", ".html");
    assert_string_equal(rv, "/q/bar/foo.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "p", "q", "bar", "foo", ".html");
    assert_string_equal(rv, "/p/q/bar/foo.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, NULL, "bar", "foo", "/index.html");
    assert_string_equal(rv, "/bar/foo/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "p", NULL, "bar", "foo", "/index.html");
    assert_string_equal(rv, "/p/bar/foo/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, "q", "bar", "foo", "/index.html");
    assert_string_equal(rv, "/q/bar/foo/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "p", "q", "bar", "foo", "/index.html");
    assert_string_equal(rv, "/p/q/bar/foo/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, NULL, NULL, NULL, ".html");
    assert_string_equal(rv, "_build/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", "p", NULL, NULL, NULL, ".html");
    assert_string_equal(rv, "_build/p.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, "q", NULL, NULL, ".html");
    assert_string_equal(rv, "_build/q.html");
    free(rv);

    rv = bm_generate_filename2("_build", "p", "q", NULL, NULL, ".html");
    assert_string_equal(rv, "_build/p/q.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, NULL, NULL, NULL, "/index.html");
    assert_string_equal(rv, "_build/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", "p", NULL, NULL, NULL, "/index.html");
    assert_string_equal(rv, "_build/p/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, "q", NULL, NULL, "/index.html");
    assert_string_equal(rv, "_build/q/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", "p", "q", NULL, NULL, "/index.html");
    assert_string_equal(rv, "_build/p/q/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, NULL, "lol", NULL, ".html");
    assert_string_equal(rv, "_build/lol.html");
    free(rv);

    rv = bm_generate_filename2("_build", "p", NULL, "lol", NULL, ".html");
    assert_string_equal(rv, "_build/p/lol.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, "q", "lol", NULL, ".html");
    assert_string_equal(rv, "_build/q/lol.html");
    free(rv);

    rv = bm_generate_filename2("_build", "p", "q", "lol", NULL, ".html");
    assert_string_equal(rv, "_build/p/q/lol.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, NULL, "lol", NULL, "/index.html");
    assert_string_equal(rv, "_build/lol/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", "p", NULL, "lol", NULL, "/index.html");
    assert_string_equal(rv, "_build/p/lol/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, "q", "lol", NULL, "/index.html");
    assert_string_equal(rv, "_build/q/lol/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", "p", "q", "lol", NULL, "/index.html");
    assert_string_equal(rv, "_build/p/q/lol/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, NULL, NULL, "foo", ".html");
    assert_string_equal(rv, "_build/foo.html");
    free(rv);

    rv = bm_generate_filename2("_build", "p", NULL, NULL, "foo", ".html");
    assert_string_equal(rv, "_build/p/foo.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, "q", NULL, "foo", ".html");
    assert_string_equal(rv, "_build/q/foo.html");
    free(rv);

    rv = bm_generate_filename2("_build", "p", "q", NULL, "foo", ".html");
    assert_string_equal(rv, "_build/p/q/foo.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, NULL, NULL, "foo", "/index.html");
    assert_string_equal(rv, "_build/foo/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", "p", NULL, NULL, "foo", "/index.html");
    assert_string_equal(rv, "_build/p/foo/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, "q", NULL, "foo", "/index.html");
    assert_string_equal(rv, "_build/q/foo/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", "p", "q", NULL, "foo", "/index.html");
    assert_string_equal(rv, "_build/p/q/foo/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, NULL, NULL, "index", ".html");
    assert_string_equal(rv, "_build/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", "p", NULL, NULL, "index", ".html");
    assert_string_equal(rv, "_build/p/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, "q", NULL, "index", ".html");
    assert_string_equal(rv, "_build/q/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", "p", "q", NULL, "index", ".html");
    assert_string_equal(rv, "_build/p/q/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, NULL, NULL, "index", "/index.html");
    assert_string_equal(rv, "_build/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", "p", NULL, NULL, "index", "/index.html");
    assert_string_equal(rv, "_build/p/index/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, "q", NULL, "index", "/index.html");
    assert_string_equal(rv, "_build/q/index/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", "p", "q", NULL, "index", "/index.html");
    assert_string_equal(rv, "_build/p/q/index/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, NULL, "bar", "foo", ".html");
    assert_string_equal(rv, "_build/bar/foo.html");
    free(rv);

    rv = bm_generate_filename2("_build", "p", NULL, "bar", "foo", ".html");
    assert_string_equal(rv, "_build/p/bar/foo.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, "q", "bar", "foo", ".html");
    assert_string_equal(rv, "_build/q/bar/foo.html");
    free(rv);

    rv = bm_generate_filename2("_build", "p", "q", "bar", "foo", ".html");
    assert_string_equal(rv, "_build/p/q/bar/foo.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, NULL, "bar", "foo", "/index.html");
    assert_string_equal(rv, "_build/bar/foo/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", "p", NULL, "bar", "foo", "/index.html");
    assert_string_equal(rv, "_build/p/bar/foo/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, "q", "bar", "foo", "/index.html");
    assert_string_equal(rv, "_build/q/bar/foo/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", "p", "q", "bar", "foo", "/index.html");
    assert_string_equal(rv, "_build/p/q/bar/foo/index.html");
    free(rv);
}


int
main(void)
{
    const UnitTest tests[] = {
        unit_test(test_generate_filename),
        unit_test(test_generate_filename2),
    };
    return run_tests(tests);
}
