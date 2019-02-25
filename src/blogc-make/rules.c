/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2019 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "../common/utils.h"
#include "atom.h"
#include "ctx.h"
#include "exec.h"
#include "exec-native.h"
#include "httpd.h"
#include "reloader.h"
#include "settings.h"
#include "utils.h"
#include "rules.h"

#ifdef USE_LIBSASS
#include "./sass.h"
#endif


static void
posts_ordering(bm_ctx_t *ctx, bc_trie_t *variables, const char *variable)
{
    if (ctx == NULL)
        return;  // something is wrong, let's not add any variable

    const char *value = bm_ctx_settings_lookup_str(ctx, variable);
    bool asc = 0 == strcasecmp(value, "asc");
    bool sort = bc_str_to_bool(bm_ctx_settings_lookup(ctx, "posts_sort"));

    if (sort) {
        bc_trie_insert(variables, "FILTER_SORT", bc_strdup("1"));
    }

    if ((sort && asc) || (!sort && !asc)) {
        bc_trie_insert(variables, "FILTER_REVERSE", bc_strdup("1"));
    }
}


static void
posts_pagination(bm_ctx_t *ctx, bc_trie_t *variables, const char *variable)
{
    if (ctx == NULL)
        return;  // something is wrong, let's not add any variable

    long posts_per_page = strtol(bm_ctx_settings_lookup_str(ctx, variable), NULL, 10);
    if (posts_per_page >= 0) {
        bc_trie_insert(variables, "FILTER_PAGE", bc_strdup("1"));
        bc_trie_insert(variables, "FILTER_PER_PAGE",
            bc_strdup(bm_ctx_settings_lookup(ctx, variable)));
    }
}


static bool
posts_pagination_enabled(bm_ctx_t *ctx, const  char *variable)
{
    if (ctx == NULL)
        return false;

    long posts_per_page = strtol(bm_ctx_settings_lookup_str(ctx, variable), NULL, 10);
    return posts_per_page != 0;
}


// INDEX RULE

static bc_slist_t*
index_outputlist(bm_ctx_t *ctx)
{
    if (ctx == NULL || ctx->settings->posts == NULL)
        return NULL;

    if (!posts_pagination_enabled(ctx, "posts_per_page"))
        return NULL;

    bc_slist_t *rv = NULL;

    const char *index_prefix = bm_ctx_settings_lookup(ctx, "index_prefix");
    const char *html_ext = bm_ctx_settings_lookup(ctx, "html_ext");

    char *f = bm_generate_filename(ctx->short_output_dir, index_prefix, NULL,
        html_ext);
    rv = bc_slist_append(rv, bm_filectx_new(ctx, f, NULL, NULL));
    free(f);

    return rv;
}

static int
index_exec(bm_ctx_t *ctx, bc_slist_t *outputs, bc_trie_t *args)
{
    if (ctx == NULL || ctx->settings->posts == NULL)
        return 0;

    int rv = 0;

    bc_trie_t *variables = bc_trie_new(free);
    posts_pagination(ctx, variables, "posts_per_page");
    posts_ordering(ctx, variables, "html_order");
    bc_trie_insert(variables, "DATE_FORMAT",
        bc_strdup(bm_ctx_settings_lookup_str(ctx, "date_format")));
    bc_trie_insert(variables, "MAKE_RULE", bc_strdup("index"));
    bc_trie_insert(variables, "MAKE_TYPE", bc_strdup("post"));

    for (bc_slist_t *l = outputs; l != NULL; l = l->next) {
        bm_filectx_t *fctx = l->data;
        if (fctx == NULL)
            continue;
        if (bm_rule_need_rebuild(ctx->posts_fctx, ctx->settings_fctx,
                ctx->listing_entry_fctx, ctx->main_template_fctx, fctx, false))
        {
            rv = bm_exec_blogc(ctx, variables, NULL, true, ctx->listing_entry_fctx,
                ctx->main_template_fctx, fctx, ctx->posts_fctx, false);
            if (rv != 0)
                break;
        }
    }

    bc_trie_free(variables);

    return rv;
}


// ATOM RULE

