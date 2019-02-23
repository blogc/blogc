/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2019 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "funcvars.h"
#include "rusage.h"
#include "sysinfo.h"
#include "../common/utils.h"


static const struct func_map {
    const char *variable;
    const blogc_funcvars_func_t func;
} funcs[] = {

#ifdef HAVE_RUSAGE
    {"BLOGC_RUSAGE_CPU_TIME", blogc_rusage_inject},
    {"BLOGC_RUSAGE_MEMORY", blogc_rusage_inject},
#endif

#ifdef HAVE_SYSINFO_HOSTNAME
    {"BLOGC_SYSINFO_HOSTNAME", blogc_sysinfo_inject_hostname},
#endif

#ifdef HAVE_SYSINFO_DATETIME
    {"BLOGC_SYSINFO_DATETIME", blogc_sysinfo_inject_datetime},
#endif

    {"BLOGC_SYSINFO_USERNAME", blogc_sysinfo_inject_username},
    {"BLOGC_SYSINFO_INSIDE_DOCKER", blogc_sysinfo_inject_inside_docker},
    {NULL, NULL},
};


void
blogc_funcvars_eval(bc_trie_t *global, const char *name)
{
    if (global == NULL || name == NULL)
        return;

    // protect against evaluating the same function twice in the same global
    // context
    if (NULL != bc_trie_lookup(global, name))
        return;

    for (size_t i = 0; funcs[i].variable != NULL; i++) {
        if (0 == strcmp(name, funcs[i].variable)) {
            funcs[i].func(global);
            return;
        }
    }

    return;
}
