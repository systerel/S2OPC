#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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

    # test secure connexions
    safety_secure_channels_test(client)
    #client.connect()
    print('Connected')

    endPoints = client.get_endpoints()
    print('endPoints:', endPoints)
    # Read tests
    n1 = client.get_node("ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM")
    browse_name = n1.get_browse_name()
    print('browse_name: ', browse_name)
    display_name = n1.get_display_name()
    print('display_name: ', display_name)
    class_name = n1.get_node_class()
    print('node_class: ', class_name)
    
    # browse tests
    print('children:')
    for n in n1.get_children():
        print('  ' + str(n.nodeid))
    
    # read tests
    n1 = client.get_node("ns=261;s=Objects.15361.SIGNALs.BALA_RDLS_G019.RM.ASK")
    print('n1:', n1.get_value())
    browse_name = n1.get_browse_name()
    print('browse_name: ', browse_name)
    display_name = n1.get_display_name()
    print('display_name: ', display_name)
    class_name = n1.get_node_class()
    print('node_class: ', class_name)      
    # write tests
    n1.set_value(ua.Variant(True, ua.VariantType.Boolean))
    print('n1:', n1.get_value())
    n1.set_value(ua.Variant(False, ua.VariantType.Boolean))
    print('n1:', n1.get_value())
    
    # node Id 1000 to 1006
    try:
        for nid in range(1001,1006):
            node = client.get_node(nid)
            original_value = node.get_value()
            print(' Value for Node {:03d}:'.format(nid), original_value)
            # TODO: adapt write to type of value
            #new_value = -original_value    
            #node.set_value(ua.Variant(new_value, ua.VariantType.Int64))
            #read_value = node.get_value()
            #print(' New value for Node {:03d}:'.format(nid), new_value)
            #assert (new_value==read_value)
    
    finally:
        client.disconnect()
        print('Disconnected')

