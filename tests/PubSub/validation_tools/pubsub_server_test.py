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

import sys
from time import sleep
from shutil import copyfile, move
import argparse

from opcua import ua
from tap_logger import TapLogger
from pubsub_server import PubSubServer, PubSubState
from sks_server import SKSServer
from pubsub_server_helpers import waitForEvent, helpTestStopStart, helpAssertState, helpConfigurationChangeAndStart, helpRestart, helpConfigurationChange

DEFAULT_URL = 'opc.tcp://localhost:4843'
SKS_URL = 'opc.tcp://localhost:4841'
NID_CONFIGURATION = u"ns=1;s=PubSubConfiguration"
NID_START_STOP = u"ns=1;s=PubSubStartStop"
NID_STATUS = u"ns=1;s=PubSubStatus"
NID_PUBLISHER= u"ns=1;s=Publisher"
NID_ACYCLIC_SEND = u"ns=1;s=AcyclicSend"
NID_ACYCLIC_SEND_STATUS= u"ns=1;s=AcyclicPubSendStatus"
NID_DSM_FILTERING = u"ns=1;s=DataSetMessageFiltering"
NID_DSM_FILTERING_STATUS = u"ns=1;s=DataSetMessageFilteringStatus"
NID_SUB_STRING = u"ns=1;s=SubString"
NID_SUB_BOOL = u"ns=1;s=SubBool"
NID_SUB_UINT16 = u"ns=1;s=SubUInt16"
NID_SUB_UINT32 = u"ns=1;s=SubUInt32"
NID_SUB_UINT = u"ns=1;s=SubUInt"
NID_SUB_INT = u"ns=1;s=SubInt"

NID_PUB_STRING = u"ns=1;s=PubString"
NID_PUB_BOOL = u"ns=1;s=PubBool"
NID_PUB_UINT16 = u"ns=1;s=PubUInt16"
NID_PUB_UINT32 = u"ns=1;s=PubUInt32"
NID_PUB_UINT = u"ns=1;s=PubUInt"
NID_PUB_INT = u"ns=1;s=PubInt"

DEFAULT_XML_PATH = 'config_pubsub_server.xml'

SKS_KEYLIFETIME_SEC = 5
SKS_NB_GENERATED_KEYS = 2
SKS_SECURITY_GROUP_ID = "1"

# PublishingInterval (seconds) of XML default configuration
DYN_CONF_PUB_INTERVAL = 1.0

PUBLISHER_METHOD_NOT_TRIGGERED = 0
PUBLISHER_METHOD_IN_PROGRESS = 1
PUBLISHER_METHOD_SUCCESS = 2
PUBLISHER_METHOD_ERROR = 3

NODE_VARIANT_TYPE = { NID_SUB_BOOL : ua.VariantType.Boolean,
                      NID_SUB_UINT16 : ua.VariantType.UInt16,
                      NID_SUB_UINT32 : ua.VariantType.UInt32,
                      NID_SUB_UINT : ua.VariantType.UInt64,
                      NID_SUB_INT : ua.VariantType.Int64,
                      NID_SUB_STRING : ua.VariantType.String,
                      NID_PUB_BOOL : ua.VariantType.Boolean,
                      NID_PUB_UINT16 : ua.VariantType.UInt16,
                      NID_PUB_UINT32 : ua.VariantType.UInt32,
                      NID_PUB_UINT : ua.VariantType.UInt64,
                      NID_PUB_INT : ua.VariantType.Int64,
                      NID_PUB_STRING : ua.VariantType.String }

XML_EMPTY = """<PubSub>
</PubSub>"""

XML_PUBLISHER_ONLY = """<PubSub>
    <connection address="opc.udp://232.1.2.100:4840" mode="publisher" publisherId="i=1">
        <message groupId="1" publishingInterval="1000" groupVersion="1">
            <dataset writerId="50">
                <variable nodeId="ns=1;s=PubBool" displayName="pubVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=PubUInt16" displayName="pubVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=PubInt" displayName="pubVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
</PubSub>"""

XML_SUBSCRIBER_ONLY = """<PubSub>
    <connection address="opc.udp://232.1.2.100:4840" mode="subscriber">
        <message groupId="1" publishingInterval="1000" groupVersion="1" publisherId="i=1">
            <dataset writerId="50">
                <variable nodeId="ns=1;s=SubBool" displayName="subVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=SubUInt16" displayName="subVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=SubInt" displayName="subVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
</PubSub>"""

XML_PUBSUB_LOOP = """<PubSub>
    <connection address="opc.udp://232.1.2.100:4840" mode="publisher" publisherId="i=1">
        <message groupId="1" publishingInterval="200" groupVersion="1">
            <dataset writerId="50">
                <variable nodeId="ns=1;s=PubBool" displayName="pubVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=PubUInt16" displayName="pubVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=PubInt" displayName="pubVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
    <connection address="opc.udp://232.1.2.100:4840" mode="subscriber">
        <message groupId="1" publishingInterval="200" groupVersion="1" publisherId="i=1">
            <dataset writerId="50">
                <variable nodeId="ns=1;s=SubBool" displayName="subVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=SubUInt16" displayName="subVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=SubInt" displayName="subVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
</PubSub>"""

XML_PUBSUB_UNICAST = """<PubSub>
    <connection address="opc.udp://127.0.0.1:4840" mode="publisher" publisherId="i=1">
        <message groupId="1" publishingInterval="200" groupVersion="1">
            <dataset writerId="50">
                <variable nodeId="ns=1;s=PubBool" displayName="pubVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=PubUInt16" displayName="pubVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=PubInt" displayName="pubVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
    <connection address="opc.udp://127.0.0.1:4840" mode="subscriber">
        <message groupId="1" publishingInterval="200" groupVersion="1" publisherId="i=1">
            <dataset writerId="50">
                <variable nodeId="ns=1;s=SubBool" displayName="subVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=SubUInt16" displayName="subVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=SubInt" displayName="subVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
</PubSub>"""

XML_PUBSUB_LOOP_MQTT = """<PubSub>
    <connection address="mqtt://127.0.0.1:1883" mode="publisher" publisherId="i=1">
        <message groupId="1" publishingInterval="1000" groupVersion="1" mqttTopic="S2OPC">
            <dataset writerId="50">
                <variable nodeId="ns=1;s=PubBool" displayName="pubVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=PubUInt16" displayName="pubVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=PubInt" displayName="pubVarInt" dataType="Int64" />
                <variable nodeId="ns=1;s=PubString" displayName="pubVarString" dataType="String"/>
            </dataset>
        </message>
    </connection>
    <connection address="mqtt://127.0.0.1:1883" mode="subscriber">
        <message groupId="1" publishingInterval="1000" groupVersion="1" publisherId="i=1" mqttTopic="S2OPC">
            <dataset writerId="50">
                <variable nodeId="ns=1;s=SubBool" displayName="subVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=SubUInt16" displayName="subVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=SubInt" displayName="subVarInt" dataType="Int64" />
                <variable nodeId="ns=1;s=SubString" displayName="subVarString" dataType="String"/>
            </dataset>
        </message>
    </connection>
</PubSub>"""

XML_PUBSUB_LOOP_MQTT_VARIOUS_TOPIC = """<PubSub>
    <connection address="mqtt://127.0.0.1:1883" mode="publisher" publisherId="i=1">
        <message groupId="1" publishingInterval="1000" groupVersion="1" mqttTopic="S2OPC">
            <dataset writerId="50">
                <variable nodeId="ns=1;s=PubBool" displayName="pubVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=PubUInt16" displayName="pubVarUInt16" dataType="UInt16" />
            </dataset>
        </message>
        <message groupId="2" publishingInterval="1000" groupVersion="1" mqttTopic="Test">
            <dataset writerId="51">
                <variable nodeId="ns=1;s=PubInt" displayName="pubVarInt" dataType="Int64" />
                <variable nodeId="ns=1;s=PubString" displayName="pubVarString" dataType="String"/>
            </dataset>
        </message>
    </connection>
    <connection address="mqtt://127.0.0.1:1883" mode="subscriber">
        <message groupId="1" publishingInterval="1000" groupVersion="1" publisherId="i=1" mqttTopic="S2OPC" >
            <dataset writerId="50" >
                <variable nodeId="ns=1;s=SubBool" displayName="subVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=SubUInt16" displayName="subVarUInt16" dataType="UInt16" />
            </dataset>
        </message>
        <message groupId="2" publishingInterval="1000" groupVersion="1" publisherId="i=1" mqttTopic="Test">
            <dataset writerId="51">
                <variable nodeId="ns=1;s=SubInt" displayName="subVarInt" dataType="Int64" />
                <variable nodeId="ns=1;s=SubString" displayName="subVarString" dataType="String"/>
            </dataset>
        </message>
    </connection>
</PubSub>"""

XML_PUBSUB_LOOP_MQTT_SECU = """<PubSub>
    <connection address="mqtt://127.0.0.1:1883" mode="publisher" publisherId="i=1" mqttUsername="user1" mqttPassword="password">
        <message groupId="1" publishingInterval="1000" groupVersion="1" mqttTopic="S2OPC">
            <dataset writerId="50">
                <variable nodeId="ns=1;s=PubBool" displayName="pubVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=PubUInt16" displayName="pubVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=PubInt" displayName="pubVarInt" dataType="Int64" />
                <variable nodeId="ns=1;s=PubString" displayName="pubVarString" dataType="String"/>
            </dataset>
        </message>
    </connection>
    <connection address="mqtt://127.0.0.1:1883" mode="subscriber" mqttUsername="user1" mqttPassword="password" >
        <message groupId="1" publishingInterval="1000" groupVersion="1" publisherId="i=1" mqttTopic="S2OPC">
            <dataset writerId="50">
                <variable nodeId="ns=1;s=SubBool" displayName="subVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=SubUInt16" displayName="subVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=SubInt" displayName="subVarInt" dataType="Int64" />
                <variable nodeId="ns=1;s=SubString" displayName="subVarString" dataType="String"/>
            </dataset>
        </message>
    </connection>
</PubSub>"""

XML_PUBSUB_LOOP_MQTT_SECU_FAIL = """<PubSub>
    <connection address="mqtt://127.0.0.1:1883" mode="publisher" publisherId="i=1" mqttUsername="user1" mqttPassword="password">
        <message groupId="1" publishingInterval="1000" groupVersion="1" mqttTopic="S2OPC">
            <dataset writerId="50">
                <variable nodeId="ns=1;s=PubBool" displayName="pubVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=PubUInt16" displayName="pubVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=PubInt" displayName="pubVarInt" dataType="Int64" />
                <variable nodeId="ns=1;s=PubString" displayName="pubVarString" dataType="String"/>
            </dataset>
        </message>
    </connection>
    <connection address="mqtt://127.0.0.1:1883" mode="subscriber" mqttUsername="user" mqttPassword="passwords">
        <message groupId="1" publishingInterval="1000" groupVersion="1" publisherId="i=1" mqttTopic="S2OPC">
            <dataset writerId="50">
                <variable nodeId="ns=1;s=SubBool" displayName="subVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=SubUInt16" displayName="subVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=SubInt" displayName="subVarInt" dataType="Int64" />
                <variable nodeId="ns=1;s=SubString" displayName="subVarString" dataType="String"/>
            </dataset>
        </message>
    </connection>
</PubSub>"""

