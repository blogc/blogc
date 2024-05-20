// SPDX-FileCopyrightText: 2014-2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "../common/error.h"

char* blogc_convert_datetime(const char *orig, const char *format,
    bc_error_t **err);
