#!/usr/bin/env bash

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

set -o errexit
set -o nounset
set -o pipefail
if [[ "${TRACE-0}" == "1" ]]; then set -o xtrace; fi # debugging by running the script with the flag TRACE=1

FILE_OUTPUT_SERVER=/tmp/toolkit_stderr.txt # File that contains the standard error of the OPCUA server
FILE_BROWSE_REQUESTS=tests/ClientServer/scripts/browse_packets.txt # File that contains the browse requests responsible for the memory leaks when there is
                                             # no specific packet responsible for the leak but multiple.
WORKING_DIRECTORY=$(pwd) # Path to the root directory of the project

# Building the OPCUA server for fuzzing. Takes one argument (TRUE or FALSE). If it set to TRUE then the server accepts
# unencrypted connections.
building_server () {
    cd $WORKING_DIRECTORY

    echo -e "\n\n************************************************"
    if [[ $1 == "TRUE" ]]; then
        echo -e "BUILDING SERVER WITH NONE MODE\n\n"
    else
        echo -e "BUILDING SERVER WITHOUT NONE MODE\n\n"
    fi  

    # Cleaning potential already built files
    ./clean.sh

    # Modify the number of sockets and sessions to be able to fuzz more efficiently
    sed -i "#define SOPC_MAX_SOCKETS/c\#define SOPC_MAX_SOCKETS 1500" src/ClientServer/configuration/sopc_toolkit_config_constants.h
    sed -i "#define SOPC_MAX_SOCKETS_CONNECTIONS/c\#define SOPC_MAX_SOCKETS_CONNECTIONS 1500" src/ClientServer/configuration/sopc_toolkit_config_constants.h
    sed -i "#define SOPC_MAX_SECURE_CONNECTIONS/c\#define SOPC_MAX_SECURE_CONNECTIONS 1200" src/ClientServer/configuration/sopc_toolkit_config_constants.h
    sed -i "#define SOPC_MAX_SESSIONS/c\#define SOPC_MAX_SESSIONS 1100" src/ClientServer/configuration/sopc_toolkit_config_constants.h

    # Modify the minimum timeout accepted by the server after a session is initialized
    sed -i "/#define SOPC_MIN_SESSION_TIMEOUT/c\#define SOPC_MIN_SESSION_TIMEOUT 2000" src/ClientServer/configuration/sopc_toolkit_config_constants.h
    sed -i "/#if SOPC_MIN_SESSION_TIMEOUT </c\#if SOPC_MIN_SESSION_TIMEOUT < 1000" src/ClientServer/configuration/sopc_config_constants_check.h 

    sed -i 's/\(#define SOPC_MAX_TIMERS \)\(UINT8_MAX\)/\1UINT16_MAX/' src/Common/configuration/sopc_common_constants.h

    # building with security flags
    WITH_ASAN=1 WITH_UBSAN=1 ./build.sh

    # Changing the configuration to accept the none mode for the server
    if [[ $1 == "TRUE" ]]; then
    sed -i '/<\/SecurityPolicies>/i\
            <SecurityPolicy uri="http://opcfoundation.org/UA/SecurityPolicy#None">\
                <SecurityModes>\
                <SecurityMode mode="None"/>\
                </SecurityModes>\
                <UserPolicies>\
                <UserPolicy policyId="anon" tokenType="anonymous"/>\
                </UserPolicies>\
            </SecurityPolicy>' build/bin/S2OPC_Server_Demo_Config.xml
    fi

    #Change encrypted server key to be able to connect without a password.
    cd build/bin/server_private
    echo "password" > secret.txt
    openssl rsa -in encrypted_server_4k_key.pem -passin file:secret.txt -out server_4k_key.pem
    cd ..
    sed -i -e 's/encrypted_server_4k_key.pem/server_4k_key.pem/' -e 's/\(encrypted=\)\("true"\)/\1"false"/' S2OPC_Server_Demo_Config.xml 

    #Change the type of server of the demo server.
    sed -i '\,</ApplicationDescription>,i <ApplicationType type="DiscoveryServer"/>' S2OPC_Server_Demo_Config.xml

    echo -e "************************************************\n\n"
}

