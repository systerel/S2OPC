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


#  Script to produce toolkit delivery archive
#  $1 must be the delivery version number (e.g.: 1.0.0)
#  $2 must be the current minor modification issue number

echo "Check master branch name is valid"
BRANCH_COMMIT=master
VERSION_HEADER=./csrc/configuration/sopc_toolkit_constants.h

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
DELIVERY_NAME=INGOPCS_Toolkit_$1

echo "Check minor modification issue number is correct: $2"
regexp='^[0-9]+$'
if ! [[ $2 =~ $regexp ]] ; then
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

echo "Check archive does not exist: $DELIVERY_NAME.tar.gz"
ls $DELIVERY_NAME.tar.gz &> /dev/null
if [[ $? == 0 ]]; then
    echo "Error: $DELIVERY_NAME.tar.gz already exists";
    exit 1
fi

echo "Checking out $BRANCH_COMMIT"
git co $BRANCH_COMMIT &> /dev/null || exit 1
echo "Creation of $DELIVERY_NAME branch"
git co -b $DELIVERY_NAME || exit 1

echo "Checking out $BRANCH_COMMIT"
git co $BRANCH_COMMIT &> /dev/null || exit 1
echo "Update to $1* version in sopc_toolkit_constants.h in $BRANCH_COMMIT"
sed -i 's/#define SOPC_TOOLKIT_VERSION_MAJOR .*/#define SOPC_TOOLKIT_VERSION_MAJOR '"$major"'/' $VERSION_HEADER || exit 1
sed -i 's/#define SOPC_TOOLKIT_VERSION_MEDIUM .*/#define SOPC_TOOLKIT_VERSION_MEDIUM '"$medium"'/' $VERSION_HEADER || exit 1
sed -i 's/#define SOPC_TOOLKIT_VERSION_MINOR .*/#define SOPC_TOOLKIT_VERSION_MINOR '"$minor"'/' $VERSION_HEADER || exit 1
sed -i "s/#define SOPC_TOOLKIT_VERSION .*/#define SOPC_TOOLKIT_VERSION \"$major.$medium.$minor*\"/" $VERSION_HEADER || exit 1
echo "Commit updated current version in $BRANCH_COMMIT: it shall be pushed on gitlab ASAP"
git commit $VERSION_HEADER -S -m "Ticket #$2: Update current version in $BRANCH_COMMIT" &> /dev/null || exit 1

echo "Checking out $DELIVERY_NAME"
git co $DELIVERY_NAME || exit 1

echo "Update to $1 version in sopc_toolkit_constants.h in $DELIVERY_NAME"
sed -i 's/#define SOPC_TOOLKIT_VERSION_MAJOR .*/#define SOPC_TOOLKIT_VERSION_MAJOR '"$major"'/' $VERSION_HEADER || exit 1
sed -i 's/#define SOPC_TOOLKIT_VERSION_MEDIUM .*/#define SOPC_TOOLKIT_VERSION_MEDIUM '"$medium"'/' $VERSION_HEADER || exit 1
sed -i 's/#define SOPC_TOOLKIT_VERSION_MINOR .*/#define SOPC_TOOLKIT_VERSION_MINOR '"$minor"'/' $VERSION_HEADER || exit 1
sed -i "s/#define SOPC_TOOLKIT_VERSION .*/#define SOPC_TOOLKIT_VERSION \"$major.$medium.$minor\"/" $VERSION_HEADER || exit 1
echo "Commit updated current version in $BRANCH_COMMIT: it shall be pushed on gitlab ASAP"
git commit $VERSION_HEADER -S -m "Update tagged $1 version information" &> /dev/null || exit 1

echo "Generate and commit version file 'VERSION' with '$1' content"
echo "$1" > VERSION
git add VERSION &> /dev/null || exit 1
git commit -S -m "Add VERSION file" &> /dev/null || exit 1
echo "Generate C source files"
./clean.sh all || exit 1
./.pre-build-in-docker.sh ./pre-build.sh || exit 1
# Format the generated files added to avoid failure on further format verification
./.check-in-docker.sh ./.format.sh || exit 1
git add -f address_space_generation/genc csrc/services/bgenc
git commit -S -m "Add generated source files"

# Clean directories in which windows binaries / libraries are installed
rm -fr bin_windows install_windows

echo "Generate Toolkit windows binaries (library and tests)"
mkdir -p build_toolchain || exit 1
cd build_toolchain || exit 1
../.mingwbuild-in-docker.sh cmake -DCMAKE_TOOLCHAIN_FILE=toolchain-mingw32-w64.cmake -DBUILD_SHARED_LIBS=true -DCMAKE_INSTALL_PREFIX=../install_windows .. || exit 1
../.mingwbuild-in-docker.sh cmake --build . --target install
if [[ $? != 0 ]]; then
    echo "Error: Generation of Toolkit windows binaries failed";
    exit 1
fi
rm -fr ../bin_windows
cp -r ../bin ../bin_windows
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
# Do not keep the binaries generated with shared library
rm -fr ../bin
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
cd ..

echo "Check code with clang tools"
./.check-in-docker.sh ./.check-code.sh
if [[ $? != 0 ]]; then
    echo "Error: Checking code with clang tools";
    exit 1
fi

echo "Add Toolkit binaries"
git add -f bin bin_windows &>/dev/null || exit 1
git add -f install_linux install_windows &>/dev/null || exit 1
git commit -S -m "Add toolkit binaries for version $DELIVERY_NAME" &> /dev/null || exit 1
echo "Generate documentation with doxygen"
doxygen doxygen/ingopcs-toolkit.doxyfile &> /dev/null || exit 1
echo "Add documentation in delivery branch"
git add -f apidoc &> /dev/null || exit 1
git commit -S -m "Add doxygen documentation for version $DELIVERY_NAME" &> /dev/null || exit 1
echo "Remove delivery script, docker scripts, .gitignore file and commit"
git rm -rf delivery.sh .gitignore .*.sh &> /dev/null || exit 1
git commit -S -m "Remove delivery script, docker scripts and .gitignore file" &> /dev/null || exit 1
echo "Generation of archive of version $DELIVERY_NAME"
git archive --prefix=$DELIVERY_NAME/ -o $DELIVERY_NAME.tar.gz $DELIVERY_NAME || exit 1

if [ $? -eq 0 ]; then
    echo "=============================================================="
    echo "Creation of delivery archive '$DELIVERY_NAME.tar.gz' succeeded"
    echo "Please push the master branch on gitlab ASAP and close the issue #$2"
    echo "Please tag the $DELIVERY_NAME branch on bare repository"
else
    echo "==========================================================="
    echo "Creation of delivery archive '$DELIVERY_NAME.tar.gz' FAILED"
    rm -f $DELIVERY_NAME.tar.gz || exit 1
fi
