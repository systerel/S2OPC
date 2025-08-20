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
import os
import sys
from threading import Thread

from pys2opc import PyS2OPC_Server, PyS2OPC_Client, BaseAddressSpaceHandler, AttributeId, StatusCode, SOPC_Failure, DataValue, VariantType, Response
import utils
import wait_server
from tap_logger import TapLogger

NODES = ['ns=1;s=Int32_007', 'ns=1;s=UInt64_099']
TEST_PASSWORD_PRIV_KEY_ENV_NAME = "TEST_PASSWORD_PRIVATE_KEY"

class AddressSpaceHandler(BaseAddressSpaceHandler):

    def __init__(self):
        self.count_cb: int = 0 # Counts the number of times the write notification callback is called to compare with the numbre of writes in the test.

    def on_datachanged(self, nodeId, attrId, dataValue, indexRange, status):
        print("Data change :", nodeId, AttributeId.get_name_from_id(attrId), dataValue, indexRange, StatusCode.get_name_from_id(status))
        self.count_cb = self.count_cb + 1


class PyS2OPC_Server_Test():
    @staticmethod
    def get_server_key_password():
        pwd = os.getenv(TEST_PASSWORD_PRIV_KEY_ENV_NAME)
        if pwd is None:
            print('The following environment variable is missing: ' + TEST_PASSWORD_PRIV_KEY_ENV_NAME)
        return pwd

# overload the default password method for tests
PyS2OPC_Server.get_server_key_password = PyS2OPC_Server_Test.get_server_key_password

# global variable corresponding to DataValues to write into NODES #
DV_ns1_Int32_007 = DataValue.from_python(55, False)
DV_ns1_Int32_007.variantType= VariantType.Int32
DV_ns1_UInt64_099 = DataValue.from_python(778899, False)
DV_ns1_UInt64_099.variantType= VariantType.UInt64

def client_write_nodes_count_cb(nodes, datavalues, addSpaceHandler) -> bool:
    '''
    Returns True if the Server write notification callback is called the right number of times (= number of written nodes)
    Otherwise returns False
    '''
    with PyS2OPC_Client.initialize():
        try :
            configs = PyS2OPC_Client.load_client_configuration_from_file(os.path.join('S2OPC_Client_Wrapper_Config.xml'))
            connect = PyS2OPC_Client.connect(configs["write"])
            connect.write_nodes(nodes, datavalues)
            t0 = time.time()
            while len(nodes) != addSpaceHandler.count_cb:
                time.sleep(.05)
                if time.time() - t0 > 0.5: # TimeOut for server callback call = 0.5s
                    print('ERROR: Server write notification callback is called the wrong number of times. Expected = {}. Result = {}.'
                        .format(len(nodes), addSpaceHandler.count_cb), len(nodes) == addSpaceHandler.count_cb)
                    return False
        except SOPC_Failure as f:
            print(f)
            return False
    return True

def start_server_forever(addSpaceHandler):
    with PyS2OPC_Server.initialize():
        PyS2OPC_Server.load_server_configuration_from_files(os.path.join('S2OPC_Server_Demo_Config.xml'),
                                                                    os.path.join('S2OPC_Demo_NodeSet.xml'),
                                                                    os.path.join('S2OPC_Users_Demo_Config.xml'),
                                                                    addSpaceHandler)
        PyS2OPC_Server.serve_forever()


