/*
 * blogc: A blog compiler.
 * Copyright (C) 2015 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file COPYING.
 */

#ifndef _RENDERER_H
#define _RENDERER_H

#include "utils/utils.h"

char*
blogc_render(b_slist_t *tmpl, b_slist_t *sources);

#endif /* _RENDERER_H */
