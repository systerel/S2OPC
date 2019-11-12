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

import xml.etree.ElementTree as ET
import argparse

TAG_PUBSUB = "PubSub"
ATTRIBUTE_PUBSUB_ID = "publisherId"

TAG_CONNECTION = "connection"
ATTRIBUTE_CONNECTION_ADDRESS = "address"
ATTRIBUTE_CONNECTION_MODE = "mode"
VALUE_CONNECTION_MODE_PUBLISHER = "publisher"
VALUE_CONNECTION_MODE_SUBSCRIBER = "subscriber"

TAG_MESSAGE = "message"
ATTRIBUTE_MESSAGE_ID = "id"
ATTRIBUTE_MESSAGE_VERSION = "version"
ATTRIBUTE_MESSAGE_INTERVAL = "publishingInterval"
ATTRIBUTE_MESSAGE_SECURITY_MODE = "securityMode"
VALUE_MESSAGE_SECURITY_MODE_NONE = "none"
VALUE_MESSAGE_SECURITY_MODE_SIGN = "sign"
VALUE_MESSAGE_SECURITY_MODE_SIGNANDENCRYPT = "signAndEncrypt"
ATTRIBUTE_MESSAGE_SECURITY_SKS_ADDR = "sksAddress"

ATTRIBUTE_MESSAGE_PUBLISHERID = "publisherId"

TAG_VARIABLE = "variable"
ATTRIBUTE_VARIABLE_NODEID = "nodeId"
ATTRIBUTE_VARIABLE_NAME = "displayName"
ATTRIBUTE_VARIABLE_TYPE="dataType"

TYPE_IDS = {"Null": "SOPC_Null_Id",
            "Boolean": "SOPC_Boolean_Id",
            "SByte": "SOPC_SByte_Id",
            "Byte": "SOPC_Byte_Id",
            "Int16": "SOPC_Int16_Id",
            "UInt16": "SOPC_UInt16_Id",
            "Int32": "SOPC_Int32_Id",
            "UInt32": "SOPC_UInt32_Id",
            "Int64": "SOPC_Int64_Id",
            "UInt64": "SOPC_UInt64_Id",
            "Float": "SOPC_Float_Id",
            "Double": "SOPC_Double_Id",
            "DateTime": "SOPC_DateTime_Id",
            "String": "SOPC_String_Id",
            "ByteString": "SOPC_ByteString_Id",
            "Guid": "SOPC_Guid_Id",
            "XmlElement": "SOPC_XmlElement_Id",
            "NodeId": "SOPC_NodeId_Id",
            "ExpandedNodeId": "SOPC_ExpandedNodeId_Id",
            "QualifiedName": "SOPC_QualifiedName_Id",
            "LocalizedText": "SOPC_LocalizedText_Id",
            "StatusCode": "SOPC_StatusCode_Id",
            "Structure": "SOPC_ExtensionObject_Id"}


##
# Used to define only once C variables and functions
##
IS_DEFINED_C_WRITER = False
IS_DEFINED_C_READER = False
IS_DEFINED_C_DATASET = False
DEFINE_C_SETPUBVARIABLEAT = False
DEFINE_C_SETSUBVARIABLEAT = False
DEFINE_C_SETSUBNBVARIABLES = False


class ResultFile:
    def __init__(self):
        self.data = ""

    def add(self, lines):
        self.data = self.data + lines




def getCSecurityMode(mode):
    assert mode in [VALUE_MESSAGE_SECURITY_MODE_NONE,
                    VALUE_MESSAGE_SECURITY_MODE_SIGN,
                    VALUE_MESSAGE_SECURITY_MODE_SIGNANDENCRYPT]

    if VALUE_MESSAGE_SECURITY_MODE_SIGN == mode:
        return "SOPC_SecurityMode_Sign"
    elif VALUE_MESSAGE_SECURITY_MODE_SIGNANDENCRYPT == mode:
        return "SOPC_SecurityMode_SignAndEncrypt"
    else:
        return "SOPC_SecurityMode_None"



def handleDoc(tree, result):
    pubSub = tree.getroot()
    handlePubSub(pubSub, result)

