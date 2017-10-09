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

BUILD_DIR=build
EXEC_DIR=bin
CERT_DIR=tests/data/cert

echo "Build log" > build.log

echo "Build the library and tests with CMake" | tee -a build.log
if [ -f "$BUILD_DIR/CMakeCache.txt" ]; then
    echo "- CMake already configured" | tee -a build.log
    cd $BUILD_DIR || exit 1
else
    echo "- Generate ./build directory" | tee -a build.log
    mkdir $BUILD_DIR || exit 1
    echo "- Run CMake" | tee -a build.log
    cd  $BUILD_DIR || exit 1
    if [[ $CROSS_COMPILE_MINGW ]]; then
        cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-mingw32-w64.cmake .. 1>> build.log
    else
        cmake .. 1>> build.log
    fi
fi
if [[ $? != 0 ]]; then
    echo "Error: build configuration failed" | tee -a build.log
    exit 1
fi

echo "- Run make" | tee -a build.log
make 1>> build.log
if [[ $? != 0 ]]; then
    echo "Error: build failed" | tee -a build.log
    exit 1
else
    echo "Built library and tests with success" | tee -a build.log
fi
cd - 1> /dev/null || exit 1

echo "Prepare tests execution" | tee -a build.log
echo "- Create $EXEC_DIR/ directory" | tee -a build.log
\rm -fr $EXEC_DIR || exit 1
mkdir $EXEC_DIR || exit 1

echo "- Copy test binaries in $EXEC_DIR/ " | tee -a build.log
for file in build/*
do
    if [[ -f "$file" ]]
    then
        if [[ -x "$file" ]]
        then
            cp "$file" $EXEC_DIR || exit 1
        fi
    fi
done

echo "- Copy test certificates in build directory" | tee -a build.log
mkdir -p $EXEC_DIR/revoked $EXEC_DIR/untrusted $EXEC_DIR/trusted \
$EXEC_DIR/client_private $EXEC_DIR/server_private \
$EXEC_DIR/client_public $EXEC_DIR/server_public || exit 1
cp $CERT_DIR/cacert.der $EXEC_DIR/trusted || exit 1
cp $CERT_DIR/client.key $EXEC_DIR/client_private || exit 1
cp $CERT_DIR/client.key $EXEC_DIR/client_private/client.pem || exit 1
cp $CERT_DIR/client.der $EXEC_DIR/client_public || exit 1
cp $CERT_DIR/server.key $EXEC_DIR/server_private || exit 1
cp $CERT_DIR/server.der $EXEC_DIR/server_public || exit 1

if [[ $? == 0 ]]; then
    echo "Terminated with SUCCESS" | tee -a build.log
    exit 0
fi
