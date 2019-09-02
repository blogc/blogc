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
#ifdef HAVE_GETHOSTNAME
#define HAVE_SYSINFO_HOSTNAME 1
#endif /* HAVE_GETHOSTNAME */
#endif /* HAVE_UNISTD_H */

#ifdef HAVE_TIME_H
#define HAVE_SYSINFO_DATETIME 1
#endif /* HAVE_TIME_H */

#include <stdbool.h>
#include <squareball.h>

char* blogc_sysinfo_get_hostname(void);
void blogc_sysinfo_inject_hostname(sb_trie_t *global);
char* blogc_sysinfo_get_username(void);
void blogc_sysinfo_inject_username(sb_trie_t *global);
char* blogc_sysinfo_get_datetime(void);
void blogc_sysinfo_inject_datetime(sb_trie_t *global);
bool blogc_sysinfo_get_inside_docker(void);
void blogc_sysinfo_inject_inside_docker(sb_trie_t *global);

#endif /* ___SYSINFO_H */
