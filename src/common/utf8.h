// SPDX-FileCopyrightText: 2014-2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "utils.h"

bool bc_utf8_validate(const uint8_t *str, size_t len);
bool bc_utf8_validate_str(bc_string_t *str);
size_t bc_utf8_skip_bom(const uint8_t *str, size_t len);
