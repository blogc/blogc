// SPDX-FileCopyrightText: 2014-2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
// SPDX-License-Identifier: BSD-3-Clause

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/common/utils.h"
#include "../../src/blogc-runserver/httpd-utils.h"


ssize_t
__wrap_read(int fd, void *buf, size_t count)
{
    assert_int_equal(fd, mock_type(int));
    const char *mock_buf = mock_type(const char*);
    strcpy(buf, mock_buf);
    assert_int_equal(count, READLINE_BUFFER_SIZE);
    return strlen(mock_buf) > 0 ? strlen(mock_buf) : -1;
}


static void
test_readline(void **state)
{
    char *t;
    will_return(__wrap_read, 1234);
    will_return(__wrap_read, "bola");
    t = br_readline(1234);
    assert_string_equal(t, "bola");
    free(t);
    will_return(__wrap_read, 1234);
    will_return(__wrap_read, "bola1\nguda\nxd");
    t = br_readline(1234);
    assert_string_equal(t, "bola1");
    free(t);
    will_return(__wrap_read, 1234);
    will_return(__wrap_read, "bola2\rguda\rxd");
    t = br_readline(1234);
    assert_string_equal(t, "bola2");
    free(t);
    will_return(__wrap_read, 1234);
    will_return(__wrap_read, "bola3\r\nguda\r\nxd");
    t = br_readline(1234);
    assert_string_equal(t, "bola3");
    free(t);
}


static void
test_hextoi(void **state)
{
    assert_int_equal(br_hextoi('0'), 0);
    assert_int_equal(br_hextoi('1'), 1);
    assert_int_equal(br_hextoi('2'), 2);
    assert_int_equal(br_hextoi('3'), 3);
    assert_int_equal(br_hextoi('4'), 4);
    assert_int_equal(br_hextoi('5'), 5);
    assert_int_equal(br_hextoi('6'), 6);
    assert_int_equal(br_hextoi('7'), 7);
    assert_int_equal(br_hextoi('8'), 8);
    assert_int_equal(br_hextoi('9'), 9);
    assert_int_equal(br_hextoi('a'), 10);
    assert_int_equal(br_hextoi('b'), 11);
    assert_int_equal(br_hextoi('c'), 12);
    assert_int_equal(br_hextoi('d'), 13);
    assert_int_equal(br_hextoi('e'), 14);
    assert_int_equal(br_hextoi('f'), 15);
    assert_int_equal(br_hextoi('A'), 10);
    assert_int_equal(br_hextoi('B'), 11);
    assert_int_equal(br_hextoi('C'), 12);
    assert_int_equal(br_hextoi('D'), 13);
    assert_int_equal(br_hextoi('E'), 14);
    assert_int_equal(br_hextoi('F'), 15);
    assert_int_equal(br_hextoi('g'), -1);
    assert_int_equal(br_hextoi('G'), -1);
    assert_int_equal(br_hextoi('-'), -1);
}


static void
test_urldecode(void **state)
{
    for (size_t i = 0; i < 128; i++) {
        char *t = bc_strdup_printf("%%%02x", i);
        char *r = br_urldecode(t);
        assert_int_equal(r[0], i);
        assert_int_equal(r[1], 0);
        free(r);
        free(t);
    }
    char *r = br_urldecode("%Ab");
    assert_string_equal(r, "\xab");
    free(r);
    r = br_urldecode("%xb");
    assert_string_equal(r, "%xb");
    free(r);
    r = br_urldecode("%C3%BC");
    assert_string_equal(r, "\xc3\xbc");
    free(r);
}


static void
test_get_extension(void **state)
{
    assert_null(br_get_extension("bola"));
    assert_string_equal(br_get_extension("bola.txt"), "txt");
    assert_string_equal(br_get_extension("bola.txt.jpg"), "jpg");
    assert_null(br_get_extension("bola.txt/foo"));
    assert_string_equal(br_get_extension("bola.txt/foo.jpg"), "jpg");
}


int
main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_readline),
        cmocka_unit_test(test_hextoi),
        cmocka_unit_test(test_urldecode),
        cmocka_unit_test(test_get_extension),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
