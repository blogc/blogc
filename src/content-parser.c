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
#include <string.h>

#include "utils/utils.h"
#include "content-parser.h"
#include "error.h"


// this is a half ass implementation of a markdown-like syntax. bugs are
// expected. feel free to improve the parser and and new features.


// TODO: block elements: list, horizontal rule
// TODO: inline elements: links, emphasis, code, images
// TODO: error handling


typedef enum {
    CONTENT_START_LINE = 1,
    CONTENT_HEADER,
    CONTENT_HEADER_TITLE_START,
    CONTENT_HEADER_TITLE,
    CONTENT_HTML,
    CONTENT_HTML_END,
    CONTENT_BLOCKQUOTE,
    CONTENT_BLOCKQUOTE_START,
    CONTENT_BLOCKQUOTE_END,
    CONTENT_CODE,
    CONTENT_CODE_START,
    CONTENT_CODE_END,
    CONTENT_PARAGRAPH,
    CONTENT_PARAGRAPH_END,
} blogc_content_parser_state_t;


char*
blogc_content_parse(const char *src, size_t src_len, blogc_error_t **err)
{
    if (err == NULL || *err != NULL)
        return NULL;

    size_t current = 0;
    size_t start = 0;
    size_t end = 0;

    unsigned int header_level = 0;
    char *prefix = NULL;
    char *tmp = NULL;

    b_slist_t *lines = NULL;

    b_string_t *rv = b_string_new();
    b_string_t *tmp_str = NULL;

    blogc_content_parser_state_t state = CONTENT_START_LINE;

    while (current < src_len) {
        char c = src[current];
        bool is_last = current == src_len - 1;

        switch (state) {

            case CONTENT_START_LINE:
                if (c == '\n' || c == '\r')
                    break;
                if (c == '#') {
                    header_level = 1;
                    state = CONTENT_HEADER;
                    break;
                }
                if (c == ' ' || c == '\t') {
                    state = CONTENT_CODE;
                    start = current;
                    break;
                }
                if (c == '<') {
                    state = CONTENT_HTML;
                    start = current;
                    break;
                }
                if (c == '>') {
                    state = CONTENT_BLOCKQUOTE;
                    start = current;
                    break;
                }
                start = current;
                state = CONTENT_PARAGRAPH;
                break;

            case CONTENT_HEADER:
                if (c == '#') {
                    header_level += 1;
                    break;
                }
                if (c == ' ' || c == '\t') {
                    state = CONTENT_HEADER_TITLE_START;
                    break;
                }
                // error;
                break;

            case CONTENT_HEADER_TITLE_START:
                if (c == ' ' || c == '\t')
                    break;
                if (c != '\n' || c != '\r') {
                    start = current;
                    state = CONTENT_HEADER_TITLE;
                    break;
                }
                // error;
                break;

            case CONTENT_HEADER_TITLE:
                if (c == '\n' || c == '\r' || is_last) {
                    end = is_last ? src_len : current;
                    tmp = b_strndup(src + start, end - start);
                    b_string_append_printf(rv, "<h%d>%s</h%d>\n", header_level,
                        tmp, header_level);
                    free(tmp);
                    tmp = NULL;
                    state = CONTENT_START_LINE;
                    start = current;
                    break;
                }
                break;


            case CONTENT_HTML:
                if (c == '\n' || c == '\r' || is_last) {
                    state = CONTENT_HTML_END;
                    end = is_last ? src_len : current;
                }
                if (!is_last)
                    break;

            case CONTENT_HTML_END:
                if (c == '\n' || c == '\r' || is_last) {
                    tmp = b_strndup(src + start, end - start);
                    b_string_append_printf(rv, "%s\n", tmp);
                    free(tmp);
                    tmp = NULL;
                    state = CONTENT_START_LINE;
                    start = current;
                }
                else
                    state = CONTENT_HTML;
                break;

            case CONTENT_BLOCKQUOTE:
                if (c == ' ' || c == '\t')
                    break;
                prefix = b_strndup(src + start, current - start);
                state = CONTENT_BLOCKQUOTE_START;

            case CONTENT_BLOCKQUOTE_START:
                if (c == '\n' || c == '\r' || is_last) {
                    end = is_last ? src_len : current;
                    tmp = b_strndup(src + start, end - start);
                    if (b_str_starts_with(tmp, prefix)) {
                        lines = b_slist_append(lines, b_strdup(tmp + strlen(prefix)));
                    }
                    else {
                        // error
                    }
                    free(tmp);
                    tmp = NULL;
                    state = CONTENT_BLOCKQUOTE_END;
                }
                if (!is_last)
                    break;

            case CONTENT_BLOCKQUOTE_END:
                if (c == '\n' || c == '\r' || is_last) {
                    tmp_str = b_string_new();
                    for (b_slist_t *l = lines; l != NULL; l = l->next) {
                        if (l->next == NULL)
                            b_string_append_printf(tmp_str, "%s", l->data);
                        else
                            b_string_append_printf(tmp_str, "%s\n", l->data);
                    }
                    tmp = blogc_content_parse(tmp_str->str, tmp_str->len, err);
                    if (*err == NULL) {
                        b_string_append_printf(rv, "<blockquote>%s</blockquote>\n",
                            tmp);
                    }
                    free(tmp);
                    tmp = NULL;
                    b_string_free(tmp_str, true);
                    tmp_str = NULL;
                    b_slist_free_full(lines, free);
                    lines = NULL;
                    free(prefix);
                    prefix = NULL;
                    state = CONTENT_START_LINE;
                    start = current;
                }
                else {
                    start = current;
                    state = CONTENT_BLOCKQUOTE_START;
                }
                break;

            case CONTENT_CODE:
                if (c == ' ' || c == '\t')
                    break;
                prefix = b_strndup(src + start, current - start);
                state = CONTENT_CODE_START;

            case CONTENT_CODE_START:
                if (c == '\n' || c == '\r' || is_last) {
                    end = is_last ? src_len : current;
                    tmp = b_strndup(src + start, end - start);
                    if (b_str_starts_with(tmp, prefix)) {
                        lines = b_slist_append(lines, b_strdup(tmp + strlen(prefix)));
                    }
                    else {
                        // error
                    }
                    free(tmp);
                    tmp = NULL;
                    state = CONTENT_CODE_END;
                }
                if (!is_last)
                    break;

            case CONTENT_CODE_END:
                if (c == '\n' || c == '\r' || is_last) {
                    b_string_append(rv, "<pre><code>");
                    for (b_slist_t *l = lines; l != NULL; l = l->next) {
                        if (l->next == NULL)
                            b_string_append_printf(rv, "%s", l->data);
                        else
                            b_string_append_printf(rv, "%s\n", l->data);
                    }
                    b_string_append(rv, "</code></pre>\n");
                    b_slist_free_full(lines, free);
                    lines = NULL;
                    free(prefix);
                    prefix = NULL;
                    state = CONTENT_START_LINE;
                    start = current;
                }
                else {
                    start = current;
                    state = CONTENT_CODE_START;
                }
                break;

            case CONTENT_PARAGRAPH:
                if (c == '\n' || c == '\r' || is_last) {
                    state = CONTENT_PARAGRAPH_END;
                    end = is_last ? src_len : current;
                }
                if (!is_last)
                    break;

            case CONTENT_PARAGRAPH_END:
                if (c == '\n' || c == '\r' || is_last) {
                    tmp = b_strndup(src + start, end - start);
                    b_string_append_printf(rv, "<p>%s</p>\n", tmp);
                    free(tmp);
                    tmp = NULL;
                    state = CONTENT_START_LINE;
                    start = current;
                }
                else
                    state = CONTENT_PARAGRAPH;
                break;

        }

        if (*err != NULL)
            break;

        current++;
    }

    if (*err != NULL) {
        b_string_free(rv, true);
        return NULL;
    }

    return b_string_free(rv, false);
}
