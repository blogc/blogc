/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2019 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <squareball.h>

#include "../../src/blogc-make/exec.h"
#include "../../src/blogc-make/settings.h"


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
    bm_settings_t *settings = sb_malloc(sizeof(bm_settings_t));
    settings->settings = sb_trie_new(free);
    sb_trie_insert(settings->settings, "locale", sb_strdup("en_US.utf8"));
    settings->global = sb_trie_new(free);
    sb_trie_insert(settings->global, "FOO", sb_strdup("BAR"));
    sb_trie_insert(settings->global, "BAR", sb_strdup("BAZ"));
    sb_trie_t *variables = sb_trie_new(free);
    sb_trie_insert(variables, "LOL", sb_strdup("HEHE"));
    sb_trie_t *local = sb_trie_new(free);
    sb_trie_insert(local, "ASD", sb_strdup("QWE"));
    settings->tags = NULL;

    char *rv = bm_exec_build_blogc_cmd("blogc", settings, variables, local, NULL,
        true, NULL, "main.tmpl", "foo.html", false, true);
    assert_string_equal(rv,
        "LC_ALL='en_US.utf8' blogc -D FOO='BAR' -D BAR='BAZ' -D LOL='HEHE' "
        "-D ASD='QWE' -l -t 'main.tmpl' -o 'foo.html' -i");
    free(rv);

    rv = bm_exec_build_blogc_cmd("blogc", settings, variables, local, NULL, true,
        "foo.txt", "main.tmpl", "foo.html", false, true);
    assert_string_equal(rv,
        "LC_ALL='en_US.utf8' blogc -D FOO='BAR' -D BAR='BAZ' -D LOL='HEHE' "
        "-D ASD='QWE' -l -e 'foo.txt' -t 'main.tmpl' -o 'foo.html' -i");
    free(rv);

    rv = bm_exec_build_blogc_cmd("blogc", settings, variables, NULL, NULL, false,
        NULL, NULL, NULL, false, false);
    assert_string_equal(rv,
        "LC_ALL='en_US.utf8' blogc -D FOO='BAR' -D BAR='BAZ' -D LOL='HEHE'");
    free(rv);

    rv = bm_exec_build_blogc_cmd("blogc", settings, NULL, NULL, NULL, false,
        NULL, NULL, NULL, false, false);
    assert_string_equal(rv,
        "LC_ALL='en_US.utf8' blogc -D FOO='BAR' -D BAR='BAZ'");
    free(rv);

    sb_trie_free(local);
    sb_trie_free(variables);
    sb_trie_free(settings->settings);
    sb_trie_free(settings->global);
    free(settings);
}


static void
test_build_blogc_cmd_with_settings_and_dev(void **state)
{
    bm_settings_t *settings = sb_malloc(sizeof(bm_settings_t));
    settings->settings = sb_trie_new(free);
    sb_trie_insert(settings->settings, "locale", sb_strdup("en_US.utf8"));
    settings->global = sb_trie_new(free);
    sb_trie_insert(settings->global, "FOO", sb_strdup("BAR"));
    sb_trie_insert(settings->global, "BAR", sb_strdup("BAZ"));
    sb_trie_t *variables = sb_trie_new(free);
    sb_trie_insert(variables, "LOL", sb_strdup("HEHE"));
    sb_trie_t *local = sb_trie_new(free);
    sb_trie_insert(local, "ASD", sb_strdup("QWE"));
    settings->tags = NULL;

    char *rv = bm_exec_build_blogc_cmd("blogc", settings, variables, local, NULL,
        true, NULL, "main.tmpl", "foo.html", true, true);
    assert_string_equal(rv,
        "LC_ALL='en_US.utf8' blogc -D FOO='BAR' -D BAR='BAZ' -D LOL='HEHE' "
        "-D ASD='QWE' -D MAKE_ENV_DEV=1 -D MAKE_ENV='dev' -l -t 'main.tmpl' "
        "-o 'foo.html' -i");
    free(rv);

    rv = bm_exec_build_blogc_cmd("blogc", settings, variables, local, NULL, true,
        "foo.txt", "main.tmpl", "foo.html", true, true);
    assert_string_equal(rv,
        "LC_ALL='en_US.utf8' blogc -D FOO='BAR' -D BAR='BAZ' -D LOL='HEHE' "
        "-D ASD='QWE' -D MAKE_ENV_DEV=1 -D MAKE_ENV='dev' -l -e 'foo.txt' "
        "-t 'main.tmpl' -o 'foo.html' -i");
    free(rv);

    rv = bm_exec_build_blogc_cmd("blogc", settings, variables, NULL, NULL, false,
        NULL, NULL, NULL, true, false);
    assert_string_equal(rv,
        "LC_ALL='en_US.utf8' blogc -D FOO='BAR' -D BAR='BAZ' -D LOL='HEHE' "
        "-D MAKE_ENV_DEV=1 -D MAKE_ENV='dev'");
    free(rv);

    rv = bm_exec_build_blogc_cmd("blogc", settings, NULL, NULL, NULL, false,
        NULL, NULL, NULL, true, false);
    assert_string_equal(rv,
        "LC_ALL='en_US.utf8' blogc -D FOO='BAR' -D BAR='BAZ' "
        "-D MAKE_ENV_DEV=1 -D MAKE_ENV='dev'");
    free(rv);

    sb_trie_free(local);
    sb_trie_free(variables);
    sb_trie_free(settings->settings);
    sb_trie_free(settings->global);
    free(settings);
}


