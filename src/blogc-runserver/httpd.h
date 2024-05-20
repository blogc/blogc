// SPDX-FileCopyrightText: 2014-2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

int br_httpd_run(const char *host, const char *port, const char *docroot,
    size_t max_threads);
