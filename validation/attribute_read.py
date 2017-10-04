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


from opcua.ua import QualifiedName, LocalizedText, XmlElement

def attribute_read_tests(client):

    # Read tests
    Type_list = ['Int64','UInt32','Double','String','ByteString','XmlElement']
    Value_list = [-1000,1000,2.0,"String:INGOPCS","ByteString:INGOPCS".encode(),XmlElement(u"XmlElement:INGOPCS")]
    try:
        for (i,e) in enumerate(Type_list):
            nid = 1000 + i + 1
            print('Checking nid:', nid)
            expectedBrowseName = QualifiedName(e,0)
            expectedDisplayName = LocalizedText(u"{}_1dn".format(e))
            expectedDescription = u"{}_1d".format(e)
            expectedValue = Value_list[i]
            node = client.get_node(nid)

            # check value
            value = node.get_value()
            print(' Value for Node {:03d}:'.format(nid), value)
            assert(value == expectedValue)

            # check browseName
            browse_name = node.get_browse_name()
            print('browse_name: ', browse_name)
            assert(browse_name == expectedBrowseName)

            # check display name
            display_name = node.get_display_name()
            assert(display_name == expectedDisplayName)

            # check node class
            class_name = node.get_node_class()
            assert(str("NodeClass.Variable")==str(class_name))

            # TODO: check data type
            #data_type = node.get_data_type()
            #print('data type: ', data_type)

    finally:
        client.disconnect()
        print('Disconnected')

