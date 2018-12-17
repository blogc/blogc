/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2018 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <libgen.h>
#include <unistd.h>
#include <stdlib.h>
#include "../common/utils.h"
#include "../common/config-parser.h"
#include "settings.h"
#include "post-receive.h"


int
bgr_post_receive_hook(int argc, char *argv[])
{
    int rv = 0;
    char *mirror = NULL;

    char *hooks_dir = dirname(argv[0]);  // this was validated by main()
    char *real_hooks_dir = realpath(hooks_dir, NULL);
    if (real_hooks_dir == NULL) {
        fprintf(stderr, "error: failed to guess repository root: %s\n",
            strerror(errno));
        return 3;
    }

    char *repo_path = bc_strdup(dirname(real_hooks_dir));
    free(real_hooks_dir);
    if (0 != chdir(repo_path)) {
        fprintf(stderr, "error: failed to change to repository root\n");
        rv = 3;
        goto cleanup;
    }

    // local repository settings should take precedence, so if the repo have
    // the 'mirror' remote, just push to it.
    // this will be removed at some point, but will be kept for compatibility
    // with old setups.
    if ((0 == system("git config --local remote.mirror.pushurl > /dev/null")) ||
        (0 == system("git config --local remote.mirror.url > /dev/null")))
    {
        mirror = bc_strdup("mirror");
        goto push;
    }

    bc_config_t *config = bgr_settings_parse();
    if (config == NULL) {
        fprintf(stderr, "warning: repository mirroring disabled\n");
        goto cleanup;
    }

    char *section = bgr_settings_get_section(config, repo_path);
    if (section == NULL) {
        fprintf(stderr, "warning: repository mirroring disabled\n");
        bc_config_free(config);
        goto cleanup;
    }

    mirror = bc_strdup(bc_config_get(config, section, "mirror"));
    free(section);
    bc_config_free(config);

    if (mirror == NULL) {
        fprintf(stderr, "warning: repository mirroring disabled\n");
        goto cleanup;
    }

push:

    {
        char *git_cmd = bc_strdup_printf("git push --mirror %s", mirror);
        if (0 != system(git_cmd))
            fprintf(stderr, "warning: failed push to git mirror\n");
        free(git_cmd);
    }

    free(mirror);

cleanup:
    free(repo_path);

    return rv;
}
