/*
 * Entry point for threads tests. Tests use libcheck.
 *
 * If you want to debug the exe, you should define env var CK_FORK=no
 * http://check.sourceforge.net/doc/check_html/check_4.html#No-Fork-Mode
 *
 *
 *  Copyright (C) 2016 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <check.h>
#include <stdlib.h>

#include "check_helpers.h"

#include "sopc_time.h"

static const int64_t UNIX_EPOCH_01012017_SECS = 1483228800;
static const int64_t UNIX_EPOCH_01012020_SECS = 1577836800;
/*
 * It represents the number of seconds between the OPC-UA (Windows) which starts on 1601/01/01 (supposedly 00:00:00
 * UTC), and Linux times starts on epoch, 1970/01/01 00:00:00 UTC.
 * */
static const int64_t SOPC_SECONDS_BETWEEN_EPOCHS = 11644473600;
static const int64_t SOPC_SECONDS_TO_100_NANOSECONDS = 10000000; // 10^7

START_TEST(test_current_time)
{
    /* Check date is between 01/01/2017 and 01/01/2020 */
    SOPC_DateTime vDate = SOPC_Time_GetCurrentTimeUTC();
    ck_assert(vDate != 0);
    ck_assert(vDate > UNIX_EPOCH_01012017_SECS * SOPC_SECONDS_BETWEEN_EPOCHS * SOPC_SECONDS_TO_100_NANOSECONDS);
    ck_assert(vDate < UNIX_EPOCH_01012020_SECS * SOPC_SECONDS_BETWEEN_EPOCHS * SOPC_SECONDS_TO_100_NANOSECONDS);
}
END_TEST

Suite* tests_make_suite_time(void)
{
    Suite* s;
    TCase* tc_time;

    s = suite_create("Time tests");
    tc_time = tcase_create("Time");
    tcase_add_test(tc_time, test_current_time);
    suite_add_tcase(s, tc_time);

    return s;
}
