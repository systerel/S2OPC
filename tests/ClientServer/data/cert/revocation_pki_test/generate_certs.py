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

"""Generate revocation_pki_test DER files with fixed validity dates."""

import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "scripts"))

from pki_gen import (
    CaSpec,
    ChainLeafSpec,
    CrlSpec,
    IntermediateCaSpec,
    LeafSpec,
    PkiStoreGenerator,
    RootCaSpec,
    VALID_PERIOD_CA,
    VALID_PERIOD_CRL,
    VALID_PERIOD_LEAF,
    build_ca,
    build_crl,
    build_intermediate_ca,
    build_leaf,
    generate_rsa_key,
    systerel_name,
)

TEST_DIR = Path(__file__).resolve().parent


def ca_name(case_id):
    return systerel_name(f"S2OPC Revocation PKI Test CA case{case_id}")


def leaf_name(case_id, label=""):
    common_name = f"S2OPC Revocation PKI Test Leaf case{case_id}"
    if label:
        common_name = f"{common_name} {label}"
    return systerel_name(common_name)


def int_root_name(case_id):
    return systerel_name(f"S2OPC Revocation PKI Test Root CA int case{case_id}")


def int_inter_name(case_id):
    return systerel_name(f"S2OPC Revocation PKI Test Intermediate CA int case{case_id}")


def int_leaf_name(case_id, label=""):
    common_name = f"S2OPC Revocation PKI Test Leaf int case{case_id}"
    if label:
        common_name = f"{common_name} {label}"
    return systerel_name(common_name)


def main():
    gen = PkiStoreGenerator(TEST_DIR)

    # Case 1: valid CA + valid CRL listing one leaf serial => revoked leaf rejected.
    case1_key = generate_rsa_key()
    case1_revoked_leaf_key = generate_rsa_key()
    case1_valid_leaf_key = generate_rsa_key()
    leaf1_revoked_serial = 0x0101
    leaf1_valid_serial = 0x0102
    case1_ca = build_ca(case1_key, ca_name(1), 0x0100, *VALID_PERIOD_CA)
    gen.write_trusted_cert("ca_case1_valid.der", case1_ca)
    gen.write_crl(
        "ca_case1_crl_revoked.der",
        build_crl(case1_key, case1_ca, *VALID_PERIOD_CRL, [leaf1_revoked_serial]),
    )
    gen.write_leaf(
        "leaf_case1_revoked.der",
        build_leaf(
            case1_revoked_leaf_key,
            case1_key,
            case1_ca,
            leaf_name(1, "revoked"),
            leaf1_revoked_serial,
            VALID_PERIOD_LEAF[0],
            VALID_PERIOD_LEAF[1],
        ),
    )
    gen.write_leaf(
        "leaf_case1_valid.der",
        build_leaf(
            case1_valid_leaf_key,
            case1_key,
            case1_ca,
            leaf_name(1, "valid"),
            leaf1_valid_serial,
            VALID_PERIOD_LEAF[0],
            VALID_PERIOD_LEAF[1],
        ),
    )

    # Case 2: valid CA + empty CRL => leaf accepted (control case).
    gen.generate_case(
        generate_rsa_key(),
        generate_rsa_key(),
        [
            CaSpec("ca_case2_valid.der", 0x0200, VALID_PERIOD_CA[0], VALID_PERIOD_CA[1], ca_name(2)),
        ],
        [
            CrlSpec("ca_case2_crl_valid.der", "ca_case2_valid.der", VALID_PERIOD_CRL[0], VALID_PERIOD_CRL[1], []),
        ],
        LeafSpec(
            "leaf_case2.der",
            "ca_case2_valid.der",
            0x0201,
            VALID_PERIOD_LEAF[0],
            VALID_PERIOD_LEAF[1],
            leaf_name(2),
        ),
    )

    # int-1: root -> intermediate -> revoked/valid leaves (CRL on intermediate lists revoked leaf serial).
    int1_root_key = generate_rsa_key()
    int1_int_key = generate_rsa_key()
    int1_revoked_leaf_key = generate_rsa_key()
    int1_valid_leaf_key = generate_rsa_key()
    leaf_int1_revoked_serial = 0x8302
    leaf_int1_valid_serial = 0x8303
    int1_root = build_ca(int1_root_key, int_root_name(1), 0x8300, *VALID_PERIOD_CA)
    gen.write_trusted_cert("ca_int_case1_root.der", int1_root)
    int1_inter = build_intermediate_ca(
        int1_int_key, int1_root_key, int1_root, int_inter_name(1), 0x8301, *VALID_PERIOD_CA
    )
    gen.write_trusted_cert("ca_int_case1_inter.der", int1_inter)
    gen.write_crl(
        "ca_int_case1_root_crl.der",
        build_crl(int1_root_key, int1_root, *VALID_PERIOD_CRL, []),
    )
    gen.write_crl(
        "ca_int_case1_inter_crl.der",
        build_crl(int1_int_key, int1_inter, *VALID_PERIOD_CRL, [leaf_int1_revoked_serial]),
    )
    gen.write_leaf(
        "leaf_int_case1_revoked.der",
        build_leaf(
            int1_revoked_leaf_key,
            int1_int_key,
            int1_inter,
            int_leaf_name(1, "revoked"),
            leaf_int1_revoked_serial,
            VALID_PERIOD_LEAF[0],
            VALID_PERIOD_LEAF[1],
        ),
    )
    gen.write_leaf(
        "leaf_int_case1_valid.der",
        build_leaf(
            int1_valid_leaf_key,
            int1_int_key,
            int1_inter,
            int_leaf_name(1, "valid"),
            leaf_int1_valid_serial,
            VALID_PERIOD_LEAF[0],
            VALID_PERIOD_LEAF[1],
        ),
    )

    # int-3: root CRL revokes the intermediate CA => leaf rejected.
    inter_int3_serial = 0x8331
    gen.generate_chain_case(
        generate_rsa_key(),
        generate_rsa_key(),
        generate_rsa_key(),
        RootCaSpec("ca_int_case3_root.der", 0x8330, VALID_PERIOD_CA[0], VALID_PERIOD_CA[1], int_root_name(3)),
        [IntermediateCaSpec("ca_int_case3_inter.der", inter_int3_serial, VALID_PERIOD_CA[0], VALID_PERIOD_CA[1], int_inter_name(3))],
        [
            CrlSpec(
                "ca_int_case3_root_crl.der",
                "ca_int_case3_root.der",
                VALID_PERIOD_CRL[0],
                VALID_PERIOD_CRL[1],
                [inter_int3_serial],
            ),
            CrlSpec("ca_int_case3_inter_crl.der", "ca_int_case3_inter.der", VALID_PERIOD_CRL[0], VALID_PERIOD_CRL[1], []),
        ],
        ChainLeafSpec(
            "leaf_int_case3.der",
            "ca_int_case3_inter.der",
            0x8332,
            VALID_PERIOD_LEAF[0],
            VALID_PERIOD_LEAF[1],
            int_leaf_name(3),
        ),
    )

    print(f"Generated DER files in {TEST_DIR}")


if __name__ == "__main__":
    main()
