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

description = '''Test: CA untrusted is present in server PKI. A client (not known) issued by this untrusted CA cannot connect.
                 Add this client certificate to trusted TL of the server, and check that it is able to connect. 
                 Remove CA untrusted, the client is disconnected and cannot reconnect. '''

if __name__ == '__main__':

    # Produce log file.
    logFile = logs.create_logFile("push_server_untrusted_trace.log")
    f = open(logFile, "w")
    f.write("Starting test.\n")

    # 0. Put the PKI into the initial state for the test: "trusted_client_cert.der" not present in trusted.
    step = "0"
    cmd = ["./push_client", "client_public/client_2k_cert.der", "client_private/encrypted_client_2k_key.pem", 
           "remove", "3A57B1EF80F4316AD03CE803E36C4E1A4B7C86D8", "trusted"]
    clientProcessManager.cmdExpectSuccess(cmd, f, step)

    # 1. Connection impossible with CA untrusted issued certificate. Add this certificate to server TL.
    step = "1.a"
    cmd = ["./push_client", "client_public/trusted_client_cert.pem", "client_private/encrypted_trusted_client_key.pem"]
    clientProcessManager.cmdExpectFail(cmd, f, step)

    step = "1.b"
    cmd = ["./push_client", "client_public/client_2k_cert.der", "client_private/encrypted_client_2k_key.pem", "add", 
           "client_public/trusted_client_cert.pem"]
    clientProcessManager.cmdExpectSuccess(cmd, f, step)

    # 2. Connection possible now since it is trusted in the server TL.
    step = "2"
    cmd = ["./push_client", "client_public/trusted_client_cert.pem", "client_private/encrypted_trusted_client_key.pem", "sleep"]
    client_process = clientProcessManager.cmdWaitForConnection(cmd, f, step)

    # 3. Admin removes untrusted_cacert.der, check the disconnection of the client.
    step = "3.a"
    cmd = ["./push_client", "client_public/client_2k_cert.der", "client_private/encrypted_client_2k_key.pem", 
           "remove", "7DF45F3C8B546F15D8822FD036E5EDD5DB50A072", "untrusted"]
    clientProcessManager.cmdExpectSuccess(cmd, f, step)

    step = "3.b"
    needCertUpdated = False
    clientProcessManager.processExpectDisconnection(client_process, needCertUpdated, f, step)

    # 4. He cannot reconnect.
    step = "4"
    cmd = ["./push_client", "client_public/trusted_client_cert.pem", "client_private/encrypted_trusted_client_key.pem"]
    clientProcessManager.cmdExpectFail(cmd, f, step)

    # 5. Add the client certificate, must fail because its issuer is not in the PKI (fail at chain verification)
    step = "5"
    cmd = ["./push_client", "client_public/client_2k_cert.der", "client_private/encrypted_client_2k_key.pem", "add", "client_public/trusted_client_cert.pem"]
    clientProcessManager.cmdExpectFail(cmd, f, step)

    # 6. Re-add the untrusted CA then add its issued certificate.
    step = "6"
    cmd = ["./push_client", "client_public/client_2k_cert.der", "client_private/encrypted_client_2k_key.pem", 
           "write", "S2OPC_Demo_PKI/issuers/certs/untrusted_cacert.pem", "S2OPC_Demo_PKI/issuers/crl/untrusted_cacrl.pem", 
           "untrusted", "add", "client_public/trusted_client_cert.pem"]
    clientProcessManager.cmdExpectSuccess(cmd, f, step)
    
    # 7. He can eventually reconnect
    step = "7"
    cmd = ["./push_client", "client_public/trusted_client_cert.pem", "client_private/encrypted_trusted_client_key.pem", "sleep"]
    client_process = clientProcessManager.cmdWaitForConnection(cmd, f, step)

    # 8. Admin removes the client certificate (not its issuer this time), check the disconnection.
    step = "8.a"
    cmd = ["./push_client", "client_public/client_2k_cert.der", "client_private/encrypted_client_2k_key.pem", 
           "remove", "3A57B1EF80F4316AD03CE803E36C4E1A4B7C86D8", "trusted"]
    clientProcessManager.cmdExpectSuccess(cmd, f, step)

    step = "8.b"
    needCertUpdated = False
    clientProcessManager.processExpectDisconnection(client_process, needCertUpdated, f, step)

    # 9. He cannot reconnect.
    step = "9"
    cmd = ["./push_client", "client_public/trusted_client_cert.pem", "client_private/encrypted_trusted_client_key.pem"]
    clientProcessManager.cmdExpectFail(cmd, f, step)
    
    # 10. Delete the updatedTrustList of the server
    clearServerPKI.clearUpdatedPKI()

    # Exit success
    f.write("Test completed with success.\n")
    f.close()
    sys.exit(0)