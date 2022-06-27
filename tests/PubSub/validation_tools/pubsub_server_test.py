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

DEFAULT_URI = 'opc.tcp://localhost:4841'
NID_CONFIGURATION = u"ns=1;s=PubSubConfiguration"
NID_START_STOP = u"ns=1;s=PubSubStartStop"
NID_STATUS = u"ns=1;s=PubSubStatus"

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

# PublishingInterval (seconds) of XML default configuration
STATIC_CONF_PUB_INTERVAL = 1.2
DYN_CONF_PUB_INTERVAL_1000 = 2.1
DYN_CONF_PUB_INTERVAL_200 = 0.5

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
    <connection address="opc.udp://232.1.2.100:4840" mode="publisher" publisherId="1">
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
        <message groupId="1" publishingInterval="1000" groupVersion="1" publisherId="1">
            <dataset writerId="50">
                <variable nodeId="ns=1;s=SubBool" displayName="subVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=SubUInt16" displayName="subVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=SubInt" displayName="subVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
</PubSub>"""

XML_PUBSUB_LOOP = """<PubSub>
    <connection address="opc.udp://232.1.2.100:4840" mode="publisher" publisherId="1">
        <message groupId="1" publishingInterval="200" groupVersion="1">
            <dataset writerId="50">
                <variable nodeId="ns=1;s=PubBool" displayName="pubVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=PubUInt16" displayName="pubVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=PubInt" displayName="pubVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
    <connection address="opc.udp://232.1.2.100:4840" mode="subscriber">
        <message groupId="1" publishingInterval="200" groupVersion="1" publisherId="1">
            <dataset writerId="50">
                <variable nodeId="ns=1;s=SubBool" displayName="subVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=SubUInt16" displayName="subVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=SubInt" displayName="subVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
</PubSub>"""

XML_PUBSUB_LOOP_MQTT = """<PubSub>
    <connection address="mqtts://127.0.0.1:1883" mode="publisher" publisherId="1">
        <message groupId="1" publishingInterval="1000" groupVersion="1">
            <dataset writerId="50">
                <variable nodeId="ns=1;s=PubBool" displayName="pubVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=PubUInt16" displayName="pubVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=PubInt" displayName="pubVarInt" dataType="Int64" />
                <variable nodeId="ns=1;s=PubString" displayName="pubVarString" dataType="String"/>
            </dataset>
        </message>
    </connection>
    <connection address="mqtts://127.0.0.1:1883" mode="subscriber">
        <message groupId="1" publishingInterval="1000" groupVersion="1" publisherId="1">
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
    <connection address="opc.udp://232.1.2.100:4840" mode="publisher" publisherId="1">
        <message groupId="1" publishingInterval="200" groupVersion="1" securityMode="signAndEncrypt">
            <dataset writerId="50">
                <variable nodeId="ns=1;s=PubBool" displayName="pubVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=PubUInt16" displayName="pubVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=PubInt" displayName="pubVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
    <connection address="opc.udp://232.1.2.100:4840" mode="subscriber">
        <message groupId="1" publishingInterval="200" groupVersion="1" publisherId="1" securityMode="signAndEncrypt">
            <dataset writerId="50">
                <variable nodeId="ns=1;s=SubBool" displayName="subVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=SubUInt16" displayName="subVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=SubInt" displayName="subVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
</PubSub>"""

XML_PUBSUB_LOOP_SECU_SIGN_SUCCEED = """<PubSub>
    <connection address="opc.udp://232.1.2.100:4840" mode="publisher" publisherId="1">
        <message groupId="1" publishingInterval="200" groupVersion="1" securityMode="sign">
            <dataset>
                <variable nodeId="ns=1;s=PubBool" displayName="pubVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=PubUInt16" displayName="pubVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=PubInt" displayName="pubVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
    <connection address="opc.udp://232.1.2.100:4840" mode="subscriber">
        <message groupId="1" publishingInterval="200" groupVersion="1" publisherId="1" securityMode="sign">
            <dataset>
                <variable nodeId="ns=1;s=SubBool" displayName="subVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=SubUInt16" displayName="subVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=SubInt" displayName="subVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
