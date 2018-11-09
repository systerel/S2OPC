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
Tests that HEL/ACK and SecureChannel timeouts are respected by the server.
"""

from time import sleep
import sys

from common import sUri, create_client
from tap_logger import TapLogger

if __name__=='__main__':

    # tests with one connexion
    print('Connecting to', sUri)
    client = create_client()
    logger = TapLogger("sc_establish_timeout.tap")
    headerString = "******************* {0} *********************"

    logger.begin_section("SC establishment timeout")
    # secure channel connection

    print(headerString.format("Connect socket"))
    client.connect_socket()

    print(headerString.format("Wait SC establishment timeout (10 seconds)"))
    sleep(11)

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
    sleep(11)

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

    sys.exit(1 if logger.has_failed_tests else 0)
