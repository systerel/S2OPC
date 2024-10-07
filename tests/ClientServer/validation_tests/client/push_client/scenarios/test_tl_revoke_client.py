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
import os

description = '''Test: a CA issued client certificate has been revoked. This client is then disconnected (and other clients are not impacted),
                 and he cannot reconnect to the server.'''

if __name__ == '__main__':
    
    # Produce log file.
    logFile = logs.create_logFile("push_server_revoke_trace.log")
    f = open(logFile, "w")
    f.write("Starting test.\n")

    # 1. Two different CA issued clients connect to the server and sleep.
    step = "1.a"
    cmd = ["./push_client", "client_public/client_2k_cert.der", "client_private/encrypted_client_2k_key.pem", "sleep"]
    client_A_process = clientProcessManager.cmdWaitForConnection(cmd, f, step)

    step = "1.b"
    cmd = ["./push_client", "client_public/client_4k_cert.der", "client_private/encrypted_client_4k_key.pem", "sleep"]
    client_B_process = clientProcessManager.cmdWaitForConnection(cmd, f, step)

    # 2. Admin revokes client B in the server trustlist, checks the disconnection of B and the non-disconnection of A.
    step = "2.a"
    cmd = ["./push_client", "client_public/client_2k_cert.der", "client_private/encrypted_client_2k_key.pem", "revoke"]
    clientProcessManager.cmdExpectSuccess(cmd, f, step)

    step = "2.b"
    needCertUpdated = False
    clientProcessManager.processExpectDisconnection(client_B_process, needCertUpdated, f, step)

    step = "2.c"
    clientProcessManager.processExpectNonDisconnection(client_A_process, f, step)
  
    # 3. Check the presence of B in the RejectedList of the server.
    step = "3"
    cmd = ["./push_client", "client_public/client_2k_cert.der", "client_private/encrypted_client_2k_key.pem", "getRejectedList"]
    clientProcessManager.cmdExpectSuccess(cmd, f, step)
    # If the rejected file certificate does not exist
    if False == os.path.isfile("S2OPC_Demo_PKI/rejected/28FAAEECC876023AAB919D89EE6B580E69AA269C.der"):
        f.write("Rejected certificate not present.\n")
        sys.exit(1)

    # 4. B cannot reconnect. A can reconnect.
    step = "4.a"
    cmd = ["./push_client", "client_public/client_4k_cert.der", "client_private/encrypted_client_4k_key.pem"]
    clientProcessManager.cmdExpectFail(cmd, f, step)
    
    step = "4.b"
    cmd = ["./push_client", "client_public/client_2k_cert.der", "client_private/encrypted_client_2k_key.pem"]
    clientProcessManager.cmdExpectSuccess(cmd, f, step)

    # 5. Safely delete the updated pki and the rejected folder
    clearServerPKI.clearUpdatedPKI()
    rootdirRejected = "S2OPC_Demo_PKI/rejected"
    for files in os.listdir(rootdirRejected):
        if files.endswith(".der"):
            os.remove(os.path.join(rootdirRejected, files))
    os.rmdir(rootdirRejected)

    # Exit success
    f.write("Test completed with success.\n")
    f.close()
    sys.exit(0)