/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2018 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <libgen.h>
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "../common/error.h"
#include "../common/file.h"
#include "../common/utils.h"
#include "atom.h"
#include "settings.h"
#include "exec.h"
#include "utils.h"
#include "ctx.h"


bm_filectx_t*
bm_filectx_new(bm_ctx_t *ctx, const char *filename, const char *slug,
    struct stat *st)
{
    if (ctx == NULL || filename == NULL)
        return NULL;

    char *f = filename[0] == '/' ? bc_strdup(filename) :
        bc_strdup_printf("%s/%s", ctx->root_dir, filename);

    bm_filectx_t *rv = bc_malloc(sizeof(bm_filectx_t));
    rv->path = f;
    rv->short_path = bc_strdup(filename);
    rv->slug = bc_strdup(slug);

    if (st == NULL) {
        struct stat buf;

        if (0 != stat(f, &buf)) {
            rv->tv_sec = 0;
            rv->tv_nsec = 0;
            rv->readable = false;
            return rv;
        }

        st = &buf;
    }

    // if it isn't NULL the file exists for sure
    rv->tv_sec = st->st_mtim_tv_sec;
    rv->tv_nsec = st->st_mtim_tv_nsec;
    rv->readable = true;
    return rv;
}


bc_slist_t*
bm_filectx_new_r(bc_slist_t *l, bm_ctx_t *ctx, const char *filename)
{
    if (ctx == NULL || filename == NULL)
        return NULL;

    char *f = filename[0] == '/' ? bc_strdup(filename) :
        bc_strdup_printf("%s/%s", ctx->root_dir, filename);

    struct stat buf;
    if (0 != stat(f, &buf)) {
        free(f);
        return l;
    }

    if (S_ISDIR(buf.st_mode)) {
        DIR *dir = opendir(f);
        if (dir == NULL) {
            free(f);
            return l;
        }

        struct dirent *e;
        while (NULL != (e = readdir(dir))) {
            if ((0 == strcmp(e->d_name, ".")) || (0 == strcmp(e->d_name, "..")))
                continue;
            char *tmp = bc_strdup_printf("%s/%s", filename, e->d_name);
            l = bm_filectx_new_r(l, ctx, tmp);
            free(tmp);
        }

        closedir(dir);
        free(f);
        return l;
    }

    l = bc_slist_append(l, bm_filectx_new(ctx, filename, NULL, &buf));
    free(f);
    return l;
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
    free(fctx->slug);
    free(fctx);
}


