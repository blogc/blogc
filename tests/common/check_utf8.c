// SPDX-FileCopyrightText: 2014-2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
// SPDX-License-Identifier: BSD-3-Clause

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "../../src/common/utf8.h"
#include "../../src/common/utils.h"

// this file MUST be ASCII


static void
test_utf8_valid(void **state)
{
    const char *c = "<a href=\"{{ BASE_URL }}/page/{{ PREVIOUS_PAGE }}/\">"
        "\xc2\xab Newer posts</a>";
    assert_true(bc_utf8_validate((uint8_t*) c, strlen(c)));
    const uint8_t d[3] = {0xe2, 0x82, 0xac};  // euro sign
    assert_true(bc_utf8_validate(d, 3));
    const uint8_t e[3] = {0xef, 0xbb, 0xbf};  // utf-8 bom
    assert_true(bc_utf8_validate(e, 3));
}


static void
test_utf8_invalid(void **state)
{
    const uint8_t c[4] = {0xff, 0xfe, 0xac, 0x20};  // utf-16
    assert_false(bc_utf8_validate(c, 4));
    const uint8_t d[8] = {0xff, 0xfe, 0x00, 0x00, 0xac, 0x20, 0x00, 0x00};  // utf-32
    assert_false(bc_utf8_validate(d, 8));
    const uint8_t e[6] = {'a', 0xff, 0xfe, 0xac, 0x20, 'b'};  // utf-16
    assert_false(bc_utf8_validate(e, 6));
    const uint8_t f[10] = {'a', 0xff, 0xfe, 0x00, 0x00, 0xac, 0x20, 0x00, 0x00, 'b'};  // utf-32
    assert_false(bc_utf8_validate(f, 10));
}


static void
test_utf8_valid_str(void **state)
{
    bc_string_t *s = bc_string_new();
    bc_string_append(s,
        "<a href=\"{{ BASE_URL }}/page/{{ PREVIOUS_PAGE }}/\">\xc2\xab Newer "
        "posts</a>");
    assert_true(bc_utf8_validate_str(s));
    bc_string_free(s, true);
    s = bc_string_new();
    bc_string_append(s, "\xe2\x82\xac");
    assert_true(bc_utf8_validate_str(s));
    bc_string_free(s, true);
}


static void
test_utf8_invalid_str(void **state)
{
    bc_string_t *s = bc_string_new();
    bc_string_append(s, "\xff\xfe\xac\x20");  // utf-16
    assert_false(bc_utf8_validate_str(s));
    bc_string_free(s, true);
    s = bc_string_new();
    bc_string_append(s, "\xff\xfe\x00\x00\xac\x20\x00\x00");  // utf-32
    assert_false(bc_utf8_validate_str(s));
    bc_string_free(s, true);
}


static void
test_utf8_skip_bom(void **state)
{
    const uint8_t c[4] = {0xef, 0xbb, 0xbf, 0};
    assert_int_equal(bc_utf8_skip_bom(c, 2), 0);
    assert_int_equal(bc_utf8_skip_bom(c, 3), 3);
    assert_string_equal(c + 3, "");
    const uint8_t d[8] = {0xef, 0xbb, 0xbf, 'b', 'o', 'l', 'a', 0};
    assert_int_equal(bc_utf8_skip_bom(d, 7), 3);
    assert_string_equal(d + 3, "bola");
    const uint8_t e[5] = "bola";
    assert_int_equal(bc_utf8_skip_bom(e, 4), 0);
}


int
main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_utf8_valid),
        cmocka_unit_test(test_utf8_invalid),
        cmocka_unit_test(test_utf8_valid_str),
        cmocka_unit_test(test_utf8_invalid_str),
        cmocka_unit_test(test_utf8_skip_bom),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
