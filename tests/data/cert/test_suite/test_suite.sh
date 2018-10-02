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

setenv -e

touch index.txt
touch index.txt.attr
echo FA > serial.txt
# -keyform der does not work...
# 4096 key length does not work...
openssl req -config test_ca.cnf -x509 -newkey rsa:2048 -nodes -keyout test_ca.key.pem -out test_ca.pem -days 36500
openssl x509 -in test_ca.pem -out test_ca.der -outform der
openssl pkey -in test_ca.key.pem -out test_ca.key -outform der
# 2048 is the keylength that works for both Basic256 and Basic256Sha256 policies
openssl req -config test_server.cnf -newkey rsa:2048 -sha256 -nodes -keyout test_server.key.pem -out test_server.csr -outform pem
openssl ca -config test_ca.cnf -policy signing_policy -days 36500 -in test_server.csr -out test_server.pem
openssl x509 -in test_server.pem -out test_server.der -outform der

echo "Server signed public key:"
hexdump -ve '/1 "%02x"' test_server.der
echo
echo -e "\nCertificate Authority signed public key:"
hexdump -ve '/1 "%02x"' test_ca.der
echo