</PubSub>"""

XML_PUBSUB_LOOP_SECU_FAIL_1 = """<PubSub>
    <connection address="opc.udp://232.1.2.100:4840" mode="publisher" publisherId="1">
        <message groupId="1" publishingInterval="200" groupVersion="1" securityMode="signAndEncrypt">
            <dataset>
                <variable nodeId="ns=1;s=PubBool" displayName="pubVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=PubUInt16" displayName="pubVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=PubInt" displayName="pubVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
    <connection address="opc.udp://232.1.2.100:4840" mode="subscriber">
        <message groupId="1" publishingInterval="200" groupVersion="1" publisherId="1">
            <dataset>
                <variable nodeId="ns=1;s=SubBool" displayName="subVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=SubUInt16" displayName="subVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=SubInt" displayName="subVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
</PubSub>"""

XML_PUBSUB_LOOP_SECU_FAIL_2 = """<PubSub>
    <connection address="opc.udp://232.1.2.100:4840" mode="publisher" publisherId="1">
        <message groupId="1" publishingInterval="200" groupVersion="1" securityMode="signAndEncrypt">
            <dataset>
                <variable nodeId="ns=1;s=PubBool" displayName="pubVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=PubUInt16" displayName="pubVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=PubInt" displayName="pubVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
    <connection address="opc.udp://232.1.2.100:4840" mode="subscriber">
        <message groupId="1" publishingInterval="200" groupVersion="1" publisherId="1" securityMode="sign">
            <dataset>
                <variable nodeId="ns=1;s=SubBool" displayName="subVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=SubUInt16" displayName="subVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=SubInt" displayName="subVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
</PubSub>"""

XML_PUBSUB_LOOP_SECU_FAIL_3 = """<PubSub>
    <connection address="opc.udp://232.1.2.100:4840" mode="publisher" publisherId="1">
        <message groupId="1" publishingInterval="200" groupVersion="1" securityMode="sign">
            <dataset>
                <variable nodeId="ns=1;s=PubBool" displayName="pubVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=PubUInt16" displayName="pubVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=PubInt" displayName="pubVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
    <connection address="opc.udp://232.1.2.100:4840" mode="subscriber">
        <message groupId="1" publishingInterval="200" groupVersion="1" publisherId="1" securityMode="signAndEncrypt">
            <dataset>
                <variable nodeId="ns=1;s=SubBool" displayName="subVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=SubUInt16" displayName="subVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=SubInt" displayName="subVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
</PubSub>"""

XML_PUBSUB_LOOP_SECU_SIGN_FAIL_4 = """<PubSub>
    <connection address="opc.udp://232.1.2.100:4840" mode="publisher" publisherId="1">
        <message groupId="1" publishingInterval="200" groupVersion="1" securityMode="sign">
            <dataset>
                <variable nodeId="ns=1;s=PubBool" displayName="pubVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=PubUInt16" displayName="pubVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=PubInt" displayName="pubVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
    <connection address="opc.udp://232.1.2.100:4840" mode="subscriber">
        <message groupId="1" publishingInterval="200" groupVersion="1" publisherId="1">
            <dataset>
                <variable nodeId="ns=1;s=SubBool" displayName="subVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=SubUInt16" displayName="subVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=SubInt" displayName="subVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
