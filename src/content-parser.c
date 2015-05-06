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


// TODO: inline elements: line breaks
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
    CONTENT_UNORDERED_LIST_OR_HORIZONTAL_RULE,
    CONTENT_HORIZONTAL_RULE,
    CONTENT_UNORDERED_LIST_START,
    CONTENT_UNORDERED_LIST_END,
    CONTENT_ORDERED_LIST,
    CONTENT_ORDERED_LIST_SPACE,
    CONTENT_ORDERED_LIST_START,
    CONTENT_ORDERED_LIST_END,
    CONTENT_PARAGRAPH,
    CONTENT_PARAGRAPH_END,
} blogc_content_parser_state_t;


char*
blogc_content_parse_inline(const char *src)
{
    // this function is always called by blogc_content_parse, then its safe to
    // assume that src is always nul-terminated.
    size_t src_len = strlen(src);

    size_t current = 0;

    b_string_t *rv = b_string_new();

    bool open_em_ast = false;
    bool open_strong_ast = false;
    bool open_em_und = false;
    bool open_strong_und = false;
    bool open_code = false;
    bool open_code_double = false;
    unsigned int link_state = 0;
    unsigned int image_state = 0;
    size_t link_start = 0;
    size_t image_start = 0;

    char *tmp = NULL;
    char *title = NULL;
    char *alt = NULL;

    while (current < src_len) {
        char c = src[current];
        bool is_last = current == src_len - 1;

        switch (c) {
            case '*':
            case '_':
                if (open_code || open_code_double) {
                    b_string_append_c(rv, c);
                    break;
                }
                if (!is_last && src[current + 1] == c) {
                    current++;
                    if ((c == '*' && open_strong_ast) ||
                        (c == '_' && open_strong_und))
                    {
                        b_string_append(rv, "</strong>");
                        if (c == '*')
                            open_strong_ast = false;
                        else
                            open_strong_und = false;
                    }
                    else {
                        b_string_append(rv, "<strong>");
                        if (c == '*')
                            open_strong_ast = true;
                        else
                            open_strong_und = true;
                    }
                }
                else {
                    if ((c == '*' && open_em_ast) || (c == '_' && open_em_und)) {
                        b_string_append(rv, "</em>");
                        if (c == '*')
                            open_em_ast = false;
                        else
                            open_em_und = false;
                    }
                    else {
                        b_string_append(rv, "<em>");
                        if (c == '*')
                            open_em_ast = true;
                        else
                            open_em_und = true;
                    }
                }
                break;

            case '`':
                if (!is_last && src[current + 1] == c) {
                    current++;
                    if (open_code_double)
                        b_string_append(rv, "</code>");
                    else
                        b_string_append(rv, "<code>");
                    open_code_double = !open_code_double;
                }
                else {
                    if (open_code)
                        b_string_append(rv, "</code>");
                    else
                        b_string_append(rv, "<code>");
                    open_code = !open_code;
                }
                break;

            case '[':
                if (open_code || open_code_double) {
                    b_string_append_c(rv, c);
                    break;
                }
                if (link_state == 0 && image_state == 0) {
                    tmp = strchr(src + current, ']');
                    if (tmp != NULL) {
                        if (strlen(tmp) > 1 && tmp[1] == '(') {
                            tmp = strchr(tmp, ')');
                            if (tmp != NULL) {  // this is a link
                                link_start = current + 1;  // its safe
                                link_state = 1;
                                break;
                            }
                        }
                    }
                    b_string_append_c(rv, c);
                }
                break;

            case '!':
                if (open_code || open_code_double) {
                    b_string_append_c(rv, c);
                    break;
                }
                if (link_state == 0 && image_state == 0) {
                    if (!is_last && src[current + 1] == '[') {
                        tmp = strchr(src + current + 1, ']');
                        if (tmp != NULL) {
                            if (strlen(tmp) > 1 && tmp[1] == '(') {
                                tmp = strchr(tmp, ')');
                                if (tmp != NULL) {  // this is an image
                                    image_start = current + 2;  // its safe
                                    image_state = 1;
                                    break;
                                }
                            }
                        }
                    }
                    b_string_append_c(rv, c);
                }
                break;

            case ']':
                if (open_code || open_code_double) {
                    b_string_append_c(rv, c);
                    break;
                }
                if (link_state == 1) {
                    link_state = 2;
                    title = b_strndup(src + link_start, current - link_start);
                    break;
                }
                if (image_state == 1) {
                    image_state = 2;
                    alt = b_strndup(src + image_start, current - image_start);
                    break;
                }
                b_string_append_c(rv, c);
                break;

            case '(':
                if (open_code || open_code_double) {
                    b_string_append_c(rv, c);
                    break;
                }
                if (link_state == 2) {
                    link_state = 3;
                    link_start = current + 1;  // its safe
                    break;
                }
                if (image_state == 2) {
                    image_state = 3;
                    image_start = current + 1;  // its safe
                    break;
                }
                b_string_append_c(rv, c);
                break;

            case ')':
                if (open_code || open_code_double) {
                    b_string_append_c(rv, c);
                    break;
                }
                if (link_state == 3) {
                    link_state = 0;
                    tmp = b_strndup(src + link_start, current - link_start);
                    b_string_append_printf(rv, "<a href=\"%s\">%s</a>", tmp, title);
                    free(tmp);
                    tmp = NULL;
                    free(title);
                    title = NULL;
                    break;
                }
                if (image_state == 3) {
                    image_state = 0;
                    tmp = b_strndup(src + image_start, current - image_start);
                    b_string_append_printf(rv, "<img src=\"%s\" alt=\"%s\">", tmp, alt);
                    free(tmp);
                    tmp = NULL;
                    free(alt);
                    alt = NULL;
                    break;
                }
                b_string_append_c(rv, c);
                break;

            case '&':
                b_string_append(rv, "&amp;");
                break;

            case '<':
                b_string_append(rv, "&lt;");
                break;

            case '>':
                b_string_append(rv, "&gt;");
                break;

            case '"':
                b_string_append(rv, "&quot;");
                break;

            case '\'':
                b_string_append(rv, "&#x27;");
                break;

            case '/':
                b_string_append(rv, "&#x2F;");
                break;

            default:
                if (link_state == 0 && image_state == 0)
                    b_string_append_c(rv, c);
        }

        current++;
    }

    return b_string_free(rv, false);
}


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
    size_t prefix_len = 0;
    char *tmp = NULL;
    char *tmp2 = NULL;
    char *tmp3 = NULL;
    char *parsed = NULL;
    char **tmpv = NULL;

    char d;

    b_slist_t *lines = NULL;

    b_string_t *rv = b_string_new();
    b_string_t *tmp_str = NULL;

    blogc_content_parser_state_t state = CONTENT_START_LINE;

    while (current < src_len) {
        char c = src[current];
        bool is_last = current == src_len - 1;

        switch (state) {

            case CONTENT_START_LINE:
                if (c == '\n' || c == '\r' || is_last)
                    break;
                if (c == '#') {
                    header_level = 1;
                    state = CONTENT_HEADER;
                    break;
                }
                if (c == '*' || c == '+' || c == '-') {
                    state = CONTENT_UNORDERED_LIST_OR_HORIZONTAL_RULE;
                    start = current;
                    d = c;
                    break;
                }
                if (c >= '0' && c <= '9') {
                    state = CONTENT_ORDERED_LIST;
                    start = current;
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
                *err = blogc_error_parser(BLOGC_ERROR_CONTENT_PARSER, src, src_len,
                    current, "Malformed header, no space or tab after '#'");
                break;

            case CONTENT_HEADER_TITLE_START:
                if (c == ' ' || c == '\t')
                    break;
                start = current;
                if (c != '\n' && c != '\r') {
                    state = CONTENT_HEADER_TITLE;
                    break;
                }

            case CONTENT_HEADER_TITLE:
                if (c == '\n' || c == '\r' || is_last) {
                    end = is_last && c != '\n' && c != '\r' ? src_len : current;
                    tmp = b_strndup(src + start, end - start);
                    parsed = blogc_content_parse_inline(tmp);
                    b_string_append_printf(rv, "<h%d>%s</h%d>\n", header_level,
                        parsed, header_level);
                    free(parsed);
                    parsed = NULL;
                    free(tmp);
                    tmp = NULL;
                    state = CONTENT_START_LINE;
                    start = current;
                }
                break;

            case CONTENT_HTML:
                if (c == '\n' || c == '\r' || is_last) {
                    state = CONTENT_HTML_END;
                    end = is_last && c != '\n' && c != '\r' ? src_len : current;
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
                break;

            case CONTENT_BLOCKQUOTE_START:
                if (c == '\n' || c == '\r' || is_last) {
                    end = is_last && c != '\n' && c != '\r' ? src_len : current;
                    tmp = b_strndup(src + start, end - start);
                    if (b_str_starts_with(tmp, prefix)) {
                        lines = b_slist_append(lines, b_strdup(tmp + strlen(prefix)));
                        state = CONTENT_BLOCKQUOTE_END;
                    }
                    else {
                        *err = blogc_error_parser(BLOGC_ERROR_CONTENT_PARSER, src, src_len,
                            current, "Malformed blockquote, must use same prefix "
                            "as previous line(s): %s", prefix);
                        free(prefix);
                        prefix = NULL;
                        b_slist_free_full(lines, free);
                        lines = NULL;
                    }
                    free(tmp);
                    tmp = NULL;
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
                break;

            case CONTENT_CODE_START:
                if (c == '\n' || c == '\r' || is_last) {
                    end = is_last && c != '\n' && c != '\r' ? src_len : current;
                    tmp = b_strndup(src + start, end - start);
                    if (b_str_starts_with(tmp, prefix)) {
                        lines = b_slist_append(lines, b_strdup(tmp + strlen(prefix)));
                        state = CONTENT_CODE_END;
                    }
                    else {
                        *err = blogc_error_parser(BLOGC_ERROR_CONTENT_PARSER, src, src_len,
                            current, "Malformed code block, must use same prefix "
                            "as previous line(s): '%s'", prefix);
                        free(prefix);
                        prefix = NULL;
                        b_slist_free_full(lines, free);
                        lines = NULL;
                    }
                    free(tmp);
                    tmp = NULL;
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

            case CONTENT_UNORDERED_LIST_OR_HORIZONTAL_RULE:
                if (c == d) {
                    state = CONTENT_HORIZONTAL_RULE;
                    break;
                }
                if (c == ' ' || c == '\t')
                    break;
                prefix = b_strndup(src + start, current - start);
                state = CONTENT_UNORDERED_LIST_START;
                break;

            case CONTENT_HORIZONTAL_RULE:
                if (c == d) {
                    break;
                }
                if (c == '\n' || c == '\r' || is_last) {
                    b_string_append(rv, "<hr />\n");
                    state = CONTENT_START_LINE;
                    start = current;
                    d = '\0';
                    break;
                }
                state = CONTENT_PARAGRAPH;
                break;

            case CONTENT_UNORDERED_LIST_START:
                if (c == '\n' || c == '\r' || is_last) {
                    end = is_last && c != '\n' && c != '\r' ? src_len : current;
                    tmp = b_strndup(src + start, end - start);
                    if (b_str_starts_with(tmp, prefix)) {
                        tmp3 = b_strdup(tmp + strlen(prefix));
                        parsed = blogc_content_parse_inline(tmp3);
                        free(tmp3);
                        tmp3 = NULL;
                        lines = b_slist_append(lines, b_strdup(parsed));
                        free(parsed);
                        parsed = NULL;
                    }
                    else {
                        *err = blogc_error_parser(BLOGC_ERROR_CONTENT_PARSER, src, src_len,
                            current, "Malformed unordered list, must use same prefix "
                            "as previous line(s): %s", prefix);
                    }
                    free(tmp);
                    tmp = NULL;
                    state = CONTENT_UNORDERED_LIST_END;
                }
                if (!is_last)
                    break;

            case CONTENT_UNORDERED_LIST_END:
                if (c == '\n' || c == '\r' || is_last) {
                    b_string_append(rv, "<ul>\n");
                    for (b_slist_t *l = lines; l != NULL; l = l->next)
                        b_string_append_printf(rv, "<li>%s</li>\n", l->data);
                    b_string_append(rv, "</ul>\n");
                    b_slist_free_full(lines, free);
                    lines = NULL;
                    free(prefix);
                    prefix = NULL;
                    state = CONTENT_START_LINE;
                    start = current;
                }
                else {
                    start = current;
                    state = CONTENT_UNORDERED_LIST_START;
                }
                break;

            case CONTENT_ORDERED_LIST:
                if (c >= '0' && c <= '9')
                    break;
                if (c == '.') {
                    state = CONTENT_ORDERED_LIST_SPACE;
                    break;
                }
                state = CONTENT_PARAGRAPH;
                break;

            case CONTENT_ORDERED_LIST_SPACE:
                if (c == ' ' || c == '\t')
                    break;
                prefix_len = current - start;
                state = CONTENT_ORDERED_LIST_START;
                break;

            case CONTENT_ORDERED_LIST_START:
                if (c == '\n' || c == '\r' || is_last) {
                    end = is_last && c != '\n' && c != '\r' ? src_len : current;
                    tmp = b_strndup(src + start, end - start);
                    if (strlen(tmp) >= prefix_len) {
                        tmp2 = b_strndup(tmp, prefix_len);
                        tmpv = b_str_split(tmp2, '.', 2);
                        free(tmp2);
                        tmp2 = NULL;
                        if (b_strv_length(tmpv) != 2) {
                            *err = blogc_error_parser(BLOGC_ERROR_CONTENT_PARSER, src, src_len,
                                current, "Malformed ordered list, prefix must be a "
                                "number, followed by a '.', followed by the content. "
                                "Content must be aligned with content from previous line(s)");
                            goto err_li;
                        }
                        for (unsigned int i = 0; tmpv[0][i] != '\0'; i++) {
                            if (!(tmpv[0][i] >= '0' && tmpv[0][i] <= '9')) {
                                *err = blogc_error_parser(BLOGC_ERROR_CONTENT_PARSER, src, src_len,
                                    current, "Malformed ordered list, prefix must be a "
                                    "number, followed by a '.', followed by the content. "
                                    "Content must be aligned with content from previous line(s)");
                                goto err_li;
                            }
                        }
                        for (unsigned int i = 0; tmpv[1][i] != '\0'; i++) {
                            if (!(tmpv[1][i] == ' ' || tmpv[1][i] == '\t')) {
                                *err = blogc_error_parser(BLOGC_ERROR_CONTENT_PARSER, src, src_len,
                                    current, "Malformed ordered list, prefix must be a "
                                    "number, followed by a '.', followed by the content. "
                                    "Content must be aligned with content from previous line(s)");
                                goto err_li;
                            }
                        }
                        tmp3 = b_strdup(tmp + prefix_len);
                        parsed = blogc_content_parse_inline(tmp3);
                        free(tmp3);
                        tmp3 = NULL;
                        lines = b_slist_append(lines, b_strdup(parsed));
                        state = CONTENT_ORDERED_LIST_END;
                        free(parsed);
                        parsed = NULL;
err_li:
                        b_strv_free(tmpv);
                        tmpv = NULL;
                    }
                    free(tmp);
                    tmp = NULL;
                }
                if (!is_last)
                    break;

            case CONTENT_ORDERED_LIST_END:
                if (c == '\n' || c == '\r' || is_last) {
                    b_string_append(rv, "<ol>\n");
                    for (b_slist_t *l = lines; l != NULL; l = l->next)
                        b_string_append_printf(rv, "<li>%s</li>\n", l->data);
                    b_string_append(rv, "</ol>\n");
                    b_slist_free_full(lines, free);
                    lines = NULL;
                    free(prefix);
                    prefix = NULL;
                    state = CONTENT_START_LINE;
                    start = current;
                }
                else {
                    start = current;
                    state = CONTENT_ORDERED_LIST_START;
                }
                break;

            case CONTENT_PARAGRAPH:
                if (c == '\n' || c == '\r' || is_last) {
                    state = CONTENT_PARAGRAPH_END;
                    end = is_last && c != '\n' && c != '\r' ? src_len : current;
                }
                if (!is_last)
                    break;

            case CONTENT_PARAGRAPH_END:
                if (c == '\n' || c == '\r' || is_last) {
                    tmp = b_strndup(src + start, end - start);
                    parsed = blogc_content_parse_inline(tmp);
                    b_string_append_printf(rv, "<p>%s</p>\n", parsed);
                    free(parsed);
                    parsed = NULL;
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