static bc_slist_t*
atom_outputlist(bm_ctx_t *ctx)
{
    if (ctx == NULL || ctx->settings->posts == NULL)
        return NULL;

    if (!posts_pagination_enabled(ctx, "atom_posts_per_page"))
        return NULL;

    bc_slist_t *rv = NULL;

    const char *atom_prefix = bm_ctx_settings_lookup(ctx, "atom_prefix");
    const char *atom_ext = bm_ctx_settings_lookup(ctx, "atom_ext");

    char *f = bm_generate_filename(ctx->short_output_dir, atom_prefix, NULL,
        atom_ext);
    rv = bc_slist_append(rv, bm_filectx_new(ctx, f, NULL, NULL));
    free(f);

    return rv;
}

static int
atom_exec(bm_ctx_t *ctx, bc_slist_t *outputs, bc_trie_t *args)
{
    if (ctx == NULL || ctx->settings->posts == NULL)
        return 0;

    int rv = 0;

    bc_trie_t *variables = bc_trie_new(free);
    posts_pagination(ctx, variables, "atom_posts_per_page");
    posts_ordering(ctx, variables, "atom_order");
    bc_trie_insert(variables, "DATE_FORMAT", bc_strdup("%Y-%m-%dT%H:%M:%SZ"));
    bc_trie_insert(variables, "MAKE_RULE", bc_strdup("atom"));
    bc_trie_insert(variables, "MAKE_TYPE", bc_strdup("atom"));

    for (bc_slist_t *l = outputs; l != NULL; l = l->next) {
        bm_filectx_t *fctx = l->data;
        if (fctx == NULL)
            continue;
        if (bm_rule_need_rebuild(ctx->posts_fctx, ctx->settings_fctx, NULL, NULL,
                fctx, false))
        {
            rv = bm_exec_blogc(ctx, variables, NULL, true, NULL, ctx->atom_template_fctx,
                fctx, ctx->posts_fctx, false);
            if (rv != 0)
                break;
        }
    }

    bc_trie_free(variables);

    return rv;
}


// ATOM TAGS RULE

static bc_slist_t*
atom_tags_outputlist(bm_ctx_t *ctx)
{
    if (ctx == NULL || ctx->settings->posts == NULL || ctx->settings->tags == NULL)
        return NULL;

    if (!posts_pagination_enabled(ctx, "atom_posts_per_page"))
        return NULL;

    bc_slist_t *rv = NULL;

    const char *atom_prefix = bm_ctx_settings_lookup(ctx, "atom_prefix");
    const char *atom_ext = bm_ctx_settings_lookup(ctx, "atom_ext");

    for (size_t i = 0; ctx->settings->tags[i] != NULL; i++) {
        char *f = bm_generate_filename(ctx->short_output_dir, atom_prefix,
            ctx->settings->tags[i], atom_ext);
        rv = bc_slist_append(rv, bm_filectx_new(ctx, f, NULL, NULL));
        free(f);
    }

    return rv;
}

static int
atom_tags_exec(bm_ctx_t *ctx, bc_slist_t *outputs, bc_trie_t *args)
{
    if (ctx == NULL || ctx->settings->posts == NULL || ctx->settings->tags == NULL)
        return 0;

    int rv = 0;
    size_t i = 0;

    bc_trie_t *variables = bc_trie_new(free);
    posts_pagination(ctx, variables, "atom_posts_per_page");
    posts_ordering(ctx, variables, "atom_order");
    bc_trie_insert(variables, "DATE_FORMAT", bc_strdup("%Y-%m-%dT%H:%M:%SZ"));
    bc_trie_insert(variables, "MAKE_RULE", bc_strdup("atom_tags"));
    bc_trie_insert(variables, "MAKE_TYPE", bc_strdup("atom"));

    for (bc_slist_t *l = outputs; l != NULL; l = l->next, i++) {
        bm_filectx_t *fctx = l->data;
        if (fctx == NULL)
            continue;

        bc_trie_insert(variables, "FILTER_TAG",
            bc_strdup(ctx->settings->tags[i]));

        if (bm_rule_need_rebuild(ctx->posts_fctx, ctx->settings_fctx, NULL, NULL,
                fctx, false))
        {
            rv = bm_exec_blogc(ctx, variables, NULL, true, NULL, ctx->atom_template_fctx,
                fctx, ctx->posts_fctx, false);
            if (rv != 0)
                break;
        }
    }

    bc_trie_free(variables);

    return rv;
}


// PAGINATION RULE

