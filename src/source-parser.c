/*
 * blogc: A blog compiler.
 * Copyright (C) 2015 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file COPYING.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdbool.h>

#include "utils/utils.h"
#include "source-parser.h"
#include "output.h"


typedef enum {
    SOURCE_START = 1,
    SOURCE_CONFIG_KEY,
    SOURCE_CONFIG_VALUE_START,
    SOURCE_CONFIG_VALUE,
    SOURCE_SEPARATOR,
    SOURCE_CONTENT_START,
    SOURCE_CONTENT,
} blogc_source_parser_state_t;


blogc_source_t*
blogc_source_parse(const char *src, size_t src_len)
{
    size_t current = 0;
    size_t start = 0;

    bool error = false;
    char *key = NULL;
    char *tmp = NULL;
    b_trie_t *config = b_trie_new(free);
    char *content = NULL;

    blogc_source_parser_state_t state = SOURCE_START;

    while (current < src_len) {
        char c = src[current];

        switch (state) {

            case SOURCE_START:
                if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
                    break;
                if (c >= 'A' && c <= 'Z') {
                    state = SOURCE_CONFIG_KEY;
                    start = current;
                    break;
                }
                if (c == '-') {
                    state = SOURCE_SEPARATOR;
                    break;
                }
                error = true;
                break;

            case SOURCE_CONFIG_KEY:
                if ((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_')
                    break;
                if (c == ':') {
                    key = b_strndup(src + start, current - start);
                    state = SOURCE_CONFIG_VALUE_START;
                    break;
                }
                error = true;
                break;

            case SOURCE_CONFIG_VALUE_START:
                if (c != '\n' && c != '\r') {
                    state = SOURCE_CONFIG_VALUE;
                    start = current;
                    break;
                }
                error = true;
                break;

            case SOURCE_CONFIG_VALUE:
                if (c == '\n' || c == '\r') {
                    tmp = b_strndup(src + start, current - start);
                    b_trie_insert(config, key, b_strdup(b_str_strip(tmp)));
                    free(tmp);
                    free(key);
                    key = NULL;
                    state = SOURCE_START;
                }
                break;

            case SOURCE_SEPARATOR:
                if (c == '-')
                    break;
                if (c == '\n' || c == '\r') {
                    state = SOURCE_CONTENT_START;
                    break;
                }
                error = true;
                break;

            case SOURCE_CONTENT_START:
                start = current;
                state = SOURCE_CONTENT;
                break;

            case SOURCE_CONTENT:
                if (current == (src_len - 1))
                    content = b_strndup(src + start, src_len - start);
                break;
        }

        if (error)
            break;

        current++;
    }

    if (error) {
        free(key);
        free(content);
        b_trie_free(config);
        blogc_parser_syntax_error("source", src, src_len, current);
        return NULL;
    }

    blogc_source_t *rv = malloc(sizeof(blogc_source_t));
    rv->config = config;
    rv->content = content;

    return rv;
}


void
blogc_source_free(blogc_source_t *source)
{
    if (source == NULL)
        return;
    free(source->content);
    b_trie_free(source->config);
    free(source);
}
