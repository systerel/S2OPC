#!/bin/bash
#
#  Run the script given parameters in the docker
#
set -e

# uactt 1.2
DOCKER_IMAGE=283e3aec4f94

if [[ -z $SOPC_DOCKER_NEEDS_SUDO ]]; then
    /etc/scripts/run-in-docker $DOCKER_IMAGE "$@"
else
    sudo /etc/scripts/run-in-docker $DOCKER_IMAGE "$@"
fi
