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

from time import sleep
from shutil import copyfile, move
import argparse

from opcua import ua
from tap_logger import TapLogger
from pubsub_server import PubSubServer

DEFAULT_URI = 'opc.tcp://localhost:4841'
NID_CONFIGURATION = u"ns=1;s=PubSubConfiguration"
NID_START_STOP = u"ns=1;s=PubSubStartStop"
NID_STATUS = u"ns=1;s=PubSubStatus"

NID_SUB_BOOL = u"ns=1;s=SubBool"
NID_SUB_UINT16 = u"ns=1;s=SubUInt16"
NID_SUB_UINT = u"ns=1;s=SubUInt"
NID_SUB_INT = u"ns=1;s=SubInt"

NID_PUB_BOOL = u"ns=1;s=PubBool"
NID_PUB_UINT16 = u"ns=1;s=PubUInt16"
NID_PUB_UINT = u"ns=1;s=PubUInt"
NID_PUB_INT = u"ns=1;s=PubInt"

DEFAULT_XML_PATH = 'config_pubsub_server.xml'

# PublishingInterval (seconds) of XML default configuration
STATIC_CONF_PUB_INTERVAL = 2.2
DYN_CONF_PUB_INTERVAL = 0.800

NODE_VARIANT_TYPE = { NID_SUB_BOOL : ua.VariantType.Boolean,
                      NID_SUB_UINT16 : ua.VariantType.UInt16,
                      NID_SUB_UINT : ua.VariantType.UInt64,
                      NID_SUB_INT : ua.VariantType.Int64,
                      NID_PUB_BOOL : ua.VariantType.Boolean,
                      NID_PUB_UINT16 : ua.VariantType.UInt16,
                      NID_PUB_UINT : ua.VariantType.UInt64,
                      NID_PUB_INT : ua.VariantType.Int64 }

XML_EMPTY = """<PubSub publisherId="1">
</PubSub>"""

XML_PUBLISHER_ONLY = """<PubSub publisherId="1">
    <connection address="opc.udp://232.1.2.100:4840" mode="publisher">
        <message id="1" publishingInterval="1000" version="1">
            <variable nodeId="ns=1;s=PubBool" displayName="pubVarBool" dataType="Boolean" />
            <variable nodeId="ns=1;s=PubUInt16" displayName="pubVarUInt16" dataType="UInt16" />
            <variable nodeId="ns=1;s=PubInt" displayName="pubVarInt" dataType="Int64" />
        </message>
    </connection>
</PubSub>"""

XML_SUBSCRIBER_ONLY = """<PubSub>
    <connection address="opc.udp://232.1.2.100:4840" mode="subscriber">
        <message id="1" publishingInterval="1000" version="1" publisherId="1">
            <variable nodeId="ns=1;s=SubBool" displayName="subVarBool" dataType="Boolean" />
            <variable nodeId="ns=1;s=SubUInt16" displayName="subVarUInt16" dataType="UInt16" />
            <variable nodeId="ns=1;s=SubInt" displayName="subVarInt" dataType="Int64" />
        </message>
    </connection>
</PubSub>"""

XML_PUBSUB_LOOP = """<PubSub publisherId="1">
    <connection address="opc.udp://232.1.2.100:4840" mode="publisher">
        <message id="1" publishingInterval="200" version="1">
            <variable nodeId="ns=1;s=PubBool" displayName="pubVarBool" dataType="Boolean" />
            <variable nodeId="ns=1;s=PubUInt16" displayName="pubVarUInt16" dataType="UInt16" />
            <variable nodeId="ns=1;s=PubInt" displayName="pubVarInt" dataType="Int64" />
        </message>
    </connection>
    <connection address="opc.udp://232.1.2.100:4840" mode="subscriber">
        <message id="1" publishingInterval="200" version="1" publisherId="1">
            <variable nodeId="ns=1;s=SubBool" displayName="subVarBool" dataType="Boolean" />
            <variable nodeId="ns=1;s=SubUInt16" displayName="subVarUInt16" dataType="UInt16" />
            <variable nodeId="ns=1;s=SubInt" displayName="subVarInt" dataType="Int64" />
        </message>
    </connection>
</PubSub>"""

XML_PUBSUB_LOOP_NULL = """<PubSub publisherId="1">
    <connection address="opc.udp://232.1.2.100:4840" mode="publisher">
        <message id="1" publishingInterval="200" version="1">
            <variable nodeId="ns=1;s=PubBool" displayName="pubVarBool" dataType="Null" />
            <variable nodeId="ns=1;s=PubUInt16" displayName="pubVarUInt16" dataType="Null" />
            <variable nodeId="ns=1;s=PubInt" displayName="pubVarInt" dataType="Int64" />
        </message>
    </connection>
    <connection address="opc.udp://232.1.2.100:4840" mode="subscriber">
        <message id="1" publishingInterval="200" version="1" publisherId="1">
            <variable nodeId="ns=1;s=SubBool" displayName="subVarBool" dataType="Null" />
            <variable nodeId="ns=1;s=SubUInt16" displayName="subVarUInt16" dataType="UInt16" />
            <variable nodeId="ns=1;s=SubInt" displayName="subVarInt" dataType="Null" />
        </message>
    </connection>
</PubSub>"""

# Configuration with message on Subscriber side not consistent with the one on Publisher
XML_PUBSUB_LOOP_MESSAGE_NOT_COMPATIBLE = """<PubSub publisherId="1">
    <connection address="opc.udp://232.1.2.100:4840" mode="publisher">
        <message id="1" publishingInterval="200" version="1">
            <variable nodeId="ns=1;s=PubBool" displayName="pubVarBool" dataType="Boolean" />
            <variable nodeId="ns=1;s=PubUInt16" displayName="pubVarUInt16" dataType="UInt16" />
            <variable nodeId="ns=1;s=PubInt" displayName="pubVarInt" dataType="Int64" />
        </message>
    </connection>
    <connection address="opc.udp://232.1.2.100:4840" mode="subscriber">
        <message id="1" publishingInterval="200" version="1" publisherId="1">
            <variable nodeId="ns=1;s=SubUInt16" displayName="subVarUInt16" dataType="UInt16" />
            <variable nodeId="ns=1;s=SubInt" displayName="subVarInt" dataType="Int64" />
            <variable nodeId="ns=1;s=SubBool" displayName="subVarBool" dataType="Boolean" />
        </message>
    </connection>
</PubSub>"""

# Bad formed Configuration ( variabl instead of variable )
XML_PUBSUB_BAD_FORMED_CONFIGURATION = """<PubSub publisherId="1">
    <connection address="opc.udp://232.1.2.100:4840" mode="publisher">
        <message id="1" publishingInterval="200" version="1">
            <variabl nodeId="ns=1;s=PubBool" displayName="pubVarBool" dataType="Boolean" />
            <variable nodeId="ns=1;s=PubUInt16" displayName="pubVarUInt16" dataType="UInt16" />
            <variable nodeId="ns=1;s=PubInt" displayName="pubVarInt" dataType="Int64" />
        </message>
    </connection>
    <connection address="opc.udp://232.1.2.100:4840" mode="subscriber">
        <message id="1" publishingInterval="200" version="1" publisherId="1">
            <variable nodeId="ns=1;s=SubUInt16" displayName="subVarUInt16" dataType="UInt16" />
            <variable nodeId="ns=1;s=SubInt" displayName="subVarInt" dataType="Int64" />
            <variable nodeId="ns=1;s=SubBool" displayName="subVarBool" dataType="Boolean" />
        </message>
    </connection>
</PubSub>"""

# Test connection and status depending of pStart command
def helpTestStopStart(pPubsubserver, pStart, pLogger):
    pLogger.add_test('Connected to OPCUA Server', pPubsubserver.isConnected())
    if pStart:
        pLogger.add_test('PubSub Module is started' , pPubsubserver.isStart())
    else:
        pLogger.add_test('PubSub Module is stopped', not pPubsubserver.isStart())
        
    status = 2 if pStart else 0
    pLogger.add_test('PubSub Module Status should be %d : %d' % (status, pPubsubserver.getStatus()) , status == pPubsubserver.getStatus())

