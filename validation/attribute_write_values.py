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
from common import variantInfoList

def attribute_write_values_tests(client, logger):

    for (i,e) in enumerate(variantInfoList):
        (testedType, variantType,  initialValue,  newValue) = e
        nid_index = 1000 + i + 1
        nid = u"ns=1;i={}".format(nid_index)
        print('Checking nid:', nid)
        node = client.get_node(nid)

        # write new value
        print(' Expected Value for Node {}:'.format(nid), newValue)
        node.set_value(ua.Variant(newValue, variantType))

        # check value
        value = node.get_value()
        print(' Value for Node {}:'.format(nid), value)
        if testedType == 'Float':
            # Handles float with epsilon diff, as random.uniform gave us a double,
            # which was allegedly truncated by freeopcua to a float...
            logger.add_test('Write Test - Value for Node {}'.format(nid), abs((value - newValue)/value) <= 2**(-24))
        else:
            logger.add_test('Write Test - Value for Node {}'.format(nid), value == newValue)

        # write back initial value
        node.set_value(ua.Variant(initialValue, variantType))

def attribute_write_values_two_clients_tests(client1, client2, logger):

    for (i,e) in enumerate(variantInfoList):
        (testedType, variantType,  initialValue,  newValue) = e
        nid_index = 1000 + i + 1
        nid = u"ns=1;i={}".format(nid_index)
        print('Checking nid:', nid)
        node1 = client1.get_node(nid)
        node2 = client2.get_node(nid)

        # write new value on client 1
        print(' Expected Value for Node {}:'.format(nid), newValue)
        node1.set_value(ua.Variant(newValue, variantType))

        # check value with client 2
        value = node2.get_value()
        print(' Value for Node {}:'.format(nid), value)
        if testedType == 'Float':
            # Handles float with epsilon diff, as random.uniform gave us a double,
            # which was allegedly truncated by freeopcua to a float...
            logger.add_test('Read/Write Test with several connexions - Value for Node {}'.format(nid), abs((value - newValue)/value) <= 2**(-24))
        else:
            logger.add_test('Read/Write Test with several connexions - Value for Node {}'.format(nid), value == newValue)

        # write back initial value with client 1
        node1.set_value(ua.Variant(initialValue, variantType))
