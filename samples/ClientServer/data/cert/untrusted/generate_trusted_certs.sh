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


set -e

CONF_FILE=untrustCA.cnf
CONF_CLI=trusted_cli_req.cnf
CONF_SRV=trusted_srv_req.cnf
CA_PREFIX=untrusted_ca
CA_KEY="$CA_PREFIX"key.pem
CA_CERT="$CA_PREFIX"cert.pem
CA_CRL="$CA_PREFIX"crl.pem
CLI_PREFIX=trusted_client
CLI_KEY="$CLI_PREFIX"_key.pem
SRV_PREFIX=trusted_server
SRV_KEY="$SRV_PREFIX"_key.pem


DURATION=730

# CA generation: generate key, generate self signed certificate
# /!\ CA key encrypted with AES-256-CBC, these commands require the password.
openssl genrsa -out $CA_KEY -aes-256-cbc 4096
openssl req -config $CONF_FILE -new -x509 -key $CA_KEY -out $CA_CERT -days $DURATION

# Generate an empty Certificate Revocation List, convert it to DER format
openssl ca -config $CONF_FILE -gencrl -crldays $DURATION -out $CA_CRL

# Generate, for both client and server, and both 2048 and 4096 key lengths, a new key pair
openssl req -config $CONF_CLI -reqexts client_cert -sha256 -nodes -newkey rsa:4096 -keyout "$CLI_PREFIX"_key.pem -out "$CLI_PREFIX".csr
openssl req -config $CONF_SRV -reqexts server_cert -sha256 -nodes -newkey rsa:4096 -keyout "$SRV_KEY" -out "$SRV_PREFIX".csr
# And sign them, for the next two years
openssl ca -batch -config $CONF_FILE -policy signing_policy -extensions client_signing_req -days $DURATION -in "$CLI_PREFIX".csr -out "$CLI_PREFIX"_cert.pem
openssl ca -batch -config $CONF_FILE -policy signing_policy -extensions server_signing_req -days $DURATION -in "$SRV_PREFIX".csr -out "$SRV_PREFIX"_cert.pem

# Remove the CSRs
rm ./*.csr

# Generate, for the client and the server, the encrypted private keys (these commands require the password).
echo "****** Server private keys encryption ******"
openssl rsa -in "$SRV_KEY" -aes-256-cbc -out encrypted_"$SRV_KEY"
echo "****** Client private keys encryption ******"
openssl rsa -in "$CLI_KEY" -aes-256-cbc -out encrypted_"$CLI_KEY"

# Remove the unencrypted keys
rm "$CLI_KEY" && rm "$SRV_KEY"

