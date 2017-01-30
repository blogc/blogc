/*
 * blogc: A blog compiler.
 * Copyright (C) 2016 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#include <sys/stat.h>
#include <libgen.h>
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../common/error.h"
#include "../common/file.h"
#include "../common/utils.h"
#include "atom.h"
#include "settings.h"
#include "ctx.h"


bm_filectx_t*
bm_filectx_new(bm_ctx_t *ctx, const char *filename)
{
    if (ctx == NULL || filename == NULL)
        return NULL;

    char *f = filename[0] == '/' ? bc_strdup(filename) :
        bc_strdup_printf("%s/%s", ctx->root_dir, filename);

    bm_filectx_t *rv = bc_malloc(sizeof(bm_filectx_t));
    rv->path = f;
    rv->short_path = bc_strdup(filename);

    struct stat buf;

    if (0 != stat(f, &buf)) {
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 0;
        rv->timestamp = ts;
        rv->readable = false;
    }
    else {
        rv->timestamp = buf.st_mtim;
        rv->readable = true;
    }

    return rv;
}


void
bm_filectx_free(bm_filectx_t *fctx)
{
    if (fctx == NULL)
        return;
    free(fctx->path);
    free(fctx->short_path);
    free(fctx);
}


bm_ctx_t*
bm_ctx_new(const char *settings_file, bc_error_t **err)
{
    if (settings_file == NULL || err == NULL || *err != NULL)
        return NULL;

    size_t content_len;
    char *content = bc_file_get_contents(settings_file, true, &content_len,
        err);
    if (*err != NULL)
        return NULL;

    bm_settings_t *settings = bm_settings_parse(content, content_len, err);
    if (*err != NULL) {
        free(content);
        return NULL;
    }
    free(content);

    // fix output_dir, if forced from environment variable
    const char *output_dir_env = getenv("OUTPUT_DIR");
    if (output_dir_env != NULL) {
        bc_trie_insert(settings->settings, "output_dir",
            bc_strdup(output_dir_env));
    }

    char *atom_template = bm_atom_deploy(settings, err);
    if (*err != NULL) {
        return NULL;
    }

    bm_ctx_t *rv = bc_malloc(sizeof(bm_ctx_t));
    rv->settings = settings;

    char *real_filename = realpath(settings_file, NULL);
    rv->settings_fctx = bm_filectx_new(rv, real_filename);
    rv->root_dir = realpath(dirname(real_filename), NULL);
    free(real_filename);

    const char *output_dir = bc_trie_lookup(settings->settings, "output_dir");
    if (output_dir[0] == '/') {
        rv->output_dir = bc_strdup(output_dir);
    }
    else {
        rv->output_dir = bc_strdup_printf("%s/%s", rv->root_dir, output_dir);
    }

    const char *template_dir = bc_trie_lookup(settings->settings,
        "template_dir");

    char *main_template = bc_strdup_printf("%s/%s", template_dir,
        bc_trie_lookup(settings->settings, "main_template"));
    rv->main_template_fctx = bm_filectx_new(rv, main_template);
    free(main_template);

    rv->atom_template_fctx = bm_filectx_new(rv, atom_template);
    free(atom_template);

    const char *content_dir = bc_trie_lookup(settings->settings, "content_dir");
    const char *post_prefix = bc_trie_lookup(settings->settings, "post_prefix");
    const char *source_ext = bc_trie_lookup(settings->settings, "source_ext");

    rv->posts_fctx = NULL;
    if (settings->posts != NULL) {
        for (size_t i = 0; settings->posts[i] != NULL; i++) {
            char *f = bc_strdup_printf("%s/%s/%s%s", content_dir, post_prefix,
                settings->posts[i], source_ext);
            rv->posts_fctx = bc_slist_append(rv->posts_fctx,
                bm_filectx_new(rv, f));
            free(f);
        }
    }

    rv->pages_fctx = NULL;
    if (settings->pages != NULL) {
        for (size_t i = 0; settings->pages[i] != NULL; i++) {
            char *f = bc_strdup_printf("%s/%s%s", content_dir,
                settings->pages[i], source_ext);
            rv->pages_fctx = bc_slist_append(rv->pages_fctx,
                bm_filectx_new(rv, f));
            free(f);
        }
    }

    rv->copy_files_fctx = NULL;
    if (settings->copy_files != NULL) {
        for (size_t i = 0; settings->copy_files[i] != NULL; i++) {
            rv->copy_files_fctx = bc_slist_append(rv->copy_files_fctx,
                bm_filectx_new(rv, settings->copy_files[i]));
        }
    }

    return rv;
}


void
bm_ctx_free(bm_ctx_t *ctx)
{
    if (ctx == NULL)
        return;

    bm_settings_free(ctx->settings);

    free(ctx->root_dir);
    free(ctx->output_dir);

    bm_atom_destroy(ctx->atom_template_fctx->path);

    bm_filectx_free(ctx->main_template_fctx);
    bm_filectx_free(ctx->atom_template_fctx);
    bm_filectx_free(ctx->settings_fctx);

    bc_slist_free_full(ctx->posts_fctx, (bc_free_func_t) bm_filectx_free);
    bc_slist_free_full(ctx->pages_fctx, (bc_free_func_t) bm_filectx_free);
    bc_slist_free_full(ctx->copy_files_fctx, (bc_free_func_t) bm_filectx_free);

    free(ctx);
}
