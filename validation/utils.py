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

import time
import traceback

def loop_until_kbdinterrupt(fRetryDelay, func, *args, **kwargs):
    """
    Calls func(*args, **kwargs) in a whil loop, unitl someone sends a KeyboardInterrupt.
    When there is an exception, print the traceback on stdout and waits fRetryDelay seconds before retrying.
    Otherwise, function is called again when it's finished.

    Returning from this means a KeyboardInterrupt occured.
    """
    assert callable(func)
    print('Running in loop, even with errors, ^C (KeyaboardInterrupt) to stop')
    while True:
        try:
            func(*args, **kwargs)
        except KeyboardInterrupt:
            print('Keyaboard Interrupt, aborting...')
            break
        except:
            print('Exception:')
            traceback.print_exc()
            print('\nRetrying in {} seconds'.format(fRetryDelay))
            time.sleep(fRetryDelay)