static bc_slist_t*
pagination_outputlist(bm_ctx_t *ctx)
{
    if (ctx == NULL || ctx->settings->posts == NULL)
        return NULL;

    if (!posts_pagination_enabled(ctx, "posts_per_page"))
        return NULL;

    bc_trie_t *variables = bc_trie_new(free);
    posts_pagination(ctx, variables, "posts_per_page");

    char *last_page = bm_exec_blogc_get_variable(ctx, variables, NULL, "LAST_PAGE",
        true, ctx->posts_fctx, false);

    bc_trie_free(variables);

    if (last_page == NULL)
        return NULL;

    long pages = strtol(last_page, NULL, 10);
    free(last_page);

    bc_slist_t *rv = NULL;

    const char *pagination_prefix = bm_ctx_settings_lookup(ctx, "pagination_prefix");
    const char *html_ext = bm_ctx_settings_lookup(ctx, "html_ext");

    for (size_t i = 0; i < pages; i++) {
        char *j = bc_strdup_printf("%d", i + 1);
        char *f = bm_generate_filename(ctx->short_output_dir, pagination_prefix,
            j, html_ext);
        rv = bc_slist_append(rv, bm_filectx_new(ctx, f, NULL, NULL));
        free(j);
        free(f);
    }

    return rv;
}

static int
pagination_exec(bm_ctx_t *ctx, bc_slist_t *outputs, bc_trie_t *args)
{
    if (ctx == NULL || ctx->settings->posts == NULL)
        return 0;

    int rv = 0;
    size_t page = 1;

    bc_trie_t *variables = bc_trie_new(free);
    // not using posts_pagination because we set FILTER_PAGE anyway, and the
    // first value inserted in that function would be useless
    bc_trie_insert(variables, "FILTER_PER_PAGE",
        bc_strdup(bm_ctx_settings_lookup_str(ctx, "posts_per_page")));
    posts_ordering(ctx, variables, "html_order");
    bc_trie_insert(variables, "DATE_FORMAT",
        bc_strdup(bm_ctx_settings_lookup_str(ctx, "date_format")));
    bc_trie_insert(variables, "MAKE_RULE", bc_strdup("pagination"));
    bc_trie_insert(variables, "MAKE_TYPE", bc_strdup("post"));

    for (bc_slist_t *l = outputs; l != NULL; l = l->next, page++) {
        bm_filectx_t *fctx = l->data;
        if (fctx == NULL)
            continue;
        bc_trie_insert(variables, "FILTER_PAGE", bc_strdup_printf("%zu", page));
        if (bm_rule_need_rebuild(ctx->posts_fctx, ctx->settings_fctx,
                ctx->listing_entry_fctx, ctx->main_template_fctx, fctx, false))
        {
            rv = bm_exec_blogc(ctx, variables, NULL, true, ctx->listing_entry_fctx,
                ctx->main_template_fctx, fctx, ctx->posts_fctx, false);
            if (rv != 0)
                break;
        }
    }

    bc_trie_free(variables);

    return rv;
}


// PAGINATION TAGS RULE

static bc_slist_t*
pagination_tags_outputlist(bm_ctx_t *ctx)
{
    if (ctx == NULL || ctx->settings->posts == NULL || ctx->settings->tags == NULL)
        return NULL;

    if (!posts_pagination_enabled(ctx, "posts_per_page"))
        return NULL;

    bc_trie_t *variables = bc_trie_new(free);
    posts_pagination(ctx, variables, "posts_per_page");

    const char *tag_prefix = bm_ctx_settings_lookup(ctx, "tag_prefix");
    const char *pagination_prefix = bm_ctx_settings_lookup(ctx, "pagination_prefix");
    const char *html_ext = bm_ctx_settings_lookup(ctx, "html_ext");

    bc_slist_t *rv = NULL;

    for (size_t k = 0; ctx->settings->tags[k] != NULL; k++) {
        bc_trie_t *local = bc_trie_new(free);
        bc_trie_insert(local, "FILTER_TAG", bc_strdup(ctx->settings->tags[k]));

        char *last_page = bm_exec_blogc_get_variable(ctx, variables, local,
            "LAST_PAGE", true, ctx->posts_fctx, false);

        bc_trie_free(local);

        if (last_page == NULL)
            continue;

        long pages = strtol(last_page, NULL, 10);
        free(last_page);

        for (size_t i = 0; i < pages; i++) {
            char *j = bc_strdup_printf("%d", i + 1);
            char *f = bm_generate_filename2(ctx->short_output_dir, tag_prefix,
                ctx->settings->tags[k], pagination_prefix, j, html_ext);
            rv = bc_slist_append(rv, bm_filectx_new(ctx, f, NULL, NULL));
            free(j);
            free(f);
        }
    }

    bc_trie_free(variables);

    return rv;
}