# Send stop command following by start command
def helpRestart(pPubsubserver, pLogger):
    pLogger.add_test('Stop Server', True)
    pPubsubserver.stop()
    helpTestStopStart(pPubsubserver, False, pLogger)
        
    pLogger.add_test('Start Server', True)
    pPubsubserver.start()
    helpTestStopStart(pPubsubserver, True, pLogger)

# Change the configuration
def helpConfigurationChange(pPubsubserver, pConfig, pLogger):
    pLogger.add_test('Set Configuration', True)
    pPubsubserver.setConfiguration(pConfig)
    pLogger.add_test('Connected to OPCUA Server', pPubsubserver.isConnected())
    pubsubconfiguration = pPubsubserver.getConfiguration()
    pLogger.add_test('PubSub Configuration Node is changed ', pConfig == pubsubconfiguration)

# Change the configuration and restart PubSub Server
# Test connection, status and configuration
def helpConfigurationChangeAndStart(pPubsubserver, pConfig, pLogger, static=False):
    fd = open(DEFAULT_XML_PATH, "r")
    oldConfig = fd.read()
    fd.close()

    helpConfigurationChange(pPubsubserver, pConfig, pLogger)
    helpRestart(pPubsubserver, pLogger)

    # check configuration saved in default file
    fd = open(DEFAULT_XML_PATH, "r")
    if static:
        pLogger.add_test('Default PubSub Configuration file is not changed ', oldConfig == fd.read())
    else:
        pLogger.add_test('Default PubSub Configuration file is changed ', pConfig == fd.read())
    fd.close()


def helpTestSetValue(pPubsubserver, nodeId, value, pLogger):
    pPubsubserver.setValue(nodeId, NODE_VARIANT_TYPE[nodeId], value)
    expected = pPubsubserver.getValue(nodeId)
    pLogger.add_test('%s is changed' % nodeId , expected == value)




