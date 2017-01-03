/*
 * blogc: A blog compiler.
 * Copyright (C) 2016 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <libgen.h>
#include <errno.h>
#include "../common/file.h"
#include "../common/utils.h"
#include "exec-native.h"
#include "ctx.h"


int
bm_exec_native_cp(bm_filectx_t *source, bm_filectx_t *dest, bool verbose)
{
    if (verbose)
        printf("Copying '%s' to '%s'\n", source->path, dest->path);
    else
        printf("  COPY     %s\n", dest->short_path);
    fflush(stdout);

    char *fname = bc_strdup(dest->path);
    for (char *tmp = fname; *tmp != '\0'; tmp++) {
        if (*tmp != '/' && *tmp != '\\')
            continue;
        char bkp = *tmp;
        *tmp = '\0';
        if ((strlen(fname) > 0) &&
            (-1 == mkdir(fname, 0777)) &&
            (errno != EEXIST))
        {
            fprintf(stderr, "blogc-make: error: failed to create output "
                "directory (%s): %s\n", fname, strerror(errno));
            free(fname);
            exit(2);
        }
        *tmp = bkp;
    }
    free(fname);

    int fd_from = open(source->path, O_RDONLY);
    if (fd_from < 0) {
        fprintf(stderr, "blogc-make: error: failed to open source file to copy "
            " (%s): %s\n", source->path, strerror(errno));
        return 3;
    }

    int fd_to = open(dest->path, O_WRONLY | O_CREAT, 0666);
    if (fd_to < 0) {
        fprintf(stderr, "blogc-make: error: failed to open destination file to "
            "copy (%s): %s\n", dest->path, strerror(errno));
        close(fd_from);
        return 3;
    }

    ssize_t nread;
    char buffer[BC_FILE_CHUNK_SIZE];
    while (0 < (nread = read(fd_from, buffer, BC_FILE_CHUNK_SIZE))) {
        char *out_ptr = buffer;
        do {
            ssize_t nwritten = write(fd_to, out_ptr, nread);
            if (nwritten == -1) {
                fprintf(stderr, "blogc-make: error: failed to write to "
                    "destination file (%s): %s\n", dest->path, strerror(errno));
                close(fd_from);
                close(fd_to);
                return 3;
            }
            nread -= nwritten;
            out_ptr += nwritten;
        } while (nread > 0);
    }

    return 0;
}


int
bm_exec_native_rm(const char *output_dir, bm_filectx_t *dest, bool verbose)
{
    if (verbose)
        printf("Removing file '%s'\n", dest->path);
    else
        printf("  CLEAN    %s\n", dest->short_path);
    fflush(stdout);

    if (0 != unlink(dest->path)) {
        fprintf(stderr, "blogc-make: error: failed to remove file (%s): %s\n",
            dest->path, strerror(errno));
        return 3;
    }

    int rv = 0;

    char *short_path = bc_strdup(dest->short_path);
    char *path = bc_strdup(dest->path);

    char *dir_short = dirname(short_path);
    char *dir = dirname(path);

    while ((0 != strcmp(dir_short, ".")) && (0 != strcmp(dir, output_dir))) {
        DIR *d = opendir(dir);
        if (d == NULL) {
            fprintf(stderr, "error: failed to open directory (%s): %s\n",
                dir, strerror(errno));
            rv = 3;
            break;
        }

        struct dirent *e;
        size_t count = 0;
        while (NULL != (e = readdir(d))) {
            if ((0 == strcmp(e->d_name, ".")) || (0 == strcmp(e->d_name, "..")))
                continue;
            count++;
            break;
        }

        if (0 != closedir(d)) {
            fprintf(stderr, "error: failed to close directory (%s): %s\n",
                dir, strerror(errno));
            rv = 3;
            break;
        }

        if (count == 0) {
            if (verbose) {
                printf("Removing directory '%s'\n", dir);
                fflush(stdout);
            }
            if (0 != rmdir(dir)) {
                fprintf(stderr, "error: failed to remove directory(%s): %s\n",
                    dir, strerror(errno));
                rv = 3;
                break;
            }
        }

        dir_short = dirname(dir_short);
        dir = dirname(dir);
    }

    // try to remove output dir
    // this is done on a best-effort basis, if we can't remove it, it probably
    // have files, so we don't really want to remove it.
    rmdir(output_dir);

    free(short_path);
    free(path);

    return rv;
}
