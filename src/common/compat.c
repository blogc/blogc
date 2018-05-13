/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2019 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif /* HAVE_SYS_WAIT_H */

#include <signal.h>
#include "compat.h"


int
bc_compat_status_code(int waitstatus)
{
    int rv = waitstatus;
#if defined(WIFEXITED) && defined(WEXITSTATUS) && defined(WIFSIGNALED) && defined(WTERMSIG)
    if (WIFEXITED(waitstatus)) {
        rv = WEXITSTATUS(waitstatus);
    }
    else if (WIFSIGNALED(waitstatus)) {
        rv = WTERMSIG(waitstatus);
        rv += 128;
    }
#elif defined(WIN32) || defined(_WIN32)
    if (waitstatus == 3) {
        rv = SIGTERM + 128;  // can't get signal on windows.
    }
#endif
    return rv;
}
