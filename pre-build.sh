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
#
# Environment variables modifying the behavior of this script:
# - NO_PREBUILD_RM:    do not delete the PREBUILD directory
# - RUN_INTERACTIVELY: run an interactive shell in the Atelier B container

BMODEL_DIR=bsrc
PREBUILD=pre-build
PROJET=s2opc_genc

CURDIR=`pwd`
LOGPATH=$CURDIR/pre-build.log

if [[ -n $RUN_INTERACTIVELY ]]; then
    if [[ -z $TOOLING_DIR ]]; then
	echo "TOOLING_DIR must be set to run interactively"
	exit 1
    fi
    # Use a different directory for interaction
    PREBUILD=$PREBUILD-interactive
    NO_PREBUILD_RM=1
fi

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

    if [[ -n $NO_PREBUILD_RM ]]; then
	# Change Atelier B database path to make the project persistent
	echo "Changing Atelier B database path to $BASE/bdb" | tee -a $LOGPATH
	mkdir -p $BASE/bdb
	sed "/Atelier_Database_Directory/ s;/home/.*;$BASE/bdb;" \
	    /opt/atelierb-4.2/AtelierB > $BASE/AtelierB
	export STARTBB="startBB -r=$BASE/AtelierB"
    fi

    if [[ -n $RUN_INTERACTIVELY ]]; then
	# Launch an interactive shell in the project
	( cd $BASE && bash -i )
	true # Prevent any error reporting
    else
	echo "Generate C sources files from B model" | tee -a $LOGPATH
	make VERBOSE=1 -C $BASE >> $LOGPATH
    fi
fi

if [[ $? != 0 ]]; then
    echo "ERROR: generating C source files from B model" | tee -a $LOGPATH
    EXITCODE=1
fi
# NO_PREBUILD_RM might be used during B model development to keep pre-build directory and use incremental build
if [[ -z $NO_PREBUILD_RM ]]; then
    # Remove pre-build directory
    rm -rf $CURDIR/$PREBUILD
fi

if [[ $EXITCODE -eq 0 ]]; then
    echo "Completed with SUCCESS" | tee -a $LOGPATH
else
    echo "Completed with ERRORS" | tee -a $LOGPATH
fi

exit $EXITCODE
