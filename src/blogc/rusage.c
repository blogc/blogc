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

#include "../common/utils.h"
#include "rusage.h"

// FIXME: optimize to use a single syscall for both cpu time and memory?


#include <stdio.h>
long long
blogc_rusage_get_cpu_time(void)
{
    struct rusage usage;
    if (0 != getrusage(RUSAGE_SELF, &usage))
        return 0;
    return (
        (usage.ru_utime.tv_sec * 1000000) + usage.ru_utime.tv_usec +
        (usage.ru_stime.tv_sec * 1000000) + usage.ru_stime.tv_usec);
}


char*
blogc_rusage_format_cpu_time(long long time)
{
    if (time > 1000000)
        return bc_strdup_printf("%.3s", ((float) time) / 1000000.0);

    // this is a special case: some systems may report the cpu time rounded up to the
    // milisecond. it is useless to show ".000" in this case.
    if (time > 1000)
        return bc_strdup_printf("%.*fms", time % 1000 ? 3 : 0, ((float) time) / 1000.0);

    return bc_strdup_printf("%dus", time);
}


char*
blogc_rusage_cpu_time(void)
{
    return blogc_rusage_format_cpu_time(blogc_rusage_get_cpu_time());
}


long
blogc_rusage_get_memory(void)
{
    struct rusage usage;
    if (0 != getrusage(RUSAGE_SELF, &usage))
        return 0;
    return usage.ru_maxrss;
}


char*
blogc_rusage_format_memory(long mem)
{
    if (mem > 1048576)
        return bc_strdup_printf("%.3fGB", ((float) mem) / 1048576.0);
    if (mem > 1024)
        return bc_strdup_printf("%.3fMB", ((float) mem) / 1024.0);
    return bc_strdup_printf("%dKB", mem);
}


char*
blogc_rusage_memory(void)
{
    return blogc_rusage_format_memory(blogc_rusage_get_memory());
}
