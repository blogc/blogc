/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2019 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdbool.h>
#include <stdlib.h>
#include "../../src/common/utils.h"

#define BC_STRING_CHUNK_SIZE 128


static void
test_slist_append(void **state)
{
    bc_slist_t *l = NULL;
    l = bc_slist_append(l, (void*) bc_strdup("bola"));
    assert_non_null(l);
    assert_string_equal(l->data, "bola");
    assert_null(l->next);
    l = bc_slist_append(l, (void*) bc_strdup("guda"));
    assert_non_null(l);
    assert_string_equal(l->data, "bola");
    assert_non_null(l->next);
    assert_string_equal(l->next->data, "guda");
    assert_null(l->next->next);
    bc_slist_free_full(l, free);
}


static void
test_slist_prepend(void **state)
{
    bc_slist_t *l = NULL;
    l = bc_slist_prepend(l, (void*) bc_strdup("bola"));
    assert_non_null(l);
    assert_string_equal(l->data, "bola");
    assert_null(l->next);
    l = bc_slist_prepend(l, (void*) bc_strdup("guda"));
    assert_non_null(l);
    assert_string_equal(l->data, "guda");
    assert_non_null(l->next);
    assert_string_equal(l->next->data, "bola");
    assert_null(l->next->next);
    bc_slist_free_full(l, free);
}


static void
test_slist_remove(void **state)
{
    bc_slist_t *l = NULL;
    l = bc_slist_append(l, bc_strdup("bola"));
    l = bc_slist_append(l, bc_strdup("guda"));
    l = bc_slist_append(l, bc_strdup("chunda"));
    l = bc_slist_append(l, bc_strdup("pumba"));
    l = bc_slist_append(l, bc_strdup("bussunda"));
    l = bc_slist_remove(l, l->next->next, free);
    assert_non_null(l);
    assert_string_equal(l->data, "bola");
    assert_string_equal(l->next->data, "guda");
    assert_string_equal(l->next->next->data, "pumba");
    assert_string_equal(l->next->next->next->data, "bussunda");
    assert_null(l->next->next->next->next);

    l = bc_slist_remove(l, l, free);
    assert_non_null(l);
    assert_string_equal(l->data, "guda");
    assert_string_equal(l->next->data, "pumba");
    assert_string_equal(l->next->next->data, "bussunda");
    assert_null(l->next->next->next);

    l = bc_slist_remove(l, l->next->next, free);
    assert_non_null(l);
    assert_string_equal(l->data, "guda");
    assert_string_equal(l->next->data, "pumba");
    assert_null(l->next->next);

    bc_slist_free_full(l, free);
}


static void
test_slist_free(void **state)
{
    bc_slist_t *l = NULL;
    char *t1 = bc_strdup("bola");
    char *t2 = bc_strdup("guda");
    char *t3 = bc_strdup("chunda");
    l = bc_slist_append(l, (void*) t1);
    l = bc_slist_append(l, (void*) t2);
    l = bc_slist_append(l, (void*) t3);
    bc_slist_free(l);
    assert_string_equal(t1, "bola");
    assert_string_equal(t2, "guda");
    assert_string_equal(t3, "chunda");
    free(t1);
    free(t2);
    free(t3);
}


static void
test_slist_length(void **state)
{
    bc_slist_t *l = NULL;
    l = bc_slist_append(l, (void*) bc_strdup("bola"));
    l = bc_slist_append(l, (void*) bc_strdup("guda"));
    l = bc_slist_append(l, (void*) bc_strdup("chunda"));
    assert_int_equal(bc_slist_length(l), 3);
    bc_slist_free_full(l, free);
    assert_int_equal(bc_slist_length(NULL), 0);
}


static void
test_strdup(void **state)
{
    char *str = bc_strdup("bola");
    assert_string_equal(str, "bola");
    free(str);
    str = bc_strdup(NULL);
    assert_null(str);
}


static void
test_strndup(void **state)
{
    char *str = bc_strndup("bolaguda", 4);
    assert_string_equal(str, "bola");
    free(str);
    str = bc_strndup("bolaguda", 30);
    assert_string_equal(str, "bolaguda");
    free(str);
    str = bc_strndup("bolaguda", 8);
    assert_string_equal(str, "bolaguda");
    free(str);
    str = bc_strdup(NULL);
    assert_null(str);
}


static void
test_strdup_printf(void **state)
{
    char *str = bc_strdup_printf("bola");
    assert_string_equal(str, "bola");
    free(str);
    str = bc_strdup_printf("bola, %s", "guda");
    assert_string_equal(str, "bola, guda");
    free(str);
}


static void
test_str_starts_with(void **state)
{
    assert_true(bc_str_starts_with("bolaguda", "bola"));
    assert_true(bc_str_starts_with("bola", "bola"));
    assert_false(bc_str_starts_with("gudabola", "bola"));
    assert_false(bc_str_starts_with("guda", "bola"));
    assert_false(bc_str_starts_with("bola", "bolaguda"));
}


static void
test_str_ends_with(void **state)
{
    assert_true(bc_str_ends_with("bolaguda", "guda"));
    assert_true(bc_str_ends_with("bola", "bola"));
    assert_false(bc_str_ends_with("gudabola", "guda"));
    assert_false(bc_str_ends_with("guda", "bola"));
    assert_false(bc_str_ends_with("bola", "gudabola"));
}


static void
test_str_lstrip(void **state)
{
    char *str = bc_strdup("  \tbola\n  \t");
    assert_string_equal(bc_str_lstrip(str), "bola\n  \t");
    free(str);
    str = bc_strdup("guda");
    assert_string_equal(bc_str_lstrip(str), "guda");
    free(str);
    str = bc_strdup("\n");
    assert_string_equal(bc_str_lstrip(str), "");
    free(str);
    str = bc_strdup("\t \n");
    assert_string_equal(bc_str_lstrip(str), "");
    free(str);
    str = bc_strdup("");
    assert_string_equal(bc_str_lstrip(str), "");
    free(str);
    assert_null(bc_str_lstrip(NULL));
}


