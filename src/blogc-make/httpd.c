/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2019 Rafael G. Martins <rafael@rafaelmartins.eng.br>
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


static void*
httpd_thread(void *arg)
{
    bm_httpd_t *httpd = arg;

    int rv = bm_exec_blogc_runserver(httpd->ctx, bc_trie_lookup(httpd->args, "host"),
        bc_trie_lookup(httpd->args, "port"), bc_trie_lookup(httpd->args, "threads"));

    free(httpd);

    // stop the reloader
    bm_reloader_stop(rv);

    return NULL;
}


int
bm_httpd_run(bm_ctx_t **ctx, bm_rule_exec_func_t rule_exec, bc_slist_t *outputs,
    bc_trie_t *args)
{
    // this is here to avoid that the httpd starts running in the middle of the
    // first build, as the reloader and the httpd are started in parallel.
    // we run the task synchronously for the first time, and start the httpd
    // thread afterwards.
    bool wait_before_reloader = false;
    if (0 != rule_exec(*ctx, outputs, args)) {
        fprintf(stderr, "blogc-make: warning: failed to rebuild website. "
            "retrying in 5 seconds ...\n\n");
        wait_before_reloader = true;
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

    // run the reloader
    if (wait_before_reloader) {
        sleep(5);
    }
    return bm_reloader_run(ctx, rule_exec, outputs, args);
}
