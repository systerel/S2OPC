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
from opcua.common import utils
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

session_timeout = 10000

class Client2(Client):

    # Overriding create session method to request 10 seconds timeout and deactivate keepalive sending
    def create_session(self):
        """
        send a CreateSessionRequest to server with reasonable parameters.
        If you want o modify settings look at code of this methods
        and make your own
        """
        desc = ua.ApplicationDescription()
        desc.ApplicationUri = self.application_uri
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
        params.RequestedSessionTimeout = session_timeout
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


min_session_timeout = 10000
max_session_timeout = 43200000

if __name__=='__main__':

    # tests with one connexion
    print('Connecting to', sUri)
    client = Client2(sUri)
    logger = TapLogger("session_timeout.tap")
    headerString = "******************* {0} *********************"

    # Test revised session timeout
    for l_session_timeout in [5000, 3600000, 45000000]:
        try:
            logger.begin_section("Requested session timeout {0}".format(l_session_timeout))
            session_timeout = l_session_timeout
            # secure channel connection
            print(headerString.format("SC connection and session establishment with requested timeout {0}"
                                     .format(l_session_timeout)))
            secure_channels_connect(client, SecurityPolicy)
            print(headerString.format("Check revised session timeout value"))
            if (l_session_timeout < min_session_timeout
                and client.session_timeout == min_session_timeout):
                logger.add_test("- Revised timeout set to min value {}".format(client.session_timeout), True)
            elif (l_session_timeout > max_session_timeout
                  and client.session_timeout == max_session_timeout):
                logger.add_test("- Revised timeout set to max value {}".format(client.session_timeout), True)
            elif (l_session_timeout >= min_session_timeout
                  and l_session_timeout <= max_session_timeout
                  and client.session_timeout == l_session_timeout):
                logger.add_test("- Revised timeout set to requested value {}".format(client.session_timeout), True)
            else:
                logger.add_test("- Revised timeout set to UNEXPECTED value {}".format(client.session_timeout), False)
        finally:
            client.disconnect()
            print('Disconnected')
        
    # Test session timeout expiration
    session_timeout = 10000
    for sp in [SecurityPolicy, security_policies.SecurityPolicyBasic256]:
        logger.begin_section("Requested session timeout {0} (policy {1})"
                             .format(session_timeout, re.split("#",sp.URI)[-1]))
        try:
            # secure channel connection
            print(headerString.format("SC connection and session establishment"))
            secure_channels_connect(client, sp)

            # Read tests
            print(headerString.format("Read before session timeout"))
            # Read a node to be sure we are using the new security token
            nid = 1001
            node = client.get_node(nid)
            value = node.get_value()

            print(headerString.format("Wait session timeout (10 seconds)"))
            sleep((session_timeout / 1000) + 1) # add 1 seconde to timeout

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
