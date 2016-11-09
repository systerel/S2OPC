#!/bin/bash
#
#  Builds INGOPCS OPC UA stack and run tests
#
#  Binary files are generated in out/
#  Use "LOCAL" as first argument to run tests locally
#

# run unit tests
make check CK_TAP_LOG_FILE_NAME=results.tap
# run client server tests
make client_server_test

