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
from opcua import ua, Client
from opcua.crypto import security_policies
from opcua.crypto import uacrypto
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.backends import default_backend

nid_objectPublisherSubscribeObject = u"i=14443"
nid_methodGetSecurityKeys = u"i=15215"

TEST_PASSWORD_PRIV_KEY_ENV_NAME = "TEST_PASSWORD_PRIVATE_KEY"
TEST_PASSWORD_USER_ENV_NAME = "TEST_PASSWORD_USER_SECUADMIN"

USERNAME = "secuAdmin"

def _load_private_key(key_path, pwd):
    with open(key_path, "rb") as f:
        data = f.read()
        return serialization.load_pem_private_key(data, password=pwd, backend=default_backend())

def secure_channels_connect_username(client, security_policy):
    cert_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..', 'samples', 'ClientServer', 'data', 'cert')

    if (security_policy == security_policies.SecurityPolicyBasic256Sha256):
        client_cert = uacrypto.load_certificate(os.path.join(cert_dir, 'client_2k_cert.der'))
        pwd = os.getenv(TEST_PASSWORD_PRIV_KEY_ENV_NAME)
        if pwd is None:
            print("{} not set: it shall be set in the environment".format(TEST_PASSWORD_PRIV_KEY_ENV_NAME))
            exit(1)
        pwd = bytes(pwd, 'utf-8')

        client_key = _load_private_key(os.path.join(cert_dir, 'encrypted_client_2k_key.pem'), pwd)
        server_cert = uacrypto.load_certificate(os.path.join(cert_dir, 'server_2k_cert.der'))

        client.security_policy = security_policy(server_cert, client_cert, client_key, ua.MessageSecurityMode.SignAndEncrypt)
        client.uaclient.set_security(client.security_policy)

    client.set_user(USERNAME)
    pwd = os.getenv(TEST_PASSWORD_USER_ENV_NAME)
    if pwd is None:
        print("{} not set: it shall be set in the environment".format(TEST_PASSWORD_USER_ENV_NAME))
        exit(2)

    client.set_password(pwd)

    client.connect()
    print('Connected')

class SKSServer:
    """Wraps a client that connects to the SKS pull model server with provided URL"""

    def __init__(self, uri):
        self.uri = uri
        self.client = Client(self.uri)
        self.client.application_uri = "urn:S2OPC:localhost"

    # Connect to the SKS server. Shall be called before other methods
    def connect(self):
        secure_channels_connect_username(self.client, security_policies.SecurityPolicyBasic256Sha256)
        self.nodeObjectPublisherSubscribe = self.client.get_node(nid_objectPublisherSubscribeObject)
        self.nodeMethodGetSecurityKeys = self.client.get_node(nid_methodGetSecurityKeys)

    # Disconnect to the server
    def disconnect(self):
        self.client.disconnect()
        print('Disconnected')

    # Call method GetSecurityKeys with parameters SecurityGroupid, StartingTokenId, RequestedKeyCount
    def callGetSecurityKeys(self, SecurityGroupid, StartingTokenId, RequestedKeyCount):
        try:
            # method call failure trigger an exception
            response = self.nodeObjectPublisherSubscribe.call_method(self.nodeMethodGetSecurityKeys,
                                                                     ua.Variant(SecurityGroupid,ua.VariantType.String),
                                                                     ua.Variant(StartingTokenId,ua.VariantType.UInt32),
                                                                     ua.Variant(RequestedKeyCount,ua.VariantType.UInt32))
        except Exception as e:
            print('Client probably not connected to SKS server. Error:')
            print(e)

        keyLifeTime = 0
        key = 0
        if response:
            assert(5 == len(response)) # GetSecurityKeys method has 5 output arguments
            keys = response[2]
            keyLifeTime = response[4]/1000
            print("Nbr of keys:", len(keys))
            for i in range(len(keys)):
                key = keys[i]
                print(f"Hexa of key {i}:", key.hex())
            print("TimeToNextKey:", response[3]/1000, "s")
            print("KeyLifeTime:", keyLifeTime, "s")

        return key, keyLifeTime
