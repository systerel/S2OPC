#!/bin/bash
#
#  Prepare tooling dependency repository & run the script given parameters in the docker
#
# Options
# - TOOLING_DIR can be defined to point to the local tooling repository root directory

set -e

PREBUILD=pre-build
TMP_TOOLING_DIR=$(pwd)/$PREBUILD/tooling

DOCKER_IMAGE=ca487e4c9dde

# Create dir to store tooling directory for generation
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