</PubSub>"""

XML_PUBSUB_INVALID_DSM_WRITERID = """<PubSub>
    <connection address="opc.udp://232.1.2.100:4840" mode="publisher" publisherId="1">
        <message groupId="1" publishingInterval="200" groupVersion="1" securityMode="sign">
            <dataset writerId="1">
                <variable nodeId="ns=1;s=PubBool" displayName="pubVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=PubUInt16" displayName="pubVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=PubInt" displayName="pubVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
    <connection address="opc.udp://232.1.2.100:4840" mode="subscriber">
        <message groupId="1" publishingInterval="200" groupVersion="1" publisherId="1">
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
    <connection address="opc.udp://232.1.2.100:4840" mode="publisher" publisherId="1">
        <message groupId="1" publishingInterval="200" groupVersion="1">
            <dataset>
                <variable nodeId="ns=1;s=PubBool" displayName="pubVarBool" dataType="Null" />
                <variable nodeId="ns=1;s=PubUInt16" displayName="pubVarUInt16" dataType="Null" />
                <variable nodeId="ns=1;s=PubInt" displayName="pubVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
    <connection address="opc.udp://232.1.2.100:4840" mode="subscriber">
        <message groupId="1" publishingInterval="200" groupVersion="1" publisherId="1">
            <dataset>
                <variable nodeId="ns=1;s=SubBool" displayName="subVarBool" dataType="Null" />
                <variable nodeId="ns=1;s=SubUInt16" displayName="subVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=SubInt" displayName="subVarInt" dataType="Null" />
            </dataset>
        </message>
    </connection>
</PubSub>"""

# Configuration with message on Subscriber side not consistent with the one on Publisher
XML_PUBSUB_LOOP_MESSAGE_NOT_COMPATIBLE = """<PubSub>
    <connection address="opc.udp://232.1.2.100:4840" mode="publisher" publisherId="1">
        <message groupId="1" publishingInterval="200" groupVersion="1">
            <dataset>
                <variable nodeId="ns=1;s=PubBool" displayName="pubVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=PubUInt16" displayName="pubVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=PubInt" displayName="pubVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
    <connection address="opc.udp://232.1.2.100:4840" mode="subscriber">
        <message groupId="1" publishingInterval="200" groupVersion="1" publisherId="1">
            <dataset>
                <variable nodeId="ns=1;s=SubUInt16" displayName="subVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=SubInt" displayName="subVarInt" dataType="Int64" />
                <variable nodeId="ns=1;s=SubBool" displayName="subVarBool" dataType="Boolean" />
            </dataset>
        </message>
    </connection>
</PubSub>"""

# Bad formed Configuration ( variabl instead of variable )
XML_PUBSUB_BAD_FORMED_CONFIGURATION = """<PubSub>
    <connection address="opc.udp://232.1.2.100:4840" mode="publisher" publisherId="1">
        <message groupId="1" publishingInterval="200" groupVersion="1">
            <dataset>
                <variabl nodeId="ns=1;s=PubBool" displayName="pubVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=PubUInt16" displayName="pubVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=PubInt" displayName="pubVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
    <connection address="opc.udp://232.1.2.100:4840" mode="subscriber">
        <message groupId="1" publishingInterval="200" groupVersion="1" publisherId="1">
            <dataset>
                <variable nodeId="ns=1;s=SubUInt16" displayName="subVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=SubInt" displayName="subVarInt" dataType="Int64" />
                <variable nodeId="ns=1;s=SubBool" displayName="subVarBool" dataType="Boolean" />
            </dataset>
        </message>
    </connection>
</PubSub>"""

XML_PUBSUB_MISMATCHING_WRITERID = """<PubSub>
    <connection address="opc.udp://232.1.2.100:4840" mode="publisher" publisherId="1">
        <message groupId="1" publishingInterval="200" groupVersion="1" securityMode="sign">
            <dataset writerId="4">
                <variable nodeId="ns=1;s=PubBool" displayName="pubVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=PubUInt16" displayName="pubVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=PubInt" displayName="pubVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
    <connection address="opc.udp://232.1.2.100:4840" mode="subscriber">
        <message groupId="1" publishingInterval="200" groupVersion="1" publisherId="1" securityMode="sign">
            <dataset writerId="5">
                <variable nodeId="ns=1;s=SubBool" displayName="subVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=SubUInt16" displayName="subVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=SubInt" displayName="subVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
