/*
 * blogc: A blog compiler.
 * Copyright (C) 2015-2016 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include "../common/error.h"
#include "../common/file.h"
#include "../common/utils.h"
#include "ctx.h"
#include "exec.h"
#include "settings.h"


int
bm_exec_command(const char *cmd, const char *input, char **output,
    char **error, bc_error_t **err)
{
    if (err == NULL || *err != NULL)
        return 3;

    int fd_in[2];
    if (-1 == pipe(fd_in)) {
        *err = bc_error_new_printf(BLOGC_MAKE_ERROR_EXEC,
            "Failed to create stdin pipe: %s", strerror(errno));
        return 3;
    }

    int fd_out[2];
    if (-1 == pipe(fd_out)) {
        *err = bc_error_new_printf(BLOGC_MAKE_ERROR_EXEC,
            "Failed to create stdout pipe: %s", strerror(errno));
        close(fd_in[0]);
        close(fd_in[1]);
        return 3;
    }

    int fd_err[2];
    if (-1 == pipe(fd_err)) {
        *err = bc_error_new_printf(BLOGC_MAKE_ERROR_EXEC,
            "Failed to create stderr pipe: %s", strerror(errno));
        close(fd_in[0]);
        close(fd_in[1]);
        close(fd_out[0]);
        close(fd_out[1]);
        return 3;
    }

    pid_t pid = fork();
    if (pid == -1) {
        *err = bc_error_new_printf(BLOGC_MAKE_ERROR_EXEC,
            "Failed to fork: %s", strerror(errno));
        close(fd_in[0]);
        close(fd_in[1]);
        close(fd_out[0]);
        close(fd_out[1]);
        close(fd_err[0]);
        close(fd_err[1]);
        return 3;
    }

    // child
    if (pid == 0) {
        close(fd_in[1]);
        close(fd_out[0]);
        close(fd_err[0]);

        dup2(fd_in[0], STDIN_FILENO);
        dup2(fd_out[1], STDOUT_FILENO);
        dup2(fd_err[1], STDERR_FILENO);

        char *const argv[] = {
            "/bin/sh",
            "-c",
            (char*) cmd,
            NULL,
        };

        execv(argv[0], argv);

        exit(1);
    }

    // parent
    close(fd_in[0]);
    close(fd_out[1]);
    close(fd_err[1]);

    if (input != NULL) {
        if (-1 == write(fd_in[1], input, strlen(input))) {
            *err = bc_error_new_printf(BLOGC_MAKE_ERROR_EXEC,
                "Failed to write to stdin pipe: %s", strerror(errno));
            close(fd_in[1]);
            close(fd_out[0]);
            close(fd_err[0]);
            return 3;
        }
    }

    close(fd_in[1]);

    char buffer[BC_FILE_CHUNK_SIZE];
    ssize_t s;

    bc_string_t *out = NULL;
    while(0 != (s = read(fd_out[0], buffer, BC_FILE_CHUNK_SIZE))) {
        if (s == -1) {
            *err = bc_error_new_printf(BLOGC_MAKE_ERROR_EXEC,
                "Failed to read from stdout pipe: %s", strerror(errno));
            close(fd_out[0]);
            close(fd_err[0]);
            bc_string_free(out, true);
            return 3;
        }
        if (out == NULL) {
            out = bc_string_new();
        }
        bc_string_append_len(out, buffer, s);
    }
    if (out != NULL) {
        *output = bc_string_free(out, false);
    }
    close(fd_out[0]);

    out = NULL;
    while(0 != (s = read(fd_err[0], buffer, BC_FILE_CHUNK_SIZE))) {
        if (s == -1) {
            *err = bc_error_new_printf(BLOGC_MAKE_ERROR_EXEC,
                "Failed to read from stderr pipe: %s", strerror(errno));
            close(fd_err[0]);
            bc_string_free(out, true);
            return 3;
        }
        if (out == NULL)
            out = bc_string_new();
        bc_string_append_len(out, buffer, s);
    }
    if (out != NULL) {
        *error = bc_string_free(out, false);
    }
    close(fd_err[0]);

    int status;
    waitpid(pid, &status, 0);

    return WEXITSTATUS(status);
}


static void
list_variables(const char *key, const char *value, bc_string_t *str)
{
    char *tmp = bc_shell_quote(value);
    bc_string_append_printf(str, " -D %s=%s", key, tmp);
    free(tmp);
}


char*
bm_exec_build_blogc_cmd(bm_settings_t *settings, bc_trie_t *variables,
    bool listing, const char *template, const char *output, bool sources_stdin)
{
    bc_string_t *rv = bc_string_new();

    const char *locale = NULL;
    if (settings != NULL) {
        locale = bc_trie_lookup(settings->settings, "locale");
    }
    if (locale != NULL) {
        char *tmp = bc_shell_quote(locale);
        bc_string_append_printf(rv, "LC_ALL=%s ", tmp);
        free(tmp);
    }

    // use blogc binary from environment, if provided
    const char *blogc_bin = getenv("BLOGC");
    if (blogc_bin != NULL) {
        char *tmp = bc_shell_quote(blogc_bin);
        bc_string_append(rv, tmp);
        free(tmp);
    }
    else {
        bc_string_append(rv, "blogc");
    }

    if (settings != NULL) {
        bc_trie_foreach(settings->env,
            (bc_trie_foreach_func_t) list_variables, rv);
    }

    bc_trie_foreach(variables, (bc_trie_foreach_func_t) list_variables, rv);

    if (listing) {
        bc_string_append(rv, " -l");
    }

    if (template != NULL) {
        char *tmp = bc_shell_quote(template);
        bc_string_append_printf(rv, " -t %s", tmp);
        free(tmp);
    }

    if (output != NULL) {
        char *tmp = bc_shell_quote(output);
        bc_string_append_printf(rv, " -o %s", tmp);
        free(tmp);
    }

    if (sources_stdin) {
        bc_string_append(rv, " -i");
    }

    return bc_string_free(rv, false);
}


int
bm_exec_blogc(bm_settings_t *settings, bc_trie_t *variables, bool listing,
    bm_filectx_t *template, bm_filectx_t *output, bc_slist_t *sources,
    bool verbose, bool only_first_source)
{
    bc_string_t *input = bc_string_new();
    for (bc_slist_t *l = sources; l != NULL; l = l->next) {
        bc_string_append_printf(input, "%s\n", ((bm_filectx_t*) l->data)->path);
        if (only_first_source)
            break;
    }

    char *cmd = bm_exec_build_blogc_cmd(settings, variables, listing,
        template->path, output->path, input->len > 0);

    if (verbose)
        printf("%s\n", cmd);
    else
        printf("  BLOGC    %s\n", output->short_path);
    fflush(stdout);

    char *out = NULL;
    char *err = NULL;
    bc_error_t *error = NULL;

    int rv = bm_exec_command(cmd, input->str, &out, &err, &error);

    if (error != NULL) {
        bc_error_print(error, "blogc-make");
        free(cmd);
        free(out);
        free(err);
        bc_string_free(input, true);
        bc_error_free(error);
        return 3;
    }

    if (rv != 0) {
        if (verbose) {
            fprintf(stderr,
                "blogc-make: error: Failed to execute command.\n"
                "\n"
                "STATUS CODE: %d\n", rv);
            if (input->len > 0) {
                fprintf(stderr, "\nSTDIN:\n"
                    "----------------------------->8-----------------------------\n"
                    "%s\n"
                    "----------------------------->8-----------------------------\n",
                    bc_str_strip(input->str));
            }
            if (out != NULL) {
                fprintf(stderr, "\nSTDOUT:\n"
                    "----------------------------->8-----------------------------\n"
                    "%s\n"
                    "----------------------------->8-----------------------------\n",
                    bc_str_strip(out));
            }
            if (err != NULL) {
                fprintf(stderr, "\nSTDERR:\n"
                    "----------------------------->8-----------------------------\n"
                    "%s\n"
                    "----------------------------->8-----------------------------\n",
                    bc_str_strip(err));
            }
        }
        else {
            fprintf(stderr,
                "blogc-make: error: Failed to execute command, returned "
                "status code: %d\n", rv);
        }
    }

    bc_string_free(input, true);
    free(cmd);
    free(out);
    free(err);

    return rv == 127 ? 3 : rv;
}


int
bm_exec_blogc_runserver(const char *output_dir, const char *host,
    const char *port, const char *threads, bool verbose)
{
    bc_string_t *cmd = bc_string_new();

    // use blogc-runserver binary from environment, if provided
    const char *blogc_runserver = getenv("BLOGC_RUNSERVER");
    if (blogc_runserver != NULL) {
        char *tmp = bc_shell_quote(blogc_runserver);
        bc_string_append(cmd, tmp);
        free(tmp);
    }
    else {
        bc_string_append(cmd, "blogc-runserver");
    }

    if (host != NULL) {
        char *tmp = bc_shell_quote(host);
        bc_string_append_printf(cmd, " -t %s", tmp);
        free(tmp);
    }

    if (port != NULL) {
        char *tmp = bc_shell_quote(port);
        bc_string_append_printf(cmd, " -p %s", tmp);
        free(tmp);
    }

    if (threads != NULL) {
        char *tmp = bc_shell_quote(threads);
        bc_string_append_printf(cmd, " -m %s", tmp);
        free(tmp);
    }

    char *tmp = bc_shell_quote(output_dir);
    bc_string_append_printf(cmd, " %s", tmp);
    free(tmp);

    if (verbose)
        printf("%s\n", cmd->str);
    else
        printf("\n");
    fflush(stdout);

    // we don't need pipes to run blogc-runserver, because it is "interactive"
    int status = system(cmd->str);
    int rv = WEXITSTATUS(status);
    bc_string_free(cmd, true);

    if (rv != 0) {
        if (rv == 127) {
            fprintf(stderr,
                "blogc-make: error: blogc-runserver command not found. Maybe "
                "it is not installed?\n");
            rv = 3;  // blogc-make exists, so we should not return 127
        }
        else {
            fprintf(stderr,
                "blogc-make: error: Failed to execute command, returned "
                "status code: %d\n", rv);
        }
    }

    return rv;
}
