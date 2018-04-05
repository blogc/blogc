/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2017 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <squareball.h>

#include "error.h"
#include "settings.h"
#include "atom.h"

static const char atom_template[] =
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
    "<feed xmlns=\"http://www.w3.org/2005/Atom\">\n"
    "  <title type=\"text\">{{ SITE_TITLE }}{%% ifdef FILTER_TAG %%} - "
        "{{ FILTER_TAG }}{%% endif %%}</title>\n"
    "  <id>{{ BASE_URL }}/%s{%% ifdef FILTER_TAG %%}/{{ FILTER_TAG }}"
        "{%% endif %%}%s</id>\n"
    "  <updated>{{ DATE_FIRST_FORMATTED }}</updated>\n"
    "  <link href=\"{{ BASE_DOMAIN }}{{ BASE_URL }}/\" />\n"
    "  <link href=\"{{ BASE_DOMAIN }}{{ BASE_URL }}/%s{%% ifdef FILTER_TAG %%}"
        "/{{ FILTER_TAG }}{%% endif %%}%s\" rel=\"self\" />\n"
    "  <author>\n"
    "    <name>{{ AUTHOR_NAME }}</name>\n"
    "    <email>{{ AUTHOR_EMAIL }}</email>\n"
    "  </author>\n"
    "  <subtitle type=\"text\">{{ SITE_TAGLINE }}</subtitle>\n"
    "  {%% block listing %%}\n"
    "  <entry>\n"
    "    <title type=\"text\">{{ TITLE }}</title>\n"
    "    <id>{{ BASE_URL }}/%s/{{ FILENAME }}/</id>\n"
    "    <updated>{{ DATE_FORMATTED }}</updated>\n"
    "    <published>{{ DATE_FORMATTED }}</published>\n"
    "    <link href=\"{{ BASE_DOMAIN }}{{ BASE_URL }}/%s/{{ FILENAME }}/\" />\n"
    "    <author>\n"
    "      <name>{{ AUTHOR_NAME }}</name>\n"
    "      <email>{{ AUTHOR_EMAIL }}</email>\n"
    "    </author>\n"
    "    <content type=\"html\"><![CDATA[{{ CONTENT }}]]></content>\n"
    "  </entry>\n"
    "  {%% endblock %%}\n"
    "</feed>\n";


char*
bm_atom_deploy(bm_settings_t *settings, sb_error_t **err)
{
    if (err == NULL || *err != NULL)
        return NULL;

    // this is not really portable
    char fname[] = "/tmp/blogc-make_XXXXXX";
    int fd;
    if (-1 == (fd = mkstemp(fname))) {
        *err = sb_error_new_printf(BLOGC_MAKE_ERROR_ATOM,
            "Failed to create temporary atom template: %s", strerror(errno));
        return NULL;
    }

    const char *atom_prefix = sb_trie_lookup(settings->settings, "atom_prefix");
    const char *atom_ext = sb_trie_lookup(settings->settings, "atom_ext");
    const char *post_prefix = sb_trie_lookup(settings->settings, "post_prefix");

    char *content = sb_strdup_printf(atom_template, atom_prefix, atom_ext,
        atom_prefix, atom_ext, post_prefix, post_prefix);

    if (-1 == write(fd, content, strlen(content))) {
        *err = sb_error_new_printf(BLOGC_MAKE_ERROR_ATOM,
            "Failed to write to temporary atom template: %s", strerror(errno));
        free(content);
        close(fd);
        unlink(fname);
        return NULL;
    }

    free(content);
    close(fd);

    return sb_strdup(fname);
}


void
bm_atom_destroy(const char *fname)
{
    unlink(fname);
}
