/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2017 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <libgen.h>
#include <squareball.h>

#include "ctx.h"
#include "error.h"
#include "exec.h"
#include "settings.h"

#define BM_FILE_CHUNK_SIZE 1024


char*
bm_exec_find_binary(const char *argv0, const char *bin, const char *env)
{
#ifdef MAKE_EMBEDDED
    // for embedded blogc-make, if we are looking for blogc, we just return
    // argv0, because the static binary may not be named `blogc`, and we
    // prefer to use our own `blogc` instead of some other version around.
    if (argv0 != NULL && bin != NULL && (0 == strcmp(bin, "blogc"))) {
        return sb_shell_quote(argv0);
    }
#endif

    // first try: env var
    const char *env_bin = getenv(env);
    if (env_bin != NULL) {
        return sb_shell_quote(env_bin);
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
        char *path = sb_strdup(argv0);
        char *dir = sb_strdup(dirname(path));
        free(path);
        char *tmp = sb_strdup_printf("%s/%s", dir, bin);
        free(dir);
        if (0 == access(tmp, X_OK)) {
            char *rv = sb_shell_quote(tmp);
            free(tmp);
            return rv;
        }
        free(tmp);
    }

    // last try: $PATH
    return sb_strdup(bin);
}


int
bm_exec_command(const char *cmd, const char *input, char **output,
    char **error, sb_error_t **err)
{
    if (err == NULL || *err != NULL)
        return 3;

    int fd_in[2];
    if (-1 == pipe(fd_in)) {
        *err = sb_error_new_printf(BLOGC_MAKE_ERROR_EXEC,
            "Failed to create stdin pipe: %s", strerror(errno));
        return 3;
    }

    int fd_out[2];
    if (-1 == pipe(fd_out)) {
        *err = sb_error_new_printf(BLOGC_MAKE_ERROR_EXEC,
            "Failed to create stdout pipe: %s", strerror(errno));
        close(fd_in[0]);
        close(fd_in[1]);
        return 3;
    }

    int fd_err[2];
    if (-1 == pipe(fd_err)) {
        *err = sb_error_new_printf(BLOGC_MAKE_ERROR_EXEC,
            "Failed to create stderr pipe: %s", strerror(errno));
        close(fd_in[0]);
        close(fd_in[1]);
        close(fd_out[0]);
        close(fd_out[1]);
        return 3;
    }

    pid_t pid = fork();
    if (pid == -1) {
        *err = sb_error_new_printf(BLOGC_MAKE_ERROR_EXEC,
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
            *err = sb_error_new_printf(BLOGC_MAKE_ERROR_EXEC,
                "Failed to write to stdin pipe: %s", strerror(errno));
            close(fd_in[1]);
            close(fd_out[0]);
            close(fd_err[0]);
            return 3;
        }
    }

    close(fd_in[1]);

    char buffer[BM_FILE_CHUNK_SIZE];
    ssize_t s;

    sb_string_t *out = NULL;
    while(0 != (s = read(fd_out[0], buffer, BM_FILE_CHUNK_SIZE))) {
        if (s == -1) {
            *err = sb_error_new_printf(BLOGC_MAKE_ERROR_EXEC,
                "Failed to read from stdout pipe: %s", strerror(errno));
            close(fd_out[0]);
            close(fd_err[0]);
            sb_string_free(out, true);
            return 3;
        }
        if (out == NULL) {
            out = sb_string_new();
        }
        sb_string_append_len(out, buffer, s);
    }
    if (out != NULL) {
        *output = sb_string_free(out, false);
    }
    close(fd_out[0]);

    out = NULL;
    while(0 != (s = read(fd_err[0], buffer, BM_FILE_CHUNK_SIZE))) {
        if (s == -1) {
            *err = sb_error_new_printf(BLOGC_MAKE_ERROR_EXEC,
                "Failed to read from stderr pipe: %s", strerror(errno));
            close(fd_err[0]);
            sb_string_free(out, true);
            return 3;
        }
        if (out == NULL)
            out = sb_string_new();
        sb_string_append_len(out, buffer, s);
    }
    if (out != NULL) {
        *error = sb_string_free(out, false);
    }
    close(fd_err[0]);

    int status;
    waitpid(pid, &status, 0);

    return WEXITSTATUS(status);
}


static void
list_variables(const char *key, const char *value, sb_string_t *str)
{
    char *tmp = sb_shell_quote(value);
    sb_string_append_printf(str, " -D %s=%s", key, tmp);
    free(tmp);
}