static void
test_str_rstrip(void **state)
{
    char *str = bc_strdup("  \tbola\n  \t");
    assert_string_equal(bc_str_rstrip(str), "  \tbola");
    free(str);
    str = bc_strdup("guda");
    assert_string_equal(bc_str_rstrip(str), "guda");
    free(str);
    str = bc_strdup("\n");
    assert_string_equal(bc_str_rstrip(str), "");
    free(str);
    str = bc_strdup("\t \n");
    assert_string_equal(bc_str_rstrip(str), "");
    free(str);
    str = bc_strdup("");
    assert_string_equal(bc_str_rstrip(str), "");
    free(str);
    assert_null(bc_str_rstrip(NULL));
}


static void
test_str_strip(void **state)
{
    char *str = bc_strdup("  \tbola\n  \t");
    assert_string_equal(bc_str_strip(str), "bola");
    free(str);
    str = bc_strdup("guda");
    assert_string_equal(bc_str_strip(str), "guda");
    free(str);
    str = bc_strdup("\n");
    assert_string_equal(bc_str_strip(str), "");
    free(str);
    str = bc_strdup("\t \n");
    assert_string_equal(bc_str_strip(str), "");
    free(str);
    str = bc_strdup("");
    assert_string_equal(bc_str_strip(str), "");
    free(str);
    assert_null(bc_str_strip(NULL));
}


static void
test_str_split(void **state)
{
    char **strv = bc_str_split("bola:guda:chunda", ':', 0);
    assert_string_equal(strv[0], "bola");
    assert_string_equal(strv[1], "guda");
    assert_string_equal(strv[2], "chunda");
    assert_null(strv[3]);
    bc_strv_free(strv);
    strv = bc_str_split("bola:guda:chunda", ':', 2);
    assert_string_equal(strv[0], "bola");
    assert_string_equal(strv[1], "guda:chunda");
    assert_null(strv[2]);
    bc_strv_free(strv);
    strv = bc_str_split("bola:guda:chunda", ':', 1);
    assert_string_equal(strv[0], "bola:guda:chunda");
    assert_null(strv[1]);
    bc_strv_free(strv);
    strv = bc_str_split("", ':', 1);
    assert_null(strv[0]);
    bc_strv_free(strv);
    assert_null(bc_str_split(NULL, ':', 0));
}


static void
test_str_replace(void **state)
{
    char *str = bc_str_replace("bolao", 'o', "zaz");
    assert_string_equal(str, "bzazlazaz");
    free(str);
    str = bc_str_replace("bolao", 'b', "zaz");
    assert_string_equal(str, "zazolao");
    free(str);
    str = bc_str_replace("bolao", 'b', NULL);
    assert_string_equal(str, "bolao");
    free(str);
    assert_null(bc_str_replace(NULL, 'b', "zaz"));
}


static void
test_str_find(void **state)
{
    assert_null(bc_str_find(NULL, 'c'));
    assert_string_equal(bc_str_find("bola", 'l'), "la");
    assert_string_equal(bc_str_find("bo\\lalala", 'l'), "lala");
    assert_string_equal(bc_str_find("bola", '\0'), "");
    assert_null(bc_str_find("bola", 'g'));
    assert_null(bc_str_find("bo\\la", 'l'));
}


static void
test_str_to_bool(void **state)
{
    assert_false(bc_str_to_bool(NULL));
    assert_true(bc_str_to_bool("1"));
    assert_true(bc_str_to_bool("y"));
    assert_true(bc_str_to_bool("Y"));
    assert_true(bc_str_to_bool("yes"));
    assert_true(bc_str_to_bool("YES"));
    assert_true(bc_str_to_bool("true"));
    assert_true(bc_str_to_bool("TRUE"));
    assert_true(bc_str_to_bool("on"));
    assert_true(bc_str_to_bool("ON"));
    assert_false(bc_str_to_bool("0"));
    assert_false(bc_str_to_bool("n"));
    assert_false(bc_str_to_bool("N"));
    assert_false(bc_str_to_bool("no"));
    assert_false(bc_str_to_bool("NO"));
    assert_false(bc_str_to_bool("false"));
    assert_false(bc_str_to_bool("FALSE"));
    assert_false(bc_str_to_bool("off"));
    assert_false(bc_str_to_bool("OFF"));
}


static void
test_strv_join(void **state)
{
    char *pieces[] = {"guda","bola", "chunda", NULL};
    char *str = bc_strv_join(pieces, ":");
    assert_string_equal(str, "guda:bola:chunda");
    free(str);
    char *pieces2[] = {NULL};
    str = bc_strv_join(pieces2, ":");
    assert_string_equal(str, "");
    free(str);
    assert_null(bc_strv_join(pieces, NULL));
    assert_null(bc_strv_join(NULL, ":"));
    assert_null(bc_strv_join(NULL, NULL));
}


static void
test_strv_length(void **state)
{
    char *pieces[] = {"guda","bola", "chunda", NULL};
    assert_int_equal(bc_strv_length(pieces), 3);
    char *pieces2[] = {NULL};
    assert_int_equal(bc_strv_length(pieces2), 0);
    assert_int_equal(bc_strv_length(NULL), 0);
}


static void
test_string_new(void **state)
{
    bc_string_t *str = bc_string_new();
    assert_non_null(str);
    assert_string_equal(str->str, "");
    assert_int_equal(str->len, 0);
    assert_int_equal(str->allocated_len, BC_STRING_CHUNK_SIZE);
    assert_null(bc_string_free(str, true));
}


static void
test_string_free(void **state)
{
    bc_string_t *str = bc_string_new();
    free(str->str);
    str->str = bc_strdup("bola");
    str->len = 4;
    str->allocated_len = BC_STRING_CHUNK_SIZE;
    char *tmp = bc_string_free(str, false);
    assert_string_equal(tmp, "bola");
    free(tmp);
    assert_null(bc_string_free(NULL, false));
}


static void
test_string_dup(void **state)
{
    bc_string_t *str = bc_string_new();
    free(str->str);
    str->str = bc_strdup("bola");
    str->len = 4;
    str->allocated_len = BC_STRING_CHUNK_SIZE;
    bc_string_t *new = bc_string_dup(str);
    assert_non_null(new);
    assert_string_equal(new->str, "bola");
    assert_int_equal(new->len, 4);
    assert_int_equal(new->allocated_len, BC_STRING_CHUNK_SIZE);
    assert_null(bc_string_free(new, true));
    assert_null(bc_string_free(str, true));
    assert_null(bc_string_dup(NULL));
}


