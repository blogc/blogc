/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2017 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#define BC_STRING_CHUNK_SIZE 128

#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "utils.h"


void*
bc_malloc(size_t size)
{
    // simple things simple!
    void *rv = malloc(size);
    if (rv == NULL) {
        fprintf(stderr, "fatal: Failed to allocate memory!\n");
        abort();
    }
    return rv;
}


void*
bc_realloc(void *ptr, size_t size)
{
    // simple things even simpler :P
    void *rv = realloc(ptr, size);
    if (rv == NULL && size != 0) {
        fprintf(stderr, "fatal: Failed to reallocate memory!\n");
        free(ptr);
        abort();
    }
    return rv;
}


bc_slist_t*
bc_slist_append(bc_slist_t *l, void *data)
{
    bc_slist_t *node = bc_malloc(sizeof(bc_slist_t));
    node->data = data;
    node->next = NULL;
    if (l == NULL) {
        l = node;
    }
    else {
        bc_slist_t *tmp;
        for (tmp = l; tmp->next != NULL; tmp = tmp->next);
        tmp->next = node;
    }
    return l;
}


bc_slist_t*
bc_slist_prepend(bc_slist_t *l, void *data)
{
    bc_slist_t *node = bc_malloc(sizeof(bc_slist_t));
    node->data = data;
    node->next = l;
    l = node;
    return l;
}


void
bc_slist_free_full(bc_slist_t *l, bc_free_func_t free_func)
{
    while (l != NULL) {
        bc_slist_t *tmp = l->next;
        if ((free_func != NULL) && (l->data != NULL))
            free_func(l->data);
        free(l);
        l = tmp;
    }
}


void
bc_slist_free(bc_slist_t *l)
{
    bc_slist_free_full(l, NULL);
}


size_t
bc_slist_length(bc_slist_t *l)
{
    if (l == NULL)
        return 0;
    size_t i;
    bc_slist_t *tmp;
    for (tmp = l, i = 0; tmp != NULL; tmp = tmp->next, i++);
    return i;
}


bc_slist_t*
bc_slist_pop(bc_slist_t *l, void **data)
{
    if (l == NULL)
        return l;
    bc_slist_t *tmp = l;
    l = l->next;
    *data = tmp->data;
    free(tmp);
    return l;
}


char*
bc_strdup(const char *s)
{
    if (s == NULL)
        return NULL;
    size_t l = strlen(s);
    char *tmp = malloc(l + 1);
    if (tmp == NULL)
        return NULL;
    memcpy(tmp, s, l + 1);
    return tmp;
}


char*
bc_strndup(const char *s, size_t n)
{
    if (s == NULL)
        return NULL;
    size_t l = strnlen(s, n);
    char *tmp = malloc(l + 1);
    if (tmp == NULL)
        return NULL;
    memcpy(tmp, s, l);
    tmp[l] = '\0';
    return tmp;
}


char*
bc_strdup_vprintf(const char *format, va_list ap)
{
    va_list ap2;
    va_copy(ap2, ap);
    int l = vsnprintf(NULL, 0, format, ap2);
    va_end(ap2);
    if (l < 0)
        return NULL;
    char *tmp = malloc(l + 1);
    if (!tmp)
        return NULL;
    int l2 = vsnprintf(tmp, l + 1, format, ap);
    if (l2 < 0) {
        free(tmp);
        return NULL;
    }
    return tmp;
}


char*
bc_strdup_printf(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    char *tmp = bc_strdup_vprintf(format, ap);
    va_end(ap);
    return tmp;
}


bool
bc_str_starts_with(const char *str, const char *prefix)
{
    int str_l = strlen(str);
    int str_lp = strlen(prefix);
    if (str_lp > str_l)
        return false;
    return strncmp(str, prefix, str_lp) == 0;
}


