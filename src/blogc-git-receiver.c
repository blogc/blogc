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

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>

#include "utils.h"

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 4096
#endif


static unsigned int
cpu_count(void)
{
#ifdef _SC_NPROCESSORS_ONLN
    long num = sysconf(_SC_NPROCESSORS_ONLN);
    if (num >= 1)
        return (unsigned int) num;
#endif
    return 1;
}


static void
rmdir_recursive(const char *dir)
{
    struct stat buf;
    if (0 != stat(dir, &buf)) {
        fprintf(stderr, "warning: failed to remove directory (%s): %s\n", dir,
            strerror(errno));
        return;
    }
    if (!S_ISDIR(buf.st_mode)) {
        fprintf(stderr, "error: trying to remove invalid directory: %s\n", dir);
        exit(2);
    }
    DIR *d = opendir(dir);
    if (d == NULL) {
        fprintf(stderr, "error: failed to open directory: %s\n",
            strerror(errno));
        exit(2);
    }
    struct dirent *e;
    while (NULL != (e = readdir(d))) {
        if ((0 == strcmp(e->d_name, ".")) || (0 == strcmp(e->d_name, "..")))
            continue;
        char *f = sb_strdup_printf("%s/%s", dir, e->d_name);
        if (0 != stat(f, &buf)) {
            fprintf(stderr, "error: failed to stat directory entry (%s): %s\n",
                e->d_name, strerror(errno));
            free(f);
            exit(2);
        }
        if (S_ISDIR(buf.st_mode)) {
            rmdir_recursive(f);
        }
        else if (0 != unlink(f)) {
            fprintf(stderr, "error: failed to remove file (%s): %s\n", f,
                strerror(errno));
            free(f);
            exit(2);
        }
        free(f);
    }
    if (0 != closedir(d)) {
        fprintf(stderr, "error: failed to close directory: %s\n",
            strerror(errno));
        exit(2);
    }
    if (0 != rmdir(dir)) {
        fprintf(stderr, "error: failed to remove directory: %s\n",
            strerror(errno));
        exit(2);
    }
}


static int
git_shell(int argc, char *argv[])
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
    command_orig = sb_strdup(argv[2]);
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

    repo = sb_strdup_printf("repos/%s", r);

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
        char *git_init_cmd = sb_strdup_printf(
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
    command_name = sb_strdup(argv[2]);
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


static int
git_post_receive_hook(int argc, char *argv[])
{
    if (0 != system("git remote get-url --push mirror &> /dev/null"))
        return 0;

    // at this point we know that we have a remote called mirror, we can just
    // push to it.
    if (0 != system("git push --mirror mirror"))
        fprintf(stderr, "warning: failed push to git mirror\n");

    return 0;
}


typedef enum {
    START_OLD = 1,
    OLD,
    START_NEW,
    NEW,
    START_REF,
    REF
} input_state_t;


static int
git_pre_receive_hook(int argc, char *argv[])
{
    int c;
    char buffer[BUFFER_SIZE];

    input_state_t state = START_OLD;
    size_t i = 0;
    size_t start = 0;

    int rv = 0;
    char *new = NULL;
    char *master = NULL;

    while (EOF != (c = getc(stdin))) {

        buffer[i] = (char) c;

        switch (state) {
            case START_OLD:
                start = i;
                state = OLD;
                break;
            case OLD:
                if (c != ' ')
                    break;
                // no need to store old
                state = START_NEW;
                break;
            case START_NEW:
                start = i;
                state = NEW;
                break;
            case NEW:
                if (c != ' ')
                    break;
                state = START_REF;
                new = strndup(buffer + start, i - start);
                break;
            case START_REF:
                start = i;
                state = REF;
                break;
            case REF:
                if (c != '\n')
                    break;
                state = START_OLD;
                // we just care about a ref (refs/heads/master), everything
                // else is disposable :)
                if (!((i - start == 17) &&
                      (0 == strncmp("refs/heads/master", buffer + start, 17))))
                {
                    free(new);
                    new = NULL;
                    break;
                }
                master = new;
                break;
        }

        if (++i >= BUFFER_SIZE) {
            fprintf(stderr, "error: pre-receive hook payload is too big.\n");
            rv = 1;
            goto cleanup2;
        }
    }

    if (master == NULL) {
        fprintf(stderr, "warning: no reference to master branch found. "
            "nothing to deploy.\n");
        goto cleanup2;
    }

    char *repo_dir = NULL;
    char *output_dir = NULL;

    if (NULL == getcwd(buffer, BUFFER_SIZE)) {
        fprintf(stderr, "error: failed to get repository remote path: %s\n",
            strerror(errno));
        rv = 1;
        goto cleanup;
    }

    repo_dir = sb_strdup(buffer);

    char dir[] = "/tmp/blogc_XXXXXX";
    if (NULL == mkdtemp(dir)) {
        rv = 1;
        goto cleanup;
    }

    char *git_archive_cmd = sb_strdup_printf(
        "git archive \"%s\" | tar -x -C \"%s\" -f -", master, dir);
    if (0 != system(git_archive_cmd)) {
        fprintf(stderr, "error: failed to extract git content to temporary "
            "directory: %s\n", dir);
        rv = 1;
        free(git_archive_cmd);
        goto cleanup;
    }
    free(git_archive_cmd);

    if (0 != chdir(dir)) {
        fprintf(stderr, "error: failed to chdir (%s): %s\n", dir,
            strerror(errno));
        rv = 1;
        goto cleanup;
    }

    if ((0 != access("Makefile", F_OK)) && (0 != access("GNUMakefile", F_OK))) {
        fprintf(stderr, "warning: no makefile found. skipping ...\n");
        goto cleanup;
    }

    char *home = getenv("HOME");
    if (home == NULL) {
        fprintf(stderr, "error: failed to find user home path\n");
        rv = 1;
        goto cleanup;
    }

    unsigned long epoch = time(NULL);
    output_dir = sb_strdup_printf("%s/builds/%s-%lu", home, master, epoch);
    char *gmake_cmd = sb_strdup_printf(
        "gmake -j%d OUTPUT_DIR=\"%s\" BLOGC_GIT_RECEIVER=1",
        cpu_count(), output_dir);
    fprintf(stdout, "running command: %s\n\n", gmake_cmd);
    fflush(stdout);
    if (0 != system(gmake_cmd)) {
        fprintf(stderr, "error: failed to build website ...\n");
        rmdir_recursive(output_dir);
        free(gmake_cmd);
        rv = 1;
        goto cleanup;
    }
    free(gmake_cmd);

    if (0 != chdir(repo_dir)) {
        fprintf(stderr, "error: failed to chdir (%s): %s\n", repo_dir,
            strerror(errno));
        rmdir_recursive(output_dir);
        rv = 1;
        goto cleanup;
    }

    char *htdocs_sym = NULL;
    ssize_t htdocs_sym_len = readlink("htdocs", buffer, BUFFER_SIZE);
    if (0 < htdocs_sym_len) {
        if (0 != unlink("htdocs")) {
            fprintf(stderr, "error: failed to remove symlink (%s/htdocs): %s\n",
                repo_dir, strerror(errno));
            rmdir_recursive(output_dir);
            rv = 1;
            goto cleanup;
        }
        buffer[htdocs_sym_len] = '\0';
        htdocs_sym = buffer;
    }

    if (0 != symlink(output_dir, "htdocs")) {
        fprintf(stderr, "error: failed to create symlink (%s/htdocs): %s\n",
            repo_dir, strerror(errno));
        rmdir_recursive(output_dir);
        rv = 1;
        goto cleanup;
    }

    if (htdocs_sym != NULL)
        rmdir_recursive(htdocs_sym);

cleanup:
    free(output_dir);
    rmdir_recursive(dir);
    free(repo_dir);
cleanup2:
    free(new);
    return rv;
}


int
main(int argc, char *argv[])
{
    if (argc > 0) {
        if (0 == strcmp(basename(argv[0]), "pre-receive"))
            return git_pre_receive_hook(argc, argv);
        if (0 == strcmp(basename(argv[0]), "post-receive"))
            return git_post_receive_hook(argc, argv);
    }

    if (argc == 3 && (0 == strcmp(argv[1], "-c")))
        return git_shell(argc, argv);

    fprintf(stderr, "error: this is a special shell, go away!\n");
    return 1;
}
