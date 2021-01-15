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

from time import sleep
from enum import Enum

from opcua import ua, Client

PUBSUBSERVER_TIMEOUT_STEP = 0.1
PUBSUBSERVER_TIMEOUT_MAX = 3


class PubSubState(Enum):
    # Values taken from SOPC_PubSubState
    DISABLED = 0
    PAUSED = 1
    OPERATIONAL = 2
    ERROR = 3
    # This value is not part of the C structure
    EXCEPTION = 4


class PubSubServer:
    """Wraps a client that connects to the sample pubsub_server"""

    def __init__(self, uri, nid_configuration, nid_start_stop, nid_status):
        self.uri = uri
        self.client = Client(self.uri)
        self.client.application_uri = "urn:S2OPC:localhost"
        self.nid_configuration = nid_configuration
        self.nid_start_stop = nid_start_stop
        self.nid_status = nid_status

    # Connect to the Pub/Sub server. Shall be called before other methods
    def connect(self):
        self.client.connect()
        print('Connected')
        self.nodeConfiguration = self.client.get_node(self.nid_configuration)
        self.nodeStartStop = self.client.get_node(self.nid_start_stop)
        self.nodeStatus = self.client.get_node(self.nid_status)

    # Is connected to Pub/Sub server
    def isConnected(self):
        try:
            self.nodeStatus.get_value()
            return True
        except:  # TODO: exception too broad
            return False

    # Disconnect to the Pub/Sub server
    def disconnect(self):
        self.client.disconnect()
        print('Disconnected')

    def __setStartStop(self, value):
        try:
            # Set value and wait until value changes or timeout expires
            self.nodeStartStop.set_value(ua.Variant(value, ua.VariantType.Byte))
            state = PubSubState.OPERATIONAL if value else PubSubState.DISABLED
            timeout = PUBSUBSERVER_TIMEOUT_MAX
            while self.getPubSubState() != state and timeout > 0:
                sleep(PUBSUBSERVER_TIMEOUT_STEP)
                timeout = timeout - PUBSUBSERVER_TIMEOUT_STEP
            # TODO: what if we reached the timeout?
        except e:  # TODO: exception too broad
            print('Client probably not connected to PubSubServer', e)

    # Is Pub/Sub module started
    def isStart(self):  # TODO: not connected does not mean that the module is not started
        try:
            return bool(self.nodeStartStop.get_value())
        except:  # TODO: exception too broad
            print('Client probably not connected to PubSubServer')
            return False

    # Start the Pub/Sub module
    # Wait until status changes or timeout expires
    def start(self):
        self.__setStartStop(1)

    # Stop the Pub/Sub module
    # Wait until status changes or timeout expires
    def stop(self):
        self.__setStartStop(0)

    # Get the Pub/Sub module state (a PubSubState instance)
    def getPubSubState(self):
        try:
            return PubSubState(self.nodeStatus.get_value())
        except:  # TODO: exception too broad
            print('Client probably not connected to PubSubServer')
            return PubSubState.EXCEPTION

    # Set the Pub/Sub module configuration
    # Value is a XML in a string
    def setConfiguration(self, value):
        try:
            self.nodeConfiguration.set_value(ua.Variant(value, ua.VariantType.String))
        except:  # TODO: exception too broad
            print('Client probably not connected to PubSubServer')

    # Get the Pub/Sub module configuration as XML string
    def getConfiguration(self):
        try:
            return self.nodeConfiguration.get_value()
        except:  # TODO: exception too broad
            print('Client probably not connected to PubSubServer')
            return None

    # Get value. Nid is a string
    def getValue(self, nid):
        node = self.client.get_node(nid)
        return node.get_value()

    # Set value.
    # - nid is a string.
    # - varianttype is a builtintype from ua package
    # - value type depends of varianttype
    def setValue(self, nid, varianttype, value):
        node = self.client.get_node(nid)
        node.set_value(value=value, varianttype=varianttype)

