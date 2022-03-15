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

from pys2opc import PyS2OPC_Server as PyS2OPC, BaseAddressSpaceHandler, DataValue, StatusCode, AttributeId#, Variant, VariantType
#from _connection_configuration import configuration_parameters_no_subscription


class AddressSpaceHandler(BaseAddressSpaceHandler):
    def on_datachanged(self, nodeId, attrId, dataValue, indexRange, status):
        print(nodeId, AttributeId.get_name_from_id(attrId), dataValue, indexRange, StatusCode.get_name_from_id(status))


if __name__ == '__main__':
    print(PyS2OPC.get_version())
    print()

    parser = argparse.ArgumentParser(description='Configurable OPC UA server')
    parser.add_argument('--config-path', default='S2OPC_Server_Demo_Config.xml',
                        help='The path to the XML configuration of the server endpoints')
    parser.add_argument('--addspace-path', default='S2OPC_Demo_NodeSet.xml',
                        help='The path to the XML configuration of the server address space')
    parser.add_argument('--log-path', default='/tmp/pys2opc_logs',)
    args = parser.parse_args()

    with PyS2OPC.initialize(logPath=args.log_path):
        # Thread safety on callbacks?
        PyS2OPC.load_address_space(args.addspace_path)
        PyS2OPC.load_configuration(args.config_path,
                                   address_space_handler=AddressSpaceHandler(),
                                   user_handler=None,
                                   method_handler=None,
                                   pki_handler=None)
        PyS2OPC.mark_configured()
        PyS2OPC.serve_forever()  # Should return exit reason
        #with PyS2OPC.serve():
        #    while PyS2OPC.serving():
        #        time.sleep(1.)
