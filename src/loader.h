/*
 * blogc: A blog compiler.
 * Copyright (C) 2015 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file COPYING.
 */

#ifndef _LOADER_H
#define _LOADER_H

#include "utils/utils.h"
#include "error.h"

#define BLOGC_FILE_CHUNK_SIZE 1024

char* blogc_file_get_contents(const char *path, size_t *len, blogc_error_t **err);
b_slist_t* blogc_template_parse_from_file(const char *f, blogc_error_t **err);
b_trie_t* blogc_source_parse_from_file(const char *f, blogc_error_t **err);
b_slist_t* blogc_source_parse_from_files(b_slist_t *l, blogc_error_t **err);

#endif /* _LOADER_H */