bm_ctx_t*
bm_ctx_new(bm_ctx_t *base, const char *settings_file, const char *argv0,
    bc_error_t **err)
{
    if (settings_file == NULL || err == NULL || *err != NULL)
        return NULL;

    char real_filename[PATH_MAX];
    if (NULL == realpath(settings_file, real_filename)) {
        *err = bc_error_new_printf(BLOGC_MAKE_ERROR_SETTINGS,
            "Failed to resolve settings file (%s): %s", settings_file,
            strerror(errno));
        return NULL;
    }

    size_t content_len;
    char *content = bc_file_get_contents(real_filename, true, &content_len,
        err);
    if (*err != NULL)
        return NULL;

    bm_settings_t *settings = bm_settings_parse(content, content_len, err);
    if (settings == NULL || *err != NULL) {
        free(content);
        return NULL;
    }
    free(content);

    const char *template_dir = bc_trie_lookup(settings->settings, "template_dir");
    if (template_dir == NULL)
        template_dir = "";

    char *atom_template = NULL;
    bool atom_template_tmp = false;
    const char *atom_template_conf = bc_trie_lookup(settings->settings,
        "atom_template");
    if (atom_template_conf != NULL) {
        atom_template = bc_strdup_printf("%s/%s", template_dir, atom_template_conf);
    }
    else {
        atom_template = bm_atom_deploy(settings, err);
        atom_template_tmp = true;
        if (*err != NULL) {
            return NULL;
        }
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

    rv->settings_fctx = bm_filectx_new(rv, real_filename, NULL, NULL);
    rv->root_dir = bc_strdup(dirname(real_filename));

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

    char *main_template = bc_strdup_printf("%s/%s", template_dir,
        bm_ctx_settings_lookup(rv, "main_template"));
    rv->main_template_fctx = bm_filectx_new(rv, main_template, NULL, NULL);
    free(main_template);

    rv->atom_template_tmp = atom_template_tmp;
    rv->atom_template_fctx = bm_filectx_new(rv, atom_template, NULL, NULL);
    free(atom_template);

    const char *content_dir = bm_ctx_settings_lookup(rv, "content_dir");
    const char *post_prefix = bm_ctx_settings_lookup(rv, "post_prefix");
    const char *source_ext = bm_ctx_settings_lookup(rv, "source_ext");

    rv->posts_fctx = NULL;
    if (settings->posts != NULL) {
        for (size_t i = 0; settings->posts[i] != NULL; i++) {
            char *f = bm_generate_filename(content_dir, post_prefix,
                settings->posts[i], source_ext);
            rv->posts_fctx = bc_slist_append(rv->posts_fctx,
                bm_filectx_new(rv, f, settings->posts[i], NULL));
            free(f);
        }
    }

    rv->pages_fctx = NULL;
    if (settings->pages != NULL) {
        for (size_t i = 0; settings->pages[i] != NULL; i++) {
            char *f = bm_generate_filename(content_dir, NULL, settings->pages[i],
                source_ext);
            rv->pages_fctx = bc_slist_append(rv->pages_fctx,
                bm_filectx_new(rv, f, settings->pages[i], NULL));
            free(f);
        }
    }

    rv->copy_fctx = NULL;
    if (settings->copy != NULL) {
        for (size_t i = 0; settings->copy[i] != NULL; i++) {
            rv->copy_fctx = bm_filectx_new_r(rv->copy_fctx, rv,
                settings->copy[i]);
        }
    }

    return rv;
}


bool
bm_ctx_reload(bm_ctx_t **ctx)
{
    if (*ctx == NULL || (*ctx)->settings_fctx == NULL)
        return false;

    if (bm_filectx_changed((*ctx)->settings_fctx, NULL, NULL)) {
        // reload everything! we could just reload settings_fctx, as this
        // would force rebuilding everything, but we need to know new/deleted
        // files

        // needs to dup path, because it may be freed when reloading.
        char *tmp = bc_strdup((*ctx)->settings_fctx->path);
        bc_error_t *err = NULL;
        *ctx = bm_ctx_new(*ctx, tmp, NULL, &err);
        free(tmp);
        if (err != NULL) {
            bc_error_print(err, "blogc-make");
            bc_error_free(err);
            return false;
        }
        return true;
    }

    bm_filectx_reload((*ctx)->main_template_fctx);
    bm_filectx_reload((*ctx)->atom_template_fctx);

    for (bc_slist_t *tmp = (*ctx)->posts_fctx; tmp != NULL; tmp = tmp->next)
        bm_filectx_reload((bm_filectx_t*) tmp->data);

    for (bc_slist_t *tmp = (*ctx)->pages_fctx; tmp != NULL; tmp = tmp->next)
        bm_filectx_reload((bm_filectx_t*) tmp->data);

    for (bc_slist_t *tmp = (*ctx)->copy_fctx; tmp != NULL; tmp = tmp->next)
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

    if (ctx->atom_template_tmp)
        bm_atom_destroy(ctx->atom_template_fctx->path);
    ctx->atom_template_tmp = false;

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


const char*
bm_ctx_settings_lookup(bm_ctx_t *ctx, const char *key)
{
    if (ctx == NULL || ctx->settings == NULL || ctx->settings->settings == NULL)
        return NULL;
    return bc_trie_lookup(ctx->settings->settings, key);
}


const char*
bm_ctx_settings_lookup_str(bm_ctx_t *ctx, const char *key)
{
    const char *rv = bm_ctx_settings_lookup(ctx, key);
    return rv == NULL ? "" : rv;
}
