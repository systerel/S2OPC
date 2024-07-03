#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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


"""
Example script: this script initializes the toolkit as a server.
"""


import time
import argparse
import os
import getpass

from pys2opc import PyS2OPC_Server, BaseAddressSpaceHandler, StatusCode, AttributeId

TEST_PASSWORD_PRIV_KEY_ENV_NAME = "TEST_PASSWORD_PRIVATE_KEY"

class AddressSpaceHandler(BaseAddressSpaceHandler):
    def on_datachanged(self, nodeId, attrId, dataValue, indexRange, status):
        print("Data change :", nodeId, AttributeId.get_name_from_id(attrId), dataValue, indexRange, StatusCode.get_name_from_id(status))

class PyS2OPC_Server_Test():
    @staticmethod
    def get_server_key_password():
        pwd = os.getenv(TEST_PASSWORD_PRIV_KEY_ENV_NAME)
        if pwd is None:
            print("{} not set: set it or enter it interactively".format(TEST_PASSWORD_PRIV_KEY_ENV_NAME))
            pwd = getpass.getpass(prompt='Server private key password:')
        return pwd

# overload the default password method for tests
PyS2OPC_Server.get_server_key_password = PyS2OPC_Server_Test.get_server_key_password

if __name__ == '__main__':
    print(PyS2OPC_Server.get_version())
    print()

    parser = argparse.ArgumentParser(description='Configurable OPC UA server')
    parser.add_argument('--config-path', default='S2OPC_Server_Demo_Config.xml',
                        help='The path to the XML configuration of the server endpoints')
    parser.add_argument('--users-path', default='S2OPC_Users_Demo_Config.xml',
                        help='The path to the XML configuration of the user authentications and authorization')
    parser.add_argument('--addspace-path', default='S2OPC_Demo_NodeSet.xml',
                        help='The path to the XML configuration of the server address space')
    parser.add_argument('--log-path', default='./pys2opc_logs/',)
    args = parser.parse_args()

    with PyS2OPC_Server.initialize(logPath=args.log_path):
        PyS2OPC_Server.load_server_configuration_from_files(xml_server_config_path = args.config_path,
                                                     xml_address_space_config_path = args.addspace_path,
                                                     xml_users_config_path = args.users_path,
                                                     address_space_handler=AddressSpaceHandler())
        PyS2OPC_Server.serve_forever()  # Should return exit reason
        # with PyS2OPC_Server.serve():
        #    while PyS2OPC_Server.serving():
        #        time.sleep(1.)