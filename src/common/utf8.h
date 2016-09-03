/*
 * blogc: A blog compiler.
 * Copyright (C) 2015-2016 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifndef _UTF_8_H
#define _UTF_8_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "utils.h"

bool blogc_utf8_validate(const uint8_t *str, size_t len);
bool blogc_utf8_validate_str(bc_string_t *str);
size_t blogc_utf8_skip_bom(const uint8_t *str, size_t len);

#endif /* _UTF_8_H */
