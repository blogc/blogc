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
#include "../common/config-parser.h"
#include "../common/error.h"
#include "../common/file.h"
#include "../common/utils.h"
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
    "sass",
    NULL,
};


bm_settings_t*
bm_settings_parse(const char *content, size_t content_len, bc_error_t **err)
{
    if (err == NULL || *err != NULL)
        return NULL;

    if (content == NULL)
        return NULL;

    bc_config_t *config = bc_config_parse(content, content_len, list_sections,
        err);
    if (config == NULL || (err != NULL && *err != NULL))
        return NULL;

    bm_settings_t *rv = bc_malloc(sizeof(bm_settings_t));
    rv->global = bc_trie_new(free);
    rv->settings = bc_trie_new(free);
    rv->posts = NULL;
    rv->pages = NULL;
    rv->copy = NULL;
    rv->tags = NULL;
    rv->sass = NULL;

    // this is some code for compatibility with the [environment] section,
    // even if I never released a version with it, but some people is using
    // it already.
    const char *section = NULL;
    char **global = bc_config_list_keys(config, "global");
    if (global != NULL) {
        section = "global";
    }
    else {
        global = bc_config_list_keys(config, "environment");
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
                    *err = bc_error_new_printf(BLOGC_MAKE_ERROR_SETTINGS,
                        "Invalid [%s] key: %s", section, global[i]);
                    bc_strv_free(global);
                    bm_settings_free(rv);
                    rv = NULL;
                    goto cleanup;
                }
            }
            bc_trie_insert(rv->global, global[i],
                bc_strdup(bc_config_get(config, section, global[i])));
        }
    }
    bc_strv_free(global);

    for (size_t i = 0; required_global[i] != NULL; i++) {
        const char *value = bc_trie_lookup(rv->global, required_global[i]);
        if (value == NULL || value[0] == '\0') {
            *err = bc_error_new_printf(BLOGC_MAKE_ERROR_SETTINGS,
                "[%s] key required but not found or empty: %s", section,
                required_global[i]);
            bm_settings_free(rv);
            rv = NULL;
            goto cleanup;
        }
    }

    for (size_t i = 0; default_settings[i].key != NULL; i++) {
        const char *value = bc_config_get_with_default(
            config, "settings", default_settings[i].key,
            default_settings[i].default_value);
        if (value != NULL) {
            bc_trie_insert(rv->settings, default_settings[i].key,
                bc_strdup(value));
        }
    }

    rv->posts = bc_config_get_list(config, "posts");
    rv->pages = bc_config_get_list(config, "pages");
    rv->tags = bc_config_get_list(config, "tags");
    rv->sass = bc_config_get_list(config, "sass");

    // this is for backward compatibility too.
    rv->copy = bc_config_get_list(config, "copy");
    if (rv->copy == NULL)
        rv->copy = bc_config_get_list(config, "copy_files");

cleanup:

    bc_config_free(config);

    return rv;
}


void
bm_settings_free(bm_settings_t *settings)
{
    if (settings == NULL)
        return;
    bc_trie_free(settings->global);
    bc_trie_free(settings->settings);
    bc_strv_free(settings->posts);
    bc_strv_free(settings->pages);
    bc_strv_free(settings->copy);
    bc_strv_free(settings->tags);
    bc_strv_free(settings->sass);
    free(settings);
}
