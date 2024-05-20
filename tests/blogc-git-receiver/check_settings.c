// SPDX-FileCopyrightText: 2014-2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
// SPDX-License-Identifier: BSD-3-Clause

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>
#include <stdlib.h>
#include "../../src/common/config-parser.h"
#include "../../src/common/utils.h"
#include "../../src/blogc-git-receiver/settings.h"


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
test_settings_get_section(void **state)
{
    bc_error_t *err = NULL;

    setenv("HOME", "/home/blogc", 1);

    bc_config_t *config = bc_config_parse("", 0, NULL, &err);
    assert_null(err);
    assert_null(bgr_settings_get_section(config, "/home/blogc/repos/foo.git"));
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
    config = bc_config_parse(conf, strlen(conf), NULL, &err);
    assert_null(err);
    char *s = bgr_settings_get_section(config, "/home/blogc/repos/bar.git");
    assert_string_equal(s, "repo:bar.git");
    free(s);
    bc_config_free(config);

    setenv("BLOGC_GIT_RECEIVER_BASE_DIR", "/home/bola", 1);
    will_return(__wrap_realpath, NULL);
    will_return(__wrap_realpath, "/home/bola/repos/asd/bar.git");
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
    config = bc_config_parse(conf, strlen(conf), NULL, &err);
    assert_null(err);
    s = bgr_settings_get_section(config, "/home/bola/repos/asd/bar.git");
    assert_string_equal(s, "repo:asd/bar.git");
    free(s);
    bc_config_free(config);
}


int
main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_settings_get_section),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
