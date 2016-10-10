/*
 * blogc: A blog compiler.
 * Copyright (C) 2015-2016 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#include <errno.h>
#include <stdio.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "../common/config-parser.h"
#include "../common/error.h"
#include "../common/file.h"


char*
bgr_post_receive_get_config_section(bc_config_t *config, const char *repo_path,
    const char *home)
{
    char *rv = NULL;
    char** sections = bc_config_list_sections(config);
    for (size_t i = 0; sections[i] != NULL; i++) {
        if (bc_str_starts_with(sections[i], "repo:")) {
            char *tmp_repo = bc_strdup_printf("%s/repos/%s", home, sections[i] + 5);
            char *real_tmp_repo = realpath(tmp_repo, NULL);  // maybe not needed
            free(tmp_repo);
            if (real_tmp_repo == NULL)
                continue;
            if (0 == strcmp(real_tmp_repo, repo_path)) {
                rv = bc_strdup(sections[i]);
                free(real_tmp_repo);
                break;
            }
            free(real_tmp_repo);
        }
    }
    bc_strv_free(sections);
    return rv;
}


int
bgr_post_receive_hook(int argc, char *argv[])
{
    char *mirror = NULL;

    // local repository settings should take precedence, so if the repo have
    // the 'mirror' remote, just push to it.
    // this will be removed at some point, but will be kept for compatibility
    // with old setups.
    if ((0 == system("git config --local remote.mirror.pushurl &> /dev/null")) ||
        (0 == system("git config --local remote.mirror.url &> /dev/null")))
    {
        mirror = bc_strdup("mirror");
        goto push;
    }

    char buffer[4096];
    if (NULL == getcwd(buffer, sizeof(buffer))) {
        fprintf(stderr, "warning: failed to get repository remote path, "
            "mirroring disabled: %s\n", strerror(errno));
        return 0;
    }

    char *repo_path = realpath(buffer, NULL);
    if (repo_path == NULL) {
        fprintf(stderr, "warning: failed to find remote repository directory, "
            "mirroring disabled: %s\n", strerror(errno));
        return 0;
    }

    char *home = getenv("HOME");
    if (home == NULL) {
        fprintf(stderr, "warning: failed to find user home path, "
            "mirroring disabled\n");
        free(repo_path);
        return 0;
    }

    char *config_file = bc_strdup_printf("%s/blogc-git-receiver.ini", home);
    if ((0 != access(config_file, F_OK))) {
        fprintf(stderr, "warning: repository mirroring disabled\n");
        free(repo_path);
        free(config_file);
        return 0;
    }

    size_t len;
    bc_error_t *err = NULL;
    char* config_content = bc_file_get_contents(config_file, true, &len, &err);
    if (err != NULL) {
        fprintf(stderr, "warning: failed to read configuration file (%s), "
            "mirroring disabled: %s\n", config_file, err->msg);
        bc_error_free(err);
        free(repo_path);
        free(config_file);
        free(config_content);
        return 0;
    }

    bc_config_t *config = bc_config_parse(config_content, len, &err);
    free(config_content);
    if (err != NULL) {
        fprintf(stderr, "warning: failed to parse configuration file (%s), "
            "mirroring disabled: %s\n", config_file, err->msg);
        bc_error_free(err);
        free(repo_path);
        free(config_file);
        return 0;
    }
    free(config_file);

    char *config_section = bgr_post_receive_get_config_section(config, repo_path,
        home);
    free(repo_path);
    if (config_section == NULL) {
        fprintf(stderr, "warning: repository mirroring disabled\n");
        bc_config_free(config);
        return 0;
    }

    mirror = bc_strdup(bc_config_get(config, config_section, "mirror"));
    free(config_section);
    bc_config_free(config);

    if (mirror == NULL) {
        fprintf(stderr, "warning: repository mirroring disabled\n");
        return 0;
    }

push:

    {
        char *git_cmd = bc_strdup_printf("git push --mirror %s", mirror);
        if (0 != system(git_cmd))
            fprintf(stderr, "warning: failed push to git mirror\n");
        free(git_cmd);
    }

    free(mirror);

    return 0;
}
