/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2019 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#include <sass.h>
#include <errno.h>
#include <libgen.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ctx.h"
#include "exec-native.h"
#include "rules.h"
#include "./sass.h"
#include "../common/utils.h"


bc_slist_t*
bm_sass_outputlist(bm_ctx_t *ctx)
{
    if (ctx == NULL || ctx->settings == NULL || ctx->settings->sass == NULL)
        return NULL;

    bc_slist_t *rv = NULL;

    for (size_t i = 0; ctx->settings->sass[i] != NULL; i++) {
        char *t = bc_strdup(ctx->settings->sass[i]);
        for (int i = strlen(t); i >= 0 ; i--) {
            if (t[i] == '.') {
                t[i] = '\0';
                break;
            }
        }

        char *f = bc_strdup_printf("%s/%s.css", ctx->short_output_dir, t);
        free(t);
        rv = bc_slist_append(rv, bm_filectx_new(ctx, f, NULL, NULL));
        free(f);
    }

    return rv;
}


int
bm_sass_exec(bm_ctx_t *ctx, bc_slist_t *outputs, bc_trie_t *args)
{
    if (ctx == NULL || ctx->settings == NULL || ctx->settings->sass == NULL)
        return 0;

    int rv = 0;

    size_t i = 0;
    bc_slist_t *o = outputs;

    for (; ctx->settings->sass[i] != NULL, o != NULL; i++, o = o->next) {
        bm_filectx_t *o_fctx = o->data;
        if (o_fctx == NULL)
            continue;

        if (bc_str_starts_with(basename(ctx->settings->sass[i]), "_")) {
            fprintf(stderr, "blogc-make: error: sass: Mixins must be included "
                "by other files (%s)\n", ctx->settings->sass[i]);
            return 3;
        }
        if (!bc_str_ends_with(ctx->settings->sass[i], ".scss")) {
            fprintf(stderr, "blogc-make: error: sass: Only .scss Sass sources "
                "are supported (%s)\n", ctx->settings->sass[i]);
            return 3;
        }

        bm_filectx_t *f = bm_filectx_new(ctx, ctx->settings->sass[i], NULL, NULL);

        struct Sass_File_Context *fctx = sass_make_file_context(f->path);
        struct Sass_Context *sctx = sass_file_context_get_context(fctx);
        struct Sass_Compiler *comp = sass_make_file_compiler(fctx);
        struct Sass_Options *opts = sass_context_get_options(sctx);
        sass_option_set_output_style(opts, SASS_STYLE_COMPRESSED);
        char *sass_dir = bc_strdup_printf("%s/sass", ctx->root_dir);
        sass_option_push_include_path(opts, sass_dir);
        free(sass_dir);
        sass_dir = bc_strdup_printf("%s/_sass", ctx->root_dir);
        sass_option_push_include_path(opts, sass_dir);
        free(sass_dir);

        bc_slist_t *sources = NULL;

        rv = sass_compiler_parse(comp);
        if (rv != 0 || 0 != sass_context_get_error_status(sctx)) {
            fprintf(stderr, "blogc-make: error: sass: %s\n%s",
                o_fctx->path, sass_context_get_error_message(sctx));
            rv = 3;
            goto clean;
        }

        char **inc = sass_context_get_included_files(sctx);
        for (size_t i = 0; inc != NULL && inc[i] != NULL; i++) {
            sources = bc_slist_append(sources, bm_filectx_new(ctx, inc[i], NULL, NULL));
        }

        if (!bm_rule_need_rebuild(sources, ctx->settings_fctx, NULL, NULL, o_fctx, false))
            goto clean;

        if (ctx->verbose)
            printf("Compiling '%s' to '%s'\n", f->path, o_fctx->path);
        else
            printf("  SASS     %s\n", o_fctx->short_path);
        fflush(stdout);

        rv = sass_compiler_execute(comp);
        if (rv != 0 || 0 != sass_context_get_error_status(sctx)) {
            fprintf(stderr, "blogc-make: error: sass: %s\n%s",
                o_fctx->path, sass_context_get_error_message(sctx));
            rv = 3;
            goto clean;
        }

        rv = bm_exec_native_mkdir_p(o_fctx->path);
        if (rv != 0)
            goto clean;

        FILE *fp = fopen(o_fctx->path, "w");
        if (fp == NULL) {
            fprintf(stderr, "blogc-make: error: sass: failed to open output "
                "file (%s): %s\n", o_fctx->path, strerror(errno));
            rv = 3;
            goto clean;
        }

        fputs(sass_context_get_output_string(sctx), fp);
        fclose(fp);

clean:
        bm_filectx_free(f);
        bc_slist_free_full(sources, (bc_free_func_t) bm_filectx_free);
        sass_delete_compiler(comp);
        sass_delete_file_context(fctx);

        if (rv != 0)
            return rv;
    }

    return 0;
}