static void
test_string_append_len(void **state)
{
    bc_string_t *str = bc_string_new();
    str = bc_string_append_len(str, "guda", 4);
    assert_non_null(str);
    assert_string_equal(str->str, "guda");
    assert_int_equal(str->len, 4);
    assert_int_equal(str->allocated_len, BC_STRING_CHUNK_SIZE);
    assert_null(bc_string_free(str, true));
    str = bc_string_new();
    str = bc_string_append_len(str, "guda", 4);
    str = bc_string_append_len(str, "bola", 4);
    assert_non_null(str);
    assert_string_equal(str->str, "gudabola");
    assert_int_equal(str->len, 8);
    assert_int_equal(str->allocated_len, BC_STRING_CHUNK_SIZE);
    assert_null(bc_string_free(str, true));
    str = bc_string_new();
    str = bc_string_append_len(str, "guda", 3);
    str = bc_string_append_len(str, "bola", 4);
    assert_non_null(str);
    assert_string_equal(str->str, "gudbola");
    assert_int_equal(str->len, 7);
    assert_int_equal(str->allocated_len, BC_STRING_CHUNK_SIZE);
    assert_null(bc_string_free(str, true));
    str = bc_string_new();
    str = bc_string_append_len(str, "guda", 4);
    str = bc_string_append_len(str,
        "cwlwmwxxmvjnwtidmjehzdeexbxjnjowruxjrqpgpfhmvwgqeacdjissntmbtsjidzkcw"
        "nnqhxhneolbwqlctcxmrsutolrjikpavxombpfpjyaqltgvzrjidotalcuwrwxtaxjiwa"
        "xfhfyzymtffusoqywaruxpybwggukltspqqmghzpqstvcvlqbkhquihzndnrvkaqvevaz"
        "dxrewtgapkompnviiyielanoyowgqhssntyvcvqqtfjmkphywbkvzfyttaalttywhqcec"
        "hgrwzaglzogwjvqncjzodaqsblcbpcdpxmrtctzginvtkckhqvdplgjvbzrnarcxjrsbc"
        "sbfvpylgjznsuhxcxoqbpxowmsrgwimxjgyzwwmryqvstwzkglgeezelvpvkwefqdatnd"
        "dxntikgoqlidfnmdhxzevqzlzubvyleeksdirmmttqthhkvfjggznpmarcamacpvwsrnr"
        "ftzfeyasjpxoevyptpdnqokswiondusnuymqwaryrmdgscbnuilxtypuynckancsfnwtg"
        "okxhegoifakimxbbafkeannglvsxprqzfekdinssqymtfexf", 600);
    str = bc_string_append_len(str, NULL, 0);
    str = bc_string_append_len(str,
        "cwlwmwxxmvjnwtidmjehzdeexbxjnjowruxjrqpgpfhmvwgqeacdjissntmbtsjidzkcw"
        "nnqhxhneolbwqlctcxmrsutolrjikpavxombpfpjyaqltgvzrjidotalcuwrwxtaxjiwa"
        "xfhfyzymtffusoqywaruxpybwggukltspqqmghzpqstvcvlqbkhquihzndnrvkaqvevaz"
        "dxrewtgapkompnviiyielanoyowgqhssntyvcvqqtfjmkphywbkvzfyttaalttywhqcec"
        "hgrwzaglzogwjvqncjzodaqsblcbpcdpxmrtctzginvtkckhqvdplgjvbzrnarcxjrsbc"
        "sbfvpylgjznsuhxcxoqbpxowmsrgwimxjgyzwwmryqvstwzkglgeezelvpvkwefqdatnd"
        "dxntikgoqlidfnmdhxzevqzlzubvyleeksdirmmttqthhkvfjggznpmarcamacpvwsrnr"
        "ftzfeyasjpxoevyptpdnqokswiondusnuymqwaryrmdgscbnuilxtypuynckancsfnwtg"
        "okxhegoifakimxbbafkeannglvsxprqzfekdinssqymtfexf", 600);
    assert_non_null(str);
    assert_string_equal(str->str,
        "gudacwlwmwxxmvjnwtidmjehzdeexbxjnjowruxjrqpgpfhmvwgqeacdjissntmbtsjid"
        "zkcwnnqhxhneolbwqlctcxmrsutolrjikpavxombpfpjyaqltgvzrjidotalcuwrwxtax"
        "jiwaxfhfyzymtffusoqywaruxpybwggukltspqqmghzpqstvcvlqbkhquihzndnrvkaqv"
        "evazdxrewtgapkompnviiyielanoyowgqhssntyvcvqqtfjmkphywbkvzfyttaalttywh"
        "qcechgrwzaglzogwjvqncjzodaqsblcbpcdpxmrtctzginvtkckhqvdplgjvbzrnarcxj"
        "rsbcsbfvpylgjznsuhxcxoqbpxowmsrgwimxjgyzwwmryqvstwzkglgeezelvpvkwefqd"
        "atnddxntikgoqlidfnmdhxzevqzlzubvyleeksdirmmttqthhkvfjggznpmarcamacpvw"
        "srnrftzfeyasjpxoevyptpdnqokswiondusnuymqwaryrmdgscbnuilxtypuynckancsf"
        "nwtgokxhegoifakimxbbafkeannglvsxprqzfekdinssqymtfexfcwlwmwxxmvjnwtidm"
        "jehzdeexbxjnjowruxjrqpgpfhmvwgqeacdjissntmbtsjidzkcwnnqhxhneolbwqlctc"
        "xmrsutolrjikpavxombpfpjyaqltgvzrjidotalcuwrwxtaxjiwaxfhfyzymtffusoqyw"
        "aruxpybwggukltspqqmghzpqstvcvlqbkhquihzndnrvkaqvevazdxrewtgapkompnvii"
        "yielanoyowgqhssntyvcvqqtfjmkphywbkvzfyttaalttywhqcechgrwzaglzogwjvqnc"
        "jzodaqsblcbpcdpxmrtctzginvtkckhqvdplgjvbzrnarcxjrsbcsbfvpylgjznsuhxcx"
        "oqbpxowmsrgwimxjgyzwwmryqvstwzkglgeezelvpvkwefqdatnddxntikgoqlidfnmdh"
        "xzevqzlzubvyleeksdirmmttqthhkvfjggznpmarcamacpvwsrnrftzfeyasjpxoevypt"
        "pdnqokswiondusnuymqwaryrmdgscbnuilxtypuynckancsfnwtgokxhegoifakimxbba"
        "fkeannglvsxprqzfekdinssqymtfexf");
    assert_int_equal(str->len, 1204);
    assert_int_equal(str->allocated_len, BC_STRING_CHUNK_SIZE * 10);
    assert_null(bc_string_free(str, true));
    str = bc_string_new();
    str = bc_string_append_len(str, NULL, 0);
    assert_non_null(str);
    assert_string_equal(str->str, "");
    assert_int_equal(str->len, 0);
    assert_int_equal(str->allocated_len, BC_STRING_CHUNK_SIZE);
    assert_null(bc_string_free(str, true));
    assert_null(bc_string_append_len(NULL, "foo", 3));
}


