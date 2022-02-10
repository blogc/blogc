/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2020 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "../common/utils.h"
#include "pre-receive-parser.h"

typedef enum {
    START_OLD = 1,
    OLD,
    START_NEW,
    NEW,
    START_REF,
    REF
} input_state_t;


bc_trie_t*
bgr_pre_receive_parse(const char *input, size_t input_len)
{
    input_state_t state = START_OLD;
    size_t start = 0;
    size_t start_new = 0;

    bc_trie_t* rv = bc_trie_new(free);

    for (size_t current = 0; current < input_len; current++) {

        char c = input[current];

        switch (state) {
            case START_OLD:
                start = current;
                state = OLD;
                break;
            case OLD:
                if (c != ' ')
                    break;
                // no need to store old
                state = START_NEW;
                break;
            case START_NEW:
                start = current;
                state = NEW;
                break;
            case NEW:
                if (c != ' ')
                    break;
                state = START_REF;
                start_new = start;
                break;
            case START_REF:
                start = current;
                state = REF;
                break;
            case REF:
                if (c != '\n')
                    break;
                state = START_OLD;
                if ((current - start > 11) &&
                    (0 == strncmp("refs/heads/", input + start, 11)))
                {
                    char *key = bc_strndup(input + start + 11, current - start - 11);
                    bc_trie_insert(rv, key, bc_strndup(input + start_new, start - 1 - start_new));
                    free(key);
                }
                break;
        }
    }

    if (bc_trie_size(rv) == 0) {
        bc_trie_free(rv);
        return NULL;
    }

    return rv;
}
