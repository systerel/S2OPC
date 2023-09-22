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

from pys2opc import PyS2OPC_Server as PyS2OPC, BaseAddressSpaceHandler, DataValue, StatusCode, AttributeId#, Variant, VariantType
#from _connection_configuration import configuration_parameters_no_subscription

TEST_PASSWORD_PRIV_KEY_ENV_NAME = "TEST_PASSWORD_PRIVATE_KEY"

class AddressSpaceHandler(BaseAddressSpaceHandler):
    def on_datachanged(self, nodeId, attrId, dataValue, indexRange, status):
        print(nodeId, AttributeId.get_name_from_id(attrId), dataValue, indexRange, StatusCode.get_name_from_id(status))

class PyS2OPC_Server_Test():
    @staticmethod
    def get_server_key_password():
        pwd = os.getenv(TEST_PASSWORD_PRIV_KEY_ENV_NAME)
        if pwd is None:
            print("{} not set: set it or enter it interactively".format(TEST_PASSWORD_PRIV_KEY_ENV_NAME))
            pwd = getpass.getpass(prompt='Server private key password:')
        return pwd

# overload the default password method for tests
PyS2OPC.get_server_key_password = PyS2OPC_Server_Test.get_server_key_password

if __name__ == '__main__':
    print(PyS2OPC.get_version())
    print()

    parser = argparse.ArgumentParser(description='Configurable OPC UA server')
    parser.add_argument('--config-path', default='S2OPC_Server_Demo_Config.xml',
                        help='The path to the XML configuration of the server endpoints')
    parser.add_argument('--users-path', default='S2OPC_Users_Demo_Config.xml',
                        help='The path to the XML configuration of the user authentications and authorization')
    parser.add_argument('--addspace-path', default='S2OPC_Demo_NodeSet.xml',
                        help='The path to the XML configuration of the server address space')
    parser.add_argument('--log-path', default='/tmp/pys2opc_logs/',)
    args = parser.parse_args()

    with PyS2OPC.initialize(logPath=args.log_path):
        # Thread safety on callbacks?
        PyS2OPC.load_server_configuration_from_files(args.addspace_path,
                                                     args.users_path,
                                                     args.config_path,
                                                     address_space_handler=AddressSpaceHandler())
        PyS2OPC.mark_configured()
        PyS2OPC.serve_forever()  # Should return exit reason
        #with PyS2OPC.serve():
        #    while PyS2OPC.serving():
        #        time.sleep(1.)