def testPubSubDynamicConf():

    logger = TapLogger("pubsub_server_test.tap")
    pubsubserver = PubSubServer(DEFAULT_URI, NID_CONFIGURATION, NID_START_STOP, NID_STATUS)

    defaultXml2Restore = False
    
    try:
        # backup of default XML file
        copyfile(DEFAULT_XML_PATH, DEFAULT_XML_PATH + ".bakup")
        defaultXml2Restore = True

        # secure channel connection
        pubsubserver.connect()

        #
        # TC 1 : Test with Publisher only configuration => only pub variables change
        #
        logger.begin_section("TC 1 : Publisher only")
        helpConfigurationChangeAndStart(pubsubserver, XML_PUBLISHER_ONLY, logger)

        # Init Subscriber variables
        helpTestSetValue(pubsubserver, NID_SUB_BOOL, True, logger)
        helpTestSetValue(pubsubserver, NID_SUB_UINT16, 500, logger)
        helpTestSetValue(pubsubserver, NID_SUB_INT, 100, logger)

        # Set Publisher variables
        helpTestSetValue(pubsubserver, NID_PUB_BOOL, False, logger)
        helpTestSetValue(pubsubserver, NID_PUB_UINT16, 1500, logger)
        helpTestSetValue(pubsubserver, NID_PUB_INT, -50, logger)
        sleep(DYN_CONF_PUB_INTERVAL)
        logger.add_test('Subscriber bool is not changed ', True == pubsubserver.getValue(NID_SUB_BOOL))
        logger.add_test('Subscriber uint16 is not changed ', 500 == pubsubserver.getValue(NID_SUB_UINT16))
        logger.add_test('Subscriber int is not changed ', 100 == pubsubserver.getValue(NID_SUB_INT))

        pubsubserver.stop()
        helpTestStopStart(pubsubserver, False, logger)

        #
        # TC 2 : Test with Subscriber only configuration => no variables change
        #
        logger.begin_section("TC 2 : Subscriber only")
        
        # Init Subscriber variables
        helpTestSetValue(pubsubserver, NID_SUB_BOOL, True, logger)
        helpTestSetValue(pubsubserver, NID_SUB_UINT16, 500, logger)
        helpTestSetValue(pubsubserver, NID_SUB_INT, 100, logger)

        helpConfigurationChangeAndStart(pubsubserver, XML_SUBSCRIBER_ONLY, logger)
        sleep(DYN_CONF_PUB_INTERVAL)
        logger.add_test('Subscriber bool is not changed ', True == pubsubserver.getValue(NID_SUB_BOOL))
        logger.add_test('Subscriber uint16 is not changed ', 500 == pubsubserver.getValue(NID_SUB_UINT16))
        logger.add_test('Subscriber int is not changed ', 100 == pubsubserver.getValue(NID_SUB_INT))

        pubsubserver.stop()
        helpTestStopStart(pubsubserver, False, logger)

        #
        # TC 3 : Test with Publisher and Subscriber configuration => subscriber variables change through Pub/Sub
        #
        logger.begin_section("TC 3 : Publisher Subscriber Loop")

        # Init Subscriber variables
        helpTestSetValue(pubsubserver, NID_SUB_BOOL, True, logger)
        helpTestSetValue(pubsubserver, NID_SUB_UINT16, 500, logger)
        helpTestSetValue(pubsubserver, NID_SUB_INT, 100, logger)

        helpConfigurationChangeAndStart(pubsubserver, XML_PUBSUB_LOOP, logger)
        sleep(DYN_CONF_PUB_INTERVAL)
        logger.add_test('Subscriber bool is changed ', False == pubsubserver.getValue(NID_SUB_BOOL))
        logger.add_test('Subscriber uint16 is changed ', 1500 == pubsubserver.getValue(NID_SUB_UINT16))
        logger.add_test('Subscriber int is changed ', -50 == pubsubserver.getValue(NID_SUB_INT))

        pubsubserver.stop()
        helpTestStopStart(pubsubserver, False, logger)

        #
        # TC 4 : Stop Pub/Sub module => subscriber variables do not change
        #
        logger.begin_section("TC 4 : Publisher Subscriber Loop")
        
        # Change Publisher variables
        helpTestSetValue(pubsubserver, NID_PUB_BOOL, True, logger)
        helpTestSetValue(pubsubserver, NID_PUB_UINT16, 6500, logger)
        helpTestSetValue(pubsubserver, NID_PUB_INT, -600, logger)
        
        sleep(DYN_CONF_PUB_INTERVAL)
        logger.add_test('Subscriber bool is not changed ', False == pubsubserver.getValue(NID_SUB_BOOL))
        logger.add_test('Subscriber uint16 is not changed ', 1500 == pubsubserver.getValue(NID_SUB_UINT16))
        logger.add_test('Subscriber int is not changed ', -50 == pubsubserver.getValue(NID_SUB_INT))

        # Start to change Subscriber variables
        pubsubserver.start()
        helpTestStopStart(pubsubserver, True, logger)
        sleep(DYN_CONF_PUB_INTERVAL)
        logger.add_test('Subscriber bool is changed ', True == pubsubserver.getValue(NID_SUB_BOOL))
        logger.add_test('Subscriber uint16 is changed ', 6500 == pubsubserver.getValue(NID_SUB_UINT16))
        logger.add_test('Subscriber int is changed ', -600 == pubsubserver.getValue(NID_SUB_INT))
        
        pubsubserver.stop()
        helpTestStopStart(pubsubserver, False, logger)

        #
        # TC 5 : Test with Publisher and Subscriber configuration. DataType is NULL
        #        => subscriber variables change through Pub/Sub
        #
        logger.begin_section("TC 5 : Publisher Subscriber Loop (DataType is Null)")

        # Init Publisher variables
        helpTestSetValue(pubsubserver, NID_PUB_BOOL, False, logger)
        helpTestSetValue(pubsubserver, NID_PUB_UINT16, 8500, logger)
        helpTestSetValue(pubsubserver, NID_PUB_INT, -300, logger)

        # Init Subscriber variables
        helpTestSetValue(pubsubserver, NID_SUB_BOOL, True, logger)
        helpTestSetValue(pubsubserver, NID_SUB_UINT16, 500, logger)
        helpTestSetValue(pubsubserver, NID_SUB_INT, 100, logger)

        helpConfigurationChangeAndStart(pubsubserver, XML_PUBSUB_LOOP_NULL, logger)
        sleep(DYN_CONF_PUB_INTERVAL)
        logger.add_test('Subscriber bool is changed ', False == pubsubserver.getValue(NID_SUB_BOOL))
        logger.add_test('Subscriber uint16 is changed ', 8500 == pubsubserver.getValue(NID_SUB_UINT16))
        logger.add_test('Subscriber int is changed ', -300 == pubsubserver.getValue(NID_SUB_INT))
     
        pubsubserver.stop()
        helpTestStopStart(pubsubserver, False, logger)
        #
        # TC 6 : Test with configuration message not consistent between the one on Publisher
        #        and the one on Subscriber  => subscriber variables do not change
        #
        logger.begin_section("TC 6 : Publisher Subscriber not consistent")

        helpConfigurationChangeAndStart(pubsubserver, XML_PUBSUB_LOOP_MESSAGE_NOT_COMPATIBLE, logger)

        # Init Subscriber variables
        helpTestSetValue(pubsubserver, NID_SUB_BOOL, False, logger)
        helpTestSetValue(pubsubserver, NID_SUB_UINT16, 500, logger)
        helpTestSetValue(pubsubserver, NID_SUB_INT, 100, logger)
        
        # Change Publisher variables
        helpTestSetValue(pubsubserver, NID_PUB_BOOL, True, logger)
        helpTestSetValue(pubsubserver, NID_PUB_UINT16, 6500, logger)
        helpTestSetValue(pubsubserver, NID_PUB_INT, -600, logger)
        
        sleep(DYN_CONF_PUB_INTERVAL)
        logger.add_test('Subscriber bool is not changed ', False == pubsubserver.getValue(NID_SUB_BOOL))
        logger.add_test('Subscriber uint16 is not changed ', 500 == pubsubserver.getValue(NID_SUB_UINT16))
        logger.add_test('Subscriber int is not changed ', 100 == pubsubserver.getValue(NID_SUB_INT))

        #
        # TC 7 : Test with a bad formed configuration => Pub/Sub status is 0
        #
        logger.begin_section("TC 7 : Bad formed configuration")
        helpConfigurationChange(pubsubserver, XML_PUBSUB_BAD_FORMED_CONFIGURATION, logger)

        pubsubserver.stop()
        helpTestStopStart(pubsubserver, False, logger)

        logger.add_test('Start Server', True)
        pubsubserver.start()
        status = pubsubserver.getStatus()
        logger.add_test('PubSub Module Status should be 0 : %d' % status , 0 == status)
       
        #
        # TC 8 : Start and Stop empty configuration
        #
        logger.begin_section("TC 8 : Empty configuration")
        helpConfigurationChange(pubsubserver, XML_EMPTY, logger)
        status = pubsubserver.getStatus()
        logger.add_test('PubSub Module Status should be 0 : %d' % status , 0 == status)

    finally:
        # restore default XML file
        if defaultXml2Restore:
            move(DEFAULT_XML_PATH + ".bakup", DEFAULT_XML_PATH)

        pubsubserver.disconnect()
        logger.add_test('Not connected to OPCUA Server', not pubsubserver.isConnected())
        logger.finalize_report()

