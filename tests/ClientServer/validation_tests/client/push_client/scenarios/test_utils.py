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

import os
import re
import sys
import subprocess
import signal
from subprocess import Popen, PIPE

def handler(signum, frame):
        raise Exception("Timeout.")
    
signal.signal(signal.SIGALRM, handler)
max_connection_waiting_time = 2

# Return codes of the C program
RET_EXIT_FAILURE = 1
RET_DISCONNECTED = 2
RET_UPDATE_CERTIFICATE_AND_DISCONNECTED = 3
class clientProcessManager:

    # Returns the Popen object (the process) if the client has successfully 
    # connected to the server (ie the keyword "Connected." has been read in stdout).
    def cmdWaitForConnection(cmd, f, step):
        f.write("Process " + ' '.join(cmd) + " started and pending.\n")
        # Set the alarm for timeout.
        signal.alarm(max_connection_waiting_time)
        try:
            proc = Popen(["stdbuf", "-oL"] + cmd, stdout=PIPE)
            s = "Connected."
            lines_iterator = iter(proc.stdout.readline, '')
            for line in lines_iterator:
                # If the process has terminated, break
                if proc.poll() != None : break
                if re.match(s, line.decode('utf-8')):
                    # Deactivate alarm and return process
                    signal.alarm(0)
                    return proc
            f.write("Errror: step " + step + ": process terminated failing to connect to the server.\n")
            sys.exit(1)
        except Exception:
            f.write("Error: step " + step + ": timeout triggered when trying to connect to the server.\n")
            sys.exit(1)
    
    def cmdExpectSuccess(cmd, f, step):
        f.write("Command " + ' '.join(cmd) + " started. Expecting return sucess.\n")
        try:
            testError = subprocess.check_call(cmd)
        except subprocess.CalledProcessError as e:
            testError = e.returncode

        # We expect success. Log + exit if the test failed.
        if 0 != testError:
            f.write("Error: step " + step + " failed. Return code is " + str(testError) + ", was expecting 0.\n")
            sys.exit(1)
        
    def cmdExpectFail(cmd, f, step):
        f.write("Command " + ' '.join(cmd) + " started. Expecting return fail.\n")
        try:
            testError = subprocess.check_call(cmd)
        except subprocess.CalledProcessError as e:
            testError = e.returncode
        
        # We expect fail. Log + exit if success.
        if 0 == testError:
            f.write("Error: step " + step + " failed. Return code is 0, was expecting " + str(RET_EXIT_FAILURE) + ".\n")
            sys.exit(1)

    def processExpectDisconnection(proc, needCertUpdated, f, step):
        f.write("Process expecting disconnection.\n")
        try:
            proc.wait(2)
            testError = proc.returncode
        except subprocess.TimeoutExpired:
            proc.kill()
            testError = 1
        
        if needCertUpdated:
            if RET_UPDATE_CERTIFICATE_AND_DISCONNECTED != testError:
                f.write("Error: step " + step + " failed. Return code is " + str(testError) + ", was expecting " + str(RET_UPDATE_CERTIFICATE_AND_DISCONNECTED) + ".\n")
                sys.exit(1)
        else:
            if RET_DISCONNECTED != testError:
                f.write("Error: step " + step + " failed. Return code is " + str(testError) + ", was expecting " + str(RET_DISCONNECTED) + ".\n")
                sys.exit(1)

    def processExpectNonDisconnection(proc, f, step):
        f.write("Process expecting non-disconnection.\n")
        try:
            proc.wait(2)
            testError = proc.returncode
        except subprocess.TimeoutExpired:
            proc.kill()
            testError = 0
        
        # We expect non-disconnection, ie Timeout, ie testError = 0.
        # Log + exit if testError != 0.
        if 0 != testError:
            f.write("Error: step " + step + " failed. Return code is " + str(testError) + ", was expecting the process to not end.\n")
            sys.exit(1)

class clearServerPKI:

    # Delete the updatedTrustList
    def clearUpdatedPKI():
        rootdirUpdated = "./S2OPC_Demo_PKI/updatedTrustList"
        for issuer_or_trusted in ["issuers", "trusted"]:
            for cert_or_crl in ["certs", "crl"]:
                for files in os.listdir(os.path.join(rootdirUpdated, issuer_or_trusted, cert_or_crl)):
                    if files.endswith(".der"):
                        os.remove(os.path.join(rootdirUpdated, issuer_or_trusted, cert_or_crl, files))
                os.rmdir(os.path.join(rootdirUpdated, issuer_or_trusted, cert_or_crl))
            os.rmdir(os.path.join(rootdirUpdated, issuer_or_trusted))
        os.rmdir(rootdirUpdated)

class logs:

    def create_logFile(file_name):
        base_log_dir = "tests_demo_push_server_logs/"
        if not os.path.exists(base_log_dir):
            os.mkdir(base_log_dir)
        return base_log_dir + file_name
