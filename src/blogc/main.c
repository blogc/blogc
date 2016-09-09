/*
 * blogc: A blog compiler.
 * Copyright (C) 2015-2016 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif /* HAVE_SYS_STAT_H */

#include <errno.h>
#include <locale.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "template-parser.h"
#include "loader.h"
#include "renderer.h"
#include "../common/error.h"
#include "../common/utf8.h"
#include "../common/utils.h"

#ifndef PACKAGE_VERSION
#define PACKAGE_VERSION "Unknown"
#endif


static void
blogc_print_help(void)
{
    printf(
        "usage:\n"
        "    blogc [-h] [-v] [-d] [-l] [-D KEY=VALUE ...] [-p KEY] [-t TEMPLATE]\n"
        "          [-o OUTPUT] [SOURCE ...] - A blog compiler.\n"
        "\n"
        "positional arguments:\n"
        "    SOURCE        source file(s)\n"
        "\n"
        "optional arguments:\n"
        "    -h            show this help message and exit\n"
        "    -v            show version and exit\n"
        "    -d            enable debug\n"
        "    -l            build listing page, from multiple source files\n"
        "    -D KEY=VALUE  set global configuration parameter\n"
        "    -p KEY        show the value of a global configuration parameter\n"
        "                  after source parsing and exit\n"
        "    -t TEMPLATE   template file\n"
        "    -o OUTPUT     output file\n");
}


static void
blogc_print_usage(void)
{
    printf(
        "usage: blogc [-h] [-v] [-d] [-l] [-D KEY=VALUE ...] [-p KEY] [-t TEMPLATE]\n"
        "             [-o OUTPUT] [SOURCE ...]\n");
}


static void
blogc_mkdir_recursive(const char *filename)
{
    char *fname = bc_strdup(filename);
    for (char *tmp = fname; *tmp != '\0'; tmp++) {
        if (*tmp != '/' && *tmp != '\\')
            continue;
#ifdef HAVE_SYS_STAT_H
        char bkp = *tmp;
        *tmp = '\0';
        if ((strlen(fname) > 0) &&
#if defined(WIN32) || defined(_WIN32)
            (-1 == mkdir(fname)) &&
#else
            (-1 == mkdir(fname, 0777)) &&
#endif
            (errno != EEXIST))
        {
            fprintf(stderr, "blogc: error: failed to create output "
                "directory (%s): %s\n", fname, strerror(errno));
            free(fname);
            exit(2);
        }
        *tmp = bkp;
#else
        // FIXME: show this warning only if actually trying to create a directory.
        fprintf(stderr, "blogc: warning: can't create output directories "
            "for your platform. please create the directories yourself.\n");
        break;
#endif
    }
    free(fname);
}


int
main(int argc, char **argv)
{
    setlocale(LC_ALL, "");

    int rv = 0;

    bool debug = false;
    bool listing = false;
    char *template = NULL;
    char *output = NULL;
    char *print = NULL;
    char *tmp = NULL;
    char **pieces = NULL;

    bc_slist_t *sources = NULL;
    bc_trie_t *config = bc_trie_new(free);
    bc_trie_insert(config, "BLOGC_VERSION", bc_strdup(PACKAGE_VERSION));

    for (unsigned int i = 1; i < argc; i++) {
        tmp = NULL;
        if (argv[i][0] == '-') {
            switch (argv[i][1]) {
                case 'h':
                    blogc_print_help();
                    goto cleanup;
                case 'v':
                    printf("%s\n", PACKAGE_STRING);
                    goto cleanup;
                case 'd':
                    debug = true;
                    break;
                case 'l':
                    listing = true;
                    break;
                case 't':
                    if (argv[i][2] != '\0')
                        template = bc_strdup(argv[i] + 2);
                    else if (i + 1 < argc)
                        template = bc_strdup(argv[++i]);
                    break;
                case 'o':
                    if (argv[i][2] != '\0')
                        output = bc_strdup(argv[i] + 2);
                    else if (i + 1 < argc)
                        output = bc_strdup(argv[++i]);
                    break;
                case 'p':
                    if (argv[i][2] != '\0')
                        print = bc_strdup(argv[i] + 2);
                    else if (i + 1 < argc)
                        print = bc_strdup(argv[++i]);
                    break;
                case 'D':
                    if (argv[i][2] != '\0')
                        tmp = argv[i] + 2;
                    else if (i + 1 < argc)
                        tmp = argv[++i];
                    if (tmp != NULL) {
                        if (!bc_utf8_validate((uint8_t*) tmp, strlen(tmp))) {
                            fprintf(stderr, "blogc: error: invalid value for "
                                "-D (must be valid UTF-8 string): %s\n", tmp);
                            goto cleanup;
                        }
                        pieces = bc_str_split(tmp, '=', 2);
                        if (bc_strv_length(pieces) != 2) {
                            fprintf(stderr, "blogc: error: invalid value for "
                                "-D (must have an '='): %s\n", tmp);
                            bc_strv_free(pieces);
                            rv = 2;
                            goto cleanup;
                        }
                        for (unsigned int j = 0; pieces[0][j] != '\0'; j++) {
                            if (!((pieces[0][j] >= 'A' && pieces[0][j] <= 'Z') ||
                                pieces[0][j] == '_'))
                            {
                                fprintf(stderr, "blogc: error: invalid value "
                                    "for -D (configuration key must be uppercase "
                                    "with '_'): %s\n", pieces[0]);
                                bc_strv_free(pieces);
                                rv = 2;
                                goto cleanup;
                            }
                        }
                        bc_trie_insert(config, pieces[0], bc_strdup(pieces[1]));
                        bc_strv_free(pieces);
                        pieces = NULL;
                    }
                    break;
                default:
                    blogc_print_usage();
                    fprintf(stderr, "blogc: error: invalid argument: -%c\n",
                        argv[i][1]);
                    rv = 2;
                    goto cleanup;
            }
        }
        else
            sources = bc_slist_append(sources, bc_strdup(argv[i]));
    }

    if (!listing && bc_slist_length(sources) == 0) {
        blogc_print_usage();
        fprintf(stderr, "blogc: error: one source file is required\n");
        rv = 2;
        goto cleanup;
    }

    if (!listing && bc_slist_length(sources) > 1) {
        blogc_print_usage();
        fprintf(stderr, "blogc: error: only one source file should be provided, "
            "if running without '-l'\n");
        rv = 2;
        goto cleanup;
    }

    bc_error_t *err = NULL;

    bc_slist_t *s = blogc_source_parse_from_files(config, sources, &err);
    if (err != NULL) {
        bc_error_print(err);
        rv = 2;
        goto cleanup2;
    }

    if (print != NULL) {
        const char *val = bc_trie_lookup(config, print);
        if (val == NULL) {
            fprintf(stderr, "blogc: error: configuration variable not found: %s\n",
                print);
            rv = 2;
        }
        else {
            printf("%s\n", val);
        }
        goto cleanup2;
    }

    if (template == NULL) {
        blogc_print_usage();
        fprintf(stderr, "blogc: error: argument -t is required when rendering content\n");
        rv = 2;
        goto cleanup2;
    }

    bc_slist_t* l = blogc_template_parse_from_file(template, &err);
    if (err != NULL) {
        bc_error_print(err);
        rv = 2;
        goto cleanup3;
    }

    if (debug)
        blogc_debug_template(l);

    char *out = blogc_render(l, s, config, listing);

    bool write_to_stdout = (output == NULL || (0 == strcmp(output, "-")));

    FILE *fp = stdout;
    if (!write_to_stdout) {
        blogc_mkdir_recursive(output);
        fp = fopen(output, "w");
        if (fp == NULL) {
            fprintf(stderr, "blogc: error: failed to open output file (%s): %s\n",
                output, strerror(errno));
            rv = 2;
            goto cleanup4;
        }
    }

    if (out != NULL)
        fprintf(fp, "%s", out);

    if (!write_to_stdout)
        fclose(fp);

cleanup4:
    free(out);
cleanup3:
    blogc_template_free_stmts(l);
cleanup2:
    bc_slist_free_full(s, (bc_free_func_t) bc_trie_free);
    bc_error_free(err);
cleanup:
    bc_trie_free(config);
    free(template);
    free(output);
    free(print);
    bc_slist_free_full(sources, free);
    return rv;
}