bool
bc_str_ends_with(const char *str, const char *suffix)
{
    int str_l = strlen(str);
    int str_ls = strlen(suffix);
    if (str_ls > str_l)
        return false;
    return strcmp(str + str_l - str_ls, suffix) == 0;
}


char*
bc_str_lstrip(char *str)
{
    if (str == NULL)
        return NULL;
    int i;
    size_t str_len = strlen(str);
    for (i = 0; i < str_len; i++) {
        if ((str[i] != ' ') && (str[i] != '\t') && (str[i] != '\n') &&
            (str[i] != '\r') && (str[i] != '\t') && (str[i] != '\f') &&
            (str[i] != '\v'))
        {
            str += i;
            break;
        }
        if (i == str_len - 1) {
            str += str_len;
            break;
        }
    }
    return str;
}


char*
bc_str_rstrip(char *str)
{
    if (str == NULL)
        return NULL;
    int i;
    size_t str_len = strlen(str);
    for (i = str_len - 1; i >= 0; i--) {
        if ((str[i] != ' ') && (str[i] != '\t') && (str[i] != '\n') &&
            (str[i] != '\r') && (str[i] != '\t') && (str[i] != '\f') &&
            (str[i] != '\v'))
        {
            str[i + 1] = '\0';
            break;
        }
        if (i == 0) {
            str[0] = '\0';
            break;
        }
    }
    return str;
}


char*
bc_str_strip(char *str)
{
    return bc_str_lstrip(bc_str_rstrip(str));
}


char**
bc_str_split(const char *str, char c, unsigned int max_pieces)
{
    if (str == NULL)
        return NULL;
    char **rv = bc_malloc(sizeof(char*));
    unsigned int i, start = 0, count = 0;
    for (i = 0; i < strlen(str) + 1; i++) {
        if (str[0] == '\0')
            break;
        if ((str[i] == c && (!max_pieces || count + 1 < max_pieces)) || str[i] == '\0') {
            rv = bc_realloc(rv, (count + 1) * sizeof(char*));
            rv[count] = bc_malloc(i - start + 1);
            memcpy(rv[count], str + start, i - start);
            rv[count++][i - start] = '\0';
            start = i + 1;
        }
    }
    rv = bc_realloc(rv, (count + 1) * sizeof(char*));
    rv[count] = NULL;
    return rv;
}


char*
bc_str_replace(const char *str, const char search, const char *replace)
{
    char **pieces = bc_str_split(str, search, 0);
    if (pieces == NULL)
        return NULL;
    char* rv = bc_strv_join(pieces, replace);
    bc_strv_free(pieces);
    if (rv == NULL)
        return bc_strdup(str);
    return rv;
}


char*
bc_str_find(const char *str, char c)
{
    // this is somewhat similar to strchr, but respects '\' escaping.
    if (str == NULL)
        return NULL;
    if (c == '\0')
        return (char*) str + strlen(str);
    for (size_t i = 0; str[i] != '\0'; i++) {
        if (str[i] == '\\') {
            i++;
            continue;
        }
        if (str[i] == c) {
            return (char*) str + i;
        }
    }
    return NULL;
}


void
bc_strv_free(char **strv)
{
    if (strv == NULL)
        return;
    for (size_t i = 0; strv[i] != NULL; i++)
        free(strv[i]);
    free(strv);
}


char*
bc_strv_join(char **strv, const char *separator)
{
    if (strv == NULL || separator == NULL)
        return NULL;
    bc_string_t *str = bc_string_new();
    for (size_t i = 0; strv[i] != NULL; i++) {
        str = bc_string_append(str, strv[i]);
        if (strv[i + 1] != NULL)
            str = bc_string_append(str, separator);
    }
    return bc_string_free(str, false);
}


size_t
bc_strv_length(char **strv)
{
    if (strv == NULL)
        return 0;
    size_t i;
    for (i = 0; strv[i] != NULL; i++);
    return i;
}


