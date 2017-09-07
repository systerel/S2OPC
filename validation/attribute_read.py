
def attribute_read_tests(client):

    n1 = client.get_node(10)
    n2 = client.get_node(11)

    print('n1:', n1.get_value())
    browse_name = n1.get_browse_name()
    print('browse_name: ', browse_name)
    display_name = n1.get_display_name()
    print('display_name: ', display_name)
    class_name = n1.get_node_class()
    print('node_class: ', class_name)
    print('n2:', n2.get_value())




