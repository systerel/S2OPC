#!/bin/bash
#
#  Run the script given parameters in the docker
#
set -e

DOCKER_IMAGE=6a3b34e578f5

sudo /etc/scripts/run-in-docker $DOCKER_IMAGE "$@"
