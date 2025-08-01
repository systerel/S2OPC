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

PUBSUB_SERVER_DEVICE_URL = 'opc.tcp://localhost:4842'
PUBSUB_SERVER_CONTROLLER_URL = 'opc.tcp://localhost:4843'

NID_SUB_STRING = u"ns=1;s=SubString"
NID_SUB_INT = u"ns=1;s=SubInt"
NID_PUB_STRING = u"ns=1;s=PubString"
NID_PUB_INT = u"ns=1;s=PubInt"

CONTROLLER_SKS_KEYLIFETIME_SEC = 5
CONTROLLER_SKS_NB_GENERATED_KEYS = 5

import argparse
from opcua import ua
from time import sleep
import sys
from tap_logger import TapLogger
from pubsub_server import PubSubServer
from pubsub_server_helpers import waitForEvent

NODE_VARIANT_TYPE = { NID_SUB_INT : ua.VariantType.Int64,
                      NID_SUB_STRING : ua.VariantType.String,
                      NID_PUB_INT : ua.VariantType.Int64,
                      NID_PUB_STRING : ua.VariantType.String }

def testControllerDevicePubsub(logger):

    pubsubserver_controller = PubSubServer(PUBSUB_SERVER_CONTROLLER_URL, None, None, None, None, None, None, None, None)
    pubsubserver_device = PubSubServer(PUBSUB_SERVER_DEVICE_URL, None, None, None, None, None, None, None, None)

    try:
        pubsubserver_controller.connect()
        pubsubserver_device.connect()

        # TC 1 : Write in controller and check that the value is updated in the device, and vice versa.
        # Write new value in controller server
        pubsubserver_controller.setValue(NID_PUB_STRING, NODE_VARIANT_TYPE[NID_PUB_STRING], "Text modified by the test!")
        # Value must have been updated in device server using Pub Sub communication
        # Maximum wait time is that SetSecurityKeys is called by controller and success.
        maxWait_call_SetSecurityKeys = CONTROLLER_SKS_KEYLIFETIME_SEC * CONTROLLER_SKS_NB_GENERATED_KEYS / 2
        waitForEvent(lambda: "Text modified by the test!" == pubsubserver_device.getValue(NID_SUB_STRING), maxWait_s=maxWait_call_SetSecurityKeys)
        logger.add_test('TC 1: Write in controller -> Published -> Read expected value in device', 
                        "Text modified by the test!" == pubsubserver_device.getValue(NID_SUB_STRING))

        # make sure Pubsub will use new key
        sleep(2 * CONTROLLER_SKS_KEYLIFETIME_SEC)
        logger.add_test('TC 1: Wait time for key renewal ', True)

        # Write new value in device server
        pubsubserver_device.setValue(NID_PUB_INT, NODE_VARIANT_TYPE[NID_PUB_INT], -288888)
        # Value must have been updated in controller server using Pub Sub communication
        waitForEvent(lambda: -288888 == pubsubserver_controller.getValue(NID_SUB_INT))
        logger.add_test('TC 1: Write in device -> Published -> Read expected value in controller', 
                        -288888 == pubsubserver_controller.getValue(NID_SUB_INT))

        # Write new value in controller server
        pubsubserver_controller.setValue(NID_PUB_STRING, NODE_VARIANT_TYPE[NID_PUB_STRING], "Text modified after key renewal")
        # Value must have been updated in device server using Pub Sub communication
        waitForEvent(lambda: "Text modified after key renewal" == pubsubserver_device.getValue(NID_SUB_STRING))
        logger.add_test('TC 1: Write in controller -> Published -> Read expected value in device', 
                        "Text modified after key renewal" == pubsubserver_device.getValue(NID_SUB_STRING))

    except Exception as e:
        logger.add_test('Received exception %s'%e, False)

    finally:
        pubsubserver_controller.disconnect()
        pubsubserver_device.disconnect()
        retcode = -1 if logger.has_failed_tests else 0
        logger.finalize_report()
        sys.exit(retcode)

if __name__=='__main__':
    argparser = argparse.ArgumentParser()
    argparser.add_argument('--tap', dest='tap', default='controller_device_pubsub_test.tap', help='Set the TAP file name for tests results')
    argparser.add_argument('--verbose', action='store_true', default=False, help='Verbose mode. Default is false')
    args = argparser.parse_args()
    logger = TapLogger(args.tap, verbose=args.verbose)
    testControllerDevicePubsub(logger)
