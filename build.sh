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

CURDIR=`pwd`
EXEC_DIR=bin

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

echo "Build log" > $CURDIR/build.log

echo "Build the library and tests with CMake" | tee -a $CURDIR/build.log
if [ -f "$BUILD_DIR/CMakeCache.txt" ]; then
    echo "- CMake already configured" | tee -a $CURDIR/build.log
else
    echo "- Generate ./$BUILD_DIR directory" | tee -a $CURDIR/build.log
    mkdir -p $BUILD_DIR || exit 1
    cd $BUILD_DIR  > /dev/null || exit 1
    echo "- Run CMake" | tee -a $CURDIR/build.log
    append_cmake_option CMAKE_TOOLCHAIN_FILE
    append_cmake_option BUILD_SHARED_LIBS
    append_cmake_option CMAKE_INSTALL_PREFIX
    append_cmake_option WITH_ASAN
    append_cmake_option WITH_UBSAN
    append_cmake_option WITH_COVERAGE
    append_cmake_option WITH_COVERITY
    append_cmake_option WITH_PYS2OPC
    append_cmake_option WITH_CONST_ADDSPACE
    append_cmake_option WITH_STATIC_SECURITY_DATA
    append_cmake_option WITH_NANO_EXTENDED
    append_cmake_option WITH_NO_ASSERT
    append_cmake_option WITH_USER_ASSERT
    append_cmake_option WITH_MINIMAL_FOOTPRINT
    append_cmake_option WARNINGS_AS_ERRORS
    append_cmake_option PUBSUB_STATIC_CONFIG
    append_cmake_option CMAKE_BUILD_TYPE RelWithDebInfo
    append_cmake_option CMAKE_C_FLAGS
    append_cmake_option CMAKE_EXE_LINKER_FLAGS
    append_cmake_option S2OPC_CLIENTSERVER_ONLY
    append_cmake_option S2OPC_PUBSUB_ONLY
    append_cmake_option ENABLE_TESTING
    append_cmake_option ENABLE_SAMPLES
    append_cmake_option USE_STATIC_EXT_LIBS
    append_cmake_option POSITION_INDEPENDENT_EXECUTABLE
    append_cmake_option SECURITY_HARDENING
    append_cmake_option PYS2OPC_WHEEL_NAME
    append_cmake_option WITH_GCC_STATIC_ANALYSIS
    echo "cmake $CMAKE_OPTIONS .." >> $CURDIR/build.log
    cmake $CMAKE_OPTIONS .. >> $CURDIR/build.log
    cd - > /dev/null || exit 1
fi
if [[ $? != 0 ]]; then
    echo "Error: build configuration failed" | tee -a $CURDIR/build.log
    exit 1
fi

echo "- Run make" | tee -a $CURDIR/build.log
make -j $(nproc) -C $BUILD_DIR >> $CURDIR/build.log
if [[ $? != 0 ]]; then
    echo "Error: build failed" | tee -a $CURDIR/build.log
    exit 1
else
    echo "Built library and tests with success" | tee -a $CURDIR/build.log
fi

if [[ $? == 0 ]]; then
    echo "Completed with SUCCESS" | tee -a $CURDIR/build.log
    exit 0
fi
