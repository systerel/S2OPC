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

allServerCapabilities = ["DA",  "HD",  "AC",   "HE",  "GDS", "LDS", "DI", "ADI", "FDI", "FDIC", "PLC", "S95"]

def register_server2_test(client, logger):
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
    mdnsConfig.ServerCapabilities = allServerCapabilities
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

    # Define 1 mDNS configuration with an invalid server capability
    degParams = copy.deepcopy(params)
    degParams.DiscoveryConfiguration[0].ServerCapabilities.pop() # remove a valid server capability
    degParams.DiscoveryConfiguration[0].ServerCapabilities.append("BEE") # add an invalid server capability
    try:
        configResults = client.uaclient.register_server2(degParams)
    except ua.uaerrors._auto.BadInvalidArgument:
        logger.add_test('RegisterServer2 test - mDNS discovery config with invalid capability register failed with BadInvalidArgument', True)
    except:
        logger.add_test('RegisterServer2 test - mDNS discovery config with invalid capability register failed. Expected BadInvalidArgument != {}'
                        .format(sys.exc_info()[0]),
                        False)
    else:
        logger.add_test('RegisterServer2 test - mDNS discovery config with invalid capability register failed. Expected BadInvalidArgument != Good',
                        False)

    # Restore empty record: IsOnline = False
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

def local_register_server2(client, logger, name, address, capabilities, isOnline):
    # Nominal behavior: IsOnline = True
    params = ua.RegisterServer2Parameters()
    params.Server.IsOnline = isOnline
    params.Server.ServerUri = "urn:S2OPC:{}".format(name)
    params.Server.ProductUri = "urn:S2OPC:{}".format(name)
    serverName = ua.uatypes.LocalizedText()
    serverName.Locale = "en"
    serverName.Text = name
    params.Server.ServerNames.append(serverName)
    params.Server.DiscoveryUrls.append(address)

    mdnsConfig = ua.MdnsDiscoveryConfiguration()
    mdnsConfig.MdnsServerName = name
    # all authorized identifier capabilities
    mdnsConfig.ServerCapabilities = capabilities
    params.DiscoveryConfiguration.append(mdnsConfig)

    try:
        configResults = client.uaclient.register_server2(params)
    except:
        logger.add_test('FindServersOnNetwork test - nominal register ServerName={} IsOnline={}. Expecting Good == {}'
                        .format(name, isOnline, sys.exc_info()[0]),
                        False)
    else:
        logger.add_test('FindServersOnNetwork test - nominal register ServerName={} IsOnline={}. Expecting Good == 0x{:02X}'
                        .format(name, isOnline, configResults[0].value),
                        ua.uatypes.StatusCode("Good") == configResults[0])


