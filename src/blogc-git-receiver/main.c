/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2017 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include "shell.h"
#include "pre-receive.h"
#include "post-receive.h"


int
main(int argc, char *argv[])
{
    if (argc > 0) {
        if (0 == strcmp(basename(argv[0]), "pre-receive"))
            return bgr_pre_receive_hook(argc, argv);
        if (0 == strcmp(basename(argv[0]), "post-receive"))
            return bgr_post_receive_hook(argc, argv);
    }

    if (argc == 3 && (0 == strcmp(argv[1], "-c")))
        return bgr_shell(argc, argv);

    fprintf(stderr, "error: this is a special shell, go away!\n");
    return 3;
}
