/*
 * blogc: A blog compiler.
 * Copyright (C) 2016 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "error.h"
#include "utils.h"
#include "thread-pool.h"

typedef struct {
    bc_threadpool_t *pool;
    pthread_t thread;
    size_t id;
} thread_info_t;

typedef struct {
    bc_threadpool_t *pool;
    void *user_data;
} job_t;


static void*
worker(void *arg)
{
    thread_info_t *info = arg;

    printf("aqui\n");

    while (1) {
        job_t *job = NULL;

        pthread_mutex_lock(&(info->pool->jobs_mutex));
        info->pool->jobs = bc_slist_pop(info->pool->jobs, (void**) &job);
        pthread_mutex_unlock(&(info->pool->jobs_mutex));

        if (job != NULL)
            printf("dentro[%d]: %s\n", info->id, job);
    }

    return NULL;
}


bc_threadpool_t*
bc_threadpool_new(bc_threadpool_func_t func, size_t max_threads,
    void *user_data, bc_error_t **err)
{
    if (err != NULL && *err != NULL)
        return NULL;

    bc_threadpool_t *rv = bc_malloc(sizeof(bc_threadpool_t));
    rv->jobs = NULL;
    pthread_mutex_init(&(rv->jobs_mutex), NULL);
    rv->threads = NULL;
    rv->func = func;
    rv->max_threads = max_threads;
    rv->user_data = user_data;

    int e;

    for (size_t i = 0; i < rv->max_threads; i++) {
        thread_info_t *info = bc_malloc(sizeof(thread_info_t));
        info->pool = rv;
        info->id = i+1;
        if (0 != (e = pthread_create(&(info->thread), NULL, worker, info))) {
            *err = bc_error_new_printf(BC_ERROR_THREADPOOL,
                "Failed to create pool: %s", strerror(e));
            // FIXME: kill any existing threads. currently leaking.
            free(info);
            return NULL;
        }
        rv->threads = bc_slist_append(rv->threads, info);
    }

    return rv;
}


void
bc_threadpool_append(bc_threadpool_t *pool, void *user_data)
{
    pthread_mutex_lock(&(pool->jobs_mutex));
    pool->jobs = bc_slist_append(pool->jobs, user_data);
    pthread_mutex_unlock(&(pool->jobs_mutex));
}
