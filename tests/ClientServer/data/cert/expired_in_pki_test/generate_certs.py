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

"""Generate expired_in_pki_test DER files with fixed validity dates."""

import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "scripts"))

from pki_gen import (
    CaSpec,
    ChainLeafSpec,
    CrlSpec,
    EXPIRED_PERIOD_CA,
    EXPIRED_PERIOD_CRL,
    EXPIRED_PERIOD_LEAF,
    IntermediateCaSpec,
    LeafSpec,
    PkiStoreGenerator,
    RootCaSpec,
    VALID_PERIOD_CA,
    VALID_PERIOD_CRL,
    VALID_PERIOD_LEAF,
    build_selfsigned,
    generate_rsa_key,
    systerel_name,
)

TEST_DIR = Path(__file__).resolve().parent


def ca_name(case_id):
    return systerel_name(f"S2OPC Expired-in-PKI Test CA case{case_id}")


def leaf_name(case_id):
    return systerel_name(f"S2OPC Expired-in-PKI Test Leaf case{case_id}")


def int_root_name(case_id):
    return systerel_name(f"S2OPC Expired-in-PKI Test Root CA int case{case_id}")


def int_inter_name(case_id, label=""):
    common_name = f"S2OPC Expired-in-PKI Test Intermediate CA int case{case_id}"
    if label:
        common_name = f"{common_name} {label}"
    return systerel_name(common_name)


def int_leaf_name(case_id):
    return systerel_name(f"S2OPC Expired-in-PKI Test Leaf int case{case_id}")


def selfsigned_name():
    return systerel_name("S2OPC Expired-in-PKI Test Self-signed Expired")


