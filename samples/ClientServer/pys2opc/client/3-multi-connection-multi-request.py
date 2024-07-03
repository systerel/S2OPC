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
Example script: this script illustrate how to hanlde multiple connections and asynchronous requests.
Some connections are secured and some unsecured. A subscription is made on the secured connection.
"""


import time
import os

# overload the default client method to get key password and username with environment variable
import utils 

from pys2opc import PyS2OPC_Client, BaseClientConnectionHandler, AsyncResponse

NODES_A = ['ns=1;s=Int32_007',
           'ns=1;s=UInt64_099']
NODES_B = ['ns=1;s=Double_032',
           'ns=1;i=1004',
           'ns=1;i=1003']

class PrintSubs(BaseClientConnectionHandler):

    def on_datachanged(self, nodeId, dataValue):
        print('  Data changed "{}" -> {}, '.format(nodeId, dataValue.variant) + time.ctime(dataValue.timestampServer))


if __name__ == '__main__':
    with PyS2OPC_Client.initialize():
        configs = PyS2OPC_Client.load_client_configuration_from_file(os.path.join('S2OPC_Client_Wrapper_Config.xml'))
        connections = [PyS2OPC_Client.connect(configs[ID], PrintSubs) for ID in ("read", "write")]
        conn_1, conn_2 = connections

        try:
            # Add node to subscriptions. This is always synchronous.
            # On secured connection, call the function twice.
            conn_2.add_nodes_to_subscription(NODES_A)
            conn_2.add_nodes_to_subscription(NODES_B)

            # Reads
            # On secured connection, make two "simultaneous" asynchronous reads.
            readA: AsyncResponse = conn_1.read_nodes(nodeIds=NODES_A, bWaitResponse=False)
            readB: AsyncResponse = conn_1.read_nodes(nodeIds=NODES_B, bWaitResponse=False)
            # On secured connection, make a synchronous read.
            respRead = conn_2.read_nodes(NODES_A + NODES_B)

            # readA and readB are Requests. Manually wait on the responses and display them.
            t0 = time.time()
            respA = readA.get_response()
            while respA is None:
                respA = readA.get_response()
                if time.time() - t0 > 1.:  # Wait at most 1 second
                    break
            respB = readB.get_response()
            while respB is None:
                respB = readB.get_response()
                if time.time() - t0 > 1.:
                    break
            assert respA is not None and respB is not None
            for node, dvAsynch, dvSynch in zip(NODES_A+NODES_B, respA.results+respB.results, respRead.results):
                assert dvAsynch.variant == dvSynch.variant, 'Read on secured connection yielded different values.'
                print('  Value of {} is {}, timestamp is {}'.format(node, str(dvAsynch.variant), time.ctime(dvAsynch.timestampSource)))

            # Waits at least a publish cycle before quitting, otherwise the callback may never be called
            time.sleep(5)
        finally:
            # Always clean all subscritions and connections, even when there is a problem
            for connection in connections:
                connection.close_subscription()
                connection.disconnect()