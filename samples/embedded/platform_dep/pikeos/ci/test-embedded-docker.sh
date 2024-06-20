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

LMX_VERSION=4.9.18

cd `dirname $0`/../../../../..
export SOPC_ROOT=`pwd`
export OUT_DIR=${SOPC_ROOT-}/build_pikeos
export CI_DIR=${SOPC_ROOT}/samples/embedded/platform_dep/pikeos/ci
export OUTPUT_QEMU_FILE=${OUT_DIR}/tests/unit_test_out.txt
# Default user used in pikeos docker
TAP_USER=pikeos_user

mkdir -p ${OUT_DIR}/tests

# Launch certificate manager
/usr/lmx-${LMX_VERSION}/lmx-serv -b || exit $*

# Prepare tap device for QEMU network simulation
sudo ip tuntap add mode tap user ${TAP_USER}
sudo ip addr add 192.168.8.4/24 dev tap0
sudo ip link set tap0 up

INTEGRATION_WS=/home/pikeos_user/S2OPC.int

echo "/opt/pikeos-5.1/bin/muxa -f ${INTEGRATION_WS}/muxa.xml"
/opt/pikeos-5.1/bin/muxa -f ${INTEGRATION_WS}/muxa.xml &

echo "Wait for muxa to wake up"
sleep 0.5

# Clean last unit test output
rm -f ${OUT_DIR}/tests/*

touch ${OUTPUT_QEMU_FILE}

[[ ! -f ${OUT_DIR}/bin/pikeos-native-devel-qemu-arm-v8hf-qemu-unit_test ]] && echo "Pikeos Kernel ${OUT_DIR}/bin/pikeos-native-devel-qemu-arm-v8hf-qemu-unit_test was not found." && echo "Check compilation procedure and path." && exit 1

# Launch QEMU simulation in background and redirect serial output to file
/opt/pikeos-5.1/share/qemu/bin/qemu-system-aarch64 -m 512 -smp 4 -M virt -cpu cortex-a57 -nographic -kernel  ${OUT_DIR}/bin/pikeos-native-devel-qemu-arm-v8hf-qemu-unit_test -device virtio-net-device,netdev=main  -netdev hubport,hubid=0,id=main -net tap,ifname=tap0 -serial file:${OUTPUT_QEMU_FILE} &
QEMU_PID=$!

${CI_DIR}/check_pikeos_test.py ${OUTPUT_QEMU_FILE} --verbose
EXIT_CODE=$?

kill ${QEMU_PID}

if [ ${EXIT_CODE} -eq 0 ]; then
    echo "Unit test succeed !"
else
    echo "Unit test failed with code ${EXIT_CODE}"
fi
exit $EXIT_CODE