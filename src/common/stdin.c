// SPDX-FileCopyrightText: 2014-2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
// SPDX-License-Identifier: BSD-3-Clause

#include <stdbool.h>
#include <stdio.h>
#include "utils.h"
#include "stdin.h"


// splitted in single file to make it easier to test
char*
bc_stdin_read(size_t *len)
{
    if (len == NULL)
        return NULL;

    int c;
    bc_string_t *rv = bc_string_new();
    while (EOF != (c = fgetc(stdin)))
        bc_string_append_c(rv, c);
    *len = rv->len;
    return bc_string_free(rv, false);
}
