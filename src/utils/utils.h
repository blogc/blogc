/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2015 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifndef _UTILS_UTILS_H
#define _UTILS_UTILS_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>

#define B_STRING_CHUNK_SIZE 128

typedef void (*b_free_func_t) (void *ptr);

typedef struct _b_slist_t {
    struct _b_slist_t *next;
    void *data;
} b_slist_t;

typedef struct _b_string_t {
    char *str;
    size_t len;
    size_t allocated_len;
} b_string_t;

typedef struct _b_trie_node_t {
    char key;
    void *data;
    struct _b_trie_node_t *next, *child;
} b_trie_node_t;

typedef struct _b_trie_t {
    b_trie_node_t *root;
    b_free_func_t free_func;
} b_trie_t;

b_slist_t* b_slist_append(b_slist_t *l, void *data);
void b_slist_free_full(b_slist_t *l, b_free_func_t free_func);
void b_slist_free(b_slist_t *l);
unsigned int b_slist_length(b_slist_t *l);

char* b_strdup(const char *s);
char* b_strndup(const char *s, size_t n);
char* b_strdup_vprintf(const char *format, va_list ap);
char* b_strdup_printf(const char *format, ...);
bool b_str_starts_with(const char *str, const char *prefix);
bool b_str_ends_with(const char *str, const char *suffix);
char* b_str_strip(char *str);
char** b_str_split(const char *str, char c, unsigned int max_pieces);
char* b_str_replace(const char *str, const char search, const char *replace);
void b_strv_free(char **strv);
char* b_strv_join(const char **strv, const char *separator);
unsigned int b_strv_length(char **strv);

b_string_t* b_string_new(void);
char* b_string_free(b_string_t *str, bool free_str);
b_string_t* b_string_append_len(b_string_t *str, const char *suffix, size_t len);
b_string_t* b_string_append(b_string_t *str, const char *suffix);
b_string_t* b_string_append_c(b_string_t *str, char c);
b_string_t* b_string_append_printf(b_string_t *str, const char *format, ...);

b_trie_t* b_trie_new(b_free_func_t free_func);
void b_trie_free(b_trie_t *trie);
void b_trie_insert(b_trie_t *trie, const char *key, void *data);
void* b_trie_lookup(b_trie_t *trie, const char *key);
unsigned int b_trie_size(b_trie_t *trie);
void b_trie_foreach(b_trie_t *trie, void (*func)(const char *key, void *data));

void* b_malloc(size_t size);
void* b_realloc(void *ptr, size_t size);

#endif /* _UTILS_UTILS_H */
