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
from subprocess import Popen, PIPE

class ConnectionAndClear:

    # Returns:
    # - None if "cmd" has terminated
    # - the Popen object (the process) if the client has successfully connected to the server 
    # (ie the keyword "Connected." has been read in stdout).
    # Warning: this function needs a timeout to handle the case "cmd" did not terminate 
    # and the keyword has not been received.
    def launch_client_and_wait_for_connection(cmd):
        proc = Popen(["stdbuf", "-oL"] + cmd, stdout=PIPE)
        s = "Connected."
        lines_iterator = iter(proc.stdout.readline, '')
        for line in lines_iterator:
            # If the process has terminated, break
            if proc.poll() != None : break
            if re.match(s, line.decode('utf-8')):
                return proc
        return None
    
    # Delete the updatedTrustList
    def clear_updated_pki():
        rootdirUpdated = "./S2OPC_Demo_PKI/updatedTrustList"
        for issuer_or_trusted in ["issuers", "trusted"]:
            for cert_or_crl in ["certs", "crl"]:
                for files in os.listdir(os.path.join(rootdirUpdated, issuer_or_trusted, cert_or_crl)):
                    if files.endswith(".der"):
                        os.remove(os.path.join(rootdirUpdated, issuer_or_trusted, cert_or_crl, files))
                os.rmdir(os.path.join(rootdirUpdated, issuer_or_trusted, cert_or_crl))
            os.rmdir(os.path.join(rootdirUpdated, issuer_or_trusted))
        os.rmdir(rootdirUpdated)