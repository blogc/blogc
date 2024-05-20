// SPDX-FileCopyrightText: 2014-2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
// SPDX-License-Identifier: BSD-3-Clause

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */

#ifdef HAVE_TIME_H
#include <time.h>
#endif /* HAVE_TIME_H */

#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "../common/error.h"
#include "../common/file.h"
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

#ifdef HAVE_NETDB_H
    struct hostent *h = gethostbyname(buf);
    if (h != NULL && h->h_name != NULL)
        return bc_strdup(h->h_name);
#endif

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
    return bc_strdup(getenv("LOGNAME"));
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
    if (t == NULL)
        return NULL;

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


// it is obviously impossible that the same process runs inside and outside
// docker at the same time, then an unprotected global variable should be fine
// here
static bool inside_docker_evaluated = false;
static bool inside_docker = false;

bool
blogc_sysinfo_get_inside_docker(void)
{
    if (inside_docker_evaluated)
        return inside_docker;
    inside_docker_evaluated = true;

    size_t len;
    bc_error_t *err = NULL;
    char *contents = bc_file_get_contents("/proc/1/cgroup", false, &len, &err);
    if (err != NULL) {
        bc_error_free(err);
        inside_docker = false;
        return inside_docker;
    }

    bool inside_docker = NULL != strstr(contents, "/docker/");
    free(contents);
    return inside_docker;
}


void
blogc_sysinfo_inject_inside_docker(bc_trie_t *global)
{
    if (blogc_sysinfo_get_inside_docker())
        bc_trie_insert(global, "BLOGC_SYSINFO_INSIDE_DOCKER", bc_strdup("1"));
}
