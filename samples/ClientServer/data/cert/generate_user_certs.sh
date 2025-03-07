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

# This script is used to generate user root CA and user certificates,
# with or without generation of new keys. Usage: 
# - Generate user root CA + user certificates, with generation of new keys (default behaviour): "./generate_user_certs.sh"
# - Generate user certificates with existing keys: "./generate_user_certs.sh --no-key-generation"

set -e

CONF_CA_USR=usr_cassl.cnf
CONF_USR=usr_req.cnf
CA_KEY_USR=user_cakey.pem
CA_CERT_USR=user_cacert.pem

DURATION=730

# If no argument provided, default behaviour: regenerate user root CA + its crl, and user certificates, all with new keys.
if [ $# == 0 ]; then
    # CA generation for users X509IdentityToken: generate key, generate self signed certificate
    # /!\ only for test as no pass phrase is embedeed
    openssl genrsa -out $CA_KEY_USR -aes-256-cbc 4096
    openssl req -config $CONF_CA_USR -new -x509 -key $CA_KEY_USR -out $CA_CERT_USR -days $DURATION

    # Generate an empty Certificate Revocation List, convert it to DER format
    openssl ca -config $CONF_CA_USR -gencrl -crldays $DURATION -out user_cacrl.pem
    openssl crl -in user_cacrl.pem -outform der -out user_cacrl.der

    # Generate user X509IdentityToken 2048 key lengths, a new key pair
    openssl req -config $CONF_USR -reqexts user_cert -sha256 -nodes -newkey rsa:2048 -keyout user_2k_key.pem -out user_2k.csr
    openssl req -config $CONF_USR -reqexts user_cert -sha256 -nodes -newkey rsa:4096 -keyout user_4k_key.pem -out user_4k.csr

    # Generate user encrypted private keys (these commands require the password).
    echo "****** User private keys encryption ******"
    openssl rsa -in user_2k_key.pem -aes-256-cbc -out encrypted_user_2k_key.pem
    openssl rsa -in user_4k_key.pem -aes-256-cbc -out encrypted_user_4k_key.pem

    # Remove the unencrypted keys
    rm user*_key.pem

    # Convert CA root into DER
    openssl x509 -in $CA_CERT_USR -out user_cacert.der -outform der

    # Print
    echo -e "\nUser certificate Authority signed public key:"
    hexdump -ve '/1 "%02x"' user_cacert.der
    echo
    echo -e "\nCRL of the CA:"
    hexdump -ve '/1 "%02x"' user_cacrl.der
    echo

# If argument --no-key-generation provided: renew user certificates only (with existing keys)
elif [ $# == 1 ] && [ $1 == "--no-key-generation" ]; then
    # create csr for existing user keys
    openssl req -new -config $CONF_USR -sha256 -key encrypted_user_2k_key.pem -reqexts user_cert -out user_2k.csr
    openssl req -new -config $CONF_USR -sha256 -key encrypted_user_4k_key.pem -reqexts user_cert -out user_4k.csr

# If invalid argument(s) provided: print usage and return
else
    echo "Please provide valid argument: --no-key-generation or no argument at all"
    exit 1
fi

# And sign them, for the next two years
openssl ca -batch -config $CONF_CA_USR -policy signing_policy -extensions user_signing_req -days $DURATION -in user_2k.csr -out user_2k_cert.pem
openssl ca -batch -config $CONF_CA_USR -policy signing_policy -extensions user_signing_req -days $DURATION -in user_4k.csr -out user_4k_cert.pem

# Remove the CSRs
rm ./*.csr

# Output application and user certificates in DER format
for fradix in user_2k_ user_4k_; do
    openssl x509 -in ${fradix}cert.pem -out ${fradix}cert.der -outform der
done

# Output hexlified certificate to include in check_crypto_certificates.c
echo "User signed 2k public key:"
hexdump -ve '/1 "%02x"' user_2k_cert.der
echo
echo -e "\nUser 2k cert's thumbprint (SHA-1):"
openssl x509 -noout -fingerprint -in user_2k_cert.pem
echo
