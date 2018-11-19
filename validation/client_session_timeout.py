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
Tests SecureChannel renewal timeouts revision, and that the server respects the timeouts.
"""

from time import sleep
import re
import sys

from opcua import ua, Client
from opcua.common import utils
from opcua.ua import SecurityPolicy
from safety_secure_channels import secure_channels_connect
from common import sUri
from tap_logger import TapLogger
from opcua.crypto import security_policies


SESSION_TIMEOUT     = 10000
MIN_SESSION_TIMEOUT = 10000
MAX_SESSION_TIMEOUT = 600000

class Client2(Client):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.session_timeout = SESSION_TIMEOUT  # Default value

    # Overriding create session method to request 10 seconds timeout and deactivate keepalive sending
    def create_session(self):
        """
        send a CreateSessionRequest to server with reasonable parameters.
        If you want o modify settings look at code of this methods
        and make your own
        """
        desc = ua.ApplicationDescription()
        desc.ApplicationUri = "urn:S2OPC:localhost"
        desc.ProductUri = self.product_uri
        desc.ApplicationName = ua.LocalizedText(self.name)
        desc.ApplicationType = ua.ApplicationType.Client

        params = ua.CreateSessionParameters()
        nonce = utils.create_nonce(32)  # at least 32 random bytes for server to prove possession of private key (specs part 4, 5.6.2.2)
        params.ClientNonce = nonce
        params.ClientCertificate = self.security_policy.client_certificate
        params.ClientDescription = desc
        params.EndpointUrl = self.server_url.geturl()
        params.SessionName = self.description + " Session" + str(self._session_counter)
        # START MODIFICATION
        params.RequestedSessionTimeout = self.session_timeout
        # END MODIFICATION
        params.MaxResponseMessageSize = 0  # means no max size
        response = self.uaclient.create_session(params)
        if self.security_policy.client_certificate is None:
            data = nonce
        else:
            data = self.security_policy.client_certificate + nonce
        self.security_policy.asymmetric_cryptography.verify(data, response.ServerSignature.Signature)
        self._server_nonce = response.ServerNonce
        if not self.security_policy.server_certificate:
            self.security_policy.server_certificate = response.ServerCertificate
        elif self.security_policy.server_certificate != response.ServerCertificate:
            raise ua.UaError("Server certificate mismatch")
        # remember PolicyId's: we will use them in activate_session()
        ep = self.find_endpoint(response.ServerEndpoints, self.security_policy.Mode, self.security_policy.URI)
        self._policy_ids = ep.UserIdentityTokens
        self.session_timeout = response.RevisedSessionTimeout
        # START MODIFICATION
        ## remove keep alive
        # END MODIFICATION
        return response


if __name__=='__main__':
    # Tests with one connexion
    print('Connecting to', sUri)
    client = Client2(sUri)
    logger = TapLogger("session_timeout.tap")
    headerString = "******************* {0} *********************"

    # Test revised session timeout
    for l_session_timeout in [5000, 360000, 45000000]:
        try:
            logger.begin_section("Requested session timeout {0}".format(l_session_timeout))
            client.session_timeout = l_session_timeout
            # secure channel connection
            print(headerString.format("SC connection and session establishment with requested timeout {0}"
                                     .format(l_session_timeout)))
            secure_channels_connect(client, SecurityPolicy)
            print(headerString.format("Check revised session timeout value"))
            if (l_session_timeout < MIN_SESSION_TIMEOUT
                and client.session_timeout == MIN_SESSION_TIMEOUT):
                logger.add_test("- Revised timeout set to min value {}".format(client.session_timeout), True)
            elif (l_session_timeout > MAX_SESSION_TIMEOUT
                  and client.session_timeout == MAX_SESSION_TIMEOUT):
                logger.add_test("- Revised timeout set to max value {}".format(client.session_timeout), True)
            elif (l_session_timeout >= MIN_SESSION_TIMEOUT
                  and l_session_timeout <= MAX_SESSION_TIMEOUT
                  and client.session_timeout == l_session_timeout):
                logger.add_test("- Revised timeout set to requested value {}".format(client.session_timeout), True)
            else:
                logger.add_test("- Revised timeout set to UNEXPECTED value {}".format(client.session_timeout), False)
        finally:
            client.disconnect()
            print('Disconnected')

    # Test session timeout expiration
    client.session_timeout = SESSION_TIMEOUT
    for sp in [SecurityPolicy, security_policies.SecurityPolicyBasic256]:
        logger.begin_section("Requested session timeout {0} (policy {1})"
                             .format(SESSION_TIMEOUT, re.split("#",sp.URI)[-1]))
        try:
            # secure channel connection
            print(headerString.format("SC connection and session establishment"))
            secure_channels_connect(client, sp)

            # Read tests
            print(headerString.format("Read before session timeout"))
            # Read a node to be sure we are using the new security token
            nid_index = 1001
            nid = u"ns=1;i={}".format(nid_index)
            node = client.get_node(nid)
            value = node.get_value()

            print(headerString.format("Wait session timeout (10 seconds)"))
            sleep((SESSION_TIMEOUT / 1000) + 1) # add 1 seconde to timeout

            # read tests attempt on session after timeout
            print(headerString.format("Read: error excepted since session timeout"))
            try:
                value = node.get_value()
            except:
                logger.add_test('- Session timeout - read refused after timeout', True)

            else:
                logger.add_test('- Session timeout - read refused after timeout', False)

            # check endpoints (no session needed)
            print(headerString.format("GetEndpoints after session timeout on session Channel"))
            try:
                endPoints = client.get_endpoints()
            except:
                logger.add_test('- Session timeout - discovery service accepted after timeout', False)
            else:
                logger.add_test('- Session timeout - discovery service accepted after timeout', True)

        finally:
            try:
                client.close_session()
            except:
                None
            else:
                print(headerString.format("NOK: unexpected close session request accepted"))
            client.close_secure_channel()
            print('Disconnected')

    logger.finalize_report()

    sys.exit(1 if logger.has_failed_tests else 0)
