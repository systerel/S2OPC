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

# Script to build the S2OPC project:
#
# Options:
# - Variable CMAKE_TOOLCHAIN_FILE set with filename of toolchain config in root directory
# - Variable BUILD_SHARED_LIBS set to compile the shared library instead of static one
# - Variable CMAKE_INSTALL_PREFIX set to configure the install prefix of cmake
#
# Steps:
# - generate build information
# - configure build with cmake
# - build the library and tests
# - prepare test execution

function _help() {
    echo "$1 build S2OPC"
    echo "Usage: $1 [-h] [--jobs <NPROC>]"
    echo "    --jobs <NPROC>  : Change number of CPUs used for make, default to nproc"
    echo "    -h : print this help and exit"
}

NPROC=

while [[ "$#" -gt 0 ]] ; do
PARAM=$1
shift
[[ "${PARAM-}" =~ "-h(elp)?" ]] && _help "$0" && exit 0
[[ "${PARAM-}" == "--jobs" ]] && NPROC="$1" && shift && continue
echo "$0: Unexpected parameter : ${PARAM-}" && exit 127
done

[[ -z $NPROC ]] && export NPROC="$(nproc)" && echo "Using default jobs : make -j $NPROC"

CURDIR=`pwd`
EXEC_DIR=bin
set -e

if [[ $CMAKE_TOOLCHAIN_FILE ]]; then
    BUILD_DIR=${BUILD_DIR:-build_toolchain}
else
    BUILD_DIR=${BUILD_DIR:-build}
fi

# Check if the option which name is $1 is defined in env,
#  and adds it and its value to CMAKE_OPTIONS
#  else add default value defined in $2.
append_cmake_option ()
{
    if [[ -n "${!1}" ]]; then
        CMAKE_OPTIONS="$CMAKE_OPTIONS -D$1=${!1}"
    elif [[ -n "$2" ]]; then
        CMAKE_OPTIONS="$CMAKE_OPTIONS -D$1=$2"
    fi
}

echo "Build log" > "$CURDIR/build.log"

echo "Build the library and tests with CMake" | tee -a "$CURDIR/build.log"
if [ -f "$BUILD_DIR/CMakeCache.txt" ]; then
    echo "- CMake already configured" | tee -a "$CURDIR/build.log"
else
    echo "- Generate ./$BUILD_DIR directory" | tee -a "$CURDIR/build.log"
    mkdir -p $BUILD_DIR || exit 1
    cd $BUILD_DIR  > /dev/null || exit 1
    echo "- Run CMake" | tee -a "$CURDIR/build.log"
    append_cmake_option S2OPC_NANO_PROFILE
    append_cmake_option S2OPC_NODE_MANAGEMENT
    append_cmake_option S2OPC_NODE_ADD_OPTIONAL
    append_cmake_option S2OPC_NODE_INTERNAL_ADD_CHILD_NODES
    append_cmake_option S2OPC_NODE_ADD_INVERSE_TYPEDEF
    append_cmake_option S2OPC_NODE_DISABLE_CHECK_UNIQUENESS_BROWSENAME
    append_cmake_option S2OPC_EXTERNAL_HISTORY_RAW_READ_SERVICE
    append_cmake_option S2OPC_NODE_DELETE_CHILD_NODES
    append_cmake_option S2OPC_NODE_DELETE_ORGANIZES_CHILD_NODES
    append_cmake_option S2OPC_EVENT_MANAGEMENT
    append_cmake_option S2OPC_HAS_AUDITING
    append_cmake_option CMAKE_TOOLCHAIN_FILE
    append_cmake_option CMAKE_PREFIX_PATH
    append_cmake_option expat_DIR
    append_cmake_option MbedTLS_DIR
    append_cmake_option BUILD_SHARED_LIBS
    append_cmake_option CMAKE_INSTALL_PREFIX
    append_cmake_option PYTHON_EXECUTABLE
    append_cmake_option WITH_ASAN
    append_cmake_option WITH_UBSAN
    append_cmake_option WITH_TSAN
    append_cmake_option WITH_COVERAGE
    append_cmake_option WITH_COVERITY
    append_cmake_option WITH_PYS2OPC
    append_cmake_option WITH_CONST_ADDSPACE
    append_cmake_option WITH_STATIC_SECURITY_DATA
    append_cmake_option WITH_NO_ASSERT
    append_cmake_option WITH_USER_ASSERT
    append_cmake_option WITH_MINIMAL_FOOTPRINT
    append_cmake_option WARNINGS_AS_ERRORS
    append_cmake_option PUBSUB_STATIC_CONFIG
    append_cmake_option CMAKE_BUILD_TYPE RelWithDebInfo
    append_cmake_option CMAKE_EXE_LINKER_FLAGS
    append_cmake_option S2OPC_CRYPTO_MBEDTLS
    append_cmake_option S2OPC_CRYPTO_CYCLONE
    append_cmake_option S2OPC_CLIENTSERVER_ONLY
    append_cmake_option S2OPC_PUBSUB_ONLY
    append_cmake_option ENABLE_TESTING
    append_cmake_option ENABLE_SAMPLES
    append_cmake_option USE_STATIC_EXT_LIBS
    append_cmake_option POSITION_INDEPENDENT_EXECUTABLE
    append_cmake_option SECURITY_HARDENING
    append_cmake_option PYS2OPC_WHEEL_NAME
    append_cmake_option WITH_GCC_STATIC_ANALYSIS
    # append_cmake_option doesn't permit to handle multiple variables because of multiple evaluations of quotes

    echo "cmake $CMAKE_OPTIONS -DCMAKE_C_FLAGS=\"$CMAKE_C_FLAGS\" .." >> "$CURDIR/build.log"
    cmake $CMAKE_OPTIONS -DCMAKE_C_FLAGS="$CMAKE_C_FLAGS" .. >> "$CURDIR/build.log"
    cd - > /dev/null || exit 1
fi
if [[ $? != 0 ]]; then
    echo "Error: build configuration failed" | tee -a "$CURDIR/build.log"
    exit 1
fi

echo "- Run make" | tee -a "$CURDIR/build.log"
make -j $NPROC -C $BUILD_DIR >> "$CURDIR/build.log"
if [[ $? != 0 ]]; then
    echo "Error: build failed" | tee -a "$CURDIR/build.log"
    exit 1
else
    echo "Built library and tests with success" | tee -a "$CURDIR/build.log"
fi

if [[ $? == 0 ]]; then
    echo "Completed with SUCCESS" | tee -a "$CURDIR/build.log"
    exit 0
fi