# Running the server, redirecting its standard error to FILE_OUTPUT_SERVER
launching_server () {
    cd $WORKING_DIRECTORY
    cd build/bin
    touch "$FILE_OUTPUT_SERVER"

    echo -e "\n\n########################################################"
    echo "LAUNCHING OPCUA SERVER\n\n"

    export TEST_SERVER_XML_CONFIG=./S2OPC_Server_Demo_Config.xml TEST_SERVER_XML_ADDRESS_SPACE=./S2OPC_Demo_NodeSet.xml TEST_USERS_XML_CONFIG=./S2OPC_Users_Demo_Config.xml
    ./toolkit_demo_server 2> "$FILE_OUTPUT_SERVER" & # Redirecting standard error to a file
    sleep 5

    echo -e "\n\nSERVER IS RUNNING"
    echo -e "########################################################\n\n"
}

# Trying to run a specific bug depending on the argument given (an integer)
# 1: Ticket 1434 https://gitlab.com/systerel/S2OPC/-/issues/1434
# 2: Ticket 1442 https://gitlab.com/systerel/S2OPC/-/issues/1442
# 3: Ticket 1444 https://gitlab.com/systerel/S2OPC/-/issues/1444
# 4: Ticket 1450 https://gitlab.com/systerel/S2OPC/-/issues/1450
# 5: Ticket 1449 https://gitlab.com/systerel/S2OPC/-/issues/1449
run_bug () {
    cd $WORKING_DIRECTORY

    echo -e "\nTRYING BUG $1\n"
    python3 tests/ClientServer/scripts/ci_fuzz_ml.py $1
    echo -e "\nBUG $1 FINISHED\n"
}

# Killing gently the OPCUA server to have the potential trace of memory leaks
killing_server () {
    #pid_server="$(ps aux | grep toolkit_demo_server | grep -v grep | tr -s " " | cut -d ' ' -f 2)"
    pid_server=$(pidof toolkit_demo_server)
    echo -e "\n\n########################################################"
    echo "KILLING OPCUA SERVER WITH PID $pid_server"
    kill "$pid_server"
    sleep 15
    echo "SERVER KILLED"
    echo -e "########################################################\n\n"
}

# Checking potential memory leaks
checking_memory_leak () {
    cd $WORKING_DIRECTORY

    echo -e "CHECKING FOR MEMORY LEAKS\n"
    if [[ "$(cat $FILE_OUTPUT_SERVER | grep ERROR | wc -l)" == "1" ]]; then
        echo -e "MEMORY LEAKS FOUND\n"
        cat $FILE_OUTPUT_SERVER
        if [[ $1 -eq 4 ]]; then
            echo "File containing the browse requests leading to the memory leaks: $FILE_BROWSE_REQUESTS. Each packet ends with the bytes \x99\x98\x97\x96."
        fi
        exit 1
    fi
    echo -e "LEAK CHECKS DONE\n\n"
}

# Checking if potential OPCUA servers are already running and if so killing them
checking_running_server () {
    echo -e "\n\n------------------------------------------------\n"
    echo -e "CHECKING FOR RUNNING SERVERS AND KILLING THEM\n\n"

    server_running="$(ps aux | grep toolkit_demo_server | wc -l)" 
    while [ $server_running -ge 2 ] # there is a process in addition for grep
    do
        pid_server="$(ps aux | grep toolkit_demo_server | grep -v grep | head -n 1 | tr -s " " | cut -d ' ' -f 2)"
        echo -e "KILLING SERVER WITH PID $pid_server \n"
        kill -9 $pid_server
        echo "SERVER KILLED"
        sleep 2
        server_running=$(ps aux | grep toolkit_demo_server | wc -l)
    done

    echo "CHECKING FINISHED"
}


##################################################################
#                   MAIN
##################################################################

building_server TRUE
for bug in 1 2 3 4 5
do
    #checking_running_server
    if [[ $bug -eq 5 ]]; then # For bug 5, building the server without the possibility to use the None Mode
        building_server FALSE
    fi  
    launching_server
    run_bug $bug
    killing_server
    checking_memory_leak $bug
done

exit 0