</PubSub>"""

XML_PUBSUB_NO_SUB_WRITER_ID = """<PubSub>
    <connection address="opc.udp://232.1.2.100:4840" mode="publisher" publisherId="1">
        <message groupId="1" publishingInterval="200" groupVersion="1" securityMode="sign">
            <dataset writerId="4">
                <variable nodeId="ns=1;s=PubBool" displayName="pubVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=PubUInt16" displayName="pubVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=PubInt" displayName="pubVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
    <connection address="opc.udp://232.1.2.100:4840" mode="subscriber">
        <message groupId="1" publishingInterval="200" groupVersion="1" publisherId="1" securityMode="sign">
            <dataset>
                <variable nodeId="ns=1;s=SubBool" displayName="subVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=SubUInt16" displayName="subVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=SubInt" displayName="subVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
</PubSub>"""

XML_PUBSUB_SUB_WRITER_ID_FILTER = """<PubSub>
    <connection address="opc.udp://232.1.2.100:4840" mode="publisher" publisherId="1">
        <message groupId="1" publishingInterval="200" groupVersion="1" securityMode="sign">
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
        <message groupId="1" publishingInterval="200" groupVersion="1" publisherId="1" securityMode="sign">
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
    <connection address="opc.udp://232.1.2.100:4840" mode="publisher" publisherId="1">
        <message publishingInterval="200" groupVersion="1" securityMode="sign">
            <dataset>
                <variable nodeId="ns=1;s=PubBool" displayName="pubVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=PubUInt16" displayName="pubVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=PubInt" displayName="pubVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
    <connection address="opc.udp://232.1.2.100:4840" mode="subscriber">
        <message groupId="1" publishingInterval="200" groupVersion="1" publisherId="1" securityMode="sign">
            <dataset>
                <variable nodeId="ns=1;s=SubBool" displayName="subVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=SubUInt16" displayName="subVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=SubInt" displayName="subVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
</PubSub>"""

XML_PUBSUB_DUPLICATE_WRITERID = """<PubSub>
    <connection address="opc.udp://232.1.2.100:4840" mode="publisher" publisherId="1">
        <message publishingInterval="200" groupId="1" groupVersion="1" securityMode="sign">
            <dataset>
                <variable nodeId="ns=1;s=PubBool" displayName="pubVarBool" dataType="Boolean" />
                <variable nodeId="ns=1;s=PubUInt16" displayName="pubVarUInt16" dataType="UInt16" />
                <variable nodeId="ns=1;s=PubInt" displayName="pubVarInt" dataType="Int64" />
            </dataset>
        </message>
    </connection>
    <connection address="opc.udp://232.1.2.100:4840" mode="subscriber">
        <message groupId="1" publishingInterval="200" groupVersion="1" publisherId="1" securityMode="sign">
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

# TODO: group these helpers in an helper class that wraps both the client to the pubsub_server and the logger instances
# Test connection and status depending of pStart command
def helpTestStopStart(pPubsubserver, pStart, pLogger, possibleFail=False):
    if not possibleFail:
        pLogger.add_test('Connected to pubsub_server', pPubsubserver.isConnected())
        if pStart:
            pLogger.add_test('PubSub Module is started' , pPubsubserver.isStart())
        else:
            pLogger.add_test('PubSub Module is stopped', not pPubsubserver.isStart())

    # TODO: for now "possibleFail" is in fact "expectedFail"
    if pStart:
        if not possibleFail:
            helpAssertState(pPubsubserver, PubSubState.OPERATIONAL, pLogger)
    else:
        helpAssertState(pPubsubserver, PubSubState.DISABLED, pLogger)

# Send stop command following by start command
def helpRestart(pPubsubserver, pLogger, possibleFail):
    pLogger.add_test('Stop Server', True)
    pPubsubserver.stop()
    helpTestStopStart(pPubsubserver, False, pLogger, possibleFail)

    pLogger.add_test('Start Server', True)
    pPubsubserver.start()
    helpTestStopStart(pPubsubserver, True, pLogger, possibleFail)