static void
test_string_append(void **state)
{
    bc_string_t *str = bc_string_new();
    str = bc_string_append(str, "guda");
    assert_non_null(str);
    assert_string_equal(str->str, "guda");
    assert_int_equal(str->len, 4);
    assert_int_equal(str->allocated_len, BC_STRING_CHUNK_SIZE);
    assert_null(bc_string_free(str, true));
    str = bc_string_new();
    str = bc_string_append(str, "guda");
    str = bc_string_append(str, "bola");
    assert_non_null(str);
    assert_string_equal(str->str, "gudabola");
    assert_int_equal(str->len, 8);
    assert_int_equal(str->allocated_len, BC_STRING_CHUNK_SIZE);
    assert_null(bc_string_free(str, true));
    str = bc_string_new();
    str = bc_string_append(str, "guda");
    str = bc_string_append(str,
        "cwlwmwxxmvjnwtidmjehzdeexbxjnjowruxjrqpgpfhmvwgqeacdjissntmbtsjidzkcw"
        "nnqhxhneolbwqlctcxmrsutolrjikpavxombpfpjyaqltgvzrjidotalcuwrwxtaxjiwa"
        "xfhfyzymtffusoqywaruxpybwggukltspqqmghzpqstvcvlqbkhquihzndnrvkaqvevaz"
        "dxrewtgapkompnviiyielanoyowgqhssntyvcvqqtfjmkphywbkvzfyttaalttywhqcec"
        "hgrwzaglzogwjvqncjzodaqsblcbpcdpxmrtctzginvtkckhqvdplgjvbzrnarcxjrsbc"
        "sbfvpylgjznsuhxcxoqbpxowmsrgwimxjgyzwwmryqvstwzkglgeezelvpvkwefqdatnd"
        "dxntikgoqlidfnmdhxzevqzlzubvyleeksdirmmttqthhkvfjggznpmarcamacpvwsrnr"
        "ftzfeyasjpxoevyptpdnqokswiondusnuymqwaryrmdgscbnuilxtypuynckancsfnwtg"
        "okxhegoifakimxbbafkeannglvsxprqzfekdinssqymtfexf");
    str = bc_string_append(str, NULL);
    str = bc_string_append(str,
        "cwlwmwxxmvjnwtidmjehzdeexbxjnjowruxjrqpgpfhmvwgqeacdjissntmbtsjidzkcw"
        "nnqhxhneolbwqlctcxmrsutolrjikpavxombpfpjyaqltgvzrjidotalcuwrwxtaxjiwa"
        "xfhfyzymtffusoqywaruxpybwggukltspqqmghzpqstvcvlqbkhquihzndnrvkaqvevaz"
        "dxrewtgapkompnviiyielanoyowgqhssntyvcvqqtfjmkphywbkvzfyttaalttywhqcec"
        "hgrwzaglzogwjvqncjzodaqsblcbpcdpxmrtctzginvtkckhqvdplgjvbzrnarcxjrsbc"
        "sbfvpylgjznsuhxcxoqbpxowmsrgwimxjgyzwwmryqvstwzkglgeezelvpvkwefqdatnd"
        "dxntikgoqlidfnmdhxzevqzlzubvyleeksdirmmttqthhkvfjggznpmarcamacpvwsrnr"
        "ftzfeyasjpxoevyptpdnqokswiondusnuymqwaryrmdgscbnuilxtypuynckancsfnwtg"
        "okxhegoifakimxbbafkeannglvsxprqzfekdinssqymtfexf");
    assert_non_null(str);
    assert_string_equal(str->str,
        "gudacwlwmwxxmvjnwtidmjehzdeexbxjnjowruxjrqpgpfhmvwgqeacdjissntmbtsjid"
        "zkcwnnqhxhneolbwqlctcxmrsutolrjikpavxombpfpjyaqltgvzrjidotalcuwrwxtax"
        "jiwaxfhfyzymtffusoqywaruxpybwggukltspqqmghzpqstvcvlqbkhquihzndnrvkaqv"
        "evazdxrewtgapkompnviiyielanoyowgqhssntyvcvqqtfjmkphywbkvzfyttaalttywh"
        "qcechgrwzaglzogwjvqncjzodaqsblcbpcdpxmrtctzginvtkckhqvdplgjvbzrnarcxj"
        "rsbcsbfvpylgjznsuhxcxoqbpxowmsrgwimxjgyzwwmryqvstwzkglgeezelvpvkwefqd"
        "atnddxntikgoqlidfnmdhxzevqzlzubvyleeksdirmmttqthhkvfjggznpmarcamacpvw"
        "srnrftzfeyasjpxoevyptpdnqokswiondusnuymqwaryrmdgscbnuilxtypuynckancsf"
        "nwtgokxhegoifakimxbbafkeannglvsxprqzfekdinssqymtfexfcwlwmwxxmvjnwtidm"
        "jehzdeexbxjnjowruxjrqpgpfhmvwgqeacdjissntmbtsjidzkcwnnqhxhneolbwqlctc"
        "xmrsutolrjikpavxombpfpjyaqltgvzrjidotalcuwrwxtaxjiwaxfhfyzymtffusoqyw"
        "aruxpybwggukltspqqmghzpqstvcvlqbkhquihzndnrvkaqvevazdxrewtgapkompnvii"
        "yielanoyowgqhssntyvcvqqtfjmkphywbkvzfyttaalttywhqcechgrwzaglzogwjvqnc"
        "jzodaqsblcbpcdpxmrtctzginvtkckhqvdplgjvbzrnarcxjrsbcsbfvpylgjznsuhxcx"
        "oqbpxowmsrgwimxjgyzwwmryqvstwzkglgeezelvpvkwefqdatnddxntikgoqlidfnmdh"
        "xzevqzlzubvyleeksdirmmttqthhkvfjggznpmarcamacpvwsrnrftzfeyasjpxoevypt"
        "pdnqokswiondusnuymqwaryrmdgscbnuilxtypuynckancsfnwtgokxhegoifakimxbba"
        "fkeannglvsxprqzfekdinssqymtfexf");
    assert_int_equal(str->len, 1204);
    assert_int_equal(str->allocated_len, BC_STRING_CHUNK_SIZE * 10);
    assert_null(bc_string_free(str, true));
    str = bc_string_new();
    str = bc_string_append(str, NULL);
    assert_non_null(str);
    assert_string_equal(str->str, "");
    assert_int_equal(str->len, 0);
    assert_int_equal(str->allocated_len, BC_STRING_CHUNK_SIZE);
    assert_null(bc_string_free(str, true));
    assert_null(bc_string_append(NULL, "asd"));
    assert_null(bc_string_append(NULL, NULL));
}


