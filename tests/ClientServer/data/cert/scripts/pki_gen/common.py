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

"""Shared helpers for PKI test certificate generation."""

from datetime import datetime, timezone
from pathlib import Path

from cryptography import x509
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.asymmetric import rsa
from cryptography.x509.oid import NameOID


def utc(year, month, day):
    return datetime(year, month, day, tzinfo=timezone.utc)


VALID_PERIOD_CA = (utc(2020, 1, 1), utc(2140, 1, 1))
EXPIRED_PERIOD_CA = (utc(2010, 1, 1), utc(2015, 1, 1))
VALID_PERIOD_LEAF = (utc(2022, 1, 1), utc(2132, 1, 1))
EXPIRED_PERIOD_LEAF = (utc(2010, 6, 1), utc(2015, 6, 1))
VALID_PERIOD_CRL = (utc(2024, 1, 1), utc(2134, 1, 1))
EXPIRED_PERIOD_CRL = (utc(2020, 1, 1), utc(2020, 2, 1))


def systerel_name(common_name):
    return x509.Name(
        [
            x509.NameAttribute(NameOID.COUNTRY_NAME, "FR"),
            x509.NameAttribute(NameOID.STATE_OR_PROVINCE_NAME, "France"),
            x509.NameAttribute(NameOID.LOCALITY_NAME, "Aix-en-Provence"),
            x509.NameAttribute(NameOID.ORGANIZATION_NAME, "Systerel"),
            x509.NameAttribute(NameOID.COMMON_NAME, common_name),
            x509.NameAttribute(NameOID.EMAIL_ADDRESS, "s2opc-support@systerel.fr"),
        ]
    )


def write_der(path, data):
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    if isinstance(data, (bytes, bytearray)):
        path.write_bytes(data)
    else:
        path.write_bytes(data.public_bytes(serialization.Encoding.DER))


def generate_rsa_key(key_size=2048):
    return rsa.generate_private_key(public_exponent=65537, key_size=key_size)
