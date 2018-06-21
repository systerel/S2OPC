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
from common import sUri

def browse_tests(client, logger):

    print("Browsing children of node ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019")
    n1 = client.get_node("ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019")
    children = n1.get_children()
    # checking number of children and their associated ids
    logger.add_test('Browse Test - number of children for Node ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019', len(children) == 6)
    #backward references shall not be taken into account
    node = Node(sUri,"ns=261;s=Objects.15361.SIGNALs")
    logger.add_test('Browse Test - child ns=261;s=Objects.15361.SIGNALs', node not in children)
    #checking forward references
    node = Node(sUri,"i=61")
    logger.add_test('Browse Test - child i=61', node in children)
    node = Node(sUri,"ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM")
    logger.add_test('Browse Test - child ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM', node in children)
    node = Node(sUri,"ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC")
    logger.add_test('Browse Test - child ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC', node in children)
    node = Node(sUri,"ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.SendCommand")
    logger.add_test('Browse Test - child ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.SendCommand', node in children)
    node = Node(sUri,"ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.OffBlocking-K")
    logger.add_test('Browse Test - child ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.OffBlocking-K', node in children)
    node = Node(sUri,"ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.OffBlocking-CC")
    logger.add_test('Browse Test - child ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.OffBlocking-CC', node in children)