static int
pagination_tags_exec(bm_ctx_t *ctx, bc_slist_t *outputs, bc_trie_t *args)
{
    if (ctx == NULL || ctx->settings->posts == NULL || ctx->settings->tags == NULL)
        return 0;

    int rv = 0;
    size_t page = 1;

    bc_trie_t *variables = bc_trie_new(free);
    // not using posts_pagination because we set FILTER_PAGE anyway, and the
    // first value inserted in that function would be useless
    bc_trie_insert(variables, "FILTER_PER_PAGE",
        bc_strdup(bm_ctx_settings_lookup_str(ctx, "posts_per_page")));
    posts_ordering(ctx, variables, "html_order");
    bc_trie_insert(variables, "DATE_FORMAT",
        bc_strdup(bm_ctx_settings_lookup_str(ctx, "date_format")));
    bc_trie_insert(variables, "MAKE_RULE", bc_strdup("pagination_tags"));
    bc_trie_insert(variables, "MAKE_TYPE", bc_strdup("post"));

    const char *tag_prefix = bm_ctx_settings_lookup(ctx, "tag_prefix");
    const char *pagination_prefix = bm_ctx_settings_lookup(ctx, "pagination_prefix");
    const char *html_ext = bm_ctx_settings_lookup(ctx, "html_ext");

    for (bc_slist_t *l = outputs; l != NULL; l = l->next) {
        bm_filectx_t *fctx = l->data;
        if (fctx == NULL)
            continue;

        // this is very expensive, but we don't have another way to detect the
        // tag and page from the file path right now :/
        char *tag = NULL;
        for (size_t i = 0; ctx->settings->tags[i] != NULL; i++) {
            bool b = false;

            // it is impossible to have more output files per tag than the whole
            // amount of output pages
            for (size_t k = 1; k <= bc_slist_length(outputs); k++) {
                char *j = bc_strdup_printf("%d", k);
                char *f = bm_generate_filename2(ctx->short_output_dir, tag_prefix,
                    ctx->settings->tags[i], pagination_prefix, j, html_ext);
                free(j);
                if (0 == strcmp(fctx->short_path, f)) {
                    tag = ctx->settings->tags[i];
                    page = k;
                    b = true;
                    free(f);
                    break;
                }
                free(f);
            }
            if (b)
                break;
        }
        if (tag == NULL)
            continue;

        bc_trie_insert(variables, "FILTER_TAG", bc_strdup(tag));
        bc_trie_insert(variables, "FILTER_PAGE", bc_strdup_printf("%zu", page));

        if (bm_rule_need_rebuild(ctx->posts_fctx, ctx->settings_fctx,
                ctx->listing_entry_fctx, ctx->main_template_fctx, fctx, false))
        {
            rv = bm_exec_blogc(ctx, variables, NULL, true, ctx->listing_entry_fctx,
                ctx->main_template_fctx, fctx, ctx->posts_fctx, false);
            if (rv != 0)
                break;
        }
    }

    bc_trie_free(variables);

    return rv;
}


// POSTS RULE

static bc_slist_t*
posts_outputlist(bm_ctx_t *ctx)
{
    if (ctx == NULL || ctx->settings->posts == NULL)
        return NULL;

    bc_slist_t *rv = NULL;

    const char *post_prefix = bm_ctx_settings_lookup(ctx, "post_prefix");
    const char *html_ext = bm_ctx_settings_lookup(ctx, "html_ext");

    for (size_t i = 0; ctx->settings->posts[i] != NULL; i++) {
        char *f = bm_generate_filename(ctx->short_output_dir, post_prefix,
            ctx->settings->posts[i], html_ext);
        rv = bc_slist_append(rv, bm_filectx_new(ctx, f, NULL, NULL));
        free(f);
    }

    return rv;
}

