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

# This script is used to generate CA root application certificate,
# with or without generation of new keys. Usage: 
# - Generate CA root application certificate with generation of new key (default behaviour): "./generate_selfsigned_certs.sh"
# - Generate CA root application certificate with existing keys: "./generate_selfsigned_certs.sh --no-key-generation"

set -e

CONF_FILE=ca_selfsigned_pathLen0ssl.cnf
CA_KEY=ca_selfsigned_pathLen0key.pem
CA_CERT=ca_selfsigned_pathLen0.pem

DURATION=730

# If no argument provided, default behaviour: CA root application certificate with new key.
if [ $# == 0 ]; then
    # CA generation: generate key, generate self signed certificate
    # /!\ CA key encrypted with AES-256-CBC, these commands require the password.
    openssl genrsa -traditional -out $CA_KEY -aes-256-cbc 4096
    openssl req -config $CONF_FILE -new -x509 -key $CA_KEY -out $CA_CERT -days $DURATION

# If argument --no-key-generation provided: renew CA root application certificate only (with existing key)
elif [ $# == 1 ] && [ $1 == "--no-key-generation" ]; then
    openssl req -new -config $CONF_FILE -new -x509 -key $CA_KEY -out $CA_CERT -days $DURATION

# If invalid argument(s) provided: print usage and return
else
    echo "Please provide valid argument: --no-key-generation or no argument at all"
    exit 1
fi

# Convert the certificate to DER format
openssl x509 -in $CA_CERT -out ca_selfsigned_pathLen0.der -outform DER
