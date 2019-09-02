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
#include <stdlib.h>
#include <locale.h>
#include <squareball.h>

#include "../../src/blogc/datetime-parser.h"


static void
test_convert_datetime(void **state)
{
    sb_error_t *err = NULL;
    char *dt = blogc_convert_datetime("2010-11-30 12:13:14",
        "%b %d, %Y, %I:%M:%S %p GMT", &err);
    assert_null(err);
    assert_string_equal(dt, "Nov 30, 2010, 12:13:14 PM GMT");
    free(dt);
}


static void
test_convert_datetime_implicit_seconds(void **state)
{
    sb_error_t *err = NULL;
    char *dt = blogc_convert_datetime("2010-11-30 12:13",
        "%b %d, %Y, %I:%M:%S %p GMT", &err);
    assert_null(err);
    assert_string_equal(dt, "Nov 30, 2010, 12:13:00 PM GMT");
    free(dt);
}


static void
test_convert_datetime_implicit_minutes(void **state)
{
    sb_error_t *err = NULL;
    char *dt = blogc_convert_datetime("2010-11-30 12",
        "%b %d, %Y, %I:%M:%S %p GMT", &err);
    assert_null(err);
    assert_string_equal(dt, "Nov 30, 2010, 12:00:00 PM GMT");
    free(dt);
}


static void
test_convert_datetime_implicit_hours(void **state)
{
    sb_error_t *err = NULL;
    char *dt = blogc_convert_datetime("2010-11-30",
        "%b %d, %Y, %I:%M:%S %p GMT", &err);
    assert_null(err);
    assert_string_equal(dt, "Nov 30, 2010, 12:00:00 AM GMT");
    free(dt);
}


static void
test_convert_datetime_invalid_formats(void **state)
{
    sb_error_t *err = NULL;
    char *dt = blogc_convert_datetime("", "%b %d, %Y, %I:%M:%S %p GMT", &err);
    assert_null(dt);
    assert_non_null(err);
    assert_string_equal(sb_error_to_string(err),
        "datetime: Invalid datetime string. Found '', formats allowed are: "
        "'yyyy-mm-dd hh:mm:ss', 'yyyy-mm-dd hh:ss', 'yyyy-mm-dd hh' and "
        "'yyyy-mm-dd'.");
    sb_error_free(err);

    err = NULL;
    dt = blogc_convert_datetime("2", "%b %d, %Y, %I:%M:%S %p GMT", &err);
    assert_null(dt);
    assert_non_null(err);
    assert_string_equal(sb_error_to_string(err),
        "datetime: Invalid datetime string. Found '2', formats allowed are: "
        "'yyyy-mm-dd hh:mm:ss', 'yyyy-mm-dd hh:ss', 'yyyy-mm-dd hh' and "
        "'yyyy-mm-dd'.");
    sb_error_free(err);

    err = NULL;
    dt = blogc_convert_datetime("20", "%b %d, %Y, %I:%M:%S %p GMT", &err);
    assert_null(dt);
    assert_non_null(err);
    assert_string_equal(sb_error_to_string(err),
        "datetime: Invalid datetime string. Found '20', formats allowed are: "
        "'yyyy-mm-dd hh:mm:ss', 'yyyy-mm-dd hh:ss', 'yyyy-mm-dd hh' and "
        "'yyyy-mm-dd'.");
    sb_error_free(err);

    err = NULL;
    dt = blogc_convert_datetime("201", "%b %d, %Y, %I:%M:%S %p GMT", &err);
    assert_null(dt);
    assert_non_null(err);
    assert_string_equal(sb_error_to_string(err),
        "datetime: Invalid datetime string. Found '201', formats allowed are: "
        "'yyyy-mm-dd hh:mm:ss', 'yyyy-mm-dd hh:ss', 'yyyy-mm-dd hh' and "
        "'yyyy-mm-dd'.");
    sb_error_free(err);

    err = NULL;
    dt = blogc_convert_datetime("2010", "%b %d, %Y, %I:%M:%S %p GMT", &err);
    assert_null(dt);
    assert_non_null(err);
    assert_string_equal(sb_error_to_string(err),
        "datetime: Invalid datetime string. Found '2010', formats allowed are: "
        "'yyyy-mm-dd hh:mm:ss', 'yyyy-mm-dd hh:ss', 'yyyy-mm-dd hh' and "
        "'yyyy-mm-dd'.");
    sb_error_free(err);

    err = NULL;
    dt = blogc_convert_datetime("2010-", "%b %d, %Y, %I:%M:%S %p GMT", &err);
    assert_null(dt);
    assert_non_null(err);
    assert_string_equal(sb_error_to_string(err),
        "datetime: Invalid datetime string. Found '2010-', formats allowed are: "
        "'yyyy-mm-dd hh:mm:ss', 'yyyy-mm-dd hh:ss', 'yyyy-mm-dd hh' and "
        "'yyyy-mm-dd'.");
    sb_error_free(err);

    err = NULL;
    dt = blogc_convert_datetime("2010-1", "%b %d, %Y, %I:%M:%S %p GMT", &err);
    assert_null(dt);
    assert_non_null(err);
    assert_string_equal(sb_error_to_string(err),
        "datetime: Invalid datetime string. Found '2010-1', formats allowed are: "
        "'yyyy-mm-dd hh:mm:ss', 'yyyy-mm-dd hh:ss', 'yyyy-mm-dd hh' and "
        "'yyyy-mm-dd'.");
    sb_error_free(err);

    err = NULL;
    dt = blogc_convert_datetime("2010-11", "%b %d, %Y, %I:%M:%S %p GMT", &err);
    assert_null(dt);
    assert_non_null(err);
    assert_string_equal(sb_error_to_string(err),
        "datetime: Invalid datetime string. Found '2010-11', formats allowed are: "
        "'yyyy-mm-dd hh:mm:ss', 'yyyy-mm-dd hh:ss', 'yyyy-mm-dd hh' and "
        "'yyyy-mm-dd'.");
    sb_error_free(err);

    err = NULL;
    dt = blogc_convert_datetime("2010-11-", "%b %d, %Y, %I:%M:%S %p GMT", &err);
    assert_null(dt);
    assert_non_null(err);
    assert_string_equal(sb_error_to_string(err),
        "datetime: Invalid datetime string. Found '2010-11-', formats allowed are: "
        "'yyyy-mm-dd hh:mm:ss', 'yyyy-mm-dd hh:ss', 'yyyy-mm-dd hh' and "
        "'yyyy-mm-dd'.");
    sb_error_free(err);

    err = NULL;
    dt = blogc_convert_datetime("2010-11-3", "%b %d, %Y, %I:%M:%S %p GMT", &err);
    assert_null(dt);
    assert_non_null(err);
    assert_string_equal(sb_error_to_string(err),
        "datetime: Invalid datetime string. Found '2010-11-3', formats allowed are: "
        "'yyyy-mm-dd hh:mm:ss', 'yyyy-mm-dd hh:ss', 'yyyy-mm-dd hh' and "
        "'yyyy-mm-dd'.");
    sb_error_free(err);

    err = NULL;
    dt = blogc_convert_datetime("2010-11-30 ", "%b %d, %Y, %I:%M:%S %p GMT", &err);
    assert_null(dt);
    assert_non_null(err);
    assert_string_equal(sb_error_to_string(err),
        "datetime: Invalid datetime string. Found '2010-11-30 ', formats allowed are: "
        "'yyyy-mm-dd hh:mm:ss', 'yyyy-mm-dd hh:ss', 'yyyy-mm-dd hh' and "
        "'yyyy-mm-dd'.");
    sb_error_free(err);

    err = NULL;
    dt = blogc_convert_datetime("2010-11-30 1", "%b %d, %Y, %I:%M:%S %p GMT", &err);
    assert_null(dt);
    assert_non_null(err);
    assert_string_equal(sb_error_to_string(err),
        "datetime: Invalid datetime string. Found '2010-11-30 1', formats allowed are: "
        "'yyyy-mm-dd hh:mm:ss', 'yyyy-mm-dd hh:ss', 'yyyy-mm-dd hh' and "
        "'yyyy-mm-dd'.");
    sb_error_free(err);

    err = NULL;
    dt = blogc_convert_datetime("2010-11-30 12:1", "%b %d, %Y, %I:%M:%S %p GMT",
        &err);
    assert_null(dt);
    assert_non_null(err);
    assert_string_equal(sb_error_to_string(err),
        "datetime: Invalid datetime string. Found '2010-11-30 12:1', formats ""allowed are: "
        "'yyyy-mm-dd hh:mm:ss', 'yyyy-mm-dd hh:ss', 'yyyy-mm-dd hh' and "
        "'yyyy-mm-dd'.");
    sb_error_free(err);

    err = NULL;
    dt = blogc_convert_datetime("2010-11-30 12:13:1", "%b %d, %Y, %I:%M:%S %p GMT",
        &err);
    assert_null(dt);
    assert_non_null(err);
    assert_string_equal(sb_error_to_string(err),
        "datetime: Invalid datetime string. Found '2010-11-30 12:13:1', formats allowed are: "
        "'yyyy-mm-dd hh:mm:ss', 'yyyy-mm-dd hh:ss', 'yyyy-mm-dd hh' and "
        "'yyyy-mm-dd'.");
    sb_error_free(err);
}


static void
test_convert_datetime_invalid_1st_year(void **state)
{
    sb_error_t *err = NULL;
    char *dt = blogc_convert_datetime("a010-11-30 12:13:14",
        "%b %d, %Y, %I:%M:%S %p GMT", &err);
    assert_null(dt);
    assert_non_null(err);
    assert_string_equal(sb_error_to_string(err),
        "datetime: Invalid first digit of year. Found 'a', must be integer >= 0 and <= 9.");
    sb_error_free(err);
}


static void
test_convert_datetime_invalid_2nd_year(void **state)
{
    sb_error_t *err = NULL;
    char *dt = blogc_convert_datetime("2a10-11-30 12:13:14",
        "%b %d, %Y, %I:%M:%S %p GMT", &err);
    assert_null(dt);
    assert_non_null(err);
    assert_string_equal(sb_error_to_string(err),
        "datetime: Invalid second digit of year. Found 'a', must be integer >= 0 and <= 9.");
    sb_error_free(err);
}


static void
test_convert_datetime_invalid_3rd_year(void **state)
{
    sb_error_t *err = NULL;
    char *dt = blogc_convert_datetime("20a0-11-30 12:13:14",
        "%b %d, %Y, %I:%M:%S %p GMT", &err);
    assert_null(dt);
    assert_non_null(err);
    assert_string_equal(sb_error_to_string(err),
        "datetime: Invalid third digit of year. Found 'a', must be integer >= 0 and <= 9.");
    sb_error_free(err);
}


static void
test_convert_datetime_invalid_4th_year(void **state)
{
    sb_error_t *err = NULL;
    char *dt = blogc_convert_datetime("201a-11-30 12:13:14",
        "%b %d, %Y, %I:%M:%S %p GMT", &err);
    assert_null(dt);
    assert_non_null(err);
    assert_string_equal(sb_error_to_string(err),
        "datetime: Invalid fourth digit of year. Found 'a', must be integer >= 0 and <= 9.");
    sb_error_free(err);
}


static void
test_convert_datetime_invalid_year(void **state)
{
    sb_error_t *err = NULL;
    char *dt = blogc_convert_datetime("1899-11-30 12:13:14",
        "%b %d, %Y, %I:%M:%S %p GMT", &err);
    assert_null(dt);
    assert_non_null(err);
    assert_string_equal(sb_error_to_string(err),
        "datetime: Invalid year. Found 1899, must be >= 1900.");
    sb_error_free(err);
}


static void
test_convert_datetime_invalid_1st_hyphen(void **state)
{
    sb_error_t *err = NULL;
    char *dt = blogc_convert_datetime("2010 11-30 12:13:14",
        "%b %d, %Y, %I:%M:%S %p GMT", &err);
    assert_null(dt);
    assert_non_null(err);
    assert_string_equal(sb_error_to_string(err),
        "datetime: Invalid separator between year and month. Found ' ', must be '-'.");
    sb_error_free(err);
}


static void
test_convert_datetime_invalid_1st_month(void **state)
{
    sb_error_t *err = NULL;
    char *dt = blogc_convert_datetime("2010-a1-30 12:13:14",
        "%b %d, %Y, %I:%M:%S %p GMT", &err);
    assert_null(dt);
    assert_non_null(err);
    assert_string_equal(sb_error_to_string(err),
        "datetime: Invalid first digit of month. Found 'a', must be integer >= 0 and <= 1.");
    sb_error_free(err);
}


static void
test_convert_datetime_invalid_2nd_month(void **state)
{
    sb_error_t *err = NULL;
    char *dt = blogc_convert_datetime("2010-1a-30 12:13:14",
        "%b %d, %Y, %I:%M:%S %p GMT", &err);
    assert_null(dt);
    assert_non_null(err);
    assert_string_equal(sb_error_to_string(err),
        "datetime: Invalid second digit of month. Found 'a', must be integer >= 0 and <= 9.");
    sb_error_free(err);
}


static void
test_convert_datetime_invalid_month(void **state)
{
    sb_error_t *err = NULL;
    char *dt = blogc_convert_datetime("2010-13-30 12:13:14",
        "%b %d, %Y, %I:%M:%S %p GMT", &err);
    assert_null(dt);
    assert_non_null(err);
    assert_string_equal(sb_error_to_string(err),
        "datetime: Invalid month. Found 13, must be >= 1 and <= 12.");
    sb_error_free(err);
}


static void
test_convert_datetime_invalid_2nd_hyphen(void **state)
{
    sb_error_t *err = NULL;
    char *dt = blogc_convert_datetime("2010-11 30 12:13:14",
        "%b %d, %Y, %I:%M:%S %p GMT", &err);
    assert_null(dt);
    assert_non_null(err);
    assert_string_equal(sb_error_to_string(err),
        "datetime: Invalid separator between month and day. Found ' ', must be '-'.");
    sb_error_free(err);
}


static void
test_convert_datetime_invalid_1st_day(void **state)
{
    sb_error_t *err = NULL;
    char *dt = blogc_convert_datetime("2010-11-a0 12:13:14",
        "%b %d, %Y, %I:%M:%S %p GMT", &err);
    assert_null(dt);
    assert_non_null(err);
    assert_string_equal(sb_error_to_string(err),
        "datetime: Invalid first digit of day. Found 'a', must be integer >= 0 and <= 3.");
    sb_error_free(err);
}


static void
test_convert_datetime_invalid_2nd_day(void **state)
{
    sb_error_t *err = NULL;
    char *dt = blogc_convert_datetime("2010-11-3a 12:13:14",
        "%b %d, %Y, %I:%M:%S %p GMT", &err);
    assert_null(dt);
    assert_non_null(err);
    assert_string_equal(sb_error_to_string(err),
        "datetime: Invalid second digit of day. Found 'a', must be integer >= 0 and <= 9.");
    sb_error_free(err);
}


static void
test_convert_datetime_invalid_day(void **state)
{
    sb_error_t *err = NULL;
    char *dt = blogc_convert_datetime("2010-12-32 12:13:14",
        "%b %d, %Y, %I:%M:%S %p GMT", &err);
    assert_null(dt);
    assert_non_null(err);
    assert_string_equal(sb_error_to_string(err),
        "datetime: Invalid day. Found 32, must be >= 1 and <= 31.");
    sb_error_free(err);
}


static void
test_convert_datetime_invalid_space(void **state)
{
    sb_error_t *err = NULL;
    char *dt = blogc_convert_datetime("2010-11-30-12:13:14",
        "%b %d, %Y, %I:%M:%S %p GMT", &err);
    assert_null(dt);
    assert_non_null(err);
    assert_string_equal(sb_error_to_string(err),
        "datetime: Invalid separator between date and time. Found '-', must be ' ' "
        "(empty space).");
    sb_error_free(err);
}


static void
test_convert_datetime_invalid_1st_hours(void **state)
{
    sb_error_t *err = NULL;
    char *dt = blogc_convert_datetime("2010-11-30 a2:13:14",
        "%b %d, %Y, %I:%M:%S %p GMT", &err);
    assert_null(dt);
    assert_non_null(err);
    assert_string_equal(sb_error_to_string(err),
        "datetime: Invalid first digit of hours. Found 'a', must be integer >= 0 and <= 2.");
    sb_error_free(err);
}


static void
test_convert_datetime_invalid_2nd_hours(void **state)
{
    sb_error_t *err = NULL;
    char *dt = blogc_convert_datetime("2010-11-30 1a:13:14",
        "%b %d, %Y, %I:%M:%S %p GMT", &err);
    assert_null(dt);
    assert_non_null(err);
    assert_string_equal(sb_error_to_string(err),
        "datetime: Invalid second digit of hours. Found 'a', must be integer >= 0 and <= 9.");
    sb_error_free(err);
}


static void
test_convert_datetime_invalid_hours(void **state)
{
    sb_error_t *err = NULL;
    char *dt = blogc_convert_datetime("2010-12-30 24:13:14",
        "%b %d, %Y, %I:%M:%S %p GMT", &err);
    assert_null(dt);
    assert_non_null(err);
    assert_string_equal(sb_error_to_string(err),
        "datetime: Invalid hours. Found 24, must be >= 0 and <= 23.");
    sb_error_free(err);
}


static void
test_convert_datetime_invalid_1st_colon(void **state)
{
    sb_error_t *err = NULL;
    char *dt = blogc_convert_datetime("2010-11-30 12 13:14",
        "%b %d, %Y, %I:%M:%S %p GMT", &err);
    assert_null(dt);
    assert_non_null(err);
    assert_string_equal(sb_error_to_string(err),
        "datetime: Invalid separator between hours and minutes. Found ' ', must be ':'.");
    sb_error_free(err);
}


static void
test_convert_datetime_invalid_1st_minutes(void **state)
{
    sb_error_t *err = NULL;
    char *dt = blogc_convert_datetime("2010-11-30 12:a3:14",
        "%b %d, %Y, %I:%M:%S %p GMT", &err);
    assert_null(dt);
    assert_non_null(err);
    assert_string_equal(sb_error_to_string(err),
        "datetime: Invalid first digit of minutes. Found 'a', must be integer >= 0 and <= 5.");
    sb_error_free(err);
}


static void
test_convert_datetime_invalid_2nd_minutes(void **state)
{
    sb_error_t *err = NULL;
    char *dt = blogc_convert_datetime("2010-11-30 12:1a:14",
        "%b %d, %Y, %I:%M:%S %p GMT", &err);
    assert_null(dt);
    assert_non_null(err);
    assert_string_equal(sb_error_to_string(err),
        "datetime: Invalid second digit of minutes. Found 'a', must be integer >= 0 and <= 9.");
    sb_error_free(err);
}


static void
test_convert_datetime_invalid_2nd_colon(void **state)
{
    sb_error_t *err = NULL;
    char *dt = blogc_convert_datetime("2010-11-30 12:13 14",
        "%b %d, %Y, %I:%M:%S %p GMT", &err);
    assert_null(dt);
    assert_non_null(err);
    assert_string_equal(sb_error_to_string(err),
        "datetime: Invalid separator between minutes and seconds. Found ' ', must be ':'.");
    sb_error_free(err);
}


static void
test_convert_datetime_invalid_1st_seconds(void **state)
{
    sb_error_t *err = NULL;
    char *dt = blogc_convert_datetime("2010-11-30 12:13:a4",
        "%b %d, %Y, %I:%M:%S %p GMT", &err);
    assert_null(dt);
    assert_non_null(err);
    assert_string_equal(sb_error_to_string(err),
        "datetime: Invalid first digit of seconds. Found 'a', must be integer >= 0 and <= 6.");
    sb_error_free(err);
}


static void
test_convert_datetime_invalid_2nd_seconds(void **state)
{
    sb_error_t *err = NULL;
    char *dt = blogc_convert_datetime("2010-11-30 12:13:1a",
        "%b %d, %Y, %I:%M:%S %p GMT", &err);
    assert_null(dt);
    assert_non_null(err);
    assert_string_equal(sb_error_to_string(err),
        "datetime: Invalid second digit of seconds. Found 'a', must be integer >= 0 and <= 9.");
    sb_error_free(err);
}


static void
test_convert_datetime_invalid_seconds(void **state)
{
    sb_error_t *err = NULL;
    char *dt = blogc_convert_datetime("2010-12-30 12:13:69",
        "%b %d, %Y, %I:%M:%S %p GMT", &err);
    assert_null(dt);
    assert_non_null(err);
    assert_string_equal(sb_error_to_string(err),
        "datetime: Invalid seconds. Found 69, must be >= 0 and <= 60.");
    sb_error_free(err);
}


static void
test_convert_datetime_invalid_format_long(void **state)
{
    sb_error_t *err = NULL;
    char *dt = blogc_convert_datetime("2010-12-30 12:13:14",
        "bovhsuhxwybfrxoluiejaoqpmoylgvkrjtnuntmcgtupwabexkapnklvkwmddmplfqopvb"
        "yjsiimtfdeveeeayqvvnthimbqotumngxxenurxhsvyaftwsfdtxqnjluvtcwfkomfffrk"
        "tywccrvnraagtnedwdjtfobinobbymanppwqxubxeepotdyxuvircyshpmtrqyvbivtycs"
        "olwvqwdqaswdafohqkthraenpueuywbocrsbmmfoqwgbeixosyjljamcqwecfoxgolyxif"
        "ltaoamuirnnsvoqcnboueqnnyksawwrdtcsiklanjxeavlaqsaswacmbvmselsnghiviet"
        "wrftrfimjshrjlwxdhjkktivwmmesihlxkmqpmvfqjuimbmaucdxaqcgjdacgksqdseqko"
        "prknyjylchdlijfgktveldlewixwrycytrjxxesrcbraydmbgitkldbxxhnjwqdmcwctat"
        "rtjvrboulykkpvmsthontrunvkylwanwnnbpgwiwrgctfsvfgtxpifmpxhwikcylfeycjl"
        "scmsjnvwfhlkwevcmvvoypmfqlnrwywkwvinkwbjpgxpdxfckghutcovrdhlatumhfvowb"
        "fyiowyqpsbqhdxxauflpteyagsjtbulpktxmhkxxbgpetlwnckwsvhgmtasviemjeatejs"
        "tslaivqeltycdgqylhqadxnrdlldbqdwuabdsrqwlxmetahvkrlvmyfgfftrlujfgktwve"
        "vwidoqvigelfaohgjtaplygmmiwrcspaqntfhthikewunxhebqbkwiopplcmywvjeehslw"
        "uaeruwnphdjonqagjatjladqhvlxppyaqgvwpjqggnsccmkjvbxqykaejvgeajqpitkwsq"
        "gmjiaopomnnlewidhgbgqlblotrnuyokspuvbckqhwnhmgcwyyitmlelnehdvclojvyswj"
        "jgipsincitulscikxviaruryfraeqssykeftcphtndlfhdxokg", &err);
    assert_null(dt);
    assert_non_null(err);
    assert_string_equal(sb_error_to_string(err),
        "datetime: Failed to format DATE variable, FORMAT is too long: "
        "bovhsuhxwybfrxoluiejaoqpmoylgvkrjtnuntmcgtupwabexkapnklvkwmddmplfqopvb"
        "yjsiimtfdeveeeayqvvnthimbqotumngxxenurxhsvyaftwsfdtxqnjluvtcwfkomfffrk"
        "tywccrvnraagtnedwdjtfobinobbymanppwqxubxeepotdyxuvircyshpmtrqyvbivtycs"
        "olwvqwdqaswdafohqkthraenpueuywbocrsbmmfoqwgbeixosyjljamcqwecfoxgolyxif"
        "ltaoamuirnnsvoqcnboueqnnyksawwrdtcsiklanjxeavlaqsaswacmbvmselsnghiviet"
        "wrftrfimjshrjlwxdhjkktivwmmesihlxkmqpmvfqjuimbmaucdxaqcgjdacgksqdseqko"
        "prknyjylchdlijfgktveldlewixwrycytrjxxesrcbraydmbgitkldbxxhnjwqdmcwctat"
        "rtjvrboulykkpvmsthontrunvkylwanwnnbpgwiwrgctfsvfgtxpifmpxhwikcylfeycjl"
        "scmsjnvwfhlkwevcmvvoypmfqlnrwywkwvinkwbjpgxpdxfckghutcovrdhlatumhfvowb"
        "fyiowyqpsbqhdxxauflpteyagsjtbulpktxmhkxxbgpetlwnckwsvhgmtasviemjeatejs"
        "tslaivqeltycdgqylhqadxnrdlldbqdwuabdsrqwlxmetahvkrlvmyfgfftrlujfgktwve"
        "vwidoqvigelfaohgjtaplygmmiwrcspaqntfhthikewunxhebqbkwiopplcmywvjeehslw"
        "uaeruwnphdjonqagjatjladqhvlxppyaqgvwpjqggnsccmkjvbxqykaejvgeajqpitkwsq"
        "gmjiaopomnnlewidhgbgqlblotrnuyokspuvbckqhwnhmgcwyyitmlelnehdvclojvyswj"
        "jgipsincitulscikxviaruryfraeqssykeftcphtndlfhdxokg");
    sb_error_free(err);
}


int
main(void)
{
    setlocale(LC_ALL, "C");
    const UnitTest tests[] = {
        unit_test(test_convert_datetime),
        unit_test(test_convert_datetime_implicit_seconds),
        unit_test(test_convert_datetime_implicit_minutes),
        unit_test(test_convert_datetime_implicit_hours),
        unit_test(test_convert_datetime_invalid_formats),
        unit_test(test_convert_datetime_invalid_1st_year),
        unit_test(test_convert_datetime_invalid_2nd_year),
        unit_test(test_convert_datetime_invalid_3rd_year),
        unit_test(test_convert_datetime_invalid_4th_year),
        unit_test(test_convert_datetime_invalid_year),
        unit_test(test_convert_datetime_invalid_1st_hyphen),
        unit_test(test_convert_datetime_invalid_1st_month),
        unit_test(test_convert_datetime_invalid_2nd_month),
        unit_test(test_convert_datetime_invalid_month),
        unit_test(test_convert_datetime_invalid_2nd_hyphen),
        unit_test(test_convert_datetime_invalid_1st_day),
        unit_test(test_convert_datetime_invalid_2nd_day),
        unit_test(test_convert_datetime_invalid_day),
        unit_test(test_convert_datetime_invalid_space),
        unit_test(test_convert_datetime_invalid_1st_hours),
        unit_test(test_convert_datetime_invalid_2nd_hours),
        unit_test(test_convert_datetime_invalid_hours),
        unit_test(test_convert_datetime_invalid_1st_colon),
        unit_test(test_convert_datetime_invalid_1st_minutes),
        unit_test(test_convert_datetime_invalid_2nd_minutes),
        //unit_test(test_convert_datetime_invalid_minutes),  // not possible
        unit_test(test_convert_datetime_invalid_2nd_colon),
        unit_test(test_convert_datetime_invalid_1st_seconds),
        unit_test(test_convert_datetime_invalid_2nd_seconds),
        unit_test(test_convert_datetime_invalid_seconds),
        unit_test(test_convert_datetime_invalid_format_long),
    };
    return run_tests(tests);
}
