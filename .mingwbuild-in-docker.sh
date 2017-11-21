#!/bin/bash
#
#  Run the script given parameters in the docker
#
set -e

DOCKER_IMAGE=0f82477cd7ac

if [[ -z $SOPC_DOCKER_NEEDS_SUDO ]]; then
    /etc/scripts/run-in-docker $DOCKER_IMAGE CROSS_COMPILE_MINGW=true "$@"
else
    sudo /etc/scripts/run-in-docker $DOCKER_IMAGE CROSS_COMPILE_MINGW=true "$@"
fi
