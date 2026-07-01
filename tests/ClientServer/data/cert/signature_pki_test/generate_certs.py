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

"""Generate signature_pki_test DER files with fixed validity dates."""

import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "scripts"))

from pki_gen import (
    ChainLeafSpec,
    CrlSpec,
    IntermediateCaSpec,
    PkiStoreGenerator,
    RootCaSpec,
    VALID_PERIOD_CA,
    VALID_PERIOD_CRL,
    VALID_PERIOD_LEAF,
    build_ca,
    build_crl,
    build_leaf,
    build_selfsigned,
    generate_rsa_key,
    systerel_name,
)

TEST_DIR = Path(__file__).resolve().parent


def root_ca_name():
    return systerel_name("S2OPC Signature PKI Test Root CA")


def root_ca_name_case3():
    return systerel_name("S2OPC Signature PKI Test Root CA CRL Bad Sig")


def root_ca_name_case5():
    return systerel_name("S2OPC Signature PKI Test Root CA Leaf Bad Sig")


def root_ca_name_case6():
    return systerel_name("S2OPC Signature PKI Test Root CA No KeyCertSign")


def leaf_name():
    return systerel_name("S2OPC Signature PKI Test Leaf")


def leaf_name_case4():
    return systerel_name("S2OPC Signature PKI Test Leaf CRL Bad Sig")


def leaf_name_case5(label):
    return systerel_name(f"S2OPC Signature PKI Test Leaf case5 {label}")


def leaf_name_case6():
    return systerel_name("S2OPC Signature PKI Test Leaf case6 No KeyCertSign")


def int_root_name():
    return systerel_name("S2OPC Signature PKI Test Root CA int case1")


def int_inter_name():
    return systerel_name("S2OPC Signature PKI Test Intermediate CA int case1")


def int_leaf_name():
    return systerel_name("S2OPC Signature PKI Test Leaf int case1")


def main():
    gen = PkiStoreGenerator(TEST_DIR)

    # Case 1: self-signed trusted cert with corrupted signature => UseNotAllowed.
    selfsigned_key = generate_rsa_key()
    selfsigned_valid = build_selfsigned(
        selfsigned_key,
        systerel_name("S2OPC Signature PKI Test Self-signed Bad Sig"),
        0x02,
        *VALID_PERIOD_CA,
    )
    gen.write_trusted_cert_corrupted("ca_selfsigned_bad_sig.der", selfsigned_valid)

    # Case 2: root CA with corrupted signature + valid CRL => leaf rejected (Untrusted).
    root_key = generate_rsa_key()
    leaf_key = generate_rsa_key()
    root_valid = build_ca(root_key, root_ca_name(), 0x10, *VALID_PERIOD_CA)
    leaf = build_leaf(leaf_key, root_key, root_valid, leaf_name(), 0x20, *VALID_PERIOD_LEAF)
    root_crl = build_crl(root_key, root_valid, *VALID_PERIOD_CRL, [])

    gen.write_trusted_cert_corrupted("ca_root_bad_sig.der", root_valid)
    gen.write_crl("ca_root_crl_valid.der", root_crl)
    gen.write_leaf("leaf_case3.der", leaf)

    # Case 3: valid root CA + CRL with corrupted signature => leaf rejected (RevocationUnknown).
    root3_key = generate_rsa_key()
    leaf3_key = generate_rsa_key()
    root3_valid = build_ca(root3_key, root_ca_name_case3(), 0x30, *VALID_PERIOD_CA)
    root3_crl = build_crl(root3_key, root3_valid, *VALID_PERIOD_CRL, [])
    leaf4 = build_leaf(leaf3_key, root3_key, root3_valid, leaf_name_case4(), 0x40, *VALID_PERIOD_LEAF)

    gen.write_trusted_cert("ca_root_valid.der", root3_valid)
    gen.write_crl_corrupted("ca_root_crl_bad_sig.der", root3_crl)
    gen.write_leaf("leaf_case4.der", leaf4)

    # Case 5: valid root CA + valid CRL + leaf with corrupted signature => leaf rejected (Untrusted).
    root5_key = generate_rsa_key()
    leaf5_bad_key = generate_rsa_key()
    leaf5_valid_key = generate_rsa_key()
    root5_valid = build_ca(root5_key, root_ca_name_case5(), 0x50, *VALID_PERIOD_CA)
    root5_crl = build_crl(root5_key, root5_valid, *VALID_PERIOD_CRL, [])
    leaf5_bad = build_leaf(
        leaf5_bad_key,
        root5_key,
        root5_valid,
        leaf_name_case5("bad sig"),
        0x51,
        *VALID_PERIOD_LEAF,
    )
    leaf5_valid = build_leaf(
        leaf5_valid_key,
        root5_key,
        root5_valid,
        leaf_name_case5("valid"),
        0x52,
        *VALID_PERIOD_LEAF,
    )

    gen.write_trusted_cert("ca_case5_root_valid.der", root5_valid)
    gen.write_crl("ca_case5_crl_valid.der", root5_crl)
    gen.write_leaf_corrupted("leaf_case5_bad_sig.der", leaf5_bad)
    gen.write_leaf("leaf_case5_valid.der", leaf5_valid)

    # Case 6: root CA without keyCertSign + valid CRL + correctly signed leaf => leaf rejected.
    root6_key = generate_rsa_key()
    leaf6_key = generate_rsa_key()
    root6_no_ku = build_ca(
        root6_key,
        root_ca_name_case6(),
        0x60,
        *VALID_PERIOD_CA,
        key_cert_sign=False,
    )
    root6_crl = build_crl(root6_key, root6_no_ku, *VALID_PERIOD_CRL, [])
    leaf6 = build_leaf(leaf6_key, root6_key, root6_no_ku, leaf_name_case6(), 0x61, *VALID_PERIOD_LEAF)

    gen.write_trusted_cert("ca_case6_root_no_key_sign_usage.der", root6_no_ku)
    gen.write_crl("ca_case6_crl_valid.der", root6_crl)
    gen.write_leaf("leaf_case6.der", leaf6)

    # int-1: valid root + intermediate with corrupted signature (signed by root) => leaf rejected (Untrusted).
    gen.generate_chain_case(
        generate_rsa_key(),
        generate_rsa_key(),
        generate_rsa_key(),
        RootCaSpec("ca_int_case1_root.der", 0x8100, VALID_PERIOD_CA[0], VALID_PERIOD_CA[1], int_root_name()),
        [
            IntermediateCaSpec(
                "ca_int_case1_inter_bad_sig.der",
                0x8101,
                VALID_PERIOD_CA[0],
                VALID_PERIOD_CA[1],
                int_inter_name(),
                corrupt_signature=True,
            )
        ],
        [
            CrlSpec("ca_int_case1_root_crl.der", "ca_int_case1_root.der", VALID_PERIOD_CRL[0], VALID_PERIOD_CRL[1], []),
            CrlSpec("ca_int_case1_inter_crl.der", "ca_int_case1_inter_bad_sig.der", VALID_PERIOD_CRL[0], VALID_PERIOD_CRL[1], []),
        ],
        ChainLeafSpec(
            "leaf_int_case1.der",
            "ca_int_case1_inter_bad_sig.der",
            0x8102,
            VALID_PERIOD_LEAF[0],
            VALID_PERIOD_LEAF[1],
            int_leaf_name(),
        ),
    )

    print(f"Generated DER files in {TEST_DIR}")


if __name__ == "__main__":
    main()
