/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2020 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "../common/utils.h"
#include "ctx.h"
#include "exec.h"
#include "reloader.h"
#include "httpd.h"

// we are not going to unit-test these functions, then printing errors
// directly is not a big issue


typedef struct {
    bm_ctx_t *ctx;
    bc_trie_t *args;
} bm_httpd_t;

static pthread_mutex_t mutex_httpd_starting = PTHREAD_MUTEX_INITIALIZER;
static bool httpd_starting = false;


static void*
httpd_thread(void *arg)
{
    bm_httpd_t *httpd = arg;

    pthread_mutex_lock(&mutex_httpd_starting);
    httpd_starting = true;
    pthread_mutex_unlock(&mutex_httpd_starting);

    int rv = bm_exec_blogc_runserver(httpd->ctx, bc_trie_lookup(httpd->args, "host"),
        bc_trie_lookup(httpd->args, "port"), bc_trie_lookup(httpd->args, "threads"));

    pthread_mutex_lock(&mutex_httpd_starting);
    httpd_starting = false;
    pthread_mutex_unlock(&mutex_httpd_starting);

    free(httpd);

    // stop the reloader
    bm_reloader_stop(rv);

    return NULL;
}


int
bm_httpd_run(bm_ctx_t **ctx, bm_rule_exec_func_t rule_exec, bc_slist_t *outputs,
    bc_trie_t *args)
{
    pthread_mutex_lock(&mutex_httpd_starting);
    bool starting = httpd_starting;
    pthread_mutex_unlock(&mutex_httpd_starting);

    if (starting) {
        fprintf(stderr, "blogc-make: error: httpd already running\n");
        return 1;
    }

    int err;

    pthread_attr_t attr;
    if (0 != (err = pthread_attr_init(&attr))) {
        fprintf(stderr, "blogc-make: error: failed to initialize httpd "
            "thread attributes: %s\n", strerror(err));
        return 1;
    }

    // we run the thread detached, because we don't want to wait it to join
    // before exiting. the OS can clean it properly
    if (0 != (err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED))) {
        fprintf(stderr, "blogc-make: error: failed to mark httpd thread as "
            "detached: %s\n", strerror(err));
        return 1;
    }

    bm_httpd_t *rv = bc_malloc(sizeof(bm_httpd_t));
    rv->ctx = *ctx;
    rv->args = args;

    pthread_t thread;
    if (0 != (err = pthread_create(&thread, &attr, httpd_thread, rv))) {
        fprintf(stderr, "blogc-make: error: failed to create httpd "
            "thread: %s\n", strerror(err));
        free(rv);
        return 1;
    }

    // we could use some pthread_*_timedwait() apis here, but I decided to
    // just use simple mutexes for the sake of portability.
    size_t count = 0;
    while (true) {
        pthread_mutex_lock(&mutex_httpd_starting);
        starting = httpd_starting;
        pthread_mutex_unlock(&mutex_httpd_starting);

        if (starting)
            break;

        if (++count > 100) {
            fprintf(stderr, "blogc-make: error: failed to start httpd thread: "
                "too many retries\n");
            // rv will leak, but it is not safe to free here
            return 1;
        }
        usleep(100000);
    }

    return bm_reloader_run(ctx, rule_exec, outputs, args);
}
