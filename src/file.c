/*
 * blogc: A blog compiler.
 * Copyright (C) 2015-2016 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "file.h"
#include "error.h"
#include "utf8.h"
#include "utils.h"

// this would belong to loader.c, but we need it in a separated file to be
// able to mock it when unit testing the loader functions.


char*
blogc_file_get_contents(const char *path, size_t *len, blogc_error_t **err)
{
    if (path == NULL || err == NULL || *err != NULL)
        return NULL;

    *len = 0;
    FILE *fp = fopen(path, "r");

    if (fp == NULL) {
        int tmp_errno = errno;
        *err = blogc_error_new_printf(BLOGC_ERROR_FILE,
            "Failed to open file (%s): %s", path, strerror(tmp_errno));
        return NULL;
    }

    sb_string_t *str = sb_string_new();
    char buffer[BLOGC_FILE_CHUNK_SIZE];
    char *tmp;

    while (!feof(fp)) {
        size_t read_len = fread(buffer, sizeof(char), BLOGC_FILE_CHUNK_SIZE, fp);

        tmp = buffer;

        if (str->len == 0 && read_len > 0) {
            // skipping BOM before validation, for performance. should be safe
            // enough
            size_t skip = blogc_utf8_skip_bom((uint8_t*) buffer, read_len);
            read_len -= skip;
            tmp += skip;
        }

        *len += read_len;
        sb_string_append_len(str, tmp, read_len);
    }
    fclose(fp);

    if (!blogc_utf8_validate_str(str)) {
        *err = blogc_error_new_printf(BLOGC_ERROR_FILE,
            "File content is not valid UTF-8: %s", path);
        sb_string_free(str, true);
        return NULL;
    }

    return sb_string_free(str, false);
}


int
blogc_fprintf(FILE *stream, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    int rv = vfprintf(stream, format, ap);
    va_end(ap);
    return rv;
}
