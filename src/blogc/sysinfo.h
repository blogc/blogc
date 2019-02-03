/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2019 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifndef ___SYSINFO_H
#define ___SYSINFO_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#ifdef HAVE_UNISTD_H
#define HAVE_SYSINFO_HOSTNAME 1
#endif /* HAVE_UNISTD_H */

#include "../common/utils.h"

char* blogc_sysinfo_get_hostname(void);
void blogc_sysinfo_inject_hostname(bc_trie_t *global);

#endif /* ___SYSINFO_H */
