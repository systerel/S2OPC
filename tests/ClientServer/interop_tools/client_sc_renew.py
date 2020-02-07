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
Tests that the server renew the SecureChannel and revises the timeout correctly,
and does not accept messages after the specified timeout.
"""

from time import sleep
import re
import sys
import concurrent.futures

from opcua.ua import SecurityPolicy
from safety_secure_channels import secure_channels_connect
from common import sUri, create_client
from tap_logger import TapLogger
from opcua.crypto import security_policies

def secure_channel_renew_nominal(client, logger):
    # Define renew time to 1 second
    client.secure_channel_timeout=1000
    # Renew with 1 second
    client.open_secure_channel(renew=True)
    print('Open Secure Channel renewed')
    # Check revised time
    logger.add_test('OPN renew test - renewed with given timeout value', client.secure_channel_timeout == 1000)
    # Read a node to be sure we are using the new security token
    nid_index = 1001
    nid = u"ns=1;i={}".format(nid_index)
    node = client.get_node(nid)
    value = node.get_value()
    print(' Value for Node {}:'.format(nid), value)
    print(' Error expected on next read:')

def secure_channel_renew_test_read_failure(client, logger):
    # Define renew time to 1 second
    client.secure_channel_timeout=1000
    # Renew with 1 second
    client.open_secure_channel(renew=True)
    print('Open Secure Channel renewed')
    # Check revised time
    logger.add_test('OPN renew test - renewed with given timeout value', client.secure_channel_timeout == 1000)
    # Change revised time to avoid client to renew the security token in time
    client.secure_channel_timeout=10000
    # Read a node to be sure we are using the new security token
    nid_index = 1001
    nid = u"ns=1;i={}".format(nid_index)
    node = client.get_node(nid)
    value = node.get_value()
    print(' Value for Node {}:'.format(nid), value)
    # Wait timeout of the security token
    sleep(2)
    print(' Error expected on next read:')
    # Try to read a node again
    try:
        node = client.get_node(nid)
        value = node.get_value()
    except:
        logger.add_test('OPN renew test - read refused after timeout', True)
    else:
        logger.add_test('OPN renew test - read refused after timeout', False)


if __name__=='__main__':

    # tests with one connexion
    print('Connecting to', sUri)
    client = create_client()
    logger = TapLogger("sc_renew.tap")

    # tests of SC renew with degraded cases
    headerString = "******************* Beginning {0} test of degraded SC renew *********************"
    for sp in [SecurityPolicy, security_policies.SecurityPolicyBasic256]:
        logger.begin_section("security policy {0}".format(re.split("#",sp.URI)[-1]))
        # secure channel connection
        print(headerString.format(re.split("#",sp.URI)[-1]))
        try:
            secure_channels_connect(client, sp)
            for i in range(0,1):
                secure_channel_renew_nominal(client, logger)
            secure_channel_renew_test_read_failure(client, logger)
        finally:
            try:
                client.disconnect()
            except (concurrent.futures.TimeoutError, TimeoutError, OSError):
                pass

    logger.finalize_report()

    sys.exit(1 if logger.has_failed_tests else 0)
