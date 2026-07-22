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

"""Generate cycle_pki_test DER files (mutually signed intermediate CAs)."""

import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "scripts"))

from pki_gen import (
    PkiStoreGenerator,
    VALID_PERIOD_CA,
    VALID_PERIOD_LEAF,
    build_ca,
    build_cross_signed_ca,
    build_leaf,
    generate_rsa_key,
    systerel_name,
)

TEST_DIR = Path(__file__).resolve().parent


def decoy_root_name():
    return systerel_name("S2OPC Cycle PKI Test Decoy Root")


def main():
    gen = PkiStoreGenerator(TEST_DIR)

    # Unrelated trusted root so the PKI store can be created (not part of the A/B cycle).
    decoy_key = generate_rsa_key()
    decoy_root = build_ca(decoy_key, decoy_root_name(), 0x9000, *VALID_PERIOD_CA)
    gen.write_trusted_cert("decoy_root.der", decoy_root)

    name_a = systerel_name("S2OPC Cycle PKI Test CA A")
    name_b = systerel_name("S2OPC Cycle PKI Test CA B")
    name_leaf = systerel_name("S2OPC Cycle PKI Test Leaf")

    key_a = generate_rsa_key()
    key_b = generate_rsa_key()
    key_leaf = generate_rsa_key()

    cert_a = build_cross_signed_ca(key_a, key_b, name_b, name_a, 0x9001, *VALID_PERIOD_CA)
    cert_b = build_cross_signed_ca(key_b, key_a, name_a, name_b, 0x9002, *VALID_PERIOD_CA)
    leaf = build_leaf(key_leaf, key_a, cert_a, name_leaf, 0x9003, *VALID_PERIOD_LEAF)

    gen.write_issuer_cert("ca_cycle_a.der", cert_a)
    gen.write_issuer_cert("ca_cycle_b.der", cert_b)
    gen.write_leaf("leaf_cycle.der", leaf)

    print(f"Generated DER files in {TEST_DIR}")


if __name__ == "__main__":
    main()
