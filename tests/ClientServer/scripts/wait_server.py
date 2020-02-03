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

# Waits for a socket to successfully connect to a server, i.e.
#  waits for the server to boot.
# Waits at most TIMEOUT seconds

import sys
import time
import socket
from urllib.parse import urlparse

TIMEOUT = 20.
DEFAULT_URL = 'opc.tcp://localhost:4841'

def wait_server(url, timeout):
    # Parse url to find the endpoint IP, connects while not TIMEOUT
    pr = urlparse(url)
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.settimeout(timeout)
    t0 = time.time()
    while 'Waiting for succesful connection':
        try:
            sock.connect((pr.hostname, pr.port))
            sock.close()
            return True
        except ConnectionError:
            time.sleep(0.1)
        if time.time()-t0 >= TIMEOUT:
            return False

if __name__ == '__main__':
    # Check args
    if len(sys.argv) == 1:
        sUrl = DEFAULT_URL
    elif len(sys.argv) == 2:
        sUrl = sys.argv[1]
    else:
        print('Usage:', sys.argv[0], '[ENDPOINT_URL]')
        sys.exit(1)

    sys.exit(0 if wait_server(sUrl, TIMEOUT) else 1)
