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

# Script to compile S2OPC demo application for HIL testing
# @params $1 : Operating system, to determinate which building script to use
#         $2 : The board name
#         $3 : S2OPC demo application to compile
#         $4 : EXTENSION is the extension of the file, (e.g. '.bin')
# Sends the right parameters for an S2OPC demo application to an interface script

cd $(dirname $0)
HIL_DIR=$(pwd)
cd ../../
HOST_DIR=$(pwd)

##########################
# Helper function to exit with an error message
# @param $* An error message
function fail() {
    echo -e "[EE] $*" >&2
    exit -1
}

OS=$1
[ -z "${OS}" ] && fail "Missing 'OS' for compilation stage"
BOARD=$2
[ -z "${BOARD}" ] && fail "Missing 'BOARD' for compilation stage"
APP=$3
[ -z "${APP}" ] && fail "Missing 'APP' for compilation stage"
EXTENSION=$4
# EXTENSION is the extension of the file, here .bin.
# For now only .bin files are supported.

cd ${HOST_DIR}/samples/embedded/platform_dep/${OS}/ci || fail "Missing folder '${HOST_DIR}/samples/embedded/platform_dep/${OS}/ci'"
[ -x ./build-${OS}-samples.sh ] || fail "Missing or invalid build script 'build-${OS}-samples.sh' in '$(pwd)'"
./build-${OS}-samples.sh -b $BOARD -a $APP