if __name__ == '__main__':

    # --- Test server local services --- #
    # Start server #
    logger = TapLogger('validation_pys2opc_server.tap')
    logger.begin_section('Start server -')
    logger.add_test('Server not yet initialized.', not(PyS2OPC_Server._initialized_srv))
    with PyS2OPC_Server.initialize():
        logger.add_test('Server initialized.', PyS2OPC_Server._initialized_srv)
        PyS2OPC_Server.load_server_configuration_from_files(os.path.join('S2OPC_Server_Demo_Config.xml'),
                                                                    os.path.join('S2OPC_Demo_NodeSet.xml'),
                                                                    os.path.join('S2OPC_Users_Demo_Config.xml'),
                                                                    AddressSpaceHandler())
        logger.add_test('Server not yet serving.', not(PyS2OPC_Server._serving))
        with PyS2OPC_Server.serve():
            logger.add_test('Server serving.', PyS2OPC_Server._serving)
    # Server started #

            # Connect client #
            logger.begin_section('Connect client -')
            res_connect_client: bool = True
            with PyS2OPC_Client.initialize():
                try :
                    configs = PyS2OPC_Client.load_client_configuration_from_file(os.path.join('S2OPC_Client_Wrapper_Config.xml'))
                    connect = PyS2OPC_Client.connect(configs["read"])
                except SOPC_Failure as f:
                    print(f)
                    res_connect_client = False
                logger.add_test('Client connected, simultaneous use of client and server demonstrated.', res_connect_client)
            # Client connected #

                res: bool = res_connect_client
                # Start Local Read Tests #
                print("Server Local Read Tests")
                logger.begin_section('Server Local Read Tests -')
                if res:
                    try :
                        read_response_server = PyS2OPC_Server.read_nodes(NODES)
                        read_response_client = connect.read_nodes(NODES)
                        for node, dv_server, dv_client in zip(NODES, read_response_server.results, read_response_client.results):
                            if dv_server.variant != dv_client.variant:
                                print('ERROR: Server and Client does not read same value for node {}: {} != {}'.format(node, dv_server.variant, dv_client.variant))
                                res = False
                    except SOPC_Failure as f:
                        print(f)
                        res = False
                logger.add_test('Server and Client read same value for nodes ' + str(NODES), res)
                # End Local Read Tests #

                res = res_connect_client
                write_response_server: Response
                # Start Local Write Tests #
                print("Server Local Write Tests")
                logger.begin_section('Server Local Write Tests -')
                # Read check
                if res:
                    try :
                        write_response_server = PyS2OPC_Server.write_nodes(NODES, [DV_ns1_Int32_007,DV_ns1_UInt64_099])
                        read_response_server = PyS2OPC_Server.read_nodes(NODES)
                        read_response_client = connect.read_nodes(NODES)
                        for node, dv_server, dv_client, dv_ref in zip(NODES, read_response_server.results, read_response_client.results, [DV_ns1_Int32_007, DV_ns1_UInt64_099]):
                            if not(dv_server.variant == dv_client.variant == dv_ref.variant):
                                print('ERROR: Server and Client does not read expected value for node {}. server value : {} client value : {} expected value: {}'
                                      .format(node, dv_server.variant, dv_client.variant, dv_ref.variant))
                                res = False
                    except SOPC_Failure as f:
                        print(f)
                        res = False
                logger.add_test('Write nodes : ' + str(NODES), write_response_server.is_ok())
                logger.add_test('Server successfully writes nodes locally (same values as those found with client).', res)
                # End Local Write Tests #

                res = res_connect_client
                browse_response_server: Response
                # Start Local Browse Tests #
                print("Server Local Browse Tests")
                logger.begin_section('Server Local Browse Tests -')
                if res:
                    try:
                        browse_response_server = PyS2OPC_Server.browse_nodes(NODES)
                        browse_response_client = connect.browse_nodes(NODES)
                        for node, bwsRes_server, bwsRes_client in zip(NODES, browse_response_server.results, browse_response_client.results):
                            if bwsRes_server.status != bwsRes_client.status:
                                print('ERROR: Server and Client do not have the same browse status for node : ' + node)
                                res = False
                            if bwsRes_server.continuationPoint != bwsRes_client.continuationPoint:
                                print('ERROR: Server and Client do not have the same browse continuationPoint for node : ' + node)
                                res = False
                            if str(bwsRes_server.references) != str(bwsRes_client.references):
                                print('ERROR: Server and Client do not have the same browse references for node : ' + node)
                                res = False
                    except SOPC_Failure as f:
                        print(f)
                        res = False
                logger.add_test('Browse nodes : ' + str(NODES), browse_response_server.is_ok())
                logger.add_test('Server successfully browses nodes locally (same values as those found with client).', res)
                # End Local Browse Tests #

            # Close server #
        logger.begin_section('Close server -')
        logger.add_test('Server has finished serving.', not(PyS2OPC_Server._serving))
    logger.add_test('Server uninitialized.', not(PyS2OPC_Server._initialized_srv))
    # Server closed #

    # --- Test 2 server modes : `serve()`, `serve_forever()` --- #
    # Test server with `serve()` #
    logger.begin_section('Test server method : serve() -')
    with PyS2OPC_Server.initialize():
        addSpaceHandler = AddressSpaceHandler()
        PyS2OPC_Server.load_server_configuration_from_files(os.path.join('S2OPC_Server_Demo_Config.xml'),
                                                                    os.path.join('S2OPC_Demo_NodeSet.xml'),
                                                                    os.path.join('S2OPC_Users_Demo_Config.xml'),
                                                                    addSpaceHandler)
        logger.add_test('Server not yet serving.', not(PyS2OPC_Server._serving))
        res_serving, res_write_notif_cb = True, False
        with PyS2OPC_Server.serve():
            t0 = time.time()
            while not(PyS2OPC_Server.serving()):
                time.sleep(.05)
                if time.time() - t0 > 0.5: # TimeOut serving = 0.5s
                    print('ERROR: Server not serving on time')
                    res_serving = False
            if res_serving:
                # Client writes on 2 nodes #
                res_write_notif_cb = client_write_nodes_count_cb(NODES, [DV_ns1_Int32_007,DV_ns1_UInt64_099], addSpaceHandler)
        logger.add_test('Server serving on time.', res_serving)
        logger.add_test('Server write notification callback is called the right number of times.', res_write_notif_cb)

    # Test server with `serve_forever()` #
    logger.begin_section('Test server method : serve_forever() -')
    res_server_start, res_write_notif_cb = False, False
    addSpaceHandler = AddressSpaceHandler()
    server_thread = Thread(target=start_server_forever, args=(addSpaceHandler,))
    server_thread.start()
    if not wait_server.wait_server(wait_server.DEFAULT_URL, wait_server.TIMEOUT):
        print('ERROR: Timeout for starting server')
        # Close server_thread with server shutdown.
        PyS2OPC_Server.stop_serve()
        server_thread.join()
    else: # Server started
        res_server_start = True
        # Client writes on 2 nodes #
        res_write_notif_cb = client_write_nodes_count_cb(NODES, [DV_ns1_Int32_007,DV_ns1_UInt64_099], addSpaceHandler)
        # Close server_thread with server shutdown.
        PyS2OPC_Server.stop_serve()
        server_thread.join()

    logger.add_test('Server started.', res_server_start)
    logger.add_test('Server write notification callback is called the right number of times.', res_write_notif_cb)

    logger.finalize_report()
    sys.exit(1 if logger.has_failed_tests else 0)
