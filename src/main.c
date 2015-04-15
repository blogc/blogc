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

#include "template-grammar.h"


int
main(int argc, char **argv)
{
    b_slist_t *t = blogc_template_parse(
        "<html>{{ BOLA }}</html>\n"
        "{% block single_source %}\n");
    printf("%s\n", (char*) ((blogc_template_stmt_t*)t->next->next->next->data)->value);
    printf("Hello, World!\n");
    return 0;
}
