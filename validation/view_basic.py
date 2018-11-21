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

from opcua.common.node import Node
from common import sUri, browseSubTree

def browse_tests(client, logger):

    nid, subBrowseNames, subNids = browseSubTree
    subNids = list(subNids)
    for bn in subBrowseNames:
        subNids.append(nid + '.' + bn)
    print("Browsing children of node", nid)
    n1 = client.get_node(nid)
    children = n1.get_children()
    # Checking number of children and their associated ids
    logger.add_test('Browse Test - number of children for Node '+nid, len(children) == len(subNids))
    # There shall not be backward references
    parentNid = nid[:nid.rfind('.')]
    node = Node(sUri, parentNid)
    logger.add_test('Browse Test - child '+parentNid, node not in children)
    # Checking forward references
    for subNid in subNids:
        print(subNid)
        node = Node(sUri, subNid)
        logger.add_test('Browse Test - child '+subNid, node in children)

