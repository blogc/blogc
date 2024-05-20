// SPDX-FileCopyrightText: 2014-2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
// SPDX-License-Identifier: BSD-3-Clause

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#ifdef HAVE_SYSEXITS_H
#include <sysexits.h>
#else
#define EX_CONFIG 78
#endif /* HAVE_SYSEXITS_H */

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <libgen.h>
#include "../common/compat.h"
#include "../common/error.h"
#include "../common/file.h"
#include "../common/utils.h"
#include "ctx.h"
#include "exec.h"
#include "settings.h"


char*
bm_exec_find_binary(const char *argv0, const char *bin, const char *env)
{
#ifdef MAKE_EMBEDDED
    // for embedded blogc-make, if we are looking for blogc, we just return
    // argv0, because the static binary may not be named `blogc`, and we
    // prefer to use our own `blogc` instead of some other version around.
    if (argv0 != NULL && bin != NULL && (0 == strcmp(bin, "blogc"))) {
        return bc_shell_quote(argv0);
    }
#endif

    // first try: env var
    const char *env_bin = getenv(env);
    if (env_bin != NULL) {
        return bc_shell_quote(env_bin);
    }

    // second try: same dir as current exec
    // we rely on some assumptions here:
    //
    // - if binary is called without a directory, that means location will
    //   be resolved from $PATH, we don't care about doing a dir lookup.
    // - we *never* call chdir anywhere in the code, so we can assume
    //   that relative paths will work as expected.
    // - windows path sep is not supported
    if (argv0 != NULL && (NULL != strchr(argv0, '/'))) {
        char *path = bc_strdup(argv0);
        char *dir = bc_strdup(dirname(path));
        free(path);
        char *tmp = bc_strdup_printf("%s/%s", dir, bin);
        free(dir);
        if (0 == access(tmp, X_OK)) {
            char *rv = bc_shell_quote(tmp);
            free(tmp);
            return rv;
        }
        free(tmp);
    }

    // last try: $PATH
    return bc_strdup(bin);
}


