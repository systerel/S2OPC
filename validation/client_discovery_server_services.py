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

from opcua import ua
import sys
import copy

def discovery_server_tests(client, logger):

    # RegisterServer2 test
    
    # Nominal behavior: IsOnline = True
    params = ua.RegisterServer2Parameters()
    params.Server.ServerUri = "urn:S2OPC:RegisteredServer"
    params.Server.ProductUri = "urn:S2OPC:RegisteredServer"
    serverName = ua.uatypes.LocalizedText()
    serverName.Locale = "en"
    serverName.Text = "MyServer registered"
    params.Server.ServerNames.append(serverName)
    params.Server.DiscoveryUrls.append("opc.tcp://localhost:4841")

    mdnsConfig = ua.MdnsDiscoveryConfiguration()
    mdnsConfig.MdnsServerName = "MyServer registered with mDNS config"
    # all authorized identifier capabilities
    mdnsConfig.ServerCapabilities = ["NA", "DA",  "HD",  "AC",   "HE",  "GDS", "LDS", "DI", "ADI", "FDI", "FDIC", "PLC", "S95"]
    params.DiscoveryConfiguration.append(mdnsConfig)  

    try:
        configResults = client.uaclient.register_server2(params)
    except:
        logger.add_test('RegisterServer2 test - nominal register IsOnline=True. Expecting Good == {}'
                        .format(sys.exc_info()[0]),
                        False)
    else:
        logger.add_test('RegisterServer2 test - nominal register IsOnline=True. Expecting Good == 0x{:02X}'
                    .format(configResults[0].value),
                    ua.uatypes.StatusCode("Good") == configResults[0])

    # Nominal behavior: IsOnline = False
    copyParams = copy.deepcopy(params)
    copyParams.Server.IsOnline = False
    try:
        configResults = client.uaclient.register_server2(copyParams)
    except:
        logger.add_test('RegisterServer2 test - nominal register IsOnline=False. Expecting Good == {}'
                        .format(sys.exc_info()[0]),
                        False)
    else:
        logger.add_test('RegisterServer2 test - nominal register IsOnline=False. Expecting Good == 0x{:02X}'
                    .format(configResults[0].value),
                    ua.uatypes.StatusCode("Good") == configResults[0])

    # Empty server Uri
    degParams = copy.deepcopy(params)
    degParams.Server.ServerUri = None
    try:
        configResults = client.uaclient.register_server2(degParams)
    except ua.uaerrors._auto.BadServerUriInvalid:
        logger.add_test('RegisterServer2 test - empty ServerUri register failed with BadServerUri', True)
    except:
        logger.add_test('RegisterServer2 test - empty ServerUri register failed. Expected BadServerUri != {}'
                        .format(sys.exc_info()[0]),
                        False)
    else:
        logger.add_test('RegisterServer2 test - empty ServerUri register failed. Expected BadServerUri != Good',
                        False)

    # Empty product Uri
    degParams = copy.deepcopy(params)
    degParams.Server.ProductUri = None
    try:
        configResults = client.uaclient.register_server2(degParams)
    except ua.uaerrors._auto.BadInvalidArgument:
        logger.add_test('RegisterServer2 test - empty ProductUri register failed with BadInvalidArgument', True)
    except:
        logger.add_test('RegisterServer2 test - empty ProductUri register failed. Expected BadInvalidArgument != {}'
                        .format(sys.exc_info()[0]),
                        False)
    else:
        logger.add_test('RegisterServer2 test - empty ProductUri register failed. Expected BadInvalidArgument != Good',
                        False)
    
    # Empty server names
    degParams = copy.deepcopy(params)
    degParams.Server.ServerNames = []
    try:
        configResults = client.uaclient.register_server2(degParams)
    except ua.uaerrors._auto.BadServerNameMissing:
        logger.add_test('RegisterServer2 test - empty ServerNames register failed with BadServerNameMissing', True)
    except:
        logger.add_test('RegisterServer2 test - empty ServerNames register failed. Expected BadServerNameMissing != {}'
                        .format(sys.exc_info()[0]),
                        False)
    else:
        logger.add_test('RegisterServer2 test - empty ServerNames register failed. Expected BadServerNameMissing != Good',
                        False)

    # Invalid ApplicationType (client)
    degParams = copy.deepcopy(params)
    degParams.Server.ServerType = ua.uaprotocol_auto.ApplicationType.Client
    try:
        configResults = client.uaclient.register_server2(degParams)
    except ua.uaerrors._auto.BadInvalidArgument:
        logger.add_test('RegisterServer2 test - invalid ApplicationType (client) register failed with BadInvalidArgument', True)
    except:
        logger.add_test('RegisterServer2 test - invalid ApplicationType (client) register failed. Expected BadInvalidArgument != {}'
                        .format(sys.exc_info()[0]),
                        False)
    else:
        logger.add_test('RegisterServer2 test - invalid ApplicationType (client) register failed. Expected BadInvalidArgument != Good',
                        False)

    # Empty discovery URLs
    degParams = copy.deepcopy(params)
    degParams.Server.DiscoveryUrls = []
    try:
        configResults = client.uaclient.register_server2(degParams)
    except ua.uaerrors._auto.BadDiscoveryUrlMissing:
        logger.add_test('RegisterServer2 test - empty ServerNames register failed with BadDiscoveryUrlMissing', True)
    except:
        logger.add_test('RegisterServer2 test - empty ServerNames register failed. Expected BadDiscoveryUrlMissing != {}'
                        .format(sys.exc_info()[0]),
                        False)
    else:
        logger.add_test('RegisterServer2 test - empty ServerNames register failed. Expected BadDiscoveryUrlMissing != Good',
                        False)

    # Define unsupported semaphoreFilePAth
    degParams = copy.deepcopy(params)
    degParams.Server.SemaphoreFilePath = "/tmp/MyServerSemaphoreFile"
    try:
        configResults = client.uaclient.register_server2(degParams)
    except ua.uaerrors._auto.BadSempahoreFileMissing:
        logger.add_test('RegisterServer2 test - unsupported SemaphoreFilePath register failed with BadSemaphoreFileMissing', True)
    except:
        logger.add_test('RegisterServer2 test - unsupported SemaphoreFilePath register failed. Expected BadSemaphoreFileMissing != {}'
                        .format(sys.exc_info()[0]),
                        False)
    else:
        logger.add_test('RegisterServer2 test - unsupported SemaphoreFilePath register failed. Expected BadSemaphoreFileMissing != Good',
                        False)

    # Define no discovery configuration 
    degParams = copy.deepcopy(params)
    degParams.DiscoveryConfiguration = []
    try:
        configResults = client.uaclient.register_server2(degParams)
    except ua.uaerrors._auto.BadInvalidArgument:
        logger.add_test('RegisterServer2 test - no discovery configuration register failed with BadInvalidArgument', True)
    except:
        logger.add_test('RegisterServer2 test - no discovery configuration register failed. Expected BadInvalidArgument != {}'
                        .format(sys.exc_info()[0]),
                        False)
    else:
        logger.add_test('RegisterServer2 test - no discovery configuration register failed. Expected BadInvalidArgument != Good',
                        False)
    

    # Define several discovery configuration (no mDNS)        
    degParams = copy.deepcopy(params)
    degParams.DiscoveryConfiguration = []
    degParams.DiscoveryConfiguration.append(ua.UserIdentityToken())
    degParams.DiscoveryConfiguration.append(ua.UserIdentityToken())
    try:
        configResults = client.uaclient.register_server2(degParams)
    except ua.uaerrors._auto.BadInvalidArgument:
        logger.add_test('RegisterServer2 test - no mDNS discovery configuration register failed with BadInvalidArgument', True)
    except:
        logger.add_test('RegisterServer2 test - no mDNS discovery configuration register failed. Expected BadInvalidArgument != {}'
                        .format(sys.exc_info()[0]),
                        False)
    else:
        logger.add_test('RegisterServer2 test - no mDNS discovery configuration register failed. Expected BadInvalidArgument != Good',
                        False)

    # Define several discovery configuration (several mDNS)        
    degParams = copy.deepcopy(params)
    degParams.DiscoveryConfiguration.append(ua.UserIdentityToken())
    degParams.DiscoveryConfiguration.append(ua.UserIdentityToken())
    degParams.DiscoveryConfiguration.append(degParams.DiscoveryConfiguration[0])
    try:
        configResults = client.uaclient.register_server2(degParams)
    except ua.uaerrors._auto.BadInvalidArgument:
        logger.add_test('RegisterServer2 test - several mDNS discovery configuration register failed with BadInvalidArgument', True)
    except:
        logger.add_test('RegisterServer2 test - several mDNS discovery configuration register failed. Expected BadInvalidArgument != {}'
                        .format(sys.exc_info()[0]),
                        False)
    else:
        logger.add_test('RegisterServer2 test - several mDNS discovery configuration register failed. Expected BadInvalidArgument != Good',
                        False)

    # Define several discovery configuration (only one mDNS)
    degParams = copy.deepcopy(params)
    degParams.DiscoveryConfiguration.append(ua.UserIdentityToken())
    degParams.DiscoveryConfiguration.append(ua.UserIdentityToken())
    try:
        configResults = client.uaclient.register_server2(degParams)
        
    except:
        logger.add_test('RegisterServer2 test - several discovery with only 1 mDNS configuration register. Expected Good != {}'
                        .format(sys.exc_info()[0]),
                        False)
    else:
        logger.add_test('RegisterServer2 test - several discovery with only 1 mDNS configuration register. Expected configurationResult[0] == Good: found 0x{:02X}'.format(configResults[0].value),
                        ua.uatypes.StatusCode("Good") == configResults[0])
        logger.add_test('RegisterServer2 test - several discovery with only 1 mDNS configuration register. Expected configurationResult[1] == BadNotSupported: found 0x{:02X}'.format(configResults[1].value),
                        ua.uatypes.StatusCode("BadNotSupported") == configResults[1])
        logger.add_test('RegisterServer2 test - several discovery with only 1 mDNS configuration register. Expected configurationResult[2] == BadNotSupported: found 0x{:02X}'.format(configResults[2].value),
                        ua.uatypes.StatusCode("BadNotSupported") == configResults[2])
    
