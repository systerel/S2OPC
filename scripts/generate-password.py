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
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.kdf.pbkdf2 import PBKDF2HMAC
from binascii import hexlify, b2a_base64

PBKDF2 = "PBKDF2"
LIST_OF_SUPPORTED_ALGO = {PBKDF2: "PBKDF2 with HMAC-SHA-256"}

class PBKDF2_HMAC_SHA256:
    def __init__(self, pwd, salt, iter, outLen=32, base64=False):
        self.pwd = pwd
        self.salt = salt
        self.iter = iter
        self.outLen = outLen
        self.out = None
        self.base64 = base64
        self._set_args()

    def _set_args(self):
        self.pwd = str.encode(self.pwd)          # password as bytes object
        if not self.salt:
            self.salt = os.urandom(16)
        else:
            self.salt = str.encode(self.salt) # salt as bytes
    
    def _check_args(self):
        if len(self.pwd) > 64:
            print("\ngenerate-password: password must not exceed 64 bytes because if the HMAC key is longer \
                    than the blocks in the hash function (SH256 => 512 bits blocks size) then the password \
                    is hashed beforehand, which can reduce the entropy of the derived output.\n")
        if self.outLen < 0:
            print("\ngenerate-password: hashlen must be positive value\n")
            return False
        if (self.outLen % 32) != 0:
            print("\ngenerate-password: haslen must be a multiple of the digest size (32 bytes).\n")
            return False
        return True

    
    def _print(self):
        if not self.out:
            print("\ngenerate-password: Unexpected error\n")
            return
        if not self.base64:
            print("\n\n*************************\n")
            print("PBKDF2 with HMAC-SHA-256\n")
            print("-iter[in]: {}".format(self.iter))
            print("-salt[in]: {} ({} bytes)".format(hexlify(self.salt), len(self.salt)))
            print("-password[in]: {} ({} bytes)".format(hexlify(self.pwd), len(self.pwd)))
            print("-hash[out]: {} ({} bytes)".format(hexlify(self.out), len(self.out)))
            print("\n*************************\n\n")
        else:
            print("\n\n*************************\n")
            print("PBKDF2 with HMAC-SHA-256\n")
            print("-iter[in]: {}".format(self.iter))
            print("-salt[in]: {} ({} bytes)".format(b2a_base64(self.salt, newline=False), len(self.salt)))
            print("-password[in]: {} ({} bytes)".format(b2a_base64(self.pwd, newline=False), len(self.pwd)))
            print("-hash[out]: {} ({} bytes)".format(b2a_base64(self.out, newline=False), len(self.out)))
            print("\n*************************\n\n")

    def run(self):
        if self._check_args():
            kdf = PBKDF2HMAC(algorithm=hashes.SHA256(), length=self.outLen, salt=self.salt, iterations=self.iter)
            self.out = kdf.derive(self.pwd)
            self._print()
    

class CryptoUser:
    def __init__(self, args):
        self.args = args
        self._algo_class = None

    def _print_supported_algo(self):
        print("\nSupported algorithms:")
        for algo, description in LIST_OF_SUPPORTED_ALGO.items():
            print("-{} ({})".format(algo, description))
        print("\n")

    def check_arguments(self):
        if self.args.list:
            self._print_supported_algo()
            return False
        if self.args.algo not in LIST_OF_SUPPORTED_ALGO:
            print("\ngenerate-password: Unrecognized algo '{}'".format(self.args.algo))
            print("generate-password: Use --help for summary\n")
            return False
        return True
    
    def run_algo(self):
        if self.args.algo == PBKDF2:
            self._algo_class = PBKDF2_HMAC_SHA256(self.args.usr_pwd, self.args.salt, self.args.iter, self.args.hashlen, self.args.base64)
            self._algo_class.run()



def main():

    argparser = argparse.ArgumentParser(description='Generate password from a secret')
    argparser.add_argument('usr_pwd', type=str, help='Input user password (ascii encoding)')
    argparser.add_argument('--salt', type=str, help='Desired user salt (ascii encoding, default 128 bits random value)')
    argparser.add_argument('--iter', type=int, default=1000000, help='Desired iteration count (default 1000000)')
    argparser.add_argument('--hashlen', type=int, default=32, help='Desired output len  (octets, default 32)')
    argparser.add_argument('--algo', type=str, default="PBKDF2", help='Desired algorithm (dafault PBKDF2)')
    argparser.add_argument("-l", "--list", help="Display the list of supported algorithms with their descriptions", action='store_true')
    argparser.add_argument("-b64", "--base64", help='Display output and salt with base64 encoding (dafault hex encoding)', action='store_true')
    args = argparser.parse_args()

    crypto_user = CryptoUser(args)
    if crypto_user.check_arguments():
        crypto_user.run_algo()

if __name__ == '__main__':
    main()