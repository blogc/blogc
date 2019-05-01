/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2019 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifndef _UTILS_H
#define _UTILS_H

#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>


// memory

typedef void (*bc_free_func_t) (void *ptr);

void* bc_malloc(size_t size);
void* bc_realloc(void *ptr, size_t size);


// slist

typedef struct _bc_slist_t {
    struct _bc_slist_t *next;
    void *data;
} bc_slist_t;

bc_slist_t* bc_slist_append(bc_slist_t *l, void *data);
bc_slist_t* bc_slist_prepend(bc_slist_t *l, void *data);
void bc_slist_free(bc_slist_t *l);
void bc_slist_free_full(bc_slist_t *l, bc_free_func_t free_func);
size_t bc_slist_length(bc_slist_t *l);


// strfuncs

char* bc_strdup(const char *s);
char* bc_strndup(const char *s, size_t n);
char* bc_strdup_vprintf(const char *format, va_list ap);
char* bc_strdup_printf(const char *format, ...);
bool bc_isspace(int c);
bool bc_str_starts_with(const char *str, const char *prefix);
bool bc_str_ends_with(const char *str, const char *suffix);
char* bc_str_lstrip(char *str);
char* bc_str_rstrip(char *str);
char* bc_str_strip(char *str);
char** bc_str_split(const char *str, char c, size_t max_pieces);
char* bc_str_replace(const char *str, const char search, const char *replace);
char* bc_str_find(const char *str, char c);
bool bc_str_to_bool(const char *str);
void bc_strv_free(char **strv);
char* bc_strv_join(char **strv, const char *separator);
size_t bc_strv_length(char **strv);


// string

typedef struct {
    char *str;
    size_t len;
    size_t allocated_len;
} bc_string_t;

bc_string_t* bc_string_new(void);
char* bc_string_free(bc_string_t *str, bool free_str);
bc_string_t* bc_string_dup(bc_string_t *str);
bc_string_t* bc_string_append_len(bc_string_t *str, const char *suffix, size_t len);
bc_string_t* bc_string_append(bc_string_t *str, const char *suffix);
bc_string_t* bc_string_append_c(bc_string_t *str, char c);
bc_string_t* bc_string_append_printf(bc_string_t *str, const char *format, ...);
bc_string_t* bc_string_append_escaped(bc_string_t *str, const char *suffix);


// trie

typedef struct _bc_trie_node_t {
    char key;
    void *data;
    struct _bc_trie_node_t *next, *child;
} bc_trie_node_t;

struct _bc_trie_t {
    bc_trie_node_t *root;
    bc_free_func_t free_func;
};

typedef struct _bc_trie_t bc_trie_t;

typedef void (*bc_trie_foreach_func_t)(const char *key, void *data,
    void *user_data);

bc_trie_t* bc_trie_new(bc_free_func_t free_func);
void bc_trie_free(bc_trie_t *trie);
void bc_trie_insert(bc_trie_t *trie, const char *key, void *data);
void* bc_trie_lookup(bc_trie_t *trie, const char *key);
size_t bc_trie_size(bc_trie_t *trie);
void bc_trie_foreach(bc_trie_t *trie, bc_trie_foreach_func_t func,
    void *user_data);


// shell

char* bc_shell_quote(const char *command);

#endif /* _UTILS_H */