# Change the configuration
def helpConfigurationChange(pPubsubserver, pConfig, pLogger):
    pLogger.add_test('Set Configuration', True)
    pPubsubserver.setConfiguration(pConfig)
    pLogger.add_test('Connected to OPCUA Server', pPubsubserver.isConnected())
    pubsubconfiguration = pPubsubserver.getConfiguration()
    pLogger.add_test('PubSub Configuration Node is changed', pConfig == pubsubconfiguration)

# Change the configuration and restart PubSub Server
# Test connection, status and configuration
def helpConfigurationChangeAndStart(pPubsubserver, pConfig, pLogger, static=False, possibleFail=False):
    fd = open(DEFAULT_XML_PATH, "r")
    oldConfig = fd.read()
    fd.close()

    helpConfigurationChange(pPubsubserver, pConfig, pLogger)
    helpRestart(pPubsubserver, pLogger, possibleFail)

    # check configuration saved in default file
    if not possibleFail:
        fd = open(DEFAULT_XML_PATH, "r")
        if static:
            pLogger.add_test('Default PubSub Configuration file is not changed', oldConfig == fd.read())
        else:
            rd = fd.read()
            pLogger.add_test('Default PubSub Configuration file is changed', pConfig == rd)
            if not(pConfig == rd):
                pLogger.add_test(f"pConfig={pConfig}",True)
                pLogger.add_test(f"fd.read()={rd}",False)
        fd.close()

def helpTestSetValue(pPubsubserver, nodeId, value, pLogger):
    pPubsubserver.setValue(nodeId, NODE_VARIANT_TYPE[nodeId], value)
    expected = pPubsubserver.getValue(nodeId)
    pLogger.add_test('write in %s succeeded' % nodeId , expected == value)

def helpAssertState(psserver, expected, pLogger):
    state = psserver.getPubSubState()
    pLogger.add_test(f'PubSub Module state is {state}, should be {expected}', state == expected)

def helperTestPubSubConnectionFail(pubsubserver, xmlfile, logger, possibleFail=False):
    
    helpConfigurationChangeAndStart(pubsubserver, xmlfile, logger, possibleFail=possibleFail)
        
    # Init Subscriber variables
    helpTestSetValue(pubsubserver, NID_SUB_BOOL, False, logger)
    helpTestSetValue(pubsubserver, NID_SUB_UINT16, 1456, logger)
    helpTestSetValue(pubsubserver, NID_SUB_INT, 123654, logger)
    
    # Change Publisher variables
    helpTestSetValue(pubsubserver, NID_PUB_BOOL, True, logger)
    helpTestSetValue(pubsubserver, NID_PUB_UINT16, 456, logger)
    helpTestSetValue(pubsubserver, NID_PUB_INT, 789, logger)
    
    sleep(DYN_CONF_PUB_INTERVAL_200)
    logger.add_test('Subscriber bool is not changed', False == pubsubserver.getValue(NID_SUB_BOOL))
    logger.add_test('Subscriber uint16 is not changed', 1456 == pubsubserver.getValue(NID_SUB_UINT16))
    logger.add_test('Subscriber int is not changed', 123654 == pubsubserver.getValue(NID_SUB_INT))
    
    if not possibleFail:
        helpAssertState(pubsubserver, PubSubState.OPERATIONAL, logger)

