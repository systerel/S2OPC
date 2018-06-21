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

## CA generation
#gen key and certificate using the configuration file cassl.cnf (see example /usr/lib/ssl/openssl.cnf)
openssl req -new -x509 -days 365 -keyout cakey.pem -out cacert.pem -config cassl.cnf
touch index.txt        # database of signed certficates with ca
echo '01' > serial.txt # current serial number

#gen CA certificate in format parsed by UA stack
openssl x509 -in cacert.pem -out cacert.der -outform DER

## Signing a "client" certificate:
openssl ca -config cassl.cnf -policy signing_policy -days 365 -in client.csr -out client.crt

## Generate an empty Certificate Revocation List
openssl ca -config cassl.cnf -gencrl -out cacrl.pem
openssl crl -in cacrl.pem -outform DER -out cacrl.der
