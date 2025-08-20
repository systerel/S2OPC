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

set -e

function append_newline {
    if [[ -z "$(tail -c 1 "$1")" ]]; then
        :
    else
        echo >> "$1"
    fi
}

SRCS_DIR=./src
TEST_DIR=./tests
BSRCS_DIR=./bsrc
SAMPLE_DIR=./samples

CLANG_FILES=$(find $SRCS_DIR $TEST_DIR $SAMPLE_DIR -name "*.[hc]" ! \( -path "./src/ClientServer/services/bgenc/*" -or -name "pys2opc.c" \))
clang-format -style=file -i ${CLANG_FILES}
# Check newlines
for f in ${CLANG_FILES}; do
    append_newline $f
done

# Check format in other files (no tabulation, end of line blank spaces and missing new line at EOF)
OTHER_FILES=$(find $BSRC_DIR \( -name "*.mch" -or -name "*.imp" -or -name "*.ref" -or -name "*.def" -or -name "*.pmm" -or -name "*.pyx" -or -name "*.py" \))
OTHER_FILES="$OTHER_FILES $(find . -name CMakeLists.txt)"
for f in ${OTHER_FILES}; do
    sed 's/\t/    /g' -i $f
    sed 's/\s\+$//g' -i $f
    append_newline $f
done
