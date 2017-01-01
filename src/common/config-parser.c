/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2016 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "error.h"
#include "utils.h"
#include "config-parser.h"


typedef enum {
    CONFIG_START = 1,
    CONFIG_SECTION_START,
    CONFIG_SECTION,
    CONFIG_SECTION_KEY,
    CONFIG_SECTION_VALUE_START,
    CONFIG_SECTION_VALUE_QUOTE,
    CONFIG_SECTION_VALUE_SQUOTE,
    CONFIG_SECTION_VALUE_POST_QUOTED,
    CONFIG_SECTION_VALUE,
    CONFIG_SECTION_LIST_START,
    CONFIG_SECTION_LIST_QUOTE,
    CONFIG_SECTION_LIST_SQUOTE,
    CONFIG_SECTION_LIST_POST_QUOTED,
    CONFIG_SECTION_LIST,
} bc_configparser_state_t;

typedef enum {
    CONFIG_SECTION_TYPE_MAP = 1,
    CONFIG_SECTION_TYPE_LIST,
} bc_configparser_section_type_t;

typedef struct {
    bc_configparser_section_type_t type;
    void *data;
} bc_configparser_section_t;


static void
free_section(bc_configparser_section_t *section)
{
    if (section == NULL)
        return;

    switch (section->type) {
        case CONFIG_SECTION_TYPE_MAP:
            bc_trie_free(section->data);
            break;
        case CONFIG_SECTION_TYPE_LIST:
            bc_slist_free_full(section->data, free);
            break;
    }
    free(section);
}


