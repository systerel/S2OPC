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

set -o errexit  
set -o nounset                                                                                          
set -o pipefail 

#Increase the number of sessions.
sed -i -e 's/\(#define SOPC_MAX_SOCKETS \)\(150\)/\11500/' -e 's/\(#define SOPC_MAX_SOCKETS_CONNECTIONS \)\(50\)/\11500/' src/ClientServer/configuration/sopc_toolkit_config_constants.h
sed -i -e 's/\(#define SOPC_MAX_SECURE_CONNECTIONS \)\(21\)/\11001/' -e 's/\(#define SOPC_MAX_SESSIONS \)\(20\)/\11000/' src/ClientServer/configuration/sopc_toolkit_config_constants.h

sed -i 's/\(#define SOPC_MAX_TIMERS \)\(UINT8_MAX\)/\1UINT16_MAX/' src/Common/configuration/sopc_common_constants.h

#Rebuild S2OPC
./build.sh && cd build && cmake --build . --target install || exit $?

#Change encrypt server key for server key without password.
cd bin/server_private
echo "${TEST_PASSWORD_PRIVATE_KEY}" > secret.txt
openssl rsa -in encrypted_server_4k_key.pem -passin file:secret.txt -out server_4k_key.pem
cd ..
sed -i -e 's/encrypted_server_4k_key.pem/server_4k_key.pem/' -e 's/\(encrypted=\)\("true"\)/\1"false"/' S2OPC_Server_Demo_Config.xml 

#Change the type of server of the demo server.
sed -i '\,</ApplicationDescription>,i <ApplicationType type="DiscoveryServer"/>' S2OPC_Server_Demo_Config.xml

#Download opcua_network_fuzzer requirements
python3 -m pip install -r /work/opcua_network_fuzzer-main/requirements.txt

# Fuzz all possible requests 
export TEST_SERVER_XML_CONFIG=./S2OPC_Server_Demo_Config.xml TEST_SERVER_XML_ADDRESS_SPACE=./S2OPC_Demo_NodeSet.xml TEST_USERS_XML_CONFIG=./S2OPC_Users_Demo_Config.xml
request_list=(read_request browse_request create_subscription_request browse_next_request add_nodes_request)
cd ../..
for request in "${request_list[@]}"
do 
    echo FUZZING "$request"
    if !  timeout --preserve-status --verbose -k 30s 4h python3 tests/ClientServer/scripts/with-opc-server.py --server-wd build/bin --server-cmd ./toolkit_demo_server "python3" /work/opcua_network_fuzzer-main/opcua_fuzzer.py --target_host_ip localhost --target_host_port 4841 --target_app_name s2opc --r "$request" ; then
        exit_status=$?
        echo fuzzing of "$request" ended with SUCCESS! 
        echo Exit status : $exit_status
        
    else
        crash_status=$?
        echo "$request" ended with an ERROR!
        echo Exit status : $crash_status
        exit $crash_status
    fi
    
done

    