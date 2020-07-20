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


"""
Defines the configurations used by the examples.
"""


import os

SERVER_URL = 'opc.tcp://localhost:4841'
CERT_BASE_PATH = '/bin-s2opc'

# See the documentation of pys2opc.PyS2OPC.add_configuration_unsecured for more parameters.
configuration_parameters_no_subscription = {'server_url': SERVER_URL,
                                            'token_target': 0}
configuration_parameters_subscription = {'server_url': SERVER_URL,
                                         'publish_period' : 500,
                                         'n_max_keepalive' : 3,
                                         'token_target': 3}

configuration_parameters_security = {'path_cert_auth': os.path.join(CERT_BASE_PATH, 'trusted', 'cacert.der'),
                                     'path_crl': os.path.join(CERT_BASE_PATH, 'revoked', 'cacrl.der'),
                                     'path_cert_srv': os.path.join(CERT_BASE_PATH, 'server_public', 'server_2k_cert.der'),
                                     'path_cert_cli': os.path.join(CERT_BASE_PATH, 'client_public', 'client_2k_cert.der'),
                                     'path_key_cli': os.path.join(CERT_BASE_PATH, 'client_private', 'client_2k_key.pem')}

def join_configs(*args):
    return {k:v for k,v in sum(map(list, map(dict.items, args)), [])}