def helperTestPubSubConnectionPass(pubsubserver, xmlfile, logger):
    
    helpConfigurationChangeAndStart(pubsubserver, xmlfile, logger)
    
    # Init Subscriber variables
    helpTestSetValue(pubsubserver, NID_SUB_BOOL, False, logger)
    helpTestSetValue(pubsubserver, NID_SUB_UINT16, 1456, logger)
    helpTestSetValue(pubsubserver, NID_SUB_INT, 123654, logger)
    
    # Change Publisher variables
    helpTestSetValue(pubsubserver, NID_PUB_BOOL, True, logger)
    helpTestSetValue(pubsubserver, NID_PUB_UINT16, 456, logger)
    helpTestSetValue(pubsubserver, NID_PUB_INT, 789, logger)
    
    sleep(DYN_CONF_PUB_INTERVAL_200)
    logger.add_test('Subscriber bool is changed', True == pubsubserver.getValue(NID_SUB_BOOL))
    logger.add_test('Subscriber uint16 is changed', 456 == pubsubserver.getValue(NID_SUB_UINT16))
    logger.add_test('Subscriber int is changed', 789 == pubsubserver.getValue(NID_SUB_INT))
        
    helpAssertState(pubsubserver, PubSubState.OPERATIONAL, logger)

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
        sleep(DYN_CONF_PUB_INTERVAL_1000)
        logger.add_test('Subscriber bool is not changed', True == pubsubserver.getValue(NID_SUB_BOOL))
        logger.add_test('Subscriber uint16 is not changed', 500 == pubsubserver.getValue(NID_SUB_UINT16))
        logger.add_test('Subscriber int is not changed', 100 == pubsubserver.getValue(NID_SUB_INT))

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
        sleep(DYN_CONF_PUB_INTERVAL_1000)
        logger.add_test('Subscriber bool is not changed', True == pubsubserver.getValue(NID_SUB_BOOL))
        logger.add_test('Subscriber uint16 is not changed', 500 == pubsubserver.getValue(NID_SUB_UINT16))
        logger.add_test('Subscriber int is not changed', 100 == pubsubserver.getValue(NID_SUB_INT))

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
        sleep(DYN_CONF_PUB_INTERVAL_200)
        logger.add_test('Subscriber bool is changed', False == pubsubserver.getValue(NID_SUB_BOOL))
        logger.add_test('Subscriber uint16 is changed', 1500 == pubsubserver.getValue(NID_SUB_UINT16))
        logger.add_test('Subscriber int is changed', -50 == pubsubserver.getValue(NID_SUB_INT))

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

        sleep(DYN_CONF_PUB_INTERVAL_200)
        logger.add_test('Subscriber bool is not changed', False == pubsubserver.getValue(NID_SUB_BOOL))
        logger.add_test('Subscriber uint16 is not changed', 1500 == pubsubserver.getValue(NID_SUB_UINT16))
        logger.add_test('Subscriber int is not changed', -50 == pubsubserver.getValue(NID_SUB_INT))

        # Start to change Subscriber variables
        pubsubserver.start()
        helpTestStopStart(pubsubserver, True, logger)
        sleep(DYN_CONF_PUB_INTERVAL_200)
        logger.add_test('Subscriber bool is changed', True == pubsubserver.getValue(NID_SUB_BOOL))
        logger.add_test('Subscriber uint16 is changed', 6500 == pubsubserver.getValue(NID_SUB_UINT16))
        logger.add_test('Subscriber int is changed', -600 == pubsubserver.getValue(NID_SUB_INT))

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
        sleep(DYN_CONF_PUB_INTERVAL_200)
        logger.add_test('Subscriber bool is changed', False == pubsubserver.getValue(NID_SUB_BOOL))
        logger.add_test('Subscriber uint16 is changed', 8500 == pubsubserver.getValue(NID_SUB_UINT16))
        logger.add_test('Subscriber int is changed', -300 == pubsubserver.getValue(NID_SUB_INT))

        pubsubserver.stop()
        helpTestStopStart(pubsubserver, False, logger)

        #
        # TC 6 : Test with Publisher and Subscriber configuration => subscriber variables change through Pub/Sub
        #
        logger.begin_section("TC 6 : Test with Publisher and Subscriber configuration (MQTT): variables change through Pub/Sub")

        helpConfigurationChangeAndStart(pubsubserver, XML_PUBSUB_LOOP_MQTT, logger)
        sleep(DYN_CONF_PUB_INTERVAL_1000)

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

        sleep(DYN_CONF_PUB_INTERVAL_1000)

        logger.add_test('Subscriber bool is changed', False == pubsubserver.getValue(NID_SUB_BOOL))
        logger.add_test('Subscriber uint16 is changed', 8500 == pubsubserver.getValue(NID_SUB_UINT16))
        logger.add_test('Subscriber int is changed', -300 == pubsubserver.getValue(NID_SUB_INT))
        logger.add_test('Subscriber string is changed', "Test MQTT From Publisher" == pubsubserver.getValue(NID_SUB_STRING))
        sleep(DYN_CONF_PUB_INTERVAL_1000)

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
        # TC 18 : Test with no writer Id on subscriber (no filter)
        #         => subscriber variables change
        #
        logger.begin_section("TC 18 : No writerId on subscriber")
        
        helperTestPubSubConnectionPass(pubsubserver, XML_PUBSUB_NO_SUB_WRITER_ID, logger)
        
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

    except Exception as e:
        logger.add_test('Received exception %s'%e, False)

    finally:
        # restore default XML file
        if defaultXml2Restore:
            move(DEFAULT_XML_PATH + ".bakup", DEFAULT_XML_PATH)

        pubsubserver.disconnect()
        logger.add_test('Not connected to OPCUA Server', not pubsubserver.isConnected())
        retcode = -1 if logger.has_failed_tests else 0
        logger.finalize_report()
        sys.exit(retcode)

