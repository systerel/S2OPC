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

from pys2opc import PyS2OPC_Client as PyS2OPC, BaseClientConnectionHandler, DataValue, StatusCode, Variant, VariantType
from _connection_configuration import configuration_parameters_no_subscription


# All those nodes are Variable nodes. They are demo nodes which have the name of the type it contains.
NODES_TO_READ = ['ns=1;s=Int32_007',
                 'ns=1;s=UInt64_099',
                 'ns=1;s=Double_032',  # A Double that is R/W but not TimestampWrite
                 'ns=1;i=1004',  # A String
                 'ns=1;i=1003',  # A Double that is R/W and TimestampWrite
                ]


if __name__ == '__main__':
    print(PyS2OPC.get_version())
    print()

    # Initialize the toolkit and automatically clean it when the script finishes
    # Set the path for logs to a new temp dir
    pathLog = tempfile.mkdtemp()
    print('Log saved to', pathLog)
    with PyS2OPC.initialize(logLevel=0, logPath=pathLog):
        # Configure a connection and freeze the S2OPC configurations.
        # See the documentation of this function for all the parameters.
        config = PyS2OPC.add_configuration_unsecured(**configuration_parameters_no_subscription)
        PyS2OPC.mark_configured()
        # Use the configuration to create a new connection.
        # The connection is automatically closed when reaching out of the with context.
        # The default BaseClientConnectionHandler is used, as we do not intent to use subscription.
        with PyS2OPC.connect(config, BaseClientConnectionHandler) as connection:
            # Make a read. Responses are in the same order. By default, reads the Value attribute.
            respRead = connection.read_nodes(NODES_TO_READ)
            for node, datavalue in zip(NODES_TO_READ, respRead.results):
                print('  Value of {} is {}, status code is {} (0x{:08X}), timestamp is {}'.format(node, str(datavalue.variant), StatusCode.get_name_from_id(datavalue.statusCode), datavalue.statusCode, time.ctime(datavalue.timestampSource)))
            # Make a write. Choose random values for doubles.
            nodes = [node for node,dv in zip(NODES_TO_READ, respRead.results) if dv.variantType == VariantType.Double]
            newValues = [DataValue.from_python(Variant(10*random.random(), variantType=VariantType.Double))
                         for dv in respRead.results if dv.variantType == VariantType.Double]
            # It is also possible to change the source timestamp manually, ot the status code
            #for dv in newValues:
            #    dv.timestampSource = 0
            #    dv.statusCode = StatusCode.Bad

            respWrite = connection.write_nodes(nodes, newValues)
            print('Written.')
            try:
                assert respWrite.is_ok()
            except AssertionError:
                print(', '.join(map(lambda sc: '{} (0x{:08X})'.format(StatusCode.get_name_from_id(sc), sc), respWrite.results)))
                raise
            # Make a new read, expect the values to have changed.
            respRead = connection.read_nodes(nodes)
            for node, datavalue, expected in zip(nodes, respRead.results, newValues):
                assert expected.variant == datavalue.variant, expected.variant - datavalue.variant
                print('  Value of {} is {}, timestamp is {}'.format(node, str(datavalue.variant), time.ctime(datavalue.timestampSource)))

