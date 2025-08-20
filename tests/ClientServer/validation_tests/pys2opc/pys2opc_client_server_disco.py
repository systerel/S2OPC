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
PyS2OPC library client with server disconnection validation tests :
- Keeping the client's `connected` status up to date
- ClientConnectionFailure on client read after server diconnection
"""

import time
import os
import sys
from threading import Thread

from pys2opc import PyS2OPC_Server, PyS2OPC_Client, ClientConnectionFailure, SOPC_Failure, BaseClientConnectionHandler
import utils
import wait_server
from tap_logger import TapLogger

NODES = ['ns=1;s=Int32_007', 'ns=1;s=UInt64_099']
TEST_PASSWORD_PRIV_KEY_ENV_NAME = "TEST_PASSWORD_PRIVATE_KEY"


class PyS2OPC_Server_Test():
    @staticmethod
    def get_server_key_password():
        pwd = os.getenv(TEST_PASSWORD_PRIV_KEY_ENV_NAME)
        if pwd is None:
            print('The following environment variable is missing: ' + TEST_PASSWORD_PRIV_KEY_ENV_NAME)
        return pwd

# overload the default password method for tests
PyS2OPC_Server.get_server_key_password = PyS2OPC_Server_Test.get_server_key_password

class PrintSubs(BaseClientConnectionHandler):
    def on_datachanged(self, nodeId, dataValue):
        print('  Data changed "{}" -> {}, '.format(nodeId, dataValue.variant) + time.ctime(dataValue.timestampServer))

def start_server_forever(addSpaceHandler=None):
    with PyS2OPC_Server.initialize():
        PyS2OPC_Server.load_server_configuration_from_files(os.path.join('S2OPC_Server_Demo_Config.xml'),
                                                                    os.path.join('S2OPC_Demo_NodeSet.xml'),
                                                                    os.path.join('S2OPC_Users_Demo_Config.xml'),
                                                                    addSpaceHandler)
        PyS2OPC_Server.serve_forever()

if __name__ == '__main__':

    # --- Test client on server disconnection --- #
    logger = TapLogger('validation_pys2opc_client_server_disco.tap')

    for ConnHandlerClass in (None, PrintSubs):
        if ConnHandlerClass is None:
            logger.begin_section('Client tests on server disconnection (without connection handler) -')
        else:
            logger.begin_section('Client tests on server disconnection (with connection handler) -')

        res_connected_before_server_shutdown, res_connected_after_server_shutdown = False, True
        res_read_failure_after_server_shutdown: bool = False

        server_thread = Thread(target=start_server_forever)
        server_thread.start()
        if not wait_server.wait_server(wait_server.DEFAULT_URL, wait_server.TIMEOUT):
            print('ERROR: Timeout for starting server', flush=True)
            # Close server_thread with server shutdown.
            PyS2OPC_Server.stop_serve()
            server_thread.join()
        else: # Server started
            print("Server started", flush=True)
            with PyS2OPC_Client.initialize():
                try :
                    # - 1 : Start client connected test - #
                    # Connect client #
                    configs = PyS2OPC_Client.load_client_configuration_from_file(os.path.join('S2OPC_Client_Wrapper_Config.xml'))
                    connect = PyS2OPC_Client.connect(configs["read"], ConnHandlerClass)
                    res_connected_before_server_shutdown = connect.connected
                    # Client connected #

                    # Close server_thread with server shutdown #
                    PyS2OPC_Server.stop_serve()
                    server_thread.join()
                    # Server Closed #

                    # Waiting for a change in connection status (`connected`)
                    t0 = time.time()
                    while connect.connected and time.time() - t0 < 2: # TimeOut connected = 2s
                        time.sleep(.05)
                    res_connected_after_server_shutdown = connect.connected
                    # - 1 : End client connected test - #

                    # - 2 : Start client write test on disconnected server - #
                    try :
                        read_response_client = connect.read_nodes(NODES)
                    except ClientConnectionFailure as ccf:
                        res_read_failure_after_server_shutdown = True
                        print(ccf, flush=True)
                    except SOPC_Failure as f:
                        print("ERROR : Not failure expected")
                        print(f, flush=True)
                    # - 2 : End client write test on disconnected server - #
                except SOPC_Failure as f:
                    print(f, flush=True)

        logger.add_test('Client connection succeeded', res_connected_before_server_shutdown)
        logger.add_test('Client connected state changed after server stopped', not(res_connected_after_server_shutdown))
        logger.add_test('Client read raise a failure exception after server stopped', res_read_failure_after_server_shutdown)

    logger.finalize_report()
    sys.exit(1 if logger.has_failed_tests else 0)
