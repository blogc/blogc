/*
 * blogc: A blog compiler.
 * Copyright (C) 2015-2016 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifndef _HTTPD_UTILS_H
#define _HTTPD_UTILS_H

#define READLINE_BUFFER_SIZE 256

char* br_readline(int socket);
int br_hextoi(const char c);
char* br_urldecode(const char *str);
const char* br_get_extension(const char *filename);

#endif /* _HTTPD_UTILS_H */
