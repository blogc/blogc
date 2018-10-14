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
#include <unistd.h>
#include "../../src/blogc-runserver/mime.h"


int
__wrap_access(const char *pathname, int mode)
{
    assert_string_equal(pathname, mock_type(const char*));
    assert_int_equal(mode, F_OK);
    return mock_type(int);
}


static void
test_guess_content_type(void **state)
{
    assert_string_equal(br_mime_guess_content_type("foo.html"), "text/html");
    assert_string_equal(br_mime_guess_content_type("foo.jpg"), "image/jpeg");
    assert_string_equal(br_mime_guess_content_type("foo.mp4"), "video/mp4");
    assert_string_equal(br_mime_guess_content_type("foo.bola"), "application/octet-stream");
}


static void
test_guess_index(void **state)
{
    char *t;
    will_return(__wrap_access, "dir/index.html");
    will_return(__wrap_access, 0);
    t = br_mime_guess_index("dir");
    assert_string_equal(t, "dir/index.html");
    free(t);
    will_return(__wrap_access, "dir/index.html");
    will_return(__wrap_access, 1);
    will_return(__wrap_access, "dir/index.htm");
    will_return(__wrap_access, 0);
    t = br_mime_guess_index("dir");
    assert_string_equal(t, "dir/index.htm");
    free(t);
    will_return(__wrap_access, "dir/index.html");
    will_return(__wrap_access, 1);
    will_return(__wrap_access, "dir/index.htm");
    will_return(__wrap_access, 1);
    will_return(__wrap_access, "dir/index.shtml");
    will_return(__wrap_access, 1);
    will_return(__wrap_access, "dir/index.xml");
    will_return(__wrap_access, 1);
    will_return(__wrap_access, "dir/index.txt");
    will_return(__wrap_access, 1);
    will_return(__wrap_access, "dir/index.xhtml");
    will_return(__wrap_access, 0);
    t = br_mime_guess_index("dir");
    assert_string_equal(t, "dir/index.xhtml");
    free(t);
    will_return(__wrap_access, "dir/index.html");
    will_return(__wrap_access, 1);
    will_return(__wrap_access, "dir/index.htm");
    will_return(__wrap_access, 1);
    will_return(__wrap_access, "dir/index.shtml");
    will_return(__wrap_access, 1);
    will_return(__wrap_access, "dir/index.xml");
    will_return(__wrap_access, 1);
    will_return(__wrap_access, "dir/index.txt");
    will_return(__wrap_access, 1);
    will_return(__wrap_access, "dir/index.xhtml");
    will_return(__wrap_access, 1);
    assert_null(br_mime_guess_index("dir"));
}


int
main(void)
{
    const UnitTest tests[] = {
        unit_test(test_guess_content_type),
        unit_test(test_guess_index),
    };
    return run_tests(tests);
}
