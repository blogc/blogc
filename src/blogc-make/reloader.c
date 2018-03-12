/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2017 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "../common/utils.h"
#include "ctx.h"
#include "rules.h"
#include "reloader.h"

// we are not going to unit-test these functions, then printing errors
// directly is not a big issue


static void*
bm_reloader_thread(void *arg)
{
    bm_reloader_t *reloader = arg;
    while (reloader->running) {
        if (!bm_ctx_reload(&(reloader->ctx))) {
            fprintf(stderr, "blogc-make: warning: failed to reload context. "
                "retrying in 5 seconds ...\n\n");
            sleep(5);
            continue;
        }
        if (0 != reloader->rule_exec(reloader->ctx, reloader->outputs, reloader->args)) {
            fprintf(stderr, "blogc-make: warning: failed to rebuild website. "
                "retrying in 5 seconds ...\n\n");
            sleep(5);
            continue;
        }
        sleep(1);
    }

    free(reloader);
    return NULL;
}


bm_reloader_t*
bm_reloader_new(bm_ctx_t *ctx, bm_rule_exec_func_t rule_exec,
    bc_slist_t *outputs, bc_trie_t *args)
{
    // first rule_exec call is syncronous, to do a 'sanity check'
    if (0 != rule_exec(ctx, outputs, args))
        return NULL;

    int err;

    pthread_attr_t attr;
    if (0 != (err = pthread_attr_init(&attr))) {
        fprintf(stderr, "blogc-make: error: failed to initialize reloader "
            "thread attributes: %s\n", strerror(err));
        return NULL;
    }

    // we run the thread detached, because we don't want to wait it to join
    // before exiting. the OS can clean it properly
    if (0 != (err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED))) {
        fprintf(stderr, "blogc-make: error: failed to mark reloader thread as "
            "detached: %s\n", strerror(err));
        return NULL;
    }

    bm_reloader_t *rv = bc_malloc(sizeof(bm_reloader_t));
    rv->ctx = ctx;
    rv->rule_exec = rule_exec;
    rv->outputs = outputs;
    rv->args = args;
    rv->running = true;

    pthread_t thread;
    if (0 != (err = pthread_create(&thread, &attr, bm_reloader_thread, rv))) {
        fprintf(stderr, "blogc-make: error: failed to create reloader "
            "thread: %s\n", strerror(err));
        free(rv);
        return NULL;
    }

    return rv;
}


void
bm_reloader_stop(bm_reloader_t *reloader)
{
    if (reloader == NULL)
        return;
    reloader->running = false;
}
