/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2017 Rafael G. Martins <rafael@rafaelmartins.eng.br>
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
#include "exec.h"
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
        rv->tv_sec = 0;
        rv->tv_nsec = 0;
        rv->readable = false;
    }
    else {
        rv->tv_sec = buf.st_mtim_tv_sec;
        rv->tv_nsec = buf.st_mtim_tv_nsec;
        rv->readable = true;
    }

    return rv;
}


bool
bm_filectx_changed(bm_filectx_t *ctx, time_t *tv_sec, long *tv_nsec)
{
    if (ctx == NULL)
        return false;

    struct stat buf;

    if (0 == stat(ctx->path, &buf)) {
        if (buf.st_mtim_tv_sec == ctx->tv_sec) {
            if (buf.st_mtim_tv_nsec > ctx->tv_nsec) {
                if (tv_sec != NULL)
                    *tv_sec = buf.st_mtim_tv_sec;
                if (tv_nsec != NULL)
                    *tv_nsec = buf.st_mtim_tv_nsec;
                return true;
            }
        }
        else if (buf.st_mtim_tv_sec > ctx->tv_sec) {
            if (tv_sec != NULL)
                *tv_sec = buf.st_mtim_tv_sec;
            if (tv_nsec != NULL)
                *tv_nsec = buf.st_mtim_tv_nsec;
            return true;
        }
    }

    return false;
}


void
bm_filectx_reload(bm_filectx_t *ctx)
{
    if (ctx == NULL)
        return;

    time_t tv_sec;
    long tv_nsec;

    if (!bm_filectx_changed(ctx, &tv_sec, &tv_nsec))
        return;

    ctx->tv_sec = tv_sec;
    ctx->tv_nsec = tv_nsec;
    ctx->readable = true;
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
bm_ctx_new(bm_ctx_t *base, const char *settings_file, const char *argv0,
    bc_error_t **err)
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

    char *atom_template = bm_atom_deploy(settings, err);
    if (*err != NULL) {
        return NULL;
    }

    bm_ctx_t *rv = NULL;
    if (base == NULL) {
        rv = bc_malloc(sizeof(bm_ctx_t));
        rv->blogc = bm_exec_find_binary(argv0, "blogc", "BLOGC");
        rv->blogc_runserver = bm_exec_find_binary(argv0, "blogc-runserver",
            "BLOGC_RUNSERVER");
        rv->dev = false;
        rv->verbose = false;
    }
    else {
        bm_ctx_free_internal(base);
        rv = base;
    }
    rv->settings = settings;

    char *real_filename = realpath(settings_file, NULL);
    rv->settings_fctx = bm_filectx_new(rv, real_filename);
    rv->root_dir = realpath(dirname(real_filename), NULL);
    free(real_filename);

    const char *output_dir = getenv("OUTPUT_DIR");
    rv->short_output_dir = bc_strdup(output_dir != NULL ? output_dir : "_build");

    if (rv->short_output_dir[0] == '/') {
        rv->output_dir = bc_strdup(rv->short_output_dir);
    }
    else {
        rv->output_dir = bc_strdup_printf("%s/%s", rv->root_dir,
            rv->short_output_dir);
    }

    // can't return null and set error after this!

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

    rv->copy_fctx = NULL;
    if (settings->copy != NULL) {
        for (size_t i = 0; settings->copy[i] != NULL; i++) {
            rv->copy_fctx = bc_slist_append(rv->copy_fctx,
                bm_filectx_new(rv, settings->copy[i]));
        }
    }

    return rv;
}


bool
bm_ctx_reload(bm_ctx_t *ctx)
{
    if (ctx == NULL || ctx->settings_fctx == NULL)
        return false;

    if (bm_filectx_changed(ctx->settings_fctx, NULL, NULL)) {
        // reload everything! we could just reload settings_fctx, as this
        // would force rebuilding everything, but we need to know new/deleted
        // files

        // needs to dup path, because it may be freed when reloading.
        char *tmp = bc_strdup(ctx->settings_fctx->path);
        bc_error_t *err = NULL;
        ctx = bm_ctx_new(ctx, tmp, NULL, &err);
        free(tmp);
        if (err != NULL) {
            bc_error_print(err, "blogc-make");
            bc_error_free(err);
            return false;
        }
        return true;
    }

    bm_filectx_reload(ctx->main_template_fctx);
    bm_filectx_reload(ctx->atom_template_fctx);

    for (bc_slist_t *tmp = ctx->posts_fctx; tmp != NULL; tmp = tmp->next)
        bm_filectx_reload((bm_filectx_t*) tmp->data);

    for (bc_slist_t *tmp = ctx->pages_fctx; tmp != NULL; tmp = tmp->next)
        bm_filectx_reload((bm_filectx_t*) tmp->data);

    for (bc_slist_t *tmp = ctx->copy_fctx; tmp != NULL; tmp = tmp->next)
        bm_filectx_reload((bm_filectx_t*) tmp->data);

    return true;
}


void
bm_ctx_free_internal(bm_ctx_t *ctx)
{
    if (ctx == NULL)
        return;

    bm_settings_free(ctx->settings);
    ctx->settings = NULL;

    free(ctx->root_dir);
    ctx->root_dir = NULL;
    free(ctx->short_output_dir);
    ctx->short_output_dir = NULL;
    free(ctx->output_dir);
    ctx->output_dir = NULL;

    bm_atom_destroy(ctx->atom_template_fctx->path);

    bm_filectx_free(ctx->main_template_fctx);
    ctx->main_template_fctx = NULL;
    bm_filectx_free(ctx->atom_template_fctx);
    ctx->atom_template_fctx = NULL;
    bm_filectx_free(ctx->settings_fctx);
    ctx->settings_fctx = NULL;

    bc_slist_free_full(ctx->posts_fctx, (bc_free_func_t) bm_filectx_free);
    ctx->posts_fctx = NULL;
    bc_slist_free_full(ctx->pages_fctx, (bc_free_func_t) bm_filectx_free);
    ctx->pages_fctx = NULL;
    bc_slist_free_full(ctx->copy_fctx, (bc_free_func_t) bm_filectx_free);
    ctx->copy_fctx = NULL;
}


void
bm_ctx_free(bm_ctx_t *ctx)
{
    if (ctx == NULL)
        return;
    bm_ctx_free_internal(ctx);
    free(ctx->blogc);
    free(ctx->blogc_runserver);
    free(ctx);
}
