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

// this is a half ass implementation of a markdown-like syntax. bugs are
// expected. feel free to improve the parser and and new features.


typedef enum {
    CONTENT_START_LINE = 1,
    CONTENT_EXCERPT,
    CONTENT_EXCERPT_END,
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


typedef enum {
    LINK_CLOSED = 1,
    LINK_IMAGE,
    LINK_TEXT,
    LINK_TEXT_CLOSE,
    LINK_URL,
    LINK_AUTO,
    LINK_AUTO_CLOSE,
} blogc_content_parser_link_state_t;


char*
blogc_content_parse_inline(const char *src)
{
    // this function is always called by blogc_content_parse or by itself,
    // then its safe to assume that src is always nul-terminated.
    size_t src_len = strlen(src);

    size_t current = 0;
    size_t start = 0;
    size_t start_state = 0;
    size_t end = 0;

    b_string_t *rv = b_string_new();

    bool open_em_ast = false;
    bool open_strong_ast = false;
    bool open_em_und = false;
    bool open_strong_und = false;
    bool open_code = false;
    bool open_code_double = false;

    blogc_content_parser_link_state_t state = LINK_CLOSED;
    bool is_image = false;

    char *tmp = NULL;
    char *tmp2 = NULL;

    unsigned int open_bracket = 0;
    unsigned int spaces = 0;

    bool escape = false;

    while (current < src_len) {
        char c = src[current];
        bool is_last = current == src_len - 1;

        if (escape) {
            if (state == LINK_CLOSED)
                b_string_append_c(rv, c);
            current++;
            escape = false;
            continue;
        }

        if (c != ' ' && c != '\n' && c != '\r')
            spaces = 0;

        switch (c) {

            case '\\':
                if (open_code || open_code_double) {
                    b_string_append_c(rv, c);
                    break;
                }
                if (!escape)
                    escape = true;
                break;

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
                        if (state == LINK_CLOSED)
                            b_string_append(rv, "</strong>");
                        if (c == '*')
                            open_strong_ast = false;
                        else
                            open_strong_und = false;
                        break;
                    }
                    if (state == LINK_CLOSED)
                        b_string_append(rv, "<strong>");
                    if (c == '*')
                        open_strong_ast = true;
                    else
                        open_strong_und = true;
                    break;
                }
                if ((c == '*' && open_em_ast) || (c == '_' && open_em_und)) {
                    if (state == LINK_CLOSED)
                        b_string_append(rv, "</em>");
                    if (c == '*')
                        open_em_ast = false;
                    else
                        open_em_und = false;
                    break;
                }
                if (state == LINK_CLOSED)
                    b_string_append(rv, "<em>");
                if (c == '*')
                    open_em_ast = true;
                else
                    open_em_und = true;
                break;

            case '`':
                if (!is_last && src[current + 1] == c) {
                    current++;
                    if (state == LINK_CLOSED)
                        b_string_append_printf(rv, "<%scode>",
                            open_code_double ? "/" : "");
                    open_code_double = !open_code_double;
                    break;
                }
                if (state == LINK_CLOSED)
                    b_string_append_printf(rv, "<%scode>", open_code ? "/" : "");
                open_code = !open_code;
                break;

            case '!':
                if (open_code || open_code_double) {
                    b_string_append_c(rv, c);
                    break;
                }
                if (state == LINK_CLOSED) {
                    state = LINK_IMAGE;
                    is_image = true;
                    start_state = current;
                }
                break;

            case '[':
                if (open_code || open_code_double) {
                    b_string_append_c(rv, c);
                    break;
                }
                if (state == LINK_CLOSED || state == LINK_IMAGE) {
                    state = LINK_TEXT;
                    start = current + 1;
                    open_bracket = 0;
                    if (state == LINK_CLOSED)
                        start_state = current;
                    break;
                }
                if (state == LINK_TEXT) {
                    if (current == start) {
                        start = current + 1;
                        state = LINK_AUTO;
                        break;
                    }
                    open_bracket++;
                    break;
                }
                break;

            case ']':
                if (open_code || open_code_double) {
                    b_string_append_c(rv, c);
                    break;
                }
                if (state == LINK_AUTO) {
                    end = current;
                    state = LINK_AUTO_CLOSE;
                    break;
                }
                if (state == LINK_AUTO_CLOSE) {
                    state = LINK_CLOSED;
                    tmp = b_strndup(src + start, end - start);
                    b_string_append_printf(rv, "<a href=\"%s\">%s</a>", tmp, tmp);
                    end = 0;
                    free(tmp);
                    tmp = NULL;
                    is_image = false;
                    break;
                }
                if (state == LINK_TEXT) {
                    if (open_bracket-- == 0) {
                        state = LINK_TEXT_CLOSE;
                        tmp = b_strndup(src + start, current - start);
                        tmp2 = blogc_content_parse_inline(tmp);
                        free(tmp);
                        tmp = NULL;
                    }
                    break;
                }
                if (state == LINK_CLOSED)
                    b_string_append_c(rv, c);
                break;

            case '(':
                if (open_code || open_code_double) {
                    b_string_append_c(rv, c);
                    break;
                }
                if (state == LINK_TEXT_CLOSE) {
                    state = LINK_URL;
                    start = current + 1;
                    break;
                }
                if (state == LINK_CLOSED)
                    b_string_append_c(rv, c);
                break;

            case ')':
                if (open_code || open_code_double) {
                    b_string_append_c(rv, c);
                    break;
                }
                if (state == LINK_URL) {
                    state = LINK_CLOSED;
                    tmp = b_strndup(src + start, current - start);
                    if (is_image)
                        b_string_append_printf(rv, "<img src=\"%s\" alt=\"%s\">",
                            tmp, tmp2);
                    else
                        b_string_append_printf(rv, "<a href=\"%s\">%s</a>",
                            tmp, tmp2);
                    free(tmp);
                    tmp = NULL;
                    free(tmp2);
                    tmp2 = NULL;
                    is_image = false;
                    break;
                }
                if (state == LINK_CLOSED)
                    b_string_append_c(rv, c);
                break;

            case ' ':
                if (state == LINK_CLOSED) {
                    spaces++;
                    b_string_append_c(rv, c);
                }
                if (!is_last)
                    break;

            case '\n':
            case '\r':
                if (state == LINK_CLOSED) {
                    if (spaces >= 2) {
                        b_string_append(rv, "<br />\n");
                        spaces = 0;
                    }
                    else if (c == '\n' || c == '\r')
                        b_string_append_c(rv, c);
                }
                break;

            case '&':
                if (state == LINK_CLOSED)
                    b_string_append(rv, "&amp;");
                break;

            case '<':
                if (state == LINK_CLOSED)
                    b_string_append(rv, "&lt;");
                break;

            case '>':
                if (state == LINK_CLOSED)
                    b_string_append(rv, "&gt;");
                break;

            case '"':
                if (state == LINK_CLOSED)
                    b_string_append(rv, "&quot;");
                break;

            case '\'':
                if (state == LINK_CLOSED)
                    b_string_append(rv, "&#x27;");
                break;

            case '/':
                if (state == LINK_CLOSED)
                    b_string_append(rv, "&#x2F;");
                break;

            default:
                if (state == LINK_CLOSED)
                    b_string_append_c(rv, c);
        }

        if (is_last && state != LINK_CLOSED) {
            b_string_append_c(rv, src[start_state]);
            tmp = blogc_content_parse_inline(src + start_state + 1);
            b_string_append(rv, tmp);
            free(tmp);
            tmp = NULL;
        }
        current++;
    }

    free(tmp);
    free(tmp2);

    return b_string_free(rv, false);
}


