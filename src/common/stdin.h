/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2020 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifndef _STDIN_H
#define _STDIN_H

#include <stddef.h>

char* bc_stdin_read(size_t *len);

#endif /* _STDIN_H */
