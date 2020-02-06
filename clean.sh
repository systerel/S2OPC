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


# Script to clean the S2OPC toolkit project:
# - clean the build logs and build directories (build and bin)
# - if first argument is all it also clean the generated code
set -e

ISALL=$1

# Clean pre-build, build and bin dirs
echo "Cleaning pre-build/, build/ and bin/ directories"
\rm -f src/configuration/sopc_toolkit_build_info.h
\rm -f pre-build-check.log pre-build.log build.log clang_tidy.log
\rm -fr pre-build build build_toolchain build-analyzer

if [[ -z $ISALL || $ISALL != "all" ]]; then
    echo "Do not clean generated source files"
else
    echo "Clean generated source files from B model"
    \rm -fr ./src/ClientServer/services/bgenc
fi
