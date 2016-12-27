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

#include <locale.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../common/error.h"
#include "../common/utils.h"
#include "ctx.h"
#include "rules.h"


static void
print_help(void)
{
    printf(
        "usage:\n"
        "    blogc-make [-h] [-v] [-V] [-f FILE] [RULE ...]\n"
        "               - A simple build tool for blogc.\n"
        "\n"
        "positional arguments:\n"
        "    RULE          build rule(s) to run (default: all)\n"
        "\n"
        "optional arguments:\n"
        "    -h            show this help message and exit\n"
        "    -v            show version and exit\n"
        "    -V            be verbose when executing commands\n"
        "    -f FILE       settings file (default: settings.ini)\n");
    bm_rule_print_help();
}


static void
print_usage(void)
{
    printf("usage: blogc-make [-h] [-v] [-V] [-f FILE] [RULE ...]\n");
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
    bc_error_t *err = NULL;

    bc_slist_t *rules = NULL;
    bool verbose = false;
    char *settings_file = NULL;
    bm_ctx_t *ctx = NULL;

    for (unsigned int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            switch (argv[i][1]) {
                case 'h':
                    print_help();
                    goto cleanup;
                case 'v':
                    printf("%s\n", PACKAGE_STRING);
                    goto cleanup;
                case 'V':
                    verbose = true;
                    break;
                case 'f':
                    if (argv[i][2] != '\0')
                        settings_file = bc_strdup(argv[i] + 2);
                    else if (i + 1 < argc)
                        settings_file = bc_strdup(argv[++i]);
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
                    rv = 3;
                    goto cleanup;
            }
        }
        else {
            rules = bc_slist_append(rules, bc_strdup(argv[i]));
        }
    }

    if (rules == NULL) {
        rules = bc_slist_append(rules, bc_strdup("all"));
    }

    ctx = bm_ctx_new(settings_file ? settings_file : "settings.ini", &err);
    if (err != NULL) {
        bc_error_print(err, "blogc-make");
        rv = 3;
        goto cleanup;
    }

    rv = bm_rule_executor(ctx, rules, verbose);

cleanup:

    bc_slist_free_full(rules, free);
    free(settings_file);
    bm_ctx_free(ctx);
    bc_error_free(err);

    return rv;
}
