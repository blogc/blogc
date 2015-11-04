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

#include <string.h>

#include "utils/utils.h"
#include "directives.h"
#include "error.h"


blogc_directive_t registry[] = {
    {"youtube", blogc_directive_youtube},
    {NULL, NULL},
};


char*
blogc_directive_loader(const char *name, const char *argument, b_trie_t *params,
    blogc_error_t **err)
{
    for (unsigned int i = 0; registry[i].name != NULL; i++)
        if (0 == strcmp(name, registry[i].name))
            return registry[i].callback(argument, params, err);
    return NULL;
}


char*
blogc_directive_youtube(const char *argument, b_trie_t *params, blogc_error_t **err)
{
    if (err != NULL && *err != NULL)
        return NULL;
    if (argument == NULL) {
        *err = blogc_error_new_printf(BLOGC_WARNING_CONTENT_PARSER,
            "youtube: Invalid video ID: %s", argument);
        return NULL;
    }

    char *width = b_trie_lookup(params, "width");
    char *height = b_trie_lookup(params, "height");

    // using default 16:9 sizes provided by youtube as of 2015-11-04
    return b_strdup_printf(
        "<iframe width=\"%s\" height=\"%s\" "
        "src=\"https://www.youtube.com/embed/%s\" frameborder=\"0\" "
        "allowfullscreen></iframe>\n",
        width == NULL ? "560" : width,
        height == NULL ? "315" : height,
        argument);
}
