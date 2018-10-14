/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2018 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifndef _SETTINGS_H
#define _SETTINGS_H

#include "../common/config-parser.h"

const char* bgr_settings_get_base_dir(void);
char* bgr_settings_get_builds_dir(void);
char* bgr_settings_get_section(bc_config_t *config, const char *repo_path);
bc_config_t* bgr_settings_parse(void);

#endif /* _SETTINGS_H */