def handlePubSub(pubSub, result):
    if ATTRIBUTE_PUBSUB_ID in pubSub.keys():
        pubid = int(pubSub.get(ATTRIBUTE_PUBSUB_ID), 10)
        assert pubid > 0
    else:
        # Subscriber could have no publisher id
        pubid = -1

    pubConnections = pubSub.findall("./%s[@%s='%s']" % (TAG_CONNECTION, ATTRIBUTE_CONNECTION_MODE, VALUE_CONNECTION_MODE_PUBLISHER))
    subConnections = pubSub.findall("./%s[@%s='%s']" % (TAG_CONNECTION, ATTRIBUTE_CONNECTION_MODE, VALUE_CONNECTION_MODE_SUBSCRIBER))

    ##
    # Create configuration and define local variable
    ##
    result.add("""
SOPC_PubSubConfiguration* SOPC_PubSubConfig_GetStatic(void)
{
    bool alloc = true;
    SOPC_PubSubConfiguration* config = SOPC_PubSubConfiguration_Create();
    """)

    if len(pubConnections) + len(subConnections) > 0:
        result.add("""
    SOPC_PubSubConnection* connection;
    """ )

    if len(pubConnections) > 0:

        # publisher shall have a publisher id
        assert pubid > 0

        ##
        # PUBLISHERS
        ##
        result.add("""

    /* %d connection pub */
    alloc = SOPC_PubSubConfiguration_Allocate_PubConnection_Array(config, %d);
    """ % (len(pubConnections), len(pubConnections)))

        index = 0
        for connection in pubConnections:
            handlePubConnection(pubid, connection, index, result)
            index += 1

    # END len(pubConnections) > 0

    if len(subConnections) > 0:

        ##
        # SUBSCRIBERS
        ##

        result.add("""

    /* %d connection Sub */
    alloc = SOPC_PubSubConfiguration_Allocate_SubConnection_Array(config, %d);
    """ % (len(subConnections), len(subConnections)))

        index = 0
        for connection in subConnections:
            handleSubConnection(pubid, connection, index, result)
            index += 1
    # END len(pubConnections) > 0

    ##
    # Delete structure if failure and return
    ##
    result.add("""

    if (!alloc)
    {
        SOPC_PubSubConfiguration_Delete(config);
        return NULL;
    }

    return config;
}
    """)

def handlePubConnection(publisherId, connection, index, result):
    address = connection.get(ATTRIBUTE_CONNECTION_ADDRESS)
    messages = connection.findall("./%s" % TAG_MESSAGE)

    result.add("""
    /** connection pub %d **/
    """ % index)

    if len(messages) > 0:
        result.add("""
    if (alloc)
    {
        // Set publisher id and address
        connection = SOPC_PubSubConfiguration_Get_PubConnection_At(config, %d);
        SOPC_PubSubConnection_Set_PublisherId_UInteger(connection, %d);
        alloc = SOPC_PubSubConnection_Set_Address(connection, "%s");
    }

    if (alloc)
    {
        // %d pub messages
        alloc = SOPC_PubSubConnection_Allocate_WriterGroup_Array(connection, %d);
    }

    if (alloc)
    {
        // %d published data sets
        alloc = SOPC_PubSubConfiguration_Allocate_PublishedDataSet_Array(config, %d);
    }

        """ % (index, publisherId, address, len(messages), len(messages), len(messages), len(messages)))

        index = 0
        for message in messages:
            handlePubMessage(message, index, result)
            index += 1




def handlePubMessage(message, index, result):
    global IS_DEFINED_C_WRITER
    global IS_DEFINED_C_DATASET

    id = int(message.get(ATTRIBUTE_MESSAGE_ID), 10)
    version = int(message.get(ATTRIBUTE_MESSAGE_VERSION), 10)
    interval = float(message.get(ATTRIBUTE_MESSAGE_INTERVAL))
    securityMode = message.get(ATTRIBUTE_MESSAGE_SECURITY_MODE, VALUE_MESSAGE_SECURITY_MODE_NONE)

    sksAddress = message.get(ATTRIBUTE_MESSAGE_SECURITY_SKS_ADDR, None)
    sksAddress = 'NULL' if sksAddress is None else '"%s"' % sksAddress

    variables = message.findall(TAG_VARIABLE)

    result.add("""
    /*** Pub Message %d ***/
    """ % id)

    if not IS_DEFINED_C_WRITER:
        result.add("""
    SOPC_DataSetWriter* writer = NULL;
        """)
    IS_DEFINED_C_WRITER = True

    result.add("""
    if (alloc)
    {
        writer = SOPC_PubSubConfig_SetPubMessageAt(connection, %d, %d, %d, %f, %s, %s);
        alloc = NULL != writer;
    }
    """ % (index, id, version, interval, getCSecurityMode(securityMode), sksAddress))

    if len(variables) > 0:

        if not IS_DEFINED_C_DATASET:
            result.add("""
    SOPC_PublishedDataSet* dataset = NULL;
    """)
        IS_DEFINED_C_DATASET = True

        result.add("""
    if (alloc)
    {
        dataset = SOPC_PubSubConfig_InitDataSet(config, %d, writer, %d);
        alloc = NULL != dataset;
    }
    if (alloc)
    {
        """ % (index, len(variables)))
        index = 0
        for variable in variables:
            handleVariable("Pub", variable, index, result)
            index += 1

        result.add("""
    }
    """)

