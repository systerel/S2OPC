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

from collections import namedtuple
import re
import sys

TapInfo = namedtuple('TapInfo', ['n_ok', 'n_nok'])

class TapFormatException(Exception):
    pass

def usage():
    sys.stderr.write('''Usage: %s CMD TAP_FILE [TAP_FILE...]

Checks the given TAP file(s) consistency: a test plan has to be present, and to
match the number of tests in the file. All tests should also be 'ok'.
''' % sys.argv[0])

PLAN_LINE_RE = re.compile(r'(\d+)\.\.(\d+)')

def check_tap(lines):
    n_tests_before_plan = 0
    n_tests = -1
    n_ok = 0
    n_nok = 0

    for line in lines:
        line = line.strip()

        if not line or line[0] == '#':
            continue

        m = PLAN_LINE_RE.match(line)

        if m:
            if m.group(1) != '1':
                raise TapFormatException('Invalid plan line \'%s\': first test should be 1' % line)

            n_tests = int(m.group(2))

            if n_tests < 1:
                raise TapFormatException('Invalid plan line \'%s\': number of tests should be strictly positive')

            n_tests_before_plan = n_ok + n_nok

        elif line.startswith('ok '):
            n_ok += 1
        elif line.startswith('not ok '):
            n_nok += 1
        else:
            raise TapFormatException('Invalid line in tap file: \'%s\'' % line)

    if n_tests == -1:
        raise TapFormatException('No test plan in file')

    if n_tests_before_plan != 0 and n_tests_before_plan != (n_ok + n_nok):
        raise TapFormatException('Test plan should be at the beginning or at the end of the file')

    if n_tests != (n_ok + n_nok):
        raise TapFormatException('Number of tests in file (%d) does not match expected number of tests (%d)' % (n_ok + n_nok, n_tests))

    return TapInfo(n_ok, n_nok)


if __name__ == '__main__':
    if len(sys.argv) < 2:
        usage()
        sys.exit(1)

    retcode = 0

    for filename in sys.argv[1:]:
        try:
            with open(filename, encoding='utf-8') as fd:
                try:
                    info = check_tap(fd)

                    if info.n_nok > 0:
                        print('%s has %d failed tests' % (filename, info.n_nok))
                        retcode = 1
                except TapFormatException as e:
                    print('%s is not a valid TAP file: %s' % (filename, str(e)))
                    retcode = 1
        except FileNotFoundError:
            print('%s does not exist' % filename)
            retcode = 1

    sys.exit(retcode)

# vim: set et ts=4 sw=4:
