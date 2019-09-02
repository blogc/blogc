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

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif /* HAVE_SYS_STAT_H */

#ifdef HAVE_SYSEXITS_H
#include <sysexits.h>
#else
#define EX_CONFIG 78
#endif /* HAVE_SYSEXITS_H */

#include <errno.h>
#include <locale.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <squareball.h>

#include "debug.h"
#include "template-parser.h"
#include "loader.h"
#include "renderer.h"

#ifdef MAKE_EMBEDDED
extern int bm_main(int argc, char **argv);
#endif

#ifndef PACKAGE_VERSION
#define PACKAGE_VERSION "Unknown"
#endif


static void
blogc_print_help(void)
{
    printf(
        "usage:\n"
        "    blogc "
#ifdef MAKE_EMBEDDED
        "[-m] "
#endif
        "[-h] [-v] [-d] [-i] [-l [-e SOURCE]] [-D KEY=VALUE ...] [-p KEY]\n"
        "          [-t TEMPLATE] [-o OUTPUT] [SOURCE ...] - A blog compiler.\n"
        "\n"
        "positional arguments:\n"
        "    SOURCE        source file(s)\n"
        "\n"
        "optional arguments:\n"
        "    -h            show this help message and exit\n"
        "    -v            show version and exit\n"
        "    -d            enable debug\n"
        "    -i            read list of source files from standard input\n"
        "    -l            build listing page, from multiple source files\n"
        "    -e SOURCE     source file with content for listing page. requires '-l'\n"
        "    -D KEY=VALUE  set global variable\n"
        "    -p KEY        show the value of a variable after source parsing and exit\n"
        "    -t TEMPLATE   template file\n"
        "    -o OUTPUT     output file\n"
#ifdef MAKE_EMBEDDED
        "    -m            call and pass arguments to embedded blogc-make\n"
#endif
        );
}


static void
blogc_print_usage(void)
{
    printf(
        "usage: blogc "
#ifdef MAKE_EMBEDDED
        "[-m] "
#endif
        "[-h] [-v] [-d] [-i] [-l [-e SOURCE]] [-D KEY=VALUE ...] [-p KEY]\n"
        "             [-t TEMPLATE] [-o OUTPUT] [SOURCE ...]\n");
}


