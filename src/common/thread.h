/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2018 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifndef _THREAD_H
#define _THREAD_H

#include <stdbool.h>

typedef void* (*bc_thread_func_t) (void *arg);

typedef struct _bc_thread_t bc_thread_t;

bc_thread_t* bc_thread_create(bc_thread_func_t func, void *arg, bool detached,
    bc_error_t **err);
void bc_thread_join(bc_thread_t *thread, bc_error_t **err);
void bc_thread_free(bc_thread_t *thread);

#endif /* _THREAD_H */
