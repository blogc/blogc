/*
 * blogc: A blog compiler.
 * Copyright (C) 2015 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdbool.h>
#include <string.h>

#include "utils/utils.h"
#include "content-parser.h"
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
    size_t end_excerpt = 0;

    char *key = NULL;
    char *tmp = NULL;
    char *content = NULL;
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
                    if (((current - start == 8) &&
                         (0 == strncmp("FILENAME", src + start, 8))) ||
                        ((current - start == 7) &&
                         (0 == strncmp("CONTENT", src + start, 7))) ||
                        ((current - start == 14) &&
                         (0 == strncmp("DATE_FORMATTED", src + start, 14))) ||
                        ((current - start == 20) &&
                         (0 == strncmp("DATE_FIRST_FORMATTED", src + start, 20))) ||
                        ((current - start == 19) &&
                         (0 == strncmp("DATE_LAST_FORMATTED", src + start, 19))) ||
                        ((current - start == 10) &&
                         (0 == strncmp("PAGE_FIRST", src + start, 10))) ||
                        ((current - start == 13) &&
                         (0 == strncmp("PAGE_PREVIOUS", src + start, 13))) ||
                        ((current - start == 12) &&
                         (0 == strncmp("PAGE_CURRENT", src + start, 12))) ||
                        ((current - start == 9) &&
                         (0 == strncmp("PAGE_NEXT", src + start, 9))) ||
                        ((current - start == 9) &&
                         (0 == strncmp("PAGE_LAST", src + start, 9))) ||
                        ((current - start == 13) &&
                         (0 == strncmp("BLOGC_VERSION", src + start, 13))))
                    {
                        *err = blogc_error_new_printf(BLOGC_ERROR_SOURCE_PARSER,
                            "'%s' variable is forbidden in source files. It will "
                            "be set for you by the compiler.", key);
                        break;
                    }
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
                    "Invalid content separator. Must be more than one '-' characters.");
                break;

            case SOURCE_CONTENT_START:
                start = current;
                state = SOURCE_CONTENT;
                break;

            case SOURCE_CONTENT:
                if (current == (src_len - 1)) {
                    tmp = b_strndup(src + start, src_len - start);
                    b_trie_insert(rv, "RAW_CONTENT", tmp);
                    content = blogc_content_parse(tmp, &end_excerpt);
                    b_trie_insert(rv, "CONTENT", content);
                    b_trie_insert(rv, "EXCERPT", end_excerpt == 0 ?
                        b_strdup(content) : b_strndup(content, end_excerpt));
                }
                break;
        }

        if (*err != NULL)
            break;

        current++;
    }

    if (*err == NULL && b_trie_size(rv) == 0) {

        // ok, nothing found in the config trie, but no error set either.
        // let's try to be nice with the users and provide some reasonable
        // output. :)
        switch (state) {
            case SOURCE_START:
                *err = blogc_error_parser(BLOGC_ERROR_SOURCE_PARSER, src, src_len,
                    current, "Your source file is empty.");
                break;
            case SOURCE_CONFIG_KEY:
                *err = blogc_error_parser(BLOGC_ERROR_SOURCE_PARSER, src, src_len,
                    current, "Your last configuration key is missing ':' and "
                    "the value");
                break;
            case SOURCE_CONFIG_VALUE_START:
                *err = blogc_error_parser(BLOGC_ERROR_SOURCE_PARSER, src, src_len,
                    current, "Configuration value not provided for '%s'.",
                    key);
                break;
            case SOURCE_CONFIG_VALUE:
                *err = blogc_error_parser(BLOGC_ERROR_SOURCE_PARSER, src, src_len,
                    current, "No line ending after the configuration value for "
                    "'%s'.", key);
                break;
            case SOURCE_SEPARATOR:
            case SOURCE_CONTENT_START:
            case SOURCE_CONTENT:
                break;  // won't happen, and if even happen, shouldn't be fatal
        }
    }

    if (*err != NULL) {
        free(key);
        b_trie_free(rv);
        return NULL;
    }

    return rv;
}
