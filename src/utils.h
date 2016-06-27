/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2016 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifndef _UTILS_H
#define _UTILS_H

#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>


// memory

typedef void (*sb_free_func_t) (void *ptr);

void* sb_malloc(size_t size);
void* sb_realloc(void *ptr, size_t size);


// slist

typedef struct _sb_slist_t {
    struct _sb_slist_t *next;
    void *data;
} sb_slist_t;

sb_slist_t* sb_slist_append(sb_slist_t *l, void *data);
sb_slist_t* sb_slist_prepend(sb_slist_t *l, void *data);
void sb_slist_free(sb_slist_t *l);
void sb_slist_free_full(sb_slist_t *l, sb_free_func_t free_func);
size_t sb_slist_length(sb_slist_t *l);


// strfuncs

char* sb_strdup(const char *s);
char* sb_strndup(const char *s, size_t n);
char* sb_strdup_vprintf(const char *format, va_list ap);
char* sb_strdup_printf(const char *format, ...);
bool sb_str_starts_with(const char *str, const char *prefix);
bool sb_str_ends_with(const char *str, const char *suffix);
char* sb_str_lstrip(char *str);
char* sb_str_rstrip(char *str);
char* sb_str_strip(char *str);
char** sb_str_split(const char *str, char c, unsigned int max_pieces);
char* sb_str_replace(const char *str, const char search, const char *replace);
char* sb_str_find(const char *str, char c);
void sb_strv_free(char **strv);
char* sb_strv_join(char **strv, const char *separator);
size_t sb_strv_length(char **strv);


// string

typedef struct {
    char *str;
    size_t len;
    size_t allocated_len;
} sb_string_t;

sb_string_t* sb_string_new(void);
char* sb_string_free(sb_string_t *str, bool free_str);
sb_string_t* sb_string_dup(sb_string_t *str);
sb_string_t* sb_string_append_len(sb_string_t *str, const char *suffix, size_t len);
sb_string_t* sb_string_append(sb_string_t *str, const char *suffix);
sb_string_t* sb_string_append_c(sb_string_t *str, char c);
sb_string_t* sb_string_append_printf(sb_string_t *str, const char *format, ...);
sb_string_t* sb_string_append_escaped(sb_string_t *str, const char *suffix);


// trie

typedef struct _sb_trie_node_t {
    char key;
    void *data;
    struct _sb_trie_node_t *next, *child;
} sb_trie_node_t;

struct _sb_trie_t {
    sb_trie_node_t *root;
    sb_free_func_t free_func;
};

typedef struct _sb_trie_t sb_trie_t;

typedef void (*sb_trie_foreach_func_t)(const char *key, void *data,
    void *user_data);

sb_trie_t* sb_trie_new(sb_free_func_t free_func);
void sb_trie_free(sb_trie_t *trie);
void sb_trie_insert(sb_trie_t *trie, const char *key, void *data);
void* sb_trie_lookup(sb_trie_t *trie, const char *key);
size_t sb_trie_size(sb_trie_t *trie);
void sb_trie_foreach(sb_trie_t *trie, sb_trie_foreach_func_t func,
    void *user_data);

#endif /* _UTILS_H */
