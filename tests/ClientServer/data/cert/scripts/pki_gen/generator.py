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

"""PKI store layout helper for test certificate generation."""

from dataclasses import dataclass
from pathlib import Path

from pki_gen.builders import build_ca, build_crl, build_intermediate_ca, build_leaf
from pki_gen.common import write_der
from pki_gen.signature import corrupt_der_signature


@dataclass
class CaSpec:
    filename: str
    serial: int
    not_before: object
    not_after: object
    name: object


@dataclass
class CrlSpec:
    filename: str
    issuer: str
    last_update: object
    next_update: object
    revoked: list
    trusted: bool = True


@dataclass
class LeafSpec:
    filename: str
    issuer: str
    serial: int
    not_before: object
    not_after: object
    name: object


@dataclass
class RootCaSpec:
    filename: str
    serial: int
    not_before: object
    not_after: object
    name: object


@dataclass
class IntermediateCaSpec:
    filename: str
    serial: int
    not_before: object
    not_after: object
    name: object
    trusted: bool = True
    corrupt_signature: bool = False


@dataclass
class ChainLeafSpec:
    filename: str
    issuer: str
    serial: int
    not_before: object
    not_after: object
    name: object


class PkiStoreGenerator:
    def __init__(self, test_dir):
        self.test_dir = Path(test_dir)
        self.trusted_certs_dir = self.test_dir / "trusted" / "certs"
        self.trusted_crl_dir = self.test_dir / "trusted" / "crl"
        self.issuer_certs_dir = self.test_dir / "issuers" / "certs"
        self.issuer_crl_dir = self.test_dir / "issuers" / "crl"

    def write_trusted_cert(self, filename, cert):
        write_der(self.trusted_certs_dir / filename, cert)

    def write_crl(self, filename, crl):
        write_der(self.trusted_crl_dir / filename, crl)

    def write_leaf(self, filename, cert):
        write_der(self.test_dir / filename, cert)

    def write_issuer_cert(self, filename, cert):
        write_der(self.issuer_certs_dir / filename, cert)

    def write_issuer_crl(self, filename, crl):
        write_der(self.issuer_crl_dir / filename, crl)

    def write_trusted_cert_corrupted(self, filename, cert):
        write_der(self.trusted_certs_dir / filename, corrupt_der_signature(cert))

    def write_crl_corrupted(self, filename, crl):
        write_der(self.trusted_crl_dir / filename, corrupt_der_signature(crl))

    def write_leaf_corrupted(self, filename, cert):
        write_der(self.test_dir / filename, corrupt_der_signature(cert))

    def _write_cert(self, filename, cert, trusted):
        if trusted:
            self.write_trusted_cert(filename, cert)
        else:
            self.write_issuer_cert(filename, cert)

    def _write_crl_to_store(self, filename, crl, trusted):
        if trusted:
            self.write_crl(filename, crl)
        else:
            self.write_issuer_crl(filename, crl)

    def generate_case(self, ca_key, leaf_key, ca_specs, crl_specs, leaf_spec):
        ca_certs = {}
        for spec in ca_specs:
            ca_certs[spec.filename] = build_ca(
                ca_key, spec.name, spec.serial, spec.not_before, spec.not_after
            )
            self.write_trusted_cert(spec.filename, ca_certs[spec.filename])

        issuer_cert = ca_certs[leaf_spec.issuer]
        leaf = build_leaf(
            leaf_key,
            ca_key,
            issuer_cert,
            leaf_spec.name,
            leaf_spec.serial,
            leaf_spec.not_before,
            leaf_spec.not_after,
        )
        self.write_leaf(leaf_spec.filename, leaf)

        for spec in crl_specs:
            self._write_crl_to_store(
                spec.filename,
                build_crl(
                    ca_key,
                    ca_certs[spec.issuer],
                    spec.last_update,
                    spec.next_update,
                    spec.revoked,
                ),
                spec.trusted,
            )

    def generate_chain_case(
        self,
        root_key,
        int_key,
        leaf_key,
        root_spec,
        intermediate_specs,
        crl_specs,
        leaf_spec,
    ):
        """Build Root -> Intermediate(s) -> Leaf and write PKI store material."""
        ca_certs = {}
        ca_keys = {}

        root_cert = build_ca(
            root_key, root_spec.name, root_spec.serial, root_spec.not_before, root_spec.not_after
        )
        ca_certs[root_spec.filename] = root_cert
        ca_keys[root_spec.filename] = root_key
        self.write_trusted_cert(root_spec.filename, root_cert)

        for spec in intermediate_specs:
            parent_filename = root_spec.filename
            parent_cert = ca_certs[parent_filename]
            parent_key = ca_keys[parent_filename]
            inter_cert = build_intermediate_ca(
                int_key,
                parent_key,
                parent_cert,
                spec.name,
                spec.serial,
                spec.not_before,
                spec.not_after,
            )
            ca_certs[spec.filename] = inter_cert
            ca_keys[spec.filename] = int_key
            if spec.corrupt_signature:
                self._write_cert(spec.filename, corrupt_der_signature(inter_cert), spec.trusted)
            else:
                self._write_cert(spec.filename, inter_cert, spec.trusted)

        issuer_cert = ca_certs[leaf_spec.issuer]
        signer_key = ca_keys[leaf_spec.issuer]
        leaf = build_leaf(
            leaf_key,
            signer_key,
            issuer_cert,
            leaf_spec.name,
            leaf_spec.serial,
            leaf_spec.not_before,
            leaf_spec.not_after,
        )
        self.write_leaf(leaf_spec.filename, leaf)

        for spec in crl_specs:
            issuer_key = ca_keys[spec.issuer]
            self._write_crl_to_store(
                spec.filename,
                build_crl(
                    issuer_key,
                    ca_certs[spec.issuer],
                    spec.last_update,
                    spec.next_update,
                    spec.revoked,
                ),
                spec.trusted,
            )
