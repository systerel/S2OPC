/*
 * Tests suites are gathered here.
 * Inspired from https://github.com/libcheck/check/blob/master/tests/check_check.h
 *
 *  Created on: Sep 27, 2016
 *      Author: PAB
 */

#ifndef CHECK_STACK_H
#define CHECK_STACK_H


Suite *tests_make_suite_crypto(void);

Suite *tests_make_suite_tools(void);

Suite *tests_make_suite_core_tools(void);


#endif  // CHECK_STACK_H
