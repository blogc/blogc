/*
 * blogc: A blog compiler.
 * Copyright (C) 2016 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <math.h>
#include "../common/utils.h"
#include "ctx.h"
#include "exec.h"
#include "exec-native.h"
#include "rules.h"


// INDEX RULE

static bc_slist_t*
index_outputlist(bm_ctx_t *ctx)
{
    if (ctx == NULL || ctx->settings->posts == NULL)
        return NULL;

    bc_slist_t *rv = NULL;
    const char *html_ext = bc_trie_lookup(ctx->settings->settings,
        "html_ext");
    const char *output_dir = bc_trie_lookup(ctx->settings->settings,
        "output_dir");
    const char *index_prefix = bc_trie_lookup(ctx->settings->settings,
        "index_prefix");
    bool is_index = (index_prefix == NULL) && (html_ext[0] == '/');
    char *f = bc_strdup_printf("%s%s%s%s", output_dir,
        is_index ? "" : "/", is_index ? "" : index_prefix,
        html_ext);
    rv = bc_slist_append(rv, bm_filectx_new(ctx, f));
    free(f);
    return rv;
}

static int
index_exec(bm_ctx_t *ctx, bc_slist_t *outputs, bool verbose)
{
    if (ctx == NULL || ctx->settings->posts == NULL)
        return 0;

    int rv = 0;

    bc_trie_t *variables = bc_trie_new(free);
    bc_trie_insert(variables, "FILTER_PER_PAGE",
        bc_strdup(bc_trie_lookup(ctx->settings->settings, "posts_per_page")));
    bc_trie_insert(variables, "FILTER_PAGE", bc_strdup("1"));
    bc_trie_insert(variables, "DATE_FORMAT",
        bc_strdup(bc_trie_lookup(ctx->settings->settings, "date_format")));
    bc_trie_insert(variables, "BM_RULE", bc_strdup("index"));
    bc_trie_insert(variables, "BM_TYPE", bc_strdup("post"));

    for (bc_slist_t *l = outputs; l != NULL; l = l->next) {
        bm_filectx_t *fctx = l->data;
        if (fctx == NULL)
            continue;
        if (bm_rule_need_rebuild(ctx->posts_fctx, ctx->settings_fctx,
                ctx->main_template_fctx, fctx, false))
        {
            rv = bm_exec_blogc(ctx->settings, variables, true,
                ctx->main_template_fctx, fctx, ctx->posts_fctx, verbose,
                false);
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

    bc_slist_t *rv = NULL;
    const char *output_dir = bc_trie_lookup(ctx->settings->settings,
        "output_dir");
    const char *atom_prefix = bc_trie_lookup(ctx->settings->settings,
        "atom_prefix");
    const char *atom_ext = bc_trie_lookup(ctx->settings->settings, "atom_ext");
    char *f = bc_strdup_printf("%s/%s%s", output_dir, atom_prefix, atom_ext);
    rv = bc_slist_append(rv, bm_filectx_new(ctx, f));
    free(f);
    return rv;
}

static int
atom_exec(bm_ctx_t *ctx, bc_slist_t *outputs, bool verbose)
{
    if (ctx == NULL || ctx->settings->posts == NULL)
        return 0;

    int rv = 0;

    bc_trie_t *variables = bc_trie_new(free);
    bc_trie_insert(variables, "FILTER_PER_PAGE",
        bc_strdup(bc_trie_lookup(ctx->settings->settings,
        "atom_posts_per_page")));
    bc_trie_insert(variables, "FILTER_PAGE", bc_strdup("1"));
    bc_trie_insert(variables, "DATE_FORMAT", bc_strdup("%Y-%m-%dT%H:%M:%SZ"));
    bc_trie_insert(variables, "BM_RULE", bc_strdup("atom"));
    bc_trie_insert(variables, "BM_TYPE", bc_strdup("atom"));

    for (bc_slist_t *l = outputs; l != NULL; l = l->next) {
        bm_filectx_t *fctx = l->data;
        if (fctx == NULL)
            continue;
        if (bm_rule_need_rebuild(ctx->posts_fctx, ctx->settings_fctx, NULL,
                fctx, false))
        {
            rv = bm_exec_blogc(ctx->settings, variables, true,
                ctx->atom_template_fctx, fctx, ctx->posts_fctx, verbose,
                false);
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

    bc_slist_t *rv = NULL;
    const char *output_dir = bc_trie_lookup(ctx->settings->settings,
        "output_dir");
    const char *atom_prefix = bc_trie_lookup(ctx->settings->settings,
        "atom_prefix");
    const char *atom_ext = bc_trie_lookup(ctx->settings->settings, "atom_ext");
    for (size_t i = 0; ctx->settings->tags[i] != NULL; i++) {
        char *f = bc_strdup_printf("%s/%s/%s%s", output_dir, atom_prefix,
            ctx->settings->tags[i], atom_ext);
        rv = bc_slist_append(rv, bm_filectx_new(ctx, f));
        free(f);
    }
    return rv;
}

static int
atom_tags_exec(bm_ctx_t *ctx, bc_slist_t *outputs, bool verbose)
{
    if (ctx == NULL || ctx->settings->posts == NULL || ctx->settings->tags == NULL)
        return 0;

    int rv = 0;
    size_t i = 0;

    bc_trie_t *variables = bc_trie_new(free);
    bc_trie_insert(variables, "FILTER_PER_PAGE",
        bc_strdup(bc_trie_lookup(ctx->settings->settings,
        "atom_posts_per_page")));
    bc_trie_insert(variables, "FILTER_PAGE", bc_strdup("1"));
    bc_trie_insert(variables, "DATE_FORMAT", bc_strdup("%Y-%m-%dT%H:%M:%SZ"));
    bc_trie_insert(variables, "BM_RULE", bc_strdup("atom_tags"));
    bc_trie_insert(variables, "BM_TYPE", bc_strdup("atom"));

    for (bc_slist_t *l = outputs; l != NULL; l = l->next, i++) {
        bm_filectx_t *fctx = l->data;
        if (fctx == NULL)
            continue;

        bc_trie_insert(variables, "FILTER_TAG",
            bc_strdup(ctx->settings->tags[i]));

        if (bm_rule_need_rebuild(ctx->posts_fctx, ctx->settings_fctx, NULL,
                fctx, false))
        {
            rv = bm_exec_blogc(ctx->settings, variables, true,
                ctx->atom_template_fctx, fctx, ctx->posts_fctx, verbose,
                false);
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

    long num_posts = bc_slist_length(ctx->posts_fctx);
    long posts_per_page = strtol(
        bc_trie_lookup(ctx->settings->settings, "posts_per_page"),
        NULL, 10);  // FIXME: improve
    size_t pages = ceilf(((float) num_posts) / posts_per_page);

    const char *output_dir = bc_trie_lookup(ctx->settings->settings,
        "output_dir");
    const char *pagination_prefix = bc_trie_lookup(ctx->settings->settings,
        "pagination_prefix");
    const char *html_ext = bc_trie_lookup(ctx->settings->settings,
        "html_ext");

    bc_slist_t *rv = NULL;
    for (size_t i = 0; i < pages; i++) {
        char *f = bc_strdup_printf("%s/%s/%d%s", output_dir, pagination_prefix,
            i + 1, html_ext);
        rv = bc_slist_append(rv, bm_filectx_new(ctx, f));
        free(f);
    }
    return rv;
}

static int
pagination_exec(bm_ctx_t *ctx, bc_slist_t *outputs, bool verbose)
{
    if (ctx == NULL || ctx->settings->posts == NULL)
        return 0;

    int rv = 0;
    size_t page = 1;

    bc_trie_t *variables = bc_trie_new(free);
    bc_trie_insert(variables, "FILTER_PER_PAGE",
        bc_strdup(bc_trie_lookup(ctx->settings->settings, "posts_per_page")));
    bc_trie_insert(variables, "DATE_FORMAT",
        bc_strdup(bc_trie_lookup(ctx->settings->settings, "date_format")));
    bc_trie_insert(variables, "BM_RULE", bc_strdup("pagination"));
    bc_trie_insert(variables, "BM_TYPE", bc_strdup("post"));

    for (bc_slist_t *l = outputs; l != NULL; l = l->next, page++) {
        bm_filectx_t *fctx = l->data;
        if (fctx == NULL)
            continue;
        bc_trie_insert(variables, "FILTER_PAGE", bc_strdup_printf("%zu", page));
        if (bm_rule_need_rebuild(ctx->posts_fctx, ctx->settings_fctx,
                ctx->main_template_fctx, fctx, false))
        {
            rv = bm_exec_blogc(ctx->settings, variables, true,
                ctx->main_template_fctx, fctx, ctx->posts_fctx, verbose, false);
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

    const char *output_dir = bc_trie_lookup(ctx->settings->settings,
        "output_dir");
    const char *post_prefix = bc_trie_lookup(ctx->settings->settings,
        "post_prefix");
    const char *html_ext = bc_trie_lookup(ctx->settings->settings,
        "html_ext");

    bc_slist_t *rv = NULL;
    for (size_t i = 0; ctx->settings->posts[i] != NULL; i++) {
        char *f = bc_strdup_printf("%s/%s/%s%s", output_dir, post_prefix,
            ctx->settings->posts[i], html_ext);
        rv = bc_slist_append(rv, bm_filectx_new(ctx, f));
        free(f);
    }
    return rv;
}

static int
posts_exec(bm_ctx_t *ctx, bc_slist_t *outputs, bool verbose)
{
    if (ctx == NULL || ctx->settings->posts == NULL)
        return 0;

    int rv = 0;

    bc_trie_t *variables = bc_trie_new(free);
    bc_trie_insert(variables, "IS_POST", bc_strdup("1"));
    bc_trie_insert(variables, "DATE_FORMAT",
        bc_strdup(bc_trie_lookup(ctx->settings->settings, "date_format")));
    bc_trie_insert(variables, "BM_RULE", bc_strdup("posts"));
    bc_trie_insert(variables, "BM_TYPE", bc_strdup("post"));

    bc_slist_t *s, *o;

    for (s = ctx->posts_fctx, o = outputs; s != NULL && o != NULL;
            s = s->next, o = o->next)
    {
        bm_filectx_t *o_fctx = o->data;
        if (o_fctx == NULL)
            continue;
        if (bm_rule_need_rebuild(s, ctx->settings_fctx,
                ctx->main_template_fctx, o_fctx, true))
        {
            rv = bm_exec_blogc(ctx->settings, variables, false,
                ctx->main_template_fctx, o_fctx, s, verbose, true);
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

    bc_slist_t *rv = NULL;
    const char *output_dir = bc_trie_lookup(ctx->settings->settings,
        "output_dir");
    const char *tag_prefix = bc_trie_lookup(ctx->settings->settings,
        "tag_prefix");
    const char *html_ext = bc_trie_lookup(ctx->settings->settings, "html_ext");
    for (size_t i = 0; ctx->settings->tags[i] != NULL; i++) {
        char *f = bc_strdup_printf("%s/%s/%s%s", output_dir, tag_prefix,
            ctx->settings->tags[i], html_ext);
        rv = bc_slist_append(rv, bm_filectx_new(ctx, f));
        free(f);
    }
    return rv;
}

static int
tags_exec(bm_ctx_t *ctx, bc_slist_t *outputs, bool verbose)
{
    if (ctx == NULL || ctx->settings->posts == NULL || ctx->settings->tags == NULL)
        return 0;

    int rv = 0;
    size_t i = 0;

    bc_trie_t *variables = bc_trie_new(free);
    bc_trie_insert(variables, "FILTER_PER_PAGE",
        bc_strdup(bc_trie_lookup(ctx->settings->settings,
        "atom_posts_per_page")));
    bc_trie_insert(variables, "FILTER_PAGE", bc_strdup("1"));
    bc_trie_insert(variables, "DATE_FORMAT",
        bc_strdup(bc_trie_lookup(ctx->settings->settings, "date_format")));
    bc_trie_insert(variables, "BM_RULE", bc_strdup("tags"));
    bc_trie_insert(variables, "BM_TYPE", bc_strdup("post"));

    for (bc_slist_t *l = outputs; l != NULL; l = l->next, i++) {
        bm_filectx_t *fctx = l->data;
        if (fctx == NULL)
            continue;

        bc_trie_insert(variables, "FILTER_TAG",
            bc_strdup(ctx->settings->tags[i]));

        if (bm_rule_need_rebuild(ctx->posts_fctx, ctx->settings_fctx,
                ctx->main_template_fctx, fctx, false))
        {
            rv = bm_exec_blogc(ctx->settings, variables, true,
                ctx->main_template_fctx, fctx, ctx->posts_fctx, verbose,
                false);
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

    const char *output_dir = bc_trie_lookup(ctx->settings->settings,
        "output_dir");
    const char *html_ext = bc_trie_lookup(ctx->settings->settings, "html_ext");

    bc_slist_t *rv = NULL;
    for (size_t i = 0; ctx->settings->pages[i] != NULL; i++) {
        bool is_index = (0 == strcmp(ctx->settings->pages[i], "index"))
            && (html_ext[0] == '/');
        char *f = bc_strdup_printf("%s%s%s%s", output_dir,
            is_index ? "" : "/", is_index ? "" : ctx->settings->pages[i],
            html_ext);
        rv = bc_slist_append(rv, bm_filectx_new(ctx, f));
        free(f);
    }
    return rv;
}

static int
pages_exec(bm_ctx_t *ctx, bc_slist_t *outputs, bool verbose)
{
    if (ctx == NULL || ctx->settings->pages == NULL)
        return 0;

    int rv = 0;

    bc_trie_t *variables = bc_trie_new(free);
    bc_trie_insert(variables, "DATE_FORMAT",
        bc_strdup(bc_trie_lookup(ctx->settings->settings, "date_format")));
    bc_trie_insert(variables, "BM_RULE", bc_strdup("pages"));
    bc_trie_insert(variables, "BM_TYPE", bc_strdup("page"));

    bc_slist_t *s, *o;

    for (s = ctx->pages_fctx, o = outputs; s != NULL && o != NULL;
            s = s->next, o = o->next)
    {
        bm_filectx_t *o_fctx = o->data;
        if (o_fctx == NULL)
            continue;
        if (bm_rule_need_rebuild(s, ctx->settings_fctx,
                ctx->main_template_fctx, o_fctx, true))
        {
            rv = bm_exec_blogc(ctx->settings, variables, false,
                ctx->main_template_fctx, o_fctx, s, verbose, true);
            if (rv != 0)
                break;
        }
    }

    bc_trie_free(variables);

    return rv;
}


// COPY FILES RULE

static bc_slist_t*
copy_files_outputlist(bm_ctx_t *ctx)
{
    if (ctx == NULL || ctx->settings->copy_files == NULL)
        return NULL;

    bc_slist_t *rv = NULL;
    const char *dir = bc_trie_lookup(ctx->settings->settings, "output_dir");
    for (size_t i = 0; ctx->settings->copy_files[i] != NULL; i++) {
        char *f = bc_strdup_printf("%s/%s", dir, ctx->settings->copy_files[i]);
        rv = bc_slist_append(rv, bm_filectx_new(ctx, f));
        free(f);
    }
    return rv;
}

static int
copy_files_exec(bm_ctx_t *ctx, bc_slist_t *outputs, bool verbose)
{
    if (ctx == NULL || ctx->settings->copy_files == NULL)
        return 0;

    int rv = 0;

    bc_slist_t *s, *o;

    for (s = ctx->copy_files_fctx, o = outputs; s != NULL && o != NULL;
            s = s->next, o = o->next)
    {
        bm_filectx_t *o_fctx = o->data;
        if (o_fctx == NULL)
            continue;

        if (bm_rule_need_rebuild(s, ctx->settings_fctx, NULL, o_fctx, true)) {
            rv = bm_exec_native_cp(s->data, o_fctx, verbose);
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
clean_exec(bm_ctx_t *ctx, bc_slist_t *outputs, bool verbose)
{
    int rv = 0;

    for (bc_slist_t *l = outputs; l != NULL; l = l->next)
    {
        bm_filectx_t *fctx = l->data;
        if (fctx == NULL)
            continue;

        if (fctx->readable) {
            rv = bm_exec_native_rm(fctx, verbose);
            if (rv != 0)
                break;
        }
    }

    return rv;
}


const bm_rule_t const rules[] = {
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
        .name = "copy_files",
        .help = "copy static files from source directory to output directory",
        .outputlist_func = copy_files_outputlist,
        .exec_func = copy_files_exec,
        .generate_files = true,
    },
    {
        .name = "clean",
        .help = "clean built files and empty directories in output directory",
        .outputlist_func = clean_outputlist,
        .exec_func = clean_exec,
        .generate_files = false,
    },
    {NULL, NULL, NULL, NULL, false},
};


int
bm_rule_executor(bm_ctx_t *ctx, bc_slist_t *rule_list, bool verbose)
{
    const bm_rule_t *rule = NULL;
    int rv = 0;

    for (bc_slist_t *l = rule_list; l != NULL; l = l->next) {
        if (0 == strcmp("all", (char*) l->data)) {
            bc_slist_t *s = NULL;
            for (size_t i = 0; rules[i].name != NULL; i++) {
                if (!rules[i].generate_files) {
                    continue;
                }
                s = bc_slist_append(s, bc_strdup(rules[i].name));
            }
            rv = bm_rule_executor(ctx, s, verbose);
            bc_slist_free_full(s, free);
            continue;
        }
        rule = NULL;
        for (size_t i = 0; rules[i].name != NULL; i++) {
            if (0 == strcmp((char*) l->data, rules[i].name)) {
                rule = &(rules[i]);
                rv = bm_rule_execute(ctx, rule, verbose);
                if (rv != 0)
                    return rv;
            }
        }
        if (rule == NULL) {
            fprintf(stderr, "blogc-make: error: rule not found: %s\n",
                (char*) l->data);
            rv = 3;
        }
    }

    return rv;
}


int
bm_rule_execute(bm_ctx_t *ctx, const bm_rule_t *rule, bool verbose)
{
    if (rule == NULL)
        return 3;

    bc_slist_t *outputs = rule->outputlist_func(ctx);
    int rv = rule->exec_func(ctx, outputs, verbose);

    bc_slist_free_full(outputs, (bc_free_func_t) bm_filectx_free);

    return rv;
}


bool
bm_rule_need_rebuild(bc_slist_t *sources, bm_filectx_t *settings,
    bm_filectx_t *template, bm_filectx_t *output, bool only_first_source)
{
    if (output == NULL || !output->readable)
        return true;

    bool rv = false;

    bc_slist_t *s = NULL;
    if (settings != NULL)
        s = bc_slist_append(s, settings);
    if (template != NULL)
        s = bc_slist_append(s, template);

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
        if (source->timestamp.tv_sec == output->timestamp.tv_sec) {
            if (source->timestamp.tv_nsec > output->timestamp.tv_nsec) {
                rv = true;
                break;
            }
        }
        else if (source->timestamp.tv_sec > output->timestamp.tv_sec) {
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
    printf(
        "\n"
        "build rules:\n"
        "    all           run all rules that generate output files\n");

    for (size_t i = 0; rules[i].name != NULL; i++) {
        printf("    %-12s  %s\n", rules[i].name, rules[i].help);
    }
}
