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
BUILD_INFO_DIR=csrc/services/b2c/
BUILD_INFO_FILE=$BUILD_INFO_DIR/toolkit_build_info.c

if [[ $CMAKE_TOOLCHAIN_FILE ]]; then
    BUILD_DIR=build_toolchain
else
    BUILD_DIR=build
fi

GITID=$(git log -1 --format='%H')
echo "Commit id : $GITID";
DATE=`date +%Y-%m-%d`

is_dirty() {
   test -n "$(git status --porcelain)"
}

if is_dirty; then
   DIRTY="*";
fi

VERSION="$GITID$DIRTY"
echo "Building S2OPC-$VERSION"

echo '#include "toolkit_build_info.h"' > $BUILD_INFO_FILE
echo -e '#include "sopc_toolkit_constants.h"\n' >> $BUILD_INFO_FILE
echo 'const SOPC_Build_Info toolkit_build_info = {' >> $BUILD_INFO_FILE
echo '.toolkitVersion = TOOLKIT_VERSION,' >> $BUILD_INFO_FILE
echo '.toolkitSrcSignature ="'$VERSION'",' >> $BUILD_INFO_FILE
echo '.toolkitDockerId = "'$DOCKER_IMAGE'",' >> $BUILD_INFO_FILE
echo '.toolkitBuildDate = "'$DATE'"' >> $BUILD_INFO_FILE
echo '};' >> $BUILD_INFO_FILE

echo "Build log" > $CURDIR/build.log

echo "Build the library and tests with CMake" | tee -a $CURDIR/build.log
if [ -f "$BUILD_DIR/CMakeCache.txt" ]; then
    echo "- CMake already configured" | tee -a $CURDIR/build.log
else
    echo "- Generate ./$BUILD_DIR directory" | tee -a $CURDIR/build.log
    mkdir -p $BUILD_DIR || exit 1
    cd $BUILD_DIR  > /dev/null || exit 1
    echo "- Run CMake" | tee -a $CURDIR/build.log
    if [[ $CMAKE_TOOLCHAIN_FILE ]]; then
        CMAKE_OPTIONS="-DCMAKE_TOOLCHAIN_FILE=../$CMAKE_TOOLCHAIN_FILE"
    fi
    if [[ $BUILD_SHARED_LIBS ]]; then
        CMAKE_OPTIONS="$CMAKE_OPTIONS -DBUILD_SHARED_LIBS=$BUILD_SHARED_LIBS"
    fi
    if [[ $CMAKE_INSTALL_PREFIX ]]; then
        CMAKE_OPTIONS="$CMAKE_OPTIONS -DCMAKE_INSTALL_PREFIX=$CMAKE_INSTALL_PREFIX"
    fi
    cmake $CMAKE_OPTIONS .. >> $CURDIR/build.log
    cd - > /dev/null || exit 1
fi
if [[ $? != 0 ]]; then
    echo "Error: build configuration failed" | tee -a $CURDIR/build.log
    exit 1
fi

echo "- Run make" | tee -a $CURDIR/build.log
make -C $BUILD_DIR >> $CURDIR/build.log
if [[ $? != 0 ]]; then
    echo "Error: build failed" | tee -a $CURDIR/build.log
    exit 1
else
    echo "Built library and tests with success" | tee -a $CURDIR/build.log
fi

if [[ $CMAKE_TOOLCHAIN_FILE ]]; then
    echo "- Do not run make test when cross compiling"
else
    echo "- Run make test" | tee -a $CURDIR/build.log
    make -C $BUILD_DIR test >> $CURDIR/build.log
    if [[ $? != 0 ]]; then
        echo "Error: test failed" | tee -a $CURDIR/build.log
        exit 1
    fi
fi

if [[ $? == 0 ]]; then
    echo "Completed with SUCCESS" | tee -a $CURDIR/build.log
    exit 0
fi
