/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2018 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifndef _MIME_H
#define _MIME_H

const char* br_mime_guess_content_type(const char *filename);
char* br_mime_guess_index(const char *path);

#endif /* _MIME_H */
