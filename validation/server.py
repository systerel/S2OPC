#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (C) 2018 Systerel and others.
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
Simple server handling miscellaneous client tests
"""

import argparse
import time

from opcua import ua, Server
from common import sUri, variantInfoList
#from tap_logger import TapLogger
from opcua.crypto import security_policies

if __name__=='__main__':
    parser = argparse.ArgumentParser(description='FreeOpcUa test server')
    parser.add_argument('msTimeout', nargs='?', default=10000., type=float,
                        help='Server timeout (ms)')
    args = parser.parse_args()

    # Open server
    print('Configuring FreeOpcUa test server')
    server = Server()
    server.set_endpoint(sUri)
    server.load_certificate('../bin/server_public/server_2k.der')
    server.load_private_key('../bin/server_private/server_2k.pem')

    # Nodes are created under the Objects node
    objects = server.get_objects_node()
    # Use the same nodes as in the Read test
    for i,(sTypName,typ,val,_) in enumerate(variantInfoList):
        nid = 1000 + i + 1
        node = objects.add_variable(ua.NodeId(nid, 0), sTypName, ua.Variant(val, typ))
        node.set_writable()

    # Starts the server
    server.start()
    try:
        print('Server started. Stops in {:.2f} s'.format(args.msTimeout/1000.))
        # The freeopcua toolkit is heavily an asyncio thing, which is run in another thread
        #  but here we have nothing else to do...
        time.sleep(args.msTimeout/1000.)
    finally:
        server.stop()
