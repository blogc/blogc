/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2019 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include "../common/utils.h"
#include "ctx.h"
#include "rules.h"
#include "reloader.h"

// we are not going to unit-test these functions, then printing errors
// directly is not a big issue

static pthread_mutex_t mutex_running = PTHREAD_MUTEX_INITIALIZER;
static bool running = false;
static int reloader_status_code = 0;
static void (*handler_func)(int) = NULL;

static void
sig_handler(int signum)
{
    bm_reloader_stop(signum + 128);
}


int
bm_reloader_run(bm_ctx_t **ctx, bm_rule_exec_func_t rule_exec,
    bc_slist_t *outputs, bc_trie_t *args)
{
    // install ^C handler
    struct sigaction current_action;
    if (sigaction(SIGINT, NULL, &current_action) < 0) {
        fprintf(stderr, "blogc-make: failed to run reloader: %s\n", strerror(errno));
        return 1;
    }
    if (current_action.sa_handler != sig_handler) {  // not installed yet
        // backup current handler
        pthread_mutex_lock(&mutex_running);
        handler_func = current_action.sa_handler;
        pthread_mutex_unlock(&mutex_running);

        // set new handler
        struct sigaction new_action;
        new_action.sa_handler = sig_handler;
        sigemptyset(&new_action.sa_mask);
        new_action.sa_flags = 0;
        if (sigaction(SIGINT, &new_action, NULL) < 0) {
            fprintf(stderr, "blogc-make: failed to run reloader: %s\n",
                strerror(errno));
            return 1;
        }
    }

    pthread_mutex_lock(&mutex_running);
    running = true;
    pthread_mutex_unlock(&mutex_running);

    while (running) {
        if (!bm_ctx_reload(ctx)) {
            fprintf(stderr, "blogc-make: warning: failed to reload context. "
                "retrying in 5 seconds ...\n\n");
            sleep(5);
            continue;
        }
        if (0 != rule_exec(*ctx, outputs, args)) {
            fprintf(stderr, "blogc-make: warning: failed to rebuild website. "
                "retrying in 5 seconds ...\n\n");
            sleep(5);
            continue;
        }
        sleep(1);
    }

    return reloader_status_code;
}


void
bm_reloader_stop(int status_code)
{
    pthread_mutex_lock(&mutex_running);

    running = false;
    reloader_status_code = status_code;

    // reraise if SIGINT
    if (status_code == SIGINT + 128) {

        // reinstall old ^C handler
        struct sigaction new_action;
        new_action.sa_handler = handler_func;
        sigemptyset(&new_action.sa_mask);
        new_action.sa_flags = 0;
        sigaction(SIGINT, &new_action, NULL);

        // run it
        raise(SIGINT);

        // SIGINT will usually kill the process, but in the case that the
        // `handler_func` prevents it, our custom handler will be reinstalled
        // by `bm_reloader_run`.
    }

    pthread_mutex_unlock(&mutex_running);
}