static int
posts_exec(bm_ctx_t *ctx, bc_slist_t *outputs, bc_trie_t *args)
{
    if (ctx == NULL || ctx->settings->posts == NULL)
        return 0;

    int rv = 0;

    bc_trie_t *variables = bc_trie_new(free);
    bc_trie_insert(variables, "IS_POST", bc_strdup("1"));
    bc_trie_insert(variables, "DATE_FORMAT",
        bc_strdup(bm_ctx_settings_lookup(ctx, "date_format")));
    posts_ordering(ctx, variables, "html_order");
    bc_trie_insert(variables, "MAKE_RULE", bc_strdup("posts"));
    bc_trie_insert(variables, "MAKE_TYPE", bc_strdup("post"));

    bc_slist_t *s, *o;

    for (s = ctx->posts_fctx, o = outputs; s != NULL && o != NULL;
            s = s->next, o = o->next)
    {
        bm_filectx_t *s_fctx = s->data;
        bm_filectx_t *o_fctx = o->data;
        if (o_fctx == NULL)
            continue;
        if (bm_rule_need_rebuild(s, ctx->settings_fctx, NULL,
                ctx->main_template_fctx, o_fctx, true))
        {
            bc_trie_t *local = bc_trie_new(NULL);
            bc_trie_insert(local, "MAKE_SLUG", s_fctx->slug);  // no need to copy
            rv = bm_exec_blogc(ctx, variables, local, false, NULL, ctx->main_template_fctx,
                o_fctx, s, true);
            bc_trie_free(local);
            if (rv != 0)
                break;
        }
    }

    bc_trie_free(variables);

    return rv;
}


// TAGS RULE

static bc_slist_t*
tags_outputlist(bm_ctx_t *ctx)
{
    if (ctx == NULL || ctx->settings->posts == NULL || ctx->settings->tags == NULL)
        return NULL;

    if (!posts_pagination_enabled(ctx, "posts_per_page"))
        return NULL;

    bc_slist_t *rv = NULL;

    const char *tag_prefix = bm_ctx_settings_lookup(ctx, "tag_prefix");
    const char *html_ext = bm_ctx_settings_lookup(ctx, "html_ext");

    for (size_t i = 0; ctx->settings->tags[i] != NULL; i++) {
        char *f = bm_generate_filename(ctx->short_output_dir, tag_prefix,
            ctx->settings->tags[i], html_ext);
        rv = bc_slist_append(rv, bm_filectx_new(ctx, f, NULL, NULL));
        free(f);
    }

    return rv;
}

static int
tags_exec(bm_ctx_t *ctx, bc_slist_t *outputs, bc_trie_t *args)
{
    if (ctx == NULL || ctx->settings->posts == NULL || ctx->settings->tags == NULL)
        return 0;

    int rv = 0;
    size_t i = 0;

    bc_trie_t *variables = bc_trie_new(free);
    posts_pagination(ctx, variables, "posts_per_page");
    posts_ordering(ctx, variables, "html_order");
    bc_trie_insert(variables, "DATE_FORMAT",
        bc_strdup(bm_ctx_settings_lookup_str(ctx, "date_format")));
    bc_trie_insert(variables, "MAKE_RULE", bc_strdup("tags"));
    bc_trie_insert(variables, "MAKE_TYPE", bc_strdup("post"));

    for (bc_slist_t *l = outputs; l != NULL; l = l->next, i++) {
        bm_filectx_t *fctx = l->data;
        if (fctx == NULL)
            continue;

        bc_trie_insert(variables, "FILTER_TAG",
            bc_strdup(ctx->settings->tags[i]));

        if (bm_rule_need_rebuild(ctx->posts_fctx, ctx->settings_fctx,
                ctx->listing_entry_fctx, ctx->main_template_fctx, fctx, false))
        {
            rv = bm_exec_blogc(ctx, variables, NULL, true, ctx->listing_entry_fctx,
                ctx->main_template_fctx, fctx, ctx->posts_fctx, false);
            if (rv != 0)
                break;
        }
    }

    bc_trie_free(variables);

    return rv;
}


// PAGES RULE

static bc_slist_t*
pages_outputlist(bm_ctx_t *ctx)
{
    if (ctx == NULL || ctx->settings->pages == NULL)
        return NULL;

    bc_slist_t *rv = NULL;

    const char *html_ext = bm_ctx_settings_lookup(ctx, "html_ext");

    for (size_t i = 0; ctx->settings->pages[i] != NULL; i++) {
        char *f = bm_generate_filename(ctx->short_output_dir, NULL,
            ctx->settings->pages[i], html_ext);
        rv = bc_slist_append(rv, bm_filectx_new(ctx, f, NULL, NULL));
        free(f);
    }

    return rv;
}

