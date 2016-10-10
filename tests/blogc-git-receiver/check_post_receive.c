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
#include <string.h>
#include <stdlib.h>
#include "../../src/common/config-parser.h"
#include "../../src/common/utils.h"
#include "../../src/blogc-git-receiver/post-receive.h"


char*
__wrap_realpath(const char *path, char *resolved_path)
{
    const char *real_path = mock_type(const char*);
    if (real_path == NULL)
        return NULL;
    assert_string_equal(path, real_path);
    return bc_strdup(real_path);
}


static void
test_post_receive_get_config_section(void **state)
{
    bc_config_t *config = bc_config_parse("", 0, NULL);
    assert_null(bgr_post_receive_get_config_section(config,
        "/home/blogc/repos/foo.git", "/home/blogc"));
    bc_config_free(config);

    will_return(__wrap_realpath, NULL);
    will_return(__wrap_realpath, "/home/blogc/repos/bar.git");
    const char *conf =
        "[repo:foo.git]\n"
        "mirror = foo\n"
        "\n"
        "[repo:bar.git]\n"
        "mirror = bar\n"
        "\n"
        "[repo:baz.git]\n"
        "mirror = baz\n"
        "\n";
    config = bc_config_parse(conf, strlen(conf), NULL);
    char *s = bgr_post_receive_get_config_section(config,
        "/home/blogc/repos/bar.git", "/home/blogc");
    assert_string_equal(s, "repo:bar.git");
    free(s);
    bc_config_free(config);

    will_return(__wrap_realpath, NULL);
    will_return(__wrap_realpath, "/home/blogc/repos/asd/bar.git");
    conf =
        "[repo:asd/foo.git]\n"
        "mirror = foo\n"
        "\n"
        "[repo:asd/bar.git]\n"
        "mirror = bar\n"
        "\n"
        "[repo:asd/baz.git]\n"
        "mirror = baz\n"
        "\n";
    config = bc_config_parse(conf, strlen(conf), NULL);
    s = bgr_post_receive_get_config_section(config,
        "/home/blogc/repos/asd/bar.git", "/home/blogc");
    assert_string_equal(s, "repo:asd/bar.git");
    free(s);
    bc_config_free(config);
}


int
main(void)
{
    const UnitTest tests[] = {
        unit_test(test_post_receive_get_config_section),
    };
    return run_tests(tests);
}