XML_PUBSUB_LOOP_SECU_ENCRYPT_SIGN_SUCCEED = """<PubSub>
    <connection address="opc.udp://232.1.2.100:4840" mode="publisher" publisherId="i=1">
        <message groupId="1" publishingInterval="200" groupVersion="1" securityMode="signAndEncrypt" securityGroupId="1">
            <skserver endpointUrl="opc.tcp://localhost:4841" serverCertPath="./server_public/sks_server_2k_cert.der" />
            <dataset writerId="50">
                <variable nodeId="ns=1;s=PubBool" displayName="pubVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=PubUInt16" displayName="pubVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=PubInt" displayName="pubVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
    <connection address="opc.udp://232.1.2.100:4840" mode="subscriber">
        <message groupId="1" publishingInterval="200" groupVersion="1" publisherId="i=1" securityMode="signAndEncrypt" securityGroupId="1">
            <skserver endpointUrl="opc.tcp://localhost:4841" serverCertPath="./server_public/sks_server_2k_cert.der" />
            <dataset writerId="50">
                <variable nodeId="ns=1;s=SubBool" displayName="subVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=SubUInt16" displayName="subVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=SubInt" displayName="subVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
</PubSub>"""

XML_PUBSUB_LOOP_SECU_SIGN_SUCCEED = """<PubSub>
    <connection address="opc.udp://232.1.2.100:4840" mode="publisher" publisherId="i=1">
        <message groupId="1" publishingInterval="200" groupVersion="1" securityMode="sign" securityGroupId="1">
            <skserver endpointUrl="opc.tcp://localhost:4841" serverCertPath="./server_public/sks_server_2k_cert.der" />
            <dataset writerId="50">>
                <variable nodeId="ns=1;s=PubBool" displayName="pubVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=PubUInt16" displayName="pubVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=PubInt" displayName="pubVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
    <connection address="opc.udp://232.1.2.100:4840" mode="subscriber">
        <message groupId="1" publishingInterval="200" groupVersion="1" publisherId="i=1" securityMode="sign" securityGroupId="1">
            <skserver endpointUrl="opc.tcp://localhost:4841" serverCertPath="./server_public/sks_server_2k_cert.der" />
            <dataset writerId="50">
                <variable nodeId="ns=1;s=SubBool" displayName="subVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=SubUInt16" displayName="subVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=SubInt" displayName="subVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
</PubSub>"""

XML_PUBSUB_LOOP_SECU_SKS_FALLBACK_SUCCEED = """<PubSub>
    <connection address="opc.udp://232.1.2.100:4840" mode="publisher" publisherId="i=1">
        <message groupId="1" publishingInterval="200" groupVersion="1" securityMode="signAndEncrypt" securityGroupId="1">
            <!-- no SK server defined => fallback mechanism use local key files -->
            <dataset writerId="50">
                <variable nodeId="ns=1;s=PubBool" displayName="pubVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=PubUInt16" displayName="pubVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=PubInt" displayName="pubVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
    <connection address="opc.udp://232.1.2.100:4840" mode="subscriber">
        <message groupId="1" publishingInterval="200" groupVersion="1" publisherId="i=1" securityMode="signAndEncrypt" securityGroupId="1">
            <!-- no SK server defined => fallback mechanism use local key files -->
            <dataset writerId="50">
                <variable nodeId="ns=1;s=SubBool" displayName="subVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=SubUInt16" displayName="subVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=SubInt" displayName="subVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
</PubSub>"""

XML_PUBSUB_LOOP_SECU_FAIL_1 = """<PubSub>
    <connection address="opc.udp://232.1.2.100:4840" mode="publisher" publisherId="i=1">
        <message groupId="1" publishingInterval="200" groupVersion="1" securityMode="signAndEncrypt" securityGroupId="1">
            <skserver endpointUrl="opc.tcp://localhost:4841" serverCertPath="./server_public/sks_server_2k_cert.der" />
            <dataset writerId="50">
                <variable nodeId="ns=1;s=PubBool" displayName="pubVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=PubUInt16" displayName="pubVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=PubInt" displayName="pubVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
    <connection address="opc.udp://232.1.2.100:4840" mode="subscriber">
        <message groupId="1" publishingInterval="200" groupVersion="1" publisherId="i=1">
            <dataset writerId="50">
                <variable nodeId="ns=1;s=SubBool" displayName="subVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=SubUInt16" displayName="subVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=SubInt" displayName="subVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
</PubSub>"""

XML_PUBSUB_LOOP_SECU_FAIL_2 = """<PubSub>
    <connection address="opc.udp://232.1.2.100:4840" mode="publisher" publisherId="i=1">
        <message groupId="1" publishingInterval="200" groupVersion="1" securityMode="signAndEncrypt" securityGroupId="1">
            <skserver endpointUrl="opc.tcp://localhost:4841" serverCertPath="./server_public/sks_server_2k_cert.der" />
            <dataset writerId="50">
                <variable nodeId="ns=1;s=PubBool" displayName="pubVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=PubUInt16" displayName="pubVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=PubInt" displayName="pubVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
    <connection address="opc.udp://232.1.2.100:4840" mode="subscriber">
        <message groupId="1" publishingInterval="200" groupVersion="1" publisherId="i=1" securityMode="sign" securityGroupId="1">
            <skserver endpointUrl="opc.tcp://localhost:4841" serverCertPath="./server_public/sks_server_2k_cert.der" />
            <dataset writerId="50">
                <variable nodeId="ns=1;s=SubBool" displayName="subVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=SubUInt16" displayName="subVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=SubInt" displayName="subVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
</PubSub>"""

XML_PUBSUB_LOOP_SECU_FAIL_3 = """<PubSub>
    <connection address="opc.udp://232.1.2.100:4840" mode="publisher" publisherId="i=1">
        <message groupId="1" publishingInterval="200" groupVersion="1" securityMode="sign" securityGroupId="1">
            <skserver endpointUrl="opc.tcp://localhost:4841" serverCertPath="./server_public/sks_server_2k_cert.der" />
            <dataset writerId="50">
                <variable nodeId="ns=1;s=PubBool" displayName="pubVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=PubUInt16" displayName="pubVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=PubInt" displayName="pubVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
    <connection address="opc.udp://232.1.2.100:4840" mode="subscriber">
        <message groupId="1" publishingInterval="200" groupVersion="1" publisherId="i=1" securityMode="signAndEncrypt" securityGroupId="1">
            <skserver endpointUrl="opc.tcp://localhost:4841" serverCertPath="./server_public/sks_server_2k_cert.der" />
            <dataset writerId="50">
                <variable nodeId="ns=1;s=SubBool" displayName="subVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=SubUInt16" displayName="subVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=SubInt" displayName="subVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
</PubSub>"""

XML_PUBSUB_LOOP_SECU_SIGN_FAIL_4 = """<PubSub>
    <connection address="opc.udp://232.1.2.100:4840" mode="publisher" publisherId="i=1">
        <message groupId="1" publishingInterval="200" groupVersion="1" securityMode="sign" securityGroupId="1">
            <skserver endpointUrl="opc.tcp://localhost:4841" serverCertPath="./server_public/sks_server_2k_cert.der" />
            <skserver endpointUrl="opc.tcp://localhost:4841" serverCertPath="./server_public/sks_server_2k_cert.der" />
            <dataset writerId="50">
                <variable nodeId="ns=1;s=PubBool" displayName="pubVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=PubUInt16" displayName="pubVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=PubInt" displayName="pubVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
    <connection address="opc.udp://232.1.2.100:4840" mode="subscriber">
        <message groupId="1" publishingInterval="200" groupVersion="1" publisherId="i=1">
            <dataset writerId="50">
                <variable nodeId="ns=1;s=SubBool" displayName="subVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=SubUInt16" displayName="subVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=SubInt" displayName="subVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
</PubSub>"""

XML_PUBSUB_INVALID_DSM_WRITERID = """<PubSub>
    <connection address="opc.udp://232.1.2.100:4840" mode="publisher" publisherId="i=1">
        <message groupId="1" publishingInterval="200" groupVersion="1">
            <dataset writerId="1">
                <variable nodeId="ns=1;s=PubBool" displayName="pubVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=PubUInt16" displayName="pubVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=PubInt" displayName="pubVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
    <connection address="opc.udp://232.1.2.100:4840" mode="subscriber">
        <message groupId="1" publishingInterval="200" groupVersion="1" publisherId="i=1">
            <dataset writerId="1">
                <variable nodeId="ns=1;s=SubBool" displayName="subVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=SubUInt16" displayName="subVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=SubInt" displayName="subVarInt" dataType="Int64" />
            </dataset>
            <dataset>
                <variable nodeId="ns=2;s=SubBool" displayName="subVarBool" dataType="Boolean" />
                <variable nodeId="ns=2;s=SubUInt16" displayName="subVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=2;s=SubInt" displayName="subVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
</PubSub>"""

XML_PUBSUB_LOOP_NULL = """<PubSub>
    <connection address="opc.udp://232.1.2.100:4840" mode="publisher" publisherId="i=1">
        <message groupId="1" publishingInterval="200" groupVersion="1">
            <dataset writerId="50">
                <variable nodeId="ns=1;s=PubBool" displayName="pubVarBool" dataType="Null" />
                <variable nodeId="ns=1;s=PubUInt16" displayName="pubVarUInt16" dataType="Null" />
                <variable nodeId="ns=1;s=PubInt" displayName="pubVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
    <connection address="opc.udp://232.1.2.100:4840" mode="subscriber">
        <message groupId="1" publishingInterval="200" groupVersion="1" publisherId="i=1">
            <dataset writerId="50">
                <variable nodeId="ns=1;s=SubBool" displayName="subVarBool" dataType="Null" />
                <variable nodeId="ns=1;s=SubUInt16" displayName="subVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=SubInt" displayName="subVarInt" dataType="Null" />
            </dataset>
        </message>
    </connection>
</PubSub>"""

# Configuration with message on Subscriber side not consistent with the one on Publisher
XML_PUBSUB_LOOP_MESSAGE_NOT_COMPATIBLE = """<PubSub>
    <connection address="opc.udp://232.1.2.100:4840" mode="publisher" publisherId="i=1">
        <message groupId="1" publishingInterval="200" groupVersion="1">
            <dataset writerId="50">
                <variable nodeId="ns=1;s=PubBool" displayName="pubVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=PubUInt16" displayName="pubVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=PubInt" displayName="pubVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
    <connection address="opc.udp://232.1.2.100:4840" mode="subscriber">
        <message groupId="1" publishingInterval="200" groupVersion="1" publisherId="i=1">
            <dataset writerId="50">
                <variable nodeId="ns=1;s=SubUInt16" displayName="subVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=SubInt" displayName="subVarInt" dataType="Int64" />
                <variable nodeId="ns=1;s=SubBool" displayName="subVarBool" dataType="Boolean" />
            </dataset>
        </message>
    </connection>
</PubSub>"""

