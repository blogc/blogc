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

#ifdef HAVE_SYSINFO_DATETIME
#include <time.h>
#endif

#ifdef HAVE_NETDB_H
#include <netdb.h>

static struct hostent h = {
    .h_name = "bola.example.com",
};

struct hostent*
__wrap_gethostbyname(const char *name)
{
    if (0 == strcmp(name, "bola"))
        return &h;
    return NULL;
}
#endif


#ifdef HAVE_SYSINFO_HOSTNAME
int
__wrap_gethostname(char *name, size_t len)
{
    assert_int_equal(len, 1024);
    const char *f = mock_type(const char*);
    if (f != NULL)
        strcpy(name, f);
    return mock_type(int);
}
#endif


char*
__wrap_getenv(const char *name)
{
    return mock_type(char*);
}


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
    assert_string_equal(f, "bola.example.com");
    free(f);

    will_return(__wrap_gethostname, "guda");
    will_return(__wrap_gethostname, 0);
    f = blogc_sysinfo_get_hostname();
    assert_non_null(f);
    assert_string_equal(f, "guda");
    free(f);
}


static void
test_sysinfo_inject_hostname(void **state)
{
    bc_trie_t *t = bc_trie_new(free);

    will_return(__wrap_gethostname, NULL);
    will_return(__wrap_gethostname, -1);
    blogc_sysinfo_inject_hostname(t);
    assert_int_equal(bc_trie_size(t), 0);

    will_return(__wrap_gethostname, "bola");
    will_return(__wrap_gethostname, 0);
    blogc_sysinfo_inject_hostname(t);
    assert_int_equal(bc_trie_size(t), 1);
    assert_string_equal(bc_trie_lookup(t, "BLOGC_SYSINFO_HOSTNAME"), "bola.example.com");
    bc_trie_free(t);

    t = bc_trie_new(free);
    will_return(__wrap_gethostname, "guda");
    will_return(__wrap_gethostname, 0);
    blogc_sysinfo_inject_hostname(t);
    assert_int_equal(bc_trie_size(t), 1);
    assert_string_equal(bc_trie_lookup(t, "BLOGC_SYSINFO_HOSTNAME"), "guda");
    bc_trie_free(t);
}


static void
test_sysinfo_get_username(void **state)
{
    will_return(__wrap_getenv, NULL);
    char *f = blogc_sysinfo_get_username();
    assert_null(f);

    will_return(__wrap_getenv, "bola");
    f = blogc_sysinfo_get_username();
    assert_non_null(f);
    assert_string_equal(f, "bola");
    free(f);
}


static void
test_sysinfo_inject_username(void **state)
{
    bc_trie_t *t = bc_trie_new(free);

    will_return(__wrap_getenv, NULL);
    blogc_sysinfo_inject_username(t);
    assert_int_equal(bc_trie_size(t), 0);

    will_return(__wrap_getenv, "bola");
    blogc_sysinfo_inject_username(t);
    assert_int_equal(bc_trie_size(t), 1);
    assert_string_equal(bc_trie_lookup(t, "BLOGC_SYSINFO_USERNAME"), "bola");
    bc_trie_free(t);
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
test_sysinfo_inject_datetime(void **state)
{
    bc_trie_t *t = bc_trie_new(free);

    will_return(__wrap_time, -1);
    blogc_sysinfo_inject_datetime(t);
    assert_int_equal(bc_trie_size(t), 0);

    will_return(__wrap_time, 2);
    blogc_sysinfo_inject_datetime(t);
    assert_int_equal(bc_trie_size(t), 0);

    will_return(__wrap_time, 1);
    blogc_sysinfo_inject_datetime(t);
    assert_int_equal(bc_trie_size(t), 1);
    assert_string_equal(bc_trie_lookup(t, "BLOGC_SYSINFO_DATETIME"), "1906-06-04 03:02:01");
    bc_trie_free(t);
}


static void
test_sysinfo_get_inside_docker(void **state)
{
    // the "positive" case was already tested in check_funcvars. this is done
    // this way because this function caches the results in a global variable.
    will_return(__wrap_bc_file_get_contents, bc_strdup("bola"));
    assert_false(blogc_sysinfo_get_inside_docker());
}


// the blogc_sysinfo_inject_inside_docker() function was already indirectly
// tested in check_funcvars.c


int
main(void)
{
    const struct CMUnitTest tests[] = {

#ifdef HAVE_SYSINFO_HOSTNAME
        cmocka_unit_test(test_sysinfo_get_hostname),
        cmocka_unit_test(test_sysinfo_inject_hostname),
#endif

        cmocka_unit_test(test_sysinfo_get_username),
        cmocka_unit_test(test_sysinfo_inject_username),

#ifdef HAVE_SYSINFO_DATETIME
        cmocka_unit_test(test_sysinfo_get_datetime),
        cmocka_unit_test(test_sysinfo_inject_datetime),
#endif

        cmocka_unit_test(test_sysinfo_get_inside_docker),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
