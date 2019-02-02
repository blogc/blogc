/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2019 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifndef ___RUSAGE_H
#define ___RUSAGE_H

#ifdef HAVE_SYS_RESOURCE_H
#define HAVE_RUSAGE
#endif

typedef struct {
    long long cpu_time;  // in microseconds
    long memory;         // in kilobytes
} blogc_rusage_t;

blogc_rusage_t* blogc_rusage_get(void);
long long blogc_rusage_get_cpu_time(void);  // in microseconds
long blogc_rusage_get_memory(void);         // in kilobytes

char* blogc_rusage_format_cpu_time(long long time);
char* blogc_rusage_format_memory(long mem);

char* blogc_rusage_cpu_time(void);
char* blogc_rusage_memory(void);

#endif /* ___RUSAGE_H */