char*
blogc_content_parse(const char *src, size_t *end_excerpt)
{
    // src is always nul-terminated.
    size_t src_len = strlen(src);

    size_t current = 0;
    size_t start = 0;
    size_t start2 = 0;
    size_t end = 0;
    size_t eend = 0;

    unsigned int header_level = 0;
    char *prefix = NULL;
    size_t prefix_len = 0;
    char *tmp = NULL;
    char *tmp2 = NULL;
    char *tmp3 = NULL;
    char *parsed = NULL;
    char **tmpv = NULL;

    char d = '\0';

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
                start = current;
                if (c == '.') {
                    if (end_excerpt != NULL) {
                        eend = rv->len;  // fuck it
                        state = CONTENT_EXCERPT;
                        break;
                    }
                }
                if (c == '#') {
                    header_level = 1;
                    state = CONTENT_HEADER;
                    break;
                }
                if (c == '*' || c == '+' || c == '-') {
                    start2 = current;
                    state = CONTENT_UNORDERED_LIST_OR_HORIZONTAL_RULE;
                    d = c;
                    break;
                }
                if (c >= '0' && c <= '9') {
                    start2 = current;
                    state = CONTENT_ORDERED_LIST;
                    break;
                }
                if (c == ' ' || c == '\t') {
                    start2 = current;
                    state = CONTENT_CODE;
                    break;
                }
                if (c == '<') {
                    state = CONTENT_HTML;
                    break;
                }
                if (c == '>') {
                    state = CONTENT_BLOCKQUOTE;
                    start2 = current;
                    break;
                }
                state = CONTENT_PARAGRAPH;
                break;

            case CONTENT_EXCERPT:
                if (end_excerpt != NULL) {
                    if (c == '.')
                        break;
                    if (c == '\n' || c == '\r') {
                        //*end_excerpt = eend;
                        //state = CONTENT_START_LINE;
                        state = CONTENT_EXCERPT_END;
                        break;
                    }
                }
                eend = 0;
                state = CONTENT_PARAGRAPH;
                break;

            case CONTENT_EXCERPT_END:
                if (end_excerpt != NULL) {
                    if (c == '\n' || c == '\r') {
                        *end_excerpt = eend;
                        state = CONTENT_START_LINE;
                        break;
                    }
                }
                eend = 0;
                state = CONTENT_PARAGRAPH_END;
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
                state = CONTENT_PARAGRAPH;
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
                    tmp = b_strndup(src + start2, end - start2);
                    if (b_str_starts_with(tmp, prefix)) {
                        lines = b_slist_append(lines, b_strdup(tmp + strlen(prefix)));
                        state = CONTENT_BLOCKQUOTE_END;
                    }
                    else {
                        state = CONTENT_PARAGRAPH;
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
                    tmp = blogc_content_parse(tmp_str->str, NULL);
                    b_string_append_printf(rv, "<blockquote>%s</blockquote>\n",
                        tmp);
                    free(tmp);
                    tmp = NULL;
                    b_string_free(tmp_str, true);
                    tmp_str = NULL;
                    b_slist_free_full(lines, free);
                    lines = NULL;
                    free(prefix);
                    prefix = NULL;
                    state = CONTENT_START_LINE;
                    start2 = current;
                }
                else {
                    start2 = current;
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
                    tmp = b_strndup(src + start2, end - start2);
                    if (b_str_starts_with(tmp, prefix)) {
                        lines = b_slist_append(lines, b_strdup(tmp + strlen(prefix)));
                        state = CONTENT_CODE_END;
                    }
                    else {
                        state = CONTENT_PARAGRAPH;
                        free(prefix);
                        prefix = NULL;
                        b_slist_free_full(lines, free);
                        lines = NULL;
                        free(tmp);
                        tmp = NULL;
                        break;
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
                    start2 = current;
                }
                else {
                    start2 = current;
                    state = CONTENT_CODE_START;
                }
                break;

            case CONTENT_UNORDERED_LIST_OR_HORIZONTAL_RULE:
                if (c == d) {
                    if (is_last)
                        goto hr;
                    state = CONTENT_HORIZONTAL_RULE;
                    break;
                }
                if (c == ' ' || c == '\t')
                    break;
                prefix = b_strndup(src + start, current - start);
                state = CONTENT_UNORDERED_LIST_START;
                break;

            case CONTENT_HORIZONTAL_RULE:
                if (c == d && !is_last) {
                    break;
                }
hr:
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
                    tmp = b_strndup(src + start2, end - start2);
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
                        state = CONTENT_PARAGRAPH_END;
                        free(tmp);
                        tmp = NULL;
                        free(prefix);
                        prefix = NULL;
                        b_slist_free_full(lines, free);
                        lines = NULL;
                        if (is_last)
                            goto para;
                        break;
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
                    start2 = current;
                }
                else {
                    start2 = current;
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
                if (c != '\n' && c != '\r' && !is_last)
                    break;

            case CONTENT_ORDERED_LIST_START:
                if (c == '\n' || c == '\r' || is_last) {
                    end = is_last && c != '\n' && c != '\r' ? src_len : current;
                    tmp = b_strndup(src + start2, end - start2);
                    if (strlen(tmp) >= prefix_len) {
                        tmp2 = b_strndup(tmp, prefix_len);
                        tmpv = b_str_split(tmp2, '.', 2);
                        free(tmp2);
                        tmp2 = NULL;
                        if (b_strv_length(tmpv) != 2) {
                            state = CONTENT_PARAGRAPH_END;
                            b_strv_free(tmpv);
                            tmpv = NULL;
                            free(tmp);
                            tmp = NULL;
                            b_slist_free_full(lines, free);
                            lines = NULL;
                            goto para;
                        }
                        for (unsigned int i = 0; tmpv[0][i] != '\0'; i++) {
                            if (!(tmpv[0][i] >= '0' && tmpv[0][i] <= '9')) {
                                state = CONTENT_PARAGRAPH_END;
                                b_strv_free(tmpv);
                                tmpv = NULL;
                                free(tmp);
                                tmp = NULL;
                                b_slist_free_full(lines, free);
                                lines = NULL;
                                goto para;
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
                        b_strv_free(tmpv);
                        tmpv = NULL;
                    }
                    free(tmp);
                    tmp = NULL;
                }
                if (state == CONTENT_PARAGRAPH || !is_last)
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
                    start2 = current;
                }
                else {
                    start2 = current;
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
para:
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

        current++;
    }

    return b_string_free(rv, false);
}
