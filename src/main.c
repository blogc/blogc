/*
 * blogc: A blog compiler.
 * Copyright (C) 2015 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file COPYING.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>

#include "source-grammar.h"


int
main(int argc, char **argv)
{
    blogc_source_t *t = blogc_source_parse(
        "\n  \nBOLA: guda\n\t\n\n\n\n"
        "CHUNDA: asd\n"
        "----\n"
        "{% block single_source %}\nbola\n\nzas\n");
    printf("%s\n", t->content);
    printf("Hello, World!\n");
    return 0;
}
