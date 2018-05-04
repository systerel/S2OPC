/*
 *  Copyright (C) 2018 Systerel and others.
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

/** \file
 *
 * \brief Entry point for threads tests. Tests use libcheck.
 *
 * If you want to debug the exe, you should define env var CK_FORK=no
 * http://check.sourceforge.net/doc/check_html/check_4.html#No-Fork-Mode
 */

#include <check.h>
#include <stdlib.h>

#include "toolkit_helpers.h"

START_TEST(test_time_conversion)
{
    /* Thu Sep 21 00:00:00 1905 UTC, unix timestamp is -2028499761.000000 */
    ck_assert(Helpers_OPCTimeToNTP(96159738390000000) == 775194519791468544);
    /* Tue Jan  3 19:44:21 1978 UTC, unix timestamp is 252701061.000000 */
    ck_assert(Helpers_OPCTimeToNTP(118971746610000000) == 10572877445889785856);
    /* Thu Nov 30 04:57:25 2034 UTC, unix timestamp is 2048471845.694287 */
    ck_assert(Helpers_OPCTimeToNTP(136929454456942870) == 18285654237264005879);
    /* Tue Nov 30 04:57:25 2055 UTC, unix timestamp is 2711159845.694287 */
    ck_assert(Helpers_OPCTimeToNTP(143556334456942870) == 2685133451006102263);
    /* Fri May  4 17:34:36 2018 UTC, unix timestamp is 1525448076.741346 */
    ck_assert(Helpers_OPCTimeToNTP(131699216767413460) == 16039284254580464121);
}
END_TEST

Suite* tests_make_suite_libsub(void)
{
    Suite* s;
    TCase *tc_time, *tc_elapsed_time;

    s = suite_create("Client subscription library");

    tc_time = tcase_create("Time");
    tcase_add_test(tc_time, test_time_conversion);
    suite_add_tcase(s, tc_time);

    return s;
}

int main(void)
{
    int number_failed = 0;
    SRunner* sr = NULL;

    sr = srunner_create(tests_make_suite_libsub());
    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
