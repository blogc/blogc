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


const static blogc_directive_t registry[] = {
    {"youtube", blogc_directive_youtube},
    {NULL, NULL},
};


char*
blogc_directive_loader(blogc_directive_ctx_t *ctx, blogc_error_t **err)
{
    if (ctx == NULL)
        return NULL;
    if (err == NULL || *err != NULL)
        return NULL;
    for (unsigned int i = 0; registry[i].name != NULL; i++)
        if (0 == strcmp(ctx->name, registry[i].name))
            return registry[i].callback(ctx, err);
    *err = blogc_error_new_printf(BLOGC_WARNING_CONTENT_PARSER,
        "Directive not found: %s", ctx->name);
    return NULL;
}


char*
blogc_directive_youtube(blogc_directive_ctx_t *ctx, blogc_error_t **err)
{
    if (ctx->argument == NULL) {
        *err = blogc_error_new_printf(BLOGC_WARNING_CONTENT_PARSER,
            "youtube: video ID must be provided as argument");
        return NULL;
    }

    char *width = b_trie_lookup(ctx->params, "width");
    char *height = b_trie_lookup(ctx->params, "height");

    // using default 16:9 sizes provided by youtube as of 2015-11-04
    return b_strdup_printf(
        "<iframe width=\"%s\" height=\"%s\" "
        "src=\"https://www.youtube.com/embed/%s\" frameborder=\"0\" "
        "allowfullscreen></iframe>%s",
        width == NULL ? "560" : width,
        height == NULL ? "315" : height,
        ctx->argument, ctx->eol);
}
