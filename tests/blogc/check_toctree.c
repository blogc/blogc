/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2020 Rafael G. Martins <rafael@rafaelmartins.eng.br>
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
#include <stdio.h>
#include "../../src/common/utils.h"
#include "../../src/blogc/toctree.h"


static void
test_toctree_append(void **state)
{
    bc_slist_t *l = NULL;

    l = blogc_toctree_append(l, 0, "", "");
    assert_null(l);

    l = blogc_toctree_append(l, 1, "f1", "bola");
    assert_non_null(l);
    assert_int_equal(((blogc_toctree_header_t*) l->data)->level, 1);
    assert_string_equal(((blogc_toctree_header_t*) l->data)->slug, "f1");
    assert_string_equal(((blogc_toctree_header_t*) l->data)->text, "bola");
    assert_null(l->next);

    l = blogc_toctree_append(l, 2, "f2", "guda");
    assert_non_null(l);
    assert_int_equal(((blogc_toctree_header_t*) l->data)->level, 1);
    assert_string_equal(((blogc_toctree_header_t*) l->data)->slug, "f1");
    assert_string_equal(((blogc_toctree_header_t*) l->data)->text, "bola");
    assert_non_null(l->next);
    assert_int_equal(((blogc_toctree_header_t*) l->next->data)->level, 2);
    assert_string_equal(((blogc_toctree_header_t*) l->next->data)->slug, "f2");
    assert_string_equal(((blogc_toctree_header_t*) l->next->data)->text, "guda");
    assert_null(l->next->next);

    l = blogc_toctree_append(l, 0, "", "");
    assert_non_null(l);
    assert_int_equal(((blogc_toctree_header_t*) l->data)->level, 1);
    assert_string_equal(((blogc_toctree_header_t*) l->data)->slug, "f1");
    assert_string_equal(((blogc_toctree_header_t*) l->data)->text, "bola");
    assert_non_null(l->next);
    assert_int_equal(((blogc_toctree_header_t*) l->next->data)->level, 2);
    assert_string_equal(((blogc_toctree_header_t*) l->next->data)->slug, "f2");
    assert_string_equal(((blogc_toctree_header_t*) l->next->data)->text, "guda");
    assert_null(l->next->next);

    blogc_toctree_free(l);
}


