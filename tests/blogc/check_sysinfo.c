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
#include "../../src/common/error.h"
#include "../../src/common/utils.h"
#include "../../src/blogc/sysinfo.h"

#ifdef HAVE_SYSINFO_HOSTNAME
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#endif

#ifdef HAVE_SYSINFO_DATETIME
#include <time.h>
#endif

int
__wrap_gethostname(char *name, size_t len)
{
    assert_int_equal(len, 1024);
    const char *f = mock_type(const char*);
    if (f != NULL)
        strcpy(name, f);
    return mock_type(int);
}


#ifdef HAVE_SYSINFO_HOSTNAME
uid_t
__wrap_geteuid(void)
{
    return 1234;
}


static struct passwd pw;

struct passwd*
__wrap_getpwuid(uid_t uid)
{
    assert_int_equal(uid, 1234);
    char *n = mock_type(char*);
    if (n == NULL)
        return NULL;
    pw.pw_name = n;
    return &pw;
}
#endif


#ifdef HAVE_SYSINFO_DATETIME
time_t
__wrap_time(time_t *tloc)
{
    *tloc = mock_type(time_t);
    return *tloc;
}


static struct tm tm = {
    .tm_sec = 1,
    .tm_min = 2,
    .tm_hour = 3,
    .tm_mday = 4,
    .tm_mon = 5,
    .tm_year = 6,
    .tm_wday = 6,
    .tm_yday = 7,
    .tm_isdst = 0,
};

struct tm*
__wrap_gmtime(const time_t *timep)
{
    if (*timep == 2)
        return NULL;
    return &tm;
}
#endif


char*
__wrap_bc_file_get_contents(const char *path, bool utf8, size_t *len, bc_error_t **err)
{
    assert_string_equal(path, "/proc/1/cgroup");
    assert_false(utf8);
    char *rv = mock_type(char*);
    *len = strlen(rv);
    return rv;
}


static void
test_sysinfo_get_hostname(void **state)
{
    will_return(__wrap_gethostname, NULL);
    will_return(__wrap_gethostname, -1);
    char *f = blogc_sysinfo_get_hostname();
    assert_null(f);

    will_return(__wrap_gethostname, "bola");
    will_return(__wrap_gethostname, 0);
    f = blogc_sysinfo_get_hostname();
    assert_non_null(f);
    assert_string_equal(f, "bola");
    free(f);
}


static void
test_sysinfo_get_username(void **state)
{
    will_return(__wrap_getpwuid, NULL);
    char *f = blogc_sysinfo_get_username();
    assert_null(f);

    will_return(__wrap_getpwuid, "bola");
    f = blogc_sysinfo_get_username();
    assert_non_null(f);
    assert_string_equal(f, "bola");
    free(f);
}


static void
test_sysinfo_get_datetime(void **state)
{
    will_return(__wrap_time, -1);
    char *f = blogc_sysinfo_get_datetime();
    assert_null(f);

    will_return(__wrap_time, 2);
    f = blogc_sysinfo_get_datetime();
    assert_null(f);

    will_return(__wrap_time, 1);
    f = blogc_sysinfo_get_datetime();
    assert_non_null(f);
    assert_string_equal(f, "1906-06-04 03:02:01");
    free(f);
}


static void
test_sysinfo_get_inside_docker(void **state)
{
    // the "positive" case was already tested in check_funcvars. this is done
    // this way because this function caches the results in a global variable.
    will_return(__wrap_bc_file_get_contents, bc_strdup("bola"));
    assert_false(blogc_sysinfo_get_inside_docker());
}


int
main(void)
{
    const UnitTest tests[] = {

#ifdef HAVE_SYSINFO_HOSTNAME
        unit_test(test_sysinfo_get_hostname),
#endif

#ifdef HAVE_SYSINFO_USERNAME
        unit_test(test_sysinfo_get_username),
#endif

#ifdef HAVE_SYSINFO_DATETIME
        unit_test(test_sysinfo_get_datetime),
#endif

        unit_test(test_sysinfo_get_inside_docker),
    };
    return run_tests(tests);
}