def find_servers_on_network_test(client, logger):
    defaultAddress = "opc.tcp://localhost:4841"
    # No filters in FindServersOnNetworkRequest by default
    params = ua.FindServersOnNetworkParameters()

    # Register 1 server and retrieve it
    local_register_server2(client, logger, "TestServer1", defaultAddress, allServerCapabilities, True)
    results = client.uaclient.find_servers_on_network(params)
    logger.add_test('FindServersOnNetwork test - expect 1 registered server "{}"'
                    .format("TestServer1"),
                    (1 == len(results.Servers) and
                     "TestServer1" == results.Servers[0].ServerName and
                     defaultAddress == results.Servers[0].DiscoveryUrl and
                     allServerCapabilities == results.Servers[0].ServerCapabilities)
                    )

    # UnRegister 1 server and check not present anymore
    local_register_server2(client, logger, "TestServer1", defaultAddress, allServerCapabilities, False)
    results = client.uaclient.find_servers_on_network(params)
    logger.add_test('FindServersOnNetwork test - expect 0 registered server',
                    0 == len(results.Servers))

    # Register 2 times same server name: expect only last record to be returned
    local_register_server2(client, logger, "TestServer1", defaultAddress, allServerCapabilities, True)
    local_register_server2(client, logger, "TestServer1", "opc.tcp://test:1000", allServerCapabilities, True)
    results = client.uaclient.find_servers_on_network(params)
    logger.add_test('FindServersOnNetwork test - expect only 1 registered server "{}" with last record content'
                    .format("TestServer1"),
                    (1 == len(results.Servers) and
                     "TestServer1" == results.Servers[0].ServerName and
                     "opc.tcp://test:1000" == results.Servers[0].DiscoveryUrl and
                     allServerCapabilities == results.Servers[0].ServerCapabilities)
                    )

    # Register a second server with unknown capabilities
    local_register_server2(client, logger, "TestServer2", defaultAddress, ["NA"], True)
    results = client.uaclient.find_servers_on_network(params)
    logger.add_test('FindServersOnNetwork test - expect 2 registered server 1 with all and 1 without capabilities',
                    (2 == len(results.Servers) and
                     "TestServer1" == results.Servers[0].ServerName and
                     allServerCapabilities == results.Servers[0].ServerCapabilities and
                     "TestServer2" == results.Servers[1].ServerName and
                     ["NA"] == results.Servers[1].ServerCapabilities)
                    )

    # Register a third server with only 'LDS' capability
    local_register_server2(client, logger, "TestServer3", defaultAddress, ["LDS"], True)
    results = client.uaclient.find_servers_on_network(params)
    logger.add_test('FindServersOnNetwork test - expect 3 registered server 1 with all, 1 without capabilities and 1 with "LDS" only',
                    (3 == len(results.Servers) and
                     "TestServer1" == results.Servers[0].ServerName and
                     allServerCapabilities == results.Servers[0].ServerCapabilities and
                     "TestServer2" == results.Servers[1].ServerName and
                     ["NA"] == results.Servers[1].ServerCapabilities and
                     "TestServer3" == results.Servers[2].ServerName and
                     ["LDS"] == results.Servers[2].ServerCapabilities)
                    )

    # Filter using the capabilities: request all
    params.ServerCapabilityFilter = allServerCapabilities
    results = client.uaclient.find_servers_on_network(params)
    logger.add_test('FindServersOnNetwork test - filter with all capabilities: expect only 1 registered server',
                    (1 == len(results.Servers) and
                     "TestServer1" == results.Servers[0].ServerName and
                     allServerCapabilities == results.Servers[0].ServerCapabilities)
                    )

    # Filter using the capabilities: request 'LDS' capability only
    params.ServerCapabilityFilter = ["LDS"]
    results = client.uaclient.find_servers_on_network(params)
    logger.add_test('FindServersOnNetwork test - filter with "LDS" capability: expect 2 registered server',
                    (2 == len(results.Servers) and
                     "TestServer1" == results.Servers[0].ServerName and
                     allServerCapabilities == results.Servers[0].ServerCapabilities and
                     "TestServer3" == results.Servers[1].ServerName and
                     ["LDS"] == results.Servers[1].ServerCapabilities)
                    )

    # Retrieve the recordIds of the servers
    params.ServerCapabilityFilter = []
    results = client.uaclient.find_servers_on_network(params)
    logger.add_test('FindServersOnNetwork test - retrieve recordId of 3 servers in numerical order',
                    (3 == len(results.Servers) and
                     results.Servers[0].RecordId < results.Servers[1].RecordId and
                     results.Servers[1].RecordId < results.Servers[2].RecordId)
                    )
    recordIdServer1 = results.Servers[0].RecordId
    recordIdServer2 = results.Servers[1].RecordId
    recordIdServer3 = results.Servers[2].RecordId

    # Request from recordId of 1st server
    params.StartingRecordId = recordIdServer1
    results = client.uaclient.find_servers_on_network(params)
    logger.add_test('FindServersOnNetwork test - start with recordId of 1st server: expect 3 servers',
                    (3 == len(results.Servers) and
                     "TestServer1" == results.Servers[0].ServerName and
                     "TestServer2" == results.Servers[1].ServerName and
                     "TestServer3" == results.Servers[2].ServerName)
                    )

    # Request from recordId of 2nd server
    params.StartingRecordId = recordIdServer2
    results = client.uaclient.find_servers_on_network(params)
    logger.add_test('FindServersOnNetwork test - start with recordId of 2nd server: expect 2 servers',
                    (2 == len(results.Servers) and
                     "TestServer2" == results.Servers[0].ServerName and
                     "TestServer3" == results.Servers[1].ServerName)
                    )

    # Request from recordId of 3rd server
    params.StartingRecordId = recordIdServer3
    results = client.uaclient.find_servers_on_network(params)
    logger.add_test('FindServersOnNetwork test - start with recordId of 3rd server: expect 1 server',
                    (1 == len(results.Servers) and
                     "TestServer3" == results.Servers[0].ServerName)
                    )

    # Request from recordId of 3rd server + 1
    params.StartingRecordId = recordIdServer3 + 1
    results = client.uaclient.find_servers_on_network(params)
    logger.add_test('FindServersOnNetwork test - start with recordId of 3rd server + 1: expect 0 servers',
                    0 == len(results.Servers)
                    )

    # Limit the number of results to 1 starting with 3rd recordId
    params.StartingRecordId = recordIdServer3
    params.MaxRecordsToReturn = 1
    results = client.uaclient.find_servers_on_network(params)
    logger.add_test('FindServersOnNetwork test - limit to 1 record starting with server3 recordId: expect server 3 only',
                    (1 == len(results.Servers) and
                     "TestServer3" == results.Servers[0].ServerName)
                    )

    # Limit the number of results to 1 starting with 2nd recordId
    params.StartingRecordId = recordIdServer2
    params.MaxRecordsToReturn = 1
    results = client.uaclient.find_servers_on_network(params)
    logger.add_test('FindServersOnNetwork test - limit to 1 record starting with server2 recordId: expect server 2 only',
                    (1 == len(results.Servers) and
                     "TestServer2" == results.Servers[0].ServerName)
                    )

    # Limit the number of results to 2 starting with 2nd recordId
    params.StartingRecordId = recordIdServer2
    params.MaxRecordsToReturn = 2
    results = client.uaclient.find_servers_on_network(params)
    logger.add_test('FindServersOnNetwork test - limit to 2 record starting with server2 recordId: expect server 2 and server 3 only',
                    (2 == len(results.Servers) and
                     "TestServer2" == results.Servers[0].ServerName and
                     "TestServer3" == results.Servers[1].ServerName)
                    )

    # Limit the number of results to 1 with recordId 0
    params.StartingRecordId = 0
    params.MaxRecordsToReturn = 1
    results = client.uaclient.find_servers_on_network(params)
    logger.add_test('FindServersOnNetwork test - limit to 1 record: expect server 1 only',
                    (1 == len(results.Servers) and
                     "TestServer1" == results.Servers[0].ServerName)
                    )

    # Limit the number of results to 2
    params.MaxRecordsToReturn = 2
    results = client.uaclient.find_servers_on_network(params)
    logger.add_test('FindServersOnNetwork test - limit to 2 records: expect server 1 and server 2 only',
                    (2 == len(results.Servers) and
                     "TestServer1" == results.Servers[0].ServerName and
                     "TestServer2" == results.Servers[1].ServerName)
                    )

    # Limit the number of results to 3
    params.MaxRecordsToReturn = 3
    results = client.uaclient.find_servers_on_network(params)
    logger.add_test('FindServersOnNetwork test - limit to 3 records: expect 3 servers',
                    (3 == len(results.Servers) and
                     "TestServer1" == results.Servers[0].ServerName and
                     "TestServer2" == results.Servers[1].ServerName and
                     "TestServer3" == results.Servers[2].ServerName)
                    )

    # UnRegister all servers
    local_register_server2(client, logger, "TestServer1", defaultAddress, [], False)
    local_register_server2(client, logger, "TestServer2", defaultAddress, [], False)
    local_register_server2(client, logger, "TestServer3", defaultAddress, [], False)

