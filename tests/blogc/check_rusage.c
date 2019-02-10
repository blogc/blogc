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
#include <sys/time.h>
#include <sys/resource.h>
#include "../../src/common/utils.h"
#include "../../src/blogc/rusage.h"


int
__wrap_getrusage(int who, struct rusage *usage)
{
    assert_int_equal(who, RUSAGE_SELF);
    int rv = mock_type(int);
    if (rv == 0) {
        usage->ru_utime.tv_sec = 1;
        usage->ru_utime.tv_usec = 2;
        usage->ru_stime.tv_sec = 3;
        usage->ru_stime.tv_usec = 4;
        usage->ru_maxrss = 10250;
    }
    return rv;
}


static void
test_rusage_get(void **state)
{
    will_return(__wrap_getrusage, -1);
    assert_null(blogc_rusage_get());

    will_return(__wrap_getrusage, 0);
    blogc_rusage_t *r = blogc_rusage_get();
    assert_non_null(r);
    assert_int_equal(r->cpu_time, 4000006);
    assert_int_equal(r->memory, 10250);
    free(r);
}


static void
test_rusage_format_cpu_time(void **state)
{
    char *f = blogc_rusage_format_cpu_time(0);
    assert_string_equal(f, "0us");
    free(f);

    f = blogc_rusage_format_cpu_time(123);
    assert_string_equal(f, "123us");
    free(f);

    f = blogc_rusage_format_cpu_time(1234);
    assert_string_equal(f, "1.234ms");
    free(f);

    f = blogc_rusage_format_cpu_time(12345678);
    assert_string_equal(f, "12.346s");
    free(f);
}


static void
test_rusage_format_memory(void **state)
{
    char *f = blogc_rusage_format_memory(0);
    assert_string_equal(f, "0KB");
    free(f);

    f = blogc_rusage_format_memory(123);
    assert_string_equal(f, "123KB");
    free(f);

    f = blogc_rusage_format_memory(1234);
    assert_string_equal(f, "1.205MB");
    free(f);

    f = blogc_rusage_format_memory(12345678);
    assert_string_equal(f, "11.774GB");
    free(f);
}


int
main(void)
{
    const UnitTest tests[] = {
        unit_test(test_rusage_get),
        unit_test(test_rusage_format_cpu_time),
        unit_test(test_rusage_format_memory),
    };
    return run_tests(tests);
}
