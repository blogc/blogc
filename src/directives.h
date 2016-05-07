/*
 * blogc: A blog compiler.
 * Copyright (C) 2015 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifndef _DIRECTIVES_H
#define _DIRECTIVES_H

#include "utils.h"
#include "error.h"

typedef struct {
    const char *name;
    const char *argument;
    sb_trie_t *params;
    const char *eol;
} blogc_directive_ctx_t;

typedef char* (*blogc_directive_func_t)(blogc_directive_ctx_t *ctx,
    blogc_error_t **err);

typedef struct {
    const char *name;
    blogc_directive_func_t callback;
} blogc_directive_t;

char* blogc_directive_loader(blogc_directive_ctx_t *ctx, blogc_error_t **err);


// built-in directives (that are everything we support right now
static char* blogc_directive_youtube(blogc_directive_ctx_t *ctx, blogc_error_t **err);

#endif /* _DIRECTIVES_H */
