/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2019 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#include <libgen.h>
#include <stdbool.h>
#include <stdlib.h>
#include <squareball.h>

#include "settings.h"


static const struct default_settings_map {
    const char *key;
    const char *default_value;
} default_settings[] = {

    // source
    {"content_dir", "content"},
    {"template_dir", "templates"},
    {"atom_template", NULL},  // default: atom.c
    {"main_template", "main.tmpl"},
    {"source_ext", ".txt"},
    {"listing_entry", NULL},
    {"posts_sort", NULL},

    // pagination
    {"pagination_prefix", "page"},
    {"posts_per_page", "10"},
    {"atom_posts_per_page", "10"},

    // html
    {"html_ext", "/index.html"},
    {"index_prefix", ""},
    {"post_prefix", "post"},
    {"tag_prefix", "tag"},
    {"html_order", "DESC"},

    // atom
    {"atom_prefix", "atom"},
    {"atom_ext", ".xml"},
    {"atom_order", "DESC"},
    {"atom_legacy_entry_id", NULL},

    // generic
    {"date_format", "%b %d, %Y, %I:%M %p GMT"},
    {"locale", NULL},

    {NULL, NULL},
};


static const char* required_global[] = {
    "AUTHOR_NAME",
    "AUTHOR_EMAIL",
    "SITE_TITLE",
    "SITE_TAGLINE",
    "BASE_DOMAIN",
    NULL,
};


static const char* list_sections[] = {
    "posts",
    "pages",
    "copy",
    "copy_files",  // backward compatibility
    "tags",
    NULL,
};


bm_settings_t*
bm_settings_parse(const char *content, size_t content_len, sb_error_t **err)
{
    if (err == NULL || *err != NULL)
        return NULL;

    if (content == NULL)
        return NULL;

    sb_config_t *config = sb_config_parse(content, content_len, list_sections,
        err);
    if (config == NULL || (err != NULL && *err != NULL))
        return NULL;

    bm_settings_t *rv = sb_malloc(sizeof(bm_settings_t));
    rv->global = sb_trie_new(free);
    rv->settings = sb_trie_new(free);
    rv->posts = NULL;
    rv->pages = NULL;
    rv->copy = NULL;
    rv->tags = NULL;

    // this is some code for compatibility with the [environment] section,
    // even if I never released a version with it, but some people is using
    // it already.
    const char *section = NULL;
    char **global = sb_config_list_keys(config, "global");
    if (global != NULL) {
        section = "global";
    }
    else {
        global = sb_config_list_keys(config, "environment");
        if (global != NULL) {
            section = "environment";
        }
        else {
            section = "global";
        }
    }

    if (global != NULL) {
        for (size_t i = 0; global[i] != NULL; i++) {
            for (size_t j = 0; global[i][j] != '\0'; j++) {
                if (!((global[i][j] >= 'A' && global[i][j] <= 'Z') || global[i][j] == '_')) {
                    *err = sb_strerror_new_printf(
                        "settings: Invalid [%s] key: %s", section, global[i]);
                    sb_strv_free(global);
                    bm_settings_free(rv);
                    rv = NULL;
                    goto cleanup;
                }
            }
            sb_trie_insert(rv->global, global[i],
                sb_strdup(sb_config_get(config, section, global[i])));
        }
    }
    sb_strv_free(global);

    for (size_t i = 0; required_global[i] != NULL; i++) {
        const char *value = sb_trie_lookup(rv->global, required_global[i]);
        if (value == NULL || value[0] == '\0') {
            *err = sb_strerror_new_printf(
                "settings: [%s] key required but not found or empty: %s", section,
                required_global[i]);
            bm_settings_free(rv);
            rv = NULL;
            goto cleanup;
        }
    }

    for (size_t i = 0; default_settings[i].key != NULL; i++) {
        const char *value = sb_config_get_with_default(
            config, "settings", default_settings[i].key,
            default_settings[i].default_value);
        if (value != NULL) {
            sb_trie_insert(rv->settings, default_settings[i].key,
                sb_strdup(value));
        }
    }

    rv->posts = sb_config_get_list(config, "posts");
    rv->pages = sb_config_get_list(config, "pages");
    rv->tags = sb_config_get_list(config, "tags");

    // this is for backward compatibility too.
    rv->copy = sb_config_get_list(config, "copy");
    if (rv->copy == NULL)
        rv->copy = sb_config_get_list(config, "copy_files");

cleanup:

    sb_config_free(config);

    return rv;
}


void
bm_settings_free(bm_settings_t *settings)
{
    if (settings == NULL)
        return;
    sb_trie_free(settings->global);
    sb_trie_free(settings->settings);
    sb_strv_free(settings->posts);
    sb_strv_free(settings->pages);
    sb_strv_free(settings->copy);
    sb_strv_free(settings->tags);
    free(settings);
}