bc_string_t*
bc_string_new(void)
{
    bc_string_t* rv = bc_malloc(sizeof(bc_string_t));
    rv->str = NULL;
    rv->len = 0;
    rv->allocated_len = 0;

    // initialize with empty string
    rv = bc_string_append(rv, "");

    return rv;
}


char*
bc_string_free(bc_string_t *str, bool free_str)
{
    if (str == NULL)
        return NULL;
    char *rv = NULL;
    if (free_str)
        free(str->str);
    else
        rv = str->str;
    free(str);
    return rv;
}


bc_string_t*
bc_string_dup(bc_string_t *str)
{
    if (str == NULL)
        return NULL;
    bc_string_t* new = bc_string_new();
    return bc_string_append_len(new, str->str, str->len);
}


bc_string_t*
bc_string_append_len(bc_string_t *str, const char *suffix, size_t len)
{
    if (str == NULL)
        return NULL;
    if (suffix == NULL)
        return str;
    size_t old_len = str->len;
    str->len += len;
    if (str->len + 1 > str->allocated_len) {
        str->allocated_len = (((str->len + 1) / BC_STRING_CHUNK_SIZE) + 1) * BC_STRING_CHUNK_SIZE;
        str->str = bc_realloc(str->str, str->allocated_len);
    }
    memcpy(str->str + old_len, suffix, len);
    str->str[str->len] = '\0';
    return str;
}


bc_string_t*
bc_string_append(bc_string_t *str, const char *suffix)
{
    if (str == NULL)
        return NULL;
    const char *my_suffix = suffix == NULL ? "" : suffix;
    return bc_string_append_len(str, my_suffix, strlen(my_suffix));
}


bc_string_t*
bc_string_append_c(bc_string_t *str, char c)
{
    if (str == NULL)
        return NULL;
    size_t old_len = str->len;
    str->len += 1;
    if (str->len + 1 > str->allocated_len) {
        str->allocated_len = (((str->len + 1) / BC_STRING_CHUNK_SIZE) + 1) * BC_STRING_CHUNK_SIZE;
        str->str = bc_realloc(str->str, str->allocated_len);
    }
    str->str[old_len] = c;
    str->str[str->len] = '\0';
    return str;
}


bc_string_t*
bc_string_append_printf(bc_string_t *str, const char *format, ...)
{
    if (str == NULL)
        return NULL;
    va_list ap;
    va_start(ap, format);
    char *tmp = bc_strdup_vprintf(format, ap);
    va_end(ap);
    str = bc_string_append(str, tmp);
    free(tmp);
    return str;
}


bc_string_t*
bc_string_append_escaped(bc_string_t *str, const char *suffix)
{
    if (str == NULL)
        return NULL;
    if (suffix == NULL)
        return str;
    bool escaped = false;
    for (size_t i = 0; suffix[i] != '\0'; i++) {
        if (suffix[i] == '\\' && !escaped) {
            escaped = true;
            continue;
        }
        escaped = false;
        str = bc_string_append_c(str, suffix[i]);
    }
    return str;
}


bc_trie_t*
bc_trie_new(bc_free_func_t free_func)
{
    bc_trie_t *trie = bc_malloc(sizeof(bc_trie_t));
    trie->root = NULL;
    trie->free_func = free_func;
    return trie;
}


static void
bc_trie_free_node(bc_trie_t *trie, bc_trie_node_t *node)
{
    if (trie == NULL || node == NULL)
        return;
    if (node->data != NULL && trie->free_func != NULL)
        trie->free_func(node->data);
    bc_trie_free_node(trie, node->next);
    bc_trie_free_node(trie, node->child);
    free(node);
}


void
bc_trie_free(bc_trie_t *trie)
{
    if (trie == NULL)
        return;
    bc_trie_free_node(trie, trie->root);
    free(trie);
}


