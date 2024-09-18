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

# Functionnal script to manage serial ports

##########################
# Getter function to retrieve the name of the port to which a board is connected
# @param 'SERIAL' : The serial number of the board
function get_dev() {
    SERIAL=$1
    for DEV in /dev/ttyACM* ; do
        if udevadm info --name=${DEV} |grep -q "ID_SERIAL_SHORT.*${SERIAL}" ; then
            echo $DEV
        fi
    done
}

##########################
# Initialisation function to configure a serial port
# @param 'PORT' : The serial port to configure
#        'COMM_OPTION' : Contains the stty parameters for serial communication
function init_port() {
    PORT=$1
    COMM_OPTION=$2
    stty -F $PORT $COMM_OPTION
}

##########################
# Writing function to write on a serial port
# @param 'PORT' : The serial port to write to
function write_protocol() {
    PORT=$1
    [[ -z "$PORT" ]] && echo "MIssing parameter" && exit 1
    shift
    echo $* > $PORT
}

##########################
# Reading function to read from a serial port
# @param 'PORT' : The serial port to read from
#        'TIMEOUT' : Time to wait to capture text from the serial port
function read_protocol() {
    PORT=$1
    TIMEOUT=$2
    [[ -z "$PORT" ]] && echo "MIssing parameter" && exit 1
    if [[ -z "$TIMEOUT" ]] ; then
        cat <$PORT
        else
            timeout $TIMEOUT cat <$PORT
    fi
}

if [ "$1" == "get_dev" ]; then
    get_dev "$2"
elif [ "$1" == "init_port" ]; then
    init_port "$2" "$3"
elif [ "$1" == "write_protocol" ]; then
    write_protocol "$2" "$3"
elif [ "$1" == "read_protocol" ]; then
    read_protocol "$2" "$3"
else
    echo "Please use one of the following : get_dev, init_port, write_protocol, read_protocol"
fi