static int
pages_exec(bm_ctx_t *ctx, bc_slist_t *outputs, bc_trie_t *args)
{
    if (ctx == NULL || ctx->settings->pages == NULL)
        return 0;

    int rv = 0;

    bc_trie_t *variables = bc_trie_new(free);
    bc_trie_insert(variables, "DATE_FORMAT",
        bc_strdup(bm_ctx_settings_lookup(ctx, "date_format")));
    bc_trie_insert(variables, "MAKE_RULE", bc_strdup("pages"));
    bc_trie_insert(variables, "MAKE_TYPE", bc_strdup("page"));

    bc_slist_t *s, *o;

    for (s = ctx->pages_fctx, o = outputs; s != NULL && o != NULL;
            s = s->next, o = o->next)
    {
        bm_filectx_t *s_fctx = s->data;
        bm_filectx_t *o_fctx = o->data;
        if (o_fctx == NULL)
            continue;
        if (bm_rule_need_rebuild(s, ctx->settings_fctx, NULL,
                ctx->main_template_fctx, o_fctx, true))
        {
            bc_trie_t *local = bc_trie_new(NULL);
            bc_trie_insert(local, "MAKE_SLUG", s_fctx->slug); // no need to copy
            rv = bm_exec_blogc(ctx, variables, local, false, NULL, ctx->main_template_fctx,
                o_fctx, s, true);
            bc_trie_free(local);
            if (rv != 0)
                break;
        }
    }

    bc_trie_free(variables);

    return rv;
}


// COPY FILES RULE

static bc_slist_t*
copy_outputlist(bm_ctx_t *ctx)
{
    if (ctx == NULL || ctx->settings->copy == NULL)
        return NULL;

    bc_slist_t *rv = NULL;

    // we iterate over ctx->copy_fctx list instead of ctx->settings->copy,
    // because bm_ctx_new() expands directories into its files, recursively.
    for (bc_slist_t *s = ctx->copy_fctx; s != NULL; s = s->next) {
        char *f = bc_strdup_printf("%s/%s", ctx->short_output_dir,
            ((bm_filectx_t*) s->data)->short_path);
        rv = bc_slist_append(rv, bm_filectx_new(ctx, f, NULL, NULL));
        free(f);
    }

    return rv;
}

static int
copy_exec(bm_ctx_t *ctx, bc_slist_t *outputs, bc_trie_t *args)
{
    if (ctx == NULL || ctx->settings->copy == NULL)
        return 0;

    int rv = 0;

    bc_slist_t *s, *o;

    for (s = ctx->copy_fctx, o = outputs; s != NULL && o != NULL;
            s = s->next, o = o->next)
    {
        bm_filectx_t *o_fctx = o->data;
        if (o_fctx == NULL)
            continue;

        if (bm_rule_need_rebuild(s, ctx->settings_fctx, NULL, NULL, o_fctx, true)) {
            rv = bm_exec_native_cp(s->data, o_fctx, ctx->verbose);
            if (rv != 0)
                break;
        }
    }

    return rv;
}


// CLEAN RULE

static bc_slist_t*
clean_outputlist(bm_ctx_t *ctx)
{
    return bm_rule_list_built_files(ctx);
}

static int
clean_exec(bm_ctx_t *ctx, bc_slist_t *outputs, bc_trie_t *args)
{
    int rv = 0;

    for (bc_slist_t *l = outputs; l != NULL; l = l->next) {
        bm_filectx_t *fctx = l->data;
        if (fctx == NULL)
            continue;

        if (fctx->readable) {
            rv = bm_exec_native_rm(ctx->output_dir, fctx, ctx->verbose);
            if (rv != 0)
                break;
        }
    }

    if (!bm_exec_native_is_empty_dir(ctx->output_dir, NULL)) {
        fprintf(stderr, "blogc-make: warning: output directory is not empty!\n");
    }

    return rv;
}


static int all_exec(bm_ctx_t *ctx, bc_slist_t *outputs, bc_trie_t *args);


// RUNSERVER RULE

static int
runserver_exec(bm_ctx_t *ctx, bc_slist_t *outputs, bc_trie_t *args)
{
    return bm_httpd_run(&ctx, all_exec, outputs, args);
}


// WATCH RULE

static int
watch_exec(bm_ctx_t *ctx, bc_slist_t *outputs, bc_trie_t *args)
{
    return bm_reloader_run(&ctx, all_exec, outputs, args);
}


// ATOM DUMP RULE

static int
atom_dump_exec(bm_ctx_t *ctx, bc_slist_t *outputs, bc_trie_t *args)
{
    char *content = bm_atom_generate(ctx->settings);
    if (content == NULL)
        return 1;
    printf("%s", content);
    free(content);
    return 0;
}


