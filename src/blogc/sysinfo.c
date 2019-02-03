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

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */

#ifdef HAVE_PWD_H
#include <pwd.h>
#endif /* HAVE_PWD_H */

#ifdef HAVE_TIME_H
#include <time.h>
#endif /* HAVE_TIME_H */

#include <stdlib.h>
#include "../common/utils.h"
#include "sysinfo.h"


char*
blogc_sysinfo_get_hostname(void)
{
#ifndef HAVE_SYSINFO_HOSTNAME
    return NULL;
#else
    char buf[1024];  // could be 256 according to gethostname(2), but *shrug*.
    buf[1023] = '\0';
    if (-1 == gethostname(buf, 1024))
        return NULL;

    // FIXME: return FQDN instead of local host name
    return bc_strdup(buf);
#endif
}


void
blogc_sysinfo_inject_hostname(bc_trie_t *global)
{
    char *hostname = blogc_sysinfo_get_hostname();
    if (hostname == NULL)
        return;

    bc_trie_insert(global, "BLOGC_SYSINFO_HOSTNAME", hostname);
}


char*
blogc_sysinfo_get_username(void)
{
#ifndef HAVE_SYSINFO_USERNAME
    return NULL;
#else
    uid_t u = geteuid();
    struct passwd *p = getpwuid(u);
    if (p == NULL)
        return NULL;

    return bc_strdup(p->pw_name);
#endif
}


void
blogc_sysinfo_inject_username(bc_trie_t *global)
{
    char *username = blogc_sysinfo_get_username();
    if (username == NULL)
        return;

    bc_trie_insert(global, "BLOGC_SYSINFO_USERNAME", username);
}


char*
blogc_sysinfo_get_datetime(void)
{
#ifndef HAVE_SYSINFO_DATETIME
    return NULL;
#else
    time_t tmp;
    if (-1 == time(&tmp))
        return NULL;

    struct tm *t = gmtime(&tmp);

    char buf[1024];
    if (0 == strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", t))
        return NULL;

    return bc_strdup(buf);
#endif
}


void
blogc_sysinfo_inject_datetime(bc_trie_t *global)
{
    char *t = blogc_sysinfo_get_datetime();
    if (t == NULL)
        return;

    bc_trie_insert(global, "BLOGC_SYSINFO_DATETIME", t);
}
