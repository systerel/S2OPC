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
Freeopcua based test client to validate the SOPC server.
Tests:
- single connection (without encryption, then with encryption):
  - GetEndpoints,
  - Read, Write, Browse,
  - SecureChannel renewal,
- two simultaneous connections:
  - connection,
  - Write.
"""

import re
import sys

from opcua.ua import SecurityPolicy
from attribute_read import attribute_read_tests
from attribute_write_values import attribute_write_values_tests, attribute_write_values_two_clients_tests
from safety_secure_channels import secure_channels_connect
from discovery_get_endpoints import discovery_get_endpoints_tests
from client_discovery_server_services import discovery_server_tests
from view_basic import browse_tests
from translate_browse_path import translate_browse_paths_to_node_ids_tests
from common import sUri, create_client
from tap_logger import TapLogger
from opcua.crypto import security_policies

if __name__=='__main__':

    # tests with one connexion
    print('Connecting to', sUri)
    client = create_client()
    logger = TapLogger("validation.tap")
    headerString = "******************* Beginning {0} tests with one connexion *********************"
    for sp in [SecurityPolicy, security_policies.SecurityPolicyBasic256]:
    #for sp in [SecurityPolicy]:
        logger.begin_section("security policy {0}".format(re.split("#",sp.URI)[-1]))
        try:
            # secure channel connection
            secure_channels_connect(client, sp)

            # discovery server services
            discovery_server_tests(client, logger)

            # check endpoints
            discovery_get_endpoints_tests(client, logger)

            # Read tests
            print(headerString.format("Read"))
            attribute_read_tests(client, logger)

            # Force renew secure channel (nominal case)
            print(headerString.format("Renew SC"))
            client.open_secure_channel(renew=True)
            logger.add_test('OPN renew test - renewed secure channel', True)
            print("Secure channel renewed")

            # write tests
            print(headerString.format("Write"))
            attribute_write_values_tests(client, logger)

            # browse tests
            print(headerString.format("Browse"))
            browse_tests(client, logger)

            # translate browse path tests
            print(headerString.format("TranslateBrowsePathToNodeIds"))
            translate_browse_paths_to_node_ids_tests(client, logger)

        finally:
            client.disconnect()
            print('Disconnected')

    # tests with several connexions
    headerString = "******************* Beginning {0} tests with several connexions *********************"
    client2 = create_client()

    for sp in [SecurityPolicy, security_policies.SecurityPolicyBasic256]:
        logger.begin_section("security policy {0}".format(re.split("#",sp.URI)[-1]))
        try:
            # secure channel connection
            secure_channels_connect(client, sp)
            secure_channels_connect(client2, sp)

            # Read/Write tests
            print(headerString.format("Read/Write"))
            attribute_write_values_two_clients_tests(client, client2, logger)

        finally:
            client.disconnect()
            client2.disconnect()
            print('Disconnected')

    logger.finalize_report()

    sys.exit(1 if logger.has_failed_tests else 0)