const bm_rule_t rules[] = {
    {
        .name = "all",
        .help = "run all build rules",
        .outputlist_func = NULL,
        .exec_func = all_exec,
        .generate_files = false,
    },
    {
        .name = "index",
        .help = "build website index from posts",
        .outputlist_func = index_outputlist,
        .exec_func = index_exec,
        .generate_files = true,
    },
    {
        .name = "atom",
        .help = "build main atom feed from posts",
        .outputlist_func = atom_outputlist,
        .exec_func = atom_exec,
        .generate_files = true,
    },
    {
        .name = "atom_tags",
        .help = "build atom feeds for each tag from posts",
        .outputlist_func = atom_tags_outputlist,
        .exec_func = atom_tags_exec,
        .generate_files = true,
    },
    {
        .name = "pagination",
        .help = "build pagination pages from posts",
        .outputlist_func = pagination_outputlist,
        .exec_func = pagination_exec,
        .generate_files = true,
    },
    {
        .name = "pagination_tags",
        .help = "build pagination pages for each tag from posts",
        .outputlist_func = pagination_tags_outputlist,
        .exec_func = pagination_tags_exec,
        .generate_files = true,
    },
    {
        .name = "posts",
        .help = "build individual pages for each post",
        .outputlist_func = posts_outputlist,
        .exec_func = posts_exec,
        .generate_files = true,
    },
    {
        .name = "tags",
        .help = "build post listings for each tag from posts",
        .outputlist_func = tags_outputlist,
        .exec_func = tags_exec,
        .generate_files = true,
    },
    {
        .name = "pages",
        .help = "build individual pages for each page",
        .outputlist_func = pages_outputlist,
        .exec_func = pages_exec,
        .generate_files = true,
    },
    {
        .name = "copy",
        .help = "copy static files from source directory to output directory",
        .outputlist_func = copy_outputlist,
        .exec_func = copy_exec,
        .generate_files = true,
    },
#ifdef USE_LIBSASS
    {
        .name = "sass",
        .help = "build CSS stylesheets from Sass (.scss) sources",
        .outputlist_func = bm_sass_outputlist,
        .exec_func = bm_sass_exec,
        .generate_files = true,
    },
#endif
    {
        .name = "clean",
        .help = "clean built files and empty directories in output directory",
        .outputlist_func = clean_outputlist,
        .exec_func = clean_exec,
        .generate_files = false,
    },
    {
        .name = "runserver",
        .help = "run blogc-runserver pointing to output directory, if available,\n"
            "                     rebuilding as needed\n"
            "                     arguments: host (127.0.0.1), port (8080) and threads (20)",
        .outputlist_func = NULL,
        .exec_func = runserver_exec,
        .generate_files = false,
    },
    {
        .name = "watch",
        .help = "watch for changes in the source files, rebuilding as needed",
        .outputlist_func = NULL,
        .exec_func = watch_exec,
        .generate_files = false,
    },
    {
        .name = "atom_dump",
        .help = "dump default Atom feed template based on current settings",
        .outputlist_func = NULL,
        .exec_func = atom_dump_exec,
        .generate_files = false,
    },
    {NULL, NULL, NULL, NULL, false},
};


// ALL RULE

static int
all_exec(bm_ctx_t *ctx, bc_slist_t *outputs, bc_trie_t *args)
{
    for (size_t i = 0; rules[i].name != NULL; i++) {
        if (!rules[i].generate_files) {
            continue;
        }

        int rv = bm_rule_execute(ctx, &(rules[i]), NULL);
        if (rv != 0) {
            return rv;
        }
    }

    return 0;
}


bc_trie_t*
bm_rule_parse_args(const char *sep)
{
    if (sep == NULL || *sep == '\0' || *sep != ':')
        return NULL;

    bc_trie_t *rv = bc_trie_new(free);
    char *end = (char*) sep + 1;
    char *kv_sep;
    while (NULL != (kv_sep = strchr(end, '='))) {
        char *key = bc_strndup(end, kv_sep - end);
        end = kv_sep + 1;
        kv_sep = strchr(end, ',');
        if (kv_sep == NULL)
            kv_sep = strchr(end, '\0');
        char *value = bc_strndup(end, kv_sep - end);
        bc_trie_insert(rv, key, value);
        free(key);
        if (*kv_sep == '\0')
            break;
        end = kv_sep + 1;
    }
    if (kv_sep == NULL) {
        bc_trie_free(rv);
        return NULL;
    }

    return rv;
}


