#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (C) 2018 Systerel and others.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