# Bad formed Configuration ( variabl instead of variable )
XML_PUBSUB_BAD_FORMED_CONFIGURATION = """<PubSub>
    <connection address="opc.udp://232.1.2.100:4840" mode="publisher" publisherId="i=1">
        <message groupId="1" publishingInterval="200" groupVersion="1">
            <dataset writerId="50">
                <variabl nodeId="ns=1;s=PubBool" displayName="pubVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=PubUInt16" displayName="pubVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=PubInt" displayName="pubVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
    <connection address="opc.udp://232.1.2.100:4840" mode="subscriber">
        <message groupId="1" publishingInterval="200" groupVersion="1" publisherId="i=1">
            <dataset writerId="50">
                <variable nodeId="ns=1;s=SubUInt16" displayName="subVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=SubInt" displayName="subVarInt" dataType="Int64" />
                <variable nodeId="ns=1;s=SubBool" displayName="subVarBool" dataType="Boolean" />
            </dataset>
        </message>
    </connection>
</PubSub>"""

XML_PUBSUB_MISMATCHING_WRITERID = """<PubSub>
    <connection address="opc.udp://232.1.2.100:4840" mode="publisher" publisherId="i=1">
        <message groupId="1" publishingInterval="200" groupVersion="1" securityMode="sign" securityGroupId="1">
            <!-- security wihtout SKS server => fallback mechanism -->
            <dataset writerId="4">
                <variable nodeId="ns=1;s=PubBool" displayName="pubVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=PubUInt16" displayName="pubVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=PubInt" displayName="pubVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
    <connection address="opc.udp://232.1.2.100:4840" mode="subscriber">
        <message groupId="1" publishingInterval="200" groupVersion="1" publisherId="i=1" securityMode="sign" securityGroupId="1">
            <!-- security wihtout SKS server => fallback mechanism -->
            <dataset writerId="5">
                <variable nodeId="ns=1;s=SubBool" displayName="subVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=SubUInt16" displayName="subVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=SubInt" displayName="subVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
</PubSub>"""

XML_PUBSUB_SUB_WRITER_ID_FILTER = """<PubSub>
    <connection address="opc.udp://232.1.2.100:4840" mode="publisher" publisherId="i=1">
        <message groupId="1" publishingInterval="200" groupVersion="1" securityMode="sign" securityGroupId="1">
            <!-- security wihtout SKS server => fallback mechanism -->
            <dataset writerId="4">
                <variable nodeId="ns=4;s=PubBool" displayName="pubVarBool" dataType="Boolean" />
                <variable nodeId="ns=4;s=PubUInt16" displayName="pubVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=4;s=PubInt" displayName="pubVarInt" dataType="Int64" />
            </dataset>
            <dataset writerId="56">
                <variable nodeId="ns=56;s=PubBool" displayName="pubVarBool" dataType="Boolean" />
                <variable nodeId="ns=56;s=PubUInt16" displayName="pubVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=56;s=PubInt" displayName="pubVarInt" dataType="Int64" />
            </dataset>
            <dataset writerId="1">
                <variable nodeId="ns=1;s=PubBool" displayName="pubVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=PubUInt16" displayName="pubVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=PubInt" displayName="pubVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
    <connection address="opc.udp://232.1.2.100:4840" mode="subscriber">
        <message groupId="1" publishingInterval="200" groupVersion="1" publisherId="i=1" securityMode="sign" securityGroupId="1">
            <!-- security wihtout SKS server => fallback mechanism -->
            <dataset writerId="2">
                <variable nodeId="ns=2;s=SubBool" displayName="subVarBool" dataType="Boolean" />
                <variable nodeId="ns=2;s=SubUInt16" displayName="subVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=2;s=SubInt" displayName="subVarInt" dataType="Int64" />
            </dataset>
            <dataset writerId="1">
                <variable nodeId="ns=1;s=SubBool" displayName="subVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=SubUInt16" displayName="subVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=SubInt" displayName="subVarInt" dataType="Int64" />
            </dataset>
            <dataset writerId="6">
                <variable nodeId="ns=1;s=SubUInt32" displayName="subUInt32" dataType="Int32" />
            </dataset>
        </message>
    </connection>
</PubSub>"""

XML_PUBSUB_NO_GROUP_ID = """<PubSub>
    <connection address="opc.udp://232.1.2.100:4840" mode="publisher" publisherId="i=1">
        <message publishingInterval="200" groupVersion="1" securityMode="sign" securityGroupId="1">
            <!-- security wihtout SKS server => fallback mechanism -->
            <dataset writerId="50">
                <variable nodeId="ns=1;s=PubBool" displayName="pubVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=PubUInt16" displayName="pubVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=PubInt" displayName="pubVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
    <connection address="opc.udp://232.1.2.100:4840" mode="subscriber">
        <message groupId="1" publishingInterval="200" groupVersion="1" publisherId="i=1" securityMode="sign" securityGroupId="1">
            <!-- security wihtout SKS server => fallback mechanism -->
            <dataset writerId="50">
                <variable nodeId="ns=1;s=SubBool" displayName="subVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=SubUInt16" displayName="subVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=SubInt" displayName="subVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
</PubSub>"""

XML_PUBSUB_DUPLICATE_WRITERID = """<PubSub>
    <connection address="opc.udp://232.1.2.100:4840" mode="publisher" publisherId="i=1">
        <message publishingInterval="200" groupId="1" groupVersion="1" securityMode="sign" securityGroupId="1">
            <!-- security wihtout SKS server => fallback mechanism -->
            <dataset writerId="2">
                <variable nodeId="ns=1;s=PubBool" displayName="pubVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=PubUInt16" displayName="pubVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=PubInt" displayName="pubVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
    <connection address="opc.udp://232.1.2.100:4840" mode="subscriber">
        <message groupId="1" publishingInterval="200" groupVersion="1" publisherId="i=1" securityMode="sign" securityGroupId="1">
            <!-- security wihtout SKS server => fallback mechanism -->
            <dataset writerId="2">
                <variable nodeId="ns=2;s=SubBool" displayName="subVarBool" dataType="Boolean" />
                <variable nodeId="ns=2;s=SubUInt16" displayName="subVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=2;s=SubInt" displayName="subVarInt" dataType="Int64" />
            </dataset>
            <dataset writerId="1">
                <variable nodeId="ns=1;s=SubBool" displayName="subVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=SubUInt16" displayName="subVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=SubInt" displayName="subVarInt" dataType="Int64" />
            </dataset>
            <dataset writerId="2">
                <variable nodeId="ns=1;s=SubUInt32" displayName="subUInt32" dataType="Int32" />
            </dataset>
        </message>
    </connection>
</PubSub>"""

XML_PUBSUB_LOOP_ACYCLIC = """<PubSub >
    <connection address="opc.udp://232.1.2.100:4840" acyclicPublisher="true" mode="publisher" publisherId="i=1">
        <message groupId="1" publishingInterval="200" keepAliveTime="200." groupVersion="1">
            <dataset writerId="50">
                <variable nodeId="ns=1;s=PubBool" displayName="pubVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=PubUInt16" displayName="pubVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=PubInt" displayName="pubVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
    <connection address="opc.udp://232.1.2.100:4840" mode="subscriber">
        <message groupId="1" publishingInterval="200" groupVersion="1" publisherId="i=1">
            <dataset writerId="50">
                <variable nodeId="ns=1;s=SubBool" displayName="subVarBool" dataType="Null" />
                <variable nodeId="ns=1;s=SubUInt16" displayName="subVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=SubInt" displayName="subVarInt" dataType="Null" />
            </dataset>
        </message>
    </connection>
</PubSub>
"""

XML_PUBSUB_LOOP_STRING_PUBID = """<PubSub>
    <connection address="opc.udp://232.1.2.100:4840" mode="publisher" publisherId="s=foo">
        <message groupId="1" publishingInterval="200" groupVersion="1">
            <dataset writerId="50">
                <variable nodeId="ns=1;s=PubBool" displayName="pubVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=PubUInt16" displayName="pubVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=PubInt" displayName="pubVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
    <connection address="opc.udp://232.1.2.100:4840" mode="subscriber">
        <message groupId="1" publishingInterval="200" groupVersion="1" publisherId="s=foo">
            <dataset writerId="50">
                <variable nodeId="ns=1;s=SubBool" displayName="subVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=SubUInt16" displayName="subVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=SubInt" displayName="subVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
</PubSub>
"""

XML_PUBSUB_LOOP_BAD_UINTEGER_PUBID = """<PubSub>
    <connection address="opc.udp://232.1.2.100:4840" mode="publisher" publisherId="i=foo">
        <message groupId="1" publishingInterval="200" groupVersion="1">
            <dataset writerId="50">
                <variable nodeId="ns=1;s=PubBool" displayName="pubVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=PubUInt16" displayName="pubVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=PubInt" displayName="pubVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
    <connection address="opc.udp://232.1.2.100:4840" mode="subscriber">
        <message groupId="1" publishingInterval="200" groupVersion="1" publisherId="s=foo">
            <dataset writerId="50">
                <variable nodeId="ns=1;s=SubBool" displayName="subVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=SubUInt16" displayName="subVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=SubInt" displayName="subVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
</PubSub>
"""

XML_PUBSUB_LOOP_FIXED_SIZE_ONE_DSM = """<PubSub>
    <connection address="opc.udp://232.1.2.100:4840" mode="publisher" publisherId="i=1">
        <message groupId="1" publishingInterval="200" groupVersion="1" securityMode="none" publisherFixedSize="true">
            <dataset writerId="50">
                <variable nodeId="ns=1;s=PubBool" displayName="pubVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=PubUInt16" displayName="pubVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=PubInt" displayName="pubVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
    <connection address="opc.udp://232.1.2.100:4840" mode="subscriber">
        <message groupId="1" publishingInterval="200" groupVersion="1" publisherId="i=1" securityMode="none">
            <dataset writerId="50">
                <variable nodeId="ns=1;s=SubBool" displayName="subVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=SubUInt16" displayName="subVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=SubInt" displayName="subVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
</PubSub>
"""

XML_PUBSUB_LOOP_FIXED_SIZE_MULTI_DSM = """<PubSub>
    <connection address="opc.udp://232.1.2.100:4840" mode="publisher" publisherId="i=1">
        <message groupId="1" publishingInterval="200" groupVersion="1" securityMode="none" publisherFixedSize="true">
            <dataset writerId="50">
                <variable nodeId="ns=1;s=PubBool" displayName="pubVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=PubUInt16" displayName="pubVarUInt16" dataType="UInt16" />
            </dataset>
            <dataset writerId="51">
                <variable nodeId="ns=1;s=PubInt" displayName="pubVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
    <connection address="opc.udp://232.1.2.100:4840" mode="subscriber">
        <message groupId="1" publishingInterval="200" groupVersion="1" publisherId="i=1" securityMode="none">
            <dataset writerId="50">
                <variable nodeId="ns=1;s=SubBool" displayName="subVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=SubUInt16" displayName="subVarUInt16" dataType="UInt16" />
            </dataset>
            <dataset writerId="51">
                <variable nodeId="ns=1;s=SubInt" displayName="subVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
</PubSub>
"""