int
bm_exec_command(const char *cmd, const char *input, char **output,
    char **error, bc_error_t **err)
{
    if (err == NULL || *err != NULL)
        return 1;

    int fd_in[2];
    if (-1 == pipe(fd_in)) {
        *err = bc_error_new_printf(BLOGC_MAKE_ERROR_EXEC,
            "Failed to create stdin pipe: %s", strerror(errno));
        return 1;
    }

    int fd_out[2];
    if (-1 == pipe(fd_out)) {
        *err = bc_error_new_printf(BLOGC_MAKE_ERROR_EXEC,
            "Failed to create stdout pipe: %s", strerror(errno));
        close(fd_in[0]);
        close(fd_in[1]);
        return 1;
    }

    int fd_err[2];
    if (-1 == pipe(fd_err)) {
        *err = bc_error_new_printf(BLOGC_MAKE_ERROR_EXEC,
            "Failed to create stderr pipe: %s", strerror(errno));
        close(fd_in[0]);
        close(fd_in[1]);
        close(fd_out[0]);
        close(fd_out[1]);
        return 1;
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
        return 1;
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
            return 1;
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
            return 1;
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
            return 1;
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

    return bc_compat_status_code(status);
}


static void
list_variables(const char *key, const char *value, bc_string_t *str)
{
    char *tmp = bc_shell_quote(value);
    bc_string_append_printf(str, " -D %s=%s", key, tmp);
    free(tmp);
}


char*
bm_exec_build_blogc_cmd(const char *blogc_bin, bm_settings_t *settings,
    bc_trie_t *global_variables, bc_trie_t *local_variables, const char *print,
    bool listing, const char *listing_entry, const char *template,
    const char *output, bool dev, bool sources_stdin)
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

    bc_string_append(rv, blogc_bin);

    if (settings != NULL) {
        if (settings->tags != NULL) {
            char *tags = bc_strv_join(settings->tags, " ");
            bc_string_append_printf(rv, " -D MAKE_TAGS='%s'", tags);
            free(tags);
        }

        bc_trie_foreach(settings->global,
            (bc_trie_foreach_func_t) list_variables, rv);
    }

    bc_trie_foreach(global_variables, (bc_trie_foreach_func_t) list_variables, rv);
    bc_trie_foreach(local_variables, (bc_trie_foreach_func_t) list_variables, rv);

    if (dev) {
        bc_string_append(rv, " -D MAKE_ENV_DEV=1 -D MAKE_ENV='dev'");
    }

    if (print != NULL) {
        bc_string_append_printf(rv, " -p %s", print);
    }

    if (listing) {
        bc_string_append(rv, " -l");
        if (listing_entry != NULL) {
            char *tmp = bc_shell_quote(listing_entry);
            bc_string_append_printf(rv, " -e %s", tmp);
            free(tmp);
        }
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
bm_exec_blogc(bm_ctx_t *ctx, bc_trie_t *global_variables, bc_trie_t *local_variables,
    bool listing, bm_filectx_t *listing_entry, bm_filectx_t *template,
    bm_filectx_t *output, bc_slist_t *sources, bool only_first_source)
{
    if (ctx == NULL)
        return 1;

    bc_string_t *input = bc_string_new();
    for (bc_slist_t *l = sources; l != NULL; l = l->next) {
        bc_string_append_printf(input, "%s\n", ((bm_filectx_t*) l->data)->path);
        if (only_first_source)
            break;
    }

    char *cmd = bm_exec_build_blogc_cmd(ctx->blogc, ctx->settings, global_variables,
        local_variables, NULL, listing, listing_entry == NULL ? NULL : listing_entry->path,
        template->path, output->path, ctx->dev, input->len > 0);

    if (ctx->verbose)
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
        return 1;
    }

    if (rv != 0 && ctx->verbose) {
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
        fprintf(stderr, "\n");
    }
    else if (err != NULL) {
        fprintf(stderr, "%s\n", err);
    }

    bc_string_free(input, true);
    free(cmd);
    free(out);
    free(err);

    return rv == 127 ? 1 : rv;
}


char*
bm_exec_blogc_get_variable(bm_ctx_t *ctx, bc_trie_t *global_variables,
    bc_trie_t *local_variables, const char *variable, bool listing,
    bc_slist_t *sources, bool only_first_source)
{
    if (ctx == NULL)
        return NULL;

    bc_string_t *input = bc_string_new();
    for (bc_slist_t *l = sources; l != NULL; l = l->next) {
        bc_string_append_printf(input, "%s\n", ((bm_filectx_t*) l->data)->path);
        if (only_first_source)
            break;
    }

    char *cmd = bm_exec_build_blogc_cmd(ctx->blogc, ctx->settings, global_variables,
        local_variables, variable, listing, NULL, NULL, NULL, ctx->dev, input->len > 0);

    if (ctx->verbose)
        printf("%s\n", cmd);
    fflush(stdout);

    char *out = NULL;
    char *err = NULL;
    bc_error_t *error = NULL;

    int rv = bm_exec_command(cmd, input->str, &out, &err, &error);

    if (error != NULL) {
        bc_error_print(error, "blogc-make");
        bc_error_free(error);
        bc_string_free(input, true);
        free(cmd);
        free(out);
        free(err);
        return NULL;
    }

    if (rv != 0) {
        if (rv != EX_CONFIG)
            fprintf(stderr, "blogc-make: error: %s\n", bc_str_strip(err));
        bc_string_free(input, true);
        free(cmd);
        free(out);
        free(err);
        return NULL;
    }

    char *val = NULL;
    if (out != NULL)
        val = bc_strndup(out, strlen(out) - 1);

    bc_string_free(input, true);
    free(cmd);
    free(out);
    free(err);

    return val;
}


int
bm_exec_blogc_runserver(bm_ctx_t *ctx, const char *host, const char *port,
    const char *threads)
{
    if (ctx == NULL)
        return 1;

    bc_string_t *cmd = bc_string_new();

    bc_string_append(cmd, ctx->blogc_runserver);

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

    char *tmp = bc_shell_quote(ctx->output_dir);
    bc_string_append_printf(cmd, " %s", tmp);
    free(tmp);

    if (ctx->verbose)
        printf("%s\n\n", cmd->str);
    else
        printf("\n");
    fflush(stdout);

    // we don't need pipes to run blogc-runserver, because it is "interactive"
    int status = system(cmd->str);
    int rv = bc_compat_status_code(status);
    bc_string_free(cmd, true);

    if (rv != 0 && rv != 130) {
        if (rv == 127) {
            fprintf(stderr,
                "blogc-make: error: blogc-runserver command not found. Maybe "
                "it is not installed?\n");
            rv = 1;  // blogc-make exists, so we should not return 127
        }
        else {
            fprintf(stderr,
                "blogc-make: error: Failed to execute command, returned "
                "status code: %d\n", rv);
        }
    }

    return rv;
}
