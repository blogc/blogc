// SPDX-FileCopyrightText: 2014-2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
// SPDX-License-Identifier: BSD-3-Clause

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../../src/common/error.h"
#include "../../src/common/utils.h"
#include "../../src/blogc/sysinfo.h"

// this test exists because we can't test more than one return values for
// blogc_sysinfo_get_inside_docker() in the same binary, because the results
// are cached globally for performance.


char*
__wrap_bc_file_get_contents(const char *path, bool utf8, size_t *len, bc_error_t **err)
{
    assert_string_equal(path, "/proc/1/cgroup");
    assert_false(utf8);
    *err = bc_error_new(0, "");
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
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_sysinfo_get_inside_docker),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
