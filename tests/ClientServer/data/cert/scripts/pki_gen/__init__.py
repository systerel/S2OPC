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

"""Shared library for PKI test certificate generation scripts."""

from pki_gen.builders import build_ca, build_crl, build_intermediate_ca, build_leaf, build_selfsigned
from pki_gen.common import (
    EXPIRED_PERIOD_CA,
    EXPIRED_PERIOD_CRL,
    EXPIRED_PERIOD_LEAF,
    VALID_PERIOD_CA,
    VALID_PERIOD_CRL,
    VALID_PERIOD_LEAF,
    generate_rsa_key,
    systerel_name,
)
from pki_gen.generator import (
    CaSpec,
    ChainLeafSpec,
    CrlSpec,
    IntermediateCaSpec,
    LeafSpec,
    PkiStoreGenerator,
    RootCaSpec,
)

__all__ = [
    "CaSpec",
    "ChainLeafSpec",
    "CrlSpec",
    "EXPIRED_PERIOD_CA",
    "EXPIRED_PERIOD_CRL",
    "EXPIRED_PERIOD_LEAF",
    "IntermediateCaSpec",
    "LeafSpec",
    "PkiStoreGenerator",
    "RootCaSpec",
    "VALID_PERIOD_CA",
    "VALID_PERIOD_CRL",
    "VALID_PERIOD_LEAF",
    "build_ca",
    "build_crl",
    "build_intermediate_ca",
    "build_leaf",
    "build_selfsigned",
    "generate_rsa_key",
    "systerel_name",
]
