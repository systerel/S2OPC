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
from opcua import ua
from time import sleep
import sys
from tap_logger import TapLogger
from pubsub_server import PubSubServer
from pubsub_server_helpers import waitForEvent, helpTestStopStart, helpConfigurationChangeAndStart

PUB_SERVER_URL = 'opc.tcp://localhost:4842'
SUB_SERVER_URL = 'opc.tcp://localhost:4843'

NID_START_STOP = u"ns=1;s=PubSubStartStop"
NID_STATUS = u"ns=1;s=PubSubStatus"
NID_CONFIGURATION = u"ns=1;s=PubSubConfiguration"

NID_SUB_STRING = u"ns=1;s=SubString"
NID_SUB_INT = u"ns=1;s=SubInt"
NID_PUB_STRING = u"ns=1;s=PubString"
NID_PUB_INT = u"ns=1;s=PubInt"

NODE_VARIANT_TYPE = { NID_SUB_INT : ua.VariantType.Int64,
                      NID_SUB_STRING : ua.VariantType.String,
                      NID_PUB_INT : ua.VariantType.Int64,
                      NID_PUB_STRING : ua.VariantType.String }

TIMEOUT_SEC_COMMUNICATION = 0.2 # 0.2sec
PUBSUB_SCHEDULER_FIRST_PERIOD = 0.5 # 0.5sec

PUB_DEFAULT_XML_PATH = 'config_pub_server.xml'
SUB_DEFAULT_XML_PATH = 'config_sub_server.xml'

XML_PUB_NO_SECURITY = """<PubSub>
    <connection address="opc.udp://232.1.2.100:4840" mode="publisher" publisherId="i=1">
        <message groupId="1" publishingInterval="100" groupVersion="1">
            <dataset writerId="50">
                <variable nodeId="ns=1;s=PubBool" displayName="varBool" dataType="Boolean"/>
                <variable nodeId="ns=1;s=PubInt" displayName="varInt" dataType="Int64"/>
                <variable nodeId="ns=1;s=PubString" displayName="varString" dataType="String"/>
            </dataset>
        </message>
    </connection>
</PubSub>"""

XML_SUB_NO_SECURITY = """<PubSub>
    <connection address="opc.udp://232.1.2.100:4840" mode="subscriber">
        <message groupId="1" publishingInterval="100" groupVersion="1" publisherId="i=1">
            <dataset writerId="50">
                <variable nodeId="ns=1;s=SubBool" displayName="varBool" dataType="Boolean"/>
                <variable nodeId="ns=1;s=SubInt" displayName="varInt" dataType="Int64"/>
                <variable nodeId="ns=1;s=SubString" displayName="varString" dataType="String"/>
            </dataset>
        </message>
    </connection>
</PubSub>"""