static void
test_build_blogc_cmd_with_settings_and_tags(void **state)
{
    bm_settings_t *settings = sb_malloc(sizeof(bm_settings_t));
    settings->settings = sb_trie_new(free);
    sb_trie_insert(settings->settings, "locale", sb_strdup("en_US.utf8"));
    settings->global = sb_trie_new(free);
    sb_trie_insert(settings->global, "FOO", sb_strdup("BAR"));
    sb_trie_insert(settings->global, "BAR", sb_strdup("BAZ"));
    sb_trie_t *variables = sb_trie_new(free);
    sb_trie_insert(variables, "LOL", sb_strdup("HEHE"));
    sb_trie_t *local = sb_trie_new(free);
    sb_trie_insert(local, "ASD", sb_strdup("QWE"));
    settings->tags = sb_str_split("asd foo bar", ' ', 0);

    char *rv = bm_exec_build_blogc_cmd("blogc", settings, variables, local, NULL,
        true, NULL, "main.tmpl", "foo.html", true, true);
    assert_string_equal(rv,
        "LC_ALL='en_US.utf8' blogc -D MAKE_TAGS='asd foo bar' -D FOO='BAR' "
        "-D BAR='BAZ' -D LOL='HEHE' -D ASD='QWE' -D MAKE_ENV_DEV=1 "
        "-D MAKE_ENV='dev' -l -t 'main.tmpl' -o 'foo.html' -i");
    free(rv);

    rv = bm_exec_build_blogc_cmd("blogc", settings, variables, local, NULL, true,
        "foo.txt", "main.tmpl", "foo.html", true, true);
    assert_string_equal(rv,
        "LC_ALL='en_US.utf8' blogc -D MAKE_TAGS='asd foo bar' -D FOO='BAR' "
        "-D BAR='BAZ' -D LOL='HEHE' -D ASD='QWE' -D MAKE_ENV_DEV=1 "
        "-D MAKE_ENV='dev' -l -e 'foo.txt' -t 'main.tmpl' -o 'foo.html' -i");
    free(rv);

    rv = bm_exec_build_blogc_cmd("blogc", settings, variables, NULL, NULL, false,
        NULL, NULL, NULL, true, false);
    assert_string_equal(rv,
        "LC_ALL='en_US.utf8' blogc -D MAKE_TAGS='asd foo bar' -D FOO='BAR' "
        "-D BAR='BAZ' -D LOL='HEHE' -D MAKE_ENV_DEV=1 -D MAKE_ENV='dev'");
    free(rv);

    rv = bm_exec_build_blogc_cmd("blogc", settings, NULL, NULL, NULL, false,
        NULL, NULL, NULL, true, false);
    assert_string_equal(rv,
        "LC_ALL='en_US.utf8' blogc -D MAKE_TAGS='asd foo bar' -D FOO='BAR' "
        "-D BAR='BAZ' -D MAKE_ENV_DEV=1 -D MAKE_ENV='dev'");
    free(rv);

    sb_trie_free(local);
    sb_trie_free(variables);
    sb_trie_free(settings->settings);
    sb_trie_free(settings->global);
    sb_strv_free(settings->tags);
    free(settings);
}


