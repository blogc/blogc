// SPDX-FileCopyrightText: 2014-2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <stddef.h>
#include <stdbool.h>
#include "../common/utils.h"

char* blogc_slugify(const char *str);
char* blogc_htmlentities(const char *str);
char* blogc_fix_description(const char *paragraph);
char* blogc_content_parse_inline(const char *src);
bool blogc_is_ordered_list_item(const char *str, size_t prefix_len);
char* blogc_content_parse(const char *src, size_t *end_excerpt,
    char **first_header, char **description, char **endl,
    bc_slist_t **headers);
