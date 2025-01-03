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

# Fuzz all possible requests 
export TEST_SERVER_XML_CONFIG=../../tests/ClientServer/data/config/S2OPC_Server_Demo_Config_Fuzzer.xml TEST_SERVER_XML_ADDRESS_SPACE=./S2OPC_Demo_NodeSet.xml TEST_USERS_XML_CONFIG=./S2OPC_Users_Demo_Config.xml
request_list=(read_request browse_request create_subscription_request browse_next_request add_nodes_request)

for request in "${request_list[@]}"
do 
    echo FUZZING "$request"
    # Timeout is set to 4 hours for every request
    if python3 tests/ClientServer/scripts/with-opc-server.py --server-wd build_fuzz/bin --server-cmd ./toolkit_demo_server --wait-timeout 14400 "python3" /work/opcua_network_fuzzer-main/opcua_fuzzer.py --target_host_ip localhost --target_host_port 4841 --target_app_name s2opc --r "$request" ; then
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

    