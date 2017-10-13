#!/bin/bash
#
#  Run the script given parameters in the docker
#
set -e

DOCKER_IMAGE=4bdff26d936f

sudo /etc/scripts/run-in-docker $DOCKER_IMAGE "$@"
