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
#include <squareball.h>

#include "../../src/blogc/sysinfo.h"

// this test exists because we can't test more than one return values for
// blogc_sysinfo_get_inside_docker() in the same binary, because the results
// are cached globally for performance.


char*
__wrap_sb_file_get_contents(const char *path, size_t *len, sb_error_t **err)
{
    assert_string_equal(path, "/proc/1/cgroup");
    *err = sb_strerror_new("");
    return NULL;
}


static void
test_sysinfo_get_inside_docker(void **state)
{
    assert_false(blogc_sysinfo_get_inside_docker());
}


int
main(void)
{
    const UnitTest tests[] = {
        unit_test(test_sysinfo_get_inside_docker),
    };
    return run_tests(tests);
}
