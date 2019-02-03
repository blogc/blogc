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
