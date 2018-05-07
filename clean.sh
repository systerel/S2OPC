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


# Script to clean the INGOPCS toolkit project:
# - clean the build logs and build directories (build and bin)
# - if first argument is all it also clean the generated code
set -e

ISALL=$1

# Clean pre-build, build and bin dirs
echo "Cleaning pre-build/, build/ and bin/ directories"
\rm -f csrc/configuration/sopc_toolkit_build_info.h
\rm -f pre-build-check.log pre-build.log build.log
\rm -fr pre-build build build_toolchain build_clang bin bin_clang

if [[ -z $ISALL || $ISALL != "all" ]]; then
    echo "Do not clean generated source files"
else
    echo "Clean generated source files from B model and test @space"
    \rm -fr ./csrc/services/bgenc
fi
