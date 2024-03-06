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

description = '''Test: Connection impossible with a self-signed certificate. Another client adds this certificate
                 to the server TL. Eventually the client with self-signed certificate can connect to the server.'''

def handler(signum, frame):
    raise Exception("Timeout.")

if __name__ == '__main__':

    glob_ret = 0
    signal.signal(signal.SIGALRM, handler)
    max_connection_waiting_time = 2

    # Produce log file.
    f = open("push_server_self_signed_trace.log", "w")

    # 0. Get to the server PKI initial state: self_signed certificate not present in trusted.
    try:
        subprocess.check_call(["./push_client", "client_public/client_2k_cert.der", "client_private/encrypted_client_2k_key.pem", "remove", "2D0A4B44350A3822496E3545D8389AFFEF90D202", "trusted"])
        test_ret = 0
    except subprocess.CalledProcessError as e:
        test_ret = e.returncode
    if 0 != test_ret:
        glob_ret = 1
        f.write("Step 0 failed.\n")
    else:
        f.write("Step 0 success.\n")

    # 1. Self-signed tries to connect. Must fail.
    try:
        subprocess.check_call(["./push_client", "S2OPC_Demo_PKI/trusted/certs/ca_selfsigned_pathLen0.der", "push_data/ca_selfsigned_pathLen0key.pem"])
        test_ret = 0
    except subprocess.CalledProcessError as e:
        test_ret = e.returncode
    if 0 == test_ret:
        glob_ret = 1
        f.write("Step 1 failed.\n")
    else:
        f.write("Step 1 success.\n")

    # 2. CA issued client adds the self-signed to server trusted, and remains connected to the server (sleeps)
    cmd = ["./push_client", "client_public/client_2k_cert.der", "client_private/encrypted_client_2k_key.pem",
            "add", "S2OPC_Demo_PKI/trusted/certs/ca_selfsigned_pathLen0.der", "sleep"]
    CA_issued_client_process = None
    signal.alarm(max_connection_waiting_time)
    try:
        CA_issued_client_process = ConnectionAndClear.launch_client_and_wait_for_connection(cmd)
        # Desactivate alarm
        signal.alarm(0)
    except Exception as exc:
        f.write("Timeout reached for connection.\n")
    if None != CA_issued_client_process:
        f.write("CA issued Client connected.\n")
    else:
        f.write("Fail at launching CA issued client process or at connection time.\n")

    # 3. Self-signed re-tries to connect and eventually connects, he sleeps.
    cmd = ["./push_client", "S2OPC_Demo_PKI/trusted/certs/ca_selfsigned_pathLen0.der", "push_data/ca_selfsigned_pathLen0key.pem", "sleep"]
    CA_self_signed_client_process = None
    signal.alarm(max_connection_waiting_time)
    try:
        CA_self_signed_client_process = ConnectionAndClear.launch_client_and_wait_for_connection(cmd)
        # Desactivate alarm
        signal.alarm(0)
    except Exception as exc:
        f.write("Timeout reached for connection.\n")
    if None != CA_self_signed_client_process:
        f.write("Client self-signed connected.\n")
    else:
        f.write("Fail at launching client self-signed process or at connection time.\n")

    # 4. Admin deletes CA root. Checks CA issued client has been disconnected, and self-signed not impacted.
    try:
        subprocess.check_call(["./push_client", "S2OPC_Demo_PKI/trusted/certs/ca_selfsigned_pathLen0.der", "push_data/ca_selfsigned_pathLen0key.pem", "remove", "8B3615C23983024A9D1E42C404481CB640B5A793", "trusted"])
        test_ret = 0
    except subprocess.CalledProcessError as e:
        test_ret = e.returncode
    if 0 != test_ret:
        glob_ret = 1
        f.write("Step 4.a failed.\n")
    else:
        f.write("Step 4.a success.\n")

    try:
        CA_issued_client_process.wait(2)
        test_ret = CA_issued_client_process.returncode
    except subprocess.TimeoutExpired as e:
        CA_issued_client_process.kill()
        test_ret = 1
    # Expect disconnection here
    if 2 != test_ret:
        glob_ret = 1 
        f.write("Step 4.b failed.\n")
    else:
        f.write("Step 4.b success.\n")  

    try:
        CA_self_signed_client_process.wait(2)
        test_ret = 0
    except subprocess.TimeoutExpired as e:
        CA_self_signed_client_process.kill()
        test_ret = 1
    # Expect TimeoutExpired here (ie no disconnection)
    if 1 != test_ret:
        glob_ret = 1
        f.write("Step 4.c failed.\n")
    else:
        f.write("Step 4.c success.\n")  

    # 5. CA issued client tries to reconnect. Must fail.
    try:
        subprocess.check_call(["./push_client", "client_public/client_2k_cert.der", "client_private/encrypted_client_2k_key.pem"])
        test_ret = 0
    except subprocess.CalledProcessError as e:
        test_ret = e.returncode
    if 0 == test_ret:
        glob_ret = 1
        f.write("Step 5 failed.\n")
    else:
        f.write("Step 5 success.\n")  

    # Close log file    
    f.close()
        
    # 6. Safely delete the updatedTrustList of the server
    ConnectionAndClear.clear_updated_pki()

    sys.exit(glob_ret)