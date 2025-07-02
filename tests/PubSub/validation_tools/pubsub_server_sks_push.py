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

from opcua import ua, Client

nid_objectPublisherSubscribe = u"i=14443"
nid_methodSetSecurityKeys = u"ns=1;i=17296"

class PubSubServerSKSPush:
    """Wraps a client that connects to the sample pubsub_server"""

    def __init__(self, uri):
        self.uri = uri
        self.client = Client(self.uri)
        self.client.application_uri = "urn:S2OPC:localhost"

    # Connect to the Pub/Sub server. Shall be called before other methods
    def connect(self):
        self.client.connect()
        print('Connected')
        self.nodeObjectPublisherSubscribe = self.client.get_node(nid_objectPublisherSubscribe)
        self.nodeMethodSetSecurityKeys = self.client.get_node(nid_methodSetSecurityKeys)

    # Disconnect to the Pub/Sub server
    def disconnect(self):
        self.client.disconnect()
        print('Disconnected')

    # Call method callSetSecurityKeys with parameters inputParameters
    def callSetSecurityKeys(self, inputParameters) :
        try:
            self.nodeObjectPublisherSubscribe.call_method(self.nodeMethodSetSecurityKeys) # add params
        except:
            print('Client probably not connected to PubSubServer')

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
        node.set_value(ua.DataValue(ua.Variant(value, varianttype)))