static void
test_toctree_render(void **state)
{
    bc_slist_t *l = NULL;
    assert_null(blogc_toctree_render(l, 6, NULL));

    l = blogc_toctree_append(l, 1, "f1", "bola");
    l = blogc_toctree_append(l, 1, "f2", "guda");
    l = blogc_toctree_append(l, 2, "f3", "asd");
    l = blogc_toctree_append(l, 3, NULL, "foo");
    l = blogc_toctree_append(l, 1, "f5", NULL);
    l = blogc_toctree_append(l, 5, NULL, NULL);
    char *o = blogc_toctree_render(l, -2, NULL);
    assert_string_equal(o,
        "<ul>\n"
        "    <li><a href=\"#f1\">bola</a></li>\n"
        "    <li><a href=\"#f2\">guda</a></li>\n"
        "    <ul>\n"
        "        <li><a href=\"#f3\">asd</a></li>\n"
        "        <ul>\n"
        "            <li>foo</li>\n"
        "        </ul>\n"
        "    </ul>\n"
        "    <li><a href=\"#f5\"></a></li>\n"
        "    <ul>\n"
        "        <ul>\n"
        "            <ul>\n"
        "                <ul>\n"
        "                    <li></li>\n"
        "                </ul>\n"
        "            </ul>\n"
        "        </ul>\n"
        "    </ul>\n"
        "</ul>\n");
    free(o);

    o = blogc_toctree_render(l, -1, "\n");
    assert_string_equal(o,
        "<ul>\n"
        "    <li><a href=\"#f1\">bola</a></li>\n"
        "    <li><a href=\"#f2\">guda</a></li>\n"
        "    <ul>\n"
        "        <li><a href=\"#f3\">asd</a></li>\n"
        "        <ul>\n"
        "            <li>foo</li>\n"
        "        </ul>\n"
        "    </ul>\n"
        "    <li><a href=\"#f5\"></a></li>\n"
        "    <ul>\n"
        "        <ul>\n"
        "            <ul>\n"
        "                <ul>\n"
        "                    <li></li>\n"
        "                </ul>\n"
        "            </ul>\n"
        "        </ul>\n"
        "    </ul>\n"
        "</ul>\n");
    free(o);

    o = blogc_toctree_render(l, 6, "\r\n");
    assert_string_equal(o,
        "<ul>\r\n"
        "    <li><a href=\"#f1\">bola</a></li>\r\n"
        "    <li><a href=\"#f2\">guda</a></li>\r\n"
        "    <ul>\r\n"
        "        <li><a href=\"#f3\">asd</a></li>\r\n"
        "        <ul>\r\n"
        "            <li>foo</li>\r\n"
        "        </ul>\r\n"
        "    </ul>\r\n"
        "    <li><a href=\"#f5\"></a></li>\r\n"
        "    <ul>\r\n"
        "        <ul>\r\n"
        "            <ul>\r\n"
        "                <ul>\r\n"
        "                    <li></li>\r\n"
        "                </ul>\r\n"
        "            </ul>\r\n"
        "        </ul>\r\n"
        "    </ul>\r\n"
        "</ul>\r\n");
    free(o);

    o = blogc_toctree_render(l, 5, NULL);
    assert_string_equal(o,
        "<ul>\n"
        "    <li><a href=\"#f1\">bola</a></li>\n"
        "    <li><a href=\"#f2\">guda</a></li>\n"
        "    <ul>\n"
        "        <li><a href=\"#f3\">asd</a></li>\n"
        "        <ul>\n"
        "            <li>foo</li>\n"
        "        </ul>\n"
        "    </ul>\n"
        "    <li><a href=\"#f5\"></a></li>\n"
        "    <ul>\n"
        "        <ul>\n"
        "            <ul>\n"
        "                <ul>\n"
        "                    <li></li>\n"
        "                </ul>\n"
        "            </ul>\n"
        "        </ul>\n"
        "    </ul>\n"
        "</ul>\n");
    free(o);

    o = blogc_toctree_render(l, 4, "\n");
    assert_string_equal(o,
        "<ul>\n"
        "    <li><a href=\"#f1\">bola</a></li>\n"
        "    <li><a href=\"#f2\">guda</a></li>\n"
        "    <ul>\n"
        "        <li><a href=\"#f3\">asd</a></li>\n"
        "        <ul>\n"
        "            <li>foo</li>\n"
        "        </ul>\n"
        "    </ul>\n"
        "    <li><a href=\"#f5\"></a></li>\n"
        "</ul>\n");
    free(o);

    o = blogc_toctree_render(l, 3, "\r\n");
    assert_string_equal(o,
        "<ul>\r\n"
        "    <li><a href=\"#f1\">bola</a></li>\r\n"
        "    <li><a href=\"#f2\">guda</a></li>\r\n"
        "    <ul>\r\n"
        "        <li><a href=\"#f3\">asd</a></li>\r\n"
        "        <ul>\r\n"
        "            <li>foo</li>\r\n"
        "        </ul>\r\n"
        "    </ul>\r\n"
        "    <li><a href=\"#f5\"></a></li>\r\n"
        "</ul>\r\n");
    free(o);

    o = blogc_toctree_render(l, 2, NULL);
    assert_string_equal(o,
        "<ul>\n"
        "    <li><a href=\"#f1\">bola</a></li>\n"
        "    <li><a href=\"#f2\">guda</a></li>\n"
        "    <ul>\n"
        "        <li><a href=\"#f3\">asd</a></li>\n"
        "    </ul>\n"
        "    <li><a href=\"#f5\"></a></li>\n"
        "</ul>\n");
    free(o);

    o = blogc_toctree_render(l, 1, NULL);
    assert_string_equal(o,
        "<ul>\n"
        "    <li><a href=\"#f1\">bola</a></li>\n"
        "    <li><a href=\"#f2\">guda</a></li>\n"
        "    <li><a href=\"#f5\"></a></li>\n"
        "</ul>\n");
    free(o);

    assert_null(blogc_toctree_render(l, 0, NULL));

    blogc_toctree_free(l);
}


int
main(void)
{
    const UnitTest tests[] = {
        unit_test(test_toctree_append),
        unit_test(test_toctree_render),
    };
    return run_tests(tests);
}
