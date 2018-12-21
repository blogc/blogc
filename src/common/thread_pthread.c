/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2018 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include "error.h"
#include "utils.h"
#include "thread.h"

struct _bc_thread_t {
    pthread_t thread;
};


bc_thread_t*
bc_thread_create(bc_thread_func_t func, void *arg, bool detached, bc_error_t **err)
{
    if (err == NULL || *err != NULL)
        return NULL;

    int r;

    pthread_attr_t attr;
    pthread_attr_t *attrp = NULL;

    if (detached) {

        if (0 != (r = pthread_attr_init(&attr))) {
            *err = bc_error_new_printf(BC_ERROR_THREAD,
                "Failed to initialize thread attributes: %s", strerror(r));
            return NULL;
        }

        if (0 != (r = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED))) {
            *err = bc_error_new_printf(BC_ERROR_THREAD,
                "Failed to mark thread as detached: %s", strerror(r));
            return NULL;
        }

        attrp = &attr;
    }

    bc_thread_t *rv = bc_malloc(sizeof(bc_thread_t));

    if (0 != (r = pthread_create(&(rv->thread), attrp, func, arg))) {
        *err = bc_error_new_printf(BC_ERROR_THREAD,
            "Failed to create thread: %s", strerror(r));
        free(rv);
        return NULL;
    }

    return rv;
}


void
bc_thread_join(bc_thread_t *thread, bc_error_t **err)
{
    if (thread == NULL || err == NULL || *err != NULL)
        return;

    int r;

    if (0 != (r = pthread_join(thread->thread, NULL))) {
        *err = bc_error_new_printf(BC_ERROR_THREAD,
            "Failed to join thread: %s", strerror(r));
    }
}


void
bc_thread_free(bc_thread_t *thread)
{
    free(thread);
}
