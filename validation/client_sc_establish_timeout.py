#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (C) 2018 Systerel and others.
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
from time import sleep
import re

if __name__=='__main__':

    # tests with one connexion
    print('Connecting to', sUri)
    client = Client(sUri)
    logger = TapLogger("sc_establish_timeout.tap")
    headerString = "******************* {0} *********************"

    logger.begin_section("SC establishment timeout")
    # secure channel connection

    print(headerString.format("Connect socket"))
    client.connect_socket()

    print(headerString.format("Wait SC establishment timeout (10 seconds)"))
    sleep(10)

    print(headerString.format("Attempt to send HEL after timeout"))
    try:
        client.send_hello()
    except:
        logger.add_test('- HEL refused after timeout', True)
    else:
        logger.add_test('- HEL refused after timeout', False)
        client.disconnect_socket()

    print(headerString.format("Connect socket + Send HEL"))
    client.connect_socket()
    client.send_hello()

    print(headerString.format("Wait SC establishment timeout (10 seconds)"))
    sleep(10)

    print(headerString.format("Attempt to send OPN after timeout"))
    try:
        client.open_secure_channel()
    except:
        logger.add_test('- OPN refused after timeout', True)
    else:
        logger.add_test('- OPN refused after timeout', False)
        client.close_secure_channel()
        client.disconnect_socket()


    logger.finalize_report()
