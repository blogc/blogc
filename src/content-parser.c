/*
 * blogc: A blog compiler.
 * Copyright (C) 2015-2016 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdbool.h>
#include <string.h>

#include "content-parser.h"
#include "directives.h"
#include "utils.h"

// this is a half ass implementation of a markdown-like syntax. bugs are
// expected. feel free to improve the parser and add new features.


char*
blogc_slugify(const char *str)
{
    if (str == NULL)
        return NULL;
    char *new_str = sb_strdup(str);
    int diff = 'a' - 'A';  // just to avoid magic numbers
    for (size_t i = 0; new_str[i] != '\0'; i++) {
        if (new_str[i] >= 'a' && new_str[i] <= 'z')
            continue;
        if (new_str[i] >= '0' && new_str[i] <= '9')
            continue;
        if (new_str[i] >= 'A' && new_str[i] <= 'Z')
            new_str[i] += diff;
        else
            new_str[i] = '-';
    }
    return new_str;
}


char*
blogc_htmlentities(const char *str)
{
    if (str == NULL)
        return NULL;
    sb_string_t *rv = sb_string_new();
    for (size_t i = 0; str[i] != '\0'; i++) {
        switch (str[i]) {
            case '&':
                sb_string_append(rv, "&amp;");
                break;
            case '<':
                sb_string_append(rv, "&lt;");
                break;
            case '>':
                sb_string_append(rv, "&gt;");
                break;
            case '"':
                sb_string_append(rv, "&quot;");
                break;
            case '\'':
                sb_string_append(rv, "&#x27;");
                break;
            case '/':
                sb_string_append(rv, "&#x2F;");
                break;
            default:
                sb_string_append_c(rv, str[i]);
        }
    }
    return sb_string_free(rv, false);
}


typedef enum {
    CONTENT_START_LINE = 1,
    CONTENT_EXCERPT_OR_DIRECTIVE,
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
    CONTENT_DIRECTIVE_NAME_START,
    CONTENT_DIRECTIVE_NAME,
    CONTENT_DIRECTIVE_COLON,
    CONTENT_DIRECTIVE_ARGUMENT_START,
    CONTENT_DIRECTIVE_ARGUMENT,
    CONTENT_DIRECTIVE_PARAM_PREFIX_START,
    CONTENT_DIRECTIVE_PARAM_PREFIX,
    CONTENT_DIRECTIVE_PARAM_KEY_START,
    CONTENT_DIRECTIVE_PARAM_KEY,
    CONTENT_DIRECTIVE_PARAM_VALUE_START,
    CONTENT_DIRECTIVE_PARAM_VALUE,
    CONTENT_DIRECTIVE_PARAM_END,
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

    sb_string_t *rv = sb_string_new();

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
                sb_string_append_c(rv, c);
            current++;
            escape = false;
            continue;
        }

        if (c != ' ' && c != '\n' && c != '\r')
            spaces = 0;

        if (state == LINK_TEXT_CLOSE && c != ' ' && c != '\n' && c != '\r' &&
            c != '(')
        {
            sb_string_append_c(rv, src[start_state]);
            tmp = blogc_content_parse_inline(src + start_state + 1);
            sb_string_append(rv, tmp);
            // no need to free here, we will exit the loop!
            break;
        }

        switch (c) {

            case '\\':
                if (state == LINK_CLOSED && (open_code || open_code_double)) {
                    sb_string_append_c(rv, c);
                    break;
                }
                if (!escape)
                    escape = true;
                break;

            case '*':
            case '_':
                if (state == LINK_CLOSED && (open_code || open_code_double)) {
                    sb_string_append_c(rv, c);
                    break;
                }
                if (!is_last && src[current + 1] == c) {
                    current++;
                    if ((c == '*' && open_strong_ast) ||
                        (c == '_' && open_strong_und))
                    {
                        if (state == LINK_CLOSED)
                            sb_string_append(rv, "</strong>");
                        if (c == '*')
                            open_strong_ast = false;
                        else
                            open_strong_und = false;
                        break;
                    }
                    if (state == LINK_CLOSED)
                        sb_string_append(rv, "<strong>");
                    if (c == '*')
                        open_strong_ast = true;
                    else
                        open_strong_und = true;
                    break;
                }
                if ((c == '*' && open_em_ast) || (c == '_' && open_em_und)) {
                    if (state == LINK_CLOSED)
                        sb_string_append(rv, "</em>");
                    if (c == '*')
                        open_em_ast = false;
                    else
                        open_em_und = false;
                    break;
                }
                if (state == LINK_CLOSED)
                    sb_string_append(rv, "<em>");
                if (c == '*')
                    open_em_ast = true;
                else
                    open_em_und = true;
                break;

            case '`':
                if (!is_last && src[current + 1] == c) {
                    current++;
                    if (state == LINK_CLOSED)
                        sb_string_append_printf(rv, "<%scode>",
                            open_code_double ? "/" : "");
                    open_code_double = !open_code_double;
                    break;
                }
                if (state == LINK_CLOSED)
                    sb_string_append_printf(rv, "<%scode>", open_code ? "/" : "");
                open_code = !open_code;
                break;

            case '!':
                if (state == LINK_CLOSED) {
                    if (open_code || open_code_double) {
                        sb_string_append_c(rv, c);
                        break;
                    }
                    if (!is_last && src[current + 1] != '[') {
                        sb_string_append_c(rv, c);
                        break;
                    }
                    state = LINK_IMAGE;
                    is_image = true;
                    start_state = current;
                }
                break;

            case '[':
                if (state == LINK_CLOSED && (open_code || open_code_double)) {
                    sb_string_append_c(rv, c);
                    break;
                }
                if (state == LINK_CLOSED || state == LINK_IMAGE) {
                    if (state == LINK_CLOSED)
                        start_state = current;
                    state = LINK_TEXT;
                    start = current + 1;
                    open_bracket = 0;
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
                if (state == LINK_AUTO) {
                    end = current;
                    state = LINK_AUTO_CLOSE;
                    break;
                }
                if (state == LINK_AUTO_CLOSE) {
                    state = LINK_CLOSED;
                    tmp = sb_strndup(src + start, end - start);
                    sb_string_append_printf(rv, "<a href=\"%s\">%s</a>", tmp, tmp);
                    end = 0;
                    free(tmp);
                    tmp = NULL;
                    is_image = false;
                    break;
                }
                if (state == LINK_TEXT) {
                    if (open_bracket-- == 0) {
                        state = LINK_TEXT_CLOSE;
                        tmp = sb_strndup(src + start, current - start);
                        tmp2 = blogc_content_parse_inline(tmp);
                        free(tmp);
                        tmp = NULL;
                    }
                    break;
                }
                if (state == LINK_CLOSED)
                    sb_string_append_c(rv, c);
                break;

            case '(':
                if (state == LINK_TEXT_CLOSE) {
                    state = LINK_URL;
                    start = current + 1;
                    break;
                }
                if (state == LINK_CLOSED)
                    sb_string_append_c(rv, c);
                break;

            case ')':
                if (state == LINK_URL) {
                    state = LINK_CLOSED;
                    tmp = sb_strndup(src + start, current - start);
                    if (is_image)
                        sb_string_append_printf(rv, "<img src=\"%s\" alt=\"%s\">",
                            tmp, tmp2);
                    else
                        sb_string_append_printf(rv, "<a href=\"%s\">%s</a>",
                            tmp, tmp2);
                    free(tmp);
                    tmp = NULL;
                    free(tmp2);
                    tmp2 = NULL;
                    is_image = false;
                    break;
                }
                if (state == LINK_CLOSED)
                    sb_string_append_c(rv, c);
                break;

            case ' ':
                if (state == LINK_CLOSED) {
                    spaces++;
                    sb_string_append_c(rv, c);
                }
                if (!is_last)
                    break;

            case '\n':
            case '\r':
                if (state == LINK_CLOSED) {
                    if (spaces >= 2) {
                        sb_string_append(rv, "<br />");
                        spaces = 0;
                    }
                    if (c == '\n' || c == '\r')
                        sb_string_append_c(rv, c);
                }
                break;

            case '-':
                if (state != LINK_CLOSED)
                    break;
                if ((current < (src_len - 1) && src[current + 1] == '-') &&
                    !(open_code || open_code_double))
                {
                    if (current < (src_len - 2) && src[current + 2] == '-') {
                        sb_string_append(rv, "&mdash;");
                        current += 2;
                    }
                    else {
                        sb_string_append(rv, "&ndash;");
                        current += 1;
                    }
                }
                else {
                    sb_string_append_c(rv, c);
                }
                break;

            case '&':
                if (state == LINK_CLOSED)
                    sb_string_append(rv, "&amp;");
                break;

            case '<':
                if (state == LINK_CLOSED)
                    sb_string_append(rv, "&lt;");
                break;

            case '>':
                if (state == LINK_CLOSED)
                    sb_string_append(rv, "&gt;");
                break;

            case '"':
                if (state == LINK_CLOSED)
                    sb_string_append(rv, "&quot;");
                break;

            case '\'':
                if (state == LINK_CLOSED)
                    sb_string_append(rv, "&#x27;");
                break;

            case '/':
                if (state == LINK_CLOSED)
                    sb_string_append(rv, "&#x2F;");
                break;

            default:
                if (state == LINK_CLOSED)
                    sb_string_append_c(rv, c);
        }

        if (is_last && state != LINK_CLOSED) {
            sb_string_append_c(rv, src[start_state]);
            tmp = blogc_content_parse_inline(src + start_state + 1);
            sb_string_append(rv, tmp);
            // no need to free here, its the last iteration
        }
        current++;
    }

    free(tmp);
    free(tmp2);

    return sb_string_free(rv, false);
}


bool
blogc_is_ordered_list_item(const char *str, size_t prefix_len)
{
    if (str == NULL)
        return false;

    if (strlen(str) < 2)
        return false;

    size_t i;

    for (i = 0; str[i] >= '0' && str[i] <= '9'; i++);

    if (i == 0)
        return false;
    if (str[i] != '.')
        return false;

    for (i++; i < prefix_len && (str[i] == ' ' || str[i] == '\t'); i++);

    if (str[i] == '\0')
        return false;

    return i == prefix_len;
}


char*
blogc_content_parse(const char *src, size_t *end_excerpt, char **description)
{
    // src is always nul-terminated.
    size_t src_len = strlen(src);

    size_t current = 0;
    size_t start = 0;
    size_t start2 = 0;
    size_t end = 0;
    size_t eend = 0;
    size_t real_end = 0;

    unsigned int header_level = 0;
    char *prefix = NULL;
    size_t prefix_len = 0;
    char *tmp = NULL;
    char *tmp2 = NULL;
    char *parsed = NULL;
    char *slug = NULL;

    char *directive_name = NULL;
    char *directive_argument = NULL;
    char *directive_key = NULL;
    sb_trie_t *directive_params = NULL;

    // this isn't empty because we need some reasonable default value in the
    // unlikely case that we need to print some line ending before evaluating
    // the "real" value.
    char line_ending[3] = "\n";
    bool line_ending_found = false;

    char d = '\0';

    sb_slist_t *lines = NULL;
    sb_slist_t *lines2 = NULL;

    sb_string_t *rv = sb_string_new();
    sb_string_t *tmp_str = NULL;

    blogc_content_parser_state_t state = CONTENT_START_LINE;

    while (current < src_len) {
        char c = src[current];
        bool is_last = current == src_len - 1;

        if (c == '\n' || c == '\r') {
            if ((current + 1) < src_len) {
                if ((c == '\n' && src[current + 1] == '\r') ||
                    (c == '\r' && src[current + 1] == '\n'))
                {
                    if (!line_ending_found) {
                        line_ending[0] = c;
                        line_ending[1] = src[current + 1];
                        line_ending[2] = '\0';
                        line_ending_found = true;
                    }
                    real_end = current;
                    c = src[++current];
                    is_last = current == src_len - 1;
                }
            }
            if (!line_ending_found) {
                line_ending[0] = c;
                line_ending[1] = '\0';
                line_ending_found = true;
            }
        }

        switch (state) {

            case CONTENT_START_LINE:
                if (c == '\n' || c == '\r' || is_last)
                    break;
                start = current;
                if (c == '.') {
                    eend = rv->len;  // fuck it
                    state = CONTENT_EXCERPT_OR_DIRECTIVE;
                    break;
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

            case CONTENT_EXCERPT_OR_DIRECTIVE:
                if (c == '.')
                    break;
                if ((c == ' ' || c == '\t') && current - start == 2) {
                    state = CONTENT_DIRECTIVE_NAME_START;
                    if (is_last) {
                        state = CONTENT_PARAGRAPH;
                        continue;
                    }
                    break;
                }
                if (c == '\n' || c == '\r') {
                    state = CONTENT_EXCERPT_END;
                    break;
                }
                eend = 0;
                state = CONTENT_PARAGRAPH;
                break;

            case CONTENT_EXCERPT_END:
                if (c == '\n' || c == '\r') {
                    if (end_excerpt != NULL)
                        *end_excerpt = eend;
                    state = CONTENT_START_LINE;
                    break;
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
                    end = is_last && c != '\n' && c != '\r' ? src_len :
                        (real_end != 0 ? real_end : current);
                    tmp = sb_strndup(src + start, end - start);
                    parsed = blogc_content_parse_inline(tmp);
                    slug = blogc_slugify(tmp);
                    if (slug == NULL)
                        sb_string_append_printf(rv, "<h%d>%s</h%d>%s",
                            header_level, parsed, header_level, line_ending);
                    else
                        sb_string_append_printf(rv, "<h%d id=\"%s\">%s</h%d>%s",
                            header_level, slug, parsed, header_level,
                            line_ending);
                    free(slug);
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
                    end = is_last && c != '\n' && c != '\r' ? src_len :
                        (real_end != 0 ? real_end : current);
                }
                if (!is_last)
                    break;

            case CONTENT_HTML_END:
                if (c == '\n' || c == '\r' || is_last) {
                    tmp = sb_strndup(src + start, end - start);
                    sb_string_append_printf(rv, "%s%s", tmp, line_ending);
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
                prefix = sb_strndup(src + start, current - start);
                state = CONTENT_BLOCKQUOTE_START;
                break;

            case CONTENT_BLOCKQUOTE_START:
                if (c == '\n' || c == '\r' || is_last) {
                    end = is_last && c != '\n' && c != '\r' ? src_len :
                        (real_end != 0 ? real_end : current);
                    tmp = sb_strndup(src + start2, end - start2);
                    if (sb_str_starts_with(tmp, prefix)) {
                        lines = sb_slist_append(lines, sb_strdup(tmp + strlen(prefix)));
                        state = CONTENT_BLOCKQUOTE_END;
                    }
                    else {
                        state = CONTENT_PARAGRAPH;
                        free(prefix);
                        prefix = NULL;
                        sb_slist_free_full(lines, free);
                        lines = NULL;
                        if (is_last) {
                            free(tmp);
                            tmp = NULL;
                            continue;
                        }
                    }
                    free(tmp);
                    tmp = NULL;
                }
                if (!is_last)
                    break;

            case CONTENT_BLOCKQUOTE_END:
                if (c == '\n' || c == '\r' || is_last) {
                    tmp_str = sb_string_new();
                    for (sb_slist_t *l = lines; l != NULL; l = l->next)
                        sb_string_append_printf(tmp_str, "%s%s", l->data,
                            line_ending);
                    tmp = blogc_content_parse(tmp_str->str, NULL, description);
                    sb_string_append_printf(rv, "<blockquote>%s</blockquote>%s",
                        tmp, line_ending);
                    free(tmp);
                    tmp = NULL;
                    sb_string_free(tmp_str, true);
                    tmp_str = NULL;
                    sb_slist_free_full(lines, free);
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
                prefix = sb_strndup(src + start, current - start);
                state = CONTENT_CODE_START;
                break;

            case CONTENT_CODE_START:
                if (c == '\n' || c == '\r' || is_last) {
                    end = is_last && c != '\n' && c != '\r' ? src_len :
                        (real_end != 0 ? real_end : current);
                    tmp = sb_strndup(src + start2, end - start2);
                    if (sb_str_starts_with(tmp, prefix)) {
                        lines = sb_slist_append(lines, sb_strdup(tmp + strlen(prefix)));
                        state = CONTENT_CODE_END;
                    }
                    else {
                        state = CONTENT_PARAGRAPH;
                        free(prefix);
                        prefix = NULL;
                        sb_slist_free_full(lines, free);
                        lines = NULL;
                        free(tmp);
                        tmp = NULL;
                        if (is_last)
                            continue;
                        break;
                    }
                    free(tmp);
                    tmp = NULL;
                }
                if (!is_last)
                    break;

            case CONTENT_CODE_END:
                if (c == '\n' || c == '\r' || is_last) {
                    sb_string_append(rv, "<pre><code>");
                    for (sb_slist_t *l = lines; l != NULL; l = l->next) {
                        char *tmp_line = blogc_htmlentities(l->data);
                        if (l->next == NULL)
                            sb_string_append_printf(rv, "%s", tmp_line);
                        else
                            sb_string_append_printf(rv, "%s%s", tmp_line,
                                line_ending);
                        free(tmp_line);
                    }
                    sb_string_append_printf(rv, "</code></pre>%s", line_ending);
                    sb_slist_free_full(lines, free);
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
                    state = CONTENT_HORIZONTAL_RULE;
                    if (is_last)
                        continue;
                    break;
                }
                if (c == ' ' || c == '\t')
                    break;
                prefix = sb_strndup(src + start, current - start);
                state = CONTENT_UNORDERED_LIST_START;
                break;

            case CONTENT_HORIZONTAL_RULE:
                if (c == d && !is_last) {
                    break;
                }
                if (c == '\n' || c == '\r' || is_last) {
                    sb_string_append_printf(rv, "<hr />%s", line_ending);
                    state = CONTENT_START_LINE;
                    start = current;
                    d = '\0';
                    break;
                }
                state = CONTENT_PARAGRAPH;
                break;

            case CONTENT_UNORDERED_LIST_START:
                if (c == '\n' || c == '\r' || is_last) {
                    end = is_last && c != '\n' && c != '\r' ? src_len :
                        (real_end != 0 ? real_end : current);
                    tmp = sb_strndup(src + start2, end - start2);
                    tmp2 = sb_strdup_printf("%-*s", strlen(prefix), "");
                    if (sb_str_starts_with(tmp, prefix)) {
                        if (lines2 != NULL) {
                            tmp_str = sb_string_new();
                            for (sb_slist_t *l = lines2; l != NULL; l = l->next) {
                                if (l->next == NULL)
                                    sb_string_append_printf(tmp_str, "%s", l->data);
                                else
                                    sb_string_append_printf(tmp_str, "%s%s", l->data,
                                        line_ending);
                            }
                            sb_slist_free_full(lines2, free);
                            lines2 = NULL;
                            parsed = blogc_content_parse_inline(tmp_str->str);
                            sb_string_free(tmp_str, true);
                            lines = sb_slist_append(lines, sb_strdup(parsed));
                            free(parsed);
                            parsed = NULL;
                        }
                        lines2 = sb_slist_append(lines2, sb_strdup(tmp + strlen(prefix)));
                    }
                    else if (sb_str_starts_with(tmp, tmp2)) {
                        lines2 = sb_slist_append(lines2, sb_strdup(tmp + strlen(prefix)));
                    }
                    else {
                        state = CONTENT_PARAGRAPH_END;
                        free(tmp);
                        tmp = NULL;
                        free(tmp2);
                        tmp2 = NULL;
                        free(prefix);
                        prefix = NULL;
                        sb_slist_free_full(lines, free);
                        sb_slist_free_full(lines2, free);
                        lines = NULL;
                        if (is_last)
                            continue;
                        break;
                    }
                    free(tmp);
                    tmp = NULL;
                    free(tmp2);
                    tmp2 = NULL;
                    state = CONTENT_UNORDERED_LIST_END;
                }
                if (!is_last)
                    break;

            case CONTENT_UNORDERED_LIST_END:
                if (c == '\n' || c == '\r' || is_last) {
                    if (lines2 != NULL) {
                        // FIXME: avoid repeting the code below
                        tmp_str = sb_string_new();
                        for (sb_slist_t *l = lines2; l != NULL; l = l->next) {
                            if (l->next == NULL)
                                sb_string_append_printf(tmp_str, "%s", l->data);
                            else
                                sb_string_append_printf(tmp_str, "%s%s", l->data,
                                    line_ending);
                        }
                        sb_slist_free_full(lines2, free);
                        lines2 = NULL;
                        parsed = blogc_content_parse_inline(tmp_str->str);
                        sb_string_free(tmp_str, true);
                        lines = sb_slist_append(lines, sb_strdup(parsed));
                        free(parsed);
                        parsed = NULL;
                    }
                    sb_string_append_printf(rv, "<ul>%s", line_ending);
                    for (sb_slist_t *l = lines; l != NULL; l = l->next)
                        sb_string_append_printf(rv, "<li>%s</li>%s", l->data,
                            line_ending);
                    sb_string_append_printf(rv, "</ul>%s", line_ending);
                    sb_slist_free_full(lines, free);
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
                if (is_last)
                    continue;
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
                    end = is_last && c != '\n' && c != '\r' ? src_len :
                        (real_end != 0 ? real_end : current);
                    tmp = sb_strndup(src + start2, end - start2);
                    tmp2 = sb_strdup_printf("%-*s", prefix_len, "");
                    if (blogc_is_ordered_list_item(tmp, prefix_len)) {
                        if (lines2 != NULL) {
                            tmp_str = sb_string_new();
                            for (sb_slist_t *l = lines2; l != NULL; l = l->next) {
                                if (l->next == NULL)
                                    sb_string_append_printf(tmp_str, "%s", l->data);
                                else
                                    sb_string_append_printf(tmp_str, "%s%s", l->data,
                                        line_ending);
                            }
                            sb_slist_free_full(lines2, free);
                            lines2 = NULL;
                            parsed = blogc_content_parse_inline(tmp_str->str);
                            sb_string_free(tmp_str, true);
                            lines = sb_slist_append(lines, sb_strdup(parsed));
                            free(parsed);
                            parsed = NULL;
                        }
                        lines2 = sb_slist_append(lines2, sb_strdup(tmp + prefix_len));
                    }
                    else if (sb_str_starts_with(tmp, tmp2)) {
                        lines2 = sb_slist_append(lines2, sb_strdup(tmp + prefix_len));
                    }
                    else {
                        state = CONTENT_PARAGRAPH_END;
                        free(tmp);
                        tmp = NULL;
                        free(tmp2);
                        tmp2 = NULL;
                        free(parsed);
                        parsed = NULL;
                        sb_slist_free_full(lines, free);
                        sb_slist_free_full(lines2, free);
                        lines = NULL;
                        if (is_last)
                            continue;
                        break;
                    }
                    free(tmp);
                    tmp = NULL;
                    free(tmp2);
                    tmp2 = NULL;
                    state = CONTENT_ORDERED_LIST_END;
                }
                if (!is_last)
                    break;

            case CONTENT_ORDERED_LIST_END:
                if (c == '\n' || c == '\r' || is_last) {
                    if (lines2 != NULL) {
                        // FIXME: avoid repeting the code below
                        tmp_str = sb_string_new();
                        for (sb_slist_t *l = lines2; l != NULL; l = l->next) {
                            if (l->next == NULL)
                                sb_string_append_printf(tmp_str, "%s", l->data);
                            else
                                sb_string_append_printf(tmp_str, "%s%s", l->data,
                                    line_ending);
                        }
                        sb_slist_free_full(lines2, free);
                        lines2 = NULL;
                        parsed = blogc_content_parse_inline(tmp_str->str);
                        sb_string_free(tmp_str, true);
                        lines = sb_slist_append(lines, sb_strdup(parsed));
                        free(parsed);
                        parsed = NULL;
                    }
                    sb_string_append_printf(rv, "<ol>%s", line_ending);
                    for (sb_slist_t *l = lines; l != NULL; l = l->next)
                        sb_string_append_printf(rv, "<li>%s</li>%s", l->data,
                            line_ending);
                    sb_string_append_printf(rv, "</ol>%s", line_ending);
                    sb_slist_free_full(lines, free);
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

            case CONTENT_DIRECTIVE_NAME_START:
                if (is_last) {
                    state = CONTENT_PARAGRAPH;
                    continue;
                }
                if (c >= 'a' && c <= 'z') {
                    start2 = current;
                    state = CONTENT_DIRECTIVE_NAME;
                    break;
                }
                state = CONTENT_PARAGRAPH;
                break;

            case CONTENT_DIRECTIVE_NAME:
                if (is_last) {
                    state = CONTENT_PARAGRAPH;
                    continue;
                }
                if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_')
                    break;
                if (c == ':') {
                    end = current;
                    state = CONTENT_DIRECTIVE_COLON;
                    break;
                }
                state = CONTENT_PARAGRAPH;
                break;

            case CONTENT_DIRECTIVE_COLON:
                if (c == ':') {
                    free(directive_name);
                    directive_name = sb_strndup(src + start2, end - start2);
                    state = CONTENT_DIRECTIVE_ARGUMENT_START;
                    if (is_last) {
                        state = CONTENT_DIRECTIVE_PARAM_END;
                        continue;
                    }
                    break;
                }
                if (is_last) {
                    state = CONTENT_PARAGRAPH;
                    continue;
                }
                state = CONTENT_PARAGRAPH;
                break;

            case CONTENT_DIRECTIVE_ARGUMENT_START:
                if (c == ' ' || c == '\t') {
                    if (is_last) {
                        state = CONTENT_DIRECTIVE_PARAM_END;
                        continue;
                    }
                    break;
                }
                if (c == '\n' || c == '\r' || is_last) {
                    state = CONTENT_DIRECTIVE_PARAM_PREFIX_START;
                    directive_argument = NULL;
                    if (is_last) {
                        state = CONTENT_DIRECTIVE_PARAM_END;
                        continue;
                    }
                    else
                        start2 = current + 1;
                    break;
                }
                start2 = current;
                state = CONTENT_DIRECTIVE_ARGUMENT;
                break;

            case CONTENT_DIRECTIVE_ARGUMENT:
                if (c == '\n' || c == '\r' || is_last) {
                    state = CONTENT_DIRECTIVE_PARAM_PREFIX_START;
                    end = is_last && c != '\n' && c != '\r' ? src_len :
                        (real_end != 0 ? real_end : current);
                    free(directive_argument);
                    directive_argument = sb_strndup(src + start2, end - start2);
                    if (is_last) {
                        state = CONTENT_DIRECTIVE_PARAM_END;
                        continue;
                    }
                    else
                        start2 = current + 1;
                }
                break;

            case CONTENT_DIRECTIVE_PARAM_PREFIX_START:
                if (is_last) {
                    state = CONTENT_PARAGRAPH;
                    continue;
                }
                if (c == ' ' || c == '\t')
                    break;
                if (c == '\n' || c == '\r') {
                    state = CONTENT_DIRECTIVE_PARAM_END;
                    continue;
                }
                prefix = sb_strndup(src + start2, current - start2);
                state = CONTENT_DIRECTIVE_PARAM_PREFIX;
                current--;
                break;

            case CONTENT_DIRECTIVE_PARAM_PREFIX:
                if (c == ' ' || c == '\t')
                    break;
                if (c == ':' && sb_str_starts_with(src + start2, prefix)) {
                    state = CONTENT_DIRECTIVE_PARAM_KEY_START;
                    break;
                }
                state = CONTENT_PARAGRAPH;
                if (is_last)
                    continue;
                break;

            case CONTENT_DIRECTIVE_PARAM_KEY_START:
                if (is_last) {
                    state = CONTENT_PARAGRAPH;
                    continue;
                }
                if (c >= 'a' && c <= 'z') {
                    start2 = current;
                    state = CONTENT_DIRECTIVE_PARAM_KEY;
                    break;
                }
                state = CONTENT_PARAGRAPH;
                break;

            case CONTENT_DIRECTIVE_PARAM_KEY:
                if (is_last) {
                    state = CONTENT_PARAGRAPH;
                    continue;
                }
                if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_')
                    break;
                if (c == ':') {
                    free(directive_key);
                    directive_key = sb_strndup(src + start2, current - start2);
                    state = CONTENT_DIRECTIVE_PARAM_VALUE_START;
                    break;
                }
                state = CONTENT_PARAGRAPH;
                break;

            case CONTENT_DIRECTIVE_PARAM_VALUE_START:
                if (is_last) {
                    state = CONTENT_PARAGRAPH;
                    continue;
                }
                if (c == ' ' || c == '\t')
                    break;
                start2 = current;
                state = CONTENT_DIRECTIVE_PARAM_VALUE;
                break;

            case CONTENT_DIRECTIVE_PARAM_VALUE:
                if (c == '\n' || c == '\r' || is_last) {
                    state = CONTENT_DIRECTIVE_PARAM_END;
                    end = is_last && c != '\n' && c != '\r' ? src_len :
                        (real_end != 0 ? real_end : current);
                    if (directive_params == NULL)
                        directive_params = sb_trie_new(free);
                    sb_trie_insert(directive_params, directive_key,
                        sb_strndup(src + start2, end - start2));
                    free(directive_key);
                    directive_key = NULL;
                    if (!is_last)
                        start2 = current + 1;
                }
                if (!is_last)
                    break;

            case CONTENT_DIRECTIVE_PARAM_END:
                if (c == '\n' || c == '\r' || is_last) {
                    // FIXME: handle errors in the rest of the parser.
                    blogc_error_t *err = NULL;
                    blogc_directive_ctx_t *ctx = sb_malloc(
                        sizeof(blogc_directive_ctx_t));
                    ctx->name = directive_name;
                    ctx->argument = directive_argument;
                    ctx->params = directive_params;
                    ctx->eol = line_ending;
                    char *rv_d = blogc_directive_loader(ctx, &err);
                    free(ctx);
                    blogc_error_print(err);
                    if (rv_d != NULL)
                        sb_string_append(rv, rv_d);
                    free(rv_d);
                    state = CONTENT_START_LINE;
                    start = current;
                    free(directive_name);
                    directive_name = NULL;
                    free(directive_argument);
                    directive_argument = NULL;
                    sb_trie_free(directive_params);
                    directive_params = NULL;
                    free(prefix);
                    prefix = NULL;
                    break;
                }
                if (c == ' ' || c == '\t') {
                    start2 = current;
                    state = CONTENT_DIRECTIVE_PARAM_PREFIX;
                    break;
                }
                state = CONTENT_PARAGRAPH;
                if (is_last)
                    continue;
                break;

            case CONTENT_PARAGRAPH:
                if (c == '\n' || c == '\r' || is_last) {
                    state = CONTENT_PARAGRAPH_END;
                    end = is_last && c != '\n' && c != '\r' ? src_len :
                        (real_end != 0 ? real_end : current);
                    if (description != NULL && *description == NULL)
                        *description = sb_strndup(src + start, end - start);
                }
                if (!is_last)
                    break;

            case CONTENT_PARAGRAPH_END:
                if (c == '\n' || c == '\r' || is_last) {
                    tmp = sb_strndup(src + start, end - start);
                    parsed = blogc_content_parse_inline(tmp);
                    sb_string_append_printf(rv, "<p>%s</p>%s", parsed,
                        line_ending);
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

    free(directive_name);
    free(directive_argument);
    free(directive_key);
    sb_trie_free(directive_params);
    free(prefix);

    return sb_string_free(rv, false);
}
