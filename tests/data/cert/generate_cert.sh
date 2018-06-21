#!/bin/bash

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

# Note: use only non commented lines for nominal use

# E.g.: gen key (encrypted output with AES 128 for a 1024 bit asymmetric key)
# openssl genrsa -aes128 -out client_1k.key 1024

# OR gen key without encryption for a 2048 bit asymmetric key the key and certificate request
openssl req -config clientssl_2k.cnf -newkey rsa:2048 -sha256 -nodes -out client_2k.csr -outform PEM
# then sign the request with certificate
openssl ca -config cassl.cnf -policy signing_policy -days 365 -in client_2k.csr -out client_2k_cert.pem

#gen certificate format parsed by UA stack
openssl x509 -in client_2k_cert.pem -out client_2k_cert.der -outform DER
