/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2019 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <squareball.h>

#include "settings.h"
#include "utils.h"
#include "atom.h"

static const char atom_template[] =
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
    "<feed xmlns=\"http://www.w3.org/2005/Atom\">\n"
    "  <title type=\"text\">{{ SITE_TITLE }}{%% ifdef FILTER_TAG %%} - "
        "{{ FILTER_TAG }}{%% endif %%}</title>\n"
    "  <id>{{ BASE_DOMAIN }}{{ BASE_URL }}%s</id>\n"
    "  <updated>{{ DATE_FIRST_FORMATTED }}</updated>\n"
    "  <link href=\"{{ BASE_DOMAIN }}{{ BASE_URL }}/\" />\n"
    "  <link href=\"{{ BASE_DOMAIN }}{{ BASE_URL }}%s\" rel=\"self\" />\n"
    "  <author>\n"
    "    <name>{{ AUTHOR_NAME }}</name>\n"
    "    <email>{{ AUTHOR_EMAIL }}</email>\n"
    "  </author>\n"
    "  <subtitle type=\"text\">{{ SITE_TAGLINE }}</subtitle>\n"
    "  {%% block listing %%}\n"
    "  <entry>\n"
    "    <title type=\"text\">{{ TITLE }}</title>\n"
    "    <id>{{ BASE_DOMAIN }}{{ BASE_URL }}%s</id>\n"
    "    <updated>{{ DATE_FORMATTED }}</updated>\n"
    "    <published>{{ DATE_FORMATTED }}</published>\n"
    "    <link href=\"{{ BASE_DOMAIN }}{{ BASE_URL }}%s\" />\n"
    "    <author>\n"
    "      <name>{{ AUTHOR_NAME }}</name>\n"
    "      <email>{{ AUTHOR_EMAIL }}</email>\n"
    "    </author>\n"
    "    <content type=\"html\"><![CDATA[{{ CONTENT }}]]></content>\n"
    "  </entry>\n"
    "  {%% endblock %%}\n"
    "</feed>\n";


char*
bm_atom_generate(bm_settings_t *settings)
{
    if (settings == NULL)
        return NULL;

    const char *atom_prefix = sb_trie_lookup(settings->settings, "atom_prefix");
    const char *atom_ext = sb_trie_lookup(settings->settings, "atom_ext");
    const char *post_prefix = sb_trie_lookup(settings->settings, "post_prefix");
    const char *post_ext = sb_trie_lookup(settings->settings, "html_ext");

    sb_string_t *atom_url = sb_string_new();

    if (atom_prefix[0] != '\0')
        sb_string_append_c(atom_url, '/');

    sb_string_append(atom_url, atom_prefix);
    sb_string_append(atom_url, "{% ifdef FILTER_TAG %}/{{ FILTER_TAG }}");

    if (atom_prefix[0] == '\0' && atom_ext[0] != '/')
        sb_string_append(atom_url, "{% else %}/index");

    sb_string_append(atom_url, "{% endif %}");
    sb_string_append(atom_url, atom_ext);

    char *post_url = bm_generate_filename(NULL, post_prefix, "{{ FILENAME }}",
        post_ext);

    char *rv = sb_strdup_printf(atom_template, atom_url->str, atom_url->str,
        post_url, post_url);

    sb_string_free(atom_url, true);
    free(post_url);

    return rv;
}


char*
bm_atom_deploy(bm_settings_t *settings, sb_error_t **err)
{
    if (settings == NULL || err == NULL || *err != NULL)
        return NULL;

    if (NULL != sb_trie_lookup(settings->settings, "atom_legacy_entry_id")) {
        *err = sb_strerror_new(
            "atom: 'atom_legacy_entry_id' setting is not supported anymore. see "
            "https://blogc.rgm.io/news/blogc-0.16.1/ for details");
        return NULL;
    }

    // this is not really portable
    char fname[] = "/tmp/blogc-make_XXXXXX";
    int fd;
    if (-1 == (fd = mkstemp(fname))) {
        *err = sb_strerror_new_printf(
            "atom: Failed to create temporary atom template: %s", strerror(errno));
        return NULL;
    }

    char *content = bm_atom_generate(settings);
    if (content == NULL) {
        close(fd);
        unlink(fname);
        return NULL;
    }

    if (-1 == write(fd, content, strlen(content))) {
        *err = sb_strerror_new_printf(
            "atom: Failed to write to temporary atom template: %s", strerror(errno));
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
