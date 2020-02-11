#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (C) Systerel SAS 2019, all rights reserved.

import os
import subprocess
import sys

def usage():
    sys.stderr.write('''Usage: %s CMD [ARGUMENTS...]

Runs the given command, logging the result to a TAP file. The path to the TAP
file is specified using the CK_TAP_LOG_FILE_NAME environment variable.
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

    retcode = 0

    with open(tap_filename, 'w') as fd:
        fd.write('1..1\n')

        try:
            subprocess.check_call(cmd)
            fd.write('ok 1 - %s: Passed\n' % ' '.join(cmd))
        except subprocess.CalledProcessError as e:
            fd.write('not ok 1 - %s: Exited with code %d\n' % (' '.join(cmd), e.returncode))
            retcode = e.returncode

    sys.exit(retcode)

# vim: set et ts=4 sw=4:
