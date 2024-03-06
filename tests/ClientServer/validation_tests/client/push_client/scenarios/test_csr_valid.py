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

from test_utils import ConnectionAndClear
import subprocess
import argparse
import sys
import os
import shutil
import signal
from cryptography import x509
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives import serialization
from datetime import datetime, timedelta

description = '''Test: Server receives a valid CSR without new key request, it then makes the CSR and sends it to the client, the client builds a certificate
                 from this CSR and with the CA, and eventually it updates the server certificate with this new certificate.
                 A former connected client is then disconnected. He cannot reconnect mentionning the old certificate, but he is able to reconnect
                 mentionning the new certificate. Same scenario with a CSR with new key this time.'''

# Info: the choice of writing the CSR file and using OpenSSL (via the Python module "cryptography") to obtain a certificate has been made.
# Another way to do it was to directly use MbedTLS sources in client C file after he received the csr from the server, but 
# doing so seems complicated (an example of making a certificate with csr is in mbedtls/programs/x509/cert_write.c).

def create_certificate_from_csr_and_ca(csr, ca_cert, ca_private_key):
    
    # Calling csr.extensions raises an error: "ValueError: error parsing asn1 value: ParseError { kind: EncodedDefault, location: ["BasicConstraints::ca"] }".
    # It seems like we cannot use for example (csr.extensions.get_extension_for_class(x509.BasicConstraints).value, 
    # csr.extensions.get_extension_for_class(x509.BasicConstraints).critical) with the Python version in the docker test.
    # Add manually the extensions to the certificate then.
    
    cert = x509.CertificateBuilder().subject_name(csr.subject).issuer_name(ca_cert.subject).public_key(
    csr.public_key()).serial_number(x509.random_serial_number()).not_valid_before(
    datetime.utcnow()).not_valid_after(datetime.utcnow() + timedelta(days=10)).add_extension(
    x509.BasicConstraints(ca=False, path_length=None), critical=False).add_extension(
    x509.KeyUsage(digital_signature=True, content_commitment=True, key_encipherment=True, data_encipherment=True,
    key_agreement=False, key_cert_sign=False, crl_sign=False, encipher_only=False, decipher_only=False), critical=False).add_extension(
    x509.ExtendedKeyUsage([x509.OID_SERVER_AUTH]), critical=False).add_extension(
    x509.SubjectAlternativeName([x509.UniformResourceIdentifier('urn:S2OPC:localhost'), x509.DNSName('localhost')]), critical=False).sign(
    ca_private_key, hashes.SHA256())
    
    return cert.public_bytes(serialization.Encoding.DER)

def handler(signum, frame):
    raise Exception("Timeout.")

