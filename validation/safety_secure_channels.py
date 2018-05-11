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

import os
from opcua import ua
from opcua.crypto import security_policies

def secure_channels_connect(client, security_policy):
    cert_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'tests', 'data', 'cert')

    if (security_policy == security_policies.SecurityPolicyBasic256):
        client.set_security(security_policy,
        os.path.join(cert_dir, 'client_2k_cert.der'),
        os.path.join(cert_dir, 'client_2k_key.pem'),
        server_certificate_path=os.path.join(cert_dir, 'server_2k_cert.der'),
        mode=ua.MessageSecurityMode.Sign)

    client.connect()
    print('Connected')

