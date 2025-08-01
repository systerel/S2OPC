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
import time

import wait_publisher

description = '''Runs a program with a test S2OPC publisher running in the
background.

The background publisher should listen on port 4840 to be considered as running by
this script.'''

def log(msg):
    print(msg)
    sys.stdout.flush()

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=description)
    parser.add_argument('--publisher-cmd', metavar='CMD', help='The command to start the background publisher')
    parser.add_argument('--pub-endpoint', metavar='CMD', help='The endpoint of the pub server')
    parser.add_argument('--pub-config', metavar='CMD', help='The config XML of the pub server')
    parser.add_argument('--sub-cmd', metavar='CMD', help='The command to start the background subscriber')
    parser.add_argument('--sub-endpoint', metavar='CMD', help='The endpoint of the sub server')
    parser.add_argument('--sub-config', metavar='CMD', help='The config XML of the sub server')
    parser.add_argument('--sks-cmd', metavar='CMD', help='The command to start the background Security Keys Server')
    parser.add_argument('--sks-url', metavar='CMD', help='The url of the Security Keys Server')
    parser.add_argument('--no-wait-pub-message', action='store_true', default=False, help='The script does not wait for a publisher message to start subscriber: only sleep(1)')
    parser.add_argument('--kill-publisher', action='store_true', default=False, help='Kill the publisher when the command is done')
    parser.add_argument('--wait-publisher', action='store_true', default=False,
                        help='Wait for the publisher to exit instead of killing it when the client is done')
    parser.add_argument('cmd', metavar='CMD', help='The command to run')
    parser.add_argument('args', metavar='ARGS', nargs=argparse.REMAINDER,
                        help='Parameters to pass to the command')

    args = parser.parse_args()

    if args.publisher_cmd is None:
        sys.stderr.write('Missing publisher command.\n')
        sys.exit(1)

    publisher_process = None
    subscriber_process = None
    sks_process = None
    try:
        if args.sks_cmd is None:
            log('No Security Keys Server')
        else:
            log('Starting Security Keys Server')
            sks_process = subprocess.Popen([args.sks_cmd, "1"])

        if sks_process is not None:
            if not wait_publisher.wait_server(args.sks_url, wait_publisher.TIMEOUT):
                log('Timeout for starting SKS server')
                sks_process.kill()
                sks_process.wait()
                sys.exit(1)

        if args.sub_cmd is not None:
            log('Starting subscriber')
            if args.sub_endpoint is not None and args.sub_config is not None:
                # Arguments are provided to the binary
                subscriber_process = subprocess.Popen([args.sub_cmd, args.sub_endpoint, args.sub_config])
            else:
                subscriber_process = subprocess.Popen(shlex.split(args.sub_cmd))
            if subscriber_process is not None:
                if not wait_publisher.wait_server(wait_publisher.DEFAULT_SUB_SERVER_URL, wait_publisher.TIMEOUT):
                    log('Timeout for starting subscriber')
                    subscriber_process.kill()
                    subscriber_process.wait()
                    sys.exit(1)

        log('Starting publisher')
        if args.pub_endpoint is not None and args.pub_config is not None:
                # Arguments are provided to the binary
            publisher_process = subprocess.Popen([args.publisher_cmd, args.pub_endpoint, args.pub_config])
        else:
            publisher_process = subprocess.Popen(shlex.split(args.publisher_cmd))

        if args.no_wait_pub_message:
            time.sleep(1)
        elif not wait_publisher.wait_publisher(wait_publisher.DEFAULT_PUB_URL, wait_publisher.TIMEOUT):
            log('Timeout for starting publisher')
            publisher_process.kill()
            publisher_process.wait()
            if sks_process is not None:
                sks_process.kill()
                sks_process.wait()
                sks_process = None

            sys.exit(1)

        cmd = [args.cmd] + args.args

        log('Starting test %s' % ' '.join(cmd))

        try:
            subprocess.check_call(cmd)
            test_ret = 0
        except subprocess.CalledProcessError as e:
            test_ret = e.returncode
        except FileNotFoundError as e:
            sys.stderr.write('%s: Not such file or directory\n' % e.filename)
            test_ret = 127
        log('Test finished')

        if not args.wait_publisher:
            log('Test finished, killing publisher')
            if args.kill_publisher:
                publisher_process.kill()
            else:
                publisher_process.terminate()
        
        log('Waiting for publisher to exit')
        publisher_ret = publisher_process.wait()
        publisher_process = None

        if sks_process is not None:
            log('killing Security Keys Server')
            sks_process.kill()
            sks_process.wait()
            sks_process = None

        if subscriber_process is not None:
            log('killing subscriber Server')
            subscriber_process.kill()
            subscriber_process.wait()
            subscriber_process = None

    except Exception as e:
        if sks_process is not None:
            sks_process.kill()
        if publisher_process is not None:
            publisher_process.kill()
        raise e

    log('Done')
    running_in_windows = sys.platform.startswith('win32')
    if test_ret == 0:
        # publisher return code can be checked on Linux only
        sys.exit(0 if running_in_windows else publisher_ret)
    else:
        sys.exit(test_ret)

