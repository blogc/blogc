// SPDX-FileCopyrightText: 2014-2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <stdbool.h>
#include "../common/error.h"
#include "ctx.h"

int bm_exec_native_cp(bm_filectx_t *source, bm_filectx_t *dest, bool verbose);
bool bm_exec_native_is_empty_dir(const char *dir, bc_error_t **err);
int bm_exec_native_rm(const char *output_dir, bm_filectx_t *dest, bool verbose);
