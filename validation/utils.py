#!/usr/bin/python3
# -*- coding: utf-8 -*-


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
