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


# Script to format code using clang-format

SRCS_DIR=./src
TEST_DIR=./tests
BSRCS_DIR=./bsrc
SAMPLE_DIR=./samples
ZEPHYR_SAMPLES_DIR=./zephyr/samples

find $SRCS_DIR $TEST_DIR $SAMPLE_DIR -name "*.[hc]" ! -path "./src/ClientServer/services/bgenc/*" -exec clang-format -style=file -i '{}' \;
find $ZEPHYR_SAMPLES_DIR -name "*.[hc]" ! -name "test_address_space.c" -exec clang-format -style=file -i '{}' \;
find $BSRCS_DIR \( -name "*.mch" -or -name "*.imp" -or -name "*.ref" -or -name "*.def" -or -name "*.pmm" \) -exec sed 's/\t/    /g' -i '{}' \;
find $BSRCS_DIR \( -name "*.mch" -or -name "*.imp" -or -name "*.ref" -or -name "*.def" -or -name "*.pmm" \) -exec sed 's/\s\+$//g' -i '{}' \;
find . -name CMakeLists.txt -exec sed 's/\t/    /g' -i '{}' \;
find . -name CMakeLists.txt -exec sed 's/\s\+$//g' -i '{}' \;
