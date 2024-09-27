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
PyS2OPC library server validation tests :
- server starting and closing
- simultaneous use of client and server
- local services of server : read, write, browse
- server method serve() (with write notification callback check)
- server method serve_forever() (with write notification callback check)
"""

from itertools import product
import time
import threading
import os
import sys
from multiprocessing import Process

from pys2opc import PyS2OPC_Server, PyS2OPC_Client, BaseAddressSpaceHandler, AttributeId, StatusCode, SOPC_Failure, DataValue, VariantType
import utils
import wait_server
from tap_logger import TapLogger

NODES = ['ns=1;s=Int32_007', 'ns=1;s=UInt64_099']
TEST_PASSWORD_PRIV_KEY_ENV_NAME = "TEST_PASSWORD_PRIVATE_KEY"

class AddressSpaceHandler(BaseAddressSpaceHandler):

    def __init__(self, logger=None):
        self.logger = logger
        self.count_cb: int = 0 # Counts the number of times the write notification callback is called to compare with the numbre of writes in the test.

    def on_datachanged(self, nodeId, attrId, dataValue, indexRange, status):
        print("Data change :", nodeId, AttributeId.get_name_from_id(attrId), dataValue, indexRange, StatusCode.get_name_from_id(status))
        self.count_cb = self.count_cb + 1
        if self.logger != None:
            self.logger.add_test('Callback called.', True)


class PyS2OPC_Server_Test():
    @staticmethod
    def get_server_key_password():
        pwd = os.getenv(TEST_PASSWORD_PRIV_KEY_ENV_NAME)
        if pwd is None:
            print('The following environment variable is missing: ' + TEST_PASSWORD_PRIV_KEY_ENV_NAME)
        return pwd

# overload the default password method for tests
PyS2OPC_Server.get_server_key_password = PyS2OPC_Server_Test.get_server_key_password

# global variable corresponding to NODES #
DV_ns1_Int32_007 = DataValue.from_python(55, False)
DV_ns1_Int32_007.variantType= VariantType.Int32
DV_ns1_UInt64_099 = DataValue.from_python(778899, False)
DV_ns1_UInt64_099.variantType= VariantType.UInt64

def client_write_nodes(nodes, datavalues):
    with PyS2OPC_Client.initialize():
        try :
            configs = PyS2OPC_Client.load_client_configuration_from_file(os.path.join('S2OPC_Client_Wrapper_Config.xml'))
            connect = PyS2OPC_Client.connect(configs["write"])
            connect.write_nodes(nodes, datavalues)
        except SOPC_Failure as f:
            print(f.what())

def start_server_forever(addSpaceHandler):
    with PyS2OPC_Server.initialize():
        PyS2OPC_Server.load_server_configuration_from_files(os.path.join('S2OPC_Server_Demo_Config.xml'),
                                                                    os.path.join('S2OPC_Demo_NodeSet.xml'),
                                                                    os.path.join('S2OPC_Users_Demo_Config.xml'),
                                                                    addSpaceHandler)
        PyS2OPC_Server.serve_forever()
    # Exits with addSpaceHandler.count_cb as exit code
    exit(addSpaceHandler.count_cb)

if __name__ == '__main__':

    # --- Test server local services --- #

    # Start server #
    logger_ = TapLogger('validation_pys2opc_server.tap')
    logger_.begin_section('Start server -')
    logger_.add_test('Server not yet initialized.', not(PyS2OPC_Server._initialized_srv))
    with PyS2OPC_Server.initialize():
        logger_.add_test('Server initialized.', PyS2OPC_Server._initialized_srv)
        PyS2OPC_Server.load_server_configuration_from_files(os.path.join('S2OPC_Server_Demo_Config.xml'),
                                                                    os.path.join('S2OPC_Demo_NodeSet.xml'),
                                                                    os.path.join('S2OPC_Users_Demo_Config.xml'),
                                                                    AddressSpaceHandler())
        logger_.add_test('Server not yet serving.', not(PyS2OPC_Server._serving))
        with PyS2OPC_Server.serve():
            logger_.add_test('Server serving.', PyS2OPC_Server._serving)
    # Server started #

            # Connect client #
            logger_.begin_section('Connect client -')
            res_connect_client:bool = True
            with PyS2OPC_Client.initialize():
                try :
                    configs = PyS2OPC_Client.load_client_configuration_from_file(os.path.join('S2OPC_Client_Wrapper_Config.xml'))
                    connect = PyS2OPC_Client.connect(configs["read"])
                except SOPC_Failure as f:
                    print(f.what())
                    res_connect_client = False
                logger_.add_test('Client connected, simultaneous use of client and server demonstrated.', res_connect_client)
            # Client connected #

                # Start Local Read Tests #
                print("Server Local Read Tests")
                logger_.begin_section('Server Local Read Tests -')
                try :
                    read_response_server = PyS2OPC_Server.read_nodes(NODES)
                    read_response_client = connect.read_nodes(NODES)
                    for node, dv_server, dv_client in zip(NODES, read_response_server.results, read_response_client.results):
                        logger_.add_test('Server and Client read same value for node : ' + node, 
                                        dv_server.variant == dv_client.variant)
                except SOPC_Failure as f:
                    print(f.what())
                    logger_.add_test('SOPC_Failure : ' + f.what(), False)
                # End Local Read Tests #

                # Start Local Write Tests #
                print("Server Local Write Tests")
                logger_.begin_section('Server Local Write Tests -')
                # Read check
                try :
                    write_response_server = PyS2OPC_Server.write_nodes(NODES, [DV_ns1_Int32_007,DV_ns1_UInt64_099])
                    logger_.add_test('Write nodes : ' + str(NODES), write_response_server.is_ok())
                    read_response_server = PyS2OPC_Server.read_nodes(NODES)
                    read_response_client = connect.read_nodes(NODES)
                    for node, dv_server, dv_client, dv_ref in zip(NODES, read_response_server.results, read_response_client.results, [DV_ns1_Int32_007, DV_ns1_UInt64_099]):
                        logger_.add_test('Server and Client read the expected value for node : ' + node, 
                                        dv_server.variant == dv_client.variant == dv_ref.variant)
                except SOPC_Failure as f:
                    print(f.what())
                    logger_.add_test('SOPC_Failure : ' + f.what(), False)
                # End Local Write Tests #

                # Start Local Browse Tests #
                print("Server Local Browse Tests")
                logger_.begin_section('Server Local Browse Tests -')
                try:
                    browse_response_server = PyS2OPC_Server.browse_nodes(NODES)
                    browse_response_client = connect.browse_nodes(NODES)
                    logger_.add_test('Browse nodes : ' + str(NODES), browse_response_server.is_ok())
                    for node, bwsRes_server, bwsRes_client in zip(NODES, browse_response_server.results, browse_response_client.results):
                        logger_.add_test('Server and Client have the same browse status for node : ' + node,
                                         bwsRes_server.status == bwsRes_client.status)
                        logger_.add_test('Server and Client have the same browse continuationPoint for node : ' + node,
                                         bwsRes_server.continuationPoint == bwsRes_client.continuationPoint)
                        logger_.add_test('Server and Client have the same browse references for node : ' + node,
                                         str(bwsRes_server.references) == str(bwsRes_client.references))
                except SOPC_Failure as f:
                    print(f.what())
                    logger_.add_test('SOPC_Failure : ' + f.what(), False)
                # End Local Browse Tests #

            # Close server #
        logger_.begin_section('Close server -')
        logger_.add_test('Server has finished serving.', not(PyS2OPC_Server._serving))
    logger_.add_test('Server uninitialized.', not(PyS2OPC_Server._initialized_srv))
    # Server closed #

    # --- Test 2 server modes : `serve()`, `serve_forever()` --- #

    # Test server with `serve()` #
    logger_.begin_section('Test server method : serve() -')
    with PyS2OPC_Server.initialize():
        addSpaceHandler = AddressSpaceHandler(logger=logger_)
        PyS2OPC_Server.load_server_configuration_from_files(os.path.join('S2OPC_Server_Demo_Config.xml'),
                                                                    os.path.join('S2OPC_Demo_NodeSet.xml'),
                                                                    os.path.join('S2OPC_Users_Demo_Config.xml'),
                                                                    addSpaceHandler)
        logger_.add_test('Server not yet serving.', not(PyS2OPC_Server._serving))
        with PyS2OPC_Server.serve():
            logger_.add_test('Server serving.', PyS2OPC_Server._serving)

            # Client writes on 2 nodes #
            client_write_nodes(NODES, [DV_ns1_Int32_007,DV_ns1_UInt64_099])
            logger_.add_test('Server is passed the right number of times in the write notification callback. Expected = {}. Result = {}.'
                            .format(len(NODES), addSpaceHandler.count_cb), len(NODES) == addSpaceHandler.count_cb)

    # Test server with `serve_forever()` #
    logger_.begin_section('Test server method : serve_forever() -')
    p_server = Process(target=start_server_forever, args=(AddressSpaceHandler(),))
    p_server.start()
    if not wait_server.wait_server(wait_server.DEFAULT_URL, wait_server.TIMEOUT):
        print('Timeout for starting server')
        # 2 times to avoid OPCUA shutdown phase
        p_server.kill()
        p_server.kill()
        p_server.join()
        logger_.add_test('Server failed to start.',False)
    else: # Server started 
        print('Server started')
        logger_.add_test('Server serving.', True)
        # Client writes on 2 nodes #
        client_write_nodes(NODES, [DV_ns1_Int32_007,DV_ns1_UInt64_099])
        p_server.terminate()
        p_server.join()
        # p_server.exitcode contains AddressSpaceHandler.count_cb
        logger_.add_test('Server is passed the right number of times in the write notification callback. Expected = {}. Result = {}.'
                        .format(len(NODES), p_server.exitcode), len(NODES) == p_server.exitcode)

    logger_.finalize_report()
    sys.exit(1 if logger_.has_failed_tests else 0)