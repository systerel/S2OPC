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


# Script to generate data necessary to build the INGOPCS project:
#
# Steps (if necessary):
# - generate sources files from B model
# - generate sources files for examples address space for tests
#
# Prerequisite (for generation from B model):
# - TOOLING_DIR contains the path of the tooling directory (root directory containing bin/)

BMODEL_DIR=bsrc
PREBUILD=pre-build
PROJET=ingopcs_genc

CURDIR=`pwd`
LOGPATH=$CURDIR/pre-build.log

# Redirect all output and errors to log file
echo "Pre-build log" > $LOGPATH

EXITCODE=0

if [[ -z $TOOLING_DIR ]]; then
    # If tooling directory not define only call make target
    echo "Environment variable TOOLING_DIR not set (CAUTION: no generation from B model possible)" | tee -a $LOGPATH
    echo "Check C sources files from B model are up to date" | tee -a $LOGPATH
    make VERBOSE=1 -C $BMODEL_DIR >> $LOGPATH
else
    echo "Prepare B project and tools configuration" | tee -a $LOGPATH
    # Set a pre-build local environment for "m" script

    export ATELIERB_VERSION="4.0.2"
    export STARTBB=startBB
    export BASE=$(pwd)/$PREBUILD
    # Create pre-build directory to check B modle and generate C code
    mkdir -p $BASE >> $LOGPATH
    # Configure "m" script environment
    export liste_projet=$BASE/liste_projets.txt
    echo $PROJET . > $liste_projet
    export TRAD_JAVA="java -jar $TOOLING_DIR/bin/trad/b2c.jar"
    export ROOT="toolkit_header"
    # Make symbolic link to all files in bsrc/
    cd $BASE && find ../$BMODEL_DIR -maxdepth 1 -type f -exec ln -f -s {} . \; && cd - >> $LOGPATH
    PATH=$TOOLING_DIR/bin/m:$TOOLING_DIR/bin/trad:$PATH

    echo "Generate C sources files from B model" | tee -a $LOGPATH
    make VERBOSE=1 -C $BASE >> $LOGPATH
fi

if [[ $? != 0 ]]; then
    echo "ERROR: generating C source files from B model" | tee -a $LOGPATH
    EXITCODE=1
fi
# Remove pre-build directory in any case
\rm -rf $CURDIR/$PREBUILD

if [[ $EXITCODE -eq 0 ]]; then
    echo "Completed with SUCCESS" | tee -a $LOGPATH
else
    echo "Completed with ERRORS" | tee -a $LOGPATH
fi

exit $EXITCODE