XML_PUBSUB_LOOP_FIXED_SIZE_SIGNED = """<PubSub>
    <connection address="opc.udp://232.1.2.100:4840" mode="publisher" publisherId="i=1">
        <message groupId="1" publishingInterval="200" groupVersion="1" securityMode="sign" securityGroupId="1" publisherFixedSize="true">
            <skserver endpointUrl="opc.tcp://localhost:4841" serverCertPath="./server_public/sks_server_2k_cert.der" />
            <dataset writerId="50">>
                <variable nodeId="ns=1;s=PubBool" displayName="pubVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=PubUInt16" displayName="pubVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=PubInt" displayName="pubVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
    <connection address="opc.udp://232.1.2.100:4840" mode="subscriber">
        <message groupId="1" publishingInterval="200" groupVersion="1" publisherId="i=1" securityMode="sign" securityGroupId="1">
            <skserver endpointUrl="opc.tcp://localhost:4841" serverCertPath="./server_public/sks_server_2k_cert.der" />
            <dataset writerId="50">
                <variable nodeId="ns=1;s=SubBool" displayName="subVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=SubUInt16" displayName="subVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=SubInt" displayName="subVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
</PubSub>"""

XML_PUBSUB_LOOP_FIXED_SIZE_SECU_FAIL = """<PubSub>
    <connection address="opc.udp://232.1.2.100:4840" mode="publisher" publisherId="i=1">
        <message groupId="1" publishingInterval="200" groupVersion="1" securityMode="signAndEncrypt" securityGroupId="1" publisherFixedSize="true">
            <skserver endpointUrl="opc.tcp://localhost:4841" serverCertPath="./server_public/sks_server_2k_cert.der" />
            <dataset writerId="50">
                <variable nodeId="ns=1;s=PubBool" displayName="pubVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=PubUInt16" displayName="pubVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=PubInt" displayName="pubVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
</PubSub>
"""

XML_PUBSUB_LOOP_FIXED_SIZE_BAD_DATASET = """<PubSub>
    <connection address="opc.udp://232.1.2.100:4840" mode="publisher" publisherId="i=1">
        <message groupId="1" publishingInterval="200" groupVersion="1" securityMode="none" publisherFixedSize="true">
            <dataset writerId="50">
                <variable nodeId="ns=1;s=PubBool" displayName="pubVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=PubString" displayName="pubVarString" dataType="String" />
            </dataset>
            <dataset writerId="51">
                <variable nodeId="ns=1;s=PubInt" displayName="pubVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
</PubSub>
"""
XML_PUBSUB_GROUP_ID_FILTER="""<PubSub>
    <connection address="opc.udp://232.1.2.100:4840" mode="publisher" publisherId="i=1">
        <message publishingInterval="200" groupId="1" groupVersion="1" securityMode="none">
            <dataset writerId="1">
                <variable nodeId="ns=1;s=PubBool" displayName="pubVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=PubUInt16" displayName="pubVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=PubInt" displayName="pubVarInt" dataType="Int64" />
            </dataset>
        </message>
        <message publishingInterval="200" groupId="2" groupVersion="1" securityMode="none">
            <dataset writerId="1">
                <variable nodeId="ns=1;s=PubString" displayName="pubVarString" dataType="String" />
            </dataset>
        </message>
    </connection>
    <connection address="opc.udp://232.1.2.100:4840" mode="subscriber">
        <message groupId="1" publishingInterval="200" groupVersion="1" publisherId="i=1" securityMode="none">
            <dataset writerId="1">
                <variable nodeId="ns=1;s=SubBool" displayName="subVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=SubUInt16" displayName="subVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=SubInt" displayName="subVarInt" dataType="Int64" />
            </dataset>
        </message>
        <message groupId="2" publishingInterval="200" groupVersion="1" publisherId="i=1" securityMode="none">
            <dataset writerId="1">
                <variable nodeId="ns=1;s=SubString" displayName="subVarString" dataType="String" />
            </dataset>
        </message>
    </connection>
</PubSub>"""

XML_PUBSUB_DATASET_DYNAMIC_EMISSION="""<PubSub>
    <connection address="opc.udp://232.1.2.100:4840" mode="publisher" publisherId="i=1">
        <message groupId="1" publishingInterval="200" groupVersion="1" securityMode="none">
            <dataset writerId="50">
                <variable nodeId="ns=1;s=PubBool" displayName="pubVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=PubUInt16" displayName="pubVarUInt16" dataType="UInt16" />
            </dataset>
            <dataset writerId="51" enableEmission="false">
                <variable nodeId="ns=1;s=PubInt" displayName="pubVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
    <connection address="opc.udp://232.1.2.100:4840" mode="subscriber">
        <message groupId="1" publishingInterval="200" groupVersion="1" publisherId="i=1" securityMode="none">
            <dataset writerId="50">
                <variable nodeId="ns=1;s=SubBool" displayName="subVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=SubUInt16" displayName="subVarUInt16" dataType="UInt16" />
            </dataset>
            <dataset writerId="51">
                <variable nodeId="ns=1;s=SubInt" displayName="subVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
</PubSub>
"""

XML_PUBSUB_DATASET_DYNAMIC_EMISSION_DISABLE_ALL="""<PubSub>
    <connection address="opc.udp://232.1.2.100:4840" mode="publisher" publisherId="i=1">
        <message groupId="1" publishingInterval="200" groupVersion="1" securityMode="none">
            <dataset writerId="50" enableEmission="false">
                <variable nodeId="ns=1;s=PubBool" displayName="pubVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=PubUInt16" displayName="pubVarUInt16" dataType="UInt16" />
            </dataset>
            <dataset writerId="51" enableEmission="false">
                <variable nodeId="ns=1;s=PubInt" displayName="pubVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
    <connection address="opc.udp://232.1.2.100:4840" mode="subscriber">
        <message groupId="1" publishingInterval="200" groupVersion="1" publisherId="i=1" securityMode="none">
            <dataset writerId="50">
                <variable nodeId="ns=1;s=SubBool" displayName="subVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=SubUInt16" displayName="subVarUInt16" dataType="UInt16" />
            </dataset>
            <dataset writerId="51">
                <variable nodeId="ns=1;s=SubInt" displayName="subVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
</PubSub>
"""

XML_PUBSUB_DATASET_DYNAMIC_EMISSION_PREENCODE="""<PubSub>
    <connection address="opc.udp://232.1.2.100:4840" mode="publisher" publisherId="i=1">
        <message groupId="1" publishingInterval="200" groupVersion="1" securityMode="none" publisherFixedSize="true">
            <dataset writerId="50" enableEmission="true">
                <variable nodeId="ns=1;s=PubBool" displayName="pubVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=PubUInt16" displayName="pubVarUInt16" dataType="UInt16" />
            </dataset>
            <dataset writerId="51" enableEmission="false">
                <variable nodeId="ns=1;s=PubInt" displayName="pubVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
    <connection address="opc.udp://232.1.2.100:4840" mode="subscriber">
        <message groupId="1" publishingInterval="200" groupVersion="1" publisherId="i=1" securityMode="none">
            <dataset writerId="50">
                <variable nodeId="ns=1;s=SubBool" displayName="subVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=SubUInt16" displayName="subVarUInt16" dataType="UInt16" />
            </dataset>
            <dataset writerId="51">
                <variable nodeId="ns=1;s=SubInt" displayName="subVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
</PubSub>
"""

def helpTestSetValue(pPubsubserver, nodeId, value, pLogger):
    pPubsubserver.setValue(nodeId, NODE_VARIANT_TYPE[nodeId], value)
    expected = pPubsubserver.getValue(nodeId)
    pLogger.add_test('write in %s succeeded' % nodeId , expected == value)

def helpTestWaitAcyclicSendStatusChange(pPubsubServer,pLogger):
    state = pPubsubServer.getAcyclicSendStatus()
    while(PUBLISHER_METHOD_NOT_TRIGGERED == state or PUBLISHER_METHOD_IN_PROGRESS == state):
        sleep(0.1)
        state = pPubsubServer.getAcyclicSendStatus()
    pLogger.add_test('send request succeed', PUBLISHER_METHOD_SUCCESS == state)

def helpTestTriggerSendValue(pPubsubServer,value,pLogger):
    pPubsubServer.triggerAcyclicSend(value)
    acyclicSendStatus = waitForEvent(lambda: PUBLISHER_METHOD_SUCCESS == pPubsubServer.getAcyclicSendStatus())
    pLogger.add_test(f'Call to Method acyclic send succeed with state {pPubsubServer.getAcyclicSendStatus()} and expect {PUBLISHER_METHOD_SUCCESS}', acyclicSendStatus)

def helpTestDsmFilter(pPubsubServer,dataSetMessageIdentifier,enableEmission,pLogger):
    pPubsubServer.callFilteringDSM(dataSetMessageIdentifier, enableEmission)
    filteringDsmSuccess = waitForEvent(lambda: PUBLISHER_METHOD_SUCCESS == pPubsubServer.getFilteringDSMStatus())
    logger.add_test(f'Dsm filtering method call succeed with state {pPubsubServer.getFilteringDSMStatus()}', filteringDsmSuccess)

def helpAssertState(psserver, expected, pLogger):
    waitForEvent(lambda:psserver.getPubSubState() == expected)
    state = psserver.getPubSubState()
    pLogger.add_test(f'PubSub Module state is {state}, should be {expected}', state == expected)

def helperTestPubSubConnectionFail(pubsubserver, xmlfile, logger, possibleFail=False):

    # ensure to get some unique value in each test case
    try: pubsubserver._testContext += 1
    except: pubsubserver._testContext = 1
    # Stop server if already running
    pubsubserver.stop()
    helpTestStopStart(pubsubserver, False, logger)

    # Init Subscriber variables
    helpTestSetValue(pubsubserver, NID_SUB_BOOL, False, logger)
    helpTestSetValue(pubsubserver, NID_SUB_UINT16, 1456 + pubsubserver._testContext, logger)
    helpTestSetValue(pubsubserver, NID_SUB_INT, 123654, logger)
    waitForEvent(lambda:not pubsubserver.getValue(NID_SUB_BOOL))

    # Change Publisher variables
    helpTestSetValue(pubsubserver, NID_PUB_BOOL, True, logger)
    helpTestSetValue(pubsubserver, NID_PUB_UINT16, 456 + pubsubserver._testContext, logger)
    helpTestSetValue(pubsubserver, NID_PUB_INT, 789, logger)

    helpConfigurationChangeAndStart(DEFAULT_XML_PATH, pubsubserver, xmlfile, logger, possibleFail=possibleFail)
    boolIsTrue = waitForEvent(lambda:pubsubserver.getValue(NID_SUB_BOOL))  # Event not reached

    logger.add_test('Subscriber bool is not changed', not boolIsTrue)
    logger.add_test('Subscriber uint16 is not changed', 1456 + pubsubserver._testContext == pubsubserver.getValue(NID_SUB_UINT16))
    logger.add_test('Subscriber int is not changed', 123654 == pubsubserver.getValue(NID_SUB_INT))

    if not possibleFail:
        helpAssertState(pubsubserver, PubSubState.OPERATIONAL, logger)

