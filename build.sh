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


# Script to build the INGOPCS project:
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
CERT_DIR=tests/data/cert

if [[ $CMAKE_TOOLCHAIN_FILE ]]; then
    BUILD_DIR=build_toolchain
else
    BUILD_DIR=build
fi

# default build mode is RELWITHDEBINFO
CMAKE_OPTIONS="$CMAKE_OPTIONS -DCMAKE_BUILD_TYPE=RelWithDebInfo"

# Check if the option which name is $1 is defined in env,
#  and adds it and its value to CMAKE_OPTIONS
append_cmake_option ()
{
    if [[ $1 ]]; then
        CMAKE_OPTIONS="$CMAKE_OPTIONS -D$1=${!1}"
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
