// SPDX-FileCopyrightText: 2014-2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

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
