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

from pys2opc import PyS2OPC_Server as PyS2OPC, BaseAddressSpaceHandler#, DataValue, StatusCode, Variant, VariantType
#from _connection_configuration import configuration_parameters_no_subscription


class AddressSpaceHandler(BaseAddressSpaceHandler):
    def on_datachanged(self, nodeId, attrId, dataValue, indexRange, status):
        print(nodeId, attrId, dataValue, indexRange, status)


if __name__ == '__main__':
    print(PyS2OPC.get_version())
    print()

    with PyS2OPC.initialize(logPath='/tmp/pys2opc_logs'):
        print('Initialized')
        # Thread safety on callbacks?
        #config = PyS2OPC.load_configuration(xml_path, pki_allow_all=False)  # Endpoint, XML ? PKI ?
        PyS2OPC.load_address_space('build/bin/s2opc.xml')
        PyS2OPC.set_connection_handlers(address_space_notifier=AddressSpaceHandler(),
                                        user_handler=None,
                                        method_handler=None)
        #PyS2OPC.mark_configured()
        #PyS2OPC.serve_forever(config)  # Should return exit reason
        ## AsyncClose()
        ## Clear()
        time.sleep(1.)
