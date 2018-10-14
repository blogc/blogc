/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2018 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <libgen.h>
#include "../common/compat.h"
#include "../common/utils.h"
#include "../common/stdin.h"
#include "settings.h"
#include "pre-receive-parser.h"
#include "pre-receive.h"


static size_t
cpu_count(void)
{
#ifdef _SC_NPROCESSORS_ONLN
    long num = sysconf(_SC_NPROCESSORS_ONLN);
    if (num >= 1)
        return (size_t) num;
#endif
    return 1;
}


static void
rmdir_recursive(const char *dir)
{
    if (dir == NULL)
        return;
    struct stat buf;
    if (0 != stat(dir, &buf)) {
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
    char *output_dir = NULL;
    char *tmpdir = NULL;
    char *sym = NULL;

    char *hooks_dir = dirname(argv[0]);  // this was validated by main()
    char *real_hooks_dir = realpath(hooks_dir, NULL);
    if (real_hooks_dir == NULL) {
        fprintf(stderr, "error: failed to guess repository root.\n");
        return 3;
    }

    char *repo_dir = bc_strdup(dirname(real_hooks_dir));
    free(real_hooks_dir);
    if (0 != chdir(repo_dir)) {
        fprintf(stderr, "error: failed to change to repository root\n");
        rv = 3;
        goto cleanup;
    }

    bc_config_t *config = bgr_settings_parse();
    if (config == NULL) {
        goto default_sym;
    }

    char *section = bgr_settings_get_section(config, repo_dir);
    if (section == NULL) {
        bc_config_free(config);
        goto default_sym;
    }

    const char *sym_tmp = bc_config_get(config, section, "symlink");
    if (sym_tmp == NULL) {
        free(section);
        bc_config_free(config);
        goto default_sym;
    }

    sym = bc_str_starts_with(sym_tmp, "/") ? bc_strdup(sym_tmp) :
            bc_strdup_printf("%s/%s", repo_dir, sym_tmp);
    free(section);
    bc_config_free(config);

default_sym:

    if (sym == NULL) {
        sym = bc_strdup_printf("%s/htdocs", repo_dir);
    }

    if (NULL == getenv("GIT_DIR")) {
        if (0 != access(sym, R_OK)) {
            fprintf(stderr, "error: no previous build found. nothing to "
                "rebuild.\n");
            rv = 3;
            goto cleanup;
        }
        char *build_dir = realpath(sym, NULL);
        if (build_dir == NULL) {
            fprintf(stderr, "error: failed to get the hash of last built "
                "commit.\n");
            rv = 3;
            goto cleanup;
        }
        char **pieces = bc_str_split(basename(build_dir), '-', 2);
        free(build_dir);
        if (bc_strv_length(pieces) != 2) {
            fprintf(stderr, "error: failed to parse the hash of last built "
                "commit.\n");
            bc_strv_free(pieces);
            rv = 3;
            goto cleanup;
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
        goto cleanup;
    }

    char dir[] = "/tmp/blogc_XXXXXX";
    if (NULL == mkdtemp(dir)) {
        rv = 3;
        goto cleanup;
    }
    tmpdir = dir;

    char *git_archive_cmd = bc_strdup_printf(
        "git archive \"%s\" | tar -x -C \"%s\" -f -", master, tmpdir);
    if (0 != system(git_archive_cmd)) {
        fprintf(stderr, "error: failed to extract git content to temporary "
            "directory: %s\n", tmpdir);
        rv = 3;
        free(git_archive_cmd);
        goto cleanup;
    }
    free(git_archive_cmd);

    if (0 != chdir(tmpdir)) {
        fprintf(stderr, "error: failed to chdir (%s): %s\n", tmpdir,
            strerror(errno));
        rv = 3;
        goto cleanup;
    }

    char *buildsd = bgr_settings_get_builds_dir();
    if (buildsd == NULL) {
        fprintf(stderr, "error: failed to find builds directory path\n");
        rv = 3;
        goto cleanup;
    }

    unsigned long epoch = time(NULL);
    output_dir = bc_strdup_printf("%s/%s-%lu", buildsd, master, epoch);
    free(buildsd);

    if (0 == access(output_dir, F_OK)) {
        char *tmp = output_dir;
        output_dir = bc_strdup_printf("%s-", tmp);
        free(tmp);
    }

    // detect if we will run blogc-make, make or nothing, and generate the
    // command.
    char *build_cmd = NULL;
    if (0 == access("blogcfile", F_OK)) {
        int status_bmake = system("blogc-make -v 2> /dev/null > /dev/null");
        if (127 == bc_compat_status_code(status_bmake)) {
            fprintf(stderr, "error: failed to find blogc-make binary\n");
            rv = 3;
            goto cleanup;
        }
        build_cmd = bc_strdup_printf("OUTPUT_DIR=\"%s\" blogc-make -V all",
            output_dir);
    }
    else if ((0 == access("Makefile", F_OK)) || (0 == access("GNUMakefile", F_OK))) {
        const char *make_impl = NULL;

        int status_gmake = system("gmake -f /dev/null 2> /dev/null > /dev/null");
        if (127 != bc_compat_status_code(status_gmake)) {
            make_impl = "gmake";
        }
        else {
            int status_make = system("make -f /dev/null 2> /dev/null > /dev/null");
            if (127 != bc_compat_status_code(status_make)) {
                make_impl = "make";
            }
        }

        if (make_impl == NULL) {
            fprintf(stderr, "error: no 'make' implementation found\n");
            rv = 3;
            goto cleanup;
        }
        build_cmd = bc_strdup_printf(
            "%s -j%d OUTPUT_DIR=\"%s\" BLOGC_GIT_RECEIVER=1", make_impl,
            cpu_count(), output_dir);
    }
    else {
        fprintf(stderr, "warning: no blogcfile or Makefile found. skipping ...\n");
        goto cleanup;
    }

    fprintf(stdout, "running command: %s\n\n", build_cmd);
    fflush(stdout);
    if (0 != system(build_cmd)) {
        fprintf(stderr, "error: failed to build website ...\n");
        rmdir_recursive(output_dir);
        free(build_cmd);
        rv = 3;
        goto cleanup;
    }
    free(build_cmd);

    char *htdocs_sym = realpath(sym, NULL);
    if ((htdocs_sym != NULL) && (0 != unlink(sym))) {
        fprintf(stderr, "error: failed to remove symlink (%s): %s\n", sym,
            strerror(errno));
        rmdir_recursive(output_dir);
        rv = 3;
        goto cleanup2;
    }

    if (0 != symlink(output_dir, sym)) {
        fprintf(stderr, "error: failed to create symlink (%s): %s\n", sym,
            strerror(errno));
        rmdir_recursive(output_dir);
        rv = 3;
        goto cleanup2;
    }

    if (htdocs_sym != NULL)
        rmdir_recursive(htdocs_sym);

cleanup2:
    free(htdocs_sym);

cleanup:
    free(sym);
    free(master);
    free(output_dir);
    rmdir_recursive(tmpdir);
    free(repo_dir);
    return rv;
}
