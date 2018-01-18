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

from opcua import ua
from opcua.ua import QualifiedName, LocalizedText, XmlElement
from common import variantInfoList

def attribute_write_values_tests(client, logger):

    for (i,e) in enumerate(variantInfoList):
        (testedType, variantType,  initialValue,  newValue) = e
        nid = 1000 + i + 1
        print('Checking nid:', nid)
        node = client.get_node(nid)

        # write new value
        print(' Expected Value for Node {:03d}:'.format(nid), newValue)
        node.set_value(ua.Variant(newValue, variantType))

        # check value
        value = node.get_value()
        print(' Value for Node {:03d}:'.format(nid), value)
        if testedType == 'Float':
            # Handles float with epsilon diff, as random.uniform gave us a double,
            # which was allegedly truncated by freeopcua to a float...
            logger.add_test('Write Test - Value for Node {:03d}'.format(nid), abs((value - newValue)/value) <= 2**(-24))
        else:
            logger.add_test('Write Test - Value for Node {:03d}'.format(nid), value == newValue)

        # write back initial value
        node.set_value(ua.Variant(initialValue, variantType))

def attribute_write_values_two_clients_tests(client1, client2, logger):

    for (i,e) in enumerate(variantInfoList):
        (testedType, variantType,  initialValue,  newValue) = e
        nid = 1000 + i + 1
        print('Checking nid:', nid)
        node1 = client1.get_node(nid)
        node2 = client2.get_node(nid)

        # write new value on client 1
        print(' Expected Value for Node {:03d}:'.format(nid), newValue)
        node1.set_value(ua.Variant(newValue, variantType))

        # check value with client 2
        value = node2.get_value()
        print(' Value for Node {:03d}:'.format(nid), value)
        if testedType == 'Float':
            # Handles float with epsilon diff, as random.uniform gave us a double,
            # which was allegedly truncated by freeopcua to a float...
            logger.add_test('Read/Write Test with several connexions - Value for Node {:03d}'.format(nid), abs((value - newValue)/value) <= 2**(-24))
        else:
            logger.add_test('Read/Write Test with several connexions - Value for Node {:03d}'.format(nid), value == newValue)

        # write back initial value with client 1
        node1.set_value(ua.Variant(initialValue, variantType))
