/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2017 Rafael G. Martins <rafael@rafaelmartins.eng.br>
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
#include <squareball.h>

#include "../../src/blogc-make/atom.h"
#include "../../src/blogc-make/settings.h"


static void
test_atom_file(void **state)
{
    bm_settings_t *settings = sb_malloc(sizeof(bm_settings_t));
    settings->settings = sb_trie_new(free);
    sb_trie_insert(settings->settings, "atom_prefix", sb_strdup("atom"));
    sb_trie_insert(settings->settings, "atom_ext", sb_strdup(".xml"));
    sb_trie_insert(settings->settings, "post_prefix", sb_strdup("post"));

    sb_error_t *err = NULL;
    char *rv = bm_atom_deploy(settings, &err);

    assert_non_null(rv);
    assert_null(err);

    size_t cmp_len;
    char *cmp = sb_file_get_contents_utf8(rv, &cmp_len, &err);

    assert_non_null(cmp);
    assert_null(err);

    assert_string_equal(cmp,
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
        "<feed xmlns=\"http://www.w3.org/2005/Atom\">\n"
        "  <title type=\"text\">{{ SITE_TITLE }}{% ifdef FILTER_TAG %} - "
            "{{ FILTER_TAG }}{% endif %}</title>\n"
        "  <id>{{ BASE_URL }}/atom{% ifdef FILTER_TAG %}/{{ FILTER_TAG }}"
        "{% endif %}.xml</id>\n"
        "  <updated>{{ DATE_FIRST_FORMATTED }}</updated>\n"
        "  <link href=\"{{ BASE_DOMAIN }}{{ BASE_URL }}/\" />\n"
        "  <link href=\"{{ BASE_DOMAIN }}{{ BASE_URL }}/atom{% ifdef FILTER_TAG %}"
            "/{{ FILTER_TAG }}{% endif %}.xml\" rel=\"self\" />\n"
        "  <author>\n"
        "    <name>{{ AUTHOR_NAME }}</name>\n"
        "    <email>{{ AUTHOR_EMAIL }}</email>\n"
        "  </author>\n"
        "  <subtitle type=\"text\">{{ SITE_TAGLINE }}</subtitle>\n"
        "  {% block listing %}\n"
        "  <entry>\n"
        "    <title type=\"text\">{{ TITLE }}</title>\n"
        "    <id>{{ BASE_URL }}/post/{{ FILENAME }}/</id>\n"
        "    <updated>{{ DATE_FORMATTED }}</updated>\n"
        "    <published>{{ DATE_FORMATTED }}</published>\n"
        "    <link href=\"{{ BASE_DOMAIN }}{{ BASE_URL }}/post/{{ FILENAME }}/\" />\n"
        "    <author>\n"
        "      <name>{{ AUTHOR_NAME }}</name>\n"
        "      <email>{{ AUTHOR_EMAIL }}</email>\n"
        "    </author>\n"
        "    <content type=\"html\"><![CDATA[{{ CONTENT }}]]></content>\n"
        "  </entry>\n"
        "  {% endblock %}\n"
        "</feed>\n");

    free(cmp);
    bm_atom_destroy(rv);
    free(rv);
    sb_trie_free(settings->settings);
    free(settings);
}


static void
test_atom_dir(void **state)
{
    bm_settings_t *settings = sb_malloc(sizeof(bm_settings_t));
    settings->settings = sb_trie_new(free);
    sb_trie_insert(settings->settings, "atom_prefix", sb_strdup("atom"));
    sb_trie_insert(settings->settings, "atom_ext", sb_strdup("/index.xml"));
    sb_trie_insert(settings->settings, "post_prefix", sb_strdup("post"));

    sb_error_t *err = NULL;
    char *rv = bm_atom_deploy(settings, &err);

    assert_non_null(rv);
    assert_null(err);

    size_t cmp_len;
    char *cmp = sb_file_get_contents_utf8(rv, &cmp_len, &err);

    assert_non_null(cmp);
    assert_null(err);

    assert_string_equal(cmp,
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
        "<feed xmlns=\"http://www.w3.org/2005/Atom\">\n"
        "  <title type=\"text\">{{ SITE_TITLE }}{% ifdef FILTER_TAG %} - "
            "{{ FILTER_TAG }}{% endif %}</title>\n"
        "  <id>{{ BASE_URL }}/atom{% ifdef FILTER_TAG %}/{{ FILTER_TAG }}"
        "{% endif %}/index.xml</id>\n"
        "  <updated>{{ DATE_FIRST_FORMATTED }}</updated>\n"
        "  <link href=\"{{ BASE_DOMAIN }}{{ BASE_URL }}/\" />\n"
        "  <link href=\"{{ BASE_DOMAIN }}{{ BASE_URL }}/atom{% ifdef FILTER_TAG %}"
            "/{{ FILTER_TAG }}{% endif %}/index.xml\" rel=\"self\" />\n"
        "  <author>\n"
        "    <name>{{ AUTHOR_NAME }}</name>\n"
        "    <email>{{ AUTHOR_EMAIL }}</email>\n"
        "  </author>\n"
        "  <subtitle type=\"text\">{{ SITE_TAGLINE }}</subtitle>\n"
        "  {% block listing %}\n"
        "  <entry>\n"
        "    <title type=\"text\">{{ TITLE }}</title>\n"
        "    <id>{{ BASE_URL }}/post/{{ FILENAME }}/</id>\n"
        "    <updated>{{ DATE_FORMATTED }}</updated>\n"
        "    <published>{{ DATE_FORMATTED }}</published>\n"
        "    <link href=\"{{ BASE_DOMAIN }}{{ BASE_URL }}/post/{{ FILENAME }}/\" />\n"
        "    <author>\n"
        "      <name>{{ AUTHOR_NAME }}</name>\n"
        "      <email>{{ AUTHOR_EMAIL }}</email>\n"
        "    </author>\n"
        "    <content type=\"html\"><![CDATA[{{ CONTENT }}]]></content>\n"
        "  </entry>\n"
        "  {% endblock %}\n"
        "</feed>\n");

    free(cmp);
    bm_atom_destroy(rv);
    free(rv);
    sb_trie_free(settings->settings);
    free(settings);
}


int
main(void)
{
    const UnitTest tests[] = {
        unit_test(test_atom_file),
        unit_test(test_atom_dir),
    };
    return run_tests(tests);
}
