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

import os
import subprocess
import sys

def usage():
    sys.stderr.write('''Usage: %s CMD [ARGUMENTS...]

Runs the given command, logging the result to a TAP file. The path to the TAP
file is specified using the CK_TAP_LOG_FILE_NAME environment variable.
The expected return code is 0 or defined by the CK_EXP_RET_CODE otherwise.
''' % sys.argv[0])

if __name__ == '__main__':
    tap_filename = os.getenv('CK_TAP_LOG_FILE_NAME')

    if tap_filename is None:
        sys.stderr.write('Missing environment variable: CK_TAP_LOG_FILE_NAME\n')
        sys.exit(1)

    cmd = sys.argv[1:]

    if not cmd:
        usage()
        sys.exit(1)

    exp_ret_code = os.getenv('CK_EXP_RET_CODE')
    if exp_ret_code is None:
        exp_ret_code = 0
    else:
        exp_ret_code = int(exp_ret_code)

    retcode = 0

    with open(tap_filename, 'w') as fd:
        fd.write('1..1\n')

        try:
            subprocess.check_call(cmd)
            fd.write('ok 1 - %s: Passed\n' % ' '.join(cmd))
        except subprocess.CalledProcessError as e:
            retcode = e.returncode
            if exp_ret_code != retcode:
                fd.write('not ok 1 - %s: Exited with code %d\n' % (' '.join(cmd), e.returncode))
            else:
                fd.write('ok 1 - %s: Passed with expected code %d\n' % (' '.join(cmd), e.returncode))
                retcode = 0

    sys.exit(retcode)

# vim: set et ts=4 sw=4:
