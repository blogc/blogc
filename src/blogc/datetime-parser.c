/*
 * blogc: A blog compiler.
 * Copyright (C) 2014-2019 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file LICENSE.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#ifdef HAVE_TIME_H
#include <time.h>
#endif /* HAVE_TIME_H */

#include <string.h>
#include <squareball.h>

#include "datetime-parser.h"


typedef enum {
    DATETIME_FIRST_YEAR = 1,
    DATETIME_SECOND_YEAR,
    DATETIME_THIRD_YEAR,
    DATETIME_FOURTH_YEAR,
    DATETIME_FIRST_HYPHEN,
    DATETIME_FIRST_MONTH,
    DATETIME_SECOND_MONTH,
    DATETIME_SECOND_HYPHEN,
    DATETIME_FIRST_DAY,
    DATETIME_SECOND_DAY,
    DATETIME_SPACE,
    DATETIME_FIRST_HOUR,
    DATETIME_SECOND_HOUR,
    DATETIME_FIRST_COLON,
    DATETIME_FIRST_MINUTE,
    DATETIME_SECOND_MINUTE,
    DATETIME_SECOND_COLON,
    DATETIME_FIRST_SECOND,
    DATETIME_SECOND_SECOND,
    DATETIME_DONE,
} blogc_datetime_state_t;


char*
blogc_convert_datetime(const char *orig, const char *format,
    sb_error_t **err)
{
    if (err == NULL || *err != NULL)
        return NULL;

#ifndef HAVE_TIME_H

    *err = sb_strerror_new(
        "datetime: Your operating system does not supports the datetime "
        "functionalities used by blogc. Sorry.");
    return NULL;

#else

    struct tm t;
    memset(&t, 0, sizeof(struct tm));
    t.tm_isdst = -1;

    blogc_datetime_state_t state = DATETIME_FIRST_YEAR;
    int tmp = 0;
    int diff = '0';

    for (size_t i = 0; orig[i] != '\0'; i++) {
        char c = orig[i];

        switch (state) {

            case DATETIME_FIRST_YEAR:
                if (c >= '0' && c <= '9') {
                    tmp += (c - diff) * 1000;
                    state = DATETIME_SECOND_YEAR;
                    break;
                }
                *err = sb_strerror_new_printf(
                    "datetime: Invalid first digit of year. "
                    "Found '%c', must be integer >= 0 and <= 9.", c);
                break;

            case DATETIME_SECOND_YEAR:
                if (c >= '0' && c <= '9') {
                    tmp += (c - diff) * 100;
                    state = DATETIME_THIRD_YEAR;
                    break;
                }
                *err = sb_strerror_new_printf(
                    "datetime: Invalid second digit of year. "
                    "Found '%c', must be integer >= 0 and <= 9.", c);
                break;

            case DATETIME_THIRD_YEAR:
                if (c >= '0' && c <= '9') {
                    tmp += (c - diff) * 10;
                    state = DATETIME_FOURTH_YEAR;
                    break;
                }
                *err = sb_strerror_new_printf(
                    "datetime: Invalid third digit of year. "
                    "Found '%c', must be integer >= 0 and <= 9.", c);
                break;

            case DATETIME_FOURTH_YEAR:
                if (c >= '0' && c <= '9') {
                    tmp += c - diff - 1900;
                    if (tmp < 0) {
                        *err = sb_strerror_new_printf(
                            "datetime: Invalid year. Found %d, must be >= 1900.",
                            tmp + 1900);
                        break;
                    }
                    t.tm_year = tmp;
                    state = DATETIME_FIRST_HYPHEN;
                    break;
                }
                *err = sb_strerror_new_printf(
                    "datetime: Invalid fourth digit of year. "
                    "Found '%c', must be integer >= 0 and <= 9.", c);
                break;

            case DATETIME_FIRST_HYPHEN:
                if (c == '-') {
                    tmp = 0;
                    state = DATETIME_FIRST_MONTH;
                    break;
                }
                *err = sb_strerror_new_printf(
                    "datetime: Invalid separator between year and month. "
                    "Found '%c', must be '-'.", c);
                break;

            case DATETIME_FIRST_MONTH:
                if (c >= '0' && c <= '1') {
                    tmp += (c - diff) * 10;
                    state = DATETIME_SECOND_MONTH;
                    break;
                }
                *err = sb_strerror_new_printf(
                    "datetime: Invalid first digit of month. "
                    "Found '%c', must be integer >= 0 and <= 1.", c);
                break;

            case DATETIME_SECOND_MONTH:
                if (c >= '0' && c <= '9') {
                    tmp += c - diff - 1;
                    if (tmp < 0 || tmp > 11) {
                        *err = sb_strerror_new_printf(
                            "datetime: Invalid month. "
                            "Found %d, must be >= 1 and <= 12.", tmp + 1);
                        break;
                    }
                    t.tm_mon = tmp;
                    state = DATETIME_SECOND_HYPHEN;
                    break;
                }
                *err = sb_strerror_new_printf(
                    "datetime: Invalid second digit of month. "
                    "Found '%c', must be integer >= 0 and <= 9.", c);
                break;

            case DATETIME_SECOND_HYPHEN:
                if (c == '-') {
                    tmp = 0;
                    state = DATETIME_FIRST_DAY;
                    break;
                }
                *err = sb_strerror_new_printf(
                    "datetime: Invalid separator between month and day. "
                    "Found '%c', must be '-'.", c);
                break;

            case DATETIME_FIRST_DAY:
                if (c >= '0' && c <= '3') {
                    tmp += (c - diff) * 10;
                    state = DATETIME_SECOND_DAY;
                    break;
                }
                *err = sb_strerror_new_printf(
                    "datetime: Invalid first digit of day. "
                    "Found '%c', must be integer >= 0 and <= 3.", c);
                break;

            case DATETIME_SECOND_DAY:
                if (c >= '0' && c <= '9') {
                    tmp += c - diff;
                    if (tmp < 1 || tmp > 31) {
                        *err = sb_strerror_new_printf(
                            "datetime: Invalid day. "
                            "Found %d, must be >= 1 and <= 31.", tmp);
                        break;
                    }
                    t.tm_mday = tmp;
                    state = DATETIME_SPACE;
                    break;
                }
                *err = sb_strerror_new_printf(
                    "datetime: Invalid second digit of day. "
                    "Found '%c', must be integer >= 0 and <= 9.", c);
                break;

            case DATETIME_SPACE:
                if (c == ' ') {
                    tmp = 0;
                    state = DATETIME_FIRST_HOUR;
                    break;
                }
                *err = sb_strerror_new_printf(
                    "datetime: Invalid separator between date and time. "
                    "Found '%c', must be ' ' (empty space).", c);
                break;

            case DATETIME_FIRST_HOUR:
                if (c >= '0' && c <= '2') {
                    tmp += (c - diff) * 10;
                    state = DATETIME_SECOND_HOUR;
                    break;
                }
                *err = sb_strerror_new_printf(
                    "datetime: Invalid first digit of hours. "
                    "Found '%c', must be integer >= 0 and <= 2.", c);
                break;

            case DATETIME_SECOND_HOUR:
                if (c >= '0' && c <= '9') {
                    tmp += c - diff;
                    if (tmp < 0 || tmp > 23) {
                        *err = sb_strerror_new_printf(
                            "datetime: Invalid hours. "
                            "Found %d, must be >= 0 and <= 23.", tmp);
                        break;
                    }
                    t.tm_hour = tmp;
                    state = DATETIME_FIRST_COLON;
                    break;
                }
                *err = sb_strerror_new_printf(
                    "datetime: Invalid second digit of hours. "
                    "Found '%c', must be integer >= 0 and <= 9.", c);
                break;

            case DATETIME_FIRST_COLON:
                if (c == ':') {
                    tmp = 0;
                    state = DATETIME_FIRST_MINUTE;
                    break;
                }
                *err = sb_strerror_new_printf(
                    "datetime: Invalid separator between hours and minutes. "
                    "Found '%c', must be ':'.", c);
                break;

            case DATETIME_FIRST_MINUTE:
                if (c >= '0' && c <= '5') {
                    tmp += (c - diff) * 10;
                    state = DATETIME_SECOND_MINUTE;
                    break;
                }
                *err = sb_strerror_new_printf(
                    "datetime: Invalid first digit of minutes. "
                    "Found '%c', must be integer >= 0 and <= 5.", c);
                break;

            case DATETIME_SECOND_MINUTE:
                if (c >= '0' && c <= '9') {
                    tmp += c - diff;
                    if (tmp < 0 || tmp > 59) {
                        // this won't happen because we are restricting the digits
                        // to 00-59 already, but lets keep the code here for
                        // reference.
                        *err = sb_strerror_new_printf(
                            "datetime: Invalid minutes. "
                            "Found %d, must be >= 0 and <= 59.", tmp);
                        break;
                    }
                    t.tm_min = tmp;
                    state = DATETIME_SECOND_COLON;
                    break;
                }
                *err = sb_strerror_new_printf(
                    "datetime: Invalid second digit of minutes. "
                    "Found '%c', must be integer >= 0 and <= 9.", c);
                break;

            case DATETIME_SECOND_COLON:
                if (c == ':') {
                    tmp = 0;
                    state = DATETIME_FIRST_SECOND;
                    break;
                }
                *err = sb_strerror_new_printf(
                    "datetime: Invalid separator between minutes and seconds. "
                    "Found '%c', must be ':'.", c);
                break;

            case DATETIME_FIRST_SECOND:
                if (c >= '0' && c <= '6') {
                    tmp += (c - diff) * 10;
                    state = DATETIME_SECOND_SECOND;
                    break;
                }
                *err = sb_strerror_new_printf(
                    "datetime: Invalid first digit of seconds. "
                    "Found '%c', must be integer >= 0 and <= 6.", c);
                break;

            case DATETIME_SECOND_SECOND:
                if (c >= '0' && c <= '9') {
                    tmp += c - diff;
                    if (tmp < 0 || tmp > 60) {
                        *err = sb_strerror_new_printf(
                            "datetime: Invalid seconds. "
                            "Found %d, must be >= 0 and <= 60.", tmp);
                        break;
                    }
                    t.tm_sec = tmp;
                    state = DATETIME_DONE;
                    break;
                }
                *err = sb_strerror_new_printf(
                    "datetime: Invalid second digit of seconds. "
                    "Found '%c', must be integer >= 0 and <= 9.", c);
                break;

            case DATETIME_DONE:
                // well, its done ;)
                break;
        }

        if (*err != NULL)
            return NULL;
    }

    if (*err == NULL) {
        switch (state) {
            case DATETIME_FIRST_YEAR:
            case DATETIME_SECOND_YEAR:
            case DATETIME_THIRD_YEAR:
            case DATETIME_FOURTH_YEAR:
            case DATETIME_FIRST_HYPHEN:
            case DATETIME_FIRST_MONTH:
            case DATETIME_SECOND_MONTH:
            case DATETIME_SECOND_HYPHEN:
            case DATETIME_FIRST_DAY:
            case DATETIME_SECOND_DAY:
            case DATETIME_FIRST_HOUR:
            case DATETIME_SECOND_HOUR:
            case DATETIME_FIRST_MINUTE:
            case DATETIME_SECOND_MINUTE:
            case DATETIME_FIRST_SECOND:
            case DATETIME_SECOND_SECOND:
                *err = sb_strerror_new_printf(
                    "datetime: Invalid datetime string. "
                    "Found '%s', formats allowed are: 'yyyy-mm-dd hh:mm:ss', "
                    "'yyyy-mm-dd hh:ss', 'yyyy-mm-dd hh' and 'yyyy-mm-dd'.",
                    orig);
                return NULL;

            case DATETIME_SPACE:
            case DATETIME_FIRST_COLON:
            case DATETIME_SECOND_COLON:
            case DATETIME_DONE:
                break;  // these states are ok
        }
    }

    mktime(&t);

    char buf[1024];
    if (0 == strftime(buf, sizeof(buf), format, &t)) {
        *err = sb_strerror_new_printf(
            "datetime: Failed to format DATE variable, FORMAT is too long: %s",
            format);
        return NULL;
    }

    return sb_strdup(buf);

#endif
}
