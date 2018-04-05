/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2017 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifndef _MAKE_ATOM_H
#define _MAKE_ATOM_H

#include <squareball.h>

#include "settings.h"

char* bm_atom_deploy(bm_settings_t *settings, sb_error_t **err);
void bm_atom_destroy(const char *fname);

#endif /* _MAKE_ATOM_H */
