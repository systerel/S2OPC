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

from test_utils import clientProcessManager, clearServerPKI, logs
import sys

description = '''Test: Connection impossible with a self-signed certificate. Another client adds this certificate
                 to the server TL. Eventually the client with self-signed certificate can connect to the server.'''

if __name__ == '__main__':

    # Produce log file.
    logFile = logs.create_logFile("push_server_self_signed_trace.log")
    f = open(logFile, "w")
    f.write("Starting test.\n")

    # 0. Get to the server PKI initial state: self_signed certificate not present in trusted.
    step = "0"
    cmd = ["./push_client", "client_public/client_2k_cert.der", "client_private/encrypted_client_2k_key.pem",
           "remove", "7925B4747C337AE9A1D382C2007B005E15B7A107", "trusted"]
    clientProcessManager.cmdExpectSuccess(cmd, f, step)

    # 1. Self-signed tries to connect. Must fail.
    step = "1"
    cmd = ["./push_client", "S2OPC_Demo_PKI/trusted/certs/ca_selfsigned_pathLen0.der", "push_data/ca_selfsigned_pathLen0key.pem"]
    clientProcessManager.cmdExpectFail(cmd, f, step)

    # 2. CA issued client adds the self-signed to server trusted, and remains connected to the server (sleeps)
    step = "2"
    cmd = ["./push_client", "client_public/client_2k_cert.der", "client_private/encrypted_client_2k_key.pem",
            "add", "S2OPC_Demo_PKI/trusted/certs/ca_selfsigned_pathLen0.der", "sleep"]
    CA_issued_client_process = clientProcessManager.cmdWaitForConnection(cmd, f, step)

    # 3. Self-signed re-tries to connect and eventually connects, he sleeps.
    step = "3"
    cmd = ["./push_client", "S2OPC_Demo_PKI/trusted/certs/ca_selfsigned_pathLen0.der", "push_data/ca_selfsigned_pathLen0key.pem", "sleep"]
    CA_self_signed_client_process = clientProcessManager.cmdWaitForConnection(cmd, f, step)

    # 4. Admin deletes CA root. Checks CA issued client has been disconnected, and self-signed not impacted.
    step = "4.a"
    cmd = ["./push_client", "S2OPC_Demo_PKI/trusted/certs/ca_selfsigned_pathLen0.der",
           "push_data/ca_selfsigned_pathLen0key.pem", "remove", "8B3615C23983024A9D1E42C404481CB640B5A793", "trusted"]
    clientProcessManager.cmdExpectSuccess(cmd, f, step)

    step = "4.b"
    needCertUpdated = False
    clientProcessManager.processExpectDisconnection(CA_issued_client_process, needCertUpdated, f, step)

    step = "4.c"
    clientProcessManager.processExpectNonDisconnection(CA_self_signed_client_process, f, step)

    # 5. CA issued client tries to reconnect. Must fail.
    step = "5"
    cmd = ["./push_client", "client_public/client_2k_cert.der", "client_private/encrypted_client_2k_key.pem"]
    clientProcessManager.cmdExpectFail(cmd, f, step)

    # 6. Safely delete the updatedTrustList of the server
    clearServerPKI.clearUpdatedPKI()

    # Exit success
    f.write("Test completed with success.\n")
    f.close()
    sys.exit(0)
