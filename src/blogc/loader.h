/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2017 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifndef _LOADER_H
#define _LOADER_H

#include <squareball.h>

char* blogc_get_filename(const char *f);
sb_slist_t* blogc_template_parse_from_file(const char *f, sb_error_t **err);
sb_trie_t* blogc_source_parse_from_file(const char *f, sb_error_t **err);
sb_slist_t* blogc_source_parse_from_files(sb_trie_t *conf, sb_slist_t *l,
    sb_error_t **err);

#endif /* _LOADER_H */
