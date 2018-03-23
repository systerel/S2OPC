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

from common import sUri, securityPolicyNoneURI, securityPolicyBasic256URI, securityPolicyBasic256Sha256URI
from opcua import ua

def discovery_get_endpoints_tests(client, logger):

    endPoints = client.get_endpoints()
    #print('endPoints:', endPoints)

    # three endpoints are expected: None, Basic256 and Basic256Sha256
    logger.add_test('Get Endpoints Test - Check number of endpoints', len(endPoints) == 4)
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

    # TODO: check transportProfileURI

