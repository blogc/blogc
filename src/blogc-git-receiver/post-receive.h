/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2017 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifndef _POST_RECEIVE_H
#define _POST_RECEIVE_H

#include "../common/config-parser.h"

char* bgr_post_receive_get_config_section(bc_config_t *config,
    const char *repo_path, const char *home);
int bgr_post_receive_hook(int argc, char *argv[]);

#endif /* _POST_RECEIVE_H */
