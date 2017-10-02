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




