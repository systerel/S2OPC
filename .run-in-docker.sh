#!/bin/bash
#
#  Run the script given parameters in the docker
#
set -e

DOCKER_IMAGE=380d473101a1

/etc/scripts/run-in-docker $DOCKER_IMAGE "$@"