static void
test_string_append_c(void **state)
{
    bc_string_t *str = bc_string_new();
    str = bc_string_append_len(str, "guda", 4);
    for (int i = 0; i < 600; i++)
        str = bc_string_append_c(str, 'c');
    assert_non_null(str);
    assert_string_equal(str->str,
        "gudaccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc"
        "ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc"
        "ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc"
        "ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc"
        "ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc"
        "ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc"
        "ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc"
        "ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc"
        "cccccccccccccccccccccccccccccccccccccccccccccccccccc");
    assert_int_equal(str->len, 604);
    assert_int_equal(str->allocated_len, BC_STRING_CHUNK_SIZE * 5);
    assert_null(bc_string_free(str, true));
    assert_null(bc_string_append_c(NULL, 0));
}


static void
test_string_append_printf(void **state)
{
    bc_string_t *str = bc_string_new();
    str = bc_string_append_printf(str, "guda: %s %d", "bola", 1);
    assert_non_null(str);
    assert_string_equal(str->str, "guda: bola 1");
    assert_int_equal(str->len, 12);
    assert_int_equal(str->allocated_len, BC_STRING_CHUNK_SIZE);
    assert_null(bc_string_free(str, true));
    assert_null(bc_string_append_printf(NULL, "asd"));
}


static void
test_string_append_escaped(void **state)
{
    bc_string_t *str = bc_string_new();
    str = bc_string_append_escaped(str, NULL);
    assert_non_null(str);
    assert_string_equal(str->str, "");
    assert_int_equal(str->len, 0);
    assert_int_equal(str->allocated_len, BC_STRING_CHUNK_SIZE);
    str = bc_string_append_escaped(str, "foo \\a bar \\\\ lol");
    assert_non_null(str);
    assert_string_equal(str->str, "foo a bar \\ lol");
    assert_int_equal(str->len, 15);
    assert_int_equal(str->allocated_len, BC_STRING_CHUNK_SIZE);
    assert_null(bc_string_free(str, true));
    assert_null(bc_string_append_escaped(NULL, "asd"));
}


static void
test_trie_new(void **state)
{
    bc_trie_t *trie = bc_trie_new(free);
    assert_non_null(trie);
    assert_null(trie->root);
    assert_true(trie->free_func == free);
    bc_trie_free(trie);
}


