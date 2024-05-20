// SPDX-FileCopyrightText: 2014-2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
// SPDX-License-Identifier: BSD-3-Clause

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

    f = blogc_rusage_format_cpu_time(3000);
    assert_string_equal(f, "3ms");
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


static void
test_rusage_inject(void **state)
{
    bc_trie_t *t = bc_trie_new(free);

    will_return(__wrap_getrusage, -1);
    blogc_rusage_inject(t);
    assert_int_equal(bc_trie_size(t), 0);

    will_return(__wrap_getrusage, 0);
    blogc_rusage_inject(t);
    assert_int_equal(bc_trie_size(t), 2);
    assert_string_equal(bc_trie_lookup(t, "BLOGC_RUSAGE_CPU_TIME"), "4.000s");
    assert_string_equal(bc_trie_lookup(t, "BLOGC_RUSAGE_MEMORY"), "10.010MB");
    bc_trie_free(t);
}


int
main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_rusage_get),
        cmocka_unit_test(test_rusage_format_cpu_time),
        cmocka_unit_test(test_rusage_format_memory),
        cmocka_unit_test(test_rusage_inject),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
