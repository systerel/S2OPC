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
    _, ext = os.path.splitext(key_path)
    with open(key_path, "rb") as f:
        data = f.read()
        if ext == ".pem":
            return serialization.load_pem_private_key(data, password=pwd, backend=default_backend())
        else:
            return serialization.load_der_private_key(data, password=None, backend=default_backend())

def _set_security(client, policy, certificate_path, private_key_path, server_certificate_path=None, mode=ua.MessageSecurityMode.SignAndEncrypt):
    if server_certificate_path is None:
        # load certificate from server's list of endpoints
        endpoints = client.connect_and_get_server_endpoints()
        endpoint = client.find_endpoint(endpoints, mode, policy.URI)
        server_cert = uacrypto.x509_from_der(endpoint.ServerCertificate)
    else:
        server_cert = uacrypto.load_certificate(server_certificate_path)
    cert = uacrypto.load_certificate(certificate_path)
    pk = _load_private_key(private_key_path, PRIVATE_KEY_PASSWORD)
    client.security_policy = policy(server_cert, cert, pk, mode)
    client.uaclient.set_security(client.security_policy)

def secure_channels_connect(client, security_policy):
    cert_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..', 'samples', 'ClientServer', 'data', 'cert')

    if (security_policy == security_policies.SecurityPolicyBasic256):
        _set_security(client,
        security_policy,
        os.path.join(cert_dir, 'client_2k_cert.der'),
        os.path.join(cert_dir, 'encrypted_client_2k_key.pem'),
        server_certificate_path=os.path.join(cert_dir, 'server_2k_cert.der'),
        mode=ua.MessageSecurityMode.Sign)

    client.connect()
    print('Connected')

