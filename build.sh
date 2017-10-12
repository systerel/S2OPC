#!/bin/bash
# Script to build the INGOPCS project:
#
# Options:
# - Variable CROSS_COMPILE_MINGW set activate mingw cross compilation
#
# Steps:
# - configure build with cmake
# - build the library and tests
# - prepare test execution

CURDIR=`pwd`
BUILD_DIR=build
EXEC_DIR=bin
CERT_DIR=tests/data/cert

echo "Build log" > $CURDIR/build.log

echo "Build the library and tests with CMake" | tee -a $CURDIR/build.log
if [ -f "$BUILD_DIR/CMakeCache.txt" ]; then
    echo "- CMake already configured" | tee -a $CURDIR/build.log
else
    echo "- Generate ./build directory" | tee -a $CURDIR/build.log
    mkdir -p $BUILD_DIR || exit 1
    cd $BUILD_DIR  > /dev/null || exit 1
    echo "- Run CMake" | tee -a $CURDIR/build.log
    if [[ $CROSS_COMPILE_MINGW ]]; then
        cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-mingw32-w64.cmake .. >> $CURDIR/build.log
    else
        cmake .. >> $CURDIR/build.log
    fi
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

if [[ $CROSS_COMPILE_MINGW ]]; then
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
    echo "Terminated with SUCCESS" | tee -a $CURDIR/build.log
    exit 0
fi
