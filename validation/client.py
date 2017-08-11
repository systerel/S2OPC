#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Simple client to launch validation tests
"""

from opcua import ua, Client

if __name__=='__main__':
    sUri = 'opc.tcp://localhost:4841'
    print('Connecting to', sUri)
    client = Client(sUri)
    client.connect()
    endPoints = client.get_endpoints()
    print('endPoints:', endPoints)
    # Read tests
    n1 = client.get_node(10)
    print('n1:', n1.get_value())
    browse_name = n1.get_browse_name()
    print('browse_name: ', browse_name)
    display_name = n1.get_display_name()
    print('display_name: ', display_name)
    class_name = n1.get_node_class()
    print('node_class: ', class_name)
    n2 = client.get_node(11)
    print('n2:', n2.get_value())
    # write tests
    n1.set_value(ua.Variant(23, ua.VariantType.Int64))
    print('n1:', n1.get_value())
    client.disconnect()

