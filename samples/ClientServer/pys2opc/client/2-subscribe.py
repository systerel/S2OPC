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
Example script: this script uses the automatically created subscription,
and add nodes to this subscription.

This is mostly illustrative, as the nodes do not change in value.
"""


import time
import os

# overload the default client method to get key password and username with environment variable
import utils 

from pys2opc import PyS2OPC_Client, BaseClientConnectionHandler

class PrintSubs(BaseClientConnectionHandler):
    """
    Derivates from BaseClientConnectionHandler to print values on datachange notifications.
    """
    def on_datachanged(self, nodeId, dataValue):
        #  dataValue contains the source and server timestamps, as well as the new value.
        print('Data changed "{}" -> {}, '.format(nodeId, dataValue.variant) + time.ctime(dataValue.timestampServer))


NODES = ['ns=1;s=Int32_007',
         'ns=1;s=UInt64_099',
         'ns=1;s=Double_032',  # A Double that is R/W but not TimestampWrite
         'ns=1;i=1004',  # A String
         'ns=1;i=1003',  # A Double that is R/W and TimestampWrite
        ]


if __name__ == '__main__':
    with PyS2OPC_Client.initialize():
        # Subscription parameters are defined in configuration_parameters_no_subscription
        configs = PyS2OPC_Client.load_client_configuration_from_file(os.path.join('S2OPC_Client_Wrapper_Config.xml'))
        with PyS2OPC_Client.connect(configs["write"], PrintSubs) as connection:
            # Add multiple nodes to the subscription.
            # The subscription always notifies of the first value.
            result = connection.add_nodes_to_subscription(NODES)
            assert all(result), 'Subscription failed for some NODES={} results={}'.format(NODES, result)
            # We wait for notifications up to 5 seconds.
            time.sleep(5.)
            connection.close_subscription()
            print('Closing the connection.')