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
rm -f index.txt serial.txt
touch index.txt
touch index.txt.attr
echo 00 > serial.txt  # Current serial
CONF_FILE=cassl.cnf
CONF_CLI=cli_req.cnf
CONF_SRV=srv_req.cnf
CA_KEY=cakey.pem
CA_CERT=cacert.pem
DURATION=730

# CA generation: generate key, generate self signed certificate# (does not work with 4096 key length)
# /!\ only for test as no pass phrase is embedeed"
openssl genrsa -out $CA_KEY 4096
openssl req -config $CONF_FILE -new -x509 -key $CA_KEY -out $CA_CERT -days $DURATION

# Generate an empty Certificate Revocation List, convert it to DER format for UA stack
openssl ca -config $CONF_FILE -gencrl -crldays $DURATION -out cacrl.pem
openssl crl -in cacrl.pem -outform der -out cacrl.der

# Generate, for both client and server, and both 2048 and 4096 key lengths, a new key pair
openssl req -config $CONF_CLI -reqexts client_cert -sha256 -nodes -newkey rsa:2048 -keyout client_2k_key.pem -out client_2k.csr
openssl req -config $CONF_CLI -reqexts client_cert -sha256 -nodes -newkey rsa:4096 -keyout client_4k_key.pem -out client_4k.csr
openssl req -config $CONF_SRV -reqexts server_cert -sha256 -nodes -newkey rsa:2048 -keyout server_2k_key.pem -out server_2k.csr
openssl req -config $CONF_SRV -reqexts server_cert -sha256 -nodes -newkey rsa:4096 -keyout server_4k_key.pem -out server_4k.csr
# Or create csr for existing certificates
#openssl req -new -config $CONF_CLI -sha256 -key client_2k_key.pem -reqexts client_cert -out client_2k.csr
#openssl req -new -config $CONF_CLI -sha256 -key client_4k_key.pem -reqexts client_cert -out client_4k.csr
#openssl req -new -config $CONF_SRV -sha256 -key server_2k_key.pem -reqexts server_cert -out server_2k.csr
#openssl req -new -config $CONF_SRV -sha256 -key server_4k_key.pem -reqexts server_cert -out server_4k.csr
# And sign them, for the next century
openssl ca -batch -config $CONF_FILE -policy signing_policy -extensions client_signing_req -days $DURATION -in client_2k.csr -out client_2k_cert.pem
openssl ca -batch -config $CONF_FILE -policy signing_policy -extensions client_signing_req -days $DURATION -in client_4k.csr -out client_4k_cert.pem
openssl ca -batch -config $CONF_FILE -policy signing_policy -extensions server_signing_req -days $DURATION -in server_2k.csr -out server_2k_cert.pem
openssl ca -batch -config $CONF_FILE -policy signing_policy -extensions server_signing_req -days $DURATION -in server_4k.csr -out server_4k_cert.pem

# Output certificates in DER format for UA stack
for fradix in ca client_2k_ client_4k_ server_2k_ server_4k_; do
    openssl x509 -in ${fradix}cert.pem -out ${fradix}cert.der -outform der
done

# Output hexlified certificate to include in check_crypto_certificates.c
echo "Server signed public key:"
hexdump -ve '/1 "%02x"' server_2k_cert.der
echo
echo -e "\nServer's thumbprint (SHA-1):"
openssl x509 -noout -fingerprint -in server_2k_cert.pem
echo
echo -e "\nCertificate Authority signed public key:"
hexdump -ve '/1 "%02x"' cacert.der
echo
echo -e "\nCRL of the CA:"
hexdump -ve '/1 "%02x"' cacrl.der
echo
