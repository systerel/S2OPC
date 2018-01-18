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

from time import sleep

def secure_channel_renew(client, logger):
    # Define renew time to 1 second
    client.secure_channel_timeout=1000
    # Renew with 1 second
    client.open_secure_channel(renew=True)
    print('Open Secure Channel renewed')
    # Check revised time
    logger.add_test('OPN renew test - renewed with given timeout value', client.secure_channel_timeout == 1000)
    # Change revised time to avoid client to renew the security token in time
    client.secure_channel_timeout=10000
    # Read a node to be sure we are using the new security token
    nid = 1001
    node = client.get_node(nid)
    value = node.get_value()
    print(' Value for Node {:03d}:'.format(nid), value)
    # Wait timeout of the security token
    sleep(1)
    print(' Error expected on next read:')
    # Try to read a node again
    try:
        node = client.get_node(nid)
        value = node.get_value()
    except:
        logger.add_test('OPN renew test - read refused after timeout', True)
    else:
        logger.add_test('OPN renew test - read refused after timeout', False)
    
    try:
        client.disconnect()
    except:
        None