def scenario_with_or_without_newKey(with_newKey, f):
    
    glob_ret = 0
    signal.signal(signal.SIGALRM, handler)
    max_connection_waiting_time = 2

    newKey = "noNewKey"
    nonce = "noNonce"
    if (with_newKey):
        newKey = "newKey"
        nonce = "nonce"
     
    # 1. CSR with new certificate and without new key. Creation succeeds.
    try:
        subprocess.check_call(["./push_client", "client_public/client_2k_cert.der", "client_private/encrypted_client_2k_key.pem", "csr", "groupIdValid", "certificateTypeIdValid", newKey, nonce])
        test_ret = 0
    except subprocess.CalledProcessError as e:
        test_ret = e.returncode
    if 0 != test_ret:
        glob_ret = 1
        f.write("Step 1 failed.\n")
    else:
        f.write("Step 1 success.\n")

    # Make the certificate file der with the CSR and CA root (issuer) certificate/key
    with open("push_data/input_csr.der", 'rb') as csr_file:
        csr = x509.load_der_x509_csr(csr_file.read())
    with open("S2OPC_Demo_PKI/trusted/certs/cacert.der", 'rb') as ca_cert_file:
        ca_cert = x509.load_der_x509_certificate(ca_cert_file.read())
    with open("push_data/cakey.pem", 'rb') as ca_private_key_file:
        ca_private_key = serialization.load_pem_private_key(ca_private_key_file.read(), bytes(os.getenv("TEST_PASSWORD_CACERT"), 'utf-8'))
        
    der_cert = create_certificate_from_csr_and_ca(csr, ca_cert, ca_private_key)

    with open("push_data/output_cert.der", 'wb') as der_cert_file:
        der_cert_file.write(der_cert)

    # 2. A client connects and sleeps.
    cmd = ["./push_client", "client_public/client_2k_cert.der", "client_private/encrypted_client_2k_key.pem", "sleep"]
    clientA_process = None
    signal.alarm(max_connection_waiting_time)
    try:
        clientA_process = ConnectionAndClear.launch_client_and_wait_for_connection(cmd)
        # Desactivate alarm
        signal.alarm(0)
    except Exception as exc:
        f.write("Timeout reached for connection.\n")
    if None != clientA_process:
        f.write("Step 2 success: client connected.\n")
    else:
        f.write("Step 2 failed: fail at launching client process or at connection time.\n")

    # 3. Another client updates the server certificate with the new certificate, it works, 
    # so they are both disconnected from the server.
    cmd = ["./push_client", "client_public/client_2k_cert.der", "client_private/encrypted_client_2k_key.pem", "updateCertificate", 
           "push_data/output_cert.der", "1", "S2OPC_Demo_PKI/trusted/certs/cacert.der", "sleep"]
    clientB_process = None
    signal.alarm(max_connection_waiting_time)
    try:
        clientB_process = ConnectionAndClear.launch_client_and_wait_for_connection(cmd)
        # Desactivate alarm
        signal.alarm(0)
    except Exception as exc:
        f.write("Timeout reached for connection.\n")
    if None != clientB_process:
        f.write("Step 3 success: client connected.\n")
    else:
        f.write("Step 3 failed: fail at launching client process or at connection time.\n")
    clientB_process.wait(2)
    test_ret = clientB_process.returncode
    if 3 != test_ret: # 3 is the return code for success at UpdateCertificate + disconnection.
        glob_ret = 1
        f.write("Step 3.a failed.\n")
    else:
        f.write("Step 3.a success.\n")

    clientA_process.wait(2)
    test_ret = clientA_process.returncode
    if 2 != test_ret: # 2 is the return code when disconnection occured.
        glob_ret = 1
        f.write("Step 3.b failed.\n")
    else:
        f.write("Step 3.b success.\n")
    
    # 4. The client cannot reconnect with old server certificate, but is able to connect with the new certificate
    try:
        subprocess.check_call(["./push_client", "client_public/client_2k_cert.der", "client_private/encrypted_client_2k_key.pem", "server_certificate", "push_data/server_4k_cert.der"])
        test_ret = 0
    except subprocess.CalledProcessError as e:
        test_ret = e.returncode
    if 0 == test_ret:
        glob_ret = 1
        f.write("Step 4.a failed.\n")
    else:
        f.write("Step 4.a success.\n")

    try:
        # not mentionning any certificate will make use "server_public/server_4k_cert.der" which is well the updated certificate of the server
        subprocess.check_call(["./push_client", "client_public/client_2k_cert.der", "client_private/encrypted_client_2k_key.pem"])
        test_ret = 0
    except subprocess.CalledProcessError as e:
        test_ret = e.returncode
    if 0 != test_ret:
        glob_ret = 1
        f.write("Step 4.b failed.\n")
    else:
        f.write("Step 4.b success.\n")

    return glob_ret

if __name__ == '__main__':
    
    parser = argparse.ArgumentParser(description=description)
    parser.add_argument('scenario', help='The scenario to run, newKey or noNewKey')
    args = parser.parse_args()

    glob_ret = 0

    if args.scenario == "newKey":
        # Produce log file.
        f = open("push_server_csr_valid_newkey.log", "w")
        test_ret = scenario_with_or_without_newKey(True, f)
        f.close()
        if 0 != test_ret:
            glob_ret = 1
        # If new key, remove it and put back the old one before shutting down the server
        os.remove("server_private/encrypted_server_4k_key.pem")
        shutil.copy("push_data/encrypted_server_4k_key.pem", "server_private")
    
    else:
        # Produce log file.
        f = open("push_server_csr_valid_nonewkey.log", "w")
        test_ret = scenario_with_or_without_newKey(False, f)
        f.close()
        if 0 != test_ret:
            glob_ret = 1

    ## Get back at the initial state before shutting down the server
    # Clear the data generated (csr and cert)...
    os.remove("push_data/input_csr.der")
    os.remove("push_data/output_cert.der")
    # ...and put back the old server certificate
    os.remove("server_public/server_4k_cert.der")
    shutil.copy("push_data/server_4k_cert.der", "server_public")

    sys.exit(glob_ret)