#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (C) 2017 Systerel and others.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

"""
Simple client to launch validation tests
"""

from opcua import ua, Client
from opcua.ua import SecurityPolicy
from attribute_read import attribute_read_tests
from attribute_write_values import attribute_write_values_tests, attribute_write_values_two_clients_tests
from safety_secure_channels import secure_channels_connect
from discovery_get_endpoints import discovery_get_endpoints_tests
from view_basic import browse_tests
from sc_renew import secure_channel_renew
from common import sUri
from tap_logger import TapLogger
from opcua.crypto import security_policies
import re

if __name__=='__main__':

    # tests with one connexion
    print('Connecting to', sUri)
    client = Client(sUri)
    logger = TapLogger("validation.tap")
    headerString = "******************* Beginning {0} tests with one connexion *********************"
    for sp in [SecurityPolicy, security_policies.SecurityPolicyBasic256]:
    #for sp in [SecurityPolicy]:
        logger.begin_section("security policy {0}".format(re.split("#",sp.URI)[-1]))
        try:
            # secure channel connection
            secure_channels_connect(client, sp)

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

        finally:
            client.disconnect()
            print('Disconnected')

    # tests with several connexions
    headerString = "******************* Beginning {0} tests with several connexions *********************"
    client2 = Client(sUri)

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
