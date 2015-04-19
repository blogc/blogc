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
#include "error.h"


typedef enum {
    SOURCE_START = 1,
    SOURCE_CONFIG_KEY,
    SOURCE_CONFIG_VALUE_START,
    SOURCE_CONFIG_VALUE,
    SOURCE_SEPARATOR,
    SOURCE_CONTENT_START,
    SOURCE_CONTENT,
} blogc_source_parser_state_t;


b_trie_t*
blogc_source_parse(const char *src, size_t src_len, blogc_error_t **err)
{
    if (err == NULL || *err != NULL)
        return NULL;

    size_t current = 0;
    size_t start = 0;

    char *key = NULL;
    char *tmp = NULL;
    b_trie_t *rv = b_trie_new(free);

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
                *err = blogc_error_parser(BLOGC_ERROR_SOURCE_PARSER, src, src_len,
                    current,
                    "Can't find a configuration key or the content separator.");
                break;

            case SOURCE_CONFIG_KEY:
                if ((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_')
                    break;
                if (c == ':') {
                    key = b_strndup(src + start, current - start);
                    state = SOURCE_CONFIG_VALUE_START;
                    break;
                }
                *err = blogc_error_parser(BLOGC_ERROR_SOURCE_PARSER, src, src_len,
                    current, "Invalid configuration key.");
                break;

            case SOURCE_CONFIG_VALUE_START:
                if (c != '\n' && c != '\r') {
                    state = SOURCE_CONFIG_VALUE;
                    start = current;
                    break;
                }
                *err = blogc_error_parser(BLOGC_ERROR_SOURCE_PARSER, src, src_len,
                    current, "Configuration value not provided for '%s'.",
                    key);
                break;

            case SOURCE_CONFIG_VALUE:
                if (c == '\n' || c == '\r') {
                    tmp = b_strndup(src + start, current - start);
                    b_trie_insert(rv, key, b_strdup(b_str_strip(tmp)));
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
                *err = blogc_error_parser(BLOGC_ERROR_SOURCE_PARSER, src, src_len,
                    current,
                    "Invalid content separator. Must be one or more '-' characters.");
                break;

            case SOURCE_CONTENT_START:
                start = current;
                state = SOURCE_CONTENT;
                break;

            case SOURCE_CONTENT:
                if (current == (src_len - 1))
                    b_trie_insert(rv, "CONTENT",
                        b_strndup(src + start, src_len - start));
                break;
        }

        if (*err != NULL)
            break;

        current++;
    }

    if (*err != NULL) {
        free(key);
        b_trie_free(rv);
        return NULL;
    }

    return rv;
}