def test(logger):

    pub_server = PubSubServer(PUB_SERVER_URL, NID_CONFIGURATION, NID_START_STOP, NID_STATUS, None, None, None, None, None)
    sub_server = PubSubServer(SUB_SERVER_URL, NID_CONFIGURATION, NID_START_STOP, NID_STATUS, None, None, None, None, None)
    try:
        pub_server.connect()
        helpTestStopStart(pub_server, True, logger)
        sub_server.connect()
        helpTestStopStart(sub_server, True, logger)

        ###
        #   TC 1 : Test communication with security
        ###
        # Write new value in pub server
        pub_server.setValue(NID_PUB_STRING, NODE_VARIANT_TYPE[NID_PUB_STRING], "New text in pub server")
        # Value must have been updated in sub server using Pub Sub communication
        res = waitForEvent(lambda: "New text in pub server" == sub_server.getValue(NID_SUB_STRING))
        logger.add_test("TC 1: Basic communication operational", res == True)
        # Stop pub after the security SN has reached a value that is not easily resynchronizable with sub's value
        sleep(1)
        # Check that the communication is still operational before stopping the pub.
        pub_server.setValue(NID_PUB_INT, NODE_VARIANT_TYPE[NID_PUB_INT], 29)
        # Value must have been updated in sub server using Pub Sub communication
        res = waitForEvent(lambda: 29 == sub_server.getValue(NID_SUB_INT))
        logger.add_test("TC 1: Basic communication operational right before stopping the pub", res == True)
        # Stop the pub
        pub_server.stop()
        helpTestStopStart(pub_server, False, logger)
        logger.add_test("TC 1: Stop pub", True)
        # Write new value in pub server
        pub_server.setValue(NID_PUB_STRING, NODE_VARIANT_TYPE[NID_PUB_STRING], "New text in pub server that will not be set in sub server")
        # Value must have NOT been updated in sub server because Pub is stopped
        res = waitForEvent(lambda: "New text in pub server that will not be set in sub server" == sub_server.getValue(NID_SUB_STRING),
                           maxWait_s = 2 * TIMEOUT_SEC_COMMUNICATION)
        logger.add_test("TC 1: Basic communication not operational", res == False)
        # Restart pub and write new published value
        pub_server.start()
        helpTestStopStart(pub_server, True, logger)
        sleep(PUBSUB_SCHEDULER_FIRST_PERIOD) # need to wait for the pub to GetSecurityKeys here
        pub_server.setValue(NID_PUB_INT, NODE_VARIANT_TYPE[NID_PUB_INT], -578)
        # Value must have been updated in sub server using Pub Sub communication
        res = waitForEvent(lambda: -578 == sub_server.getValue(NID_SUB_INT), maxWait_s = 2 * TIMEOUT_SEC_COMMUNICATION)
        logger.add_test("TC 1: Communication has resumed after the interruption", res == True)

        ### 
        #   TC 2 : Test communication without security.
        ###
        # Load new configuration
        pub_server.stop()
        helpTestStopStart(pub_server, False, logger)
        sub_server.stop()
        helpTestStopStart(sub_server, False, logger)
        helpConfigurationChangeAndStart(PUB_DEFAULT_XML_PATH, pub_server, XML_PUB_NO_SECURITY, logger)
        helpConfigurationChangeAndStart(SUB_DEFAULT_XML_PATH, sub_server, XML_SUB_NO_SECURITY, logger)
        # Write new value in pub server
        pub_server.setValue(NID_PUB_STRING, NODE_VARIANT_TYPE[NID_PUB_STRING], "No security: New text in pub server")
        # Value must have been updated in sub server using Pub Sub communication
        res = waitForEvent(lambda: "No security: New text in pub server" == sub_server.getValue(NID_SUB_STRING))
        logger.add_test("TC 2: Basic communication operational", res == True)
        # Stop pub after the security SN has reached a value that is not easily resynchronizable with sub's value
        sleep(1)
        # Check that the communication is still operational before stopping the pub.
        pub_server.setValue(NID_PUB_INT, NODE_VARIANT_TYPE[NID_PUB_INT], 57778)
        # Value must have been updated in sub server using Pub Sub communication
        res = waitForEvent(lambda: 57778 == sub_server.getValue(NID_SUB_INT))
        logger.add_test("TC 2: Basic communication operational right before stopping the pub", res == True)
        # Stop the pub
        pub_server.stop()
        helpTestStopStart(pub_server, False, logger)
        logger.add_test("TC 2: Stop pub", True)
        # Write new value in pub server
        pub_server.setValue(NID_PUB_STRING, NODE_VARIANT_TYPE[NID_PUB_STRING], "No security: New text in pub server that will not be set in sub server")
        # Value must have NOT been updated in sub server because Pub is stopped
        res = waitForEvent(lambda: "No security: New text in pub server that will not be set in sub server" == sub_server.getValue(NID_SUB_STRING),
                           maxWait_s = 2 * TIMEOUT_SEC_COMMUNICATION)
        logger.add_test("TC 2: Basic communication not operational", res == False)
        # Restart pub and write new published value
        pub_server.start()
        helpTestStopStart(pub_server, True, logger)
        pub_server.setValue(NID_PUB_INT, NODE_VARIANT_TYPE[NID_PUB_INT], 996)
        # Value must have been updated in sub server using Pub Sub communication
        res = waitForEvent(lambda: 996 == sub_server.getValue(NID_SUB_INT), maxWait_s = 2 * TIMEOUT_SEC_COMMUNICATION)
        logger.add_test("TC 2: Communication has resumed after the interruption", res == True)

        # Stop pub and sub
        pub_server.stop()
        helpTestStopStart(pub_server, False, logger)
        sub_server.stop()
        helpTestStopStart(sub_server, False, logger)

    except Exception as e:
        logger.add_test('Received exception %s'%e, False)

    finally:
        pub_server.disconnect()
        sub_server.disconnect()
        retcode = -1 if logger.has_failed_tests else 0
        logger.finalize_report()
        sys.exit(retcode)

if __name__=='__main__':
    argparser = argparse.ArgumentParser()
    argparser.add_argument('--tap', dest='tap', default='pub_sub_communication_test.tap', help='Set the TAP file name for tests results')
    argparser.add_argument('--verbose', action='store_true', default=False, help='Verbose mode. Default is false')
    args = argparser.parse_args()
    logger = TapLogger(args.tap, verbose=args.verbose)
    test(logger)

