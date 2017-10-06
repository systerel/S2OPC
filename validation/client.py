#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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

"""
Simple client to launch validation tests
"""

from opcua import ua, Client
from attribute_read import attribute_read_tests
from attribute_write_values import attribute_write_values_tests
from safety_secure_channels import safety_secure_channels_test

if __name__=='__main__':
    sUri = 'opc.tcp://localhost:4841'
    print('Connecting to', sUri)
    client = Client(sUri)

    try:
        # test secure connexions
        safety_secure_channels_test(client)
        #client.connect()
        print('Connected')

        endPoints = client.get_endpoints()
        #print('endPoints:', endPoints)

        # Read tests
        attribute_read_tests(client)

        # write tests
        attribute_write_values_tests(client)

    finally:
        client.disconnect()
        print('Disconnected')

    # browse tests
#    n1 = client.get_node("ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM")
#    print('children:')
#    for n in n1.get_children():
#        print('  ' + str(n.nodeid))
    
    # write tests
    #n1 = client.get_node("ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.ASK")
    #n1.set_value(ua.Variant(True, ua.VariantType.Boolean))
    #print('n1:', n1.get_value())
    #n1.set_value(ua.Variant(False, ua.VariantType.Boolean))
    #print('n1:', n1.get_value())
    
