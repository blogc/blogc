// SPDX-FileCopyrightText: 2014-2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
// SPDX-License-Identifier: BSD-3-Clause

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

    assert_null(bm_generate_filename(NULL, NULL, NULL, NULL, NULL));
    assert_null(bm_generate_filename(NULL, NULL, "", "", ""));
    assert_null(bm_generate_filename("_build", NULL, NULL, NULL, NULL));
    assert_null(bm_generate_filename("_build", NULL, "", "", ""));

    rv = bm_generate_filename(NULL, NULL, NULL, NULL, ".html");
    assert_string_equal(rv, "/index.html");
    free(rv);

    rv = bm_generate_filename(NULL, NULL, NULL, NULL, "/index.html");
    assert_string_equal(rv, "/index.html");
    free(rv);

    rv = bm_generate_filename(NULL, NULL, "lol", NULL, ".html");
    assert_string_equal(rv, "/lol.html");
    free(rv);

    rv = bm_generate_filename(NULL, NULL, "lol", NULL, "/index.html");
    assert_string_equal(rv, "/lol/index.html");
    free(rv);

    rv = bm_generate_filename(NULL, NULL, NULL, "foo", ".html");
    assert_string_equal(rv, "/foo.html");
    free(rv);

    rv = bm_generate_filename(NULL, NULL, NULL, "foo", "/index.html");
    assert_string_equal(rv, "/foo/index.html");
    free(rv);

    rv = bm_generate_filename(NULL, NULL, NULL, "index", ".html");
    assert_string_equal(rv, "/index.html");
    free(rv);

    rv = bm_generate_filename(NULL, NULL, "lol", "index", ".html");
    assert_string_equal(rv, "/lol/index.html");
    free(rv);

    rv = bm_generate_filename(NULL, NULL, NULL, "index", "/index.html");
    assert_string_equal(rv, "/index.html");
    free(rv);

    rv = bm_generate_filename(NULL, NULL, "lol", "index", "/index.html");
    assert_string_equal(rv, "/lol/index/index.html");
    free(rv);

    rv = bm_generate_filename(NULL, NULL, "bar", "foo", ".html");
    assert_string_equal(rv, "/bar/foo.html");
    free(rv);

    rv = bm_generate_filename(NULL, NULL, "bar", "foo", "/index.html");
    assert_string_equal(rv, "/bar/foo/index.html");
    free(rv);

    rv = bm_generate_filename("_build", NULL, NULL, NULL, ".html");
    assert_string_equal(rv, "_build/index.html");
    free(rv);

    rv = bm_generate_filename("_build", NULL, NULL, NULL, "/index.html");
    assert_string_equal(rv, "_build/index.html");
    free(rv);

    rv = bm_generate_filename("_build", NULL, "lol", NULL, ".html");
    assert_string_equal(rv, "_build/lol.html");
    free(rv);

    rv = bm_generate_filename("_build", NULL, "lol", NULL, "/index.html");
    assert_string_equal(rv, "_build/lol/index.html");
    free(rv);

    rv = bm_generate_filename("_build", NULL, NULL, "foo", ".html");
    assert_string_equal(rv, "_build/foo.html");
    free(rv);

    rv = bm_generate_filename("_build", NULL, NULL, "foo", "/index.html");
    assert_string_equal(rv, "_build/foo/index.html");
    free(rv);

    rv = bm_generate_filename("_build", NULL, NULL, "index", ".html");
    assert_string_equal(rv, "_build/index.html");
    free(rv);

    rv = bm_generate_filename("_build", NULL, "lol", "index", ".html");
    assert_string_equal(rv, "_build/lol/index.html");
    free(rv);

    rv = bm_generate_filename("_build", NULL, NULL, "index", "/index.html");
    assert_string_equal(rv, "_build/index.html");
    free(rv);

    rv = bm_generate_filename("_build", NULL, "lol", "index", "/index.html");
    assert_string_equal(rv, "_build/lol/index/index.html");
    free(rv);

    rv = bm_generate_filename("_build", NULL, "bar", "foo", ".html");
    assert_string_equal(rv, "_build/bar/foo.html");
    free(rv);

    rv = bm_generate_filename("_build", NULL, "bar", "foo", "/index.html");
    assert_string_equal(rv, "_build/bar/foo/index.html");
    free(rv);

    rv = bm_generate_filename(NULL, "foo", NULL, NULL, ".html");
    assert_string_equal(rv, "/foo/index.html");
    free(rv);

    rv = bm_generate_filename(NULL, "foo", NULL, NULL, "/index.html");
    assert_string_equal(rv, "/foo/index.html");
    free(rv);

    rv = bm_generate_filename(NULL, "foo", "lol", NULL, ".html");
    assert_string_equal(rv, "/foo/lol.html");
    free(rv);

    rv = bm_generate_filename(NULL, "foo", "lol", NULL, "/index.html");
    assert_string_equal(rv, "/foo/lol/index.html");
    free(rv);

    rv = bm_generate_filename(NULL, "foo", NULL, "foo", ".html");
    assert_string_equal(rv, "/foo/foo.html");
    free(rv);

    rv = bm_generate_filename(NULL, "foo", NULL, "foo", "/index.html");
    assert_string_equal(rv, "/foo/foo/index.html");
    free(rv);

    rv = bm_generate_filename(NULL, "foo", NULL, "index", ".html");
    assert_string_equal(rv, "/foo/index.html");
    free(rv);

    rv = bm_generate_filename(NULL, "foo", "lol", "index", ".html");
    assert_string_equal(rv, "/foo/lol/index.html");
    free(rv);

    rv = bm_generate_filename(NULL, "foo", NULL, "index", "/index.html");
    assert_string_equal(rv, "/foo/index.html");
    free(rv);

    rv = bm_generate_filename(NULL, "foo", "lol", "index", "/index.html");
    assert_string_equal(rv, "/foo/lol/index/index.html");
    free(rv);

    rv = bm_generate_filename(NULL, "foo", "bar", "foo", ".html");
    assert_string_equal(rv, "/foo/bar/foo.html");
    free(rv);

    rv = bm_generate_filename(NULL, "foo", "bar", "foo", "/index.html");
    assert_string_equal(rv, "/foo/bar/foo/index.html");
    free(rv);

    rv = bm_generate_filename("_build", "foo", NULL, NULL, ".html");
    assert_string_equal(rv, "_build/foo/index.html");
    free(rv);

    rv = bm_generate_filename("_build", "foo", NULL, NULL, "/index.html");
    assert_string_equal(rv, "_build/foo/index.html");
    free(rv);

    rv = bm_generate_filename("_build", "foo", "lol", NULL, ".html");
    assert_string_equal(rv, "_build/foo/lol.html");
    free(rv);

    rv = bm_generate_filename("_build", "foo", "lol", NULL, "/index.html");
    assert_string_equal(rv, "_build/foo/lol/index.html");
    free(rv);

    rv = bm_generate_filename("_build", "foo", NULL, "foo", ".html");
    assert_string_equal(rv, "_build/foo/foo.html");
    free(rv);

    rv = bm_generate_filename("_build", "foo", NULL, "foo", "/index.html");
    assert_string_equal(rv, "_build/foo/foo/index.html");
    free(rv);

    rv = bm_generate_filename("_build", "foo", NULL, "index", ".html");
    assert_string_equal(rv, "_build/foo/index.html");
    free(rv);

    rv = bm_generate_filename("_build", "foo", "lol", "index", ".html");
    assert_string_equal(rv, "_build/foo/lol/index.html");
    free(rv);

    rv = bm_generate_filename("_build", "foo", NULL, "index", "/index.html");
    assert_string_equal(rv, "_build/foo/index.html");
    free(rv);

    rv = bm_generate_filename("_build", "foo", "lol", "index", "/index.html");
    assert_string_equal(rv, "_build/foo/lol/index/index.html");
    free(rv);

    rv = bm_generate_filename("_build", "foo", "bar", "foo", ".html");
    assert_string_equal(rv, "_build/foo/bar/foo.html");
    free(rv);

    rv = bm_generate_filename("_build", "foo", "bar", "foo", "/index.html");
    assert_string_equal(rv, "_build/foo/bar/foo/index.html");
    free(rv);
}


