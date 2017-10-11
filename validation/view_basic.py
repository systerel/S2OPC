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

def browse_tests(client):

    print("Browsing children of node ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019")
    print("Checking children identifiers")
    n1 = client.get_node("ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019")
    children = n1.get_children()
    assert(len(children) == 7)
    node = Node(sUri,"ns=261;s=Objects.15361.SIGNALs")
    assert(node in children)
    node = Node(sUri,"i=61")
    assert(node in children)
    node = Node(sUri,"ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM")
    assert(node in children)
    node = Node(sUri,"ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RC")
    assert(node in children)
    node = Node(sUri,"ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.SendCommand")
    assert(node in children)
    node = Node(sUri,"ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.OffBlocking-K")
    assert(node in children)
    node = Node(sUri,"ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.OffBlocking-CC")
    assert(node in children)