def helperTestPubSubConnectionPass(pubsubserver, xmlfile, logger):

    # Stop server if already running
    pubsubserver.stop()
    helpTestStopStart(pubsubserver, False, logger)

    # Init Subscriber variables
    helpTestSetValue(pubsubserver, NID_SUB_BOOL, False, logger)
    helpTestSetValue(pubsubserver, NID_SUB_UINT16, 1456, logger)
    helpTestSetValue(pubsubserver, NID_SUB_INT, 123654, logger)

    # Change Publisher variables
    helpTestSetValue(pubsubserver, NID_PUB_BOOL, True, logger)
    helpTestSetValue(pubsubserver, NID_PUB_UINT16, 456, logger)
    helpTestSetValue(pubsubserver, NID_PUB_INT, 789, logger)

    helpConfigurationChangeAndStart(DEFAULT_XML_PATH, pubsubserver, xmlfile, logger)

    waitForEvent(lambda:pubsubserver.getValue(NID_SUB_BOOL))

    logger.add_test('Subscriber bool is changed', True == pubsubserver.getValue(NID_SUB_BOOL))
    logger.add_test('Subscriber uint16 is changed', 456 == pubsubserver.getValue(NID_SUB_UINT16))
    logger.add_test('Subscriber int is changed', 789 == pubsubserver.getValue(NID_SUB_INT))

    helpAssertState(pubsubserver, PubSubState.OPERATIONAL, logger)

