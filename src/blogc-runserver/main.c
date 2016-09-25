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

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../common/utils.h"
#include "httpd.h"


static void
print_help(void)
{
    printf(
        "usage:\n"
        "    blogc-runserver [-h] [-v] [-t HOST] [-p PORT] [-m THREADS] DOCROOT\n"
        "                    - A simple HTTP server to test blogc websites.\n"
        "\n"
        "positional arguments:\n"
        "    DOCROOT       document root directory\n"
        "\n"
        "optional arguments:\n"
        "    -h            show this help message and exit\n"
        "    -v            show version and exit\n"
        "    -t HOST       set server listen address (default: 127.0.0.1)\n"
        "    -p PORT       set server listen port (default: 8080)\n"
        "    -m THREADS    set maximum number of threads to spawn (default: 20)\n");
}


static void
print_usage(void)
{
    printf("usage: blogc-runserver [-h] [-v] [-t HOST] [-p PORT] [-m THREADS] DOCROOT\n");
}


int
main(int argc, char **argv)
{
    signal(SIGPIPE, SIG_IGN);

    int rv = 0;
    char *host = NULL;
    char *docroot = NULL;
    unsigned short port = 8080;
    size_t max_threads = 20;

    unsigned int args = 0;

    for (unsigned int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            switch (argv[i][1]) {
                case 'h':
                    print_help();
                    goto cleanup;
                case 'v':
                    printf("%s\n", PACKAGE_STRING);
                    goto cleanup;
                case 't':
                    if (argv[i][2] != '\0')
                        host = bc_strdup(argv[i] + 2);
                    else
                        host = bc_strdup(argv[++i]);
                    break;
                case 'p':
                    if (argv[i][2] != '\0')
                        port = strtoul(argv[i] + 2, NULL, 10);
                    else
                        port = strtoul(argv[++i], NULL, 10);
                    break;
                case 'm':
                    if (argv[i][2] != '\0')
                        max_threads = strtoul(argv[i] + 2, NULL, 10);
                    else
                        max_threads = strtoul(argv[++i], NULL, 10);
                    break;
                default:
                    print_usage();
                    fprintf(stderr, "blogc-runserver: error: invalid "
                        "argument: -%c\n", argv[i][1]);
                    rv = 2;
                    goto cleanup;
            }
        }
        else {
            if (args > 0) {
                print_usage();
                fprintf(stderr, "blogc-runserver: error: only one positional "
                    "argument allowed\n");
                rv = 2;
                goto cleanup;
            }
            args++;
            docroot = bc_strdup(argv[i]);
        }
    }

    if (docroot == NULL) {
        print_usage();
        fprintf(stderr, "blogc-runserver: error: document root directory "
            "required\n");
        rv = 2;
        goto cleanup;
    }

    if (max_threads <= 0 || max_threads > 1000) {
        print_usage();
        fprintf(stderr, "blogc-runserver: error: invalid value for -m. "
            "Must be integer > 0 and <= 1000\n");
        rv = 2;
        goto cleanup;
    }

    if (host == NULL)
        host = bc_strdup("127.0.0.1");

    rv = br_httpd_run(host, port, docroot, max_threads);

cleanup:
    free(host);
    free(docroot);

    return rv;
}
