/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2016 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#define SB_STRING_CHUNK_SIZE 128

#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>

#include "utils.h"


void*
sb_malloc(size_t size)
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
sb_realloc(void *ptr, size_t size)
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


sb_slist_t*
sb_slist_append(sb_slist_t *l, void *data)
{
    sb_slist_t *node = sb_malloc(sizeof(sb_slist_t));
    node->data = data;
    node->next = NULL;
    if (l == NULL) {
        l = node;
    }
    else {
        sb_slist_t *tmp;
        for (tmp = l; tmp->next != NULL; tmp = tmp->next);
        tmp->next = node;
    }
    return l;
}


sb_slist_t*
sb_slist_prepend(sb_slist_t *l, void *data)
{
    sb_slist_t *node = sb_malloc(sizeof(sb_slist_t));
    node->data = data;
    node->next = l;
    l = node;
    return l;
}


void
sb_slist_free_full(sb_slist_t *l, sb_free_func_t free_func)
{
    while (l != NULL) {
        sb_slist_t *tmp = l->next;
        if ((free_func != NULL) && (l->data != NULL))
            free_func(l->data);
        free(l);
        l = tmp;
    }
}


void
sb_slist_free(sb_slist_t *l)
{
    sb_slist_free_full(l, NULL);
}


size_t
sb_slist_length(sb_slist_t *l)
{
    if (l == NULL)
        return 0;
    size_t i;
    sb_slist_t *tmp;
    for (tmp = l, i = 0; tmp != NULL; tmp = tmp->next, i++);
    return i;
}


char*
sb_strdup(const char *s)
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
sb_strndup(const char *s, size_t n)
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
sb_strdup_vprintf(const char *format, va_list ap)
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
sb_strdup_printf(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    char *tmp = sb_strdup_vprintf(format, ap);
    va_end(ap);
    return tmp;
}


bool
sb_str_starts_with(const char *str, const char *prefix)
{
    int str_l = strlen(str);
    int str_lp = strlen(prefix);
    if (str_lp > str_l)
        return false;
    return strncmp(str, prefix, str_lp) == 0;
}


bool
sb_str_ends_with(const char *str, const char *suffix)
{
    int str_l = strlen(str);
    int str_ls = strlen(suffix);
    if (str_ls > str_l)
        return false;
    return strcmp(str + str_l - str_ls, suffix) == 0;
}


char*
sb_str_lstrip(char *str)
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
sb_str_rstrip(char *str)
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
sb_str_strip(char *str)
{
    return sb_str_lstrip(sb_str_rstrip(str));
}


char**
sb_str_split(const char *str, char c, unsigned int max_pieces)
{
    if (str == NULL)
        return NULL;
    char **rv = sb_malloc(sizeof(char*));
    unsigned int i, start = 0, count = 0;
    for (i = 0; i < strlen(str) + 1; i++) {
        if (str[0] == '\0')
            break;
        if ((str[i] == c && (!max_pieces || count + 1 < max_pieces)) || str[i] == '\0') {
            rv = sb_realloc(rv, (count + 1) * sizeof(char*));
            rv[count] = sb_malloc(i - start + 1);
            memcpy(rv[count], str + start, i - start);
            rv[count++][i - start] = '\0';
            start = i + 1;
        }
    }
    rv = sb_realloc(rv, (count + 1) * sizeof(char*));
    rv[count] = NULL;
    return rv;
}


char*
sb_str_replace(const char *str, const char search, const char *replace)
{
    char **pieces = sb_str_split(str, search, 0);
    if (pieces == NULL)
        return NULL;
    char* rv = sb_strv_join(pieces, replace);
    sb_strv_free(pieces);
    if (rv == NULL)
        return sb_strdup(str);
    return rv;
}


void
sb_strv_free(char **strv)
{
    if (strv == NULL)
        return;
    for (size_t i = 0; strv[i] != NULL; i++)
        free(strv[i]);
    free(strv);
}


char*
sb_strv_join(char **strv, const char *separator)
{
    if (strv == NULL || separator == NULL)
        return NULL;
    sb_string_t *str = sb_string_new();
    for (size_t i = 0; strv[i] != NULL; i++) {
        str = sb_string_append(str, strv[i]);
        if (strv[i + 1] != NULL)
            str = sb_string_append(str, separator);
    }
    return sb_string_free(str, false);
}


size_t
sb_strv_length(char **strv)
{
    if (strv == NULL)
        return 0;
    size_t i;
    for (i = 0; strv[i] != NULL; i++);
    return i;
}


sb_string_t*
sb_string_new(void)
{
    sb_string_t* rv = sb_malloc(sizeof(sb_string_t));
    rv->str = NULL;
    rv->len = 0;
    rv->allocated_len = 0;

    // initialize with empty string
    rv = sb_string_append(rv, "");

    return rv;
}


char*
sb_string_free(sb_string_t *str, bool free_str)
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


sb_string_t*
sb_string_dup(sb_string_t *str)
{
    if (str == NULL)
        return NULL;
    sb_string_t* new = sb_string_new();
    return sb_string_append_len(new, str->str, str->len);
}