def testPubSubDynamicConf(logger):

    pubsubserver = PubSubServer(DEFAULT_URL, NID_CONFIGURATION, NID_START_STOP, NID_STATUS, NID_PUBLISHER, NID_ACYCLIC_SEND,NID_ACYCLIC_SEND_STATUS, NID_DSM_FILTERING, NID_DSM_FILTERING_STATUS)

    def lSubBoolIsFalse():return False == pubsubserver.getValue(NID_SUB_BOOL)
    def lSubBoolIsTrue():return True == pubsubserver.getValue(NID_SUB_BOOL)
    def lPubBoolIsFalse():return False == pubsubserver.getValue(NID_PUB_BOOL)
    def lPubBoolIsTrue():return True == pubsubserver.getValue(NID_PUB_BOOL)

    defaultXml2Restore = False
    allTestsDone = False

    try:
        # backup of default XML file
        copyfile(DEFAULT_XML_PATH, DEFAULT_XML_PATH + ".bakup")
        defaultXml2Restore = True

        # secure channel connection
        pubsubserver.connect()

        # Stop the running PubSub Server
        pubsubserver.stop()
        #
        # TC 1 : Test with Publisher only configuration => only pub variables change
        #
        logger.begin_section("TC 1 : Publisher only")

        helpTestStopStart(pubsubserver, False, logger)

        helperTestPubSubConnectionFail(pubsubserver, XML_PUBLISHER_ONLY, logger, possibleFail=False)

        #
        # TC 2 : Test with Subscriber only configuration => no variables change
        #
        logger.begin_section("TC 2 : Subscriber only")

        helperTestPubSubConnectionFail(pubsubserver, XML_SUBSCRIBER_ONLY, logger, possibleFail=False)

        #
        # TC 3 : Test with Publisher and Subscriber configuration => subscriber variables change through Pub/Sub
        #
        logger.begin_section("TC 3 : Publisher Subscriber Loop")

        helperTestPubSubConnectionPass(pubsubserver, XML_PUBSUB_LOOP, logger)

        pubsubserver.stop()
        helpTestStopStart(pubsubserver, False, logger)

        #
        # TC 4 : Stop Pub/Sub module => subscriber variables do not change
        #
        logger.begin_section("TC 4 : Publisher Subscriber Loop")

        boolInit = pubsubserver.getValue(NID_SUB_BOOL)
        uint16Init = pubsubserver.getValue(NID_PUB_UINT16)
        intInit = pubsubserver.getValue(NID_PUB_INT)

        # Change Publisher variables
        helpTestSetValue(pubsubserver, NID_PUB_BOOL, not boolInit, logger)
        helpTestSetValue(pubsubserver, NID_PUB_UINT16, uint16Init + 100, logger)
        helpTestSetValue(pubsubserver, NID_PUB_INT, -intInit, logger)

        waitForEvent(lambda:  pubsubserver.getValue(NID_SUB_BOOL) != boolInit)
        logger.add_test('Subscriber bool is not changed', boolInit == pubsubserver.getValue(NID_SUB_BOOL))
        logger.add_test('Subscriber uint16 is not changed', uint16Init == pubsubserver.getValue(NID_SUB_UINT16))
        logger.add_test('Subscriber int is not changed', intInit == pubsubserver.getValue(NID_SUB_INT))

        # Start to change Subscriber variables
        pubsubserver.start()
        helpTestStopStart(pubsubserver, True, logger)
        waitForEvent(lambda:  pubsubserver.getValue(NID_SUB_BOOL) != boolInit)
        logger.add_test('Subscriber bool is changed', (not boolInit) == pubsubserver.getValue(NID_SUB_BOOL))
        logger.add_test('Subscriber uint16 is changed', uint16Init + 100 == pubsubserver.getValue(NID_SUB_UINT16))
        logger.add_test('Subscriber int is changed', -intInit == pubsubserver.getValue(NID_SUB_INT))

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

        helpConfigurationChangeAndStart(DEFAULT_XML_PATH, pubsubserver, XML_PUBSUB_LOOP_NULL, logger)
        waitForEvent(lSubBoolIsFalse)
        logger.add_test('Subscriber bool is changed', False == pubsubserver.getValue(NID_SUB_BOOL))
        logger.add_test('Subscriber uint16 is changed', 8500 == pubsubserver.getValue(NID_SUB_UINT16))
        logger.add_test('Subscriber int is changed', -300 == pubsubserver.getValue(NID_SUB_INT))

        pubsubserver.stop()
        helpTestStopStart(pubsubserver, False, logger)

        #
        # TC 6 : Test with Publisher and Subscriber configuration => subscriber variables change through Pub/Sub
        #
        logger.begin_section("TC 6 : Test with Publisher and Subscriber configuration (MQTT): variables change through Pub/Sub")

        # Init Publisher variables
        helpTestSetValue(pubsubserver, NID_PUB_BOOL, False, logger)
        helpTestSetValue(pubsubserver, NID_PUB_UINT16, 8500, logger)
        helpTestSetValue(pubsubserver, NID_PUB_INT, -300, logger)
        helpTestSetValue(pubsubserver, NID_PUB_STRING, "Test MQTT From Publisher", logger)

        # Init Subscriber variables
        helpTestSetValue(pubsubserver, NID_SUB_BOOL, True, logger)
        helpTestSetValue(pubsubserver, NID_SUB_UINT16, 500, logger)
        helpTestSetValue(pubsubserver, NID_SUB_INT, 100, logger)
        helpTestSetValue(pubsubserver, NID_SUB_STRING, "Test MQTT Not set", logger)
        waitForEvent(lSubBoolIsTrue)

        helpConfigurationChangeAndStart(DEFAULT_XML_PATH, pubsubserver, XML_PUBSUB_LOOP_MQTT, logger)
        waitForEvent(lSubBoolIsFalse)

        logger.add_test('Subscriber bool is changed', False == pubsubserver.getValue(NID_SUB_BOOL))
        logger.add_test('Subscriber uint16 is changed', 8500 == pubsubserver.getValue(NID_SUB_UINT16))
        logger.add_test('Subscriber int is changed', -300 == pubsubserver.getValue(NID_SUB_INT))
        logger.add_test('Subscriber string is changed', "Test MQTT From Publisher" == pubsubserver.getValue(NID_SUB_STRING))
        sleep(DYN_CONF_PUB_INTERVAL)

        pubsubserver.stop()
        helpTestStopStart(pubsubserver, False, logger)


        #
        # TC 7 : Test with configuration message not consistent between the one on Publisher
        #        and the one on Subscriber  => subscriber variables do not change
        #
        logger.begin_section("TC 7 : Publisher Subscriber not consistent")

        helperTestPubSubConnectionFail(pubsubserver, XML_PUBSUB_LOOP_MESSAGE_NOT_COMPATIBLE, logger)

        #
        # TC 8 : Test with a bad formed configuration => Pub/Sub status is 0
        #
        logger.begin_section("TC 8 : Bad formed configuration")
        helpConfigurationChange(pubsubserver, XML_PUBSUB_BAD_FORMED_CONFIGURATION, logger)

        pubsubserver.stop()
        helpTestStopStart(pubsubserver, False, logger)

        logger.add_test('Start Server', True)
        pubsubserver.start()
        helpAssertState(pubsubserver, PubSubState.DISABLED, logger)

        #
        # TC 9 : Start and Stop empty configuration
        #
        logger.begin_section("TC 9 : Empty configuration")
        helpConfigurationChange(pubsubserver, XML_EMPTY, logger)
        helpAssertState(pubsubserver, PubSubState.DISABLED, logger)

        #
        # TC 10 : Test with message configured with security mode encrypt and sign for publisher
        #         but no security for subscriber => subscriber variables do not change
        #
        logger.begin_section('TC 10 : Inconsistent security mode - publisher is signAndEncrypt, subscriber is None;')

        helperTestPubSubConnectionFail(pubsubserver, XML_PUBSUB_LOOP_SECU_FAIL_1, logger, possibleFail=True)

        helpAssertState(pubsubserver, PubSubState.OPERATIONAL, logger)

        #
        # TC 11 : Test with message configured with security mode encrypt and sign for publisher
        #         but only sign for subscriber => subscriber variables do not change
        #
        logger.begin_section('TC 11 : Inconsistent security mode - publisher is signAndEncrypt, subscriber is sign;')

        helperTestPubSubConnectionFail(pubsubserver, XML_PUBSUB_LOOP_SECU_FAIL_2, logger, possibleFail=True)

        helpAssertState(pubsubserver, PubSubState.OPERATIONAL, logger)

        #
        # TC 12 : Test with message configured with security mode sign for publisher
        #         but encrypt and sign for subscriber => subscriber variables do not change
        #
        logger.begin_section('TC 12 : Inconsistent security mode - publisher is sign, subscriber is signAndEncrypt;')

        helperTestPubSubConnectionFail(pubsubserver, XML_PUBSUB_LOOP_SECU_FAIL_3, logger, possibleFail=True)

        helpAssertState(pubsubserver, PubSubState.OPERATIONAL, logger)

        #
        # TC 13 : Test with message configured with security mode sign and encrypt
        #         for both publisher and subscriber => subscriber variables change
        #
        logger.begin_section("TC 13 : Security mode sign and encrypt")

        helperTestPubSubConnectionPass(pubsubserver, XML_PUBSUB_LOOP_SECU_ENCRYPT_SIGN_SUCCEED, logger)

        #
        # TC 13-bis : Test with message configured with security mode sign and encrypt
        #         for both publisher and subscriber => subscriber variables change
        #
        logger.begin_section("TC 13-bis : Security using fallback security key files")

        helperTestPubSubConnectionPass(pubsubserver, XML_PUBSUB_LOOP_SECU_SKS_FALLBACK_SUCCEED, logger)

        #
        # TC 14 : Test with message configured with security mode sign
        #         for both publisher and subscriber => subscriber variables change
        #
        logger.begin_section("TC 14 : Security mode sign")

        helperTestPubSubConnectionPass(pubsubserver, XML_PUBSUB_LOOP_SECU_SIGN_SUCCEED, logger)

        #
        # TC 15 : Test with message configured with security mode sign only
        #         for publisher => subscriber variables do not change
        #
        logger.begin_section('TC 15 : Inconsistent security mode - publisher is sign, subscriber is None;')

        helperTestPubSubConnectionFail(pubsubserver, XML_PUBSUB_LOOP_SECU_SIGN_FAIL_4, logger, possibleFail=True)

        #
        # TC 16 : Test with invalid mixing of DSM with null and non null writerId
        #         => subscriber variables do not change
        #
        logger.begin_section('TC 16 : Inconsistent writer ids ')

        helperTestPubSubConnectionFail(pubsubserver, XML_PUBSUB_INVALID_DSM_WRITERID, logger, possibleFail=True)

        #
        # TC 17 : Test with mismatching writerId
        #         => subscriber variables does not change
        #
        logger.begin_section("TC 17 : Mismatching writerId")

        helperTestPubSubConnectionFail(pubsubserver, XML_PUBSUB_MISMATCHING_WRITERID, logger)

        #
        # TC 18 : Test removed
        #
        logger.begin_section("TC 18 : Test removed")

        #
        # TC 19 : Test with second writer Id match writer Id on subscriber (1st is filtered out)
        #         => subscriber variables change
        #
        logger.begin_section("TC 19 : WriterId filter")

        helpTestSetValue(pubsubserver, NID_SUB_UINT32, 0x12345678, logger)
        helpTestSetValue(pubsubserver, NID_PUB_UINT32, 0xCAFECAFE, logger)
        helperTestPubSubConnectionPass(pubsubserver, XML_PUBSUB_SUB_WRITER_ID_FILTER, logger)

        # Also check that other variables are unchaged
        logger.add_test('Subscriber (NID_SUB_UINT32) bool is unchanged', 0x12345678 == pubsubserver.getValue(NID_SUB_UINT32))

        pubsubserver.stop()
        helpTestStopStart(pubsubserver, False, logger)

        # TC 20 : Test with duplicate writer Id on same group
        #         => server does not start
        #
        logger.begin_section("TC 20 : Duplicate writerId")
        # helperTestPubSubConnectionFail(pubsubserver, XML_PUBSUB_DUPLICATE_WRITERID, logger)

        helpConfigurationChange(pubsubserver, XML_PUBSUB_DUPLICATE_WRITERID, logger)
        helpRestart(pubsubserver, logger, possibleFail=True)

        helpAssertState(pubsubserver, PubSubState.DISABLED, logger)

        # TC 21 : Test with username and password from mqtt security  ==> subscriber variables change through Pub/Sub
        logger.begin_section("TC 21 : Test with Publisher and Subscriber security configuration (MQTT): variables change through Pub/Sub ")

        # Init Publisher variables
        helpTestSetValue(pubsubserver, NID_PUB_BOOL, False, logger)
        helpTestSetValue(pubsubserver, NID_PUB_UINT16, 8500, logger)
        helpTestSetValue(pubsubserver, NID_PUB_INT, -300, logger)
        helpTestSetValue(pubsubserver, NID_PUB_STRING, "Test MQTT From Publisher", logger)

        # Init Subscriber variables
        helpTestSetValue(pubsubserver, NID_SUB_BOOL, True, logger)
        helpTestSetValue(pubsubserver, NID_SUB_UINT16, 500, logger)
        helpTestSetValue(pubsubserver, NID_SUB_INT, 100, logger)
        helpTestSetValue(pubsubserver, NID_SUB_STRING, "Test MQTT Not set", logger)

        helpConfigurationChangeAndStart(DEFAULT_XML_PATH, pubsubserver, XML_PUBSUB_LOOP_MQTT_SECU, logger)

        subIsFalse = waitForEvent(lSubBoolIsFalse)

        logger.add_test('Subscriber bool is changed', subIsFalse)
        logger.add_test('Subscriber uint16 is changed', 8500 == pubsubserver.getValue(NID_SUB_UINT16))
        logger.add_test('Subscriber int is changed', -300 == pubsubserver.getValue(NID_SUB_INT))
        logger.add_test('Subscriber string is changed', "Test MQTT From Publisher" == pubsubserver.getValue(NID_SUB_STRING))
        sleep(DYN_CONF_PUB_INTERVAL)

        # TC 22 : Test with bad username and bad password from mqtt security
        logger.begin_section("TC 22 : Test with Publisher and Subscriber bad security configuration (MQTT) ")
        helperTestPubSubConnectionFail(pubsubserver, XML_PUBSUB_LOOP_MQTT_SECU_FAIL, logger, possibleFail=True)
        sleep(DYN_CONF_PUB_INTERVAL)

        # TC 23 : Test with various topics
        logger.begin_section("TC 23 : Test with Publisher and Subscriber various topic (MQTT)")

        # Init Publisher variables
        helpTestSetValue(pubsubserver, NID_PUB_BOOL, False, logger)
        helpTestSetValue(pubsubserver, NID_PUB_UINT16, 8500, logger)
        helpTestSetValue(pubsubserver, NID_PUB_INT, -300, logger)
        helpTestSetValue(pubsubserver, NID_PUB_STRING, "Test MQTT From Publisher", logger)

        # Init Subscriber variables
        helpTestSetValue(pubsubserver, NID_SUB_BOOL, True, logger)
        helpTestSetValue(pubsubserver, NID_SUB_UINT16, 500, logger)
        helpTestSetValue(pubsubserver, NID_SUB_INT, 100, logger)
        helpTestSetValue(pubsubserver, NID_SUB_STRING, "Test MQTT Not set", logger)

        helpConfigurationChangeAndStart(DEFAULT_XML_PATH, pubsubserver, XML_PUBSUB_LOOP_MQTT_VARIOUS_TOPIC, logger)

        subIsFalse = waitForEvent(lSubBoolIsFalse)

        logger.add_test('Subscriber bool is changed', subIsFalse)
        logger.add_test('Subscriber uint16 is changed', 8500 == pubsubserver.getValue(NID_SUB_UINT16))

        # There are 2 distinct messages. Ensure second message is received when checking
        subIntIsMinus300 = waitForEvent(lambda: -300 == pubsubserver.getValue(NID_SUB_INT))
        logger.add_test('Subscriber int is changed', subIntIsMinus300)
        logger.add_test('Subscriber string is changed', "Test MQTT From Publisher" == pubsubserver.getValue(NID_SUB_STRING))
        sleep(DYN_CONF_PUB_INTERVAL)

        pubsubserver.stop()
        helpTestStopStart(pubsubserver, False, logger)

        # TC 24 : Test acyclic publisher send
        logger.begin_section("TC 24 : Test with acyclic Publisher and Subscriber")

        # Init Publisher variables
        helpTestSetValue(pubsubserver, NID_PUB_BOOL, False, logger)
        helpTestSetValue(pubsubserver, NID_PUB_UINT16, 8500, logger)
        helpTestSetValue(pubsubserver, NID_PUB_INT, -300, logger)
        PubIsFalse = waitForEvent(lPubBoolIsFalse)
        logger.add_test('Init subscriber variables', PubIsFalse)

        # Init Subscriber variables
        helpTestSetValue(pubsubserver, NID_SUB_BOOL, True, logger)
        helpTestSetValue(pubsubserver, NID_SUB_UINT16, 500, logger)
        helpTestSetValue(pubsubserver, NID_SUB_INT, 100, logger)
        subIsTrue = waitForEvent(lSubBoolIsTrue)
        logger.add_test('Init subscriber variables', subIsTrue)

        # Wait some long time and check nothing happen
        helpConfigurationChangeAndStart(DEFAULT_XML_PATH, pubsubserver, XML_PUBSUB_LOOP_ACYCLIC, logger)
        subIsFalse = waitForEvent(lSubBoolIsFalse)
        logger.add_test('Subscriber bool is not changed', not subIsFalse)
        logger.add_test('Subscriber uint16 is not changed', 500 == pubsubserver.getValue(NID_SUB_UINT16))
        logger.add_test('Subscriber int is not changed', 100 == pubsubserver.getValue(NID_SUB_INT))

        # Send the message
        PublisherId = 1
        writerGroupId = 1
        networkMessageIdentifier = (PublisherId, writerGroupId)
        helpTestTriggerSendValue(pubsubserver,networkMessageIdentifier,logger)
        subIsFalse = waitForEvent(lSubBoolIsFalse)

        logger.add_test('Subscriber bool is not changed', subIsFalse)
        logger.add_test('Subscriber uint16 is changed', 8500 == pubsubserver.getValue(NID_SUB_UINT16))
        logger.add_test('Subscriber int is changed', -300 == pubsubserver.getValue(NID_SUB_INT))

        pubsubserver.stop()
        helpTestStopStart(pubsubserver, False, logger)

        # TC 25 : Test String publisher Id
        logger.begin_section("TC 25 : Test string publisher Id")

        # Init Publisher variables
        helpTestSetValue(pubsubserver, NID_PUB_BOOL, False, logger)
        helpTestSetValue(pubsubserver, NID_PUB_UINT16, 8500, logger)
        helpTestSetValue(pubsubserver, NID_PUB_INT, -300, logger)
        helpTestSetValue(pubsubserver, NID_PUB_STRING, "Test String pubId From Publisher", logger)

        # Init Subscriber variables
        helpTestSetValue(pubsubserver, NID_SUB_BOOL, True, logger)
        helpTestSetValue(pubsubserver, NID_SUB_UINT16, 500, logger)
        helpTestSetValue(pubsubserver, NID_SUB_INT, 100, logger)
        helpTestSetValue(pubsubserver, NID_SUB_STRING, "Test String pubId Not set", logger)

        helpConfigurationChangeAndStart(DEFAULT_XML_PATH, pubsubserver, XML_PUBSUB_LOOP_STRING_PUBID, logger)

        waitForEvent(lSubBoolIsFalse)

        logger.add_test('Subscriber bool is changed', False == pubsubserver.getValue(NID_SUB_BOOL))
        logger.add_test('Subscriber uint16 is changed', 8500 == pubsubserver.getValue(NID_SUB_UINT16))
        logger.add_test('Subscriber int is changed', -300 == pubsubserver.getValue(NID_SUB_INT))

        # TC 26 : Test malformed uinteger publisher Id
        logger.begin_section("TC 26 : Test malformed UInteger publisher Id")
        helperTestPubSubConnectionFail(pubsubserver, XML_PUBSUB_LOOP_BAD_UINTEGER_PUBID, logger, possibleFail=True)

        #
        # TC 27 : Test with Publisher and Subscriber configuration(unicast) => subscriber variables change through Pub/Sub
        #
        logger.begin_section("TC 27 : Publisher Subscriber Unicast")

        helperTestPubSubConnectionPass(pubsubserver, XML_PUBSUB_UNICAST, logger)

        # TC 28 : Test preencode mechanism
        logger.begin_section("TC 28 : Publisher fixed size buffer")

        helperTestPubSubConnectionPass(pubsubserver, XML_PUBSUB_LOOP_FIXED_SIZE_ONE_DSM, logger)

        # TC 29 : Test preencode mechanism with multi dataSetMessage
        logger.begin_section("TC 29 : Publisher fixed size buffer multi DataSetMessage")

        helperTestPubSubConnectionPass(pubsubserver, XML_PUBSUB_LOOP_FIXED_SIZE_MULTI_DSM, logger)

        # TC 30 : Test preencode mechanism with sign security
        logger.begin_section("TC 30 : Publisher fixed size buffer signing security mode")

        helperTestPubSubConnectionPass(pubsubserver, XML_PUBSUB_LOOP_FIXED_SIZE_SIGNED, logger)

        # TC 31 : Test preencode mechanism with sign and encrypt security
        logger.begin_section("TC 31 : Publisher fixed size buffer with sign and encrypt security fail")

        helperTestPubSubConnectionFail(pubsubserver, XML_PUBSUB_LOOP_FIXED_SIZE_SECU_FAIL, logger, possibleFail=True)

        # TC 32 : Test preencode mechanism failed with bad dataSet configuration
        logger.begin_section("TC 32 : Publisher fixed size buffer incompatible dataSetField configured")

        helpConfigurationChangeAndStart(DEFAULT_XML_PATH, pubsubserver, XML_PUBSUB_LOOP_FIXED_SIZE_BAD_DATASET, logger, possibleFail=True)

        waitForEvent(lambda: 3 == pubsubserver.getValue(NID_STATUS))

        pubsubserver.stop()
        helpTestStopStart(pubsubserver, False, logger)

        # TC 33 : Test Writer Group Id filtering
        logger.begin_section("TC 33 : Writer Group Id filtering")
        # Init Publisher variables
        helpTestSetValue(pubsubserver, NID_PUB_BOOL, False, logger)
        helpTestSetValue(pubsubserver, NID_PUB_UINT16, 8500, logger)
        helpTestSetValue(pubsubserver, NID_PUB_INT, -300, logger)
        helpTestSetValue(pubsubserver, NID_PUB_STRING, "Test from Publisher", logger)

        # Init Subscriber variables
        helpTestSetValue(pubsubserver, NID_SUB_BOOL, True, logger)
        helpTestSetValue(pubsubserver, NID_SUB_UINT16, 500, logger)
        helpTestSetValue(pubsubserver, NID_SUB_INT, 100, logger)
        helpTestSetValue(pubsubserver, NID_SUB_STRING, "Test not set", logger)

        helpConfigurationChangeAndStart(DEFAULT_XML_PATH, pubsubserver, XML_PUBSUB_GROUP_ID_FILTER, logger, possibleFail=False)

        waitForEvent(lSubBoolIsFalse)

        logger.add_test('Subscriber bool is changed', False == pubsubserver.getValue(NID_SUB_BOOL))
        logger.add_test('Subscriber uint16 is changed', 8500 == pubsubserver.getValue(NID_SUB_UINT16))
        logger.add_test('Subscriber int is changed', -300 == pubsubserver.getValue(NID_SUB_INT))
        logger.add_test('Subscriber string is changed', "Test from Publisher" == pubsubserver.getValue(NID_SUB_STRING))
        pubsubserver.stop()
        helpTestStopStart(pubsubserver, False, logger)

        # TC 33 : Test filtering DatasetMessage emission at configuration
        logger.begin_section("TC 33 : Test filtering dataSetMessage at configuration")

        # Change Publisher variables
        helpTestSetValue(pubsubserver, NID_PUB_BOOL, False, logger)
        helpTestSetValue(pubsubserver, NID_PUB_UINT16, 8500, logger)
        helpTestSetValue(pubsubserver, NID_PUB_INT, -300, logger)

        # Init Subscriber variables
        helpTestSetValue(pubsubserver, NID_SUB_BOOL, True, logger)
        helpTestSetValue(pubsubserver, NID_SUB_UINT16, 500, logger)
        helpTestSetValue(pubsubserver, NID_SUB_INT, 100, logger)

        helpConfigurationChangeAndStart(DEFAULT_XML_PATH, pubsubserver, XML_PUBSUB_DATASET_DYNAMIC_EMISSION, logger, possibleFail=False)
        boolIsFalse = waitForEvent(lambda:not pubsubserver.getValue(NID_SUB_BOOL))  # Event not reached

        logger.add_test('Subscriber bool is changed', boolIsFalse)
        logger.add_test('Subscriber uint16 is changed', 8500 == pubsubserver.getValue(NID_SUB_UINT16))
        logger.add_test('Subscriber int is not changed', 100 == pubsubserver.getValue(NID_SUB_INT))
        pubsubserver.stop()
        helpTestStopStart(pubsubserver, False, logger)

        # TC 34 : Test filtering DatasetMessage emission dynamically
        # Stop server if already running
        logger.begin_section("TC 34 : Test enabling dataSetMessage at runtime")
        # Change Publisher variables
        helpTestSetValue(pubsubserver, NID_PUB_BOOL, False, logger)
        helpTestSetValue(pubsubserver, NID_PUB_UINT16, 8500, logger)
        helpTestSetValue(pubsubserver, NID_PUB_INT, -300, logger)

        # Init Subscriber variables
        helpTestSetValue(pubsubserver, NID_SUB_BOOL, True, logger)
        helpTestSetValue(pubsubserver, NID_SUB_UINT16, 500, logger)
        helpTestSetValue(pubsubserver, NID_SUB_INT, 100, logger)

        helpConfigurationChangeAndStart(DEFAULT_XML_PATH, pubsubserver, XML_PUBSUB_DATASET_DYNAMIC_EMISSION, logger, possibleFail=False)
        boolIsTrue = waitForEvent(lambda:not pubsubserver.getValue(NID_SUB_BOOL))  # Event not reached

        logger.add_test('Subscriber bool is changed', boolIsTrue)
        logger.add_test('Subscriber uint16 is changed', 8500 == pubsubserver.getValue(NID_SUB_UINT16))
        logger.add_test('Subscriber int is not changed', 100 == pubsubserver.getValue(NID_SUB_INT))

        # Enable datasetMessage

        publisherId = 1
        writerGroupId = 1
        dataSetMessageId = 51
        # DSM to enable
        dataSetMessageIdentifier = (publisherId, writerGroupId, dataSetMessageId)
        helpTestDsmFilter(pubsubserver, dataSetMessageIdentifier, True, logger)

        subIntChange = waitForEvent(lambda:-300 == pubsubserver.getValue(NID_SUB_INT))
        logger.add_test('Subscriber int is changed', subIntChange)
        pubsubserver.stop()
        helpTestStopStart(pubsubserver, False, logger)

        # TC 35 : Test no packet is emmit when all DSM are disabled
        logger.begin_section("TC 35 : Test no packet emission when all DSM disabled")
        # Change Publisher variables
        helpTestSetValue(pubsubserver, NID_PUB_BOOL, False, logger)
        helpTestSetValue(pubsubserver, NID_PUB_UINT16, 8500, logger)
        helpTestSetValue(pubsubserver, NID_PUB_INT, -300, logger)

        # Init Subscriber variables
        helpTestSetValue(pubsubserver, NID_SUB_BOOL, True, logger)
        helpTestSetValue(pubsubserver, NID_SUB_UINT16, 500, logger)
        helpTestSetValue(pubsubserver, NID_SUB_INT, 100, logger)
        helpConfigurationChangeAndStart(DEFAULT_XML_PATH, pubsubserver, XML_PUBSUB_DATASET_DYNAMIC_EMISSION_DISABLE_ALL, logger, possibleFail=False)
        boolIsTrue = waitForEvent(lambda:not pubsubserver.getValue(NID_SUB_BOOL))  # Event shall not be reached

        logger.add_test('Subscriber bool is not changed',not boolIsTrue)
        logger.add_test('Subscriber uint16 is not changed', 500 == pubsubserver.getValue(NID_SUB_UINT16))
        logger.add_test('Subscriber int is not changed', 100 == pubsubserver.getValue(NID_SUB_INT))

        # TC 36 : Test disabling dsm emission dynamicaly
        logger.begin_section("TC 36 : Test disabling dataSetMessage at runtime")
        # Change Publisher variables
        helpTestSetValue(pubsubserver, NID_PUB_BOOL, False, logger)
        helpTestSetValue(pubsubserver, NID_PUB_UINT16, 8500, logger)
        helpTestSetValue(pubsubserver, NID_PUB_INT, -300, logger)

        # Init Subscriber variables
        helpTestSetValue(pubsubserver, NID_SUB_BOOL, True, logger)
        helpTestSetValue(pubsubserver, NID_SUB_UINT16, 500, logger)
        helpTestSetValue(pubsubserver, NID_SUB_INT, 100, logger)

        helpConfigurationChangeAndStart(DEFAULT_XML_PATH, pubsubserver, XML_PUBSUB_DATASET_DYNAMIC_EMISSION, logger, possibleFail=False)
        boolIsTrue = waitForEvent(lambda:not pubsubserver.getValue(NID_SUB_BOOL))  # Event not reached

        logger.add_test('Subscriber bool is changed', boolIsTrue)
        logger.add_test('Subscriber uint16 is changed', 8500 == pubsubserver.getValue(NID_SUB_UINT16))
        logger.add_test('Subscriber int is not changed', 100 == pubsubserver.getValue(NID_SUB_INT))

        # Disable dataSetMessage
        publisherId = 1
        writerGroupId = 1
        dataSetMessageId = 50
        dataSetMessageIdentifier = (publisherId, writerGroupId, dataSetMessageId)
        helpTestDsmFilter(pubsubserver, dataSetMessageIdentifier, False, logger)

        # Once disable publisher chnge value and check that it doesn't change in subscriber side
        helpTestSetValue(pubsubserver, NID_PUB_BOOL, True, logger)
        helpTestSetValue(pubsubserver, NID_PUB_UINT16, 600, logger)
        logger.add_test('Subscriber bool is not changed', not pubsubserver.getValue(NID_SUB_BOOL))
        logger.add_test('Subscriber uint16 is not changed', 8500 == pubsubserver.getValue(NID_SUB_UINT16))
        pubsubserver.stop()
        helpTestStopStart(pubsubserver, False, logger)

        # TC 37 : Test filtering DatasetMessage emission dynamically with preencode mechanism
        logger.begin_section("TC 37 : Test filtering dataSetMessage with preencode optimisation")
        # Change Publisher variables
        helpTestSetValue(pubsubserver, NID_PUB_BOOL, False, logger)
        helpTestSetValue(pubsubserver, NID_PUB_UINT16, 8500, logger)
        helpTestSetValue(pubsubserver, NID_PUB_INT, -300, logger)

        # Init Subscriber variables
        helpTestSetValue(pubsubserver, NID_SUB_BOOL, True, logger)
        helpTestSetValue(pubsubserver, NID_SUB_UINT16, 500, logger)
        helpTestSetValue(pubsubserver, NID_SUB_INT, 100, logger)

        helpConfigurationChangeAndStart(DEFAULT_XML_PATH, pubsubserver, XML_PUBSUB_DATASET_DYNAMIC_EMISSION_PREENCODE, logger, possibleFail=False)
        boolIsTrue = waitForEvent(lambda:not pubsubserver.getValue(NID_SUB_BOOL))  # Event not reached

        logger.add_test('Subscriber bool is changed', boolIsTrue)
        logger.add_test('Subscriber uint16 is changed', 8500 == pubsubserver.getValue(NID_SUB_UINT16))
        logger.add_test('Subscriber int is not changed', 100 == pubsubserver.getValue(NID_SUB_INT))

        # Enable dataSetMessage
        publisherId = 1
        writerGroupId = 1
        dataSetMessageId = 51
        dataSetMessageIdentifier = (publisherId, writerGroupId, dataSetMessageId)
        helpTestDsmFilter(pubsubserver, dataSetMessageIdentifier, True, logger)
        subIntChange = waitForEvent(lambda:-300 == pubsubserver.getValue(NID_SUB_INT))
        logger.add_test('Subscriber int is changed', subIntChange)

        # Disable dataSetMessage
        publisherId = 1
        writerGroupId = 1
        dataSetMessageId = 50
        dataSetMessageIdentifier = (publisherId, writerGroupId, dataSetMessageId)
        helpTestDsmFilter(pubsubserver, dataSetMessageIdentifier, False, logger)
        helpTestSetValue(pubsubserver, NID_PUB_BOOL, True, logger)
        helpTestSetValue(pubsubserver, NID_PUB_UINT16, 600, logger)
        logger.add_test('Subscriber bool is not changed', not pubsubserver.getValue(NID_SUB_BOOL))
        logger.add_test('Subscriber uint16 is not changed', 8500 == pubsubserver.getValue(NID_SUB_UINT16))
        pubsubserver.stop()
        helpTestStopStart(pubsubserver, False, logger)

        # TC 38 : Test communication with updated SecurityKeys
        logger.begin_section("TC 38 : Test communication with updated SecurityKeys")

        # Init Subscriber variables
        helpTestSetValue(pubsubserver, NID_SUB_BOOL, False, logger)
        helpTestSetValue(pubsubserver, NID_SUB_UINT16, 156, logger)
        helpTestSetValue(pubsubserver, NID_SUB_INT, 1654, logger)

        # Load and start Pubsub with secure config
        helpConfigurationChangeAndStart(DEFAULT_XML_PATH, pubsubserver, XML_PUBSUB_LOOP_SECU_ENCRYPT_SIGN_SUCCEED, logger)
        # Wait for the SKS to update the keys
        testSKSKeyLifetime(logger)

        # Change Publisher variables
        helpTestSetValue(pubsubserver, NID_PUB_BOOL, True, logger)
        helpTestSetValue(pubsubserver, NID_PUB_UINT16, 4588, logger)
        helpTestSetValue(pubsubserver, NID_PUB_INT, -27, logger)

        waitForEvent(lambda: True == pubsubserver.getValue(NID_SUB_BOOL))
        logger.add_test('Subscriber bool is changed', True == pubsubserver.getValue(NID_SUB_BOOL))
        waitForEvent(lambda: 4588 == pubsubserver.getValue(NID_SUB_UINT16))
        logger.add_test('Subscriber uint16 is changed', 4588 == pubsubserver.getValue(NID_SUB_UINT16))
        waitForEvent(lambda: -27 == pubsubserver.getValue(NID_SUB_INT))
        logger.add_test('Subscriber int is changed', -27 == pubsubserver.getValue(NID_SUB_INT))

        # Stop server
        pubsubserver.stop()
        helpTestStopStart(pubsubserver, False, logger)

        allTestsDone = True

    except Exception as e:
        logger.add_test('Received exception %s'%e, False)

    finally:
        # restore default XML file
        if defaultXml2Restore:
            move(DEFAULT_XML_PATH + ".bakup", DEFAULT_XML_PATH)

        logger.begin_section("TC 99 : End of test")
        pubsubserver.disconnect()
        logger.add_test('Not connected to OPCUA Server', not pubsubserver.isConnected())
        logger.add_test('All test executed.', allTestsDone)
        retcode = -1 if logger.has_failed_tests else 0
        logger.finalize_report()
        sys.exit(retcode)