# type is Pub or Sub
def handleVariable(mode, variable, index, result):
    global DEFINE_C_SETPUBVARIABLEAT
    global DEFINE_C_SETSUBVARIABLEAT

    nodeId = variable.get(ATTRIBUTE_VARIABLE_NODEID)
    datatype = variable.get(ATTRIBUTE_VARIABLE_TYPE)
    assert datatype in TYPE_IDS
    dataid = TYPE_IDS[datatype]
    if mode == "Pub":
        result.add("""
        SOPC_PubSubConfig_SetPubVariableAt(dataset, %d, "%s", %s);""" % (index, nodeId, dataid))
        DEFINE_C_SETPUBVARIABLEAT = True
    else:
        result.add("""
        SOPC_PubSubConfig_SetSubVariableAt(reader, %d, "%s", %s);""" % (index, nodeId, dataid))
        DEFINE_C_SETSUBVARIABLEAT = True


def handleSubConnection(publisherId, connection, index, result):
        address = connection.get(ATTRIBUTE_CONNECTION_ADDRESS)
        messages = connection.findall("./%s" % TAG_MESSAGE)

        result.add("""
    /** connection sub %d **/
    """ % index)


        if len(messages) > 0:

            result.add("""
    if (alloc)
    {
        // Set subscriber id and address
        connection = SOPC_PubSubConfiguration_Get_SubConnection_At(config, %d);
        alloc = SOPC_PubSubConnection_Set_Address(connection, "%s");
    }

    if (alloc)
    {
        // %d sub messages
        alloc = SOPC_PubSubConnection_Allocate_ReaderGroup_Array(connection, %d);
    }

        """ % (index, address, len(messages), len(messages)))
            index = 0
            for message in messages:
                handleSubMessage(message, index, result)
                index += 1


def handleSubMessage(message, index, result):
    global IS_DEFINED_C_READER
    global DEFINE_C_SETSUBNBVARIABLES

    for attr in [ATTRIBUTE_MESSAGE_ID,
                 ATTRIBUTE_MESSAGE_VERSION,
                 ATTRIBUTE_MESSAGE_INTERVAL,
                 ATTRIBUTE_MESSAGE_PUBLISHERID]:
        assert attr in message.keys()
    id = int(message.get(ATTRIBUTE_MESSAGE_ID), 10)
    version = int(message.get(ATTRIBUTE_MESSAGE_VERSION), 10)
    interval = float(message.get(ATTRIBUTE_MESSAGE_INTERVAL))
    securityMode = message.get(ATTRIBUTE_MESSAGE_SECURITY_MODE, VALUE_MESSAGE_SECURITY_MODE_NONE)
    sksAddress = message.get(ATTRIBUTE_MESSAGE_SECURITY_SKS_ADDR, None)
    sksAddress = 'NULL' if sksAddress is None else '"%s"' % sksAddress
    publisherId = int(message.get(ATTRIBUTE_MESSAGE_PUBLISHERID), 10)
    variables = message.findall(TAG_VARIABLE)

    if not IS_DEFINED_C_READER:
        result.add("""
    SOPC_DataSetReader* reader = NULL;
        """)
    IS_DEFINED_C_READER = True;

    result.add("""
    /*** Sub Message %d ***/
    """ % id)

    result.add("""
    if (alloc)
    {
        reader = SOPC_PubSubConfig_SetSubMessageAt(connection, %d, %d, %d, %d, %f, %s, %s);
        alloc = NULL != reader;
    }
    """ % (index, publisherId, id, version, interval, getCSecurityMode(securityMode), sksAddress))

    if len(variables) > 0:

        DEFINE_C_SETSUBNBVARIABLES = True;

        result.add("""
    if (alloc)
    {
        alloc = SOPC_PubSubConfig_SetSubNbVariables(reader, %d);
    }
    if (alloc)
    {
        """ % len(variables))
        index = 0
        for variable in variables:
            handleVariable("Sub", variable, index, result)
            index += 1

        result.add("""
    }
    """)



def generate_pubsub_config(xml_file_in, c_file_out):


    tmpCFile = ResultFile()

    tree = ET.parse(xml_file_in)
    handleDoc(tree, tmpCFile)

    cFile = ResultFile()

    cFile.add("""/* Copyright (C) Systerel SAS 2019, all rights reserved. */

#include "pubsub_config_static.h"

#include <assert.h>
#include <stdbool.h>
#include <string.h>

""")

    if IS_DEFINED_C_WRITER:
        cFile.add("""

static SOPC_DataSetWriter* SOPC_PubSubConfig_SetPubMessageAt(SOPC_PubSubConnection* connection,
                                                             uint16_t index,
                                                             uint16_t messageId,
                                                             uint32_t version,
                                                             double interval,
                                                             SOPC_SecurityMode_Type securityMode,
                                                             char* sksAddress)
{
    bool alloc;
    OpcUa_EndpointDescription* endpoint = NULL;
    SOPC_DataSetWriter* writer = NULL;
    SOPC_WriterGroup* group = SOPC_PubSubConnection_Get_WriterGroup_At(connection, index);
    SOPC_WriterGroup_Set_Id(group, messageId);
    SOPC_WriterGroup_Set_Version(group, version);
    SOPC_WriterGroup_Set_PublishingInterval(group, interval);
    SOPC_WriterGroup_Set_SecurityMode(group, securityMode);

    alloc = SOPC_EndpointDescription_Create_From_URL(sksAddress, &endpoint);
    if(alloc && NULL != endpoint) {
        // only one endpoint is created
        SOPC_WriterGroup_Set_EndPointDescription(group, endpoint, 1);
    }

    // Create one DataSet Writer
    if (alloc)
    {
        alloc = SOPC_WriterGroup_Allocate_DataSetWriter_Array(group, 1);
    }
    if (alloc)
    {
        writer = SOPC_WriterGroup_Get_DataSetWriter_At(group, 0);
        alloc = NULL != writer;
    }
    if (alloc)
    {
        SOPC_DataSetWriter_Set_Id(writer, messageId);
    }
    return writer;
}


""")

    if IS_DEFINED_C_DATASET:
        cFile.add("""
static SOPC_PublishedDataSet* SOPC_PubSubConfig_InitDataSet(SOPC_PubSubConfiguration* config,
                                                            uint16_t dataSetIndex,
                                                            SOPC_DataSetWriter* writer,
                                                            uint16_t nbVar)
{
    SOPC_PublishedDataSet* dataset = SOPC_PubSubConfiguration_Get_PublishedDataSet_At(config, dataSetIndex);
    SOPC_PublishedDataSet_Init(dataset, SOPC_PublishedDataItemsDataType, nbVar);
    SOPC_DataSetWriter_Set_DataSet(writer, dataset);

    return dataset;
}

""")

    if DEFINE_C_SETPUBVARIABLEAT:
        cFile.add("""
static void SOPC_PubSubConfig_SetPubVariableAt(SOPC_PublishedDataSet* dataset,
                                               uint16_t index,
                                               char* strNodeId,
                                               SOPC_BuiltinId builtinType)
{
    SOPC_FieldMetaData* fieldmetadata = SOPC_PublishedDataSet_Get_FieldMetaData_At(dataset, index);
    SOPC_FieldMetaData_Set_ValueRank(fieldmetadata, -1);
    SOPC_FieldMetaData_Set_BuiltinType(fieldmetadata, builtinType);
    SOPC_PublishedVariable* publishedVar = SOPC_FieldMetaData_Get_PublishedVariable(fieldmetadata);
    assert(NULL != publishedVar);
    SOPC_NodeId* nodeId = SOPC_NodeId_FromCString(strNodeId, (int32_t) strlen(strNodeId));
    assert(NULL != nodeId);
    SOPC_PublishedVariable_Set_NodeId(publishedVar, nodeId);
    SOPC_PublishedVariable_Set_AttributeId(publishedVar, 13); // Value => AttributeId=13
}

""")

    if IS_DEFINED_C_READER:
        cFile.add("""
static SOPC_DataSetReader* SOPC_PubSubConfig_SetSubMessageAt(SOPC_PubSubConnection* connection,
                                                             uint16_t index,
                                                             uint32_t publisherId,
                                                             uint16_t messageId,
                                                             uint32_t version,
                                                             double interval,
                                                             SOPC_SecurityMode_Type securityMode,
                                                             char* sksAddress)
{
    bool alloc;
    OpcUa_EndpointDescription* endpoint = NULL;
    SOPC_ReaderGroup* readerGroup = SOPC_PubSubConnection_Get_ReaderGroup_At(connection, index);
    assert(readerGroup != NULL);
    SOPC_ReaderGroup_Set_SecurityMode(readerGroup, securityMode);
    alloc = SOPC_EndpointDescription_Create_From_URL(sksAddress, &endpoint);
    if(alloc && NULL != endpoint) {
        // only one endpoint is created
        SOPC_ReaderGroup_Set_EndPointDescription(readerGroup, endpoint, 1);
    }

    alloc = SOPC_ReaderGroup_Allocate_DataSetReader_Array(readerGroup, 1);
    if (alloc)
    {
        SOPC_DataSetReader* reader = SOPC_ReaderGroup_Get_DataSetReader_At(readerGroup, 0);
        assert(reader != NULL);
        SOPC_DataSetReader_Set_WriterGroupVersion(reader, version);
        SOPC_DataSetReader_Set_WriterGroupId(reader, messageId);
        SOPC_DataSetReader_Set_DataSetWriterId(reader, messageId); // Same as WriterGroup
        SOPC_DataSetReader_Set_ReceiveTimeout(reader, 2.0 * interval);
        SOPC_DataSetReader_Set_PublisherId_UInteger(reader, publisherId);
        return reader;
    }
    return NULL;
}

""")

    if DEFINE_C_SETSUBNBVARIABLES:
        cFile.add("""
static bool SOPC_PubSubConfig_SetSubNbVariables(SOPC_DataSetReader* reader, uint16_t nbVar)
{
    return SOPC_DataSetReader_Allocate_FieldMetaData_Array(reader, SOPC_TargetVariablesDataType, nbVar);
}

""")

    if DEFINE_C_SETSUBVARIABLEAT:
        cFile.add("""
static void SOPC_PubSubConfig_SetSubVariableAt(SOPC_DataSetReader* reader,
                                               uint16_t index,
                                               char* strNodeId,
                                               SOPC_BuiltinId builtinType)
{
    SOPC_FieldMetaData* fieldmetadata = SOPC_DataSetReader_Get_FieldMetaData_At(reader, index);
    assert(fieldmetadata != NULL);

    /* fieldmetadata: type the field */
    SOPC_FieldMetaData_Set_ValueRank(fieldmetadata, -1);
    SOPC_FieldMetaData_Set_BuiltinType(fieldmetadata, builtinType);

    /* FieldTarget: link to the source/target data */
    SOPC_FieldTarget* fieldTarget = SOPC_FieldMetaData_Get_TargetVariable(fieldmetadata);
    assert(fieldTarget != NULL);
    SOPC_NodeId* nodeId = SOPC_NodeId_FromCString(strNodeId, (int32_t) strlen(strNodeId));
    SOPC_FieldTarget_Set_NodeId(fieldTarget, nodeId);
    SOPC_FieldTarget_Set_AttributeId(fieldTarget, 13); // Value => AttributeId=13
}
""")

    cFile.add(tmpCFile.data)


    c_file_out.write(cFile.data)




def main():
    argparser = argparse.ArgumentParser(description='Generate the S2OPC Pub-Sub configuration from an XML file')
    argparser.add_argument('xml_file', metavar='XML_FILE',
                           help='Path to Pub-Sub configuration XML file')
    argparser.add_argument('c_file', metavar='C_FILE',
                           help='Path to the generated C file')
    args = argparser.parse_args()

    print('Generating C Pub-Sub configuration...')
    with open(args.xml_file, 'rb') as xml_fd, open(args.c_file, 'w', encoding='utf8') as out_fd:
        try:
            generate_pubsub_config(xml_fd, out_fd)
        except ET.ParseError as e:
            sys.stderr.write('Woops, an error occurred: %s\n' % str(e))
            sys.exit(1)

    print('Done.')


if __name__ == '__main__':
    main()
