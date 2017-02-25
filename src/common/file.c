/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2017 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "file.h"
#include "error.h"
#include "utf8.h"
#include "utils.h"


char*
bc_file_get_contents(const char *path, bool utf8, size_t *len, bc_error_t **err)
{
    if (path == NULL || err == NULL || *err != NULL)
        return NULL;

    *len = 0;
    FILE *fp = fopen(path, "r");

    if (fp == NULL) {
        int tmp_errno = errno;
        *err = bc_error_new_printf(BC_ERROR_FILE,
            "Failed to open file (%s): %s", path, strerror(tmp_errno));
        return NULL;
    }

    bc_string_t *str = bc_string_new();
    char buffer[BC_FILE_CHUNK_SIZE];
    char *tmp;

    while (!feof(fp)) {
        size_t read_len = fread(buffer, sizeof(char), BC_FILE_CHUNK_SIZE, fp);

        tmp = buffer;

        if (utf8 && str->len == 0 && read_len > 0) {
            // skipping BOM before validation, for performance. should be safe
            // enough
            size_t skip = bc_utf8_skip_bom((uint8_t*) buffer, read_len);
            read_len -= skip;
            tmp += skip;
        }

        *len += read_len;
        bc_string_append_len(str, tmp, read_len);
    }
    fclose(fp);

    if (utf8 && !bc_utf8_validate_str(str)) {
        *err = bc_error_new_printf(BC_ERROR_FILE,
            "File content is not valid UTF-8: %s", path);
        bc_string_free(str, true);
        return NULL;
    }

    return bc_string_free(str, false);
}
