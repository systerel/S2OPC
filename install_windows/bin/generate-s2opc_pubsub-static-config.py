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
import sys, os

TAG_PUBSUB = "PubSub"

TAG_CONNECTION = "connection"
ATTRIBUTE_CONNECTION_ADDRESS = "address"
ATTRIBUTE_CONNECTION_MODE = "mode"
VALUE_CONNECTION_MODE_PUBLISHER = "publisher"
VALUE_CONNECTION_MODE_SUBSCRIBER = "subscriber"

TAG_MESSAGE = "message"
ATTRIBUTE_MESSAGE_ID = "groupId"
ATTRIBUTE_MESSAGE_VERSION = "groupVersion"
ATTRIBUTE_MESSAGE_INTERVAL = "publishingInterval"
ATTRIBUTE_MESSAGE_SECURITY_MODE = "securityMode"
VALUE_MESSAGE_SECURITY_MODE_NONE = "none"
VALUE_MESSAGE_SECURITY_MODE_SIGN = "sign"
VALUE_MESSAGE_SECURITY_MODE_SIGNANDENCRYPT = "signAndEncrypt"
ATTRIBUTE_MESSAGE_PUBLISHERID = "publisherId"

TAG_DATASET = "dataset"
ATTRIBUTE_DATASET_WRITERID = "writerId"
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

PUB_MODE="Pub"
SUB_MODE="Sub"
##
# Used to define only once C variables and functions
##
IS_DEFINED_C_WRITER = False
IS_DEFINED_C_GROUP = False
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

# A global context
class GContext:
    pubDataSetIndex = 0
    
class CnxContext:
    def __init__(self, connection):
        self.publisherId = None
        self.address = connection.get(ATTRIBUTE_CONNECTION_ADDRESS)
        self.messages = connection.findall("./%s" % TAG_MESSAGE)


class MessageContext:
    def __init__(self, cnxContext : CnxContext , message):
        self.cnxContext = cnxContext
        self.id = int(getOptionalAttribute(message, ATTRIBUTE_MESSAGE_ID, 0))
        self.version = int(getOptionalAttribute(message, ATTRIBUTE_MESSAGE_VERSION, 0))
        self.interval = float(message.get(ATTRIBUTE_MESSAGE_INTERVAL))
        self.securityMode = message.get(ATTRIBUTE_MESSAGE_SECURITY_MODE, VALUE_MESSAGE_SECURITY_MODE_NONE)


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



def getOptionalAttribute(node, attrName : str, defValue):
    try:return node.get(attrName, defValue)
    except:return defValue
    
    

def handleDoc(tree, result):
    pubSub = tree.getroot()
    handlePubSub(pubSub, result)

