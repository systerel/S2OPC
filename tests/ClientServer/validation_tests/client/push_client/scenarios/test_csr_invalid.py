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

from test_utils import clientProcessManager, logs
import sys

description = '''Test: Update the server certificate (with the method UpdateCertificate) with the same certificate, expected success 
                 and no impact on the client connections. Create some signing requests with invalid parameters and check for the 
                 response of the server.'''

if __name__ == '__main__':

    # Produce log file.
    logFile = logs.create_logFile("push_server_csr_invalid.log")
    f = open(logFile, "w")
    f.write("Starting test.\n")

    # 1. Connect a client and sleep.
    step = "1"
    cmd = ["./push_client", "client_public/client_2k_cert.der", "client_private/encrypted_client_2k_key.pem", "sleep"]
    client_process = clientProcessManager.cmdWaitForConnection(cmd, f, step)

    # 2. Update the server certificate with the same certificate.
    step = "2.a"
    cmd = ["./push_client", "client_public/client_2k_cert.der", "client_private/encrypted_client_2k_key.pem", 
           "updateCertificate", "server_public/server_4k_cert.der", "1", "S2OPC_Demo_PKI/trusted/certs/cacert.der"]
    clientProcessManager.cmdExpectSuccess(cmd, f, step)
    
    # this update must have no impact on the other client connection:
    clientProcessManager.processExpectNonDisconnection(client_process, f, step)

    # 3. Create 3 successive invalid csr requests
    # --> invalid groupID + valid certificateTypeID
    step = "3.a"
    cmd = ["./push_client", "client_public/client_2k_cert.der", "client_private/encrypted_client_2k_key.pem", 
           "csr", "groupIdInvalid", "certificateTypeIdValid", "noNewKey", "noNonce"]
    clientProcessManager.cmdExpectFail(cmd, f, step)

    # --> empty groupID + invalid certificateTypeID
    step = "3.b"
    cmd = ["./push_client", "client_public/client_2k_cert.der", "client_private/encrypted_client_2k_key.pem", 
           "csr", "groupIdEmpty", "certificateTypeIdInvalid", "noNewKey", "noNonce"]
    clientProcessManager.cmdExpectFail(cmd, f, step)
    
    # Exit success
    f.write("Test completed with success.\n")
    f.close()
    sys.exit(0)