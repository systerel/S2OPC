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
import sys
import re
import time

class TestParserLog:
    def __init__(self, log_name, verbose):
        self.log_name = log_name
        self.fd = open(file = self.log_name, mode='w', encoding='utf-8')
        self.verbose = verbose

    def log(self, msg):
        if self.verbose:
            self.fd.write(msg + '\n')

def log(verbose, msg):
    if verbose:
        print(msg)
        sys.stdout.flush()

def check_line(line):
    # check if there is an assertion
    if re.match("Assertion failed", line):
        return 1
    return 0

def watch_output(file, logger):
    breakLine = " ------ END Unit Test ------ "
    line = ""
    ret = 0
    while line != breakLine and ret == 0:
        line = file.readline().strip('\n')
        if (line == '') :
            time.sleep(0.01)
        else :
            logger.log("[Test parser] check line >>> %s" %(line))
            ret = check_line(line)
    return ret

description = '''Watch a file where qemu is writting logs and check for errors'''

if __name__ == "__main__":
    verbose = False
    parser = argparse.ArgumentParser(description=description)
    parser.add_argument('qemu_out', metavar='file', help='The file where qemu redirect serial output')
    parser.add_argument('-v', '--verbose', action='store_true', help='The file where qemu redirect serial output')

    args = parser.parse_args()

    verbose = args.verbose
    try:
        qemu_out = open(args.qemu_out)
        logger = TestParserLog(args.qemu_out.replace('.txt','.log'), verbose)
        ret = watch_output(qemu_out, logger)
        exit(ret)
    except FileNotFoundError:
        log(True, "File %s not found" %(args.qemu_out))
        exit(2)
