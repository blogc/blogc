/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2019 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifndef _SHELL_H
#define _SHELL_H

char* bgr_shell_get_repo(const char *command);
int bgr_shell(int argc, char *argv[]);

#endif /* _SHELL_H */
