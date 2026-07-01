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

"""Signature corruption helpers for signature_pki_test."""

from cryptography.hazmat.primitives import serialization


def corrupt_der_signature(data):
    if isinstance(data, (bytes, bytearray)):
        der = bytearray(data)
    else:
        der = bytearray(data.public_bytes(serialization.Encoding.DER))
    for index in range(len(der) - 1, max(len(der) - 128, 0), -1):
        if der[index] != 0:
            der[index] ^= 0xFF
            break
    return der
