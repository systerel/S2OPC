#!/bin/bash
#
#  Script to produce toolkit delivery archive
#  $1 must be the branch/commit to use for generating the delivery
#  $2 must be the delivery version number (e.g.: 1.0.0)

echo "Check branch name is valid (1st param): $1"
git show-ref refs/heads/$1 &> /dev/null
if [[ $? != 0 ]]; then
    echo "Provide the LOCAL branch name or commit reference to use for generating the delivery"
    exit 1
fi
BRANCH_COMMIT=$1

if [[ -z $BRANCH_COMMIT ]]; then
    exit 1
fi

echo "Check version number is correct: $2"
regexp='^[0-9]+\.[0-9]+.[0-9]+$'
if ! [[ $2 =~ $regexp ]] ; then
   echo "Error: '$2' is not correct version number X.Y.Z";
   exit 1
fi
DELIVERY_NAME=INGOPCS_Toolkit_$2

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
echo "Generate and commit version file 'VERSION' with '$2' content"
echo "$2" > VERSION
git add VERSION &> /dev/null || exit 1
git commit -S -m "Add VERSION file" &> /dev/null || exit 1
echo "Generate C source files"
./clean.sh all || exit 1
./.pre-build-in-docker.sh ./pre-build.sh || exit 1
# Format the generated files added to avoid failure on further format verification
./.check-in-docker.sh ./.format.sh || exit 1
git add -f address_space_generation/genc csrc/services/bgenc
git commit -S -m "Add generated source files"

# Clean directories in which linux binaries / library are installed
rm -fr bin_linux install_linux

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
cp -r ../bin ../bin_linux
cd ..


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

echo "Check code with clang tools"
./.check-in-docker.sh ./.check-code.sh
if [[ $? != 0 ]]; then
    echo "Error: Checking code with clang tools";
    exit 1
fi

echo "Add Toolkit binaries"
git add -f bin_linux bin_windows &>/dev/null || exit 1
git add -f install_linux install_windows &>/dev/null || exit 1
./clean.sh || exit 1
echo "Generate documentation with doxygen"
doxygen doxygen/ingopcs-toolkit.doxyfile &> /dev/null || exit 1
echo "Add documentation in delivery branch"
git add -f apidoc &> /dev/null || exit 1
git commit -S -m "Add doxygen documentation for version $DELIVERY_NAME" &> /dev/null || exit 1
echo "Remove delivery script, docker scripts, .gitignore file and commit"
git rm -f delivery.sh .gitignore .*.sh acceptances_tests &> /dev/null || exit 1
git commit -S -m "Remove delivery script, docker scripts and .gitignore file" &> /dev/null || exit 1
echo "Generation of archive of version $DELIVERY_NAME"
git archive --prefix=$DELIVERY_NAME/ -o $DELIVERY_NAME.tar.gz $DELIVERY_NAME || exit 1

if [ $? -eq 0 ]; then
    echo "=============================================================="
    echo "Creation of delivery archive '$DELIVERY_NAME.tar.gz' succeeded"
    echo "Please tag the $DELIVERY_NAME branch on bare repository"
else
    echo "==========================================================="
    echo "Creation of delivery archive '$DELIVERY_NAME.tar.gz' FAILED"
    rm -f $DELIVERY_NAME.tar.gz || exit 1
fi
