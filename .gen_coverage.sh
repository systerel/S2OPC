#!/bin/bash

# Licensed to Systerel under one or more contributor license
# agreements. See the NOTICE file distributed with this work
# for additional information regarding copyright ownership.
# Systerel licenses this file to you under the Apache
# License, Version 2.0 (the "License"); you may not use this
# file except in compliance with the License. You may obtain
# a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.


# Calls lcov/genhtml after project has been built with WITH_COVERAGE=1
set -e

COMMON_DIR=build/src/Common/CMakeFiles/s2opc_common.dir/
CLIENTSERVER_DIR=build/src/ClientServer/CMakeFiles/s2opc_clientserver.dir/
PUBSUB_DIR=build/src/PubSub/CMakeFiles/s2opc_pubsub.dir/
REPORT_DIR=./report
REPORT_FILE=$REPORT_DIR/report.info

mkdir -p $REPORT_DIR
lcov -d $COMMON_DIR -d $CLIENTSERVER_DIR -d $PUBSUB_DIR -c -o $REPORT_FILE
# Remove bogus mbedtls files
lcov -r $REPORT_FILE "/usr/*" -o $REPORT_FILE
genhtml -o $REPORT_DIR -t "Code coverage from all tests" $REPORT_FILE
lcov --summary $REPORT_FILE
