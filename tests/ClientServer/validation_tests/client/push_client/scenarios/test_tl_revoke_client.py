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
from subprocess import check_call
import sys
import signal
import os

description = '''Test: a CA issued client certificate has been revoked. This client is then disconnected (and other clients are not impacted),
                 and he cannot reconnect to the server.'''

def handler(signum, frame):
    raise Exception("Timeout.")

if __name__ == '__main__':
    
    glob_ret = 0
    signal.signal(signal.SIGALRM, handler)
    max_connection_waiting_time = 2

    # Produce log file.
    f = open("push_server_revoke_trace.log", "w")

    # 1. Two different CA issued clients connect to the server and sleep.
    cmd = ["./push_client", "client_public/client_2k_cert.der", "client_private/encrypted_client_2k_key.pem", "sleep"]
    client_A_process = None
    # Set the alarm for timeout.
    signal.alarm(max_connection_waiting_time)
    try:
        client_A_process = ConnectionAndClear.launch_client_and_wait_for_connection(cmd)
        # Desactivate alarm
        signal.alarm(0)
    except Exception as exc:
        f.write("Timeout reached for connection.\n")
    if None != client_A_process:
        f.write("Client A connected.\n")
    else:
        f.write("Fail at launching client A process or at connection time.\n")

    cmd = ["./push_client", "client_public/client_4k_cert.der", "client_private/encrypted_client_4k_key.pem", "sleep"]
    client_B_process = None
    signal.alarm(max_connection_waiting_time)
    try:
        client_B_process = ConnectionAndClear.launch_client_and_wait_for_connection(cmd)
        signal.alarm(0)
    except Exception as exc:
        f.write("Timeout reached for connection.\n")
    if None != client_B_process:
        f.write("Client B connected.\n")
    else:
        f.write("Fail at launching client B process or at connection time.\n")

    # 2. Admin revokes client B in the server trustlist, checks the disconnection of B and the non-disconnection of A.
    try:
        check_call(["./push_client", "client_public/client_2k_cert.der", "client_private/encrypted_client_2k_key.pem", "revoke"])
        test_ret = 0
    except subprocess.CalledProcessError as e:
        test_ret = e.returncode
    if 0 != test_ret:
        glob_ret = 1
        f.write("Step 2.a failed.\n")
    else:
        f.write("Step 2.a success.\n")

    try:
        client_B_process.wait(2)
        test_ret = client_B_process.returncode
    except subprocess.TimeoutExpired as e:
        client_B_process.kill()
        test_ret = 1
    if 2 != test_ret:
        glob_ret = 1 
        f.write("Step 2.b failed.\n")
    else:
        f.write("Step 2.b success.\n")
  
    try:
        client_A_process.wait(2)
        test_ret = 0
    except subprocess.TimeoutExpired as e:
        client_A_process.kill()
        test_ret = 1
    if 1 != test_ret:
        glob_ret = 1
        f.write("Step 2.c failed.\n")
    else:
        f.write("Step 2.c success.\n")

    # 3. Check the presence of B in the RejectedList of the server.
    try:
        check_call(["./push_client", "client_public/client_2k_cert.der", "client_private/encrypted_client_2k_key.pem", "getRejectedList"])
        test_ret = 0
    except subprocess.CalledProcessError as e:
        test_ret = e.returncode
    # If client command failed or the rejected file certificate does not exist
    if 0 != test_ret or False == os.path.isfile("S2OPC_Demo_PKI/rejected/28FAAEECC876023AAB919D89EE6B580E69AA269C.der"):
        glob_ret = 1
        f.write("Step 3 failed.\n")
    else:
        f.write("Step 3 success.\n")

    # 4. B cannot reconnect. A can reconnect.
    try:
        check_call(["./push_client", "client_public/client_4k_cert.der", "client_private/encrypted_client_4k_key.pem"])
        test_ret = 0
    except subprocess.CalledProcessError as e:
        test_ret = e.returncode
    if 0 == test_ret:
        glob_ret = 1
        f.write("Step 4.a failed.\n")
    else:
        f.write("Step 4.a success.\n")
    try:
        check_call(["./push_client", "client_public/client_2k_cert.der", "client_private/encrypted_client_2k_key.pem"])
        test_ret = 0
    except subprocess.CalledProcessError as e:
        test_ret = e.returncode
    if 0 != test_ret:
        glob_ret = 1
        f.write("Step 4.b failed.\n")
    else:
        f.write("Step 4.b success.\n")

    # Close log file
    f.close()

    # 5. Safely delete the updated pki and the rejected folder
    ConnectionAndClear.clear_updated_pki()
    rootdirRejected = "S2OPC_Demo_PKI/rejected"
    for files in os.listdir(rootdirRejected):
        if files.endswith(".der"):
            os.remove(os.path.join(rootdirRejected, files))
    os.rmdir(rootdirRejected)

    sys.exit(glob_ret)