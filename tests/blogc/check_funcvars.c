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

#include "../../src/blogc/funcvars.h"


char*
__wrap_sb_file_get_contents(const char *path, size_t *len, sb_error_t **err)
{
    assert_string_equal(path, "/proc/1/cgroup");
    char *rv = mock_type(char*);
    *len = strlen(rv);
    return rv;
}


static void
test_funcvars_eval(void **state)
{
    sb_trie_t *t = sb_trie_new(free);
    blogc_funcvars_eval(t, NULL);
    blogc_funcvars_eval(t, "");
    blogc_funcvars_eval(t, "BOLA");
    assert_int_equal(sb_trie_size(t), 0);

    sb_trie_insert(t, "BOLA", sb_strdup("GUDA"));
    blogc_funcvars_eval(t, "BOLA");
    assert_int_equal(sb_trie_size(t), 1);

    sb_trie_free(t);
}


static void
test_funcvars_eval_mocked(void **state)
{
    sb_trie_t *t = sb_trie_new(free);

    // this is the only function that isn't hidden behind conditional macros
    // as of when this test was written. the other functions should be tested
    // separately
    will_return(__wrap_sb_file_get_contents, sb_strdup("asd/docker/asd"));
    blogc_funcvars_eval(t, "BLOGC_SYSINFO_INSIDE_DOCKER");
    assert_string_equal(sb_trie_lookup(t, "BLOGC_SYSINFO_INSIDE_DOCKER"), "1");
    assert_int_equal(sb_trie_size(t), 1);

    // this specific function call is cached, so calling it again should not
    // call sb_file_get_contents_again
    blogc_funcvars_eval(t, "BLOGC_SYSINFO_INSIDE_DOCKER");
    assert_string_equal(sb_trie_lookup(t, "BLOGC_SYSINFO_INSIDE_DOCKER"), "1");
    assert_int_equal(sb_trie_size(t), 1);

    sb_trie_free(t);
}


int
main(void)
{
    const UnitTest tests[] = {
        unit_test(test_funcvars_eval),
        unit_test(test_funcvars_eval_mocked),
    };
    return run_tests(tests);
}
