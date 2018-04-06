#!/bin/bash

# Copyright (C) 2018 Systerel and others.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.


#  Check toolkit tests binaries are present and run them

MY_DIR=$(cd $(dirname $0) && pwd)
BIN_DIR="${MY_DIR}/bin"
BUILD_DIR="${MY_DIR}/build"
VALIDATION_DIR="${MY_DIR}/validation"
TEST_DIR=${BUILD_DIR}
CTEST_FILE="${TEST_DIR}/CTestTestfile.cmake"

if [ ! -f "${CTEST_FILE}" ]; then
	TEST_DIR=${BIN_DIR}
	CTEST_FILE="${TEST_DIR}/CTestTestfile.cmake"
fi

if [ ! -f "${CTEST_FILE}" ]; then
	echo "No CTestTestfile in ${BIN_DIR} or ${BUILD_DIR}"
	echo "Is this a tagged release, or has CMake been run?"
	exit 1
fi

cd "${TEST_DIR}" && ctest -T test --no-compress-output --test-output-size-passed 65536 --test-output-size-failed 65536
CTEST_RET=$?

ls "${VALIDATION_DIR}"/*.tap >/dev/null 2>&1 && mv "${VALIDATION_DIR}"/*.tap "${BIN_DIR}"/

exit $CTEST_RET