static void
test_trie_insert(void **state)
{
    bc_trie_t *trie = bc_trie_new(free);

    bc_trie_insert(trie, "bola", bc_strdup("guda"));
    assert_true(trie->root->key == 'b');
    assert_null(trie->root->data);
    assert_true(trie->root->child->key == 'o');
    assert_null(trie->root->child->data);
    assert_true(trie->root->child->child->key == 'l');
    assert_null(trie->root->child->child->data);
    assert_true(trie->root->child->child->child->key == 'a');
    assert_null(trie->root->child->child->child->data);
    assert_true(trie->root->child->child->child->child->key == '\0');
    assert_string_equal(trie->root->child->child->child->child->data, "guda");


    bc_trie_insert(trie, "chu", bc_strdup("nda"));
    assert_true(trie->root->key == 'b');
    assert_null(trie->root->data);
    assert_true(trie->root->child->key == 'o');
    assert_null(trie->root->child->data);
    assert_true(trie->root->child->child->key == 'l');
    assert_null(trie->root->child->child->data);
    assert_true(trie->root->child->child->child->key == 'a');
    assert_null(trie->root->child->child->child->data);
    assert_true(trie->root->child->child->child->child->key == '\0');
    assert_string_equal(trie->root->child->child->child->child->data, "guda");

    assert_true(trie->root->next->key == 'c');
    assert_null(trie->root->next->data);
    assert_true(trie->root->next->child->key == 'h');
    assert_null(trie->root->next->child->data);
    assert_true(trie->root->next->child->child->key == 'u');
    assert_null(trie->root->next->child->child->data);
    assert_true(trie->root->next->child->child->child->key == '\0');
    assert_string_equal(trie->root->next->child->child->child->data, "nda");


    bc_trie_insert(trie, "bote", bc_strdup("aba"));
    assert_true(trie->root->key == 'b');
    assert_null(trie->root->data);
    assert_true(trie->root->child->key == 'o');
    assert_null(trie->root->child->data);
    assert_true(trie->root->child->child->key == 'l');
    assert_null(trie->root->child->child->data);
    assert_true(trie->root->child->child->child->key == 'a');
    assert_null(trie->root->child->child->child->data);
    assert_true(trie->root->child->child->child->child->key == '\0');
    assert_string_equal(trie->root->child->child->child->child->data, "guda");

    assert_true(trie->root->next->key == 'c');
    assert_null(trie->root->next->data);
    assert_true(trie->root->next->child->key == 'h');
    assert_null(trie->root->next->child->data);
    assert_true(trie->root->next->child->child->key == 'u');
    assert_null(trie->root->next->child->child->data);
    assert_true(trie->root->next->child->child->child->key == '\0');
    assert_string_equal(trie->root->next->child->child->child->data, "nda");

    assert_true(trie->root->child->child->next->key == 't');
    assert_null(trie->root->child->child->next->data);
    assert_true(trie->root->child->child->next->child->key == 'e');
    assert_null(trie->root->child->child->next->child->data);
    assert_true(trie->root->child->child->next->child->child->key == '\0');
    assert_string_equal(trie->root->child->child->next->child->child->data, "aba");


    bc_trie_insert(trie, "bo", bc_strdup("haha"));
    assert_true(trie->root->key == 'b');
    assert_null(trie->root->data);
    assert_true(trie->root->child->key == 'o');
    assert_null(trie->root->child->data);
    assert_true(trie->root->child->child->key == 'l');
    assert_null(trie->root->child->child->data);
    assert_true(trie->root->child->child->child->key == 'a');
    assert_null(trie->root->child->child->child->data);
    assert_true(trie->root->child->child->child->child->key == '\0');
    assert_string_equal(trie->root->child->child->child->child->data, "guda");

    assert_true(trie->root->next->key == 'c');
    assert_null(trie->root->next->data);
    assert_true(trie->root->next->child->key == 'h');
    assert_null(trie->root->next->child->data);
    assert_true(trie->root->next->child->child->key == 'u');
    assert_null(trie->root->next->child->child->data);
    assert_true(trie->root->next->child->child->child->key == '\0');
    assert_string_equal(trie->root->next->child->child->child->data, "nda");

    assert_true(trie->root->child->child->next->key == 't');
    assert_null(trie->root->child->child->next->data);
    assert_true(trie->root->child->child->next->child->key == 'e');
    assert_null(trie->root->child->child->next->child->data);
    assert_true(trie->root->child->child->next->child->child->key == '\0');
    assert_string_equal(trie->root->child->child->next->child->child->data, "aba");

    assert_true(trie->root->child->child->next->next->key == '\0');
    assert_string_equal(trie->root->child->child->next->next->data, "haha");

    bc_trie_free(trie);


    trie = bc_trie_new(free);

    bc_trie_insert(trie, "chu", bc_strdup("nda"));
    assert_true(trie->root->key == 'c');
    assert_null(trie->root->data);
    assert_true(trie->root->child->key == 'h');
    assert_null(trie->root->child->data);
    assert_true(trie->root->child->child->key == 'u');
    assert_null(trie->root->child->child->data);
    assert_true(trie->root->child->child->child->key == '\0');
    assert_string_equal(trie->root->child->child->child->data, "nda");


    bc_trie_insert(trie, "bola", bc_strdup("guda"));
    assert_true(trie->root->key == 'c');
    assert_null(trie->root->data);
    assert_true(trie->root->child->key == 'h');
    assert_null(trie->root->child->data);
    assert_true(trie->root->child->child->key == 'u');
    assert_null(trie->root->child->child->data);
    assert_true(trie->root->child->child->child->key == '\0');
    assert_string_equal(trie->root->child->child->child->data, "nda");

    assert_true(trie->root->next->key == 'b');
    assert_null(trie->root->next->data);
    assert_true(trie->root->next->child->key == 'o');
    assert_null(trie->root->next->child->data);
    assert_true(trie->root->next->child->child->key == 'l');
    assert_null(trie->root->next->child->child->data);
    assert_true(trie->root->next->child->child->child->key == 'a');
    assert_null(trie->root->next->child->child->child->data);
    assert_true(trie->root->next->child->child->child->child->key == '\0');
    assert_string_equal(trie->root->next->child->child->child->child->data, "guda");


    bc_trie_insert(trie, "bote", bc_strdup("aba"));
    assert_true(trie->root->key == 'c');
    assert_null(trie->root->data);
    assert_true(trie->root->child->key == 'h');
    assert_null(trie->root->child->data);
    assert_true(trie->root->child->child->key == 'u');
    assert_null(trie->root->child->child->data);
    assert_true(trie->root->child->child->child->key == '\0');
    assert_string_equal(trie->root->child->child->child->data, "nda");

    assert_true(trie->root->next->key == 'b');
    assert_null(trie->root->next->data);
    assert_true(trie->root->next->child->key == 'o');
    assert_null(trie->root->next->child->data);
    assert_true(trie->root->next->child->child->key == 'l');
    assert_null(trie->root->next->child->child->data);
    assert_true(trie->root->next->child->child->child->key == 'a');
    assert_null(trie->root->next->child->child->child->data);
    assert_true(trie->root->next->child->child->child->child->key == '\0');
    assert_string_equal(trie->root->next->child->child->child->child->data, "guda");

    assert_true(trie->root->next->child->child->next->key == 't');
    assert_null(trie->root->next->child->child->next->data);
    assert_true(trie->root->next->child->child->next->child->key == 'e');
    assert_null(trie->root->next->child->child->next->child->data);
    assert_true(trie->root->next->child->child->next->child->child->key == '\0');
    assert_string_equal(trie->root->next->child->child->next->child->child->data, "aba");


    bc_trie_insert(trie, "bo", bc_strdup("haha"));
    assert_true(trie->root->key == 'c');
    assert_null(trie->root->data);
    assert_true(trie->root->child->key == 'h');
    assert_null(trie->root->child->data);
    assert_true(trie->root->child->child->key == 'u');
    assert_null(trie->root->child->child->data);
    assert_true(trie->root->child->child->child->key == '\0');
    assert_string_equal(trie->root->child->child->child->data, "nda");

    assert_true(trie->root->next->key == 'b');
    assert_null(trie->root->next->data);
    assert_true(trie->root->next->child->key == 'o');
    assert_null(trie->root->next->child->data);
    assert_true(trie->root->next->child->child->key == 'l');
    assert_null(trie->root->next->child->child->data);
    assert_true(trie->root->next->child->child->child->key == 'a');
    assert_null(trie->root->next->child->child->child->data);
    assert_true(trie->root->next->child->child->child->child->key == '\0');
    assert_string_equal(trie->root->next->child->child->child->child->data, "guda");

    assert_true(trie->root->next->child->child->next->key == 't');
    assert_null(trie->root->next->child->child->next->data);
    assert_true(trie->root->next->child->child->next->child->key == 'e');
    assert_null(trie->root->next->child->child->next->child->data);
    assert_true(trie->root->next->child->child->next->child->child->key == '\0');
    assert_string_equal(trie->root->next->child->child->next->child->child->data, "aba");

    assert_true(trie->root->next->child->child->next->next->key == '\0');
    assert_string_equal(trie->root->next->child->child->next->next->data, "haha");

    bc_trie_free(trie);
}


