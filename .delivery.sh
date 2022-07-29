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


#  Script to produce toolkit delivery archive
#  $1 must be the delivery version number (e.g.: 1.0.0)
#  $2 must be the current minor modification issue number

echo "Check master branch name is valid"
BRANCH_COMMIT=master
VERSION_HEADER=./src/Common/helpers/sopc_version.h

git show-ref refs/heads/$BRANCH_COMMIT &> /dev/null
if [[ $? != 0 ]]; then
    echo "master branch invalid for generating the delivery"
    exit 1
fi

if [[ -z $BRANCH_COMMIT ]]; then
    exit 1
fi

echo "Check version number is correct: $1"
regexp='^([0-9]+)\.([0-9]+).([0-9]+)$'
if ! [[ $1 =~ $regexp ]] ; then
   echo "Error: '$1' is not a correct version number X.Y.Z";
   exit 1
else
   major=${BASH_REMATCH[1]}
   medium=${BASH_REMATCH[2]}
   minor=${BASH_REMATCH[3]}
fi
DELIVERY_NAME=S2OPC_Toolkit_$1

echo "Check minor modification issue number is correct: $2"
regexpTicket='^[0-9]+$'
if ! [[ $2 =~ $regexpTicket ]] ; then
   echo "Error: '$2' is not a correct ticket number";
   exit 1
fi

echo "Check branch and tag does not exist: $DELIVERY_NAME"
git show-ref refs/heads/$DELIVERY_NAME &> /dev/null
if [[ $? == 0 ]]; then
    echo "Error: refs/heads/$DELIVERY_NAME already exists";
    exit 1
fi
git show-ref refs/tags/$DELIVERY_NAME &> /dev/null
if [[ $? == 0 ]]; then
    echo "Error: refs/tags/$DELIVERY_NAME already exists";
    exit 1
fi
git show-ref refs/heads/$2-update-tagged-version &> /dev/null
if [[ $? == 0 ]]; then
    echo "Error: refs/heads/$2-update-tagged-version already exists";
    exit 1
fi

echo "Check archive does not exist: $DELIVERY_NAME.tar.gz"
ls $DELIVERY_NAME.tar.gz &> /dev/null
if [[ $? == 0 ]]; then
    echo "Error: $DELIVERY_NAME.tar.gz already exists";
    exit 1
fi

echo "Checking out $BRANCH_COMMIT"
git checkout $BRANCH_COMMIT &> /dev/null || exit 1
echo "Creation of $DELIVERY_NAME branch"
git checkout -b $DELIVERY_NAME &> /dev/null || exit 1

echo "Checking out $2-update-tagged-version"
git checkout -b $2-update-tagged-version &> /dev/null || exit 1
echo "Update to $1* version in $VERSION_HEADER in $2-update-tagged-version"
sed -i 's/#define SOPC_TOOLKIT_VERSION_MAJOR .*/#define SOPC_TOOLKIT_VERSION_MAJOR '"$major"'/' $VERSION_HEADER || exit 1
sed -i 's/#define SOPC_TOOLKIT_VERSION_MEDIUM .*/#define SOPC_TOOLKIT_VERSION_MEDIUM '"$medium"'/' $VERSION_HEADER || exit 1
sed -i 's/#define SOPC_TOOLKIT_VERSION_MINOR .*/#define SOPC_TOOLKIT_VERSION_MINOR '"$minor"'/' $VERSION_HEADER || exit 1

echo "Update to $1 version in src/CMakeLists.txt file"
sed -i 's/VERSION [0-9]\+\.[0-9]\+\.[0-9]\+/VERSION '"$1"'/' src/CMakeLists.txt || exit 1

echo "Update to $1 version in README.md file"
sed -i 's/S2OPC_Toolkit_[0-9]\+\.[0-9]\+\.[0-9]\+/S2OPC_Toolkit_'"$1"'/' README.md || exit 1

echo "Commit updated current version in $2-update-tagged-version: it shall be pushed as MR on gitlab ASAP"
git commit src/CMakeLists.txt README.md $VERSION_HEADER -S -m "Ticket #$2: Update current version of Toolkit to $1" &> /dev/null || exit 1

echo "Checking out $DELIVERY_NAME"
git checkout $DELIVERY_NAME || exit 1

echo "Update to $1 version in $VERSION_HEADER in $DELIVERY_NAME"
sed -i 's/#define SOPC_TOOLKIT_VERSION_MAJOR .*/#define SOPC_TOOLKIT_VERSION_MAJOR '"$major"'/' $VERSION_HEADER || exit 1
sed -i 's/#define SOPC_TOOLKIT_VERSION_MEDIUM .*/#define SOPC_TOOLKIT_VERSION_MEDIUM '"$medium"'/' $VERSION_HEADER || exit 1
sed -i 's/#define SOPC_TOOLKIT_VERSION_MINOR .*/#define SOPC_TOOLKIT_VERSION_MINOR '"$minor"'/' $VERSION_HEADER || exit 1
sed -i 's/ "\*"//' $VERSION_HEADER

echo "Update to $1 version in src/CMakeLists.txt file"
sed -i 's/VERSION [0-9]\+\.[0-9]\+\.[0-9]\+/VERSION '"$1"'/' src/CMakeLists.txt || exit 1

echo "Update to $1 version in README.md file"
sed -i 's/S2OPC_Toolkit_[0-9]\+\.[0-9]\+\.[0-9]\+/S2OPC_Toolkit_'"$1"'/' README.md || exit 1

git commit src/CMakeLists.txt README.md $VERSION_HEADER -S -m "Update tagged $1 S2OPC version" &> /dev/null || exit 1

echo "Generate and commit version file 'VERSION' with '$1' content"
echo "$1" > VERSION
git add VERSION &> /dev/null || exit 1
git commit -S -m "Add VERSION file" &> /dev/null || exit 1
echo "Generate C source files"
./clean.sh all || exit 1
./.pre-build-in-docker.sh ./pre-build.sh || exit 1
echo "Check generated C source files were up to date"
# check that generated C code is up to date in configuration management
./clean.sh && ./.check_generated_code.sh || exit 1

echo "Re-Generate C source files and commit them with current date"
# regenerate C source files to be sure we keep current date in source files
./.pre-build-in-docker.sh ./pre-build.sh || exit 1
git commit src/ClientServer/services/bgenc -m "Add generated source files with up to date generation timestamp"

echo "Check code with clang tools"
./.check-in-docker.sh ./.check-code.sh
if [[ $? != 0 ]]; then
    echo "Error: Checking code with clang tools";
    exit 1
fi

# Clean directories in which windows binaries / libraries are installed
rm -fr bin_windows install_windows

echo "Generate Toolkit windows binaries (library and tests)"
mkdir -p build_toolchain || exit 1
cd build_toolchain || exit 1
../.mingwbuild-in-docker.sh cmake -DS2OPC_CLIENTSERVER_ONLY=1 -DWARNINGS_AS_ERRORS=0 -DCMAKE_TOOLCHAIN_FILE=toolchain-mingw32-w64.cmake -DCMAKE_INSTALL_PREFIX=../install_windows -DCMAKE_BUILD_TYPE=RelWithDebInfo .. || exit 1
../.mingwbuild-in-docker.sh cmake --build . --target install
if [[ $? != 0 ]]; then
    echo "Error: Generation of Toolkit windows binaries failed";
    exit 1
fi
rm -fr ../bin_windows
cp -r bin ../bin_windows
cd ..

# Clean directories in which linux binaries / library are installed
rm -fr bin install_linux

echo "Generate Toolkit linux shared binaries (library and tests)"
mkdir -p build || exit 1
cd build || exit 1
../.build-in-docker.sh cmake -DBUILD_SHARED_LIBS=true -DCMAKE_INSTALL_PREFIX=../install_linux .. || exit 1
../.build-in-docker.sh cmake --build . --target install
if [[ $? != 0 ]]; then
    echo "Error: Generation of Toolkit linux shared binaries failed";
    exit 1
fi
cd ..

echo "Generate Toolkit linux static binaries (library and tests)"
mkdir -p build || exit 1
cd build || exit 1
../.build-in-docker.sh cmake -DBUILD_SHARED_LIBS=false -DCMAKE_INSTALL_PREFIX=../install_linux .. || exit 1
../.build-in-docker.sh cmake --build . --target install
if [[ $? != 0 ]]; then
    echo "Error: Generation of Toolkit linux static binaries failed";
    exit 1
fi
mv bin ../
cd ..

echo "Add Toolkit binaries"
git add -f bin bin_windows &>/dev/null || exit 1
git add -f install_linux install_windows &>/dev/null || exit 1
git commit -S -m "Add toolkit binaries for version $DELIVERY_NAME" &> /dev/null || exit 1

echo "Generate test script"
mkdir bin/ClientServer bin/PubSub
./tests/scripts/make-ctestfile-relative build/tests/ClientServer/CTestTestfile.cmake > bin/ClientServer/CTestTestfile.cmake
./tests/scripts/make-ctestfile-relative build/tests/PubSub/CTestTestfile.cmake > bin/PubSub/CTestTestfile.cmake

# The bin folder is moved to the root in releases
sed -i 's,build/bin,bin,g' bin/ClientServer/CTestTestfile.cmake bin/PubSub/CTestTestfile.cmake
sed -i 's,/usr/local/bin/python3,python3,g' bin/ClientServer/CTestTestfile.cmake bin/PubSub/CTestTestfile.cmake

git add -f bin/ClientServer/CTestTestfile.cmake bin/PubSub/CTestTestfile.cmake &>/dev/null || exit 1
git commit -S -m "Add CTest test file for version $DELIVERY_NAME" &> /dev/null || exit 1

echo "Generate documentation with doxygen"
doxygen doxygen/s2opc-toolkit.doxyfile &> /dev/null
if [[ $? != 0 ]]; then
    echo "Error: Documentation generation with doxygen failed";
    exit 1
fi

echo "Add documentation in delivery branch"
git add -f apidoc &> /dev/null || exit 1
git commit -S -m "Add doxygen documentation for version $DELIVERY_NAME" &> /dev/null || exit 1
echo "Keep CI artifacts, remove internal CI files, .gitignore file and commit"
sed -i 's/expire_in:.*$/expire_in: never/g' .gitlab-ci.yml
git add .gitlab-ci.yml
git rm -rf .systerel/ .gitignore &> /dev/null || exit 1
git commit -S -m "Keep CI artifacts, remove internal CI files and .gitignore file" &> /dev/null || exit 1
echo "Generation of archive of version $DELIVERY_NAME"
git archive --prefix=S2OPC-$DELIVERY_NAME/ -o $DELIVERY_NAME.tar.gz $DELIVERY_NAME || exit 1

if [ $? -eq 0 ]; then
    echo "=============================================================="
    echo "Creation of delivery archive '$DELIVERY_NAME.tar.gz' succeeded"
    echo "PLEASE USE ARCHIVE ONLY FOR TESTING PURPOSE: retrieve the final archive from gitlab directly to avoid md5sum possible issue (--prefix choice)"
    echo "Please push the $2-update-tagged-version branch as MR on gitlab closing issue #$2"
    echo "Please tag the $DELIVERY_NAME branch on bare repository WITH SIGNATURE: $DELIVERY_NAME"
else
    echo "==========================================================="
    echo "Creation of delivery archive '$DELIVERY_NAME.tar.gz' FAILED"
    rm -f $DELIVERY_NAME.tar.gz || exit 1
fi