def handlePubSub(pubSub, result):
    
    pubConnections = pubSub.findall("./%s[@%s='%s']" % (TAG_CONNECTION, ATTRIBUTE_CONNECTION_MODE, VALUE_CONNECTION_MODE_PUBLISHER))
    subConnections = pubSub.findall("./%s[@%s='%s']" % (TAG_CONNECTION, ATTRIBUTE_CONNECTION_MODE, VALUE_CONNECTION_MODE_SUBSCRIBER))
    pubDataSets = pubSub.findall("./%s[@%s='%s']/%s/%s" % (TAG_CONNECTION, ATTRIBUTE_CONNECTION_MODE, VALUE_CONNECTION_MODE_PUBLISHER,
                                                           TAG_MESSAGE, TAG_DATASET))

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
    SOPC_PubSubConnection* connection = NULL;""" )

    if len(pubConnections) > 0:

        ##
        # PUBLISHERS
        ##
        result.add("""

    /* %d publisher connection */
    alloc = SOPC_PubSubConfiguration_Allocate_PubConnection_Array(config, %d);
    """ % (len(pubConnections), len(pubConnections)))
        
        result.add("""
    /* %d Published Datasets */
    alloc = SOPC_PubSubConfiguration_Allocate_PublishedDataSet_Array(config, %d);
    """ % (len(pubDataSets), len(pubDataSets)))
        GContext.pubDataSetIndex = 0

        cnxIndex = 0
        for connection in pubConnections:
            pubid = int(getOptionalAttribute(connection, ATTRIBUTE_MESSAGE_PUBLISHERID, -1))
            assert pubid > 0
                
            handlePubConnection(pubid, connection, cnxIndex, result)
            cnxIndex += 1

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
            handleSubConnection(connection, index, result)
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
    cnxContext = CnxContext(connection)
    
    result.add("""
    /** Publisher connection %d **/
    """ % index)
    nbMsg = len(cnxContext.messages)
    if nbMsg > 0:
        result.add("""
    if (alloc)
    {
        // Set publisher id and address
        connection = SOPC_PubSubConfiguration_Get_PubConnection_At(config, %d);
        assert(NULL != connection);
        SOPC_PubSubConnection_Set_PublisherId_UInteger(connection, %d);
        alloc = SOPC_PubSubConnection_Set_Address(connection, "%s");
    }
    """ % (index, publisherId, cnxContext.address))
    
        result.add("""
    if (alloc)
    {
        // Allocate %d writer groups (messages)
        alloc = SOPC_PubSubConnection_Allocate_WriterGroup_Array(connection, %d);
    }

    """ % (len(cnxContext.messages), len(cnxContext.messages)))

        msgIndex = 0
        for message in cnxContext.messages:
            handlePubMessage(cnxContext, message, msgIndex, result)
            msgIndex += 1


def handlePubMessage(cnxContext, message, msgIndex, result):
    global IS_DEFINED_C_GROUP
    
    msgContext = MessageContext(cnxContext, message)
 
    datasets = message.findall(TAG_DATASET)
     
    if not IS_DEFINED_C_GROUP:
        result.add("""
    SOPC_WriterGroup* writerGroup = NULL;""")

    IS_DEFINED_C_GROUP = True
    
    result.add("""
    /*** Pub Message %d ***/
    """ % msgContext.id)
    result.add("""
    if (alloc)
    {
        // GroupId = %s
        // GroupVersion = %s
        // Interval = %f ms
 """ % (msgContext.id, msgContext.version, msgContext.interval))
    result.add("""
       writerGroup = SOPC_PubSubConfig_SetPubMessageAt(connection, %d, %d, %d, %f, %s);
       alloc = NULL != writerGroup;
    }
 """ % (msgIndex, msgContext.id, msgContext.version, msgContext.interval,
       getCSecurityMode(msgContext.securityMode)))
     
    result.add("""
    if (alloc)
    {
       // %d data sets for message %d
       alloc = SOPC_WriterGroup_Allocate_DataSetWriter_Array(writerGroup, %d);
    }

    """ % (len(datasets), msgContext.id, len(datasets)))

    dsIndex = 0
    for dataset in datasets:
        handleDataset(PUB_MODE, msgContext, dataset, dsIndex, result)
        dsIndex += 1
    
def handleDataset(mode, msgContext : MessageContext, dataset, dsIndex, result):
    global IS_DEFINED_C_WRITER
    global IS_DEFINED_C_DATASET
    global DEFINE_C_SETSUBNBVARIABLES
    
    writerId = int(getOptionalAttribute(dataset, ATTRIBUTE_DATASET_WRITERID, 0))
    
    result.add("""
    /*** DataSetMessage No %d of message %d ***/
    """ % (dsIndex+1, msgContext.id))
    variables = dataset.findall(TAG_VARIABLE)


    if len(variables) > 0:
        if mode == PUB_MODE:
    
            if not IS_DEFINED_C_WRITER:
                result.add("""
    SOPC_DataSetWriter* writer = NULL;""")
         
                IS_DEFINED_C_WRITER = True
        
            if not IS_DEFINED_C_DATASET:
                result.add("""
    SOPC_PublishedDataSet* dataset = NULL;""")
            IS_DEFINED_C_DATASET = True
    
            result.add("""
    if (alloc)
    {
        writer = SOPC_WriterGroup_Get_DataSetWriter_At(writerGroup, %d);
        assert(NULL != writer);
        // WriterId = %d
        dataset = SOPC_PubSubConfig_InitDataSet(config, %d, writer, %d, %d);
        alloc = NULL != dataset;
    }
    if (alloc)
    {""" % (dsIndex,
            writerId,
            GContext.pubDataSetIndex, writerId, len(variables)))
            GContext.pubDataSetIndex += 1
        elif mode == SUB_MODE:
            
            DEFINE_C_SETSUBNBVARIABLES = True;
            
            assert(None != msgContext.cnxContext.publisherId)
            result.add("""
    if (alloc)
    {
        // Interval = %f ms
        dsReader = SOPC_PubSubConfig_SetReaderAt(readerGroup, %s, %s, %f);
        alloc = NULL != dsReader;
    }
    """ % (msgContext.interval,
           dsIndex,
           writerId, msgContext.interval))
            
            
            result.add("""
    if (alloc)
    {
        alloc = SOPC_PubSubConfig_SetSubNbVariables(dsReader, %d);
    
    }
    
    if (alloc)
    {
    """ % len(variables))
        
        varIndex = 0
        for variable in variables:
            handleVariable(mode, variable, varIndex, result)
            varIndex += 1

        result.add("""
    }
    """)

# type is Pub or Sub
def handleVariable(mode, variable, index, result):
    global DEFINE_C_SETPUBVARIABLEAT
    global DEFINE_C_SETSUBVARIABLEAT

    nodeId = variable.get(ATTRIBUTE_VARIABLE_NODEID)
    datatype = variable.get(ATTRIBUTE_VARIABLE_TYPE)
    displayName = variable.get(ATTRIBUTE_VARIABLE_NAME)
    assert datatype in TYPE_IDS
    dataid = TYPE_IDS[datatype]
    if mode == PUB_MODE:
        result.add("""
        SOPC_PubSubConfig_SetPubVariableAt(dataset, %d, "%s", %s); // %s""" % (index, nodeId, dataid, displayName))
        DEFINE_C_SETPUBVARIABLEAT = True
    elif mode == SUB_MODE:
        result.add("""
        SOPC_PubSubConfig_SetSubVariableAt(dsReader, %d, "%s", %s); // %s""" % (index, nodeId, dataid, displayName))
        DEFINE_C_SETSUBVARIABLEAT = True
    else: assert(false)

def handleSubConnection(connection, index, result):
    cnxContext = CnxContext(connection)
    
    nbMsg = len(cnxContext.messages)
    
    result.add("""
    /** Subscriber connection %d **/
    """ % index)


    if nbMsg > 0:

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

        """ % (index, cnxContext.address, nbMsg, nbMsg))
        msgIndex = 0
        for message in cnxContext.messages:
            
            cnxContext.publisherId = getOptionalAttribute(message, ATTRIBUTE_MESSAGE_PUBLISHERID, 0)
            handleSubMessage(cnxContext, message, msgIndex, result)
            msgIndex += 1


def handleSubMessage(cnxContext : CnxContext, message, index, result):
    global IS_DEFINED_C_READER
    global DEFINE_C_SETSUBNBVARIABLES
    
    msgContext = MessageContext(cnxContext, message)
 
    datasets = message.findall(TAG_DATASET)
    
    for attr in [ATTRIBUTE_MESSAGE_ID,
                 ATTRIBUTE_MESSAGE_VERSION,
                 ATTRIBUTE_MESSAGE_INTERVAL,
                 ATTRIBUTE_MESSAGE_PUBLISHERID]:
        assert attr in message.keys()
    
    if not IS_DEFINED_C_READER:
        result.add("""
    SOPC_DataSetReader* dsReader = NULL;
    SOPC_ReaderGroup* readerGroup = NULL;""")

    IS_DEFINED_C_READER = True;

    result.add("""
    /*** Sub Message %d ***/
    """ % msgContext.id)

    result.add("""
    if (alloc)
    {
        // Allocate %d datasets
        // GroupId = %d
        // GroupVersion = %d
        // PubId = %s
        readerGroup = SOPC_PubSubConfig_SetSubMessageAt(connection, %d, %s, %s, %s, %s, %d);
        alloc = NULL != readerGroup;
    }
    """ % (len(datasets), msgContext.id, msgContext.version, msgContext.cnxContext.publisherId,
           index, getCSecurityMode(msgContext.securityMode),
           msgContext.id, msgContext.version, msgContext.cnxContext.publisherId, len(datasets)))

    
    dsIndex = 0
    for dataset in datasets:
        handleDataset(SUB_MODE, msgContext, dataset, dsIndex, result)
        dsIndex += 1



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

// DO NOT EDIT THIS FILE HAS BEEN GENERATED BY %s

"""%(os.path.basename(sys.argv[0])))

    if IS_DEFINED_C_GROUP:
        cFile.add("""

static SOPC_WriterGroup* SOPC_PubSubConfig_SetPubMessageAt(SOPC_PubSubConnection* connection,
                                                           uint16_t index,
                                                           uint16_t groupId,
                                                           uint32_t groupVersion,
                                                           double interval,
                                                           SOPC_SecurityMode_Type securityMode)
{
    SOPC_WriterGroup* group = SOPC_PubSubConnection_Get_WriterGroup_At(connection, index);
    SOPC_WriterGroup_Set_Id(group, groupId);
    SOPC_WriterGroup_Set_Version(group, groupVersion);
    SOPC_WriterGroup_Set_PublishingInterval(group, interval);
    SOPC_WriterGroup_Set_SecurityMode(group, securityMode);
    
    return group;
}


""")

    if IS_DEFINED_C_DATASET:
        cFile.add("""
static SOPC_PublishedDataSet* SOPC_PubSubConfig_InitDataSet(SOPC_PubSubConfiguration* config,
                                                            uint16_t dataSetIndex,
                                                            SOPC_DataSetWriter* writer,
                                                            uint16_t dataSetId,
                                                            uint16_t nbVar)
{
    SOPC_PublishedDataSet* dataset = SOPC_PubSubConfiguration_Get_PublishedDataSet_At(config, dataSetIndex);
    SOPC_PublishedDataSet_Init(dataset, SOPC_PublishedDataItemsDataType, nbVar);
    SOPC_DataSetWriter_Set_DataSet(writer, dataset);
    SOPC_DataSetWriter_Set_Id(writer, dataSetId);

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
    SOPC_PublishedVariable_Set_AttributeId(publishedVar,
                                           13); // Value => AttributeId=13
}

""")

    if IS_DEFINED_C_READER:
        cFile.add("""
static SOPC_ReaderGroup* SOPC_PubSubConfig_SetSubMessageAt(SOPC_PubSubConnection* connection,
                                                           uint16_t index,
                                                           SOPC_SecurityMode_Type securityMode,
                                                           uint16_t groupId,
                                                           uint32_t groupVersion,
                                                           uint32_t publisherId,
                                                           uint16_t nbDataSets)
{
    SOPC_ReaderGroup* readerGroup = SOPC_PubSubConnection_Get_ReaderGroup_At(connection, index);
    assert(readerGroup != NULL);
    SOPC_ReaderGroup_Set_SecurityMode(readerGroup, securityMode);
    SOPC_ReaderGroup_Set_GroupVersion(readerGroup, groupVersion);
    SOPC_ReaderGroup_Set_GroupId(readerGroup, groupId);
    SOPC_ReaderGroup_Set_PublisherId_UInteger(readerGroup, publisherId);
    assert(nbDataSets < 0x100);
    bool allocSuccess = SOPC_ReaderGroup_Allocate_DataSetReader_Array(readerGroup, (uint8_t) nbDataSets);
    assert(allocSuccess);
    
    return readerGroup;
}

""")
        cFile.add("""
static SOPC_DataSetReader* SOPC_PubSubConfig_SetReaderAt(SOPC_ReaderGroup* readerGroup,
                                                         uint16_t index,
                                                         uint16_t writerId,   
                                                         double interval)
{
    assert(index < 0x100);
    SOPC_DataSetReader* reader = SOPC_ReaderGroup_Get_DataSetReader_At(readerGroup, (uint8_t) index);
    assert(reader != NULL);
    SOPC_DataSetReader_Set_DataSetWriterId(reader, writerId);
    SOPC_DataSetReader_Set_ReceiveTimeout(reader, 2.0 * interval);
    return reader;
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