bc_config_t*
bc_config_parse(const char *src, size_t src_len, const char *list_sections[],
    bc_error_t **err)
{
    if (err == NULL || *err != NULL)
        return NULL;

    size_t current = 0;
    size_t start = 0;

    bc_configparser_section_t *section = NULL;

    char *section_name = NULL;
    char *key = NULL;
    bc_string_t *value = NULL;
    bool escaped = false;

    bc_config_t *rv = bc_malloc(sizeof(bc_config_t));
    rv->root = bc_trie_new((bc_free_func_t) free_section);

    bc_configparser_state_t state = CONFIG_START;

    while (current < src_len) {
        char c = src[current];
        bool is_last = current == src_len - 1;

        if (escaped) {
            bc_string_append_c(value, c);
            escaped = false;
            current++;
            continue;
        }

        if (value != NULL && c == '\\') {
            escaped = true;
            current++;
            continue;
        }

        switch (state) {

            case CONFIG_START:
                if (c == '#' || c == ';') {
                    while (current < src_len) {
                        if (src[current] == '\r' || src[current] == '\n')
                            break;
                        current++;
                    }
                    break;
                }
                if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
                    break;
                if (c == '[') {
                    state = CONFIG_SECTION_START;
                    break;
                }
                if (section != NULL) {
                    start = current;
                    switch (section->type) {
                        case CONFIG_SECTION_TYPE_MAP:
                            state = CONFIG_SECTION_KEY;
                            break;
                        case CONFIG_SECTION_TYPE_LIST:
                            state = CONFIG_SECTION_LIST_START;
                            if (value == NULL)
                                value = bc_string_new();
                            break;
                    }
                    continue;
                }
                *err = bc_error_parser(BC_ERROR_CONFIG_PARSER, src, src_len,
                    current, "File must start with section.");
                break;

            case CONFIG_SECTION_START:
                start = current;
                state = CONFIG_SECTION;
                break;

            case CONFIG_SECTION:
                if (c == ']') {
                    section_name = bc_strndup(src + start, current - start);
                    section = bc_malloc(sizeof(bc_configparser_section_t));
                    section->type = CONFIG_SECTION_TYPE_MAP;
                    if (list_sections != NULL) {
                        for (size_t i = 0; list_sections[i] != NULL; i++) {
                            if (0 == strcmp(section_name, list_sections[i])) {
                                section->type = CONFIG_SECTION_TYPE_LIST;
                                break;
                            }
                        }
                    }
                    switch (section->type) {
                        case CONFIG_SECTION_TYPE_MAP:
                            section->data = bc_trie_new(free);
                            break;
                        case CONFIG_SECTION_TYPE_LIST:
                            section->data = NULL;
                            break;
                    }
                    bc_trie_insert(rv->root, section_name, section);
                    free(section_name);
                    section_name = NULL;
                    state = CONFIG_START;
                    break;
                }
                if (c != '\r' && c != '\n')
                    break;
                *err = bc_error_parser(BC_ERROR_CONFIG_PARSER, src, src_len,
                    current, "Section names can't have new lines.");
                break;

            case CONFIG_SECTION_KEY:
                if (c == '=') {
                    key = bc_strndup(src + start, current - start);
                    state = CONFIG_SECTION_VALUE_START;
                    if (value == NULL)
                        value = bc_string_new();
                    break;
                }
                if (c != '\r' && c != '\n' && !is_last)
                    break;
                // key without value, should we support it?
                size_t end = is_last && c != '\n' && c != '\r' ? src_len :
                    current;
                key = bc_strndup(src + start, end - start);
                *err = bc_error_parser(BC_ERROR_CONFIG_PARSER, src, src_len,
                    current, "Key without value: %s.", key);
                free(key);
                key = NULL;
                break;

            case CONFIG_SECTION_VALUE_START:
                if (c == ' ' || c == '\t' || c == '\f' || c == '\v')
                    break;
                if (c == '"') {
                    state = CONFIG_SECTION_VALUE_QUOTE;
                    break;
                }
                if (c == '\'') {
                    state = CONFIG_SECTION_VALUE_SQUOTE;
                    break;
                }
                bc_string_append_c(value, c);
                state = CONFIG_SECTION_VALUE;
                break;

            case CONFIG_SECTION_VALUE_QUOTE:
                if (c == '"') {
                    bc_trie_insert(section->data, bc_str_strip(key),
                        bc_string_free(value, false));
                    free(key);
                    key = NULL;
                    value = NULL;
                    state = CONFIG_SECTION_VALUE_POST_QUOTED;
                    break;
                }
                bc_string_append_c(value, c);
                break;

            case CONFIG_SECTION_VALUE_SQUOTE:
                if (c == '\'') {
                    bc_trie_insert(section->data, bc_str_strip(key),
                        bc_string_free(value, false));
                    free(key);
                    key = NULL;
                    value = NULL;
                    state = CONFIG_SECTION_VALUE_POST_QUOTED;
                    break;
                }
                bc_string_append_c(value, c);
                break;

            case CONFIG_SECTION_VALUE_POST_QUOTED:
                if (c == ' ' || c == '\t' || c == '\f' || c == '\v')
                    break;
                if (c == '\r' || c == '\n' || is_last) {
                    state = CONFIG_START;
                    break;
                }
                *err = bc_error_parser(BC_ERROR_CONFIG_PARSER, src, src_len,
                    current, "Invalid value for key, should not have anything "
                    "after quotes.");
                break;

            case CONFIG_SECTION_VALUE:
                if (c == '\r' || c == '\n' || is_last) {
                    if (is_last && c != '\r' && c != '\n')
                        bc_string_append_c(value, c);
                    bc_trie_insert(section->data, bc_str_strip(key),
                        bc_strdup(bc_str_rstrip(value->str)));
                    free(key);
                    key = NULL;
                    bc_string_free(value, true);
                    value = NULL;
                    state = CONFIG_START;
                    break;
                }
                bc_string_append_c(value, c);
                break;

            case CONFIG_SECTION_LIST_START:
                if (c == ' ' || c == '\t' || c == '\f' || c == '\v')
                    break;
                if (c == '"') {
                    state = CONFIG_SECTION_LIST_QUOTE;
                    break;
                }
                if (c == '\'') {
                    state = CONFIG_SECTION_LIST_SQUOTE;
                    break;
                }
                bc_string_append_c(value, c);
                state = CONFIG_SECTION_LIST;
                break;

            case CONFIG_SECTION_LIST_QUOTE:
                if (c == '"') {
                    section->data = bc_slist_append(section->data,
                        bc_string_free(value, false));
                    value = NULL;
                    state = CONFIG_SECTION_LIST_POST_QUOTED;
                    break;

                }
                bc_string_append_c(value, c);
                break;

            case CONFIG_SECTION_LIST_SQUOTE:
                if (c == '\'') {
                    section->data = bc_slist_append(section->data,
                        bc_string_free(value, false));
                    value = NULL;
                    state = CONFIG_SECTION_LIST_POST_QUOTED;
                    break;

                }
                bc_string_append_c(value, c);
                break;

            case CONFIG_SECTION_LIST_POST_QUOTED:
                if (c == ' ' || c == '\t' || c == '\f' || c == '\v')
                    break;
                if (c == '\r' || c == '\n' || is_last) {
                    state = CONFIG_START;
                    break;
                }
                *err = bc_error_parser(BC_ERROR_CONFIG_PARSER, src, src_len,
                    current, "Invalid value for list item, should not have "
                    "anything after quotes.");
                break;

            case CONFIG_SECTION_LIST:
                if (c == '\r' || c == '\n' || is_last) {
                    if (is_last && c != '\r' && c != '\n')
                        bc_string_append_c(value, c);
                    section->data = bc_slist_append(section->data,
                        bc_strdup(bc_str_strip(value->str)));
                    bc_string_free(value, true);
                    value = NULL;
                    state = CONFIG_START;
                    break;

                }
                bc_string_append_c(value, c);
                break;

        }

        if (*err != NULL) {
            bc_config_free(rv);
            rv = NULL;
            break;
        }

        current++;
    }

    free(section_name);
    free(key);
    bc_string_free(value, true);

    return rv;
}


