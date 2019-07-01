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
Simple client to launch validation tests
"""

from opcua import Client
from opcua import ua
from time import sleep

headerString = "******************* {0} *********************"
uri = 'opc.tcp://192.168.1.102:4841'

client = Client(uri)
client.application_uri = "urn:S2OPC:localhost"

print(headerString.format("Connect"))
client.connect()

nid_index = 1001
nid = u"ns=1;i={}".format(nid_index)

node = client.get_node(nid)
value = node.get_value()
print(' Value for Node {}:'.format(nid), value)

node.set_value(ua.Variant(12345, ua.VariantType.Int64))

value = node.get_value()
print(' Value for Node {}:'.format(nid), value)

print(headerString.format("Close "))

client.disconnect()









