/*
 * Tests suites are gathered here.
 * Inspired from https://github.com/libcheck/check/blob/master/tests/check_check.h
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

#ifndef CHECK_HELPERS_H
#define CHECK_HELPERS_H

Suite* tests_make_suite_crypto_B256S256(void);
Suite* tests_make_suite_crypto_B256(void);
Suite* tests_make_suite_crypto_None(void);

Suite* tests_make_suite_tools(void);

Suite* tests_make_suite_threads(void);

Suite* tests_make_suite_time(void);

#endif // CHECK_HELPERS_H
