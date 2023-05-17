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
import sys
if platform.system() == "Windows":
    # pexpect API for windows was tested without success
    import wexpect as pexpect
    # string output
    stdout = sys.stdout
else:
    import pexpect
    # binary output
    stdout = sys.stdout.buffer
import argparse
import signal

PASSWORD="password"
description = '''Runs a program and enter default password when asked with ".*[Pp]assword :" prompt'''

binProc = None

def signal_handler(sig, frame):
    if binProc is not None:
        binProc.kill(sig)

def run_with_password(cmd, nb_pwd, interact):
    global binProc
    binProc = pexpect.spawn(cmd)
    try:
        for i in range(0,nb_pwd):
           binProc.expect('.*[Pp]assword.*:')
           binProc.sendline(PASSWORD)
           # Display output prior to expected prompt
           stdout.write(binProc.before)
    except:
        stdout.write(binProc.before)
        raise
    # Change to interactive mode (output displays in stdout, etc.)
    # note: display issues when several samples binaries at same time + not implemented for windows
    if interact:
        binProc.interact()
    retCode = binProc.wait()
    sys.exit(retCode)

if __name__ == '__main__':
    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)

    parser = argparse.ArgumentParser(description=description)
    parser.add_argument('--nb-passwords', metavar='nb_pwds', help='The command to start the background server', default=1, type=int)
    parser.add_argument('--password', metavar='pwd', help='Change the default password to this value')
    parser.add_argument('--interact', default=False, action='store_true', dest='interact', help='Switch behavior to interact after entering password (works only for linux)')
    parser.add_argument('cmd', metavar='CMD', nargs='+', help='The command to run')
    parser.add_argument('args', metavar='ARG', nargs=argparse.REMAINDER, help='The command arguments')

    args = parser.parse_args()

    if args.password is not None:
        PASSWORD=args.password

    cmd = " ".join(args.cmd + args.args)

    run_with_password(cmd, args.nb_passwords, args.interact)
