/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2019 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifndef ___RUSAGE_H
#define ___RUSAGE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#ifdef HAVE_SYS_TIME_H
#ifdef HAVE_SYS_RESOURCE_H
#define HAVE_RUSAGE 1
#endif
#endif

#include "../common/utils.h"

typedef struct {
    long long cpu_time;  // in microseconds
    long memory;         // in kilobytes
} blogc_rusage_t;

blogc_rusage_t* blogc_rusage_get(void);

char* blogc_rusage_format_cpu_time(long long time);
char* blogc_rusage_format_memory(long mem);

void blogc_rusage_inject(bc_trie_t *global);

#endif /* ___RUSAGE_H */
