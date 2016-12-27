/*
 * blogc: A blog compiler.
 * Copyright (C) 2016 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "../common/error.h"
#include "../common/utils.h"
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
bm_atom_deploy(bm_settings_t *settings, bc_error_t **err)
{
    if (err == NULL || *err != NULL)
        return NULL;

    // this is not really portable
    char fname[] = "/tmp/blogc-make_XXXXXX";
    int fd;
    if (-1 == (fd = mkstemp(fname))) {
        *err = bc_error_new_printf(BLOGC_MAKE_ERROR_ATOM,
            "Failed to create temporary atom template: %s", strerror(errno));
        return NULL;
    }

    const char *atom_prefix = bc_trie_lookup(settings->settings, "atom_prefix");
    const char *atom_ext = bc_trie_lookup(settings->settings, "atom_ext");
    const char *post_prefix = bc_trie_lookup(settings->settings, "post_prefix");

    char *content = bc_strdup_printf(atom_template, atom_prefix, atom_ext,
        atom_prefix, atom_ext, post_prefix, post_prefix);

    if (-1 == write(fd, content, strlen(content))) {
        *err = bc_error_new_printf(BLOGC_MAKE_ERROR_ATOM,
            "Failed to write to temporary atom template: %s", strerror(errno));
        free(content);
        close(fd);
        unlink(fname);
        return NULL;
    }

    free(content);
    close(fd);

    return bc_strdup(fname);
}


void
bm_atom_destroy(const char *fname)
{
    unlink(fname);
}
