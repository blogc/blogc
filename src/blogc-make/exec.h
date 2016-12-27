/*
 * blogc: A blog compiler.
 * Copyright (C) 2015-2016 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifndef _MAKE_EXEC_H
#define _MAKE_EXEC_H

#include "../common/error.h"
#include "../common/utils.h"
#include "settings.h"

int bm_exec_command(const char *cmd, const char *input, char **output,
    char **error, bc_error_t **err);
char* bm_exec_build_blogc_cmd(bm_settings_t *settings, bc_trie_t *variables,
    bool listing, const char *template, const char *output, bool sources_stdin);
int bm_exec_blogc(bm_settings_t *settings, bc_trie_t *variables, bool listing,
    bm_filectx_t *template, bm_filectx_t *output, bc_slist_t *sources,
    bool verbose, bool only_first_source);
int bm_exec_blogc_runserver(bm_settings_t *settings, bool verbose);

#endif /* _MAKE_EXEC_H */