# test with static configuration : data/config_pubsub_server.xml
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

        logger.add_test('Subscriber bool is changed', False == pubsubserver.getValue(NID_SUB_BOOL))
        logger.add_test('Subscriber uint is changed', 8500 == pubsubserver.getValue(NID_SUB_UINT))
        logger.add_test('Subscriber int is changed', -300 == pubsubserver.getValue(NID_SUB_INT))


        # Set Publisher variables and test change in Subscriber variables
        helpTestSetValue(pubsubserver, NID_PUB_BOOL, True, logger)
        helpTestSetValue(pubsubserver, NID_PUB_UINT, 5800, logger)
        helpTestSetValue(pubsubserver, NID_PUB_INT, -30, logger)

        sleep(STATIC_CONF_PUB_INTERVAL)

        logger.add_test('Subscriber bool is changed', True == pubsubserver.getValue(NID_SUB_BOOL))
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

        sleep(STATIC_CONF_PUB_INTERVAL)

        logger.add_test('Subscriber bool is changed', True == pubsubserver.getValue(NID_SUB_BOOL))
        logger.add_test('Subscriber uint is changed', 5800 == pubsubserver.getValue(NID_SUB_UINT))
        logger.add_test('Subscriber int is changed', -30 == pubsubserver.getValue(NID_SUB_INT))

        #
        # TC 3 : Pub-Sub server is restarted => Subscriber variables changed
        #
        logger.begin_section("TC 3 : Pub-Sub server is restarted")

        pubsubserver.start()
        logger.add_test('PubSub Module is started' , pubsubserver.isStart())

        sleep(STATIC_CONF_PUB_INTERVAL)

        logger.add_test('Subscriber bool is changed', False == pubsubserver.getValue(NID_SUB_BOOL))
        logger.add_test('Subscriber uint is changed', 7777 == pubsubserver.getValue(NID_SUB_UINT))
        logger.add_test('Subscriber int is changed', 123654 == pubsubserver.getValue(NID_SUB_INT))

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

        logger.add_test('Subscriber bool is changed', True == pubsubserver.getValue(NID_SUB_BOOL))
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




if __name__=='__main__':
    argparser = argparse.ArgumentParser()
    argparser.add_argument('--static', action='store_true', default=False,
                           help='Flag to indicates that Pub-Sub configuration is static. Default is false')
    args = argparser.parse_args()

    if args.static:
        testPubSubStaticConf()
    else:
        testPubSubDynamicConf()
