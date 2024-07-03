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

set -o nounset
set -o pipefail

LMX_VERSION=4.9.18

cd `dirname $0`/../../../../..
export SOPC_ROOT=`pwd`

echo "SOPC_ROOT=$SOPC_ROOT"
APPLICATION_WS=/home/pikeos_user/S2OPC.app
INTEGRATION_WS=/home/pikeos_user/S2OPC.int

SHARED_POOL=/home/pikeos_user/shared_pool

export OUT_DIR=${SOPC_ROOT-}/build_pikeos

sudo ip tuntap add mode tap user pikeos_user
sudo ip addr add 192.168.8.4/24 dev tap0
sudo ip link set tap0 up


mkdir -p ${OUT_DIR} ${OUT_DIR}/bin ${OUT_DIR}/lib

/usr/lmx-${LMX_VERSION}/lmx-serv -b || exit $*

cd ${APPLICATION_WS}
make -j`nproc` install 2>&1 |tee ${OUT_DIR-}/build_app.log || exit $?

cp ${SHARED_POOL}/pikeos-native/object/*.elf ${SHARED_POOL}/pikeos-native/object/*.unstripped ${OUT_DIR}/bin
cp ${SHARED_POOL}/pikeos-native/object/*.a  ${OUT_DIR}/lib

cd ${INTEGRATION_WS}
make -j`nproc` install 2>&1 |tee ${OUT_DIR-}/build_int.log || exit $?

# Specific for qemu arm-v8hf
cp ${SHARED_POOL}/images/pikeos-native-devel-qemu-arm-v8hf-qemu  ${OUT_DIR}/bin

echo "/opt/pikeos-5.1/bin/muxa"
/opt/pikeos-5.1/bin/muxa &

echo "Wait for muxa to wake up"
sleep 0.5

echo "/opt/pikeos-5.1/share/qemu/bin/qemu-system-aarch64 -m 512 -smp 4 -M virt -cpu cortex-a57 -nographic -kernel  /home/pikeos_user/S2OPC.int/boot/pikeos-native-devel-qemu-arm-v8hf-qemu  -device virtio-net-device,netdev=main  -netdev hubport,hubid=0,id=main -net tap,ifname=tap0"
/opt/pikeos-5.1/share/qemu/bin/qemu-system-aarch64 -m 512 -smp 4 -M virt -cpu cortex-a57 -nographic -kernel  /home/pikeos_user/S2OPC.int/boot/pikeos-native-devel-qemu-arm-v8hf-qemu  -device virtio-net-device,netdev=main  -netdev hubport,hubid=0,id=main -net tap,ifname=tap0
