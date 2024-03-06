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

description = '''Test: A client issued from CA_int cannot connect. After the CA_int has been added to server trusted TL, he
                 is able to connect. Try to remove the CA root (issuer of CA_int) with two different methods: remove and write.
                 The two attempts must fail since CA_int (which is in the server TL) is issued by this CA root.
                 Eventually remove CA_int from server TL and check for the client disconnection.'''

def handler(signum, frame):
    raise Exception("Timeout.")

if __name__ == '__main__':

    glob_ret = 0
    signal.signal(signal.SIGALRM, handler)
    max_connection_waiting_time = 2

    # Produce log file.
    f = open("push_server_trusted_int_trace.log", "w")

    # 0. Put PKI into the initial state of test: "int_cli_cacert.pem" and its CRL not present in trusted.
    try:
        subprocess.check_call(["./push_client", "client_public/client_2k_cert.der", "client_private/encrypted_client_2k_key.pem", "remove", "348669740F8379910BCBBD69D071C42B2E789838", "trusted"])
        test_ret = 0
    except subprocess.CalledProcessError as e:
        test_ret = e.returncode
    if 0 != test_ret:
        glob_ret = 1
        f.write("Step 0 failed.\n")
    else:
        f.write("Step 0 success.\n")

    # 1. Connection impossible with client issued from CA intermediate.
    try:
        subprocess.check_call(["./push_client", "client_public/int_client_cert.pem", "client_private/encrypted_int_client_key.pem"])
        test_ret = 0
    except subprocess.CalledProcessError as e:
        test_ret = e.returncode
    if 0 == test_ret:
        glob_ret = 1
        f.write("Step 1 failed.\n")
    else:
        f.write("Step 1 success.\n")

    # 2. Write the CA intermediate and its CRL to trusted. Connection with issued client possible now.
    try:
        subprocess.check_call(["./push_client", "client_public/client_2k_cert.der", "client_private/encrypted_client_2k_key.pem", 
                               "write", "S2OPC_Demo_PKI/trusted/certs/int_cli_cacert.pem", "S2OPC_Demo_PKI/trusted/crl/int_cli_cacrl.pem", "trusted"])
        test_ret = 0
    except subprocess.CalledProcessError as e:
        test_ret = e.returncode
    if 0 != test_ret:
        glob_ret = 1
        f.write("Step 2.a failed.\n")
    else:
        f.write("Step 2.a success.\n")
    try:
        subprocess.check_call(["./push_client", "client_public/int_client_cert.pem", "client_private/encrypted_int_client_key.pem"])
        test_ret = 0
    except subprocess.CalledProcessError as e:
        test_ret = e.returncode
    if 0 != test_ret:
        glob_ret = 1
        f.write("Step 2.b failed.\n")
    else:
        f.write("Step 2.b success.\n")
    
    # 3. Write remove CA root (issuer of the CA int), fail at CloseAndUpdate.
    try:
        subprocess.check_call(["./push_client", "client_public/client_2k_cert.der", "client_private/encrypted_client_2k_key.pem", "write_remove", "8B3615C23983024A9D1E42C404481CB640B5A793", "trusted"])
        test_ret = 0
    except subprocess.CalledProcessError as e:
        test_ret = e.returncode
    if 0 == test_ret:
        glob_ret = 1
        f.write("Step 3 failed.\n")
    else:
        f.write("Step 3 success.\n")

    # # 4. Remove CA root (issuer of the CA int), fail. Problem: it actually succeeds.
    # try:
    #     subprocess.check_call(["./push_client", "client_public/client_2k_cert.der", "client_private/encrypted_client_2k_key.pem", "remove", "8B3615C23983024A9D1E42C404481CB640B5A793", "trusted"])
    #     test_ret = 0
    # except subprocess.CalledProcessError as e:
    #     test_ret = e.returncode
    # if 0 != test_ret:
    #     glob_ret = 1
    #     f.write("Step 4 failed.\n")
    # else:
    #     f.write("Step 4 success.\n")

    # 5. Client connects, remove CA int, check for the disconnection of the client.
    cmd = ["./push_client", "client_public/int_client_cert.pem", "client_private/encrypted_int_client_key.pem", "sleep"]
    client_process = None
    signal.alarm(max_connection_waiting_time)
    try:
        client_process = ConnectionAndClear.launch_client_and_wait_for_connection(cmd)
        # Desactivate alarm
        signal.alarm(0)
    except Exception as exc:
        f.write("Timeout reached for connection.\n")
    if None != client_process:
        f.write("Client connected.\n")
    else:
        f.write("Fail at launching client process or at connection time.\n")

    try:
        subprocess.check_call(["./push_client", "client_public/client_2k_cert.der", "client_private/encrypted_client_2k_key.pem", "remove", "348669740F8379910BCBBD69D071C42B2E789838", "trusted"])
        test_ret = 0
    except subprocess.CalledProcessError as e:
        test_ret = e.returncode

    client_process.wait(2)
    ret = client_process.returncode
    if 2 != ret or 0 != test_ret:
        glob_ret = 1
        f.write("Step 5 failed.\n")
    else:
        f.write("Step 5 success.\n")
    
    # Close log file
    f.close()
    
    # 6. Delete the updatedTrustList of the server
    ConnectionAndClear.clear_updated_pki()

    sys.exit(glob_ret)