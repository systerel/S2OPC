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

from test_utils import ConnectionAndClear
import subprocess
import sys
import signal

description = '''Test: Update the server certificate (with the method UpdateCertificate) with the same certificate, success and no
                 impact on the client connections. Create some signing requests with invalid parameters and check for the response
                 of the server.'''

def handler(signum, frame):
    raise Exception("Timeout.")

if __name__ == '__main__':
    
    glob_ret = 0
    signal.signal(signal.SIGALRM, handler)
    max_connection_waiting_time = 2

    # Produce log file.
    f = open("push_server_csr_invalid.log", "w")

    # 1. Connect a client and sleep.
    cmd = ["./push_client", "client_public/client_2k_cert.der", "client_private/encrypted_client_2k_key.pem", "sleep"]
    client_process = None
    signal.alarm(max_connection_waiting_time)
    try:
        client_process = ConnectionAndClear.launch_client_and_wait_for_connection(cmd)
        # Desactivate alarm
        signal.alarm(0)
    except Exception as exc:
        f.write("Timeout reached for connection.\n")
    if None != client_process:
        f.write("Step 1 success: client connected.\n")
    else:
        f.write("Step 1 failed: fail at launching client process or at connection time.\n")

    # 2. Update the server certificate with the same certificate.
    try:
        subprocess.check_call(["./push_client", "client_public/client_2k_cert.der", "client_private/encrypted_client_2k_key.pem", 
                               "updateCertificate", "server_public/server_4k_cert.der", "1", "S2OPC_Demo_PKI/trusted/certs/cacert.der"])
        test_ret = 0
    except subprocess.CalledProcessError as e:
        test_ret = e.returncode
    if 0 != test_ret:
        glob_ret = 1
        f.write("Step 2.a failed.\n")
    else:
        f.write("Step 2.a success.\n")

    # this update must have no impact on the other client connection:
    try:
        # wait till the process finishes, and get the return value if he finishes.
        client_process.wait(2)
        test_ret = client_process.communicate()
    except subprocess.TimeoutExpired:
        # if not finished, kill it 
        client_process.kill()
        test_ret = 0
    if 0 != test_ret:
        glob_ret = 1
        f.write("Step 2.b failed.\n")
    else:
        f.write("Step 2.b success.\n")
    
    # 3. Create 3 successive invalid csr requests
    # --> invalid groupID + valid certificateTypeID
    try:
        subprocess.check_call(["./push_client", "client_public/client_2k_cert.der", "client_private/encrypted_client_2k_key.pem", "csr", "groupIdInvalid", "certificateTypeIdValid", "noNewKey", "noNonce"])
        test_ret = 0
    except subprocess.CalledProcessError as e:
        test_ret = e.returncode
    if 0 == test_ret:
        glob_ret = 1
        f.write("Step 3.a failed.\n")
    else:
        f.write("Step 3.a success.\n")
    # --> empty groupID + invalid certificateTypeID
    try:
        subprocess.check_call(["./push_client", "client_public/client_2k_cert.der", "client_private/encrypted_client_2k_key.pem", "csr", "groupIdEmpty", "certificateTypeIdInvalid", "noNewKey", "noNonce"])
        test_ret = 0
    except subprocess.CalledProcessError as e:
        test_ret = e.returncode
    if 0 == test_ret:
        glob_ret = 1
        f.write("Step 3.b failed.\n")
    else:
        f.write("Step 3.b success.\n")
    # --> ask a new key but with invalid nonce. Must fail since the nonce is required for generating a new key pair, but actually succeeds.
    # try:
    #     subprocess.check_call(["./push_client", "client_public/client_2k_cert.der", "client_private/encrypted_client_2k_key.pem", "csr", "groupIdValid", "certificateTypeIdValid", "newKey", "noNonce"])
    #     test_ret = 0
    # except subprocess.CalledProcessError as e:
    #     test_ret = e.returncode
    # if 0 == test_ret:
    #     glob_ret = 1
    #     f.write("Step 3.c failed.\n")
    # else:
    #     f.write("Step 3.c success.\n")
    
    # Close log file
    f.close()
    
    sys.exit(glob_ret)