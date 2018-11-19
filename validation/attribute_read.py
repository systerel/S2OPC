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

from opcua.ua import QualifiedName, LocalizedText
from common import variantInfoList

def attribute_read_tests(client, logger):

    for (i,e) in enumerate(variantInfoList):
        (testedType, variantType, expectedValue, _) = e
        nid_index = 1000 + i + 1
        nid = u"ns=1;i={}".format(nid_index)
        print('Checking nid:', nid)
        expectedBrowseName = QualifiedName(testedType,1)
        expectedDisplayName = LocalizedText(u"{}_1dn".format(testedType))
        node = client.get_node(nid)

        # check value
        value = node.get_value()
        if testedType == 'Float':
            # Handles float with epsilon diff, as random.uniform gave us a double,
            # which was allegedly truncated by freeopcua to a float...
            logger.add_test('Read Test - Value for Node {}'.format(nid), abs((value - expectedValue)/value) <= 2**(-24))
        else:
            logger.add_test('Read Test - Value for Node {}'.format(nid), value == expectedValue)
        print(' Expected value for Node {}:'.format(nid), expectedValue)
        print(' Value for Node {}:'.format(nid), value)

        # check browseName
        browse_name = node.get_browse_name()
        logger.add_test('Read Test - browse name for Node {}'.format(nid), browse_name == expectedBrowseName)
        print('expected browse_name: ', expectedBrowseName)
        print('browse_name: ', browse_name)

        # check display name
        display_name = node.get_display_name()
        logger.add_test('Read Test - display name for Node {}'.format(nid), display_name == expectedDisplayName)
        print('expected display_name: ', expectedDisplayName)
        print('display_name: ', display_name)

        # check node class
        class_name = node.get_node_class()
        logger.add_test('Read Test - node class for Node {}'.format(nid), str("NodeClass.Variable")==str(class_name))

        # TODO: check data type
        #data_type = node.get_data_type()
        #print('data type: ', data_type)

