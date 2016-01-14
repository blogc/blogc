/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2016 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <stdlib.h>

#include "../src/utils/utils.h"


static void
test_slist_append(void **state)
{
    b_slist_t *l = NULL;
    l = b_slist_append(l, (void*) b_strdup("bola"));
    assert_non_null(l);
    assert_string_equal(l->data, "bola");
    assert_null(l->next);
    l = b_slist_append(l, (void*) b_strdup("guda"));
    assert_non_null(l);
    assert_string_equal(l->data, "bola");
    assert_non_null(l->next);
    assert_string_equal(l->next->data, "guda");
    assert_null(l->next->next);
    b_slist_free_full(l, free);
}


static void
test_slist_free(void **state)
{
    b_slist_t *l = NULL;
    char *t1 = b_strdup("bola");
    char *t2 = b_strdup("guda");
    char *t3 = b_strdup("chunda");
    l = b_slist_append(l, (void*) t1);
    l = b_slist_append(l, (void*) t2);
    l = b_slist_append(l, (void*) t3);
    b_slist_free(l);
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
    b_slist_t *l = NULL;
    l = b_slist_append(l, (void*) b_strdup("bola"));
    l = b_slist_append(l, (void*) b_strdup("guda"));
    l = b_slist_append(l, (void*) b_strdup("chunda"));
    assert_int_equal(b_slist_length(l), 3);
    b_slist_free_full(l, free);
}


static void
test_strdup(void **state)
{
    char *str = b_strdup("bola");
    assert_string_equal(str, "bola");
    free(str);
    str = b_strdup(NULL);
    assert_null(str);
}


static void
test_strndup(void **state)
{
    char *str = b_strndup("bolaguda", 4);
    assert_string_equal(str, "bola");
    free(str);
    str = b_strndup("bolaguda", 30);
    assert_string_equal(str, "bolaguda");
    free(str);
    str = b_strndup("bolaguda", 8);
    assert_string_equal(str, "bolaguda");
    free(str);
    str = b_strdup(NULL);
    assert_null(str);
}


static void
test_strdup_printf(void **state)
{
    char *str = b_strdup_printf("bola");
    assert_string_equal(str, "bola");
    free(str);
    str = b_strdup_printf("bola, %s", "guda");
    assert_string_equal(str, "bola, guda");
    free(str);
}


static void
test_str_starts_with(void **state)
{
    assert_true(b_str_starts_with("bolaguda", "bola"));
    assert_true(b_str_starts_with("bola", "bola"));
    assert_false(b_str_starts_with("gudabola", "bola"));
    assert_false(b_str_starts_with("guda", "bola"));
    assert_false(b_str_starts_with("bola", "bolaguda"));
}


static void
test_str_ends_with(void **state)
{
    assert_true(b_str_ends_with("bolaguda", "guda"));
    assert_true(b_str_ends_with("bola", "bola"));
    assert_false(b_str_ends_with("gudabola", "guda"));
    assert_false(b_str_ends_with("guda", "bola"));
    assert_false(b_str_ends_with("bola", "gudabola"));
}


static void
test_str_lstrip(void **state)
{
    char *str = b_strdup("  \tbola\n  \t");
    assert_string_equal(b_str_lstrip(str), "bola\n  \t");
    free(str);
    str = b_strdup("guda");
    assert_string_equal(b_str_lstrip(str), "guda");
    free(str);
    str = b_strdup("\n");
    assert_string_equal(b_str_lstrip(str), "");
    free(str);
    str = b_strdup("\t \n");
    assert_string_equal(b_str_lstrip(str), "");
    free(str);
    str = b_strdup("");
    assert_string_equal(b_str_lstrip(str), "");
    free(str);
    assert_null(b_str_lstrip(NULL));
}


static void
test_str_rstrip(void **state)
{
    char *str = b_strdup("  \tbola\n  \t");
    assert_string_equal(b_str_rstrip(str), "  \tbola");
    free(str);
    str = b_strdup("guda");
    assert_string_equal(b_str_rstrip(str), "guda");
    free(str);
    str = b_strdup("\n");
    assert_string_equal(b_str_rstrip(str), "");
    free(str);
    str = b_strdup("\t \n");
    assert_string_equal(b_str_rstrip(str), "");
    free(str);
    str = b_strdup("");
    assert_string_equal(b_str_rstrip(str), "");
    free(str);
    assert_null(b_str_rstrip(NULL));
}


static void
test_str_strip(void **state)
{
    char *str = b_strdup("  \tbola\n  \t");
    assert_string_equal(b_str_strip(str), "bola");
    free(str);
    str = b_strdup("guda");
    assert_string_equal(b_str_strip(str), "guda");
    free(str);
    str = b_strdup("\n");
    assert_string_equal(b_str_strip(str), "");
    free(str);
    str = b_strdup("\t \n");
    assert_string_equal(b_str_strip(str), "");
    free(str);
    str = b_strdup("");
    assert_string_equal(b_str_strip(str), "");
    free(str);
    assert_null(b_str_strip(NULL));
}


static void
test_str_split(void **state)
{
    char **strv = b_str_split("bola:guda:chunda", ':', 0);
    assert_string_equal(strv[0], "bola");
    assert_string_equal(strv[1], "guda");
    assert_string_equal(strv[2], "chunda");
    assert_null(strv[3]);
    b_strv_free(strv);
    strv = b_str_split("bola:guda:chunda", ':', 2);
    assert_string_equal(strv[0], "bola");
    assert_string_equal(strv[1], "guda:chunda");
    assert_null(strv[2]);
    b_strv_free(strv);
    strv = b_str_split("bola:guda:chunda", ':', 1);
    assert_string_equal(strv[0], "bola:guda:chunda");
    assert_null(strv[1]);
    b_strv_free(strv);
    strv = b_str_split("", ':', 1);
    assert_null(strv[0]);
    b_strv_free(strv);
    assert_null(b_str_split(NULL, ':', 0));
}


static void
test_str_replace(void **state)
{
    char *str = b_str_replace("bolao", 'o', "zaz");
    assert_string_equal(str, "bzazlazaz");
    free(str);
    str = b_str_replace("bolao", 'b', "zaz");
    assert_string_equal(str, "zazolao");
    free(str);
}


static void
test_strv_join(void **state)
{
    const char *pieces[] = {"guda","bola", "chunda", NULL};
    char *str = b_strv_join(pieces, ":");
    assert_string_equal(str, "guda:bola:chunda");
    free(str);
    const char *pieces2[] = {NULL};
    str = b_strv_join(pieces2, ":");
    assert_string_equal(str, "");
    free(str);
    assert_null(b_strv_join(NULL, ":"));
}


static void
test_strv_length(void **state)
{
    char *pieces[] = {"guda","bola", "chunda", NULL};
    assert_int_equal(b_strv_length(pieces), 3);
    char *pieces2[] = {NULL};
    assert_int_equal(b_strv_length(pieces2), 0);
    assert_int_equal(b_strv_length(NULL), 0);
}


static void
test_string_new(void **state)
{
    b_string_t *str = b_string_new();
    assert_non_null(str);
    assert_string_equal(str->str, "");
    assert_int_equal(str->len, 0);
    assert_int_equal(str->allocated_len, B_STRING_CHUNK_SIZE);
    assert_null(b_string_free(str, true));
}


static void
test_string_free(void **state)
{
    b_string_t *str = b_string_new();
    free(str->str);
    str->str = b_strdup("bola");
    str->len = 4;
    str->allocated_len = B_STRING_CHUNK_SIZE;
    char *tmp = b_string_free(str, false);
    assert_string_equal(tmp, "bola");
    free(tmp);
}


static void
test_string_append_len(void **state)
{
    b_string_t *str = b_string_new();
    str = b_string_append_len(str, "guda", 4);
    assert_non_null(str);
    assert_string_equal(str->str, "guda");
    assert_int_equal(str->len, 4);
    assert_int_equal(str->allocated_len, B_STRING_CHUNK_SIZE);
    assert_null(b_string_free(str, true));
    str = b_string_new();
    str = b_string_append_len(str, "guda", 4);
    str = b_string_append_len(str, "bola", 4);
    assert_non_null(str);
    assert_string_equal(str->str, "gudabola");
    assert_int_equal(str->len, 8);
    assert_int_equal(str->allocated_len, B_STRING_CHUNK_SIZE);
    assert_null(b_string_free(str, true));
    str = b_string_new();
    str = b_string_append_len(str, "guda", 3);
    str = b_string_append_len(str, "bola", 4);
    assert_non_null(str);
    assert_string_equal(str->str, "gudbola");
    assert_int_equal(str->len, 7);
    assert_int_equal(str->allocated_len, B_STRING_CHUNK_SIZE);
    assert_null(b_string_free(str, true));
    str = b_string_new();
    str = b_string_append_len(str, "guda", 4);
    str = b_string_append_len(str,
        "cwlwmwxxmvjnwtidmjehzdeexbxjnjowruxjrqpgpfhmvwgqeacdjissntmbtsjidzkcw"
        "nnqhxhneolbwqlctcxmrsutolrjikpavxombpfpjyaqltgvzrjidotalcuwrwxtaxjiwa"
        "xfhfyzymtffusoqywaruxpybwggukltspqqmghzpqstvcvlqbkhquihzndnrvkaqvevaz"
        "dxrewtgapkompnviiyielanoyowgqhssntyvcvqqtfjmkphywbkvzfyttaalttywhqcec"
        "hgrwzaglzogwjvqncjzodaqsblcbpcdpxmrtctzginvtkckhqvdplgjvbzrnarcxjrsbc"
        "sbfvpylgjznsuhxcxoqbpxowmsrgwimxjgyzwwmryqvstwzkglgeezelvpvkwefqdatnd"
        "dxntikgoqlidfnmdhxzevqzlzubvyleeksdirmmttqthhkvfjggznpmarcamacpvwsrnr"
        "ftzfeyasjpxoevyptpdnqokswiondusnuymqwaryrmdgscbnuilxtypuynckancsfnwtg"
        "okxhegoifakimxbbafkeannglvsxprqzfekdinssqymtfexf", 600);
    str = b_string_append_len(str, NULL, 0);
    str = b_string_append_len(str,
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
    assert_int_equal(str->allocated_len, B_STRING_CHUNK_SIZE * 10);
    assert_null(b_string_free(str, true));
}


static void
test_string_append(void **state)
{
    b_string_t *str = b_string_new();
    str = b_string_append(str, "guda");
    assert_non_null(str);
    assert_string_equal(str->str, "guda");
    assert_int_equal(str->len, 4);
    assert_int_equal(str->allocated_len, B_STRING_CHUNK_SIZE);
    assert_null(b_string_free(str, true));
    str = b_string_new();
    str = b_string_append(str, "guda");
    str = b_string_append(str, "bola");
    assert_non_null(str);
    assert_string_equal(str->str, "gudabola");
    assert_int_equal(str->len, 8);
    assert_int_equal(str->allocated_len, B_STRING_CHUNK_SIZE);
    assert_null(b_string_free(str, true));
    str = b_string_new();
    str = b_string_append(str, "guda");
    str = b_string_append(str,
        "cwlwmwxxmvjnwtidmjehzdeexbxjnjowruxjrqpgpfhmvwgqeacdjissntmbtsjidzkcw"
        "nnqhxhneolbwqlctcxmrsutolrjikpavxombpfpjyaqltgvzrjidotalcuwrwxtaxjiwa"
        "xfhfyzymtffusoqywaruxpybwggukltspqqmghzpqstvcvlqbkhquihzndnrvkaqvevaz"
        "dxrewtgapkompnviiyielanoyowgqhssntyvcvqqtfjmkphywbkvzfyttaalttywhqcec"
        "hgrwzaglzogwjvqncjzodaqsblcbpcdpxmrtctzginvtkckhqvdplgjvbzrnarcxjrsbc"
        "sbfvpylgjznsuhxcxoqbpxowmsrgwimxjgyzwwmryqvstwzkglgeezelvpvkwefqdatnd"
        "dxntikgoqlidfnmdhxzevqzlzubvyleeksdirmmttqthhkvfjggznpmarcamacpvwsrnr"
        "ftzfeyasjpxoevyptpdnqokswiondusnuymqwaryrmdgscbnuilxtypuynckancsfnwtg"
        "okxhegoifakimxbbafkeannglvsxprqzfekdinssqymtfexf");
    str = b_string_append(str, NULL);
    str = b_string_append(str,
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
    assert_int_equal(str->allocated_len, B_STRING_CHUNK_SIZE * 10);
    assert_null(b_string_free(str, true));
}


static void
test_string_append_c(void **state)
{
    b_string_t *str = b_string_new();
    str = b_string_append_len(str, "guda", 4);
    for (int i = 0; i < 600; i++)
        str = b_string_append_c(str, 'c');
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
    assert_int_equal(str->allocated_len, B_STRING_CHUNK_SIZE * 5);
    assert_null(b_string_free(str, true));
}


static void
test_string_append_printf(void **state)
{
    b_string_t *str = b_string_new();
    str = b_string_append_printf(str, "guda: %s %d", "bola", 1);
    assert_non_null(str);
    assert_string_equal(str->str, "guda: bola 1");
    assert_int_equal(str->len, 12);
    assert_int_equal(str->allocated_len, B_STRING_CHUNK_SIZE);
    assert_null(b_string_free(str, true));
}


static void
test_trie_new(void **state)
{
    b_trie_t *trie = b_trie_new(free);
    assert_non_null(trie);
    assert_null(trie->root);
    assert_true(trie->free_func == free);
    b_trie_free(trie);
}


static void
test_trie_insert(void **state)
{
    b_trie_t *trie = b_trie_new(free);

    b_trie_insert(trie, "bola", b_strdup("guda"));
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


    b_trie_insert(trie, "chu", b_strdup("nda"));
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


    b_trie_insert(trie, "bote", b_strdup("aba"));
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


    b_trie_insert(trie, "bo", b_strdup("haha"));
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

    b_trie_free(trie);


    trie = b_trie_new(free);

    b_trie_insert(trie, "chu", b_strdup("nda"));
    assert_true(trie->root->key == 'c');
    assert_null(trie->root->data);
    assert_true(trie->root->child->key == 'h');
    assert_null(trie->root->child->data);
    assert_true(trie->root->child->child->key == 'u');
    assert_null(trie->root->child->child->data);
    assert_true(trie->root->child->child->child->key == '\0');
    assert_string_equal(trie->root->child->child->child->data, "nda");


    b_trie_insert(trie, "bola", b_strdup("guda"));
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


    b_trie_insert(trie, "bote", b_strdup("aba"));
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


    b_trie_insert(trie, "bo", b_strdup("haha"));
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

    b_trie_free(trie);
}


static void
test_trie_insert_duplicated(void **state)
{
    b_trie_t *trie = b_trie_new(free);

    b_trie_insert(trie, "bola", b_strdup("guda"));
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

    b_trie_insert(trie, "bola", b_strdup("asdf"));
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

    b_trie_free(trie);
}


static void
test_trie_keep_data(void **state)
{
    b_trie_t *trie = b_trie_new(NULL);

    char *t1 = "guda";
    char *t2 = "nda";
    char *t3 = "aba";
    char *t4 = "haha";

    b_trie_insert(trie, "bola", t1);
    b_trie_insert(trie, "chu", t2);
    b_trie_insert(trie, "bote", t3);
    b_trie_insert(trie, "bo", t4);

    b_trie_free(trie);

    assert_string_equal(t1, "guda");
    assert_string_equal(t2, "nda");
    assert_string_equal(t3, "aba");
    assert_string_equal(t4, "haha");
}


static void
test_trie_lookup(void **state)
{
    b_trie_t *trie = b_trie_new(free);

    b_trie_insert(trie, "bola", b_strdup("guda"));
    b_trie_insert(trie, "chu", b_strdup("nda"));
    b_trie_insert(trie, "bote", b_strdup("aba"));
    b_trie_insert(trie, "bo", b_strdup("haha"));

    assert_string_equal(b_trie_lookup(trie, "bola"), "guda");
    assert_string_equal(b_trie_lookup(trie, "chu"), "nda");
    assert_string_equal(b_trie_lookup(trie, "bote"), "aba");
    assert_string_equal(b_trie_lookup(trie, "bo"), "haha");

    assert_null(b_trie_lookup(trie, "arcoiro"));

    b_trie_free(trie);

    trie = b_trie_new(free);

    b_trie_insert(trie, "chu", b_strdup("nda"));
    b_trie_insert(trie, "bola", b_strdup("guda"));
    b_trie_insert(trie, "bote", b_strdup("aba"));
    b_trie_insert(trie, "bo", b_strdup("haha"));
    b_trie_insert(trie, "copa", b_strdup("bu"));
    b_trie_insert(trie, "b", b_strdup("c"));
    b_trie_insert(trie, "test", b_strdup("asd"));

    assert_string_equal(b_trie_lookup(trie, "bola"), "guda");
    assert_string_equal(b_trie_lookup(trie, "chu"), "nda");
    assert_string_equal(b_trie_lookup(trie, "bote"), "aba");
    assert_string_equal(b_trie_lookup(trie, "bo"), "haha");

    assert_null(b_trie_lookup(trie, "arcoiro"));

    b_trie_free(trie);
}


static void
test_trie_size(void **state)
{
    b_trie_t *trie = b_trie_new(free);

    b_trie_insert(trie, "bola", b_strdup("guda"));
    b_trie_insert(trie, "chu", b_strdup("nda"));
    b_trie_insert(trie, "bote", b_strdup("aba"));
    b_trie_insert(trie, "bo", b_strdup("haha"));

    assert_int_equal(b_trie_size(trie), 4);
    assert_int_equal(b_trie_size(NULL), 0);

    b_trie_free(trie);

    trie = b_trie_new(free);

    b_trie_insert(trie, "chu", b_strdup("nda"));
    b_trie_insert(trie, "bola", b_strdup("guda"));
    b_trie_insert(trie, "bote", b_strdup("aba"));
    b_trie_insert(trie, "bo", b_strdup("haha"));
    b_trie_insert(trie, "copa", b_strdup("bu"));
    b_trie_insert(trie, "b", b_strdup("c"));
    b_trie_insert(trie, "test", b_strdup("asd"));

    assert_int_equal(b_trie_size(trie), 7);
    assert_int_equal(b_trie_size(NULL), 0);

    b_trie_free(trie);
}


static unsigned int counter;
static char *expected_keys[] = {"chu", "copa", "bola", "bote", "bo", "b", "test"};
static char *expected_datas[] = {"nda", "bu", "guda", "aba", "haha", "c", "asd"};

static void
mock_foreach(const char *key, void *data)
{
    assert_string_equal(key, expected_keys[counter]);
    assert_string_equal((char*) data, expected_datas[counter++]);
}


static void
test_trie_foreach(void **state)
{
    b_trie_t *trie = b_trie_new(free);

    b_trie_insert(trie, "chu", b_strdup("nda"));
    b_trie_insert(trie, "bola", b_strdup("guda"));
    b_trie_insert(trie, "bote", b_strdup("aba"));
    b_trie_insert(trie, "bo", b_strdup("haha"));
    b_trie_insert(trie, "copa", b_strdup("bu"));
    b_trie_insert(trie, "b", b_strdup("c"));
    b_trie_insert(trie, "test", b_strdup("asd"));

    counter = 0;
    b_trie_foreach(trie, mock_foreach);

    b_trie_free(trie);
}


int
main(void)
{
    const UnitTest tests[] = {

        // slist
        unit_test(test_slist_append),
        unit_test(test_slist_free),
        unit_test(test_slist_length),

        // strings
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
        unit_test(test_strv_join),
        unit_test(test_strv_length),
        unit_test(test_string_new),
        unit_test(test_string_free),
        unit_test(test_string_append_len),
        unit_test(test_string_append),
        unit_test(test_string_append_c),
        unit_test(test_string_append_printf),

        // trie
        unit_test(test_trie_new),
        unit_test(test_trie_insert),
        unit_test(test_trie_insert_duplicated),
        unit_test(test_trie_keep_data),
        unit_test(test_trie_lookup),
        unit_test(test_trie_size),
        unit_test(test_trie_foreach),
    };
    return run_tests(tests);
}