static void
test_trie_insert_duplicated(void **state)
{
    bc_trie_t *trie = bc_trie_new(free);

    bc_trie_insert(trie, "bola", bc_strdup("guda"));
    assert_true(trie->root->key == 'b');
    assert_null(trie->root->data);
    assert_true(trie->root->child->key == 'o');
    assert_null(trie->root->child->data);
    assert_true(trie->root->child->child->key == 'l');
    assert_null(trie->root->child->child->data);
    assert_true(trie->root->child->child->child->key == 'a');
    assert_null(trie->root->child->child->child->data);
    assert_true(trie->root->child->child->child->child->key == '\0');
    assert_string_equal(trie->root->child->child->child->child->data, "guda");

    bc_trie_insert(trie, "bola", bc_strdup("asdf"));
    assert_true(trie->root->key == 'b');
    assert_null(trie->root->data);
    assert_true(trie->root->child->key == 'o');
    assert_null(trie->root->child->data);
    assert_true(trie->root->child->child->key == 'l');
    assert_null(trie->root->child->child->data);
    assert_true(trie->root->child->child->child->key == 'a');
    assert_null(trie->root->child->child->child->data);
    assert_true(trie->root->child->child->child->child->key == '\0');
    assert_string_equal(trie->root->child->child->child->child->data, "asdf");

    bc_trie_free(trie);

    trie = NULL;
    bc_trie_insert(trie, "bola", NULL);
    assert_null(trie);
}


static void
test_trie_keep_data(void **state)
{
    bc_trie_t *trie = bc_trie_new(NULL);

    char *t1 = "guda";
    char *t2 = "nda";
    char *t3 = "aba";
    char *t4 = "haha";

    bc_trie_insert(trie, "bola", t1);
    bc_trie_insert(trie, "chu", t2);
    bc_trie_insert(trie, "bote", t3);
    bc_trie_insert(trie, "bo", t4);

    bc_trie_free(trie);

    assert_string_equal(t1, "guda");
    assert_string_equal(t2, "nda");
    assert_string_equal(t3, "aba");
    assert_string_equal(t4, "haha");
}


static void
test_trie_lookup(void **state)
{
    bc_trie_t *trie = bc_trie_new(free);

    bc_trie_insert(trie, "bola", bc_strdup("guda"));
    bc_trie_insert(trie, "chu", bc_strdup("nda"));
    bc_trie_insert(trie, "bote", bc_strdup("aba"));
    bc_trie_insert(trie, "bo", bc_strdup("haha"));

    assert_string_equal(bc_trie_lookup(trie, "bola"), "guda");
    assert_string_equal(bc_trie_lookup(trie, "chu"), "nda");
    assert_string_equal(bc_trie_lookup(trie, "bote"), "aba");
    assert_string_equal(bc_trie_lookup(trie, "bo"), "haha");

    assert_null(bc_trie_lookup(trie, "arcoiro"));

    bc_trie_free(trie);

    trie = bc_trie_new(free);

    bc_trie_insert(trie, "chu", bc_strdup("nda"));
    bc_trie_insert(trie, "bola", bc_strdup("guda"));
    bc_trie_insert(trie, "bote", bc_strdup("aba"));
    bc_trie_insert(trie, "bo", bc_strdup("haha"));
    bc_trie_insert(trie, "copa", bc_strdup("bu"));
    bc_trie_insert(trie, "b", bc_strdup("c"));
    bc_trie_insert(trie, "test", bc_strdup("asd"));

    assert_string_equal(bc_trie_lookup(trie, "bola"), "guda");
    assert_string_equal(bc_trie_lookup(trie, "chu"), "nda");
    assert_string_equal(bc_trie_lookup(trie, "bote"), "aba");
    assert_string_equal(bc_trie_lookup(trie, "bo"), "haha");

    assert_null(bc_trie_lookup(trie, "arcoiro"));

    bc_trie_free(trie);

    assert_null(bc_trie_lookup(NULL, "bola"));
}


static void
test_trie_size(void **state)
{
    bc_trie_t *trie = bc_trie_new(free);

    bc_trie_insert(trie, "bola", bc_strdup("guda"));
    bc_trie_insert(trie, "chu", bc_strdup("nda"));
    bc_trie_insert(trie, "bote", bc_strdup("aba"));
    bc_trie_insert(trie, "bo", bc_strdup("haha"));

    assert_int_equal(bc_trie_size(trie), 4);
    assert_int_equal(bc_trie_size(NULL), 0);

    bc_trie_free(trie);

    trie = bc_trie_new(free);

    bc_trie_insert(trie, "chu", bc_strdup("nda"));
    bc_trie_insert(trie, "bola", bc_strdup("guda"));
    bc_trie_insert(trie, "bote", bc_strdup("aba"));
    bc_trie_insert(trie, "bo", bc_strdup("haha"));
    bc_trie_insert(trie, "copa", bc_strdup("bu"));
    bc_trie_insert(trie, "b", bc_strdup("c"));
    bc_trie_insert(trie, "test", bc_strdup("asd"));

    assert_int_equal(bc_trie_size(trie), 7);
    assert_int_equal(bc_trie_size(NULL), 0);

    bc_trie_free(trie);
}


