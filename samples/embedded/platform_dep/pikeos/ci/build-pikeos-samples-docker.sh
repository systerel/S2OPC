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
    echo "$1 build PikeOS kernel with unit_test and pikeos samples in principal partition. This script must be called from within the docker."
    echo "$1 [--no-tests] [--no-samples] [help]"
    echo "Build the library, samples and test in a context where PikeOS projects are well configured."
    echo ""
    echo "      help            show this help"
    echo "      --no-tests      Don't compile unit tests"
    echo "      --no-samples    Don't compile samples"

}

BUILD_SAMPLES=true
BUILD_TESTS=true

while [[ $# -gt 0 ]]; do
    case $1 in
        -- )
            ;;
        --no-tests )
            BUILD_TESTS=false
            ;;
        --no-samples )
            BUILD_SAMPLES=false
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

mkdir -p ${OUT_DIR} ${OUT_DIR}/bin ${OUT_DIR}/lib ${OUT_DIR}/tests
chmod -R 777 ${OUT_DIR}

BUILD_LOG=${OUT_DIR-}/build_pikeos.log


function build_app()
{
    cd ${APPLICATION_WS}
    echo "Change S2OPC project location in makefiles.defs to ${SOPC_ROOT}"
    SOPC_ROOT_SED_FRIENDLY=${SOPC_ROOT//\//\\\/}
    sed -i -e "/S2OPC_PRJ_DIR = \/s2opc/s/\/s2opc/${SOPC_ROOT_SED_FRIENDLY}/" makefile.defs
    ! ${BUILD_SAMPLES}  && echo "Don't compile samples"  && sed -i -e '/S2OPC_SAMPLES_ENABLE = true/s/true/false/' makefile.defs
    ! ${BUILD_TESTS} && echo "Don't compile tests" && sed -i -e '/S2OPC_TEST_ENABLE = true/s/true/false/' makefile.defs
    make -j`nproc` install 2>&1 |tee ${BUILD_LOG} || exit $?

    (${BUILD_SAMPLES} || ${BUILD_TESTS}) && cp ${SHARED_POOL}/pikeos-native/object/*.elf ${OUT_DIR}/bin
    cp ${SHARED_POOL}/pikeos-native/object/*.a  ${OUT_DIR}/lib
}

function build_pikeos()
{
    # Binary attach to PikeOS partition
    # Can take value unit_test, cli_pubsub_server, cli_client
    EXEC_BIN=$1
    cd ${INTEGRATION_WS}
    [[ ${EXEC_BIN} == "unit_test" || ${EXEC_BIN} == "cli_pubsub_server" || ${EXEC_BIN} == "cli_client" ]] && echo "" && echo "Configure project to execute ${EXEC_BIN}" && echo "" && /opt/pikeos-5.1/bin/pikeos-projectconfigurator --command="goto \"PikeOS\";goto \"PikeOS Native Process\";set FILE CUSTOM_POOL/pikeos-native/object/${EXEC_BIN}.elf;save"

    make -j`nproc` install 2>&1 |tee -a ${BUILD_LOG} || exit $?

    # Specific for qemu arm-v8hf.
    echo "cp ${SHARED_POOL}/images/pikeos-native-devel-qemu-arm-v8hf-qemu  ${OUT_DIR}/bin/pikeos-native-devel-qemu-arm-v8hf-qemu-${EXEC_BIN}"
    cp ${SHARED_POOL}/images/pikeos-native-devel-qemu-arm-v8hf-qemu  ${OUT_DIR}/bin/pikeos-native-devel-qemu-arm-v8hf-qemu-${EXEC_BIN}
}

# Launch certificate manager
/usr/lmx-${LMX_VERSION}/lmx-serv -b || exit $*

build_app

${BUILD_SAMPLES} && build_pikeos "cli_pubsub_server" && build_pikeos "cli_client"
${BUILD_TESTS} && build_pikeos "unit_test"
