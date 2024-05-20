// SPDX-FileCopyrightText: 2014-2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "../common/error.h"
#include "settings.h"

char* bm_atom_generate(bm_settings_t *settings);
char* bm_atom_deploy(bm_settings_t *settings, bc_error_t **err);
void bm_atom_destroy(const char *fname);
