/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2016 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#include <stdbool.h>
#include <stdlib.h>
#include "error.h"
#include "utils.h"
#include "config-parser.h"


typedef enum {
    CONFIG_START = 1,
    CONFIG_SECTION_START,
    CONFIG_SECTION,
    CONFIG_SECTION_KEY,
    CONFIG_SECTION_VALUE_START,
    CONFIG_SECTION_VALUE,
} bc_configparser_state_t;


bc_config_t*
bc_config_parse(const char *src, size_t src_len, bc_error_t **err)
{
    if (err != NULL && *err != NULL)
        return NULL;

    size_t current = 0;
    size_t start = 0;

    bc_trie_t *section = NULL;

    char *section_name = NULL;
    char *key = NULL;
    char *value = NULL;

    bc_config_t *rv = bc_malloc(sizeof(bc_config_t));
    rv->root = bc_trie_new((bc_free_func_t) bc_trie_free);

    bc_configparser_state_t state = CONFIG_START;

    while (current < src_len) {
        char c = src[current];
        bool is_last = current == src_len - 1;

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
                    state = CONFIG_SECTION_KEY;
                    continue;
                }
                if (err != NULL)
                    *err = bc_error_new_printf(BC_ERROR_CONFIG_PARSER,
                        "File must start with section");
                break;

            case CONFIG_SECTION_START:
                start = current;
                state = CONFIG_SECTION;
                break;

            case CONFIG_SECTION:
                if (c == ']') {
                    section_name = bc_strndup(src + start, current - start);
                    section = bc_trie_new(free);
                    bc_trie_insert(rv->root, section_name, section);
                    free(section_name);
                    section_name = NULL;
                    state = CONFIG_START;
                    break;
                }
                if (c != '\r' && c != '\n')
                    break;
                if (err != NULL)
                    *err = bc_error_new_printf(BC_ERROR_CONFIG_PARSER,
                        "Section names can't have new lines");
                break;

            case CONFIG_SECTION_KEY:
                if (c == '=') {
                    key = bc_strndup(src + start, current - start);
                    state = CONFIG_SECTION_VALUE_START;
                    break;
                }
                if (c != '\r' && c != '\n' && !is_last)
                    break;
                // key without value, should we support it?
                if (err != NULL) {
                    size_t end = is_last && c != '\n' && c != '\r' ? src_len :
                        current;
                    key = bc_strndup(src + start, end - start);
                    *err = bc_error_new_printf(BC_ERROR_CONFIG_PARSER,
                        "Key without value: %s", key);
                    free(key);
                    key = NULL;
                }
                break;

            case CONFIG_SECTION_VALUE_START:
                start = current;
                state = CONFIG_SECTION_VALUE;
                break;

            case CONFIG_SECTION_VALUE:
                if (c == '\r' || c == '\n' || is_last) {
                    size_t end = is_last && c != '\n' && c != '\r' ? src_len :
                        current;
                    value = bc_strndup(src + start, end - start);
                    bc_trie_insert(section, bc_str_strip(key),
                        bc_strdup(bc_str_strip(value)));
                    free(key);
                    key = NULL;
                    free(value);
                    value = NULL;
                    state = CONFIG_START;
                    break;
                }
                break;

        }

        if (err != NULL && *err != NULL) {
            bc_config_free(rv);
            rv = NULL;
            break;
        }

        current++;
    }

    free(section_name);
    free(key);
    free(value);

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

    unsigned int i = 0;
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

    bc_trie_t *s = bc_trie_lookup(config->root, section);
    if (s == NULL)
        return NULL;

    bc_slist_t *l = NULL;
    bc_trie_foreach(s, (bc_trie_foreach_func_t) list_keys, &l);

    char **rv = bc_malloc(sizeof(char*) * (bc_slist_length(l) + 1));

    unsigned int i = 0;
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

    bc_trie_t *s = bc_trie_lookup(config->root, section);
    if (s == NULL)
        return NULL;

    return bc_trie_lookup(s, key);
}


void
bc_config_free(bc_config_t *config)
{
    if (config == NULL)
        return;
    bc_trie_free(config->root);
    free(config);
}