int
bm_rule_executor(bm_ctx_t *ctx, bc_slist_t *rule_list)
{
    if (ctx == NULL)
        return 1;

    const bm_rule_t *rule = NULL;
    int rv = 0;

    for (bc_slist_t *l = rule_list; l != NULL; l = l->next) {

        char *rule_str = l->data;
        char *sep = strchr(rule_str, ':');

        bc_trie_t *args = NULL;
        if (sep == NULL) {
            sep = strchr(rule_str, '\0');
        }
        else {
            args = bm_rule_parse_args(sep);
            if (args == NULL) {
                fprintf(stderr, "blogc-make: warning: failed to parse rule "
                    "arguments, ignoring: %s\n", rule_str);
            }
        }

        rule = NULL;
        for (size_t i = 0; rules[i].name != NULL; i++) {
            if (strlen(rules[i].name) != (sep - rule_str))
                continue;
            if (0 == strncmp(rule_str, rules[i].name, sep - rule_str)) {
                rule = &(rules[i]);
                rv = bm_rule_execute(ctx, rule, args);
                if (rv != 0)
                    return rv;
                break;
            }
        }
        if (rule == NULL) {
            fprintf(stderr, "blogc-make: error: rule not found: %.*s\n",
                (int) (sep - rule_str), rule_str);
            rv = 1;
        }
    }

    return rv;
}


int
bm_rule_execute(bm_ctx_t *ctx, const bm_rule_t *rule, bc_trie_t *args)
{
    if (ctx == NULL || rule == NULL)
        return 1;

    bc_slist_t *outputs = NULL;
    if (rule->outputlist_func != NULL) {
        outputs = rule->outputlist_func(ctx);
    }

    int rv = rule->exec_func(ctx, outputs, args);

    bc_slist_free_full(outputs, (bc_free_func_t) bm_filectx_free);

    return rv;
}


bool
bm_rule_need_rebuild(bc_slist_t *sources, bm_filectx_t *settings,
    bm_filectx_t *listing_entry, bm_filectx_t *template, bm_filectx_t *output,
    bool only_first_source)
{
    if (output == NULL || !output->readable)
        return true;

    bool rv = false;

    bc_slist_t *s = NULL;
    if (settings != NULL)
        s = bc_slist_append(s, settings);
    if (template != NULL)
        s = bc_slist_append(s, template);
    if (listing_entry != NULL)
        s = bc_slist_append(s, listing_entry);

    for (bc_slist_t *l = sources; l != NULL; l = l->next) {
        s = bc_slist_append(s, l->data);
        if (only_first_source)
            break;
    }

    for (bc_slist_t *l = s; l != NULL; l = l->next) {
        bm_filectx_t *source = l->data;
        if (source == NULL || !source->readable) {
            // this is unlikely to happen, but lets just say that we need
            // a rebuild and let blogc bail out.
            rv = true;
            break;
        }
        if (source->tv_sec == output->tv_sec) {
            if (source->tv_nsec > output->tv_nsec) {
                rv = true;
                break;
            }
        }
        else if (source->tv_sec > output->tv_sec) {
            rv = true;
            break;
        }
    }

    bc_slist_free(s);

    return rv;
}


bc_slist_t*
bm_rule_list_built_files(bm_ctx_t *ctx)
{
    if (ctx == NULL)
        return NULL;

    bc_slist_t *rv = NULL;
    for (size_t i = 0; rules[i].name != NULL; i++) {
        if (!rules[i].generate_files) {
            continue;
        }

        bc_slist_t *o = rules[i].outputlist_func(ctx);
        for (bc_slist_t *l = o; l != NULL; l = l->next) {
            rv = bc_slist_append(rv, l->data);
        }
        bc_slist_free(o);
    }

    return rv;
}


void
bm_rule_print_help(void)
{
    printf("\nhelper rules:\n");
    for (size_t i = 0; rules[i].name != NULL; i++) {
        if (!rules[i].generate_files) {
            printf("    %-15s  %s\n", rules[i].name, rules[i].help);
        }
    }
    printf("\nbuild rules:\n");
    for (size_t i = 0; rules[i].name != NULL; i++) {
        if (rules[i].generate_files) {
            printf("    %-15s  %s\n", rules[i].name, rules[i].help);
        }
    }
}
