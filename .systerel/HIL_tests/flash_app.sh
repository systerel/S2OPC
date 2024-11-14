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

# Script to flash S2OPC demo application for HIL testing
# @param $1: 'DEVICE_ID' : The serial number of the board, used to create a mounting point
#        $2: 'P_FILE' : The name of app to flash
#        $3: 'OS' : The operating system name to differenciate which folder the app was built to
# Creates a mounting point with the serial number of the board, then copies the app on it
#
# To add boards, a new line needs to be added to /etc/fstab
#     /dev/disk/by-id/<ID of the board> /media/HIL/<Serial number of the board> vfat user 0 0
# For example for one of the STM32 nucleo board, this line was added :
#     /dev/disk/by-id/usb-MBED_microcontroller_001900273331510B33323639-0:0 /media/HIL/001900273331510B33323639 vfat user 0 0

# For now only .bin files are supported
cd $(dirname $0)
HIL_DIR=$(pwd)
cd ../../
HOST_DIR=$(pwd)
EMB_DIR=${HOST_DIR}/samples/embedded

##########################
# Helper function to exit with an error message
# @param $* An error message
function fail() {
    echo -e "[EE] $*" >&2
    exit -1
}

DEVICE_ID=$1
[ -z "${DEVICE_ID}" ] && fail "Missing 'DEVICE_ID' for flash_app.sh"
P_FILE=$2
[ -z "${P_FILE}" ] && fail "Missing 'P_FILE' for flash_app.sh"
[[ $P_FILE =~ .*[.]bin ]] || fail "Only .bin files are supported currently"
OS=$3
[ -z "${OS}" ] && fail "Missing 'OS' for flash_app.sh"
MOUNT="/media/HIL/${DEVICE_ID}"
BUILD_FOLDER="${HOST_DIR}/build_${OS}"
[ -d "$BUILD_FOLDER" ] || fail "build folder : ${BUILD_FOLDER} does not exist"
DEV=$( ls /dev/disk/by-id/ | grep "${DEVICE_ID}" )
if ! [ -z "$DEV" ] ; then
    echo "Found disk as '${DEV}'"
    umount "$MOUNT" 2> /dev/null
    mkdir -p "$MOUNT"
    mount "$MOUNT" || fail "failed to mount $MOUNT"
    echo "$DEV mounted on $MOUNT"
else
    fail "Please connect the board with SN:${DEVICE_ID}"
fi

# Application as .BIN file can be installed by copying it on device mount point.
if [ -f "${BUILD_FOLDER}/${P_FILE}" ] ; then
    cp "$BUILD_FOLDER/$P_FILE" "$MOUNT" || exit 4
    sync
    cp "$BUILD_FOLDER/$P_FILE" "$BUILD_FOLDER/$P_FILE".last
    echo "... copied to $MOUNT "
else
    fail "The file ${P_FILE} doesn't exist"
fi
