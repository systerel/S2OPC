#!/bin/bash
# Script to clean the INGOPCS toolkit project:
# - clean the build logs and build directories (build and bin)
# - if first argument is all it also clean the generated code
set -e

ISALL=$1

# Clean build and bin dirs
echo "Cleaning build/ and bin/ directories"
\rm -f pre-build.log build.log
\rm -fr build bin

if [[ -z $ISALL || $ISALL != "all" ]]; then
    echo "Do not clean generated source files"
else
    echo "Clean generated source files from B model and test @space"
    \rm -fr ./src/services/bgenc
    make -C address_space_generation clean > /dev/null
fi
