// SPDX-FileCopyrightText: 2014-2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
// SPDX-License-Identifier: BSD-3-Clause

#include <stdio.h>
#include <stdlib.h>
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

    if (argc == 3 && (0 == strcmp(argv[1], "-c"))) {
        return bgr_shell(argc, argv);
    }

    // this is a hack to make blogc-git-receiver work out-of-the-box as a
    // `command=` in authorized_keys file. it will only work if the command
    // path is absolute.
    char *ssh_orig = getenv("SSH_ORIGINAL_COMMAND");
    if (argc == 1 && ssh_orig != NULL && argv[0][0] == '/') {
        setenv("SHELL", argv[0], 1);
        char* _argv[] = {argv[0], "-c", ssh_orig};
        return bgr_shell(3, _argv);
    }

    fprintf(stderr, "error: this is a special shell, go away!\n");
    return 1;
}
