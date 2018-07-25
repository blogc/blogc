/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2017 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifndef _MAKE_CTX_H
#define _MAKE_CTX_H

#include <sys/stat.h>
#include <stdbool.h>
#include <time.h>
#include "settings.h"
#include "../common/error.h"
#include "../common/utils.h"

#ifdef __APPLE__
#define st_mtim_tv_sec st_mtimespec.tv_sec
#define st_mtim_tv_nsec st_mtimespec.tv_nsec
#endif

#ifdef __ANDROID__
#define st_mtim_tv_sec st_mtime
#define st_mtim_tv_nsec st_mtime_nsec
#endif

#ifndef st_mtim_tv_sec
#define st_mtim_tv_sec st_mtim.tv_sec
#endif
#ifndef st_mtim_tv_nsec
#define st_mtim_tv_nsec st_mtim.tv_nsec
#endif


typedef struct {
    char *path;
    char *short_path;
    char *slug;
    time_t tv_sec;
    long tv_nsec;
    bool readable;
} bm_filectx_t;

typedef struct {
    char *blogc;
    char *blogc_runserver;

    bool dev;
    bool verbose;

    bm_settings_t *settings;

    char *root_dir;
    char *output_dir;
    char *short_output_dir;

    bm_filectx_t *main_template_fctx;
    bm_filectx_t *atom_template_fctx;
    bm_filectx_t *settings_fctx;

    bc_slist_t *posts_fctx;
    bc_slist_t *pages_fctx;
    bc_slist_t *copy_fctx;
} bm_ctx_t;

bm_filectx_t* bm_filectx_new(bm_ctx_t *ctx, const char *filename, const char *slug,
    struct stat *st);
bc_slist_t* bm_filectx_new_r(bc_slist_t *l, bm_ctx_t *ctx, const char *filename);
bool bm_filectx_changed(bm_filectx_t *ctx, time_t *tv_sec, long *tv_nsec);
void bm_filectx_reload(bm_filectx_t *ctx);
void bm_filectx_free(bm_filectx_t *fctx);
bm_ctx_t* bm_ctx_new(bm_ctx_t *base, const char *settings_file,
    const char *argv0, bc_error_t **err);
bool bm_ctx_reload(bm_ctx_t **ctx);
void bm_ctx_free_internal(bm_ctx_t *ctx);
void bm_ctx_free(bm_ctx_t *ctx);
const char* bm_ctx_settings_lookup(bm_ctx_t *ctx, const char *key);
const char* bm_ctx_settings_lookup_str(bm_ctx_t *ctx, const char *key);

#endif /* _MAKE_CTX_H */