def main():
    gen = PkiStoreGenerator(TEST_DIR)

    # Case 1: one valid trusted CA (+ CRL) => leaf accepted.
    gen.generate_case(
        generate_rsa_key(),
        generate_rsa_key(),
        [CaSpec("ca_case1_valid.der", 0x0101, VALID_PERIOD_CA[0], VALID_PERIOD_CA[1], ca_name(1))],
        [CrlSpec("ca_case1_crl_valid.der", "ca_case1_valid.der", VALID_PERIOD_CRL[0], VALID_PERIOD_CRL[1], [])],
        LeafSpec("leaf_case1.der", "ca_case1_valid.der", 0x1101, VALID_PERIOD_LEAF[0], VALID_PERIOD_LEAF[1], leaf_name(1)),
    )

    # Case 2: one expired trusted CA (+ CRL) => leaf rejected.
    gen.generate_case(
        generate_rsa_key(),
        generate_rsa_key(),
        [CaSpec("ca_case2_expired.der", 0x0201, EXPIRED_PERIOD_CA[0], EXPIRED_PERIOD_CA[1], ca_name(2))],
        [CrlSpec("ca_case2_crl_valid.der", "ca_case2_expired.der", VALID_PERIOD_CRL[0], VALID_PERIOD_CRL[1], [])],
        LeafSpec("leaf_case2.der", "ca_case2_expired.der", 0x1201, VALID_PERIOD_LEAF[0], VALID_PERIOD_LEAF[1], leaf_name(2)),
    )

    # Case 3: expired + valid CA with same subject (+ CRL) => leaf accepted.
    gen.generate_case(
        generate_rsa_key(),
        generate_rsa_key(),
        [
            CaSpec("ca_case3_expired.der", 0x0301, EXPIRED_PERIOD_CA[0], EXPIRED_PERIOD_CA[1], ca_name(3)),
            CaSpec("ca_case3_valid.der", 0x0302, VALID_PERIOD_CA[0], VALID_PERIOD_CA[1], ca_name(3)),
        ],
        [CrlSpec("ca_case3_crl_valid.der", "ca_case3_valid.der", VALID_PERIOD_CRL[0], VALID_PERIOD_CRL[1], [])],
        LeafSpec("leaf_case3.der", "ca_case3_valid.der", 0x1301, VALID_PERIOD_LEAF[0], VALID_PERIOD_LEAF[1], leaf_name(3)),
    )

    # Case 4: valid CA + expired CRL + valid CRL => leaf rejected.
    gen.generate_case(
        generate_rsa_key(),
        generate_rsa_key(),
        [CaSpec("ca_case4_valid.der", 0x0401, VALID_PERIOD_CA[0], VALID_PERIOD_CA[1], ca_name(4))],
        [
            CrlSpec("ca_case4_crl_expired.der", "ca_case4_valid.der", EXPIRED_PERIOD_CRL[0], EXPIRED_PERIOD_CRL[1], []),
            CrlSpec("ca_case4_crl_valid.der", "ca_case4_valid.der", VALID_PERIOD_CRL[0], VALID_PERIOD_CRL[1], []),
        ],
        LeafSpec("leaf_case4.der", "ca_case4_valid.der", 0x1401, VALID_PERIOD_LEAF[0], VALID_PERIOD_LEAF[1], leaf_name(4)),
    )

    # Case 5: valid CA + expired revoked CRL + valid CRL => leaf rejected.
    leaf5_serial = 0x1501
    gen.generate_case(
        generate_rsa_key(),
        generate_rsa_key(),
        [CaSpec("ca_case5_valid.der", 0x0501, VALID_PERIOD_CA[0], VALID_PERIOD_CA[1], ca_name(5))],
        [
            CrlSpec(
                "ca_case5_crl_expired_revoked.der",
                "ca_case5_valid.der",
                EXPIRED_PERIOD_CRL[0],
                EXPIRED_PERIOD_CRL[1],
                [leaf5_serial],
            ),
            CrlSpec("ca_case5_crl_valid.der", "ca_case5_valid.der", VALID_PERIOD_CRL[0], VALID_PERIOD_CRL[1], []),
        ],
        LeafSpec("leaf_case5.der", "ca_case5_valid.der", leaf5_serial, VALID_PERIOD_LEAF[0], VALID_PERIOD_LEAF[1], leaf_name(5)),
    )

    # Case 7: valid CA (+ CRL), expired leaf => leaf rejected (TimeInvalid).
    gen.generate_case(
        generate_rsa_key(),
        generate_rsa_key(),
        [CaSpec("ca_case7_valid.der", 0x0701, VALID_PERIOD_CA[0], VALID_PERIOD_CA[1], ca_name(7))],
        [CrlSpec("ca_case7_crl_valid.der", "ca_case7_valid.der", VALID_PERIOD_CRL[0], VALID_PERIOD_CRL[1], [])],
        LeafSpec(
            "leaf_expired_case7.der",
            "ca_case7_valid.der",
            0x1701,
            EXPIRED_PERIOD_LEAF[0],
            EXPIRED_PERIOD_LEAF[1],
            leaf_name(7),
        ),
    )

    # Case 6: expired self-signed trusted cert => TimeInvalid.
    selfsigned_key = generate_rsa_key()
    selfsigned_expired = build_selfsigned(selfsigned_key, selfsigned_name(), 0x0601, *EXPIRED_PERIOD_CA)
    gen.write_trusted_cert("ca_selfsigned_expired.der", selfsigned_expired)

    # Intermediate CA chain cases (int-1 .. int-4b).
    # int-1: valid root + valid trusted intermediate => leaf accepted.
    gen.generate_chain_case(
        generate_rsa_key(),
        generate_rsa_key(),
        generate_rsa_key(),
        RootCaSpec("ca_int_case1_root.der", 0x8100, VALID_PERIOD_CA[0], VALID_PERIOD_CA[1], int_root_name(1)),
        [IntermediateCaSpec("ca_int_case1_inter.der", 0x8101, VALID_PERIOD_CA[0], VALID_PERIOD_CA[1], int_inter_name(1))],
        [
            CrlSpec("ca_int_case1_root_crl.der", "ca_int_case1_root.der", VALID_PERIOD_CRL[0], VALID_PERIOD_CRL[1], []),
            CrlSpec("ca_int_case1_inter_crl.der", "ca_int_case1_inter.der", VALID_PERIOD_CRL[0], VALID_PERIOD_CRL[1], []),
        ],
        ChainLeafSpec("leaf_int_case1.der", "ca_int_case1_inter.der", 0x8102, VALID_PERIOD_LEAF[0], VALID_PERIOD_LEAF[1], int_leaf_name(1)),
    )

    # int-1b: valid root trusted, valid intermediate in issuers => leaf accepted.
    gen.generate_chain_case(
        generate_rsa_key(),
        generate_rsa_key(),
        generate_rsa_key(),
        RootCaSpec("ca_int_case1b_root.der", 0x8110, VALID_PERIOD_CA[0], VALID_PERIOD_CA[1], int_root_name("1b")),
        [
            IntermediateCaSpec(
                "ca_int_case1b_inter.der",
                0x8111,
                VALID_PERIOD_CA[0],
                VALID_PERIOD_CA[1],
                int_inter_name("1b"),
                trusted=False,
            )
        ],
        [
            CrlSpec("ca_int_case1b_root_crl.der", "ca_int_case1b_root.der", VALID_PERIOD_CRL[0], VALID_PERIOD_CRL[1], []),
            CrlSpec(
                "ca_int_case1b_inter_crl.der",
                "ca_int_case1b_inter.der",
                VALID_PERIOD_CRL[0],
                VALID_PERIOD_CRL[1],
                [],
                trusted=False,
            ),
        ],
        ChainLeafSpec(
            "leaf_int_case1b.der",
            "ca_int_case1b_inter.der",
            0x8112,
            VALID_PERIOD_LEAF[0],
            VALID_PERIOD_LEAF[1],
            int_leaf_name("1b"),
        ),
    )

    # int-2: valid root + expired trusted intermediate => leaf rejected (TimeInvalid).
    gen.generate_chain_case(
        generate_rsa_key(),
        generate_rsa_key(),
        generate_rsa_key(),
        RootCaSpec("ca_int_case2_root.der", 0x8120, VALID_PERIOD_CA[0], VALID_PERIOD_CA[1], int_root_name(2)),
        [
            IntermediateCaSpec(
                "ca_int_case2_inter.der", 0x8121, EXPIRED_PERIOD_CA[0], EXPIRED_PERIOD_CA[1], int_inter_name(2)
            )
        ],
        [
            CrlSpec("ca_int_case2_root_crl.der", "ca_int_case2_root.der", VALID_PERIOD_CRL[0], VALID_PERIOD_CRL[1], []),
            CrlSpec("ca_int_case2_inter_crl.der", "ca_int_case2_inter.der", VALID_PERIOD_CRL[0], VALID_PERIOD_CRL[1], []),
        ],
        ChainLeafSpec("leaf_int_case2.der", "ca_int_case2_inter.der", 0x8122, VALID_PERIOD_LEAF[0], VALID_PERIOD_LEAF[1], int_leaf_name(2)),
    )

    # int-2b: expired root + valid trusted intermediate => leaf rejected.
    gen.generate_chain_case(
        generate_rsa_key(),
        generate_rsa_key(),
        generate_rsa_key(),
        RootCaSpec("ca_int_case2b_root.der", 0x8130, EXPIRED_PERIOD_CA[0], EXPIRED_PERIOD_CA[1], int_root_name("2b")),
        [IntermediateCaSpec("ca_int_case2b_inter.der", 0x8131, VALID_PERIOD_CA[0], VALID_PERIOD_CA[1], int_inter_name("2b"))],
        [
            CrlSpec("ca_int_case2b_root_crl.der", "ca_int_case2b_root.der", VALID_PERIOD_CRL[0], VALID_PERIOD_CRL[1], []),
            CrlSpec("ca_int_case2b_inter_crl.der", "ca_int_case2b_inter.der", VALID_PERIOD_CRL[0], VALID_PERIOD_CRL[1], []),
        ],
        ChainLeafSpec(
            "leaf_int_case2b.der",
            "ca_int_case2b_inter.der",
            0x8132,
            VALID_PERIOD_LEAF[0],
            VALID_PERIOD_LEAF[1],
            int_leaf_name("2b"),
        ),
    )

    # int-3: expired + valid intermediate (same subject) => leaf accepted.
    gen.generate_chain_case(
        generate_rsa_key(),
        generate_rsa_key(),
        generate_rsa_key(),
        RootCaSpec("ca_int_case3_root.der", 0x8140, VALID_PERIOD_CA[0], VALID_PERIOD_CA[1], int_root_name(3)),
        [
            IntermediateCaSpec(
                "ca_int_case3_inter_expired.der",
                0x8141,
                EXPIRED_PERIOD_CA[0],
                EXPIRED_PERIOD_CA[1],
                int_inter_name(3),
            ),
            IntermediateCaSpec(
                "ca_int_case3_inter_valid.der", 0x8142, VALID_PERIOD_CA[0], VALID_PERIOD_CA[1], int_inter_name(3)
            ),
        ],
        [
            CrlSpec("ca_int_case3_root_crl.der", "ca_int_case3_root.der", VALID_PERIOD_CRL[0], VALID_PERIOD_CRL[1], []),
            CrlSpec(
                "ca_int_case3_inter_crl.der",
                "ca_int_case3_inter_valid.der",
                VALID_PERIOD_CRL[0],
                VALID_PERIOD_CRL[1],
                [],
            ),
        ],
        ChainLeafSpec(
            "leaf_int_case3.der",
            "ca_int_case3_inter_valid.der",
            0x8143,
            VALID_PERIOD_LEAF[0],
            VALID_PERIOD_LEAF[1],
            int_leaf_name(3),
        ),
    )

    # int-4: valid root + intermediate, expired intermediate CRL + valid intermediate CRL => RevocationUnknown.
    gen.generate_chain_case(
        generate_rsa_key(),
        generate_rsa_key(),
        generate_rsa_key(),
        RootCaSpec("ca_int_case4_root.der", 0x8150, VALID_PERIOD_CA[0], VALID_PERIOD_CA[1], int_root_name(4)),
        [IntermediateCaSpec("ca_int_case4_inter.der", 0x8151, VALID_PERIOD_CA[0], VALID_PERIOD_CA[1], int_inter_name(4))],
        [
            CrlSpec("ca_int_case4_root_crl.der", "ca_int_case4_root.der", VALID_PERIOD_CRL[0], VALID_PERIOD_CRL[1], []),
            CrlSpec(
                "ca_int_case4_inter_crl_expired.der",
                "ca_int_case4_inter.der",
                EXPIRED_PERIOD_CRL[0],
                EXPIRED_PERIOD_CRL[1],
                [],
            ),
            CrlSpec(
                "ca_int_case4_inter_crl_valid.der",
                "ca_int_case4_inter.der",
                VALID_PERIOD_CRL[0],
                VALID_PERIOD_CRL[1],
                [],
            ),
        ],
        ChainLeafSpec("leaf_int_case4.der", "ca_int_case4_inter.der", 0x8152, VALID_PERIOD_LEAF[0], VALID_PERIOD_LEAF[1], int_leaf_name(4)),
    )

    # int-4b: valid root + intermediate, expired root CRL + valid root CRL => RevocationUnknown.
    gen.generate_chain_case(
        generate_rsa_key(),
        generate_rsa_key(),
        generate_rsa_key(),
        RootCaSpec("ca_int_case4b_root.der", 0x8160, VALID_PERIOD_CA[0], VALID_PERIOD_CA[1], int_root_name("4b")),
        [IntermediateCaSpec("ca_int_case4b_inter.der", 0x8161, VALID_PERIOD_CA[0], VALID_PERIOD_CA[1], int_inter_name("4b"))],
        [
            CrlSpec(
                "ca_int_case4b_root_crl_expired.der",
                "ca_int_case4b_root.der",
                EXPIRED_PERIOD_CRL[0],
                EXPIRED_PERIOD_CRL[1],
                [],
            ),
            CrlSpec(
                "ca_int_case4b_root_crl_valid.der",
                "ca_int_case4b_root.der",
                VALID_PERIOD_CRL[0],
                VALID_PERIOD_CRL[1],
                [],
            ),
            CrlSpec("ca_int_case4b_inter_crl.der", "ca_int_case4b_inter.der", VALID_PERIOD_CRL[0], VALID_PERIOD_CRL[1], []),
        ],
        ChainLeafSpec(
            "leaf_int_case4b.der",
            "ca_int_case4b_inter.der",
            0x8162,
            VALID_PERIOD_LEAF[0],
            VALID_PERIOD_LEAF[1],
            int_leaf_name("4b"),
        ),
    )

    print(f"Generated DER files in {TEST_DIR}")


if __name__ == "__main__":
    main()
