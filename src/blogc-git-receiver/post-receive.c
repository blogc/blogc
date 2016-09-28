/*
 * blogc: A blog compiler.
 * Copyright (C) 2015-2016 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#include <stdio.h>
#include <stdlib.h>


int
bgr_post_receive_hook(int argc, char *argv[])
{
    if (0 != system("git config --local remote.mirror.pushurl &> /dev/null")) {
        if (0 != system("git config --local remote.mirror.url &> /dev/null")) {
            fprintf(stderr, "warning: repository mirroring disabled\n");
            return 0;
        }
    }

    // at this point we know that we have a remote called mirror, we can just
    // push to it.
    if (0 != system("git push --mirror mirror"))
        fprintf(stderr, "warning: failed push to git mirror\n");

    return 0;
}
