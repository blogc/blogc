// SPDX-FileCopyrightText: 2014-2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
// SPDX-License-Identifier: BSD-3-Clause

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>
#include <stdlib.h>
#include "../../src/common/utils.h"
#include "../../src/blogc/filelist-parser.h"


static void
test_filelist_parse_empty(void **state)
{
    bc_slist_t *l = blogc_filelist_parse("", 0);
    assert_null(l);
}


static void
test_filelist_parse(void **state)
{
    const char *a =
        "content/post/post1.txt";
    bc_slist_t *l = blogc_filelist_parse(a, strlen(a));
    assert_non_null(l);
    assert_string_equal(l->data, "content/post/post1.txt");
    assert_null(l->next);
    bc_slist_free_full(l, free);
    a =
        "content/post/post1.txt\n";
    l = blogc_filelist_parse(a, strlen(a));
    assert_non_null(l);
    assert_string_equal(l->data, "content/post/post1.txt");
    assert_null(l->next);
    bc_slist_free_full(l, free);
    a =
        "content/post/post1.txt\n"
        "content/post/post2.txt\n";
    l = blogc_filelist_parse(a, strlen(a));
    assert_non_null(l);
    assert_string_equal(l->data, "content/post/post1.txt");
    assert_string_equal(l->next->data, "content/post/post2.txt");
    assert_null(l->next->next);
    bc_slist_free_full(l, free);
    a =
        "content/post/post1.txt\n"
        "content/post/post2.txt\n"
        "content/post/post3.txt";
    l = blogc_filelist_parse(a, strlen(a));
    assert_non_null(l);
    assert_string_equal(l->data, "content/post/post1.txt");
    assert_string_equal(l->next->data, "content/post/post2.txt");
    assert_string_equal(l->next->next->data, "content/post/post3.txt");
    assert_null(l->next->next->next);
    bc_slist_free_full(l, free);
    a =
        "   content/post/post1.txt\n"
        "content/post/post2.txt\t\n"
        "\tcontent/post/post3.txt";
    l = blogc_filelist_parse(a, strlen(a));
    assert_non_null(l);
    assert_string_equal(l->data, "content/post/post1.txt");
    assert_string_equal(l->next->data, "content/post/post2.txt");
    assert_string_equal(l->next->next->data, "content/post/post3.txt");
    assert_null(l->next->next->next);
    bc_slist_free_full(l, free);
}


static void
test_filelist_parse_crlf(void **state)
{
    const char *a =
        "content/post/post1.txt\r\n";
    bc_slist_t *l = blogc_filelist_parse(a, strlen(a));
    assert_non_null(l);
    assert_string_equal(l->data, "content/post/post1.txt");
    assert_null(l->next);
    bc_slist_free_full(l, free);
    a =
        "content/post/post1.txt\r\n"
        "content/post/post2.txt\r\n";
    l = blogc_filelist_parse(a, strlen(a));
    assert_non_null(l);
    assert_string_equal(l->data, "content/post/post1.txt");
    assert_string_equal(l->next->data, "content/post/post2.txt");
    assert_null(l->next->next);
    bc_slist_free_full(l, free);
    a =
        "content/post/post1.txt\r\n"
        "content/post/post2.txt\r\n"
        "content/post/post3.txt";
    l = blogc_filelist_parse(a, strlen(a));
    assert_non_null(l);
    assert_string_equal(l->data, "content/post/post1.txt");
    assert_string_equal(l->next->data, "content/post/post2.txt");
    assert_string_equal(l->next->next->data, "content/post/post3.txt");
    assert_null(l->next->next->next);
    bc_slist_free_full(l, free);
    a =
        "   content/post/post1.txt\r\n"
        "content/post/post2.txt\t\r\n"
        "\tcontent/post/post3.txt";
    l = blogc_filelist_parse(a, strlen(a));
    assert_non_null(l);
    assert_string_equal(l->data, "content/post/post1.txt");
    assert_string_equal(l->next->data, "content/post/post2.txt");
    assert_string_equal(l->next->next->data, "content/post/post3.txt");
    assert_null(l->next->next->next);
    bc_slist_free_full(l, free);
}


int
main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_filelist_parse_empty),
        cmocka_unit_test(test_filelist_parse),
        cmocka_unit_test(test_filelist_parse_crlf),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
