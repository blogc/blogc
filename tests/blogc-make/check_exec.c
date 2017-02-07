/*
 * blogc: A blog compiler.
 * Copyright (C) 2015-2016 Rafael G. Martins <rafael@rafaelmartins.eng.br>
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

#include "../../src/blogc-make/exec.h"
#include "../../src/blogc-make/settings.h"
#include "../../src/common/utils.h"


static void
test_build_blogc_cmd_with_settings(void **state)
{
    unsetenv("BLOGC");

    bm_settings_t *settings = bc_malloc(sizeof(bm_settings_t));
    settings->settings = bc_trie_new(free);
    bc_trie_insert(settings->settings, "locale", bc_strdup("en_US.utf8"));
    settings->env = bc_trie_new(free);
    bc_trie_insert(settings->env, "FOO", bc_strdup("BAR"));
    bc_trie_insert(settings->env, "BAR", bc_strdup("BAZ"));
    bc_trie_t *variables = bc_trie_new(free);
    bc_trie_insert(variables, "LOL", bc_strdup("HEHE"));

    char *rv = bm_exec_build_blogc_cmd(settings, variables, true, "main.tmpl",
        "foo.html", true);
    assert_string_equal(rv,
        "LC_ALL='en_US.utf8' blogc -D FOO='BAR' -D BAR='BAZ' -D LOL='HEHE' -l "
        "-t 'main.tmpl' -o 'foo.html' -i");
    free(rv);

    rv = bm_exec_build_blogc_cmd(settings, variables, false, NULL, NULL, false);
    assert_string_equal(rv,
        "LC_ALL='en_US.utf8' blogc -D FOO='BAR' -D BAR='BAZ' -D LOL='HEHE'");
    free(rv);

    rv = bm_exec_build_blogc_cmd(settings, NULL, false, NULL, NULL, false);
    assert_string_equal(rv,
        "LC_ALL='en_US.utf8' blogc -D FOO='BAR' -D BAR='BAZ'");
    free(rv);

    setenv("BLOGC", "/path/to/blogc", 1);

    rv = bm_exec_build_blogc_cmd(settings, variables, true, "main.tmpl",
        "foo.html", true);
    assert_string_equal(rv,
        "LC_ALL='en_US.utf8' '/path/to/blogc' -D FOO='BAR' -D BAR='BAZ' "
        "-D LOL='HEHE' -l -t 'main.tmpl' -o 'foo.html' -i");
    free(rv);

    rv = bm_exec_build_blogc_cmd(settings, variables, false, NULL, NULL, false);
    assert_string_equal(rv,
        "LC_ALL='en_US.utf8' '/path/to/blogc' -D FOO='BAR' -D BAR='BAZ' "
        "-D LOL='HEHE'");
    free(rv);

    rv = bm_exec_build_blogc_cmd(settings, NULL, false, NULL, NULL, false);
    assert_string_equal(rv,
        "LC_ALL='en_US.utf8' '/path/to/blogc' -D FOO='BAR' -D BAR='BAZ'");
    free(rv);

    unsetenv("BLOGC");

    bc_trie_free(variables);
    bc_trie_free(settings->settings);
    bc_trie_free(settings->env);
    free(settings);
}


static void
test_build_blogc_cmd_without_settings(void **state)
{
    unsetenv("BLOGC");

    bc_trie_t *variables = bc_trie_new(free);
    bc_trie_insert(variables, "LOL", bc_strdup("HEHE"));

    char *rv = bm_exec_build_blogc_cmd(NULL, variables, true, "main.tmpl",
        "foo.html", true);
    assert_string_equal(rv,
        "blogc -D LOL='HEHE' -l -t 'main.tmpl' -o 'foo.html' -i");
    free(rv);

    rv = bm_exec_build_blogc_cmd(NULL, variables, false, NULL, NULL, false);
    assert_string_equal(rv,
        "blogc -D LOL='HEHE'");
    free(rv);

    rv = bm_exec_build_blogc_cmd(NULL, NULL, false, NULL, NULL, false);
    assert_string_equal(rv,
        "blogc");
    free(rv);

    setenv("BLOGC", "/path/to/blogc", 1);

    rv = bm_exec_build_blogc_cmd(NULL, variables, true, "main.tmpl", "foo.html",
        true);
    assert_string_equal(rv,
        "'/path/to/blogc' -D LOL='HEHE' -l -t 'main.tmpl' -o 'foo.html' -i");
    free(rv);

    rv = bm_exec_build_blogc_cmd(NULL, variables, false, NULL, NULL, false);
    assert_string_equal(rv,
        "'/path/to/blogc' -D LOL='HEHE'");
    free(rv);

    rv = bm_exec_build_blogc_cmd(NULL, NULL, false, NULL, NULL, false);
    assert_string_equal(rv,
        "'/path/to/blogc'");
    free(rv);

    unsetenv("BLOGC");

    bc_trie_free(variables);
}


int
main(void)
{
    const UnitTest tests[] = {
        unit_test(test_build_blogc_cmd_with_settings),
        unit_test(test_build_blogc_cmd_without_settings),
    };
    return run_tests(tests);
}
