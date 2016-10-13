#!/bin/bash
#
#  Builds INGOPCS OPC UA stack and run tests
#
#  Binary files are generated in out/
#  
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
mid cleanall all
# run unit tests
mid check CK_TAP_LOG_FILE_NAME=results.tap
# run client server tests
./run_client_server_test.sh

