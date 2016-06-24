/*
 * blogc: A blog compiler.
 * Copyright (C) 2015-2016 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifndef _CONTENT_PARSER_H
#define _CONTENT_PARSER_H

#include <stddef.h>
#include <stdbool.h>

#include "utils.h"

/*
 * Raw node
 *
 * +-----+
 * | RAW | -> ...
 * +-----+
 *
 */

/*
 * Header node
 *
 * +--------+
 * | HEADER | -> ...
 * +--------+
 *     |
 *  +-----+    +------+    +-----+
 *  | raw | -> | bold | -> | raw | -> ...
 *  +-----+    +------+    +-----+
 *
 */

/*
 * Blockquote node
 *
 * +------------+
 * | BLOCKQUOTE | -> ...
 * +------------+
 *       |
 *  +-----------+    +--------+    +-----------+
 *  | PARAGRAPH | -> | HEADER | -> | PARAGRAPH | -> ...
 *  +-----------+    +--------+    +-----------+
 *        |
 *     +-----+
 *     | raw | -> ...
 *     +-----+
 *
 */

/*
 * Code node
 *
 * +------+
 * | CODE | -> ...
 * +------+
 *
 */

/*
 * Horizontal rule node
 *
 * +-----------------+
 * | HORIZONTAL_RULE | -> ...
 * +-----------------+
 *
 */

/*
 * Unordered list node
 *
 * +----------------+
 * | UNORDERED_LIST | -> ...
 * +----------------+
 *         |
 *   +-----------+    +-----------+
 *   | LIST_ITEM | -> | LIST_ITEM | -> ...
 *   +-----------+    +-----------+
 *         |
 *      +-----+    +------+
 *      | raw | -> | bold | -> ...
 *      +-----+    +------+
 *
 */

/*
 * Ordered list node
 *
 * +--------------+
 * | ORDERED_LIST | -> ...
 * +--------------+
 *        |
 *  +-----------+    +-----------+
 *  | LIST_ITEM | -> | LIST_ITEM | -> ...
 *  +-----------+    +-----------+
 *        |
 *     +-----+    +------+
 *     | raw | -> | bold | -> ...
 *     +-----+    +------+
 *
 */

/*
 * Paragraph node
 *
 * +-----------+
 * | PARAGRAPH | -> ...
 * +-----------+
 *       |
 *    +-----+    +------+    +-----+
 *    | raw | -> | bold | -> | raw |
 *    +-----+    +------+    +-----+
 *
 */


typedef enum {
    BLOGC_CONTENT_BLOCK = 1,
    BLOGC_CONTENT_INLINE,
} blogc_content_node_type_t;

typedef enum {
    BLOGC_CONTENT_BLOCK_RAW = 1,
    BLOGC_CONTENT_BLOCK_HEADER,
    BLOGC_CONTENT_BLOCK_BLOCKQUOTE,
    BLOGC_CONTENT_BLOCK_CODE,
    BLOGC_CONTENT_BLOCK_HORIZONTAL_RULE,
    BLOGC_CONTENT_BLOCK_UNORDERED_LIST,
    BLOGC_CONTENT_BLOCK_ORDERED_LIST,
    BLOGC_CONTENT_BLOCK_LIST_ITEM,
    BLOGC_CONTENT_BLOCK_PARAGRAPH,
    BLOGC_CONTENT_BLOCK_EXCERPT,
} blogc_content_block_type_t;

typedef enum {
    BLOGC_CONTENT_INLINE_RAW = 1,
    BLOGC_CONTENT_INLINE_LINK,
    BLOGC_CONTENT_INLINE_IMAGE,
    BLOGC_CONTENT_INLINE_BOLD,
    BLOGC_CONTENT_INLINE_ITALIC,
    BLOGC_CONTENT_INLINE_CODE,
    BLOGC_CONTENT_INLINE_BREAK_LINE,
} blogc_content_inline_type_t;

typedef struct _blogc_content_node_t {
    blogc_content_node_type_t node_type;
    union {
        blogc_content_block_type_t block_type;
        blogc_content_inline_type_t inline_type;
    } type;
    char *content;
    struct _blogc_content_node_t *child;
    struct _blogc_content_node_t *next;
    sb_trie_t *parameters;
} blogc_content_node_t;

char* blogc_slugify(const char *str);
char* blogc_htmlentities(const char *str);
char* blogc_fix_description(const char *paragraph);
char* blogc_content_parse_inline(const char *src);
bool blogc_is_ordered_list_item(const char *str, size_t prefix_len);
blogc_content_node_t* blogc_content_parse_ast(const char *src, char **nl);
void blogc_content_free_ast(blogc_content_node_t *ast);
char* blogc_content_parse(const char *src, char **excerpt, char **description);
char* blogc_content_render_html(blogc_content_node_t *ast, char *nl, char **excerpt,
    char **description);
void blogc_content_debug(blogc_content_node_t *ast);

#endif /* _CONTENT_PARSER_H */
