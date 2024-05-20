// SPDX-FileCopyrightText: 2014-2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "../common/config-parser.h"

const char* bgr_settings_get_base_dir(void);
char* bgr_settings_get_builds_dir(void);
char* bgr_settings_get_section(bc_config_t *config, const char *repo_path);
bc_config_t* bgr_settings_parse(void);
