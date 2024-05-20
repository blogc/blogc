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
#include "../../src/blogc/funcvars.h"


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
test_funcvars_eval(void **state)
{
    bc_trie_t *t = bc_trie_new(free);
    blogc_funcvars_eval(t, NULL);
    blogc_funcvars_eval(t, "");
    blogc_funcvars_eval(t, "BOLA");
    assert_int_equal(bc_trie_size(t), 0);

    bc_trie_insert(t, "BOLA", bc_strdup("GUDA"));
    blogc_funcvars_eval(t, "BOLA");
    assert_int_equal(bc_trie_size(t), 1);

    bc_trie_free(t);
}


static void
test_funcvars_eval_mocked(void **state)
{
    bc_trie_t *t = bc_trie_new(free);

    // this is the only function that isn't hidden behind conditional macros
    // as of when this test was written. the other functions should be tested
    // separately
    will_return(__wrap_bc_file_get_contents, bc_strdup("asd/docker/asd"));
    blogc_funcvars_eval(t, "BLOGC_SYSINFO_INSIDE_DOCKER");
    assert_string_equal(bc_trie_lookup(t, "BLOGC_SYSINFO_INSIDE_DOCKER"), "1");
    assert_int_equal(bc_trie_size(t), 1);

    // this specific function call is cached, so calling it again should not
    // call bc_file_get_contents_again
    blogc_funcvars_eval(t, "BLOGC_SYSINFO_INSIDE_DOCKER");
    assert_string_equal(bc_trie_lookup(t, "BLOGC_SYSINFO_INSIDE_DOCKER"), "1");
    assert_int_equal(bc_trie_size(t), 1);

    bc_trie_free(t);
}


int
main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_funcvars_eval),
        cmocka_unit_test(test_funcvars_eval_mocked),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
