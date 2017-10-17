#!/usr/bin/env python3
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
#!/usr/bin/python3.4
#-*-coding:Utf-8 -*

# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

from common import sUri
from opcua import ua, Client

if __name__=='__main__':

    print('Connecting to', sUri)
    client = Client(sUri)
    client.connect()
    try:
        while True:
            for i in range(1,6):
                nid = 1000 + i
                node = client.get_node(nid)
                value = node.get_value()
                print(' Value for Node {:03d}:'.format(nid), value)
    finally:
        client.disconnect()
        print('Disconnected')