static size_t counter;
static char *expected_keys[] = {"chu", "copa", "bola", "bote", "bo", "b", "test", "testa"};
static char *expected_datas[] = {"nda", "bu", "guda", "aba", "haha", "c", "asd", "lol"};

static void
mock_foreach(const char *key, void *data, void *user_data)
{
    assert_string_equal(user_data, "foo");
    assert_string_equal(key, expected_keys[counter]);
    assert_string_equal((char*) data, expected_datas[counter++]);
}


static void
test_trie_foreach(void **state)
{
    bc_trie_t *trie = bc_trie_new(free);

    bc_trie_insert(trie, "chu", bc_strdup("nda"));
    bc_trie_insert(trie, "bola", bc_strdup("guda"));
    bc_trie_insert(trie, "bote", bc_strdup("aba"));
    bc_trie_insert(trie, "bo", bc_strdup("haha"));
    bc_trie_insert(trie, "copa", bc_strdup("bu"));
    bc_trie_insert(trie, "b", bc_strdup("c"));
    bc_trie_insert(trie, "test", bc_strdup("asd"));
    bc_trie_insert(trie, "testa", bc_strdup("lol"));

    counter = 0;
    bc_trie_foreach(trie, mock_foreach, "foo");
    bc_trie_foreach(NULL, mock_foreach, "foo");
    bc_trie_foreach(trie, NULL, "foo");
    bc_trie_foreach(NULL, NULL, "foo");
    assert_int_equal(counter, 8);

    bc_trie_free(trie);
}


static void
test_trie_inserted_after_prefix(void **state)
{
    bc_trie_t *trie = bc_trie_new(free);

    bc_trie_insert(trie, "bola", bc_strdup("guda"));
    assert_true(trie->root->key == 'b');
    assert_null(trie->root->data);
    assert_true(trie->root->child->key == 'o');
    assert_null(trie->root->child->data);
    assert_true(trie->root->child->child->key == 'l');
    assert_null(trie->root->child->child->data);
    assert_true(trie->root->child->child->child->key == 'a');
    assert_null(trie->root->child->child->child->data);
    assert_true(trie->root->child->child->child->child->key == '\0');
    assert_string_equal(trie->root->child->child->child->child->data, "guda");

    bc_trie_insert(trie, "bolaoo", bc_strdup("asdf"));
    assert_true(trie->root->key == 'b');
    assert_null(trie->root->data);
    assert_true(trie->root->child->key == 'o');
    assert_null(trie->root->child->data);
    assert_true(trie->root->child->child->key == 'l');
    assert_null(trie->root->child->child->data);
    assert_true(trie->root->child->child->child->key == 'a');
    assert_null(trie->root->child->child->child->data);
    assert_true(trie->root->child->child->child->child->key == '\0');
    assert_string_equal(trie->root->child->child->child->child->data, "guda");
    assert_non_null(trie->root->child->child->child->child->next);
    assert_true(trie->root->child->child->child->child->next->key == 'o');
    assert_null(trie->root->child->child->child->child->next->data);
    assert_true(trie->root->child->child->child->child->next->child->key == 'o');
    assert_null(trie->root->child->child->child->child->next->child->data);
    assert_true(trie->root->child->child->child->child->next->child->child->key == '\0');
    assert_string_equal(trie->root->child->child->child->child->next->child->child->data, "asdf");

    assert_int_equal(bc_trie_size(trie), 2);
    assert_string_equal(bc_trie_lookup(trie, "bola"), "guda");
    assert_string_equal(bc_trie_lookup(trie, "bolaoo"), "asdf");

    bc_trie_free(trie);
}


static void
test_shell_quote(void **state)
{
    char *t;
    t = bc_shell_quote(NULL);
    assert_string_equal(t, "''");
    free(t);
    t = bc_shell_quote("!bola");
    assert_string_equal(t, "''\\!'bola'");
    free(t);
    t = bc_shell_quote("'bola");
    assert_string_equal(t, "''\\''bola'");
    free(t);
    t = bc_shell_quote("bo!bola");
    assert_string_equal(t, "'bo'\\!'bola'");
    free(t);
    t = bc_shell_quote("bo'bola");
    assert_string_equal(t, "'bo'\\''bola'");
    free(t);
    t = bc_shell_quote("bola!");
    assert_string_equal(t, "'bola'\\!''");
    free(t);
    t = bc_shell_quote("bola'");
    assert_string_equal(t, "'bola'\\'''");
    free(t);
}


int
main(void)
{
    const UnitTest tests[] = {

        // slist
        unit_test(test_slist_append),
        unit_test(test_slist_prepend),
        unit_test(test_slist_remove),
        unit_test(test_slist_free),
        unit_test(test_slist_length),

        // strfuncs
        unit_test(test_strdup),
        unit_test(test_strndup),
        unit_test(test_strdup_printf),
        unit_test(test_str_starts_with),
        unit_test(test_str_ends_with),
        unit_test(test_str_lstrip),
        unit_test(test_str_rstrip),
        unit_test(test_str_strip),
        unit_test(test_str_split),
        unit_test(test_str_replace),
        unit_test(test_str_find),
        unit_test(test_str_to_bool),
        unit_test(test_strv_join),
        unit_test(test_strv_length),

        // string
        unit_test(test_string_new),
        unit_test(test_string_free),
        unit_test(test_string_dup),
        unit_test(test_string_append_len),
        unit_test(test_string_append),
        unit_test(test_string_append_c),
        unit_test(test_string_append_printf),
        unit_test(test_string_append_escaped),

        // trie
        unit_test(test_trie_new),
        unit_test(test_trie_insert),
        unit_test(test_trie_insert_duplicated),
        unit_test(test_trie_keep_data),
        unit_test(test_trie_lookup),
        unit_test(test_trie_size),
        unit_test(test_trie_foreach),
        unit_test(test_trie_inserted_after_prefix),

        // shell
        unit_test(test_shell_quote),
    };
    return run_tests(tests);
}
