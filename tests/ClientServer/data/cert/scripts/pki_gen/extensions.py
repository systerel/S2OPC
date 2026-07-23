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

"""X.509 extension sets for PKI test certificate generation."""

from cryptography import x509
from cryptography.x509.oid import ExtendedKeyUsageOID


def ca_extensions(public_key, *, key_cert_sign=True, path_length=None):
    return [
        x509.BasicConstraints(ca=True, path_length=path_length),
        x509.KeyUsage(
            digital_signature=True,
            key_cert_sign=key_cert_sign,
            crl_sign=True,
            content_commitment=False,
            key_encipherment=False,
            data_encipherment=False,
            key_agreement=False,
            encipher_only=False,
            decipher_only=False,
        ),
        x509.SubjectKeyIdentifier.from_public_key(public_key),
    ]


def leaf_extensions(leaf_public_key, ca_public_key):
    return [
        x509.BasicConstraints(ca=False, path_length=None),
        x509.KeyUsage(
            digital_signature=True,
            content_commitment=True,
            key_encipherment=True,
            data_encipherment=True,
            key_cert_sign=False,
            crl_sign=False,
            key_agreement=False,
            encipher_only=False,
            decipher_only=False,
        ),
        x509.ExtendedKeyUsage([ExtendedKeyUsageOID.CLIENT_AUTH]),
        x509.SubjectAlternativeName(
            [
                x509.UniformResourceIdentifier("urn:S2OPC:localhost"),
                x509.DNSName("localhost"),
            ]
        ),
        x509.SubjectKeyIdentifier.from_public_key(leaf_public_key),
        x509.AuthorityKeyIdentifier.from_issuer_public_key(ca_public_key),
    ]


def selfsigned_app_extensions(public_key):
    return [
        x509.BasicConstraints(ca=True, path_length=0),
        x509.KeyUsage(
            digital_signature=True,
            content_commitment=True,
            key_cert_sign=True,
            key_encipherment=True,
            data_encipherment=True,
            crl_sign=False,
            key_agreement=False,
            encipher_only=False,
            decipher_only=False,
        ),
        x509.ExtendedKeyUsage([ExtendedKeyUsageOID.CLIENT_AUTH, ExtendedKeyUsageOID.SERVER_AUTH]),
        x509.SubjectAlternativeName(
            [
                x509.UniformResourceIdentifier("urn:S2OPC:localhost"),
                x509.DNSName("localhost"),
            ]
        ),
        x509.SubjectKeyIdentifier.from_public_key(public_key),
        x509.AuthorityKeyIdentifier.from_issuer_public_key(public_key),
    ]
