#!/bin/bash

# Licensed to Systerel under one or more contributor license
# agreements. See the NOTICE file distributed with this work
# for additional information regarding copyright ownership.
# Systerel licenses this file to you under the Apache
# License, Version 2.0 (the "License"); you may not use this
# file except in compliance with the License. You may obtain
# a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.


set -e
export SCRIPT=build-zephyr-samples-docker.sh
cd "$(dirname "$0")"/..
HOST_DIR=$(pwd)

source .docker-images.sh
rm -rf build_zephyr/* 2>/dev/null
echo "Mapping ${HOST_DIR} to DOCKER '/workdir'"
(docker run --rm -v ${HOST_DIR}:/host_zephyr -w /host_zephyr ${ZEPHYR_IMAGE} zephyr/${SCRIPT})&

wait $!

echo "Result = $?"
# Check results
EXPECTED_FILES="zephyr_client_stm32h735g_disco.bin  zephyr_pubsub_stm32h735g_disco.bin  zephyr_server_stm32h735g_disco.bin  zephyr_server_mimxrt1064_evk.bin"
RESULT=true
for f in ${EXPECTED_FILES} ; do
	[ ! -f build_zephyr/${f} ] && echo "File not build : ${f}" && RESULT=false
	[ -f build_zephyr/${f} ] && echo "File OK : ${f}"
done

if ! ${RESULT} ; then
	echo "Build failed. To run docker manually:"
	echo "docker run -it --rm -v ${HOST_DIR}:/host_zephyr -w /host_zephyr ${ZEPHYR_IMAGE} "
	exit 200
fi
echo "Build Successful"

