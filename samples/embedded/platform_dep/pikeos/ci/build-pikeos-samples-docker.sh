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

function _help() {
    echo "$1 Pikeos helper script. This script must be called from within the docker."
    echo "$1 build [BUILD_OPTIONS]"
    echo "Build the library, samples and test in a context where PikeOS projects are well configured."
    echo ""
    echo "$1 exec [unit_test|cli_pubsub_server|cli_client] [BUILD_OPTIONS]"
    echo "Build the library, tests, samples and the PikeOS kernel then launch selected binary on QEMU x86 board. By default will launch unit test"
    echo ""
    echo "$1 help"
    echo "show this help"
    echo ""
    echo " BUILD_OPTIONS :"
    echo "          --no-tests      Don't compile unit tests"
    echo "          --no-samples    Don't compile samples"

}

BUILD_SAMPLES=true
BUILD_TESTS=true
# Can take following values
# unit_test, cli_pubsub_server, cli_client
EXEC_BIN="unit_test"
BUILD_ONLY=false

while [[ $# -gt 0 ]]; do
    case $1 in
        -- )
            ;;
        build )
            BUILD_ONLY=true
            ;;
        --no-tests )
            BUILD_TESTS=false
            ;;
        --no-samples )
            BUILD_SAMPLES=false
            ;;
        exec )
            shift
            [[ $# -eq 0 ]] && break
            ${BUILD_ONLY} && echo "Cannot use both exec and build command" && exit 2
            case $1 in
                cli_pubsub_server )
                    EXEC_BIN="cli_pubsub_server"
                    ;;
                cli_client )
                    EXEC_BIN="cli_client"
                    ;;
                unit_test )
                    EXEC_BIN="unit_test"
                    ;;
                * )
                    echo "Unknown option: $1"
                    echo "Valid option with exec are [unit_test|cli_pubsub_server|cli_client]"
                    exit 3
                    ;;
            esac
            ;;
        h| help| -h| --help )
            _help $0
            ;;
        * )
            echo "Unknonw option: $1"
            _help $0
            exit 1
            ;;
    esac
    shift
done


cd `dirname $0`/../../../../..
export SOPC_ROOT=`pwd`
echo "SOPC_ROOT=$SOPC_ROOT"

export CI_DIR=${SOPC_ROOT}/samples/embedded/platform_dep/pikeos/ci

APPLICATION_WS=/home/pikeos_user/S2OPC.app
INTEGRATION_WS=/home/pikeos_user/S2OPC.int

SHARED_POOL=/home/pikeos_user/shared_pool

export OUT_DIR=${SOPC_ROOT-}/build_pikeos

sudo ip tuntap add mode tap user pikeos_user
sudo ip addr add 192.168.8.4/24 dev tap0
sudo ip link set tap0 up


mkdir -p ${OUT_DIR} ${OUT_DIR}/bin ${OUT_DIR}/lib ${OUT_DIR}/tests

/usr/lmx-${LMX_VERSION}/lmx-serv -b || exit $*

cd ${APPLICATION_WS}
! ${BUILD_SAMPLES}  && echo "Don't compile samples"  && sed -i -e '/S2OPC_SAMPLES_ENABLE = true/s/true/false/' makefile.defs
! ${BUILD_TESTS} && echo "Don't compile tests" && sed -i -e '/S2OPC_TEST_ENABLE = true/s/true/false/' makefile.defs
make -j`nproc` install 2>&1 |tee ${OUT_DIR-}/build_app.log || exit $?

cp ${SHARED_POOL}/pikeos-native/object/*.elf ${OUT_DIR}/bin
cp ${SHARED_POOL}/pikeos-native/object/*.a  ${OUT_DIR}/lib

if ! $BUILD_ONLY ; then
    cd ${INTEGRATION_WS}
    [[ ${EXEC_BIN} == "unit_test" ]] && echo "" && echo "Configure project to execute unit test" && echo "" && /opt/pikeos-5.1/bin/pikeos-projectconfigurator --command='goto "PikeOS";goto "PikeOS Native Process";set FILE CUSTOM_POOL/pikeos-native/object/unit_test.elf;save'
    [[ ${EXEC_BIN} == "cli_pubsub_server" ]] && echo "" && echo "Configure project to execute cli_pubsub_server" && echo "" && /opt/pikeos-5.1/bin/pikeos-projectconfigurator --command='goto "PikeOS";goto "PikeOS Native Process";set FILE CUSTOM_POOL/pikeos-native/object/cli_pubsub_server.elf;save'
    [[ ${EXEC_BIN} == "cli_client" ]] && echo "" && echo "Configure project to execute cli_client" && echo "" && /opt/pikeos-5.1/bin/pikeos-projectconfigurator --command='goto "PikeOS";goto "PikeOS Native Process";set FILE CUSTOM_POOL/pikeos-native/object/cli_client.elf;save'
    make -j`nproc` install 2>&1 |tee ${OUT_DIR-}/build_int.log || exit $?

    # Specific for qemu arm-v8hf
    cp ${SHARED_POOL}/images/pikeos-native-devel-qemu-arm-v8hf-qemu  ${OUT_DIR}/bin

    echo "/opt/pikeos-5.1/bin/muxa"
    /opt/pikeos-5.1/bin/muxa &

    echo "Wait for muxa to wake up"
    sleep 0.5


    if [ ${EXEC_BIN} == "unit_test" ]; then
        # Clean last unit test output
        rm -f ${OUT_DIR}/tests/*

        # Launch QEMU simulation in background and redirect serial output to file
        /opt/pikeos-5.1/share/qemu/bin/qemu-system-aarch64 -m 512 -smp 4 -M virt -cpu cortex-a57 -nographic -kernel  /home/pikeos_user/S2OPC.int/boot/pikeos-native-devel-qemu-arm-v8hf-qemu  -device virtio-net-device,netdev=main  -netdev hubport,hubid=0,id=main -net tap,ifname=tap0 -serial file:${OUT_DIR}/tests/${EXEC_BIN}_out.txt &

        # Test parser script
        ${CI_DIR}/check_pikeos_test.py ${OUT_DIR}/tests/${EXEC_BIN}_out.txt --verbose
        EXIT_CODE=$?
        if [ ${EXIT_CODE} -eq 0 ]; then
            echo "Unit test succeed !"
        else
            echo "Unit test failed with code ${EXIT_CODE}"
        fi
        exit $EXIT_CODE
    fi

    # Launch samples in QEMU simulation
    /opt/pikeos-5.1/share/qemu/bin/qemu-system-aarch64 -m 512 -smp 4 -M virt -cpu cortex-a57 -nographic -kernel  /home/pikeos_user/S2OPC.int/boot/pikeos-native-devel-qemu-arm-v8hf-qemu  -device virtio-net-device,netdev=main  -netdev hubport,hubid=0,id=main -net tap,ifname=tap0
fi

