#!/usr/bin/python3
# -*- coding: utf-8 -*-

# Copyright (C) 2017 Systerel and others.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
#!/usr/bin/python3.4
#-*-coding:Utf-8 -*

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

