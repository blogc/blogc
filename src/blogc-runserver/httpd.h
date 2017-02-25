/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2017 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifndef _HTTPD_H
#define _HTTPD_H

int br_httpd_run(const char *host, unsigned short port, const char *docroot,
    size_t max_threads);

#endif /* _HTTPD_H */
