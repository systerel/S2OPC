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

description = '''Test: A client issued from CA_int cannot connect. After the CA_int has been added to server trusted TL, he
                 is able to connect. Try to remove the CA root (issuer of CA_int) with the method write.
                 It must fail since CA_int (which is in the server TL) is issued by this CA root.
                 Eventually remove CA_int from server TL and check for the client disconnection.'''

if __name__ == '__main__':

    # Produce log file.
    logFile = logs.create_logFile("push_server_trusted_int_trace.log")
    f = open(logFile, "w")
    f.write("Starting test.\n")

    # 0. Put PKI into the initial state of test: "int_cli_cacert.der" and its CRL not present in trusted.
    step = "0"
    cmd = ["./push_client", "client_public/client_2k_cert.der", "client_private/encrypted_client_2k_key.pem",
           "remove", "F9BC1EC02D083879F11638286C9894288DA2D26D", "trusted"]
    clientProcessManager.cmdExpectSuccess(cmd, f, step)

    # 1. Connection impossible with client issued from CA intermediate.
    stp = "1"
    cmd = ["./push_client", "client_public/int_client_cert.pem", "client_private/encrypted_int_client_key.pem"]
    clientProcessManager.cmdExpectFail(cmd, f, step)

    # 2. Write the CA intermediate and its CRL to trusted in the server PKI. Connection with issued client possible now.
    step = "2.a"
    cmd = ["./push_client", "client_public/client_2k_cert.der", "client_private/encrypted_client_2k_key.pem",
           "write", "S2OPC_Demo_PKI/trusted/certs/int_cli_cacert.pem", "S2OPC_Demo_PKI/trusted/crl/int_cli_cacrl.pem", "trusted"]
    clientProcessManager.cmdExpectSuccess(cmd, f, step)

    step = "2.b"
    cmd = ["./push_client", "client_public/int_client_cert.pem", "client_private/encrypted_int_client_key.pem"]
    clientProcessManager.cmdExpectSuccess(cmd, f, step)

    # 3. Write remove CA root (issuer of the CA int), fail at CloseAndUpdate.
    step = "3"
    cmd = ["./push_client", "client_public/client_2k_cert.der", "client_private/encrypted_client_2k_key.pem",
           "write_remove", "8B3615C23983024A9D1E42C404481CB640B5A793", "trusted"]
    clientProcessManager.cmdExpectFail(cmd, f, step)

    # 4. Client connects, remove CA int, check for the disconnection of the client.
    step = "4.a"
    cmd = ["./push_client", "client_public/int_client_cert.pem", "client_private/encrypted_int_client_key.pem", "sleep"]
    client_process = clientProcessManager.cmdWaitForConnection(cmd, f, step)

    step = "4.b"
    cmd = ["./push_client", "client_public/client_2k_cert.der", "client_private/encrypted_client_2k_key.pem",
           "remove", "F9BC1EC02D083879F11638286C9894288DA2D26D", "trusted"]
    clientProcessManager.cmdExpectSuccess(cmd, f, step)

    step = "4.c"
    needCertUpdated = False
    clientProcessManager.processExpectDisconnection(client_process, needCertUpdated, f, step)

    # 6. Delete the updatedTrustList of the server
    clearServerPKI.clearUpdatedPKI()

    # Exit success
    f.write("Test completed with success.\n")
    f.close()
    sys.exit(0)
