/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2019 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include "../common/utils.h"
#include "settings.h"
#include "shell-command-parser.h"
#include "shell.h"


static bool
lexists(const char *pathname)
{
    struct stat b;
    int tmp_errno = errno;
    bool rv = lstat(pathname, &b) == 0;
    errno = tmp_errno;
    return rv;
}


int
bgr_shell(int argc, char *argv[])
{
    int rv = 0;

    char *repo = NULL;
    char *quoted_repo = NULL;

    // get shell path
    char *self = getenv("SHELL");
    if (self == NULL) {
        fprintf(stderr, "error: failed to find blogc-git-receiver path\n");
        rv = 1;
        goto cleanup;
    }

    // get base dir path
    const char *bd = bgr_settings_get_base_dir();
    if (bd == NULL) {
        fprintf(stderr, "error: failed to find base directory path\n");
        rv = 1;
        goto cleanup;
    }

    // validate command and extract git repository
    char *tmp_repo = bgr_shell_command_parse(argv[2]);
    if (tmp_repo == NULL) {
        fprintf(stderr, "error: invalid git-shell command: %s\n", argv[2]);
        rv = 1;
        goto cleanup;
    }

    repo = bc_strdup_printf("%s/repos/%s", bd, tmp_repo);
    quoted_repo = bc_shell_quote(repo);
    free(tmp_repo);

    if (0 == strncmp(argv[2], "git-upload-", 11))  // no need to check len here
        goto git_exec;

    if (0 != access(repo, F_OK)) {
        char *git_init_cmd = bc_strdup_printf(
            "git init --bare %s > /dev/null", quoted_repo);
        if (0 != system(git_init_cmd)) {
            fprintf(stderr, "error: failed to create git repository: %s\n",
                repo);
            rv = 1;
            free(git_init_cmd);
            goto cleanup;
        }
        free(git_init_cmd);
    }

    if (0 != chdir(repo)) {
        fprintf(stderr, "error: failed to chdir (%s): %s\n", repo,
            strerror(errno));
        rv = 1;
        goto cleanup;
    }

    if (0 != access("hooks", F_OK)) {
        // openwrt git package won't install git templates, then the git
        // repositories created with it won't have the hooks/ directory.
        if (0 != mkdir("hooks", 0777)) {  // mkdir honors umask for us.
            fprintf(stderr, "error: failed to create directory (%s/hooks): "
                "%s\n", repo, strerror(errno));
            rv = 1;
            goto cleanup;
        }
    }

    if (0 != chdir("hooks")) {
        fprintf(stderr, "error: failed to chdir (%s/hooks): %s\n", repo,
            strerror(errno));
        rv = 1;
        goto cleanup;
    }

    if (lexists("pre-receive")) {
        if (0 != unlink("pre-receive")) {
            fprintf(stderr, "error: failed to remove old symlink "
                "(%s/hooks/pre-receive): %s\n", repo, strerror(errno));
            rv = 1;
            goto cleanup;
        }
    }

    if (0 != symlink(self, "pre-receive")) {
        fprintf(stderr, "error: failed to create symlink "
            "(%s/hooks/pre-receive): %s\n", repo, strerror(errno));
        rv = 1;
        goto cleanup;
    }

    if (lexists("post-receive")) {
        if (0 != unlink("post-receive")) {
            fprintf(stderr, "error: failed to remove old symlink "
                "(%s/hooks/post-receive): %s\n", repo, strerror(errno));
            rv = 1;
            goto cleanup;
        }
    }

    if (0 != symlink(self, "post-receive")) {
        fprintf(stderr, "error: failed to create symlink "
            "(%s/hooks/post-receive): %s\n", repo, strerror(errno));
        rv = 1;
        goto cleanup;
    }

git_exec:

    if (0 != chdir(bd)) {
        fprintf(stderr, "error: failed to chdir (%s): %s\n", bd, strerror(errno));
        rv = 1;
        goto cleanup;
    }

    // static allocation instead of bc_strdup_printf to avoid leaks
    char buffer[4096];
    char *command = bc_strdup(argv[2]);
    char *p;
    for (p = command; *p != ' ' && *p != '\0'; p++);
    if (*p == ' ')
        *p = '\0';

    if (sizeof(buffer) < (strlen(command) + strlen(quoted_repo) + 2)) {
        fprintf(stderr, "error: git-shell command is too big\n");
        rv = 1;
        goto cleanup;
    }

    if (0 > snprintf(buffer, sizeof(buffer), "%s %s", command, quoted_repo)) {
        fprintf(stderr, "error: failed to generate git-shell command\n");
        rv = 1;
        goto cleanup;
    }

    free(command);
    free(repo);
    free(quoted_repo);

    // this is a hack. no memory handling should be done inside this block
    if (NULL == getenv("__VALGRIND_ENABLED")) {
        execlp("git-shell", "git-shell", "-c", buffer, NULL);

        // execlp only returns on error, then something bad happened
        fprintf(stderr, "error: failed to execute git-shell\n");
        return 1;
    }

    printf("git-shell -c \"%s\"\n", buffer);  // used by tests, ignore
    return 0;

cleanup:
    free(repo);
    free(quoted_repo);
    return rv;
}
