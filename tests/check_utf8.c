/*
 * blogc: A blog compiler.
 * Copyright (C) 2015-2016 Rafael G. Martins <rafael@rafaelmartins.eng.br>
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
#include <string.h>
#include "../src/utf8.h"
#include "../src/utils.h"


static void
test_utf8_valid(void **state)
{
    const char *c = "<a href=\"{{ BASE_URL }}/page/{{ PREVIOUS_PAGE }}/\">"
        "\xc2\xab Newer posts</a>";
    assert_true(blogc_utf8_validate((uint8_t*) c, strlen(c)));
    const uint8_t d[3] = {0xe2, 0x82, 0xac};
    assert_true(blogc_utf8_validate(d, 3));
}


static void
test_utf8_invalid(void **state)
{
    const uint8_t c[4] = {0xff, 0xfe, 0xac, 0x20};  // utf-16
    assert_false(blogc_utf8_validate(c, 4));
    const uint8_t d[8] = {0xff, 0xfe, 0x00, 0x00, 0xac, 0x20, 0x00, 0x00};  // utf-32
    assert_false(blogc_utf8_validate(d, 8));
}


static void
test_utf8_valid_str(void **state)
{
    sb_string_t *s = sb_string_new();
    sb_string_append(s,
        "<a href=\"{{ BASE_URL }}/page/{{ PREVIOUS_PAGE }}/\">\xc2\xab Newer "
        "posts</a>");
    assert_true(blogc_utf8_validate_str(s));
    sb_string_free(s, true);
    s = sb_string_new();
    sb_string_append(s, "\xe2\x82\xac");
    assert_true(blogc_utf8_validate_str(s));
    sb_string_free(s, true);
}


static void
test_utf8_invalid_str(void **state)
{
    sb_string_t *s = sb_string_new();
    sb_string_append(s, "\xff\xfe\xac\x20");  // utf-16
    assert_false(blogc_utf8_validate_str(s));
    sb_string_free(s, true);
    s = sb_string_new();
    sb_string_append(s, "\xff\xfe\x00\x00\xac\x20\x00\x00");  // utf-32
    assert_false(blogc_utf8_validate_str(s));
    sb_string_free(s, true);
}


int
main(void)
{
    const UnitTest tests[] = {
        unit_test(test_utf8_valid),
        unit_test(test_utf8_invalid),
        unit_test(test_utf8_valid_str),
        unit_test(test_utf8_invalid_str),
    };
    return run_tests(tests);
}
