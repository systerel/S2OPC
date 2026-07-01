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

"""X.509 certificate and CRL builders for PKI test data."""

from cryptography import x509
from cryptography.hazmat.primitives import hashes

from pki_gen.extensions import ca_extensions, leaf_extensions, selfsigned_app_extensions


def build_ca(ca_key, name, serial, not_before, not_after, *, key_cert_sign=True):
    public_key = ca_key.public_key()
    builder = (
        x509.CertificateBuilder()
        .subject_name(name)
        .issuer_name(name)
        .public_key(public_key)
        .serial_number(serial)
        .not_valid_before(not_before)
        .not_valid_after(not_after)
    )
    for extension in ca_extensions(public_key, key_cert_sign=key_cert_sign):
        builder = builder.add_extension(extension, critical=isinstance(extension, x509.BasicConstraints))
    return builder.sign(ca_key, hashes.SHA256())


def intermediate_ca_extensions(public_key, issuer_public_key):
    return [
        x509.BasicConstraints(ca=True, path_length=0),
        x509.KeyUsage(
            digital_signature=True,
            key_cert_sign=True,
            crl_sign=True,
            content_commitment=False,
            key_encipherment=False,
            data_encipherment=False,
            key_agreement=False,
            encipher_only=False,
            decipher_only=False,
        ),
        x509.SubjectKeyIdentifier.from_public_key(public_key),
        x509.AuthorityKeyIdentifier.from_issuer_public_key(issuer_public_key),
    ]


def build_intermediate_ca(int_key, root_key, root_cert, name, serial, not_before, not_after):
    public_key = int_key.public_key()
    builder = (
        x509.CertificateBuilder()
        .subject_name(name)
        .issuer_name(root_cert.subject)
        .public_key(public_key)
        .serial_number(serial)
        .not_valid_before(not_before)
        .not_valid_after(not_after)
    )
    for extension in intermediate_ca_extensions(public_key, root_key.public_key()):
        builder = builder.add_extension(extension, critical=isinstance(extension, x509.BasicConstraints))
    return builder.sign(root_key, hashes.SHA256())


def build_leaf(leaf_key, ca_key, ca_cert, name, serial, not_before, not_after):
    public_key = leaf_key.public_key()
    builder = (
        x509.CertificateBuilder()
        .subject_name(name)
        .issuer_name(ca_cert.subject)
        .public_key(public_key)
        .serial_number(serial)
        .not_valid_before(not_before)
        .not_valid_after(not_after)
    )
    for extension in leaf_extensions(leaf_key.public_key(), ca_key.public_key()):
        builder = builder.add_extension(extension, critical=isinstance(extension, x509.BasicConstraints))
    return builder.sign(ca_key, hashes.SHA256())


def build_crl(ca_key, ca_cert, last_update, next_update, revoked_serials):
    builder = (
        x509.CertificateRevocationListBuilder()
        .issuer_name(ca_cert.subject)
        .last_update(last_update)
        .next_update(next_update)
        .add_extension(
            x509.AuthorityKeyIdentifier.from_issuer_public_key(ca_key.public_key()),
            critical=False,
        )
    )
    for serial in revoked_serials:
        revoked = (
            x509.RevokedCertificateBuilder()
            .serial_number(serial)
            .revocation_date(last_update)
            .build()
        )
        builder = builder.add_revoked_certificate(revoked)
    return builder.sign(ca_key, hashes.SHA256())


def build_selfsigned(ca_key, name, serial, not_before, not_after):
    public_key = ca_key.public_key()
    builder = (
        x509.CertificateBuilder()
        .subject_name(name)
        .issuer_name(name)
        .public_key(public_key)
        .serial_number(serial)
        .not_valid_before(not_before)
        .not_valid_after(not_after)
    )
    for extension in selfsigned_app_extensions(public_key):
        builder = builder.add_extension(extension, critical=isinstance(extension, x509.BasicConstraints))
    return builder.sign(ca_key, hashes.SHA256())
