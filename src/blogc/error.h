/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2017 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifndef _ERROR_H
#define _ERROR_H

typedef enum {
    BLOGC_ERROR_SOURCE_PARSER = 100,
    BLOGC_ERROR_TEMPLATE_PARSER,
    BLOGC_ERROR_LOADER,
    BLOGC_WARNING_DATETIME_PARSER,
} blogc_error_type_t;

void blogc_error_print(sb_error_t *err);

#endif /* _ERROR_H */
