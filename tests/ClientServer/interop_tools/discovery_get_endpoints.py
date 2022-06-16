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

from common import sUri, securityPolicyNoneURI, securityPolicyBasic256URI, securityPolicyBasic256Sha256URI, securityPolicyAes128Sha256RsaOaep, securityPolicyAes256Sha256RsaPss
from opcua import ua

def discovery_get_endpoints_tests(client, logger):

    endPoints = client.get_endpoints()
    #print('endPoints:', endPoints)

    # 7 endpoints are expected: None (1), Basic256 (1 SignOnly and 1 SignAndEncrypt), Basic256Sha256 (1 SignOnly and 1 SignAndEncrypt)
    # Aes128Sha256RsaOaep (1 SignOnly and 1 SignAndEncrypt) and Aes256Sha256RsaPss (1 SignOnly and 1 SignAndEncrypt)
    logger.add_test('Get Endpoints Test - Check number of endpoints', len(endPoints) == 9)
    # print('number of endPoints:', len(endPoints))

    # check endpoints URL
    for (i, ep) in enumerate(endPoints):
        logger.add_test('Discovery Get Endpoints Test - endPoint {:01d} URL'.format(i),  ep.EndpointUrl == sUri)

    # check endpoints security mode and security level (TODO: Spec TBC)
    # None
    for ep in endPoints:
        if ep.SecurityPolicyUri == securityPolicyNoneURI:
            logger.add_test('Discovery Get Endpoints Test - None endPoint exists', True)
            logger.add_test('Discovery Get Endpoints Test - None endPoint security mode', ep.SecurityMode == ua.MessageSecurityMode.None_)
            logger.add_test('Discovery Get Endpoints Test - None endPoint security level', ep.SecurityLevel == 0)
            break
    else:
        logger.add_test('Discovery Get Endpoints Test - None endPoint exists', False)
        logger.add_test('Discovery Get Endpoints Test - None endPoint security mode', False)
        logger.add_test('Discovery Get Endpoints Test - None endPoint security level', False)

    # Basic256
    for ep in endPoints:
        if ep.SecurityPolicyUri == securityPolicyBasic256URI:
            logger.add_test('Discovery Get Endpoints Test - Basic256 endPoint exists', True)
            logger.add_test('Discovery Get Endpoints Test - Basic256 endPoint security mode', ep.SecurityMode in (ua.MessageSecurityMode.Sign, ua.MessageSecurityMode.SignAndEncrypt))
            logger.add_test('Discovery Get Endpoints Test - Basic256 endPoint security level', ep.SecurityLevel > 0)
            break
    else:
        logger.add_test('Discovery Get Endpoints Test - Basic256 endPoint exists', False)
        logger.add_test('Discovery Get Endpoints Test - Basic256 endPoint security mode', False)
        logger.add_test('Discovery Get Endpoints Test - Basic256 endPoint security level', False)

    # Basic256Sha256
    for ep in endPoints:
        if ep.SecurityPolicyUri == securityPolicyBasic256Sha256URI:
            logger.add_test('Discovery Get Endpoints Test - Basic256Sha256 endPoint exists', True)
            logger.add_test('Discovery Get Endpoints Test - Basic256Sha256 endPoint security mode', ep.SecurityMode in (ua.MessageSecurityMode.Sign, ua.MessageSecurityMode.SignAndEncrypt))
            logger.add_test('Discovery Get Endpoints Test - Basic256Sha256 endPoint security level', ep.SecurityLevel > 0)
            break
    else:
        logger.add_test('Discovery Get Endpoints Test - Basic256Sha256 endPoint exists', False)
        logger.add_test('Discovery Get Endpoints Test - Basic256Sha256 endPoint security mode', False)
        logger.add_test('Discovery Get Endpoints Test - Basic256Sha256 endPoint security level', False)

    # Aes128Sha256RsaOaep
    for ep in endPoints:
        if ep.SecurityPolicyUri == securityPolicyAes128Sha256RsaOaep:
            logger.add_test('Discovery Get Endpoints Test - Aes128Sha256RsaOaep endPoint exists', True)
            logger.add_test('Discovery Get Endpoints Test - Aes128Sha256RsaOaep endPoint security mode', ep.SecurityMode in (ua.MessageSecurityMode.Sign, ua.MessageSecurityMode.SignAndEncrypt))
            logger.add_test('Discovery Get Endpoints Test - Aes128Sha256RsaOaep endPoint security level', ep.SecurityLevel > 0)
            break
    else:
        logger.add_test('Discovery Get Endpoints Test - Aes128Sha256RsaOaep endPoint exists', False)
        logger.add_test('Discovery Get Endpoints Test - Aes128Sha256RsaOaep endPoint security mode', False)
        logger.add_test('Discovery Get Endpoints Test - Aes128Sha256RsaOaep endPoint security level', False)
    
    # Aes256Sha256RsaPss
    for ep in endPoints:
        if ep.SecurityPolicyUri == securityPolicyAes256Sha256RsaPss:
            logger.add_test('Discovery Get Endpoints Test - Aes256Sha256RsaPss endPoint exists', True)
            logger.add_test('Discovery Get Endpoints Test - Aes256Sha256RsaPss endPoint security mode', ep.SecurityMode in (ua.MessageSecurityMode.Sign, ua.MessageSecurityMode.SignAndEncrypt))
            logger.add_test('Discovery Get Endpoints Test - Aes256Sha256RsaPss endPoint security level', ep.SecurityLevel > 0)
            break
    else:
        logger.add_test('Discovery Get Endpoints Test - Aes256Sha256RsaPss endPoint exists', False)
        logger.add_test('Discovery Get Endpoints Test - Aes256Sha256RsaPss endPoint security mode', False)
        logger.add_test('Discovery Get Endpoints Test - Aes256Sha256RsaPss endPoint security level', False)

    # TODO: check transportProfileURI