sb_string_t*
sb_string_append_len(sb_string_t *str, const char *suffix, size_t len)
{
    if (str == NULL)
        return NULL;
    if (suffix == NULL)
        return str;
    size_t old_len = str->len;
    str->len += len;
    if (str->len + 1 > str->allocated_len) {
        str->allocated_len = (((str->len + 1) / SB_STRING_CHUNK_SIZE) + 1) * SB_STRING_CHUNK_SIZE;
        str->str = sb_realloc(str->str, str->allocated_len);
    }
    memcpy(str->str + old_len, suffix, len);
    str->str[str->len] = '\0';
    return str;
}


sb_string_t*
sb_string_append(sb_string_t *str, const char *suffix)
{
    if (str == NULL)
        return NULL;
    const char *my_suffix = suffix == NULL ? "" : suffix;
    return sb_string_append_len(str, my_suffix, strlen(my_suffix));
}


sb_string_t*
sb_string_append_c(sb_string_t *str, char c)
{
    if (str == NULL)
        return NULL;
    size_t old_len = str->len;
    str->len += 1;
    if (str->len + 1 > str->allocated_len) {
        str->allocated_len = (((str->len + 1) / SB_STRING_CHUNK_SIZE) + 1) * SB_STRING_CHUNK_SIZE;
        str->str = sb_realloc(str->str, str->allocated_len);
    }
    str->str[old_len] = c;
    str->str[str->len] = '\0';
    return str;
}


sb_string_t*
sb_string_append_printf(sb_string_t *str, const char *format, ...)
{
    if (str == NULL)
        return NULL;
    va_list ap;
    va_start(ap, format);
    char *tmp = sb_strdup_vprintf(format, ap);
    va_end(ap);
    str = sb_string_append(str, tmp);
    free(tmp);
    return str;
}


sb_trie_t*
sb_trie_new(sb_free_func_t free_func)
{
    sb_trie_t *trie = sb_malloc(sizeof(sb_trie_t));
    trie->root = NULL;
    trie->free_func = free_func;
    return trie;
}


static void
sb_trie_free_node(sb_trie_t *trie, sb_trie_node_t *node)
{
    if (trie == NULL || node == NULL)
        return;
    if (node->data != NULL && trie->free_func != NULL)
        trie->free_func(node->data);
    sb_trie_free_node(trie, node->next);
    sb_trie_free_node(trie, node->child);
    free(node);
}


void
sb_trie_free(sb_trie_t *trie)
{
    if (trie == NULL)
        return;
    sb_trie_free_node(trie, trie->root);
    free(trie);
}


void
sb_trie_insert(sb_trie_t *trie, const char *key, void *data)
{
    if (trie == NULL || key == NULL || data == NULL)
        return;

    sb_trie_node_t *parent = NULL;
    sb_trie_node_t *previous;
    sb_trie_node_t *current;
    sb_trie_node_t *tmp;

    while (1) {

        if (trie->root == NULL || (parent != NULL && parent->child == NULL)) {
            current = sb_malloc(sizeof(sb_trie_node_t));
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

        current = sb_malloc(sizeof(sb_trie_node_t));
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
sb_trie_lookup(sb_trie_t *trie, const char *key)
{
    if (trie == NULL || trie->root == NULL || key == NULL)
        return NULL;

    sb_trie_node_t *parent = trie->root;
    sb_trie_node_t *tmp;
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
sb_trie_size_node(sb_trie_node_t *node, size_t *count)
{
    if (node == NULL || count == NULL)
        return;

    if (node->key == '\0')
        (*count)++;

    sb_trie_size_node(node->next, count);
    sb_trie_size_node(node->child, count);
}


size_t
sb_trie_size(sb_trie_t *trie)
{
    if (trie == NULL)
        return 0;

    size_t count = 0;
    sb_trie_size_node(trie->root, &count);
    return count;
}


static void
sb_trie_foreach_node(sb_trie_node_t *node, sb_string_t *str,
    sb_trie_foreach_func_t func, void *user_data)
{
    if (node == NULL || str == NULL || func == NULL)
        return;

    if (node->key == '\0') {
        char *tmp = sb_string_free(str, false);
        func(tmp, node->data, user_data);
        free(tmp);
    }

    if (node->child != NULL) {
        sb_string_t *child = sb_string_dup(str);
        child = sb_string_append_c(child, node->key);
        sb_trie_foreach_node(node->child, child, func, user_data);
    }

    if (node->next != NULL)
        sb_trie_foreach_node(node->next, str, func, user_data);

    if (node->child != NULL && node->next == NULL)
        sb_string_free(str, true);
}


void
sb_trie_foreach(sb_trie_t *trie, sb_trie_foreach_func_t func,
    void *user_data)
{
    if (trie == NULL || trie->root == NULL || func == NULL)
        return;

    sb_string_t *str = sb_string_new();
    sb_trie_foreach_node(trie->root, str, func, user_data);
}
