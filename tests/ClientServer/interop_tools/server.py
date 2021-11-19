#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Licensed to Systerel under one or more contributor license
# agreements. See the NOTICE file distributed with this work
# for additional information regarding copyright ownership.
# Systerel licenses this file to you under the Apache
# License, Version 2.0 (the "License"); you may not use this
# file except in compliance with the License. You may obtain
# a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.

"""
Simple server handling miscellaneous client tests
"""

import argparse
import os.path
import time
import signal
import sys

from opcua import ua, Server
from common import sUri, variantInfoList

stopFlag = False

def signal_handler(sig, frame):
    global stopFlag
    print('Stopping server due to SIGINT/SIGTERM')
    if stopFlag:
        sys.exit(0)
    else:
        stopFlag = True


if __name__=='__main__':
    parser = argparse.ArgumentParser(description='FreeOpcUa test server')
    parser.add_argument('msTimeout', nargs='?', default=10000., type=float,
                        help='Server timeout (ms)')
    args = parser.parse_args()

    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)

    # Open server
    print('Configuring FreeOpcUa test server')
    #logging.basicConfig(level=logging.DEBUG)
    server = Server()
    server.set_endpoint(sUri)

    cert_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..', 'samples', 'ClientServer', 'data', 'cert')
    server.load_certificate(os.path.join(cert_dir, 'server_4k_cert.der'))
    server.load_private_key(os.path.join(cert_dir, 'server_4k_key.pem'))

    # Nodes are created under the Objects node
    objects = server.get_objects_node()
    # Use the same nodes as in the Read test
    for i,(sTypName,typ,val,_) in enumerate(variantInfoList):
        nid = 1000 + i + 1
        node = objects.add_variable(ua.NodeId(nid, 0), sTypName, ua.Variant(val, typ))
        node.set_writable()

    # Add a node which increments, the target of an interesting subscription
    nodeCnt = objects.add_variable(ua.NodeId('Counter', 0), 'Counter', ua.Variant(0, ua.VariantType.UInt64))
    nodeCnt.set_writable()
    # Add a writable node, so that a client can subscribe to it, while another one can modify it.
    nodeStr = objects.add_variable(ua.NodeId('StatusString', 0), 'StatusString', ua.Variant('Everything is ok.', ua.VariantType.String))
    nodeStr.set_writable()

    # Starts the server
    server.start()
    try:
        print('Server started. Stops in {:.2f} s'.format(args.msTimeout/1000.))
        t0 = time.time()+args.msTimeout/1000.
        # The freeopcua toolkit is heavily an asyncio thing, which is run in another thread
        i = 0
        while time.time() < t0 and not stopFlag:
            # The counter is updated every ~100ms
            i += 1
            nodeCnt.set_value(i, varianttype=ua.VariantType.UInt64)
            time.sleep(.1)
    finally:
        server.stop()