# test with static configuration : data/config_pubsub_server.xml
def testPubSubStaticConf(logger):

    pubsubserver = PubSubServer(DEFAULT_URL, NID_CONFIGURATION, NID_START_STOP, NID_STATUS, NID_PUBLISHER, NID_ACYCLIC_SEND, NID_ACYCLIC_SEND_STATUS, NID_DSM_FILTERING, NID_DSM_FILTERING_STATUS)

    def lSubBoolIsFalse():return False == pubsubserver.getValue(NID_SUB_BOOL)
    def lSubBoolIsTrue():return True == pubsubserver.getValue(NID_SUB_BOOL)

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

        waitForEvent(lSubBoolIsFalse)

        logger.add_test('Subscriber bool is changed', False == pubsubserver.getValue(NID_SUB_BOOL))

        waitForEvent(lambda: 8500 == pubsubserver.getValue(NID_SUB_UINT))

        logger.add_test('Subscriber uint is changed', 8500 == pubsubserver.getValue(NID_SUB_UINT))
        logger.add_test('Subscriber int is changed', -300 == pubsubserver.getValue(NID_SUB_INT))


        # Set Publisher variables and test change in Subscriber variables
        helpTestSetValue(pubsubserver, NID_PUB_BOOL, True, logger)
        helpTestSetValue(pubsubserver, NID_PUB_UINT, 5800, logger)
        helpTestSetValue(pubsubserver, NID_PUB_INT, -30, logger)

        waitForEvent(lSubBoolIsTrue)

        logger.add_test('Subscriber bool is changed', True == pubsubserver.getValue(NID_SUB_BOOL))

        waitForEvent(lambda: 5800 == pubsubserver.getValue(NID_SUB_UINT))

        logger.add_test('Subscriber uint is changed', 5800 == pubsubserver.getValue(NID_SUB_UINT))
        logger.add_test('Subscriber int is changed', -30 == pubsubserver.getValue(NID_SUB_INT))


        #
        # TC 2 : Pub-Sub server is stopped => no change in Subscriber variables
        #
        logger.begin_section("TC 2 : Pub-Sub server is stopped")

        pubsubserver.stop()
        logger.add_test('PubSub Module is stopped' , not pubsubserver.isStart())

        helpTestSetValue(pubsubserver, NID_PUB_BOOL, False, logger)
        helpTestSetValue(pubsubserver, NID_PUB_UINT, 7777, logger)
        helpTestSetValue(pubsubserver, NID_PUB_INT, 123654, logger)

        waitForEvent(lSubBoolIsFalse)

        logger.add_test('Subscriber bool is not changed', True == pubsubserver.getValue(NID_SUB_BOOL))

        waitForEvent(lambda: 5800 != pubsubserver.getValue(NID_SUB_UINT))

        logger.add_test('Subscriber uint is not changed', 5800 == pubsubserver.getValue(NID_SUB_UINT))
        logger.add_test('Subscriber int is not changed', -30 == pubsubserver.getValue(NID_SUB_INT))

        #
        # TC 3 : Pub-Sub server is restarted => Subscriber variables changed
        #
        logger.begin_section("TC 3 : Pub-Sub server is restarted")

        pubsubserver.start()
        logger.add_test('PubSub Module is started' , pubsubserver.isStart())

        waitForEvent(lSubBoolIsFalse)

        logger.add_test('Subscriber bool is changed', False == pubsubserver.getValue(NID_SUB_BOOL))

        waitForEvent(lambda: 7777 == pubsubserver.getValue(NID_SUB_UINT))

        logger.add_test('Subscriber uint is changed', 7777 == pubsubserver.getValue(NID_SUB_UINT))
        logger.add_test('Subscriber int is changed', 123654 == pubsubserver.getValue(NID_SUB_INT))

        #
        # TC 4 : Change Pub-Sub server configuration => configuration cannot be changed in static mode
        #
        logger.begin_section("TC 4 : Change Pub-Sub server configuration")
        # Change Pub-Sub configuration node and check file is not change
        helpConfigurationChangeAndStart(DEFAULT_XML_PATH, pubsubserver, XML_SUBSCRIBER_ONLY, logger, static=True)

        # Check Pub-Sub module is still running
        helpTestSetValue(pubsubserver, NID_PUB_BOOL, True, logger)
        helpTestSetValue(pubsubserver, NID_PUB_UINT, 1245, logger)
        helpTestSetValue(pubsubserver, NID_PUB_INT, 9874, logger)

        waitForEvent(lSubBoolIsTrue)

        logger.add_test('Subscriber bool is changed', True == pubsubserver.getValue(NID_SUB_BOOL))

        waitForEvent(lambda: 1245 == pubsubserver.getValue(NID_SUB_UINT))

        logger.add_test('Subscriber uint is changed', 1245 == pubsubserver.getValue(NID_SUB_UINT))
        logger.add_test('Subscriber int is changed', 9874 == pubsubserver.getValue(NID_SUB_INT))

    except Exception as e:
        logger.add_test('Received exception %s'%e, False)

    finally:
        pubsubserver.disconnect()
        logger.add_test('Not connected to OPCUA Server', not pubsubserver.isConnected())
        retcode = -1 if logger.has_failed_tests else 0
        logger.finalize_report()
        sys.exit(retcode)

