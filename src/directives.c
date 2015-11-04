/*
 * blogc: A blog compiler.
 * Copyright (C) 2015 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include "utils/utils.h"


char*
blogc_directive_loader(const char *name, const char *argument, b_trie_t *params)
{
    // TODO: implement me!
    return b_strdup("TODO\n");
}
