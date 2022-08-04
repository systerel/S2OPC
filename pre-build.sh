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


# Script to generate data necessary to build the S2OPC project:
#
# Steps (if necessary):
# - generate sources files from B model
# - generate sources files for examples address space for tests
#
# Prerequisite (for generation from B model):
# - TOOLING_DIR contains the path of the tooling directory (root directory containing bin/)

BMODEL_DIR=bsrc
PREBUILD=pre-build
PROJET=s2opc_genc

CURDIR=`pwd`
LOGPATH=$CURDIR/pre-build.log

# Redirect all output and errors to log file
echo "Pre-build log" > $LOGPATH

EXITCODE=0

if [[ -z $TOOLING_DIR ]]; then
    # If tooling directory is not defined, only call make target
    echo "Environment variable TOOLING_DIR not set (CAUTION: no generation from B model possible)" | tee -a $LOGPATH
    echo "Check C sources files from B model are up to date" | tee -a $LOGPATH
    make VERBOSE=1 -C $BMODEL_DIR >> $LOGPATH
else
    echo "Prepare B project and tools configuration" | tee -a $LOGPATH
    # Set a pre-build local environment for "m" script

    export ATELIERB_VERSION="4.2.1"
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
rm -rf $CURDIR/$PREBUILD

if [[ $EXITCODE -eq 0 ]]; then
    echo "Completed with SUCCESS" | tee -a $LOGPATH
else
    echo "Completed with ERRORS" | tee -a $LOGPATH
fi

exit $EXITCODE
