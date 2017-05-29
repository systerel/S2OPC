#!/bin/bash
#
#  Builds INGOPCS OPC UA stack and run tests
#
#  Binary files are generated in out/
#  Use "LOCAL" as first argument to run tests locally
#

set -e

BIN_DIR=bin
BIN_PATH=../$BIN_DIR

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
# run stack client server tests
mid -C stack EXEC_DIR=$BIN_PATH client_server_test
# run stack unit tests
mid -C stack EXEC_DIR=$BIN_PATH check CK_TAP_LOG_FILE_NAME=results.tap
# run toolkit client server tests
mid -C toolkit PATHEXEC=$BIN_PATH client_server_test
