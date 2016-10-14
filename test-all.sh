#!/bin/bash
#
#  Builds INGOPCS OPC UA stack and run tests
#
#  Binary files are generated in out/
#  Use "LOCAL" as first argument to run tests locally
#

set -e

DOCKER_IMAGE=c706f497db33
ISLOCAL=$1

mid() {
if [[ -z $ISLOCAL || $ISLOCAL != "LOCAL" ]]; then
    sudo /etc/scripts/make-in-docker $DOCKER_IMAGE CC=gcc "$@"
else
    make "$@"
fi
}

# Build and run tests
if [[ -z $ISLOCAL || $ISLOCAL != "LOCAL" ]]; then
    mid cleanall all
else
    mid clean all
fi
# run unit tests
mid check CK_TAP_LOG_FILE_NAME=results.tap
# run client server tests
mid client_server_test

