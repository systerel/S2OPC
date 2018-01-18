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
    logger.add_test('Get Endpoints Test - Check number of endpoints', len(endPoints) == 3)
    # print('number of endPoints:', len(endPoints))

    # check endpoints URL
    for (i, ep) in enumerate(endPoints):
        logger.add_test('Discovery Get Endpoints Test - endPoint {:01d} URL'.format(i),  ep.EndpointUrl == sUri)

    endPointNone = [ep for ep in endPoints if (ep.SecurityPolicyUri == securityPolicyNoneURI)][0]
    endPointBasic256 = [ep for ep in endPoints if (ep.SecurityPolicyUri == securityPolicyBasic256URI)][0]
    endPointBasic256Sha256 = [ep for ep in endPoints if (ep.SecurityPolicyUri ==securityPolicyBasic256Sha256URI)][0]

    # check endpoints security mode
    logger.add_test('Discovery Get Endpoints Test - None endPoint security mode', endPointNone.SecurityMode == ua.MessageSecurityMode.None_)
    logger.add_test('Discovery Get Endpoints Test - Basic256 endPoint security mode', endPointBasic256.SecurityMode == ua.MessageSecurityMode.Sign)
    logger.add_test('Discovery Get Endpoints Test - Basic256 endPoint security mode', endPointBasic256Sha256.SecurityMode == ua.MessageSecurityMode.SignAndEncrypt)

    # check security level (TODO: Spec TBC)
    logger.add_test('Discovery Get Endpoints Test - None endPoint security level', endPointNone.SecurityLevel == 0)
    logger.add_test('Discovery Get Endpoints Test - Basic256 endPoint security level', endPointBasic256.SecurityLevel == 1)
    logger.add_test('Discovery Get Endpoints Test - Basic256 endPoint security level', endPointBasic256Sha256.SecurityLevel == 1)

    # TODO: check transportProfileURI

