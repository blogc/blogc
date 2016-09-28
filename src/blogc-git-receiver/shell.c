/*
 * blogc: A blog compiler.
 * Copyright (C) 2015-2016 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "../common/utils.h"
#include "shell.h"

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 4096
#endif


int
bgr_shell(int argc, char *argv[])
{
    int rv = 0;

    char *repo = NULL;
    char *command_orig = NULL;
    char *command_name = NULL;
    char command_new[BUFFER_SIZE];

    bool exec_git = false;

    // validate git command
    size_t len = strlen(argv[2]);
    if (!((len > 17 && (0 == strncmp(argv[2], "git-receive-pack ", 17))) ||
          (len > 16 && (0 == strncmp(argv[2], "git-upload-pack ", 16))) ||
          (len > 19 && (0 == strncmp(argv[2], "git-upload-archive ", 19)))))
    {
        fprintf(stderr, "error: unsupported git command: %s\n", argv[2]);
        rv = 1;
        goto cleanup;
    }

    // get shell path
    char *self = getenv("SHELL");
    if (self == NULL) {
        fprintf(stderr, "error: failed to find blogc-git-receiver path\n");
        rv = 1;
        goto cleanup;
    }

    // get home path
    char *home = getenv("HOME");
    if (home == NULL) {
        fprintf(stderr, "error: failed to find user home path\n");
        rv = 1;
        goto cleanup;
    }

    // get git repository
    command_orig = bc_strdup(argv[2]);
    char *p, *r;
    for (p = command_orig; *p != ' ' && *p != '\0'; p++);
    if (*p == ' ')
        p++;
    if (*p == '\'' || *p == '"')
        p++;
    if (*p == '/')
        p++;
    for (r = p; *p != '\'' && *p != '"' && *p != '\0'; p++);
    if (*p == '\'' || *p == '"')
        *p = '\0';
    if (*--p == '/')
        *p = '\0';

    repo = bc_strdup_printf("repos/%s", r);

    // check if repository is sane
    if (0 == strlen(repo)) {
        fprintf(stderr, "error: invalid repository\n");
        rv = 1;
        goto cleanup;
    }

    if (0 == strncmp(argv[2], "git-upload-", 11))  // no need to check len here
        goto git_exec;

    if (0 != chdir(home)) {
        fprintf(stderr, "error: failed to chdir (%s): %s\n", home,
            strerror(errno));
        rv = 1;
        goto cleanup;
    }

    if (0 != access(repo, F_OK)) {
        char *git_init_cmd = bc_strdup_printf(
            "git init --bare \"%s\" > /dev/null", repo);
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
        fprintf(stderr, "error: failed to chdir (%s/%s): %s\n", home, repo,
            strerror(errno));
        rv = 1;
        goto cleanup;
    }

    if (0 != access("hooks", F_OK)) {
        // openwrt git package won't install git templates, then the git
        // repositories created with it won't have the hooks/ directory.
        if (0 != mkdir("hooks", 0777)) {  // mkdir honors umask for us.
            fprintf(stderr, "error: failed to create directory (%s/%s/hooks): "
                "%s\n", home, repo, strerror(errno));
            rv = 1;
            goto cleanup;
        }
    }

    if (0 != chdir("hooks")) {
        fprintf(stderr, "error: failed to chdir (%s/%s/hooks): %s\n", home,
            repo, strerror(errno));
        rv = 1;
        goto cleanup;
    }

    if (0 == access("pre-receive", F_OK)) {
        if (0 != unlink("pre-receive")) {
            fprintf(stderr, "error: failed to remove old symlink "
                "(%s/%s/hooks/pre-receive): %s\n", home, repo, strerror(errno));
            rv = 1;
            goto cleanup;
        }
    }

    if (0 != symlink(self, "pre-receive")) {
        fprintf(stderr, "error: failed to create symlink "
            "(%s/%s/hooks/pre-receive): %s\n", home, repo, strerror(errno));
        rv = 1;
        goto cleanup;
    }

    if (0 == access("post-receive", F_OK)) {
        if (0 != unlink("post-receive")) {
            fprintf(stderr, "error: failed to remove old symlink "
                "(%s/%s/hooks/post-receive): %s\n", home, repo, strerror(errno));
            rv = 1;
            goto cleanup;
        }
    }

    if (0 != symlink(self, "post-receive")) {
        fprintf(stderr, "error: failed to create symlink "
            "(%s/%s/hooks/post-receive): %s\n", home, repo, strerror(errno));
        rv = 1;
        goto cleanup;
    }

    if (0 != chdir(home)) {
        fprintf(stderr, "error: failed to chdir (%s): %s\n", home,
            strerror(errno));
        rv = 1;
        goto cleanup;
    }

git_exec:
    command_name = bc_strdup(argv[2]);
    for (p = command_name; *p != ' ' && *p != '\0'; p++);
    if (*p == ' ')
        *p = '\0';

    if (BUFFER_SIZE < (strlen(command_name) + strlen(repo) + 4)) {
        fprintf(stderr, "error: git-shell command is too big\n");
        rv = 1;
        goto cleanup;
    }

    if (snprintf(command_new, BUFFER_SIZE, "%s '%s'", command_name, repo))
        exec_git = true;

cleanup:
    free(repo);
    free(command_orig);
    free(command_name);

    if (exec_git) {
        execlp("git-shell", "git-shell", "-c", command_new, NULL);

        // execlp only returns on error, then something bad happened
        fprintf(stderr, "error: failed to execute git-shell\n");
        rv = 1;
    }

    return rv;
}
