#!/bin/bash
# Script to build the INGOPCS project:
#
# Options:
# - Variable CROSS_COMPILE_MINGW set activate mingw cross compilation
# - Variable NO_BMODEL_GEN set deactivate B model generation
#
# Steps:
# - generate sources files from B model
# - generate sources files for examples address space for tests
# - build the library and tests
# - prepare test execution

BUILD_DIR=build
EXEC_DIR=bin
CERT_DIR=tests/data/cert

if [[ $NO_BMODEL_GEN ]]; then
    echo "No generation of C source files from B model"
    if ! [[ -d src/services/B_genC/ ]]; then
        echo "Error: generated are not present"
        exit 1
    fi
else
    echo "Generate C sources files from B model"
    ./.translateBmodel.sh 1> /dev/null || exit 1
    if [[ $? != 0 ]]; then
        echo "Error: generating C source files from B model"
        exit 1
    fi
fi

echo "Generate address space C files for tests"
./.generateTestAddSpaces.sh 1> /dev/null || exit 1
if [[ $? != 0 ]]; then
    echo "Error: generating address spaces for tests"
    exit 1
fi

echo "Build the library and tests with CMake"
if [ -d "$BUILD_DIR" ]; then
    echo "- CMake already configured"
    cd $BUILD_DIR || exit 1
else
    echo "- Generate ./build directory"
    mkdir $BUILD_DIR || exit 1
    echo "- Run CMake"
    cd  $BUILD_DIR || exit 1
    if [[ $CROSS_COMPILE_MINGW ]]; then
        cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-mingw32-w64.cmake ..
    else
        cmake ..
    fi
fi
if [[ $? != 0 ]]; then
    echo "Error: build configuration failed"
    exit 1
fi

echo "- Run make"
make 1> /dev/null
if [[ $? != 0 ]]; then
    echo "Error: build failed"
    exit 1
else
    echo "Built library and tests with success"
fi
cd - 1> /dev/null || exit 1

echo "Prepare tests execution"
echo "- Create $EXEC_DIR/ directory"
\rm -fr $EXEC_DIR || exit 1
mkdir $EXEC_DIR || exit 1

echo "- Copy test binaries in $EXEC_DIR/ "
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

echo "- Copy test certificates in build directory"
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
    echo "Terminated with SUCCESS"
    exit 0
else
    exit 1
fi
