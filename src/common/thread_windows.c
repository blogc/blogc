/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2018 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#include <stdbool.h>
#include <windows.h>
#include "error.h"
#include "utils.h"
#include "thread.h"

struct _bc_thread_t {
    HANDLE thread;
};

struct wrapper_arg {
    bc_thread_func_t func;
    void *arg;
};


static DWORD WINAPI
wrapper(LPVOID arg)
{
    if (arg != NULL) {
        struct wrapper_arg *r = arg;
        r->func(r->arg);
        free(r);
    }
    return 0;
}


bc_thread_t*
bc_thread_create(bc_thread_func_t func, void *arg, bool detached, bc_error_t **err)
{
    if (err == NULL || *err != NULL)
        return NULL;

    struct wrapper_arg *a = bc_malloc(sizeof(struct wrapper_arg));
    a->func = func;
    a->arg = arg;

    bc_thread_t *rv = bc_malloc(sizeof(bc_thread_t));
    rv->thread = CreateThread(NULL, 0, wrapper, a, 0, NULL);
    if (rv->thread == NULL) {
        LPTSTR buf;
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR)&buf, 0, NULL);
        *err = bc_error_new_printf(BC_ERROR_THREAD,
            "Failed to create thread: %s", buf);
        LocalFree(buf);
        free(rv);
        free(a);
        return NULL;
    }

    if (detached && !CloseHandle(rv->thread)) {
        LPTSTR buf;
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR)&buf, 0, NULL);
        *err = bc_error_new_printf(BC_ERROR_THREAD,
            "Failed to detach thread: %s", buf);
        LocalFree(buf);
        free(rv);
        free(a);
        return NULL;
    }

    return rv;
}


void
bc_thread_join(bc_thread_t *thread, bc_error_t **err)
{
    if (thread == NULL || err == NULL || *err != NULL)
        return;

    DWORD r = WaitForSingleObject(thread->thread, INFINITE);
    switch (r) {
        case WAIT_OBJECT_0:
            break;
        case WAIT_FAILED:
        {
            LPTSTR buf;
            FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR)&buf, 0, NULL);
            *err = bc_error_new_printf(BC_ERROR_THREAD,
                "Failed to join thread: %s", buf);
            LocalFree(buf);
            break;
        }
        default:
            *err = bc_error_new_printf(BC_ERROR_THREAD, "Failed to join thread");
            break;
    }
}


void
bc_thread_free(bc_thread_t *thread)
{
    free(thread);
}