static void
blogc_mkdir_recursive(const char *filename)
{
    char *fname = sb_strdup(filename);
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


static sb_slist_t*
blogc_read_stdin_to_list(sb_slist_t *l)
{
    char buffer[4096];
    while (NULL != fgets(buffer, 4096, stdin)) {
        size_t len = strlen(buffer);
        if (len == 0)
            continue;
        if (buffer[0] == '#')
            continue;
        if (len >= 2 && ((buffer[len - 2] == '\r') || (buffer[len - 2] == '\n')))
            buffer[len - 2] = '\0';
        if ((buffer[len - 1] == '\r') || (buffer[len - 1] == '\n'))
            buffer[len - 1] = '\0';
        if (strlen(buffer) == 0)
            continue;
        l = sb_slist_append(l, sb_strdup(buffer));
    }
    return l;
}


int
main(int argc, char **argv)
{
    setlocale(LC_ALL, "");

    int rv = 0;

#ifdef MAKE_EMBEDDED
    bool embedded = false;
#endif
    bool debug = false;
    bool input_stdin = false;
    bool listing = false;
    char *listing_entry = NULL;
    char *template = NULL;
    char *output = NULL;
    char *print = NULL;
    char *tmp = NULL;
    char **pieces = NULL;

    sb_slist_t *sources = NULL;
    sb_trie_t *listing_entry_source = NULL;
    sb_trie_t *config = sb_trie_new(free);
    sb_trie_insert(config, "BLOGC_VERSION", sb_strdup(PACKAGE_VERSION));

    for (size_t i = 1; i < argc; i++) {
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
                case 'i':
                    input_stdin = true;
                    break;
                case 'l':
                    listing = true;
                    break;
                case 'e':
                    if (argv[i][2] != '\0')
                        listing_entry = sb_strdup(argv[i] + 2);
                    else if (i + 1 < argc)
                        listing_entry = sb_strdup(argv[++i]);
                    break;
                case 't':
                    if (argv[i][2] != '\0')
                        template = sb_strdup(argv[i] + 2);
                    else if (i + 1 < argc)
                        template = sb_strdup(argv[++i]);
                    break;
                case 'o':
                    if (argv[i][2] != '\0')
                        output = sb_strdup(argv[i] + 2);
                    else if (i + 1 < argc)
                        output = sb_strdup(argv[++i]);
                    break;
                case 'p':
                    if (argv[i][2] != '\0')
                        print = sb_strdup(argv[i] + 2);
                    else if (i + 1 < argc)
                        print = sb_strdup(argv[++i]);
                    break;
                case 'D':
                    if (argv[i][2] != '\0')
                        tmp = argv[i] + 2;
                    else if (i + 1 < argc)
                        tmp = argv[++i];
                    if (tmp != NULL) {
                        if (!sb_utf8_validate((uint8_t*) tmp, strlen(tmp))) {
                            fprintf(stderr, "blogc: error: invalid value for "
                                "-D (must be valid UTF-8 string): %s\n", tmp);
                            goto cleanup;
                        }
                        pieces = sb_str_split(tmp, '=', 2);
                        if (sb_strv_length(pieces) != 2) {
                            fprintf(stderr, "blogc: error: invalid value for "
                                "-D (must have an '='): %s\n", tmp);
                            sb_strv_free(pieces);
                            rv = 1;
                            goto cleanup;
                        }
                        for (size_t j = 0; pieces[0][j] != '\0'; j++) {
                            if (!((pieces[0][j] >= 'A' && pieces[0][j] <= 'Z') ||
                                pieces[0][j] == '_'))
                            {
                                fprintf(stderr, "blogc: error: invalid value "
                                    "for -D (configuration key must be uppercase "
                                    "with '_'): %s\n", pieces[0]);
                                sb_strv_free(pieces);
                                rv = 1;
                                goto cleanup;
                            }
                        }
                        sb_trie_insert(config, pieces[0], sb_strdup(pieces[1]));
                        sb_strv_free(pieces);
                        pieces = NULL;
                    }
                    break;
#ifdef MAKE_EMBEDDED
                case 'm':
                    embedded = true;
                    break;
#endif
                default:
                    blogc_print_usage();
                    fprintf(stderr, "blogc: error: invalid argument: -%c\n",
                        argv[i][1]);
                    rv = 1;
                    goto cleanup;
            }
        }
        else {
            sources = sb_slist_append(sources, sb_strdup(argv[i]));
        }

#ifdef MAKE_EMBEDDED
        if (embedded) {
            rv = bm_main(argc, argv);
            goto cleanup;
        }
#endif

    }

    if (input_stdin)
        sources = blogc_read_stdin_to_list(sources);

    if (!listing && sb_slist_length(sources) == 0) {
        blogc_print_usage();
        fprintf(stderr, "blogc: error: one source file is required\n");
        rv = 1;
        goto cleanup;
    }

    if (!listing && sb_slist_length(sources) > 1) {
        blogc_print_usage();
        fprintf(stderr, "blogc: error: only one source file should be provided, "
            "if running without '-l'\n");
        rv = 1;
        goto cleanup;
    }

    sb_error_t *err = NULL;

    sb_slist_t *s = blogc_source_parse_from_files(config, sources, &err);
    if (err != NULL) {
        fprintf(stderr, "blogc: error: %s\n", sb_error_to_string(err));
        rv = 1;
        goto cleanup2;
    }

    if (listing && listing_entry != NULL) {
        listing_entry_source = blogc_source_parse_from_file(listing_entry, &err);
        if (err != NULL) {
            fprintf(stderr, "blogc: error: %s\n", sb_error_to_string(err));
            rv = 1;
            goto cleanup2;
        }
    }

    if (print != NULL) {
        sb_trie_t *local = NULL;
        if (!listing && s != NULL) {
            local = s->data;
        }
        char *val = blogc_format_variable(print, config, local, NULL);
        if (val == NULL) {
            fprintf(stderr, "blogc: error: variable not found: %s\n",
                print);
            rv = EX_CONFIG;
        }
        else {
            printf("%s\n", val);
        }
        free(val);
        goto cleanup2;
    }

    if (template == NULL) {
        blogc_print_usage();
        fprintf(stderr, "blogc: error: argument -t is required when rendering content\n");
        rv = 1;
        goto cleanup2;
    }

    sb_slist_t* l = blogc_template_parse_from_file(template, &err);
    if (err != NULL) {
        fprintf(stderr, "blogc: error: %s\n", sb_error_to_string(err));
        rv = 1;
        goto cleanup3;
    }

    if (debug)
        blogc_debug_template(l);

    char *out = blogc_render(l, s, listing_entry_source, config, listing);

    bool write_to_stdout = (output == NULL || (0 == strcmp(output, "-")));

    FILE *fp = stdout;
    if (!write_to_stdout) {
        blogc_mkdir_recursive(output);
        fp = fopen(output, "w");
        if (fp == NULL) {
            fprintf(stderr, "blogc: error: failed to open output file (%s): %s\n",
                output, strerror(errno));
            rv = 1;
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
    blogc_template_free_ast(l);
cleanup2:
    sb_slist_free_full(s, (sb_free_func_t) sb_trie_free);
    sb_error_free(err);
cleanup:
    sb_trie_free(config);
    free(template);
    free(output);
    free(print);
    free(listing_entry);
    sb_trie_free(listing_entry_source);
    sb_slist_free_full(sources, free);
    return rv;
}
