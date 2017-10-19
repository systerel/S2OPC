#!/usr/bin/python3.4
#-*-coding:Utf-8 -*

# Copyright (C) 2017 Systerel and others.
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
        (testedType, variantType,  _,  newValue) = e
        nid = 1000 + i + 1
        print('Checking nid:', nid)
        node = client.get_node(nid)

        # write new value
        #newValue = New_values_list[i]
        print(' Expected Value for Node {:03d}:'.format(nid), newValue)
        node.set_value(ua.Variant(newValue, variantType))

        # check value
        value = node.get_value()
        print(' Value for Node {:03d}:'.format(nid), value)
        logger.add_test('Write Test - Value for Node {:03d}'.format(nid), value == newValue)

