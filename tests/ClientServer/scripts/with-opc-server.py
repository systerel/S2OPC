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
import shlex
import subprocess
import sys

import wait_server

description = '''Runs a program with a test S2OPC server running in the
background.

The background server should listen on port 4841 to be considered as running by
this script.'''

def log(msg):
    print(msg)
    sys.stdout.flush()

def parse_env(string_env):
    pairs = string_env.split()
    env = {}
    for pair in pairs :
        if "=" in pair :
            key, value = pair.split('=',1)
            env[key] = value
        else :
            print("Malformed pair {} expect \"Variable=Value\" format", pair)
            return None
    return env


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=description)
    parser.add_argument('--server-cmd', metavar='CMD', help='The command to start the background server')
    parser.add_argument('--server-wd', metavar='DIR', help='The directory to run the server in')
    parser.add_argument('--server-env', metavar='ENV', help='Environment variable to run along the server')
    parser.add_argument('--wait-server', action='store_true', default=False,
                        help='Wait for the server to exit instead of killing it when the client is done')
    parser.add_argument('--wait-timeout',
                        help='Timeout duration expressed as a float in seconds. Return code of the function is 0 in this case.')
    parser.add_argument('cmd', metavar='CMD', help='The command to run')
    parser.add_argument('args', metavar='ARGS', nargs=argparse.REMAINDER,
                        help='Parameters to pass to the command')

    args = parser.parse_args()

    if args.server_cmd is None:
        sys.stderr.write('Missing server command.\n')
        sys.exit(1)

    env=None
    if None != args.server_env:
        env = parse_env(args.server_env)
    log('Starting server')
    server_process = subprocess.Popen(shlex.split(args.server_cmd), cwd=args.server_wd, env=env)

    if not wait_server.wait_server(wait_server.DEFAULT_URL, wait_server.TIMEOUT):
        log('Timeout for starting server')
        # 2 times to avoid OPCUA shutdown phase
        server_process.kill()
        server_process.kill()
        server_process.wait()
        sys.exit(1)

    cmd = [args.cmd] + args.args

    log('Starting test %s' % ' '.join(cmd))

    try:
        if not args.wait_timeout:
            subprocess.check_call(cmd)
        else:
            log('Launch command with timeout of %f seconds \n' % float(args.wait_timeout))
            subprocess.check_call(cmd, timeout=float(args.wait_timeout))
        test_ret = 0
    except subprocess.TimeoutExpired as e:
        # Nominal exit
        log('Test time out')
        test_ret = 0
    except subprocess.CalledProcessError as e:
        test_ret = e.returncode
    except FileNotFoundError as e:
        sys.stderr.write('%s: Not such file or directory\n' % e.filename)
        test_ret = 127

    if not args.wait_server:
        log('Test finished, killing server')
        server_process.terminate()

    log('Waiting for server to exit')
    server_ret = server_process.wait()

    log('Done with test_ret=%d and server_ret=%d' % (test_ret, server_ret))
    running_in_windows = sys.platform.startswith('win32')
    if test_ret == 0:
        # server return code can be checked on Linux only
        sys.exit(0 if running_in_windows else server_ret)
    else:
        sys.exit(test_ret)

# vim: set et ts=4 sw=4:
