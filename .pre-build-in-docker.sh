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


#  Prepare tooling dependency repository & run the script given parameters in the docker
#
# Options
# - TOOLING_DIR can be defined to point to the local tooling repository root directory

set -e

PREBUILD=pre-build
TMP_TOOLING_DIR=$(pwd)/$PREBUILD/tooling

DOCKER_IMAGE=ca487e4c9dde

# Create dir to store tooling directory for generation
\rm -rf $PREBUILD
mkdir -p $TMP_TOOLING_DIR > /dev/null
cd $TMP_TOOLING_DIR > /dev/null

# Copy tooling repository with git archive from server repository or local one
if [[ -z $TOOLING_DIR ]]; then
    echo "Environment variable TOOLING_DIR not set => copy of tooling.git through SSH"
    ssh pcb git --git-dir /git/c765/tooling.git archive master bin | tar x
else
    echo "Environment variable TOOLING_DIR set to '$TOOLING_DIR' => copy of repository"
    git --git-dir $TOOLING_DIR/.git archive master bin | tar x
fi
cd - > /dev/null


if [[ -z $SOPC_DOCKER_NEEDS_SUDO ]]; then
    /etc/scripts/run-in-docker $DOCKER_IMAGE "TOOLING_DIR=$TMP_TOOLING_DIR $@"
else
    sudo /etc/scripts/run-in-docker $DOCKER_IMAGE "TOOLING_DIR=$TMP_TOOLING_DIR $@"
fi