static void
list_keys(const char *key, const char *value, bc_slist_t **l)
{
    *l = bc_slist_append(*l, bc_strdup(key));
}


char**
bc_config_list_sections(bc_config_t *config)
{
    if (config == NULL)
        return NULL;

    bc_slist_t *l = NULL;
    bc_trie_foreach(config->root, (bc_trie_foreach_func_t) list_keys, &l);

    char **rv = bc_malloc(sizeof(char*) * (bc_slist_length(l) + 1));

    size_t i = 0;
    for (bc_slist_t *tmp = l; tmp != NULL; tmp = tmp->next, i++)
        rv[i] = tmp->data;
    rv[i] = NULL;

    bc_slist_free(l);

    return rv;
}


char**
bc_config_list_keys(bc_config_t *config, const char *section)
{
    if (config == NULL)
        return NULL;

    bc_configparser_section_t *s = bc_trie_lookup(config->root, section);
    if (s == NULL)
        return NULL;

    if (s->type != CONFIG_SECTION_TYPE_MAP)
        return NULL;

    bc_slist_t *l = NULL;
    bc_trie_foreach(s->data, (bc_trie_foreach_func_t) list_keys, &l);

    char **rv = bc_malloc(sizeof(char*) * (bc_slist_length(l) + 1));

    size_t i = 0;
    for (bc_slist_t *tmp = l; tmp != NULL; tmp = tmp->next, i++)
        rv[i] = tmp->data;
    rv[i] = NULL;

    bc_slist_free(l);

    return rv;
}


const char*
bc_config_get(bc_config_t *config, const char *section, const char *key)
{
    if (config == NULL)
        return NULL;

    bc_configparser_section_t *s = bc_trie_lookup(config->root, section);
    if (s == NULL)
        return NULL;

    if (s->type != CONFIG_SECTION_TYPE_MAP)
        return NULL;

    return bc_trie_lookup(s->data, key);
}


const char*
bc_config_get_with_default(bc_config_t *config, const char *section, const char *key,
    const char *default_)
{
    const char *rv = bc_config_get(config, section, key);
    if (rv == NULL)
        return default_;
    return rv;
}


char**
bc_config_get_list(bc_config_t *config, const char *section)
{
    if (config == NULL)
        return NULL;

    bc_configparser_section_t *s = bc_trie_lookup(config->root, section);
    if (s == NULL)
        return NULL;

    if (s->type != CONFIG_SECTION_TYPE_LIST)
        return NULL;

    char **rv = bc_malloc(sizeof(char*) * (bc_slist_length(s->data) + 1));

    size_t i = 0;
    for (bc_slist_t *tmp = s->data; tmp != NULL; tmp = tmp->next, i++)
        rv[i] = bc_strdup(tmp->data);
    rv[i] = NULL;

    return rv;
}


void
bc_config_free(bc_config_t *config)
{
    if (config == NULL)
        return;
    bc_trie_free(config->root);
    free(config);
}
