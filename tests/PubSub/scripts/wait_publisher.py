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

TIMEOUT = 1.0
DEFAULT_URL = 'opc.udp://232.1.2.100:4840'

def wait_publisher(url, timeout):
    # Parse url to find the endpoint IP, connects while not TIMEOUT
    pr = urlparse(url)
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock.settimeout(timeout)
    t0 = time.time()
    while 'Waiting for succesful connection':
        try:
            group = socket.inet_aton(pr.hostname)
            host = socket.gethostname()
            _, _, _, _, sockaddr = socket.getaddrinfo(None, pr.port, family=socket.AF_INET, type=socket.SOCK_DGRAM, proto=socket.IPPROTO_UDP, flags=socket.AI_PASSIVE)[0]
            iface = socket.inet_aton(sockaddr[0]) # first element of tuple is address
            sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, group + iface)
            sock.bind((pr.hostname, pr.port))

            data = sock.recv(4096)
            sock.close()
            return True
        except OSError:
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
        print('Usage:', sys.argv[0], '[opc.udp://UDP_IP:PORT]')
        sys.exit(1)

    sys.exit(0 if wait_publisher(sUrl, TIMEOUT) else 1)
