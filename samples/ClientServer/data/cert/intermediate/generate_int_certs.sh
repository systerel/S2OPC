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

# This script is used to generate intermediate CA client / server root and application certificates,
# with or without generation of new keys. Usage: 
# - Generate intermediate CA client / server + int application certificates, with generation of new keys (default behaviour): "./generate_int_certs.sh"
# - Generate application certificates with existing keys: "./generate_int_certs.sh --no-key-generation"

set -e

ROOT_CONF_FILE=cassl.cnf
CA_CLI_PREFIX=int_cli
CA_SRV_PREFIX=int_srv
CONF_CA_CLI="$CA_CLI_PREFIX"_cassl.cnf
CONF_CA_SRV="$CA_SRV_PREFIX"_cassl.cnf
CONF_CLI=cli_req.cnf
CONF_SRV=srv_req.cnf
CLI_PREFIX=int_client
CLI_KEY="$CLI_PREFIX"_key.pem
SRV_PREFIX=int_server
SRV_KEY="$SRV_PREFIX"_key.pem

DURATION=730

# If no argument provided, default behaviour: regenerate intermediate CA client / server + their crls, and application certificates, all with new keys.
if [ $# == 0 ]; then
    # Intermediate client/server CAs generation: generate key, generate root CA signed certificate
    openssl req -config $CONF_CA_CLI -sha256 -nodes -newkey rsa:4096 -keyout "$CA_CLI_PREFIX"_cakey.pem -out "$CA_CLI_PREFIX"_ca.csr
    openssl req -config $CONF_CA_SRV -sha256 -nodes -newkey rsa:4096 -keyout "$CA_SRV_PREFIX"_cakey.pem -out "$CA_SRV_PREFIX"_ca.csr

    pushd ..
    openssl ca -batch -config $ROOT_CONF_FILE -policy signing_policy -days $DURATION -in intermediate/"$CA_CLI_PREFIX"_ca.csr -out intermediate/"$CA_CLI_PREFIX"_cacert.pem
    openssl ca -batch -config $ROOT_CONF_FILE -policy signing_policy -days $DURATION -in intermediate/"$CA_SRV_PREFIX"_ca.csr -out intermediate/"$CA_SRV_PREFIX"_cacert.pem
    popd

    # Generate an empty Certificate Revocation List, convert it to DER format
    openssl ca -config $CONF_CA_CLI -gencrl -crldays $DURATION -out "$CA_CLI_PREFIX"_cacrl.pem
    openssl ca -config $CONF_CA_SRV -gencrl -crldays $DURATION -out "$CA_SRV_PREFIX"_cacrl.pem

    # Generate, for both client and server a new key pair
    openssl req -config $CONF_CLI -reqexts client_cert -sha256 -nodes -newkey rsa:4096 -keyout "$CLI_KEY" -out "$CLI_PREFIX".csr
    openssl req -config $CONF_SRV -reqexts server_cert -sha256 -nodes -newkey rsa:4096 -keyout "$SRV_KEY" -out "$SRV_PREFIX".csr

    # Generate, for the client and the server, the encrypted private keys (these commands require the password).
    echo "****** Server private keys encryption ******"
    openssl rsa -traditional  -in "$SRV_KEY" -aes-256-cbc -out encrypted_"$SRV_KEY"
    echo "****** Client private keys encryption ******"
    openssl rsa -traditional  -in "$CLI_KEY" -aes-256-cbc -out encrypted_"$CLI_KEY"
    # Generate the encrypted intermediate client/server CA private keys (these commands require the password).
    echo "****** Intermediate Server CA private keys encryption ******"
    openssl rsa -traditional  -in "$CA_SRV_PREFIX"_cakey.pem -aes-256-cbc -out encrypted_"$CA_SRV_PREFIX"_cakey.pem
    echo "****** Intermediate Client CA private keys encryption ******"
    openssl rsa -traditional  -in "$CA_CLI_PREFIX"_cakey.pem -aes-256-cbc -out encrypted_"$CA_CLI_PREFIX"_cakey.pem

    # Remove the unencrypted keys
    rm "$CLI_KEY" && rm "$SRV_KEY"
    rm "$CA_CLI_PREFIX"_cakey.pem && rm "$CA_SRV_PREFIX"_cakey.pem

# If argument --no-key-generation provided: renew application certificates only (with existing keys)
elif [ $# == 1 ] && [ $1 == "--no-key-generation" ]; then
    openssl req -new -config $CONF_CLI -sha256 -key encrypted_"$CLI_KEY" -reqexts client_cert -out "$CLI_PREFIX".csr
    openssl req -new -config $CONF_SRV -sha256 -key encrypted_"$SRV_KEY" -reqexts server_cert -out "$SRV_PREFIX".csr

# If invalid argument(s) provided: print usage and return
else
    echo "Please provide valid argument: --no-key-generation or no argument at all"
    exit 1
fi

# And sign them, for the next two years
openssl ca -batch -config $CONF_CA_CLI -policy signing_policy -extensions client_signing_req -days $DURATION -in "$CLI_PREFIX".csr -out "$CLI_PREFIX"_cert.pem
openssl ca -batch -config $CONF_CA_SRV -policy signing_policy -extensions server_signing_req -days $DURATION -in "$SRV_PREFIX".csr -out "$SRV_PREFIX"_cert.pem

# Remove the CSRs
rm ./*.csr
