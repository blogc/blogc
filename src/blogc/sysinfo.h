// SPDX-FileCopyrightText: 2014-2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

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
#include "../common/utils.h"

char* blogc_sysinfo_get_hostname(void);
void blogc_sysinfo_inject_hostname(bc_trie_t *global);
char* blogc_sysinfo_get_username(void);
void blogc_sysinfo_inject_username(bc_trie_t *global);
char* blogc_sysinfo_get_datetime(void);
void blogc_sysinfo_inject_datetime(bc_trie_t *global);
bool blogc_sysinfo_get_inside_docker(void);
void blogc_sysinfo_inject_inside_docker(bc_trie_t *global);