# test with static configuration : data/xml_test/config_pubsub_server.xml
def testPubSubStaticConf():

    logger = TapLogger("pubsub_server_test.tap")
    pubsubserver = PubSubServer(DEFAULT_URI, NID_CONFIGURATION, NID_START_STOP, NID_STATUS)

    defaultXml2Restore = False
    
    try:
        # secure channel connection
        pubsubserver.connect()

        #
        # TC 1 : Test with Static Publisher and Subscriber configuration => subscriber variables change through Pub/Sub
        #
        logger.begin_section("TC 1 : Subscriber variables change through Pub/Sub")

        # Set Publisher variables and test change in Subscriber variables
        helpTestSetValue(pubsubserver, NID_PUB_BOOL, False, logger)
        helpTestSetValue(pubsubserver, NID_PUB_UINT, 8500, logger)
        helpTestSetValue(pubsubserver, NID_PUB_INT, -300, logger)
        
        sleep(STATIC_CONF_PUB_INTERVAL)
        
        logger.add_test('Subscriber bool is changed ', False == pubsubserver.getValue(NID_SUB_BOOL))
        logger.add_test('Subscriber uint is changed ', 8500 == pubsubserver.getValue(NID_SUB_UINT))
        logger.add_test('Subscriber int is changed ', -300 == pubsubserver.getValue(NID_SUB_INT))


        # Set Publisher variables and test change in Subscriber variables
        helpTestSetValue(pubsubserver, NID_PUB_BOOL, True, logger)
        helpTestSetValue(pubsubserver, NID_PUB_UINT, 5800, logger)
        helpTestSetValue(pubsubserver, NID_PUB_INT, -30, logger)
        
        sleep(STATIC_CONF_PUB_INTERVAL)
        
        logger.add_test('Subscriber bool is changed ', True == pubsubserver.getValue(NID_SUB_BOOL))
        logger.add_test('Subscriber uint is changed ', 5800 == pubsubserver.getValue(NID_SUB_UINT))
        logger.add_test('Subscriber int is changed ', -30 == pubsubserver.getValue(NID_SUB_INT))


        #
        # TC 2 : Pub-Sub server is stopped => no change in Subscriber variables
        #
        logger.begin_section("TC 2 : Pub-Sub server is stopped")

        pubsubserver.stop()
        logger.add_test('PubSub Module is stopped' , not pubsubserver.isStart())

        helpTestSetValue(pubsubserver, NID_PUB_BOOL, False, logger)
        helpTestSetValue(pubsubserver, NID_PUB_UINT, 7777, logger)
        helpTestSetValue(pubsubserver, NID_PUB_INT, 123654, logger)
        
        sleep(STATIC_CONF_PUB_INTERVAL)
        
        logger.add_test('Subscriber bool is changed ', True == pubsubserver.getValue(NID_SUB_BOOL))
        logger.add_test('Subscriber uint is changed ', 5800 == pubsubserver.getValue(NID_SUB_UINT))
        logger.add_test('Subscriber int is changed ', -30 == pubsubserver.getValue(NID_SUB_INT))

        #
        # TC 3 : Pub-Sub server is restarted => Subscriber variables changed
        #
        logger.begin_section("TC 3 : Pub-Sub server is restarted")

        pubsubserver.start()
        logger.add_test('PubSub Module is started' , pubsubserver.isStart())

        sleep(STATIC_CONF_PUB_INTERVAL)
        
        logger.add_test('Subscriber bool is changed ', False == pubsubserver.getValue(NID_SUB_BOOL))
        logger.add_test('Subscriber uint is changed ', 7777 == pubsubserver.getValue(NID_SUB_UINT))
        logger.add_test('Subscriber int is changed ', 123654 == pubsubserver.getValue(NID_SUB_INT))

        #
        # TC 4 : Change Pub-Sub server configuration => configuration cannot be changed in static mode
        #
        logger.begin_section("TC 4 : Change Pub-Sub server configuration")
        # Change Pub-Sub configuration node and check file is not change
        helpConfigurationChangeAndStart(pubsubserver, XML_SUBSCRIBER_ONLY, logger, static=True)

        # Check Pub-Sub module is still running
        helpTestSetValue(pubsubserver, NID_PUB_BOOL, True, logger)
        helpTestSetValue(pubsubserver, NID_PUB_UINT, 1245, logger)
        helpTestSetValue(pubsubserver, NID_PUB_INT, 9874, logger)
        
        sleep(STATIC_CONF_PUB_INTERVAL)
        
        logger.add_test('Subscriber bool is changed ', True == pubsubserver.getValue(NID_SUB_BOOL))
        logger.add_test('Subscriber uint is changed ', 1245 == pubsubserver.getValue(NID_SUB_UINT))
        logger.add_test('Subscriber int is changed ', 9874 == pubsubserver.getValue(NID_SUB_INT))
        
    finally:
        pubsubserver.disconnect()
        logger.add_test('Not connected to OPCUA Server', not pubsubserver.isConnected())
        logger.finalize_report()




if __name__=='__main__':
    argparser = argparse.ArgumentParser()
    argparser.add_argument('--static', action='store_true', default=False,
                           help='Flag to indicates that Pub-Sub configuration is static. Default is false')
    args = argparser.parse_args()

    if args.static:
        testPubSubStaticConf()
    else:
        testPubSubDynamicConf()