static void
test_generate_filename2(void **state)
{
    char *rv;

    assert_null(bm_generate_filename2(NULL, NULL, NULL, NULL, NULL, NULL, NULL));
    assert_null(bm_generate_filename2(NULL, NULL, "", "", "", "", ""));
    assert_null(bm_generate_filename2("_build", NULL, NULL, NULL, NULL, NULL, NULL));
    assert_null(bm_generate_filename2("_build", NULL, "", "", "", "", ""));

    rv = bm_generate_filename2(NULL, NULL, NULL, NULL, NULL, NULL, ".html");
    assert_string_equal(rv, "/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, "p", NULL, NULL, NULL, ".html");
    assert_string_equal(rv, "/p.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, NULL, "q", NULL, NULL, ".html");
    assert_string_equal(rv, "/q.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, "p", "q", NULL, NULL, ".html");
    assert_string_equal(rv, "/p/q.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, NULL, NULL, NULL, NULL, "/index.html");
    assert_string_equal(rv, "/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, "p", NULL, NULL, NULL, "/index.html");
    assert_string_equal(rv, "/p/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, NULL, "q", NULL, NULL, "/index.html");
    assert_string_equal(rv, "/q/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, "p", "q", NULL, NULL, "/index.html");
    assert_string_equal(rv, "/p/q/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, NULL, NULL, "lol", NULL, ".html");
    assert_string_equal(rv, "/lol.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, "p", NULL, "lol", NULL, ".html");
    assert_string_equal(rv, "/p/lol.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, NULL, "q", "lol", NULL, ".html");
    assert_string_equal(rv, "/q/lol.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, "p", "q", "lol", NULL, ".html");
    assert_string_equal(rv, "/p/q/lol.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, NULL, NULL, "lol", NULL, "/index.html");
    assert_string_equal(rv, "/lol/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, "p", NULL, "lol", NULL, "/index.html");
    assert_string_equal(rv, "/p/lol/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, NULL, "q", "lol", NULL, "/index.html");
    assert_string_equal(rv, "/q/lol/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, "p", "q", "lol", NULL, "/index.html");
    assert_string_equal(rv, "/p/q/lol/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, NULL, NULL, NULL, "foo", ".html");
    assert_string_equal(rv, "/foo.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, "p", NULL, NULL, "foo", ".html");
    assert_string_equal(rv, "/p/foo.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, NULL, "q", NULL, "foo", ".html");
    assert_string_equal(rv, "/q/foo.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, "p", "q", NULL, "foo", ".html");
    assert_string_equal(rv, "/p/q/foo.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, NULL, NULL, NULL, "foo", "/index.html");
    assert_string_equal(rv, "/foo/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, "p", NULL, NULL, "foo", "/index.html");
    assert_string_equal(rv, "/p/foo/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, NULL, "q", NULL, "foo", "/index.html");
    assert_string_equal(rv, "/q/foo/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, "p", "q", NULL, "foo", "/index.html");
    assert_string_equal(rv, "/p/q/foo/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, NULL, NULL, NULL, "index", ".html");
    assert_string_equal(rv, "/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, "p", NULL, NULL, "index", ".html");
    assert_string_equal(rv, "/p/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, NULL, "q", NULL, "index", ".html");
    assert_string_equal(rv, "/q/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, "p", "q", NULL, "index", ".html");
    assert_string_equal(rv, "/p/q/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, NULL, NULL, NULL, "index", "/index.html");
    assert_string_equal(rv, "/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, "p", NULL, NULL, "index", "/index.html");
    assert_string_equal(rv, "/p/index/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, NULL, "q", NULL, "index", "/index.html");
    assert_string_equal(rv, "/q/index/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, "p", "q", NULL, "index", "/index.html");
    assert_string_equal(rv, "/p/q/index/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, NULL, NULL, "bar", "foo", ".html");
    assert_string_equal(rv, "/bar/foo.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, "p", NULL, "bar", "foo", ".html");
    assert_string_equal(rv, "/p/bar/foo.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, NULL, "q", "bar", "foo", ".html");
    assert_string_equal(rv, "/q/bar/foo.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, "p", "q", "bar", "foo", ".html");
    assert_string_equal(rv, "/p/q/bar/foo.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, NULL, NULL, "bar", "foo", "/index.html");
    assert_string_equal(rv, "/bar/foo/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, "p", NULL, "bar", "foo", "/index.html");
    assert_string_equal(rv, "/p/bar/foo/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, NULL, "q", "bar", "foo", "/index.html");
    assert_string_equal(rv, "/q/bar/foo/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, NULL, "p", "q", "bar", "foo", "/index.html");
    assert_string_equal(rv, "/p/q/bar/foo/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, NULL, NULL, NULL, NULL, ".html");
    assert_string_equal(rv, "_build/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, "p", NULL, NULL, NULL, ".html");
    assert_string_equal(rv, "_build/p.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, NULL, "q", NULL, NULL, ".html");
    assert_string_equal(rv, "_build/q.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, "p", "q", NULL, NULL, ".html");
    assert_string_equal(rv, "_build/p/q.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, NULL, NULL, NULL, NULL, "/index.html");
    assert_string_equal(rv, "_build/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, "p", NULL, NULL, NULL, "/index.html");
    assert_string_equal(rv, "_build/p/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, NULL, "q", NULL, NULL, "/index.html");
    assert_string_equal(rv, "_build/q/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, "p", "q", NULL, NULL, "/index.html");
    assert_string_equal(rv, "_build/p/q/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, NULL, NULL, "lol", NULL, ".html");
    assert_string_equal(rv, "_build/lol.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, "p", NULL, "lol", NULL, ".html");
    assert_string_equal(rv, "_build/p/lol.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, NULL, "q", "lol", NULL, ".html");
    assert_string_equal(rv, "_build/q/lol.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, "p", "q", "lol", NULL, ".html");
    assert_string_equal(rv, "_build/p/q/lol.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, NULL, NULL, "lol", NULL, "/index.html");
    assert_string_equal(rv, "_build/lol/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, "p", NULL, "lol", NULL, "/index.html");
    assert_string_equal(rv, "_build/p/lol/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, NULL, "q", "lol", NULL, "/index.html");
    assert_string_equal(rv, "_build/q/lol/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, "p", "q", "lol", NULL, "/index.html");
    assert_string_equal(rv, "_build/p/q/lol/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, NULL, NULL, NULL, "foo", ".html");
    assert_string_equal(rv, "_build/foo.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, "p", NULL, NULL, "foo", ".html");
    assert_string_equal(rv, "_build/p/foo.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, NULL, "q", NULL, "foo", ".html");
    assert_string_equal(rv, "_build/q/foo.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, "p", "q", NULL, "foo", ".html");
    assert_string_equal(rv, "_build/p/q/foo.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, NULL, NULL, NULL, "foo", "/index.html");
    assert_string_equal(rv, "_build/foo/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, "p", NULL, NULL, "foo", "/index.html");
    assert_string_equal(rv, "_build/p/foo/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, NULL, "q", NULL, "foo", "/index.html");
    assert_string_equal(rv, "_build/q/foo/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, "p", "q", NULL, "foo", "/index.html");
    assert_string_equal(rv, "_build/p/q/foo/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, NULL, NULL, NULL, "index", ".html");
    assert_string_equal(rv, "_build/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, "p", NULL, NULL, "index", ".html");
    assert_string_equal(rv, "_build/p/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, NULL, "q", NULL, "index", ".html");
    assert_string_equal(rv, "_build/q/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, "p", "q", NULL, "index", ".html");
    assert_string_equal(rv, "_build/p/q/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, NULL, NULL, NULL, "index", "/index.html");
    assert_string_equal(rv, "_build/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, "p", NULL, NULL, "index", "/index.html");
    assert_string_equal(rv, "_build/p/index/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, NULL, "q", NULL, "index", "/index.html");
    assert_string_equal(rv, "_build/q/index/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, "p", "q", NULL, "index", "/index.html");
    assert_string_equal(rv, "_build/p/q/index/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, NULL, NULL, "bar", "foo", ".html");
    assert_string_equal(rv, "_build/bar/foo.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, "p", NULL, "bar", "foo", ".html");
    assert_string_equal(rv, "_build/p/bar/foo.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, NULL, "q", "bar", "foo", ".html");
    assert_string_equal(rv, "_build/q/bar/foo.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, "p", "q", "bar", "foo", ".html");
    assert_string_equal(rv, "_build/p/q/bar/foo.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, NULL, NULL, "bar", "foo", "/index.html");
    assert_string_equal(rv, "_build/bar/foo/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, "p", NULL, "bar", "foo", "/index.html");
    assert_string_equal(rv, "_build/p/bar/foo/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, NULL, "q", "bar", "foo", "/index.html");
    assert_string_equal(rv, "_build/q/bar/foo/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", NULL, "p", "q", "bar", "foo", "/index.html");
    assert_string_equal(rv, "_build/p/q/bar/foo/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "foo", NULL, NULL, NULL, NULL, ".html");
    assert_string_equal(rv, "/foo/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "foo", "p", NULL, NULL, NULL, ".html");
    assert_string_equal(rv, "/foo/p.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "foo", NULL, "q", NULL, NULL, ".html");
    assert_string_equal(rv, "/foo/q.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "foo", "p", "q", NULL, NULL, ".html");
    assert_string_equal(rv, "/foo/p/q.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "foo", NULL, NULL, NULL, NULL, "/index.html");
    assert_string_equal(rv, "/foo/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "foo", "p", NULL, NULL, NULL, "/index.html");
    assert_string_equal(rv, "/foo/p/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "foo", NULL, "q", NULL, NULL, "/index.html");
    assert_string_equal(rv, "/foo/q/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "foo", "p", "q", NULL, NULL, "/index.html");
    assert_string_equal(rv, "/foo/p/q/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "foo", NULL, NULL, "lol", NULL, ".html");
    assert_string_equal(rv, "/foo/lol.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "foo", "p", NULL, "lol", NULL, ".html");
    assert_string_equal(rv, "/foo/p/lol.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "foo", NULL, "q", "lol", NULL, ".html");
    assert_string_equal(rv, "/foo/q/lol.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "foo", "p", "q", "lol", NULL, ".html");
    assert_string_equal(rv, "/foo/p/q/lol.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "foo", NULL, NULL, "lol", NULL, "/index.html");
    assert_string_equal(rv, "/foo/lol/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "foo", "p", NULL, "lol", NULL, "/index.html");
    assert_string_equal(rv, "/foo/p/lol/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "foo", NULL, "q", "lol", NULL, "/index.html");
    assert_string_equal(rv, "/foo/q/lol/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "foo", "p", "q", "lol", NULL, "/index.html");
    assert_string_equal(rv, "/foo/p/q/lol/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "foo", NULL, NULL, NULL, "foo", ".html");
    assert_string_equal(rv, "/foo/foo.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "foo", "p", NULL, NULL, "foo", ".html");
    assert_string_equal(rv, "/foo/p/foo.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "foo", NULL, "q", NULL, "foo", ".html");
    assert_string_equal(rv, "/foo/q/foo.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "foo", "p", "q", NULL, "foo", ".html");
    assert_string_equal(rv, "/foo/p/q/foo.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "foo", NULL, NULL, NULL, "foo", "/index.html");
    assert_string_equal(rv, "/foo/foo/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "foo", "p", NULL, NULL, "foo", "/index.html");
    assert_string_equal(rv, "/foo/p/foo/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "foo", NULL, "q", NULL, "foo", "/index.html");
    assert_string_equal(rv, "/foo/q/foo/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "foo", "p", "q", NULL, "foo", "/index.html");
    assert_string_equal(rv, "/foo/p/q/foo/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "foo", NULL, NULL, NULL, "index", ".html");
    assert_string_equal(rv, "/foo/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "foo", "p", NULL, NULL, "index", ".html");
    assert_string_equal(rv, "/foo/p/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "foo", NULL, "q", NULL, "index", ".html");
    assert_string_equal(rv, "/foo/q/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "foo", "p", "q", NULL, "index", ".html");
    assert_string_equal(rv, "/foo/p/q/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "foo", NULL, NULL, NULL, "index", "/index.html");
    assert_string_equal(rv, "/foo/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "foo", "p", NULL, NULL, "index", "/index.html");
    assert_string_equal(rv, "/foo/p/index/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "foo", NULL, "q", NULL, "index", "/index.html");
    assert_string_equal(rv, "/foo/q/index/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "foo", "p", "q", NULL, "index", "/index.html");
    assert_string_equal(rv, "/foo/p/q/index/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "foo", NULL, NULL, "bar", "foo", ".html");
    assert_string_equal(rv, "/foo/bar/foo.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "foo", "p", NULL, "bar", "foo", ".html");
    assert_string_equal(rv, "/foo/p/bar/foo.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "foo", NULL, "q", "bar", "foo", ".html");
    assert_string_equal(rv, "/foo/q/bar/foo.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "foo", "p", "q", "bar", "foo", ".html");
    assert_string_equal(rv, "/foo/p/q/bar/foo.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "foo", NULL, NULL, "bar", "foo", "/index.html");
    assert_string_equal(rv, "/foo/bar/foo/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "foo", "p", NULL, "bar", "foo", "/index.html");
    assert_string_equal(rv, "/foo/p/bar/foo/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "foo", NULL, "q", "bar", "foo", "/index.html");
    assert_string_equal(rv, "/foo/q/bar/foo/index.html");
    free(rv);

    rv = bm_generate_filename2(NULL, "foo", "p", "q", "bar", "foo", "/index.html");
    assert_string_equal(rv, "/foo/p/q/bar/foo/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", "foo", NULL, NULL, NULL, NULL, ".html");
    assert_string_equal(rv, "_build/foo/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", "foo", "p", NULL, NULL, NULL, ".html");
    assert_string_equal(rv, "_build/foo/p.html");
    free(rv);

    rv = bm_generate_filename2("_build", "foo", NULL, "q", NULL, NULL, ".html");
    assert_string_equal(rv, "_build/foo/q.html");
    free(rv);

    rv = bm_generate_filename2("_build", "foo", "p", "q", NULL, NULL, ".html");
    assert_string_equal(rv, "_build/foo/p/q.html");
    free(rv);

    rv = bm_generate_filename2("_build", "foo", NULL, NULL, NULL, NULL, "/index.html");
    assert_string_equal(rv, "_build/foo/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", "foo", "p", NULL, NULL, NULL, "/index.html");
    assert_string_equal(rv, "_build/foo/p/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", "foo", NULL, "q", NULL, NULL, "/index.html");
    assert_string_equal(rv, "_build/foo/q/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", "foo", "p", "q", NULL, NULL, "/index.html");
    assert_string_equal(rv, "_build/foo/p/q/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", "foo", NULL, NULL, "lol", NULL, ".html");
    assert_string_equal(rv, "_build/foo/lol.html");
    free(rv);

    rv = bm_generate_filename2("_build", "foo", "p", NULL, "lol", NULL, ".html");
    assert_string_equal(rv, "_build/foo/p/lol.html");
    free(rv);

    rv = bm_generate_filename2("_build", "foo", NULL, "q", "lol", NULL, ".html");
    assert_string_equal(rv, "_build/foo/q/lol.html");
    free(rv);

    rv = bm_generate_filename2("_build", "foo", "p", "q", "lol", NULL, ".html");
    assert_string_equal(rv, "_build/foo/p/q/lol.html");
    free(rv);

    rv = bm_generate_filename2("_build", "foo", NULL, NULL, "lol", NULL, "/index.html");
    assert_string_equal(rv, "_build/foo/lol/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", "foo", "p", NULL, "lol", NULL, "/index.html");
    assert_string_equal(rv, "_build/foo/p/lol/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", "foo", NULL, "q", "lol", NULL, "/index.html");
    assert_string_equal(rv, "_build/foo/q/lol/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", "foo", "p", "q", "lol", NULL, "/index.html");
    assert_string_equal(rv, "_build/foo/p/q/lol/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", "foo", NULL, NULL, NULL, "foo", ".html");
    assert_string_equal(rv, "_build/foo/foo.html");
    free(rv);

    rv = bm_generate_filename2("_build", "foo", "p", NULL, NULL, "foo", ".html");
    assert_string_equal(rv, "_build/foo/p/foo.html");
    free(rv);

    rv = bm_generate_filename2("_build", "foo", NULL, "q", NULL, "foo", ".html");
    assert_string_equal(rv, "_build/foo/q/foo.html");
    free(rv);

    rv = bm_generate_filename2("_build", "foo", "p", "q", NULL, "foo", ".html");
    assert_string_equal(rv, "_build/foo/p/q/foo.html");
    free(rv);

    rv = bm_generate_filename2("_build", "foo", NULL, NULL, NULL, "foo", "/index.html");
    assert_string_equal(rv, "_build/foo/foo/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", "foo", "p", NULL, NULL, "foo", "/index.html");
    assert_string_equal(rv, "_build/foo/p/foo/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", "foo", NULL, "q", NULL, "foo", "/index.html");
    assert_string_equal(rv, "_build/foo/q/foo/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", "foo", "p", "q", NULL, "foo", "/index.html");
    assert_string_equal(rv, "_build/foo/p/q/foo/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", "foo", NULL, NULL, NULL, "index", ".html");
    assert_string_equal(rv, "_build/foo/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", "foo", "p", NULL, NULL, "index", ".html");
    assert_string_equal(rv, "_build/foo/p/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", "foo", NULL, "q", NULL, "index", ".html");
    assert_string_equal(rv, "_build/foo/q/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", "foo", "p", "q", NULL, "index", ".html");
    assert_string_equal(rv, "_build/foo/p/q/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", "foo", NULL, NULL, NULL, "index", "/index.html");
    assert_string_equal(rv, "_build/foo/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", "foo", "p", NULL, NULL, "index", "/index.html");
    assert_string_equal(rv, "_build/foo/p/index/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", "foo", NULL, "q", NULL, "index", "/index.html");
    assert_string_equal(rv, "_build/foo/q/index/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", "foo", "p", "q", NULL, "index", "/index.html");
    assert_string_equal(rv, "_build/foo/p/q/index/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", "foo", NULL, NULL, "bar", "foo", ".html");
    assert_string_equal(rv, "_build/foo/bar/foo.html");
    free(rv);

    rv = bm_generate_filename2("_build", "foo", "p", NULL, "bar", "foo", ".html");
    assert_string_equal(rv, "_build/foo/p/bar/foo.html");
    free(rv);

    rv = bm_generate_filename2("_build", "foo", NULL, "q", "bar", "foo", ".html");
    assert_string_equal(rv, "_build/foo/q/bar/foo.html");
    free(rv);

    rv = bm_generate_filename2("_build", "foo", "p", "q", "bar", "foo", ".html");
    assert_string_equal(rv, "_build/foo/p/q/bar/foo.html");
    free(rv);

    rv = bm_generate_filename2("_build", "foo", NULL, NULL, "bar", "foo", "/index.html");
    assert_string_equal(rv, "_build/foo/bar/foo/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", "foo", "p", NULL, "bar", "foo", "/index.html");
    assert_string_equal(rv, "_build/foo/p/bar/foo/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", "foo", NULL, "q", "bar", "foo", "/index.html");
    assert_string_equal(rv, "_build/foo/q/bar/foo/index.html");
    free(rv);

    rv = bm_generate_filename2("_build", "foo", "p", "q", "bar", "foo", "/index.html");
    assert_string_equal(rv, "_build/foo/p/q/bar/foo/index.html");
    free(rv);
}


int
main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_generate_filename),
        cmocka_unit_test(test_generate_filename2),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
