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
from opcua import ua
from opcua.crypto import security_policies
from opcua.crypto import uacrypto
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.backends import default_backend

PRIVATE_KEY_PASSWORD = b'password'

def _load_private_key(key_path, pwd):
    with open(key_path, "rb") as f:
        data = f.read()
        return serialization.load_pem_private_key(data, password=pwd, backend=default_backend())

def secure_channels_connect(client, security_policy):
    cert_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..', 'samples', 'ClientServer', 'data', 'cert')

    if (security_policy == security_policies.SecurityPolicyBasic256):
        client_cert = uacrypto.load_certificate(os.path.join(cert_dir, 'client_2k_cert.der'))
        cient_key = _load_private_key(os.path.join(cert_dir, 'encrypted_client_2k_key.pem'), PRIVATE_KEY_PASSWORD)
        server_cert = uacrypto.load_certificate(os.path.join(cert_dir, 'server_2k_cert.der'))

        client.security_policy = security_policy(server_cert, client_cert, cient_key, ua.MessageSecurityMode.Sign)
        client.uaclient.set_security(client.security_policy)

    client.connect()
    print('Connected')

