/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2017 Rafael G. Martins <rafael@rafaelmartins.eng.br>
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
#include <unistd.h>

#include "../../src/blogc-make/exec.h"
#include "../../src/blogc-make/settings.h"
#include "../../src/common/utils.h"


int
__wrap_access(const char *pathname, int mode)
{
    assert_int_equal(mode, X_OK);
    assert_string_equal(pathname, mock_type(const char*));
    return mock_type(int);
}


static void
test_find_binary(void **state)
{
    unsetenv("BLOGC");

    char *bin = bm_exec_find_binary(NULL, "blogc", "BLOGC");
    assert_string_equal(bin, "blogc");
    free(bin);

    bin = bm_exec_find_binary("blogc-make", "blogc", "BLOGC");
    assert_string_equal(bin, "blogc");
    free(bin);

    will_return(__wrap_access, "../blogc");
    will_return(__wrap_access, 0);
    bin = bm_exec_find_binary("../blogc-make", "blogc", "BLOGC");
    assert_string_equal(bin, "'../blogc'");
    free(bin);

    will_return(__wrap_access, "/usr/bin/blogc");
    will_return(__wrap_access, 0);
    bin = bm_exec_find_binary("/usr/bin/blogc-make", "blogc", "BLOGC");
    assert_string_equal(bin, "'/usr/bin/blogc'");
    free(bin);

    will_return(__wrap_access, "../blogc");
    will_return(__wrap_access, 1);
    bin = bm_exec_find_binary("../blogc-make", "blogc", "BLOGC");
    assert_string_equal(bin, "blogc");
    free(bin);

    setenv("BLOGC", "/path/to/blogc", 1);
    bin = bm_exec_find_binary(NULL, "blogc", "BLOGC");
    assert_string_equal(bin, "'/path/to/blogc'");
    free(bin);
    unsetenv("BLOGC");
}


static void
test_build_blogc_cmd_with_settings(void **state)
{
    bm_settings_t *settings = bc_malloc(sizeof(bm_settings_t));
    settings->settings = bc_trie_new(free);
    bc_trie_insert(settings->settings, "locale", bc_strdup("en_US.utf8"));
    settings->env = bc_trie_new(free);
    bc_trie_insert(settings->env, "FOO", bc_strdup("BAR"));
    bc_trie_insert(settings->env, "BAR", bc_strdup("BAZ"));
    bc_trie_t *variables = bc_trie_new(free);
    bc_trie_insert(variables, "LOL", bc_strdup("HEHE"));

    char *rv = bm_exec_build_blogc_cmd("blogc", settings, variables, true,
        "main.tmpl", "foo.html", true);
    assert_string_equal(rv,
        "LC_ALL='en_US.utf8' blogc -D FOO='BAR' -D BAR='BAZ' -D LOL='HEHE' -l "
        "-t 'main.tmpl' -o 'foo.html' -i");
    free(rv);

    rv = bm_exec_build_blogc_cmd("blogc", settings, variables, false, NULL, NULL,
        false);
    assert_string_equal(rv,
        "LC_ALL='en_US.utf8' blogc -D FOO='BAR' -D BAR='BAZ' -D LOL='HEHE'");
    free(rv);

    rv = bm_exec_build_blogc_cmd("blogc", settings, NULL, false, NULL, NULL,
        false);
    assert_string_equal(rv,
        "LC_ALL='en_US.utf8' blogc -D FOO='BAR' -D BAR='BAZ'");
    free(rv);

    bc_trie_free(variables);
    bc_trie_free(settings->settings);
    bc_trie_free(settings->env);
    free(settings);
}



static void
test_build_blogc_cmd_without_settings(void **state)
{
    bc_trie_t *variables = bc_trie_new(free);
    bc_trie_insert(variables, "LOL", bc_strdup("HEHE"));

    char *rv = bm_exec_build_blogc_cmd("blogc", NULL, variables, true,
        "main.tmpl", "foo.html", true);
    assert_string_equal(rv,
        "blogc -D LOL='HEHE' -l -t 'main.tmpl' -o 'foo.html' -i");
    free(rv);

    rv = bm_exec_build_blogc_cmd("blogc", NULL, variables, false, NULL, NULL,
        false);
    assert_string_equal(rv,
        "blogc -D LOL='HEHE'");
    free(rv);

    rv = bm_exec_build_blogc_cmd("blogc", NULL, NULL, false, NULL, NULL, false);
    assert_string_equal(rv,
        "blogc");
    free(rv);

    bc_trie_free(variables);
}


int
main(void)
{
    const UnitTest tests[] = {
        unit_test(test_find_binary),
        unit_test(test_build_blogc_cmd_with_settings),
        unit_test(test_build_blogc_cmd_without_settings),
    };
    return run_tests(tests);
}
