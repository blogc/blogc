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

    {NULL, NULL},
};


char*
blogc_funcvars_lookup(const char *name, bc_trie_t *global)
{
    // protect against evaluating the same function twice in the same global
    // context
    if (NULL != bc_trie_lookup(global, name))
        return NULL;

    for (size_t i = 0; funcs[i].variable != NULL; i++) {
        if (0 == strcmp(name, funcs[i].variable)) {
            funcs[i].func(global);
            return bc_strdup(bc_trie_lookup(global, name));
        }
    }

    return NULL;
}
