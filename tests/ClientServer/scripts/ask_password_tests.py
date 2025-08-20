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

import platform
if platform.system() == "Windows":
    # pexpect API for windows was tested without success
    import wexpect as pexpect
    BIN = "ask_password.exe"
else:
    import pexpect
    BIN = "ask_password"
import sys
import random
import string

PASSWORD_MAX_LENGTH = 128

def test_ask_password(expected, actual, success):
    child = pexpect.spawn('./%s "%s"' % (BIN, expected))
    child.expect('Password:')
    child.sendline(actual)
    res = child.wait() == 0
    if res != success:
        sys.stderr.write('./ask_password %s <<< %s : unexpected result: %s\n' % (expected, actual, res))
        return False
    return True

def get_random_pwd_of_length(pwd_length):
    return ''.join(random.choices(string.ascii_letters + string.digits + string.punctuation.replace('"', '').replace('\\', ''), k=pwd_length))

if __name__ == '__main__':
    res = test_ask_password("toto", "toto", True)
    res = test_ask_password("toto", "titi", False) and res
    res = test_ask_password("tototo", "toto", False) and res
    res = test_ask_password("toto", "tototo", False) and res

    exp_password = get_random_pwd_of_length(PASSWORD_MAX_LENGTH)
    res = test_ask_password(exp_password, exp_password, True) and res

    exp_password = get_random_pwd_of_length(PASSWORD_MAX_LENGTH + 1)
    res = test_ask_password(exp_password, exp_password, False) and res

    sys.exit(0 if res else 1)
