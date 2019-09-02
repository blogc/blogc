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

#include <locale.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <squareball.h>

#include "ctx.h"
#include "rules.h"


static void
print_help(void)
{
    printf(
        "usage:\n"
        "    blogc-make [-h] [-v] [-D] [-V] [-f FILE] [RULE ...]\n"
        "               - A simple build tool for blogc.\n"
        "\n"
        "positional arguments:\n"
        "    RULE             build rule(s) to run. can include comma-separated\n"
        "                     key-value pairs of rule arguments in the format\n"
        "                     RULE:arg1=value1,arg2=value2,... (default: all)\n"
        "\n"
        "optional arguments:\n"
        "    -h               show this help message and exit\n"
        "    -v               show version and exit\n"
        "    -D               build for development environment\n"
        "    -V               be verbose when executing commands\n"
        "    -f FILE          read FILE as blogcfile\n");
    bm_rule_print_help();
}


static void
print_usage(void)
{
    printf("usage: blogc-make [-h] [-v] [-D] [-V] [-f FILE] [RULE ...]\n");
}


int
#ifdef MAKE_EMBEDDED
bm_main(int argc, char **argv)
#else
main(int argc, char **argv)
#endif
{
    setlocale(LC_ALL, "");

    int rv = 0;
    sb_error_t *err = NULL;

    sb_slist_t *rules = NULL;
    bool verbose = false;
    bool dev = false;
    char *blogcfile = NULL;
    bm_ctx_t *ctx = NULL;

    for (size_t i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            switch (argv[i][1]) {
                case 'h':
                    print_help();
                    goto cleanup;
                case 'v':
                    printf("%s\n", PACKAGE_STRING);
                    goto cleanup;
                case 'D':
                    dev = true;
                    break;
                case 'V':
                    verbose = true;
                    break;
                case 'f':
                    if (argv[i][2] != '\0')
                        blogcfile = sb_strdup(argv[i] + 2);
                    else if (i + 1 < argc)
                        blogcfile = sb_strdup(argv[++i]);
                    break;
#ifdef MAKE_EMBEDDED
                case 'm':
                    // no-op, for embedding into blogc binary.
                    break;
#endif
                default:
                    print_usage();
                    fprintf(stderr, "blogc-make: error: invalid argument: "
                        "-%c\n", argv[i][1]);
                    rv = 1;
                    goto cleanup;
            }
        }
        else {
            rules = sb_slist_append(rules, sb_strdup(argv[i]));
        }
    }

    if (rules == NULL) {
        rules = sb_slist_append(rules, sb_strdup("all"));
    }

    ctx = bm_ctx_new(NULL, blogcfile ? blogcfile : "blogcfile",
        argc > 0 ? argv[0] : NULL, &err);
    if (err != NULL) {
        fprintf(stderr, "blogc-make: error: %s\n", sb_error_to_string(err));
        rv = 1;
        goto cleanup;
    }
    ctx->dev = dev;
    ctx->verbose = verbose;

    rv = bm_rule_executor(ctx, rules);

cleanup:

    sb_slist_free_full(rules, free);
    free(blogcfile);
    bm_ctx_free(ctx);
    sb_error_free(err);

    return rv;
}
