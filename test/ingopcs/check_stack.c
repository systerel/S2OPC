/*
 * Entry point for tests. Tests use libcheck.
 * https://libcheck.github.io/check/doc/check_html/check_3.html
 *
 * If you want to debug the exe, you should define env var CK_FORK=no
 * http://check.sourceforge.net/doc/check_html/check_4.html#No-Fork-Mode
 *
 *  Created on: Sep 6, 2016
 *      Author: PAB
 */


#include <stdlib.h>
#include <check.h>
#include "check_stack.h"


START_TEST(test_empty)
{
    ;
}
END_TEST


Suite *tests_make_suite_stack(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("Stack");
    tc_core = tcase_create("Dummy");
    tcase_add_test(tc_core, test_empty);
    suite_add_tcase(s, tc_core);

    return s;
}


int main(void)
{
    int number_failed;
    SRunner *sr;

    sr = srunner_create(tests_make_suite_stack());
    srunner_add_suite(sr, tests_make_suite_crypto());

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