def check_self_in_find_servers_response(defaultAddress, server):
    return ("urn:S2OPC:localhost" == server.ApplicationUri and
            1 == len(server.DiscoveryUrls) and
            defaultAddress == server.DiscoveryUrls[0])

def find_servers_test(client, logger):
    # Note: LocaleIds are not tested here because it is already done in UACTT

    defaultAddress = "opc.tcp://localhost:4841"
    # No filters in FindServers by default
    params = ua.FindServersParameters()

    # Register 0 servers: retrieve server-Self
    Servers = client.uaclient.find_servers(params)
    logger.add_test('FindServers test - expect 0 registered server but self returned "{}"'
                    .format("urn:S2OPC:localhost"),
                    (1 == len(Servers) and
                     check_self_in_find_servers_response(defaultAddress, Servers[0]))
                    )

    # Register 1 server and retrieve it
    local_register_server2(client, logger, "TestServer1", defaultAddress, allServerCapabilities, True)
    Servers = client.uaclient.find_servers(params)
    logger.add_test('FindServers test - expect 1 registered server "{}" + self'
                    .format("urn:S2OPC:TestServer1"),
                    (2 == len(Servers) and
                     "urn:S2OPC:TestServer1" == Servers[0].ApplicationUri and
                     defaultAddress == Servers[0].DiscoveryUrls[0] and
                     check_self_in_find_servers_response(defaultAddress, Servers[1]))
                    )

    # UnRegister 1 server and check not present anymore
    local_register_server2(client, logger, "TestServer1", defaultAddress, allServerCapabilities, False)
    Servers = client.uaclient.find_servers(params)
    logger.add_test('FindServers test - expect 0 registered server + self',
                    1 == len(Servers) and
                    check_self_in_find_servers_response(defaultAddress, Servers[0]))

    # Register 2 times same server name: expect only last record to be returned
    local_register_server2(client, logger, "TestServer1", defaultAddress, allServerCapabilities, True)
    local_register_server2(client, logger, "TestServer1", "opc.tcp://test:1000", allServerCapabilities, True)
    Servers = client.uaclient.find_servers(params)
    logger.add_test('FindServers test - expect only 1 registered server "{}" with last record content + self'
                    .format("urn:S2OPC:TestServer1"),
                    (2 == len(Servers) and
                     "urn:S2OPC:TestServer1" == Servers[0].ApplicationUri and
                     "opc.tcp://test:1000" == Servers[0].DiscoveryUrls[0] and
                     check_self_in_find_servers_response(defaultAddress, Servers[1]))
                    )

    # Register a second server with unknown capabilities
    local_register_server2(client, logger, "TestServer2", defaultAddress, ["NA"], True)
    Servers = client.uaclient.find_servers(params)
    logger.add_test('FindServers test - expect 2 registered server 1 with all and 1 without capabilities',
                    (3 == len(Servers) and
                     "urn:S2OPC:TestServer1" == Servers[0].ApplicationUri and
                     "urn:S2OPC:TestServer2" == Servers[1].ApplicationUri and
                     check_self_in_find_servers_response(defaultAddress, Servers[2]))
                    )

    # Register a third server with only 'LDS' capability
    local_register_server2(client, logger, "TestServer3", defaultAddress, ["LDS"], True)
    Servers = client.uaclient.find_servers(params)
    logger.add_test('FindServers test - expect 3 registered server 1 with all, 1 without capabilities and 1 with "LDS" only',
                    (4 == len(Servers) and
                     "urn:S2OPC:TestServer1" == Servers[0].ApplicationUri and
                     "urn:S2OPC:TestServer2" == Servers[1].ApplicationUri and
                     "urn:S2OPC:TestServer3" == Servers[2].ApplicationUri and
                     check_self_in_find_servers_response(defaultAddress, Servers[3]))
                    )

    # Filter using the serverUri: request server 1
    params.ServerUris = ["urn:S2OPC:TestServer1"]
    Servers = client.uaclient.find_servers(params)
    logger.add_test('FindServers test - filter with serverUri of server 1: expect only 1 registered server',
                    (1 == len(Servers) and
                     "urn:S2OPC:TestServer1" == Servers[0].ApplicationUri)
                    )

    # Filter using the serverUri: request server 3
    params.ServerUris = ["urn:S2OPC:TestServer3"]
    Servers = client.uaclient.find_servers(params)
    logger.add_test('FindServers test - filter with serverUri of server 3: expect only 3 registered server',
                    (1 == len(Servers) and
                     "urn:S2OPC:TestServer3" == Servers[0].ApplicationUri)
                    )

    # Filter using the serverUri: request server 1 & 2
    params.ServerUris = ["urn:S2OPC:TestServer1", "urn:S2OPC:TestServer2"]
    Servers = client.uaclient.find_servers(params)
    logger.add_test('FindServers test - filter with serverUri of server 1 & 2: expect 2 registered server',
                    (2 == len(Servers) and
                     "urn:S2OPC:TestServer1" == Servers[0].ApplicationUri and
                     "urn:S2OPC:TestServer2" == Servers[1].ApplicationUri)
                    )

    # Filter using the serverUri: request server 1 & 2 & 3
    params.ServerUris = ["urn:S2OPC:TestServer1", "urn:S2OPC:TestServer2", "urn:S2OPC:TestServer3"]
    Servers = client.uaclient.find_servers(params)
    logger.add_test('FindServers test - filter with serverUri of server 1 & 2 & 3: expect 3 registered server',
                    (3 == len(Servers) and
                     "urn:S2OPC:TestServer1" == Servers[0].ApplicationUri and
                     "urn:S2OPC:TestServer2" == Servers[1].ApplicationUri and
                     "urn:S2OPC:TestServer3" == Servers[2].ApplicationUri)
                    )

    # UnRegister all servers
    local_register_server2(client, logger, "TestServer1", defaultAddress, [], False)
    local_register_server2(client, logger, "TestServer2", defaultAddress, [], False)
    local_register_server2(client, logger, "TestServer3", defaultAddress, [], False)


def discovery_server_tests(client, logger):
    register_server2_test(client, logger)
    find_servers_on_network_test(client, logger)
    find_servers_test(client, logger)
