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

from pys2opc import PyS2OPC_Client as PyS2OPC, BaseClientConnectionHandler
from _connection_configuration import configuration_parameters_no_subscription, configuration_parameters_subscription, configuration_parameters_security, join_configs


NODES_A = ['ns=1;s=Int32_007',
           'ns=1;s=UInt64_099']
NODES_B = ['ns=1;s=Double_032',
           'ns=1;i=1004',
           'ns=1;i=1003']

class PrintSubs(BaseClientConnectionHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.tag = ''  # Will use this tag to differentiate connections with subscriptions

    def on_datachanged(self, nodeId, dataValue):
        print('  Data changed on connection "{}", "{}" -> {}, '.format(self.tag, nodeId, dataValue.variant) + time.ctime(dataValue.timestampServer))


if __name__ == '__main__':
    with PyS2OPC.initialize():
        config_unsec_nosub = PyS2OPC.add_configuration_unsecured(**configuration_parameters_no_subscription)
        config_sec_nosub = PyS2OPC.add_configuration_secured(**join_configs(configuration_parameters_no_subscription, configuration_parameters_security))
        config_unsec_sub = PyS2OPC.add_configuration_unsecured(**configuration_parameters_subscription)
        config_sec_sub = PyS2OPC.add_configuration_secured(**join_configs(configuration_parameters_subscription, configuration_parameters_security))
        PyS2OPC.mark_configured()
        connections = [PyS2OPC.connect(config, PrintSubs) for config in (config_unsec_nosub, config_sec_nosub,
                                                                         config_unsec_sub, config_sec_sub)]
        conn_unsec_nosub, conn_sec_nosub, conn_unsec_sub, conn_sec_sub = connections
        conn_unsec_sub.tag = 'unsecure'
        conn_sec_sub.tag = 'secure'

        try:
            # Add node to subscriptions. This is always synchronous.
            # On secured connection, call the function twice.
            conn_sec_sub.add_nodes_to_subscription(NODES_A)
            conn_sec_sub.add_nodes_to_subscription(NODES_B)
            # On unsecured connection, calls it once.
            conn_unsec_sub.add_nodes_to_subscription(NODES_A + NODES_B)

            # Reads
            # On secured connection, make two "simultaneous" asynchronous reads.
            readA = conn_sec_nosub.read_nodes(NODES_A, bWaitResponse=False)
            readB = conn_sec_nosub.read_nodes(NODES_B, bWaitResponse=False)
            # On unsecured connection, make a synchronous read.
            respRead = conn_unsec_nosub.read_nodes(NODES_A + NODES_B)

            # readA and readB are Requests. Manually wait on the responses and display them.
            t0 = time.time()
            respA = conn_sec_nosub.get_response(readA)
            while respA is None:
                respA = conn_sec_nosub.get_response(readA)
                if time.time() - t0 > 1.:  # Wait at most 1 second
                    break
            respB = conn_sec_nosub.get_response(readB)
            while respB is None:
                respB = conn_sec_nosub.get_response(readB)
                if time.time() - t0 > 1.:
                    break
            assert respA is not None and respB is not None
            for node, dvAsynch, dvSynch in zip(NODES_A+NODES_B, readA.response.results+readB.response.results, respRead.results):
                assert dvAsynch.variant == dvSynch.variant, 'Read on secured and unsecured connection yielded different values.'
                print('  Value of {} is {}, timestamp is {}'.format(node, str(dvAsynch.variant), time.ctime(dvAsynch.timestampSource)))

            # Waits at least a publish cycle before quitting, otherwise the callback may never be called
            time.sleep(configuration_parameters_subscription['publish_period']/1000)
        finally:
            # Always clean all connections, even when there is a problem
            for connection in connections:
                connection.disconnect()

