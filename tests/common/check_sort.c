// SPDX-FileCopyrightText: 2014-2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
// SPDX-License-Identifier: BSD-3-Clause

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/common/utils.h"
#include "../../src/common/sort.h"


static int
sort_func(void *a, void *b)
{
    return strcmp((char*) a, (char*) b);
}


static void
test_slist_sort_empty(void **state)
{
    bc_slist_t *l = NULL;
    assert_null(bc_slist_sort(l, (bc_sort_func_t) sort_func));
}


static void
test_slist_sort_single(void **state)
{
    bc_slist_t *l = NULL;
    l = bc_slist_append(l, bc_strdup("a"));

    l = bc_slist_sort(l, (bc_sort_func_t) sort_func);

    assert_non_null(l);
    assert_string_equal(l->data, "a");
    assert_null(l->next);

    bc_slist_free_full(l, free);
}


static void
test_slist_sort_sorted(void **state)
{
    bc_slist_t *l = NULL;
    l = bc_slist_append(l, bc_strdup("a"));
    l = bc_slist_append(l, bc_strdup("b"));
    l = bc_slist_append(l, bc_strdup("c"));

    l = bc_slist_sort(l, (bc_sort_func_t) sort_func);

    assert_non_null(l);
    assert_string_equal(l->data, "a");
    assert_string_equal(l->next->data, "b");
    assert_string_equal(l->next->next->data, "c");
    assert_null(l->next->next->next);

    bc_slist_free_full(l, free);
}


static void
test_slist_sort_reverse(void **state)
{
    bc_slist_t *l = NULL;
    l = bc_slist_append(l, bc_strdup("d"));
    l = bc_slist_append(l, bc_strdup("c"));
    l = bc_slist_append(l, bc_strdup("b"));
    l = bc_slist_append(l, bc_strdup("a"));

    l = bc_slist_sort(l, (bc_sort_func_t) sort_func);

    assert_non_null(l);
    assert_string_equal(l->data, "a");
    assert_string_equal(l->next->data, "b");
    assert_string_equal(l->next->next->data, "c");
    assert_string_equal(l->next->next->next->data, "d");
    assert_null(l->next->next->next->next);

    bc_slist_free_full(l, free);
}


static void
test_slist_sort_mixed1(void **state)
{
    bc_slist_t *l = NULL;
    l = bc_slist_append(l, bc_strdup("a"));
    l = bc_slist_append(l, bc_strdup("d"));
    l = bc_slist_append(l, bc_strdup("c"));
    l = bc_slist_append(l, bc_strdup("b"));

    l = bc_slist_sort(l, (bc_sort_func_t) sort_func);

    assert_non_null(l);
    assert_string_equal(l->data, "a");
    assert_string_equal(l->next->data, "b");
    assert_string_equal(l->next->next->data, "c");
    assert_string_equal(l->next->next->next->data, "d");
    assert_null(l->next->next->next->next);

    bc_slist_free_full(l, free);
}


static void
test_slist_sort_mixed2(void **state)
{
    bc_slist_t *l = NULL;
    l = bc_slist_append(l, bc_strdup("c"));
    l = bc_slist_append(l, bc_strdup("b"));
    l = bc_slist_append(l, bc_strdup("a"));
    l = bc_slist_append(l, bc_strdup("d"));

    l = bc_slist_sort(l, (bc_sort_func_t) sort_func);

    assert_non_null(l);
    assert_string_equal(l->data, "a");
    assert_string_equal(l->next->data, "b");
    assert_string_equal(l->next->next->data, "c");
    assert_string_equal(l->next->next->next->data, "d");
    assert_null(l->next->next->next->next);

    bc_slist_free_full(l, free);
}


int
main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_slist_sort_empty),
        cmocka_unit_test(test_slist_sort_single),
        cmocka_unit_test(test_slist_sort_sorted),
        cmocka_unit_test(test_slist_sort_reverse),
        cmocka_unit_test(test_slist_sort_mixed1),
        cmocka_unit_test(test_slist_sort_mixed2),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