static void
test_build_blogc_cmd_without_settings(void **state)
{
    sb_trie_t *variables = sb_trie_new(free);
    sb_trie_insert(variables, "LOL", sb_strdup("HEHE"));
    sb_trie_t *local = sb_trie_new(free);
    sb_trie_insert(local, "ASD", sb_strdup("QWE"));

    char *rv = bm_exec_build_blogc_cmd("blogc", NULL, variables, local, NULL,
        true, NULL, "main.tmpl", "foo.html", false, true);
    assert_string_equal(rv,
        "blogc -D LOL='HEHE' -D ASD='QWE' -l -t 'main.tmpl' -o 'foo.html' -i");
    free(rv);

    rv = bm_exec_build_blogc_cmd("blogc", NULL, variables, local, NULL, true,
        "foo.txt", "main.tmpl", "foo.html", false, true);
    assert_string_equal(rv,
        "blogc -D LOL='HEHE' -D ASD='QWE' -l -e 'foo.txt' -t 'main.tmpl' "
        "-o 'foo.html' -i");
    free(rv);

    rv = bm_exec_build_blogc_cmd("blogc", NULL, variables, NULL, NULL, false,
        NULL, NULL, NULL, false, false);
    assert_string_equal(rv,
        "blogc -D LOL='HEHE'");
    free(rv);

    rv = bm_exec_build_blogc_cmd("blogc", NULL, NULL, NULL, NULL, false, NULL,
        NULL, NULL, false, false);
    assert_string_equal(rv,
        "blogc");
    free(rv);

    sb_trie_free(local);
    sb_trie_free(variables);
}


static void
test_build_blogc_cmd_print(void **state)
{
    sb_trie_t *variables = sb_trie_new(free);
    sb_trie_insert(variables, "LOL", sb_strdup("HEHE"));
    sb_trie_t *local = sb_trie_new(free);
    sb_trie_insert(local, "ASD", sb_strdup("QWE"));

    char *rv = bm_exec_build_blogc_cmd("blogc", NULL, variables, local, "LOL",
        false, NULL, NULL, NULL, false, true);
    assert_string_equal(rv, "blogc -D LOL='HEHE' -D ASD='QWE' -p LOL -i");
    free(rv);

    rv = bm_exec_build_blogc_cmd("blogc", NULL, variables, local, "LOL", true,
        NULL, NULL, NULL, false, false);
    assert_string_equal(rv, "blogc -D LOL='HEHE' -D ASD='QWE' -p LOL -l");
    free(rv);

    rv = bm_exec_build_blogc_cmd("blogc", NULL, variables, NULL, "LOL", false,
        NULL, NULL, NULL, false, true);
    assert_string_equal(rv,
        "blogc -D LOL='HEHE' -p LOL -i");
    free(rv);

    rv = bm_exec_build_blogc_cmd("blogc", NULL, NULL, NULL, "LOL", false, NULL,
        NULL, NULL, false, false);
    assert_string_equal(rv,
        "blogc -p LOL");
    free(rv);

    sb_trie_free(local);
    sb_trie_free(variables);
}


int
main(void)
{
    const UnitTest tests[] = {
#ifndef MAKE_EMBEDDED
        unit_test(test_find_binary),
#endif
        unit_test(test_build_blogc_cmd_with_settings),
        unit_test(test_build_blogc_cmd_with_settings_and_dev),
        unit_test(test_build_blogc_cmd_with_settings_and_tags),
        unit_test(test_build_blogc_cmd_without_settings),
        unit_test(test_build_blogc_cmd_print),
    };
    return run_tests(tests);
}
