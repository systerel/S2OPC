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

    nid, subPostfixNodeIdHierarchical, subNodeIdNonHierarchical, parentNid = browseSubTree

    # Hierarchical referenced node
    childrenNids = list()
    for bn in subPostfixNodeIdHierarchical:
        childrenNids.append('ns=1;s=' + bn)
    print("Browsing children of node {} expecting nodes {}".format(nid, childrenNids))
    n1 = client.get_node(nid)
     # Get all children of a node. By default hierarchical references and all node classes are returned.
    children = n1.get_children() # <=> n1.get_children(refs=33) for HierarchicalReferences
    # Checking number of children and their associated ids
    logger.add_test('Browse Test - number of children for Node {}. Expecting {} == {}'.format(nid, len(childrenNids), len(children)), len(childrenNids) == len(children))
    # There shall not be backward references
    node = Node(sUri, parentNid)
    logger.add_test('Browse Test - parent {} is not in browsed children'.format(parentNid), node not in children)
    # Checking forward references
    for childNid in childrenNids:
        print(childNid)
        node = Node(sUri, childNid)
        logger.add_test('Browse Test - child {} retrieved in browsed children'.format(childNid), node in children)

    nonHierarchicalNids = list(subNodeIdNonHierarchical)
    nonHierChildren = n1.get_children(refs=32) #NonHierarchicalReferences
    print ("Non hier children: "+str(nonHierChildren))
    # Checking number of children and their associated ids
    logger.add_test('Browse Test - number of non hierarchical children for Node {}. Expecting {} == {}'.format(nid, len(nonHierarchicalNids), len(nonHierChildren)), len(nonHierarchicalNids) == len(nonHierChildren))
    # Checking forward references
    for childNid in nonHierarchicalNids:
        print(childNid)
        node = Node(sUri, childNid)
        logger.add_test('Browse Test - non hierarchical child '+childNid, node in nonHierChildren)
