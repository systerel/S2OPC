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


# Calls lcov/genhtml after project has been built with WITH_COVERAGE=1
set -e

SRC_DIR=build/CMakeFiles/ingopcs.dir/
REPORT_DIR=./report
REPORT_FILE=$REPORT_DIR/report.info

mkdir -p $REPORT_DIR
lcov --directory $SRC_DIR -c -o $REPORT_FILE
# Remove bogus mbedtls files
lcov -r $REPORT_FILE "/usr/*" -o $REPORT_FILE
genhtml -o $REPORT_DIR -t "Code coverage from all tests" $REPORT_FILE
lcov --summary $REPORT_FILE
