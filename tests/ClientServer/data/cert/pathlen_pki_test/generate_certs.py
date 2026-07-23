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

"""Generate pathlen_pki_test DER files (pathLenConstraint chain tests)."""

import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "scripts"))

from pki_gen import (
    PkiStoreGenerator,
    VALID_PERIOD_CA,
    VALID_PERIOD_LEAF,
    build_ca,
    build_intermediate_ca,
    build_leaf,
    generate_rsa_key,
    systerel_name,
)

TEST_DIR = Path(__file__).resolve().parent


def main():
    gen = PkiStoreGenerator(TEST_DIR)

    # PL-OK: root pathLen=1, one intermediate (no pathLenConstraint), leaf.
    name_root_ok = systerel_name("S2OPC PathLen PKI Test Root OK")
    name_int_ok = systerel_name("S2OPC PathLen PKI Test Intermediate OK")
    name_leaf_ok = systerel_name("S2OPC PathLen PKI Test Leaf OK")

    key_root_ok = generate_rsa_key()
    key_int_ok = generate_rsa_key()
    key_leaf_ok = generate_rsa_key()

    root_ok = build_ca(key_root_ok, name_root_ok, 0xA001, *VALID_PERIOD_CA, path_length=1)
    int_ok = build_intermediate_ca(
        key_int_ok, key_root_ok, root_ok, name_int_ok, 0xA002, *VALID_PERIOD_CA, path_length=None
    )
    leaf_ok = build_leaf(key_leaf_ok, key_int_ok, int_ok, name_leaf_ok, 0xA003, *VALID_PERIOD_LEAF)

    gen.write_trusted_cert("root_pathlen1.der", root_ok)
    gen.write_issuer_cert("ca_int_ok.der", int_ok)
    gen.write_leaf("leaf_ok.der", leaf_ok)

    # PL-NOK (root pathLen=1): second intermediate under ca_int_ok, leaf.
    name_int2_ok = systerel_name("S2OPC PathLen PKI Test Intermediate OK-2")
    name_leaf_pl1 = systerel_name("S2OPC PathLen PKI Test Leaf pathLen1 two int")

    key_int2_ok = generate_rsa_key()
    key_leaf_pl1 = generate_rsa_key()

    int2_ok = build_intermediate_ca(
        key_int2_ok, key_int_ok, int_ok, name_int2_ok, 0xA004, *VALID_PERIOD_CA, path_length=None
    )
    leaf_pl1 = build_leaf(key_leaf_pl1, key_int2_ok, int2_ok, name_leaf_pl1, 0xA005, *VALID_PERIOD_LEAF)

    gen.write_issuer_cert("ca_int2_ok.der", int2_ok)
    gen.write_leaf("leaf_nok_pathlen1.der", leaf_pl1)

    # PL-NOK (root pathLen=0): one intermediate and leaf.
    name_root_nok = systerel_name("S2OPC PathLen PKI Test Root pathLen0")
    name_int_nok = systerel_name("S2OPC PathLen PKI Test Intermediate pathLen0")
    name_leaf_nok = systerel_name("S2OPC PathLen PKI Test Leaf pathLen0")
    name_leaf_ok_pl0 = systerel_name("S2OPC PathLen PKI Test Leaf pathLen0 OK")

    key_root_nok = generate_rsa_key()
    key_int_nok = generate_rsa_key()
    key_leaf_nok = generate_rsa_key()
    key_leaf_ok_pl0 = generate_rsa_key()

    root_nok = build_ca(key_root_nok, name_root_nok, 0xA011, *VALID_PERIOD_CA, path_length=0)
    int_nok = build_intermediate_ca(
        key_int_nok, key_root_nok, root_nok, name_int_nok, 0xA012, *VALID_PERIOD_CA, path_length=None
    )
    leaf_nok = build_leaf(key_leaf_nok, key_int_nok, int_nok, name_leaf_nok, 0xA013, *VALID_PERIOD_LEAF)
    leaf_ok_pl0 = build_leaf(
        key_leaf_ok_pl0, key_root_nok, root_nok, name_leaf_ok_pl0, 0xA014, *VALID_PERIOD_LEAF
    )

    gen.write_trusted_cert("root_pathlen0.der", root_nok)
    gen.write_issuer_cert("ca_int_from_root_pathlen0.der", int_nok)
    gen.write_leaf("leaf_nok_pathlen0.der", leaf_nok)
    gen.write_leaf("leaf_ok_pathlen0.der", leaf_ok_pl0)

    # PL-NOK (intermediate pathLen=0): root without limit, two intermediates, leaf.
    name_root_int = systerel_name("S2OPC PathLen PKI Test Root Int")
    name_int_pathlen0 = systerel_name("S2OPC PathLen PKI Test Intermediate int pathLen0")
    name_int_from_int = systerel_name("S2OPC PathLen PKI Test Intermediate from int pathLen0")
    name_leaf_int = systerel_name("S2OPC PathLen PKI Test Leaf int pathLen0")

    key_root_int = generate_rsa_key()
    key_int_pathlen0 = generate_rsa_key()
    key_int_from_int = generate_rsa_key()
    key_leaf_int = generate_rsa_key()

    root_int = build_ca(key_root_int, name_root_int, 0xA021, *VALID_PERIOD_CA, path_length=None)
    int_pathlen0 = build_intermediate_ca(
        key_int_pathlen0, key_root_int, root_int, name_int_pathlen0, 0xA022, *VALID_PERIOD_CA, path_length=0
    )
    int_from_int = build_intermediate_ca(
        key_int_from_int, key_int_pathlen0, int_pathlen0, name_int_from_int, 0xA023, *VALID_PERIOD_CA, path_length=None
    )
    leaf_int = build_leaf(key_leaf_int, key_int_from_int, int_from_int, name_leaf_int, 0xA024, *VALID_PERIOD_LEAF)

    gen.write_trusted_cert("root_int.der", root_int)
    gen.write_issuer_cert("ca_int_pathLen0.der", int_pathlen0)
    gen.write_issuer_cert("ca_int_from_int_pathLen0.der", int_from_int)
    gen.write_leaf("leaf_nok_int_pathlen0.der", leaf_int)

    print(f"Generated DER files in {TEST_DIR}")


if __name__ == "__main__":
    main()
