// SPDX-FileCopyrightText: 2014-2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <stddef.h>
#include <stdbool.h>
#include "error.h"

#define BC_FILE_CHUNK_SIZE 1024

char* bc_file_get_contents(const char *path, bool utf8, size_t *len, bc_error_t **err);