void
bc_trie_insert(bc_trie_t *trie, const char *key, void *data)
{
    if (trie == NULL || key == NULL || data == NULL)
        return;

    bc_trie_node_t *parent = NULL;
    bc_trie_node_t *previous;
    bc_trie_node_t *current;
    bc_trie_node_t *tmp;

    while (1) {

        if (trie->root == NULL || (parent != NULL && parent->child == NULL)) {
            current = bc_malloc(sizeof(bc_trie_node_t));
            current->key = *key;
            current->data = NULL;
            current->next = NULL;
            current->child = NULL;
            if (trie->root == NULL)
                trie->root = current;
            else
                parent->child = current;
            parent = current;
            goto clean;
        }

        tmp = parent == NULL ? trie->root : parent->child;
        previous = NULL;

        while (tmp != NULL && tmp->key != *key) {
            previous = tmp;
            tmp = tmp->next;
        }

        parent = tmp;

        if (previous == NULL || parent != NULL)
            goto clean;

        current = bc_malloc(sizeof(bc_trie_node_t));
        current->key = *key;
        current->data = NULL;
        current->next = NULL;
        current->child = NULL;
        previous->next = current;
        parent = current;

clean:
        if (*key == '\0') {
            if (parent->data != NULL && trie->free_func != NULL)
                trie->free_func(parent->data);
            parent->data = data;
            break;
        }
        key++;
    }
}


void*
bc_trie_lookup(bc_trie_t *trie, const char *key)
{
    if (trie == NULL || trie->root == NULL || key == NULL)
        return NULL;

    bc_trie_node_t *parent = trie->root;
    bc_trie_node_t *tmp;
    while (1) {
        for (tmp = parent; tmp != NULL; tmp = tmp->next) {

            if (tmp->key == *key) {
                if (tmp->key == '\0')
                    return tmp->data;
                parent = tmp->child;
                break;
            }
        }
        if (tmp == NULL)
            return NULL;

        if (*key == '\0')
            break;
        key++;
    }
    return NULL;
}


static void
bc_trie_size_node(bc_trie_node_t *node, size_t *count)
{
    if (node == NULL || count == NULL)
        return;

    if (node->key == '\0')
        (*count)++;

    bc_trie_size_node(node->next, count);
    bc_trie_size_node(node->child, count);
}


size_t
bc_trie_size(bc_trie_t *trie)
{
    if (trie == NULL)
        return 0;

    size_t count = 0;
    bc_trie_size_node(trie->root, &count);
    return count;
}


static void
bc_trie_foreach_node(bc_trie_node_t *node, bc_string_t *str,
    bc_trie_foreach_func_t func, void *user_data)
{
    if (node == NULL || str == NULL || func == NULL)
        return;

    if (node->key == '\0')
        func(str->str, node->data, user_data);

    if (node->child != NULL) {
        bc_string_t *child = bc_string_dup(str);
        child = bc_string_append_c(child, node->key);
        bc_trie_foreach_node(node->child, child, func, user_data);
        bc_string_free(child, true);
    }

    if (node->next != NULL)
        bc_trie_foreach_node(node->next, str, func, user_data);
}


void
bc_trie_foreach(bc_trie_t *trie, bc_trie_foreach_func_t func,
    void *user_data)
{
    if (trie == NULL || trie->root == NULL || func == NULL)
        return;

    bc_string_t *str = bc_string_new();
    bc_trie_foreach_node(trie->root, str, func, user_data);
    bc_string_free(str, true);
}


char*
bc_shell_quote(const char *command)
{
    bc_string_t *rv = bc_string_new();
    bc_string_append_c(rv, '\'');
    if (command != NULL) {
        for (size_t i = 0; i < strlen(command); i++) {
            switch (command[i]) {
                case '!':
                    bc_string_append(rv, "'\\!'");
                    break;
                case '\'':
                    bc_string_append(rv, "'\\''");
                    break;
                default:
                    bc_string_append_c(rv, command[i]);
            }
        }
    }
    bc_string_append_c(rv, '\'');
    return bc_string_free(rv, false);
}
