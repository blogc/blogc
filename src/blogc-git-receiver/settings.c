/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2019 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#include <stdio.h>
#include <libgen.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../common/utils.h"
#include "../common/config-parser.h"
#include "../common/error.h"
#include "../common/file.h"
#include "settings.h"


const char*
bgr_settings_get_base_dir(void)
{
    char *rv = getenv("BLOGC_GIT_RECEIVER_BASE_DIR");
    if (rv != NULL) {
        return rv;
    }
    return getenv("HOME");
}


char*
bgr_settings_get_builds_dir(void)
{
    char *rv = getenv("BLOGC_GIT_RECEIVER_BUILDS_DIR");
    if (rv != NULL) {
        return bc_strdup(rv);
    }
    return bc_strdup_printf("%s/builds", bgr_settings_get_base_dir());
}


char*
bgr_settings_get_section(bc_config_t *config, const char *repo_path)
{
    const char *bd = bgr_settings_get_base_dir();
    if (bd == NULL) {
        return NULL;
    }
    char *rv = NULL;
    char** sections = bc_config_list_sections(config);
    for (size_t i = 0; sections[i] != NULL; i++) {
        if (bc_str_starts_with(sections[i], "repo:")) {
            char *tmp_repo = bc_strdup_printf("%s/repos/%s", bd, sections[i] + 5);
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


bc_config_t*
bgr_settings_parse(void)
{
    const char *bd = bgr_settings_get_base_dir();
    if (bd == NULL) {
        return NULL;
    }
    char *config_file = bc_strdup_printf("%s/blogc-git-receiver.ini", bd);
    if ((0 != access(config_file, F_OK))) {
        free(config_file);
        return NULL;
    }

    size_t len;
    bc_error_t *err = NULL;
    char* config_content = bc_file_get_contents(config_file, true, &len, &err);
    if (err != NULL) {
        fprintf(stderr, "warning: failed to read configuration file (%s): %s\n",
            config_file, err->msg);
        bc_error_free(err);
        free(config_file);
        free(config_content);
        return NULL;
    }

    bc_config_t *config = bc_config_parse(config_content, len, NULL, &err);
    free(config_content);
    if (err != NULL) {
        fprintf(stderr, "warning: failed to parse configuration file (%s): %s\n",
            config_file, err->msg);
        bc_error_free(err);
        free(config_file);
        return NULL;
    }
    free(config_file);

    return config;
}
