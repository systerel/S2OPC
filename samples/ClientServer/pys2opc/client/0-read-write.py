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
Example script: this script initializes the toolkit, creates a unsecured connection,
makes a single read on multiple nodes, prints the result, write something else, reads again,
and prints the new results.
"""


import time
import tempfile
import random
import os

# overload the default client method to get key password and username with environment variable
import utils 

from pys2opc import PyS2OPC, PyS2OPC_Client, SOPC_Log_Level, VariantType, DataValue, Variant

# All those nodes are Variable nodes. They are demo nodes which have the name of the type it contains.
NODES_TO_READ = ['ns=1;s=Int32_007',
                 'ns=1;s=UInt64_099',
                 'ns=1;s=Double_032',  # A Double that is R/W but not TimestampWrite
                 'ns=1;i=1004',  # A String
                 'ns=1;i=1003',  # A Double that is R/W and TimestampWrite
                ]

if __name__ == '__main__':
    print(PyS2OPC.get_version())
    # Set the path for logs to a new temp dir
    pathLog = tempfile.mkdtemp() + "/"
    print('Log saved to', pathLog)
    # Initialize the toolkit and automatically clean it when the script finishes
    with PyS2OPC_Client.initialize(logLevel=SOPC_Log_Level.SOPC_LOG_LEVEL_ERROR, logPath=pathLog):
        # Configure a connection and freeze the S2OPC configurations.
        configs = PyS2OPC_Client.load_client_configuration_from_file(os.path.join('S2OPC_Client_Wrapper_Config.xml'))
        # Create a new connection by retrieving configuration from ID in the previous XML.
        # The connection is automatically closed when reaching out of the with context.
        with PyS2OPC_Client.connect(configs["write"]) as connection:
            # Make a read. Responses are in the same order. By default, reads the Value attribute.
            respRead = connection.read_nodes(nodeIds=NODES_TO_READ)
            for node, datavalue in zip(NODES_TO_READ, respRead.results):
                print('Value of {} is {}, status code is 0x{:08X}, timestamp server is {}, timestamp source is {}'.format(
                    node, str(datavalue.variant), datavalue.statusCode, time.ctime(datavalue.timestampServer), time.ctime(datavalue.timestampSource)))
            # Make a write. Choose random values for doubles.
            nodes = [node for node,dv in zip(NODES_TO_READ, respRead.results) if dv.variantType == VariantType.Double]
            newValues = [DataValue.from_python(Variant(10*random.random(), variantType=VariantType.Double))
                            for dv in respRead.results if dv.variantType == VariantType.Double]
            for dv in newValues:
                dv.timestampSource = 0
            respWrite = connection.write_nodes(nodes, newValues)
            if respWrite.is_ok():
                print("Node : {} successfully written !".format(nodes))
            else:
                print("Write fail !")
            # Make a new read, expect the values to have changed.
            respRead = connection.read_nodes(nodes)
            for node, datavalue, expected in zip(nodes, respRead.results, newValues):
                assert expected.variant == datavalue.variant
                print('Value of {} is {}, status code is 0x{:08X}, expected value is {}'.format(
                    node, str(datavalue.variant), datavalue.statusCode, expected.variant))