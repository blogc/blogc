// SPDX-FileCopyrightText: 2014-2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "../common/error.h"

char* bm_generate_filename(const char *dir, const char *prefix, const char *fname,
    const char *ext);
char* bm_generate_filename2(const char *dir, const char *prefix, const char *fname,
    const char *prefix2, const char *fname2, const char *ext);
char* bm_abspath(const char *path, bc_error_t **err);
