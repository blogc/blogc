/*
 * blogc: A blog compiler.
 * Copyright (C) 2015 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file COPYING.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "utils/utils.h"
#include "source-parser.h"
#include "template-parser.h"
#include "loader.h"
#include "renderer.h"
#include "error.h"


static void
blogc_print_help(void)
{
    printf(
        "usage:\n"
        "    blogc [-h] -t TEMPLATE [-o OUTPUT] SOURCE [SOURCE ...] - A blog compiler.\n"
        "\n"
        "positional arguments:\n"
        "    SOURCE       source file(s)\n"
        "\n"
        "optional arguments:\n"
        "    -h, --help   show this help message and exit\n"
        "    -t TEMPLATE  template file\n"
        "    -o OUTPUT    output file\n");
}


static void
blogc_print_usage(void)
{
    printf("usage: blogc [-h] -t TEMPLATE [-o OUTPUT] SOURCE [SOURCE ...]\n");
}


int
main(int argc, char **argv)
{
    int rv = 0;

    char *template = NULL;
    char *output = NULL;
    b_slist_t *sources = NULL;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            switch (argv[i][1]) {
                case 'h':
                    blogc_print_help();
                    goto cleanup;
                case 't':
                    if (i + 1 < argc)
                        template = b_strdup(argv[++i]);
                    break;
                case 'o':
                    if (i + 1 < argc)
                        output = b_strdup(argv[++i]);
                    break;
            }
        }
        else
            sources = b_slist_append(sources, b_strdup(argv[i]));
    }

    if (template == NULL) {
        blogc_print_usage();
        fprintf(stderr, "blogc: error: argument -t is required\n");
        rv = 2;
        goto cleanup;
    }

    if (b_slist_length(sources) == 0) {
        blogc_print_usage();
        fprintf(stderr, "blogc: error: at least one source file is required\n");
        rv = 2;
        goto cleanup;
    }

    blogc_error_t *err = NULL;

    b_slist_t* l = blogc_template_parse_from_file(template, &err);
    if (err != NULL) {
        blogc_error_print(err);
        goto cleanup2;
    }

    b_slist_t *s = blogc_source_parse_from_files(sources, &err);
    if (err != NULL) {
        blogc_error_print(err);
        goto cleanup3;
    }

    char *out = blogc_render(l, s);
    printf("%s", out);
    free(out);

cleanup3:
    b_slist_free_full(s, (b_free_func_t) b_trie_free);
cleanup2:
    blogc_template_free_stmts(l);
    blogc_error_free(err);
cleanup:
    free(template);
    free(output);
    b_slist_free_full(sources, free);
    return rv;
}
