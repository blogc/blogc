/*
 * blogc: A blog compiler.
 * Copyright (C) 2016 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifndef _THREAD_POOL_H
#define _THREAD_POOL_H

#include "utils.h"

typedef void (*bc_threadpool_func_t) (void *job, void *user_data);

typedef struct {
    bc_slist_t *jobs;
    bc_slist_t *threads;
    size_t max_threads;
    bc_threadpool_func_t func;
    void *user_data;
} bc_threadpool_t;

bc_threadpool_t* bc_threadpool_new(bc_threadpool_func_t func,
    size_t max_threads, void *user_data, bc_error_t **err);

#endif /* _THREAD_POOL_H */
