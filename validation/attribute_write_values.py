from opcua import ua

def attribute_write_values_tests(client):

    n1 = client.get_node(10)
    
    n1.set_value(ua.Variant(23, ua.VariantType.Int64))
    print('n1:', n1.get_value())

