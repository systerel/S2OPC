#!/usr/bin/python3
# -*- coding: utf-8 -*-

# Copyright (C) 2017 Systerel and others.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
#!/usr/bin/python3.4
#-*-coding:Utf-8 -*

# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
