/*
 * blogc: A blog compiler.
 * Copyright (C) 2015-2016 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#include <time.h>
#include <libgen.h>
#include "../common/utils.h"
#include "../common/stdin.h"
#include "pre-receive-parser.h"
#include "pre-receive.h"


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
        char *f = bc_strdup_printf("%s/%s", dir, e->d_name);
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


int
bgr_pre_receive_hook(int argc, char *argv[])
{
    int rv = 0;
    char *master = NULL;

    if (isatty(STDIN_FILENO)) {
        char *hooks_dir = dirname(argv[0]);  // this was validated by main()
        char *real_hooks_dir = realpath(hooks_dir, NULL);
        if (real_hooks_dir == NULL) {
            fprintf(stderr, "error: failed to guess repository root.\n");
            return 1;
        }
        char *repo_dir = dirname(real_hooks_dir);
        if (0 != chdir(repo_dir)) {
            fprintf(stderr, "error: failed to change to repository root\n");
            free(real_hooks_dir);
            return 1;
        }
        char *htdocs_sym = bc_strdup_printf("%s/htdocs", repo_dir);
        free(real_hooks_dir);
        if (0 != access(htdocs_sym, F_OK)) {
            fprintf(stderr, "error: no previous build found. nothing to "
                "rebuild.\n");
            return 1;
        }
        char *build_dir = realpath(htdocs_sym, NULL);
        free(htdocs_sym);
        if (build_dir == NULL) {
            fprintf(stderr, "error: failed to get the hash of last built "
                "commit.\n");
            return 1;
        }
        char **pieces = bc_str_split(basename(build_dir), '-', 2);
        free(build_dir);
        if (bc_strv_length(pieces) != 2) {
            fprintf(stderr, "error: failed to parse the hash of last built "
                "commit.\n");
            return 1;
        }
        master = bc_strdup(pieces[0]);
        bc_strv_free(pieces);
    }
    else {
        char *input = bc_stdin_read();
        master = bgr_pre_receive_parse(input);
        free(input);
    }

    if (master == NULL) {
        fprintf(stderr, "warning: no reference to master branch found. "
            "nothing to deploy.\n");
        return rv;
    }

    char *repo_dir = NULL;
    char *output_dir = NULL;

    repo_dir = getcwd(NULL, 0);
    if (repo_dir == NULL) {
        fprintf(stderr, "error: failed to get repository path: %s\n",
            strerror(errno));
        rv = 1;
        goto cleanup;
    }

    char dir[] = "/tmp/blogc_XXXXXX";
    if (NULL == mkdtemp(dir)) {
        rv = 1;
        goto cleanup;
    }

    char *git_archive_cmd = bc_strdup_printf(
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

    const char *make_impl = NULL;

    if (127 != WEXITSTATUS(system("gmake -f /dev/null 2> /dev/null > /dev/null"))) {
        make_impl = "gmake";
    }
    else if (127 != WEXITSTATUS(system("make -f /dev/null 2> /dev/null > /dev/null"))) {
        make_impl = "make";
    }

    if (make_impl == NULL) {
        fprintf(stderr, "error: no 'make' implementation found\n");
        rv = 1;
        goto cleanup;
    }

    unsigned long epoch = time(NULL);
    output_dir = bc_strdup_printf("%s/builds/%s-%lu", home, master, epoch);

    if (0 == access(output_dir, F_OK)) {
        char *tmp = output_dir;
        output_dir = bc_strdup_printf("%s-", tmp);
        free(tmp);
    }

    char *gmake_cmd = bc_strdup_printf(
        "%s -j%d OUTPUT_DIR=\"%s\" BLOGC_GIT_RECEIVER=1", make_impl,
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

    char *htdocs_sym = realpath("htdocs", NULL);
    if ((htdocs_sym != NULL) && (0 != unlink("htdocs"))) {
        fprintf(stderr, "error: failed to remove symlink (%s/htdocs): %s\n",
            repo_dir, strerror(errno));
        rmdir_recursive(output_dir);
        rv = 1;
        goto cleanup2;
    }

    if (0 != symlink(output_dir, "htdocs")) {
        fprintf(stderr, "error: failed to create symlink (%s/htdocs): %s\n",
            repo_dir, strerror(errno));
        rmdir_recursive(output_dir);
        rv = 1;
        goto cleanup2;
    }

    if (htdocs_sym != NULL)
        rmdir_recursive(htdocs_sym);

cleanup2:
    free(htdocs_sym);

cleanup:
    free(master);
    free(output_dir);
    rmdir_recursive(dir);
    free(repo_dir);
    return rv;
}