char*
bm_exec_build_blogc_cmd(const char *blogc_bin, bm_settings_t *settings,
    sb_trie_t *global_variables, sb_trie_t *local_variables, bool listing,
    const char *template, const char *output, bool dev, bool sources_stdin)
{
    sb_string_t *rv = sb_string_new();

    const char *locale = NULL;
    if (settings != NULL) {
        locale = sb_trie_lookup(settings->settings, "locale");
    }
    if (locale != NULL) {
        char *tmp = sb_shell_quote(locale);
        sb_string_append_printf(rv, "LC_ALL=%s ", tmp);
        free(tmp);
    }

    sb_string_append(rv, blogc_bin);

    if (settings != NULL) {
        if (settings->tags != NULL) {
            char *tags = sb_strv_join(settings->tags, " ");
            sb_string_append_printf(rv, " -D MAKE_TAGS='%s'", tags);
            free(tags);
        }

        sb_trie_foreach(settings->global,
            (sb_trie_foreach_func_t) list_variables, rv);
    }

    sb_trie_foreach(global_variables, (sb_trie_foreach_func_t) list_variables, rv);
    sb_trie_foreach(local_variables, (sb_trie_foreach_func_t) list_variables, rv);

    if (dev) {
        sb_string_append(rv, " -D MAKE_ENV_DEV=1 -D MAKE_ENV='dev'");
    }

    if (listing) {
        sb_string_append(rv, " -l");
    }

    if (template != NULL) {
        char *tmp = sb_shell_quote(template);
        sb_string_append_printf(rv, " -t %s", tmp);
        free(tmp);
    }

    if (output != NULL) {
        char *tmp = sb_shell_quote(output);
        sb_string_append_printf(rv, " -o %s", tmp);
        free(tmp);
    }

    if (sources_stdin) {
        sb_string_append(rv, " -i");
    }

    return sb_string_free(rv, false);
}


int
bm_exec_blogc(bm_ctx_t *ctx, sb_trie_t *global_variables, sb_trie_t *local_variables,
    bool listing, bm_filectx_t *template, bm_filectx_t *output, sb_slist_t *sources,
    bool only_first_source)
{
    if (ctx == NULL)
        return 3;

    sb_string_t *input = sb_string_new();
    for (sb_slist_t *l = sources; l != NULL; l = l->next) {
        sb_string_append_printf(input, "%s\n", ((bm_filectx_t*) l->data)->path);
        if (only_first_source)
            break;
    }

    char *cmd = bm_exec_build_blogc_cmd(ctx->blogc, ctx->settings, global_variables,
        local_variables, listing, template->path, output->path, ctx->dev,
        input->len > 0);

    if (ctx->verbose)
        printf("%s\n", cmd);
    else
        printf("  BLOGC    %s\n", output->short_path);
    fflush(stdout);

    char *out = NULL;
    char *err = NULL;
    sb_error_t *error = NULL;

    int rv = bm_exec_command(cmd, input->str, &out, &err, &error);

    if (err != NULL) {
        fprintf(stderr, "%s", err);
    }

    if (error != NULL) {
        bm_error_print(error);
        free(cmd);
        free(out);
        free(err);
        sb_string_free(input, true);
        sb_error_free(error);
        return 3;
    }

    if (rv != 0) {
        if (ctx->verbose) {
            fprintf(stderr,
                "blogc-make: error: Failed to execute command.\n"
                "\n"
                "STATUS CODE: %d\n", rv);
            if (input->len > 0) {
                fprintf(stderr, "\nSTDIN:\n"
                    "----------------------------->8-----------------------------\n"
                    "%s\n"
                    "----------------------------->8-----------------------------\n",
                    sb_str_strip(input->str));
            }
            if (out != NULL) {
                fprintf(stderr, "\nSTDOUT:\n"
                    "----------------------------->8-----------------------------\n"
                    "%s\n"
                    "----------------------------->8-----------------------------\n",
                    sb_str_strip(out));
            }
        }
        else {
            fprintf(stderr,
                "blogc-make: error: Failed to execute command (%d)", rv);
            if (err != NULL) {
                fprintf(stderr, ":\n%s", sb_str_strip(err));
            }
            fprintf(stderr, "\n");
        }
    }

    sb_string_free(input, true);
    free(cmd);
    free(out);
    free(err);

    return rv == 127 ? 3 : rv;
}


int
bm_exec_blogc_runserver(bm_ctx_t *ctx, const char *host, const char *port,
    const char *threads)
{
    if (ctx == NULL)
        return 3;

    sb_string_t *cmd = sb_string_new();

    sb_string_append(cmd, ctx->blogc_runserver);

    if (host != NULL) {
        char *tmp = sb_shell_quote(host);
        sb_string_append_printf(cmd, " -t %s", tmp);
        free(tmp);
    }

    if (port != NULL) {
        char *tmp = sb_shell_quote(port);
        sb_string_append_printf(cmd, " -p %s", tmp);
        free(tmp);
    }

    if (threads != NULL) {
        char *tmp = sb_shell_quote(threads);
        sb_string_append_printf(cmd, " -m %s", tmp);
        free(tmp);
    }

    char *tmp = sb_shell_quote(ctx->output_dir);
    sb_string_append_printf(cmd, " %s", tmp);
    free(tmp);

    if (ctx->verbose)
        printf("%s\n\n", cmd->str);
    else
        printf("\n");
    fflush(stdout);

    // we don't need pipes to run blogc-runserver, because it is "interactive"
    int status = system(cmd->str);
    int rv = WEXITSTATUS(status);
    sb_string_free(cmd, true);

    if (rv != 0 && rv != 130) {
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
