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

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif /* HAVE_SYS_TIME_H */

#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif /* HAVE_SYS_RESOURCE_H */

#include <stdlib.h>
#include <squareball.h>

#include "rusage.h"


blogc_rusage_t*
blogc_rusage_get(void)
{
#ifndef HAVE_RUSAGE
    return NULL;
#else
    struct rusage usage;
    if (0 != getrusage(RUSAGE_SELF, &usage))
        return NULL;

    blogc_rusage_t *rv = sb_malloc(sizeof(blogc_rusage_t));
    rv->cpu_time = (
        (usage.ru_utime.tv_sec * 1000000) + usage.ru_utime.tv_usec +
        (usage.ru_stime.tv_sec * 1000000) + usage.ru_stime.tv_usec);
    rv->memory = usage.ru_maxrss;

    return rv;
#endif
}


char*
blogc_rusage_format_cpu_time(long long time)
{
    if (time >= 1000000)
        return sb_strdup_printf("%.3fs", ((float) time) / 1000000.0);

    // this is a special case: some systems may report the cpu time rounded up to the
    // milisecond. it is useless to show ".000" in this case.
    if (time >= 1000)
        return sb_strdup_printf("%.*fms", time % 1000 ? 3 : 0, ((float) time) / 1000.0);

    return sb_strdup_printf("%dus", time);
}


char*
blogc_rusage_format_memory(long mem)
{
    if (mem >= 1048576)
        return sb_strdup_printf("%.3fGB", ((float) mem) / 1048576.0);
    if (mem >= 1024)
        return sb_strdup_printf("%.3fMB", ((float) mem) / 1024.0);
    return sb_strdup_printf("%dKB", mem);
}


void
blogc_rusage_inject(sb_trie_t *global)
{
    blogc_rusage_t *usage = blogc_rusage_get();
    if (usage == NULL)
        return;

    sb_trie_insert(global, "BLOGC_RUSAGE_CPU_TIME",
        blogc_rusage_format_cpu_time(usage->cpu_time));
    sb_trie_insert(global, "BLOGC_RUSAGE_MEMORY",
        blogc_rusage_format_memory(usage->memory));

    free(usage);
}
