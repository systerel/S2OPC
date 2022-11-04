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

import argparse
import os
import getpass
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.kdf.pbkdf2 import PBKDF2HMAC
from binascii import hexlify, unhexlify, b2a_base64, a2b_base64

SHA256_DIGEST_SIZE_BYTES = 32
SHA256_BLOCK_SIZE_BYTES = 64
PBKDF2 = "PBKDF2"
SUPPORTED_ALGO = {PBKDF2: "PBKDF2 with HMAC-SHA-256"}

def print_supported_algo():
    print("\nSupported algorithms:")
    for algo, description in SUPPORTED_ALGO.items():
        print("-{} ({})".format(algo, description))
    print("\n")

def pbkdf2_print_result(pwd, salt, iter, result, base64_format=False):
    print("\n\n*************************\n")
    print("PBKDF2 with HMAC-SHA-256\n")
    print("-iter[in]: {}".format(iter))
    if base64_format:
        print("-salt[in]: {} ({} bytes)".format(b2a_base64(salt, newline=False), len(salt)))
        print("-hash[out]: {} ({} bytes)".format(b2a_base64(result, newline=False), len(result)))
        print("\n*************************\n\n")
    else:
        print("-salt[in]: {} ({} bytes)".format(hexlify(salt), len(salt)))
        print("-hash[out]: {} ({} bytes)".format(hexlify(result), len(result)))
        print("\n*************************\n\n")

def main():

    argparser = argparse.ArgumentParser(description='This tool Generate a hash from a password, the output hash is useful to prepare the XML user \
                                                    configuration file for server. Password is asking interactively.')
    argparser.add_argument('--salt', type=str, help='Desired user salt (hexadecimal encoding, default 128 bits random value)')
    argparser.add_argument('--iter', type=int, default=1000000, help='Desired iteration count (default 1000000)')
    argparser.add_argument('--hashlen', type=int, default=SHA256_DIGEST_SIZE_BYTES, help='Desired output len  (default 32 bytes)')
    argparser.add_argument('--algo', type=str, default=PBKDF2, help='Desired algorithm (dafault PBKDF2)')
    argparser.add_argument("-l", "--list", help="Display the list of supported algorithms with their descriptions", action='store_true')
    argparser.add_argument("-b", "--base64", help='Input salt in base64 encoding format as well as the result display (dafault hexadecimal encoding)', action='store_true')
    args = argparser.parse_args()

    # Check if it is necessary to print the supported algorithm.
    if args.list:
        print_supported_algo()
        return

    # Check it the user algorithm is supported by the tool
    if args.algo not in SUPPORTED_ALGO:
        print("\ngenerate-password: Unrecognized algo '{}'".format(args.algo))
        print("generate-password: Use --help for summary\n")
        return
    
    # Get the right algorithm
    if args.algo == PBKDF2:
        # Check hashLen argument
        if (args.hashlen % SHA256_DIGEST_SIZE_BYTES) != 0:
            print("\ngenerate-password: hashlen must be a multiple of the digest size ({} bytes).\n".format(SHA256_DIGEST_SIZE_BYTES))
            return
        # Retrieve the password/salt and encode them as byte objects.
        try:
            # get password form interactive console
            pwd_tmp = getpass.getpass()
            pwd = getpass.getpass(prompt='Confirm your password:')
        except:
            print('\ngenerate-password: RUN_INTERACTIVELY environement variable is missing\n')
            return
        if pwd_tmp != pwd:
            print('\ngenerate-password: ERROR: not the same password\n')
            return
        # from ascii string to byte object
        pwd = str.encode(pwd)
        if len(pwd) > SHA256_BLOCK_SIZE_BYTES:
            print("\ngenerate-password: password should not exceed 64 bytes because if the HMAC key is longer \
                    than the blocks in the hash function (SH256 => 512 bits blocks size) then the password \
                    is hashed beforehand, which can reduce the entropy of the derived output.\n")

        if not args.salt:
            # If user doesn't defined the salt then generate it randomly
            salt = os.urandom(16)
        else:
            if args.base64:
                # User salt in base64 format
                try:
                    # from ascii base64 string to byte object
                    salt = a2b_base64(args.salt)
                except:
                    print('\ngenerate-password: Non-base64 digit found for the salt\n')
                    return
            else:
                # User salt in hexadecimal format
                try:
                    # from ascii hex to byte object
                    salt = unhexlify(args.salt)
                except:
                    print('\ngenerate-password: Non-hexadecimal digit found for the salt\n')
                    return

        # Run PBKDF2
        kdf = PBKDF2HMAC(algorithm=hashes.SHA256(), length=args.hashlen, salt=salt, iterations=args.iter)
        result = kdf.derive(pwd)
        # Print result 
        pbkdf2_print_result(pwd, salt, args.iter, result, args.base64)

if __name__ == '__main__':
    main()