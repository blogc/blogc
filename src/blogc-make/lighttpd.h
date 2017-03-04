/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2017 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifndef _MAKE_LIGHTTPD_H
#define _MAKE_LIGHTTPD_H

char* bm_lighttpd_deploy(const char *output_dir, const char *host,
    const char *port);
void bm_lighttpd_destroy(const char *fname);

#endif /* _MAKE_LIGHTTPD_H */
