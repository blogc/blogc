// SPDX-FileCopyrightText: 2014-2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#define READLINE_BUFFER_SIZE 2048

char* br_readline(int socket);
int br_hextoi(const char c);
char* br_urldecode(const char *str);
const char* br_get_extension(const char *filename);
