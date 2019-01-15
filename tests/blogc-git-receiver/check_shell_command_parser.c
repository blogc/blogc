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
#include "../../src/blogc-git-receiver/shell-command-parser.h"


static void
test_shell_command_parse(void **state)
{
    char *t;
    assert_null(bgr_shell_command_parse(""));
    assert_null(bgr_shell_command_parse("bola"));
    assert_null(bgr_shell_command_parse("bola guda"));
    assert_null(bgr_shell_command_parse("bola 'guda'"));
    t = bgr_shell_command_parse("git-receive-pack 'bola.git'");
    assert_string_equal(t, "bola.git");
    free(t);
    t = bgr_shell_command_parse("git-upload-pack 'bolaa.git'");
    assert_string_equal(t, "bolaa.git");
    free(t);
    t = bgr_shell_command_parse("git-upload-archive 'bolab.git'");
    assert_string_equal(t, "bolab.git");
    free(t);
    t = bgr_shell_command_parse("git-receive-pack '/bola1.git'");
    assert_string_equal(t, "bola1.git");
    free(t);
    t = bgr_shell_command_parse("git-receive-pack '/bola2.git/'");
    assert_string_equal(t, "bola2.git/");
    free(t);
    t = bgr_shell_command_parse("git-receive-pack 'bola3.git/'");
    assert_string_equal(t, "bola3.git/");
    free(t);
    t = bgr_shell_command_parse("git-receive-pack 'foo/bola.git'");
    assert_string_equal(t, "foo/bola.git");
    free(t);
    t = bgr_shell_command_parse("git-receive-pack '/foo/bola1.git'");
    assert_string_equal(t, "foo/bola1.git");
    free(t);
    t = bgr_shell_command_parse("git-receive-pack '/foo/bola2.git/'");
    assert_string_equal(t, "foo/bola2.git/");
    free(t);
    t = bgr_shell_command_parse("git-receive-pack 'foo/bola3.git/'");
    assert_string_equal(t, "foo/bola3.git/");
    free(t);

    t = bgr_shell_command_parse("git-receive-pack ''\\''bola.git'");
    assert_string_equal(t, "'bola.git");
    free(t);
    t = bgr_shell_command_parse("git-receive-pack ''\\''/bola1.git'");
    assert_string_equal(t, "'/bola1.git");
    free(t);
    t = bgr_shell_command_parse("git-receive-pack ''\\''/bola2.git/'");
    assert_string_equal(t, "'/bola2.git/");
    free(t);
    t = bgr_shell_command_parse("git-receive-pack ''\\''bola3.git/'");
    assert_string_equal(t, "'bola3.git/");
    free(t);
    t = bgr_shell_command_parse("git-receive-pack ''\\''foo/bola.git'");
    assert_string_equal(t, "'foo/bola.git");
    free(t);
    t = bgr_shell_command_parse("git-receive-pack ''\\''/foo/bola1.git'");
    assert_string_equal(t, "'/foo/bola1.git");
    free(t);
    t = bgr_shell_command_parse("git-receive-pack ''\\''/foo/bola2.git/'");
    assert_string_equal(t, "'/foo/bola2.git/");
    free(t);
    t = bgr_shell_command_parse("git-receive-pack ''\\''foo/bola3.git/'");
    assert_string_equal(t, "'foo/bola3.git/");
    free(t);

    t = bgr_shell_command_parse("git-receive-pack 'bola.git'\\'''");
    assert_string_equal(t, "bola.git'");
    free(t);
    t = bgr_shell_command_parse("git-receive-pack '/bola1.git'\\'''");
    assert_string_equal(t, "bola1.git'");
    free(t);
    t = bgr_shell_command_parse("git-receive-pack '/bola2.git/'\\'''");
    assert_string_equal(t, "bola2.git/'");
    free(t);
    t = bgr_shell_command_parse("git-receive-pack 'bola3.git/'\\'''");
    assert_string_equal(t, "bola3.git/'");
    free(t);
    t = bgr_shell_command_parse("git-receive-pack 'foo/bola.git'\\'''");
    assert_string_equal(t, "foo/bola.git'");
    free(t);
    t = bgr_shell_command_parse("git-receive-pack '/foo/bola1.git'\\'''");
    assert_string_equal(t, "foo/bola1.git'");
    free(t);
    t = bgr_shell_command_parse("git-receive-pack '/foo/bola2.git/'\\'''");
    assert_string_equal(t, "foo/bola2.git/'");
    free(t);
    t = bgr_shell_command_parse("git-receive-pack 'foo/bola3.git/'\\'''");
    assert_string_equal(t, "foo/bola3.git/'");
    free(t);

    t = bgr_shell_command_parse("git-receive-pack 'bo'\\''la.git'");
    assert_string_equal(t, "bo'la.git");
    free(t);
    t = bgr_shell_command_parse("git-receive-pack '/bo'\\''la1.git'");
    assert_string_equal(t, "bo'la1.git");
    free(t);
    t = bgr_shell_command_parse("git-receive-pack '/bo'\\''la2.git/'");
    assert_string_equal(t, "bo'la2.git/");
    free(t);
    t = bgr_shell_command_parse("git-receive-pack 'bo'\\''la3.git/'");
    assert_string_equal(t, "bo'la3.git/");
    free(t);
    t = bgr_shell_command_parse("git-receive-pack 'foo/bo'\\''la.git'");
    assert_string_equal(t, "foo/bo'la.git");
    free(t);
    t = bgr_shell_command_parse("git-receive-pack '/foo/bo'\\''la1.git'");
    assert_string_equal(t, "foo/bo'la1.git");
    free(t);
    t = bgr_shell_command_parse("git-receive-pack '/foo/bo'\\''la2.git/'");
    assert_string_equal(t, "foo/bo'la2.git/");
    free(t);
    t = bgr_shell_command_parse("git-receive-pack 'foo/bo'\\''la3.git/'");
    assert_string_equal(t, "foo/bo'la3.git/");
    free(t);

    t = bgr_shell_command_parse("git-receive-pack ''\\!'bola.git'");
    assert_string_equal(t, "!bola.git");
    free(t);
    t = bgr_shell_command_parse("git-receive-pack ''\\!'/bola1.git'");
    assert_string_equal(t, "!/bola1.git");
    free(t);
    t = bgr_shell_command_parse("git-receive-pack ''\\!'/bola2.git/'");
    assert_string_equal(t, "!/bola2.git/");
    free(t);
    t = bgr_shell_command_parse("git-receive-pack ''\\!'bola3.git/'");
    assert_string_equal(t, "!bola3.git/");
    free(t);
    t = bgr_shell_command_parse("git-receive-pack ''\\!'foo/bola.git'");
    assert_string_equal(t, "!foo/bola.git");
    free(t);
    t = bgr_shell_command_parse("git-receive-pack ''\\!'/foo/bola1.git'");
    assert_string_equal(t, "!/foo/bola1.git");
    free(t);
    t = bgr_shell_command_parse("git-receive-pack ''\\!'/foo/bola2.git/'");
    assert_string_equal(t, "!/foo/bola2.git/");
    free(t);
    t = bgr_shell_command_parse("git-receive-pack ''\\!'foo/bola3.git/'");
    assert_string_equal(t, "!foo/bola3.git/");
    free(t);

    t = bgr_shell_command_parse("git-receive-pack 'bola.git'\\!''");
    assert_string_equal(t, "bola.git!");
    free(t);
    t = bgr_shell_command_parse("git-receive-pack '/bola1.git'\\!''");
    assert_string_equal(t, "bola1.git!");
    free(t);
    t = bgr_shell_command_parse("git-receive-pack '/bola2.git/'\\!''");
    assert_string_equal(t, "bola2.git/!");
    free(t);
    t = bgr_shell_command_parse("git-receive-pack 'bola3.git/'\\!''");
    assert_string_equal(t, "bola3.git/!");
    free(t);
    t = bgr_shell_command_parse("git-receive-pack 'foo/bola.git'\\!''");
    assert_string_equal(t, "foo/bola.git!");
    free(t);
    t = bgr_shell_command_parse("git-receive-pack '/foo/bola1.git'\\!''");
    assert_string_equal(t, "foo/bola1.git!");
    free(t);
    t = bgr_shell_command_parse("git-receive-pack '/foo/bola2.git/'\\!''");
    assert_string_equal(t, "foo/bola2.git/!");
    free(t);
    t = bgr_shell_command_parse("git-receive-pack 'foo/bola3.git/'\\!''");
    assert_string_equal(t, "foo/bola3.git/!");
    free(t);

    t = bgr_shell_command_parse("git-receive-pack 'bo'\\!'la.git'");
    assert_string_equal(t, "bo!la.git");
    free(t);
    t = bgr_shell_command_parse("git-receive-pack '/bo'\\!'la1.git'");
    assert_string_equal(t, "bo!la1.git");
    free(t);
    t = bgr_shell_command_parse("git-receive-pack '/bo'\\!'la2.git/'");
    assert_string_equal(t, "bo!la2.git/");
    free(t);
    t = bgr_shell_command_parse("git-receive-pack 'bo'\\!'la3.git/'");
    assert_string_equal(t, "bo!la3.git/");
    free(t);
    t = bgr_shell_command_parse("git-receive-pack 'foo/bo'\\!'la.git'");
    assert_string_equal(t, "foo/bo!la.git");
    free(t);
    t = bgr_shell_command_parse("git-receive-pack '/foo/bo'\\!'la1.git'");
    assert_string_equal(t, "foo/bo!la1.git");
    free(t);
    t = bgr_shell_command_parse("git-receive-pack '/foo/bo'\\!'la2.git/'");
    assert_string_equal(t, "foo/bo!la2.git/");
    free(t);
    t = bgr_shell_command_parse("git-receive-pack 'foo/bo'\\!'la3.git/'");
    assert_string_equal(t, "foo/bo!la3.git/");
    free(t);
}


int
main(void)
{
    const UnitTest tests[] = {
        unit_test(test_shell_command_parse),
    };
    return run_tests(tests);
}
