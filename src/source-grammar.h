/*
 * blogc: A blog compiler.
 * Copyright (C) 2015 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the LGPL-2 License.
 * See the file COPYING.
 */

#ifndef _SOURCE_GRAMAR_H
#define _SOURCE_GRAMAR_H

#include "utils/utils.h"

typedef struct {
    b_trie_t *config;
    char *content;
} blogc_source_t;

blogc_source_t* blogc_source_parse(const char *tmpl);
void blogc_source_free(blogc_source_t *source);

#endif /* _SOURCE_GRAMAR_H */
