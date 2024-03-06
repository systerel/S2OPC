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

description = '''Test: CA untrusted is present in server PKI. A client (not known) issued by this untrusted CA cannot connect.
                 Add this client certificate to trusted TL of the server, and it is able to connect. 
                 Remove CA untrusted, the client is disconnected and cannot reconnect. '''

def handler(signum, frame):
    raise Exception("Timeout.")

if __name__ == '__main__':

    glob_ret = 0
    signal.signal(signal.SIGALRM, handler)
    max_connection_waiting_time = 2

    # Produce log file.
    f = open("push_server_untrusted_trace.log", "w")

    # 0. Put the PKI into the initial state for the test: "trusted_client_cert.pem" not present in trusted.
    try:
        subprocess.check_call(["./push_client", "client_public/client_2k_cert.der", "client_private/encrypted_client_2k_key.pem", "remove", "34EC8A8CA7735401B6F93BB3C93B0D1D311802E6", "trusted"])
        test_ret = 0
    except subprocess.CalledProcessError as e:
        test_ret = e.returncode
    if 0 != test_ret:
        glob_ret = 1
        f.write("Step 0 failed.\n")
    else:
        f.write("Step 0 success.\n")

    # 1. Connection impossible with CA untrusted issued certificate. Add this certificate to server TL.
    try:
        subprocess.check_call(["./push_client", "client_public/trusted_client_cert.pem", "client_private/encrypted_trusted_client_key.pem"])
        test_ret = 0
    except subprocess.CalledProcessError as e:
        test_ret = e.returncode
    if 0 == test_ret:
        glob_ret = 1
        f.write("Step 1.a failed.\n")
    else:
        f.write("Step 1.a success.\n")

    try:
        subprocess.check_call(["./push_client", "client_public/client_2k_cert.der", "client_private/encrypted_client_2k_key.pem", "add", "client_public/trusted_client_cert.pem"])
        test_ret = 0
    except subprocess.CalledProcessError as e:
        test_ret = e.returncode
    if 0 != test_ret:
        glob_ret = 1
        f.write("Step 1.b failed.\n")
    else:
        f.write("Step 1.b success.\n")

    # 2. Connection possible now since it is trusted in the server TL.
    cmd = ["./push_client", "client_public/trusted_client_cert.pem", "client_private/encrypted_trusted_client_key.pem", "sleep"]
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

    # 3. Admin removes untrusted_cacert, check the disconnection of the client.
    try:
        subprocess.check_call(["./push_client", "client_public/client_2k_cert.der", "client_private/encrypted_client_2k_key.pem", "remove", "50A96600D8C3ADA0A4EC4737A5163D067122BC44", "untrusted"])
        test_ret = 0
    except subprocess.CalledProcessError as e:
        test_ret = e.returncode

    client_process.wait(2)
    ret = client_process.returncode
    if 2 != ret or 0 != test_ret:
        glob_ret = 1
        f.write("Step 3 failed.\n")
    else:
        f.write("Step 3 success.\n")

    # 4. He cannot reconnect.
    try:
        subprocess.check_call(["./push_client", "client_public/trusted_client_cert.pem", "client_private/encrypted_trusted_client_key.pem"])
        test_ret = 0
    except subprocess.CalledProcessError as e:
        test_ret = e.returncode
    if 0 == test_ret:
        glob_ret = 1
        f.write("Step 4 failed.\n")
    else:
        f.write("Step 4 success.\n")

    # 5. Add the client certificate, must fail because its issuer is not in the PKI (fail at chain verification)
    try:
        subprocess.check_call(["./push_client", "client_public/client_2k_cert.der", "client_private/encrypted_client_2k_key.pem", "add", "client_public/trusted_client_cert.pem"])
        test_ret = 0
    except subprocess.CalledProcessError as e:
        test_ret = e.returncode
    if 0 == test_ret:
        glob_ret = 1
        f.write("Step 5 failed.\n")
    else:
        f.write("Step 5 success.\n")

    # 6. Re-add the untrusted CA then add its issued certificate.
    try:
        subprocess.check_call(["./push_client", "client_public/client_2k_cert.der", "client_private/encrypted_client_2k_key.pem", 
                               "write", "S2OPC_Demo_PKI/issuers/certs/untrusted_cacert.pem", "S2OPC_Demo_PKI/issuers/crl/untrusted_cacrl.pem", "untrusted", 
                               "add", "client_public/trusted_client_cert.pem"])
        test_ret = 0
    except subprocess.CalledProcessError as e:
        test_ret = e.returncode
    if 0 != test_ret:
        glob_ret = 1
        f.write("Step 6 failed.\n")
    else:
        f.write("Step 6 success.\n")
    
    # 7. He can eventually reconnect
    cmd = ["./push_client", "client_public/trusted_client_cert.pem", "client_private/encrypted_trusted_client_key.pem", "sleep"]
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

    # 8. Admin removes the client certificate (not its issuer this time), check the disconnection.
    try:
        subprocess.check_call(["./push_client", "client_public/client_2k_cert.der", "client_private/encrypted_client_2k_key.pem", "remove", "34EC8A8CA7735401B6F93BB3C93B0D1D311802E6", "trusted"])
        test_ret = 0
    except subprocess.CalledProcessError as e:
        test_ret = e.returncode

    client_process.wait(2)
    ret = client_process.returncode
    if 2 != ret or 0 != test_ret:
        glob_ret = 1
        f.write("Step 8 failed.\n")
    else:
        f.write("Step 8 success.\n")

    # 9. He cannot reconnect.
    try:
        subprocess.check_call(["./push_client", "client_public/trusted_client_cert.pem", "client_private/encrypted_trusted_client_key.pem"])
        test_ret = 0
    except subprocess.CalledProcessError as e:
        test_ret = e.returncode
    if 0 == test_ret:
        glob_ret = 1
        f.write("Step 9 failed.\n")
    else:
        f.write("Step 9 success.\n")
    
    # Close log file
    f.close()
    
    # 10. Delete the updatedTrustList of the server
    ConnectionAndClear.clear_updated_pki()

    sys.exit(glob_ret)