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

from opcua.ua import QualifiedName, LocalizedText, XmlElement
from common import variantInfoList

def attribute_read_tests(client, logger):

    for (i,e) in enumerate(variantInfoList):
        (testedType, variantType, expectedValue, _) = e
        nid = 1000 + i + 1
        print('Checking nid:', nid)
        expectedBrowseName = QualifiedName(testedType,0)
        expectedDisplayName = LocalizedText(u"{}_1dn".format(testedType))
        expectedDescription = u"{}_1d".format(testedType)
        #expectedValue = Initial_values_list[i]
        node = client.get_node(nid)

        # check value
        value = node.get_value()
        logger.add_test('Read Test - Value for Node {:03d}'.format(nid), value == expectedValue)
        print(' Value for Node {:03d}:'.format(nid), value)

        # check browseName
        browse_name = node.get_browse_name()
        logger.add_test('Read Test - browse name for Node {:03d}'.format(nid), browse_name == expectedBrowseName)
        print('browse_name: ', browse_name)

        # check display name
        display_name = node.get_display_name()
        logger.add_test('Read Test - display name for Node {:03d}'.format(nid), display_name == expectedDisplayName)

        # check node class
        class_name = node.get_node_class()
        logger.add_test('Read Test - node class for Node {:03d}'.format(nid), str("NodeClass.Variable")==str(class_name))

        # TODO: check data type
        #data_type = node.get_data_type()
        #print('data type: ', data_type)

