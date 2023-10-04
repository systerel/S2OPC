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

CONF_FILE=cassl.cnf
CONF_CLI=cli_req.cnf
CONF_SRV=srv_req.cnf
CA_KEY=cakey.pem
CA_CERT=cacert.pem

DURATION=730


# Revoke certificates
openssl ca -config $CONF_FILE -revoke client_2k_cert.pem
openssl ca -config $CONF_FILE -revoke client_4k_cert.pem
openssl ca -config $CONF_FILE -revoke server_2k_cert.pem
openssl ca -config $CONF_FILE -revoke server_4k_cert.pem

# Generate new CRL
openssl ca -config $CONF_FILE -crldays $DURATION -gencrl -out cacrl.pem
openssl crl -in cacrl.pem -outform der -out cacrl.der

echo -e "\nCRL of the CA:"
hexdump -ve '/1 "%02x"' cacrl.der
echo