def testSKSKeyLifetime(logger):

    sksServer = SKSServer(SKS_URL)

    try:
        sksServer.connect()
        StartingTokenId = 0
        RequestedKeyCount = 1
        # We will wait SKS_NB_GENERATED_KEYS * SKS_KEYLIFETIME_SEC seconds to be sure that new keys generated
        # by the SKS are valid for communication + check if these keys are different from previous ones.
        key_prev, keyLifeTime_prev = sksServer.callGetSecurityKeys(SKS_SECURITY_GROUP_ID, StartingTokenId, RequestedKeyCount)
        for _ in range(SKS_NB_GENERATED_KEYS):
            sleep(SKS_KEYLIFETIME_SEC)
            key_next, keyLifeTime_next = sksServer.callGetSecurityKeys(SKS_SECURITY_GROUP_ID, StartingTokenId, RequestedKeyCount)
            logger.add_test('Key is changed', key_next != key_prev and keyLifeTime_next == 5)
            key_prev = key_next

    except Exception as e:
        logger.add_test('Received exception %s'%e, False)

    finally:
        sksServer.disconnect()

if __name__=='__main__':
    argparser = argparse.ArgumentParser()
    argparser.add_argument('--static', action='store_true', default=False,
                           help='Flag to indicates that Pub-Sub configuration is static. Default is false')
    argparser.add_argument('--tap', dest='tap', default='pubsub_server_test.tap', help='Set the TAP file name for tests results')
    argparser.add_argument('--verbose', action='store_true', default=False, help='Verbose mode. Default is false')
    args = argparser.parse_args()
    logger = TapLogger(args.tap, verbose=args.verbose)
    if args.static:
        testPubSubStaticConf(logger)
    else:
        testPubSubDynamicConf(logger)
