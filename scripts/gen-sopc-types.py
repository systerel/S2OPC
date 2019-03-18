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

#
#  Generate the ``sopc_types.[hc]`` files.
#

import argparse
from collections import OrderedDict
import xml.etree.ElementTree as ET
import sys

H_FILE_PATH = 'csrc/opcua_types/sopc_types.h'
C_FILE_PATH = 'csrc/opcua_types/sopc_types.c'


def main():
    """Main program"""
    ns = parse_args()
    schema = BinarySchema(ns.bsd)
    gen_header_file(schema)
    untranslated = schema.untranslated_typenames()
    toignore = (set('ua:' + t for t in PREDEFINED_TYPES) |
                set('ua:' + t for t in ORPHANED_TYPES))
    missing = untranslated - toignore
    unexpected = toignore - untranslated
    if missing:
        print("The following types have not been translated:")
        for name in sorted(missing):
            print("  " + name)
    if unexpected:
        print("The following types should not have been translated:")
        for name in sorted(unexpected):
            print("  " + name)


def parse_args():
    """
    Parse the command line arguments and return a dictionary of arguments.
    """
    parser = argparse.ArgumentParser(
        description='Generate the sopc_types.h and sopc_types.c files.')
    parser.add_argument('bsd',
                        metavar='file.bsd',
                        help='OPC UA type description')
    return parser.parse_args()


def gen_header_file(schema):
    """
    Generates the sopc_types.h file from the given schema.
    """
    with open(H_FILE_PATH, "w") as out:
        out.write(H_FILE_START)
        gen_header_types(out, schema)
        out.write(H_FILE_ENUM_FUN_DECLS)
        schema.gen_type_index_decl(out)
        out.write(H_FILE_END)


def gen_header_types(out, schema):
    """
    Generates the types in the header file.
    """
    #
    # The list of types to generate is currently hard-coded below to match
    # exactly with the legacy version produced by the C# tool.  We can later
    # make it shorter by iterating only on the top-level types (generation is
    # recursive).
    #
    schema.gen_header_type(out, 'IdType')
    schema.gen_header_type(out, 'Node')
    schema.gen_header_type(out, 'InstanceNode')
    schema.gen_header_type(out, 'TypeNode')
    schema.gen_header_type(out, 'ObjectNode')
    schema.gen_header_type(out, 'ObjectTypeNode')
    schema.gen_header_type(out, 'VariableNode')
    schema.gen_header_type(out, 'VariableTypeNode')
    schema.gen_header_type(out, 'ReferenceTypeNode')
    schema.gen_header_type(out, 'MethodNode')
    schema.gen_header_type(out, 'ViewNode')
    schema.gen_header_type(out, 'DataTypeNode')
    schema.gen_header_type(out, 'Argument')
    schema.gen_header_type(out, 'EnumValueType')
    schema.gen_header_type(out, 'EnumField')
    schema.gen_header_type(out, 'OptionSet')
    schema.gen_header_type(out, 'TimeZoneDataType')
    schema.gen_header_type(out, 'ApplicationDescription')
    schema.gen_header_type(out, 'RequestHeader')
    schema.gen_header_type(out, 'ServiceFault')
    schema.gen_header_pair(out, 'FindServers')
    schema.gen_header_type(out, 'ServerOnNetwork')
    schema.gen_header_pair(out, 'FindServersOnNetwork')
    schema.gen_header_type(out, 'EndpointDescription')
    schema.gen_header_pair(out, 'GetEndpoints')
    schema.gen_header_type(out, 'RegisteredServer')
    schema.gen_header_pair(out, 'RegisterServer')
    schema.gen_header_type(out, 'MdnsDiscoveryConfiguration')
    schema.gen_header_pair(out, 'RegisterServer2')
    schema.gen_header_type(out, 'SecurityTokenRequestType')
    schema.gen_header_type(out, 'ChannelSecurityToken')
    schema.gen_header_pair(out, 'OpenSecureChannel')
    schema.gen_header_pair(out, 'CloseSecureChannel')
    schema.gen_header_type(out, 'SignedSoftwareCertificate')
    schema.gen_header_type(out, 'SignatureData')
    schema.gen_header_pair(out, 'CreateSession')
    schema.gen_header_type(out, 'UserIdentityToken')
    schema.gen_header_type(out, 'AnonymousIdentityToken')
    schema.gen_header_type(out, 'UserNameIdentityToken')
    schema.gen_header_type(out, 'X509IdentityToken')
    schema.gen_header_type(out, 'KerberosIdentityToken')
    schema.gen_header_type(out, 'IssuedIdentityToken')
    schema.gen_header_pair(out, 'ActivateSession')
    schema.gen_header_pair(out, 'CloseSession')
    schema.gen_header_pair(out, 'Cancel')
    schema.gen_header_type(out, 'NodeAttributesMask')
    schema.gen_header_type(out, 'NodeAttributes')
    schema.gen_header_type(out, 'ObjectAttributes')
    schema.gen_header_type(out, 'VariableAttributes')
    schema.gen_header_type(out, 'MethodAttributes')
    schema.gen_header_type(out, 'ObjectTypeAttributes')
    schema.gen_header_type(out, 'VariableTypeAttributes')
    schema.gen_header_type(out, 'ReferenceTypeAttributes')
    schema.gen_header_type(out, 'DataTypeAttributes')
    schema.gen_header_type(out, 'ViewAttributes')
    schema.gen_header_type(out, 'AddNodesItem')
    schema.gen_header_type(out, 'AddNodesResult')
    schema.gen_header_pair(out, 'AddNodes')
    schema.gen_header_type(out, 'AddReferencesItem')
    schema.gen_header_pair(out, 'AddReferences')
    schema.gen_header_type(out, 'DeleteNodesItem')
    schema.gen_header_pair(out, 'DeleteNodes')
    schema.gen_header_type(out, 'DeleteReferencesItem')
    schema.gen_header_pair(out, 'DeleteReferences')
    schema.gen_header_type(out, 'AttributeWriteMask')
    schema.gen_header_type(out, 'BrowseDirection')
    schema.gen_header_type(out, 'ViewDescription')
    schema.gen_header_type(out, 'BrowseDescription')
    schema.gen_header_type(out, 'BrowseResultMask')
    schema.gen_header_type(out, 'BrowseResult')
    schema.gen_header_pair(out, 'Browse')
    schema.gen_header_pair(out, 'BrowseNext')
    schema.gen_header_type(out, 'BrowsePath')
    schema.gen_header_type(out, 'BrowsePathResult')
    schema.gen_header_pair(out, 'TranslateBrowsePathsToNodeIds')
    schema.gen_header_pair(out, 'RegisterNodes')
    schema.gen_header_pair(out, 'UnregisterNodes')
    schema.gen_header_type(out, 'EndpointConfiguration')
    schema.gen_header_type(out, 'ComplianceLevel')
    schema.gen_header_type(out, 'SupportedProfile')
    schema.gen_header_type(out, 'SoftwareCertificate')
    schema.gen_header_type(out, 'NodeTypeDescription')
    schema.gen_header_type(out, 'FilterOperator')
    schema.gen_header_type(out, 'QueryDataSet')
    schema.gen_header_type(out, 'NodeReference')
    schema.gen_header_type(out, 'ContentFilter')
    schema.gen_header_type(out, 'ElementOperand')
    schema.gen_header_type(out, 'LiteralOperand')
    schema.gen_header_type(out, 'AttributeOperand')
    schema.gen_header_type(out, 'SimpleAttributeOperand')
    schema.gen_header_type(out, 'ContentFilterElementResult')
    schema.gen_header_type(out, 'ContentFilterResult')
    schema.gen_header_type(out, 'ParsingResult')
    schema.gen_header_pair(out, 'QueryFirst')
    schema.gen_header_pair(out, 'QueryNext')
    schema.gen_header_type(out, 'TimestampsToReturn')
    schema.gen_header_type(out, 'ReadValueId')
    schema.gen_header_pair(out, 'Read')
    schema.gen_header_type(out, 'HistoryReadValueId')
    schema.gen_header_type(out, 'HistoryReadResult')
    schema.gen_header_type(out, 'ReadEventDetails')
    schema.gen_header_type(out, 'ReadRawModifiedDetails')
    schema.gen_header_type(out, 'ReadProcessedDetails')
    schema.gen_header_type(out, 'ReadAtTimeDetails')
    schema.gen_header_type(out, 'HistoryData')
    schema.gen_header_type(out, 'HistoryModifiedData')
    schema.gen_header_type(out, 'HistoryEvent')
    schema.gen_header_pair(out, 'HistoryRead')
    schema.gen_header_type(out, 'WriteValue')
    schema.gen_header_pair(out, 'Write')
    schema.gen_header_type(out, 'HistoryUpdateDetails')
    schema.gen_header_type(out, 'PerformUpdateType')
    schema.gen_header_type(out, 'UpdateDataDetails')
    schema.gen_header_type(out, 'UpdateStructureDataDetails')
    schema.gen_header_type(out, 'UpdateStructureDataDetails')
    schema.gen_header_type(out, 'UpdateEventDetails')
    schema.gen_header_type(out, 'DeleteRawModifiedDetails')
    schema.gen_header_type(out, 'DeleteAtTimeDetails')
    schema.gen_header_type(out, 'DeleteEventDetails')
    schema.gen_header_type(out, 'HistoryUpdateResult')
    schema.gen_header_pair(out, 'HistoryUpdate')
    schema.gen_header_type(out, 'CallMethodRequest')
    schema.gen_header_type(out, 'CallMethodResult')
    schema.gen_header_pair(out, 'Call')
    schema.gen_header_type(out, 'MonitoringMode')
    schema.gen_header_type(out, 'DataChangeTrigger')
    schema.gen_header_type(out, 'DeadbandType')
    schema.gen_header_type(out, 'DataChangeFilter')
    schema.gen_header_type(out, 'AggregateFilter')
    schema.gen_header_type(out, 'EventFilterResult')
    schema.gen_header_type(out, 'AggregateFilterResult')
    schema.gen_header_type(out, 'MonitoredItemCreateRequest')
    schema.gen_header_type(out, 'MonitoredItemCreateResult')
    schema.gen_header_pair(out, 'CreateMonitoredItems')
    schema.gen_header_type(out, 'MonitoredItemModifyRequest')
    schema.gen_header_type(out, 'MonitoredItemModifyResult')
    schema.gen_header_pair(out, 'ModifyMonitoredItems')
    schema.gen_header_pair(out, 'SetMonitoringMode')
    schema.gen_header_pair(out, 'SetTriggering')
    schema.gen_header_pair(out, 'DeleteMonitoredItems')
    schema.gen_header_pair(out, 'CreateSubscription')
    schema.gen_header_pair(out, 'ModifySubscription')
    schema.gen_header_pair(out, 'SetPublishingMode')
    schema.gen_header_type(out, 'NotificationMessage')
    schema.gen_header_type(out, 'DataChangeNotification')
    schema.gen_header_type(out, 'EventNotificationList')
    schema.gen_header_type(out, 'StatusChangeNotification')
    schema.gen_header_type(out, 'SubscriptionAcknowledgement')
    schema.gen_header_pair(out, 'Publish')
    schema.gen_header_pair(out, 'Republish')
    schema.gen_header_type(out, 'TransferResult')
    schema.gen_header_pair(out, 'TransferSubscriptions')
    schema.gen_header_pair(out, 'DeleteSubscriptions')
    schema.gen_header_type(out, 'EnumeratedTestType')
    schema.gen_header_type(out, 'BuildInfo')
    schema.gen_header_type(out, 'RedundancySupport')
    schema.gen_header_type(out, 'RedundantServerDataType')
    schema.gen_header_type(out, 'NetworkGroupDataType')
    schema.gen_header_type(out, 'SamplingIntervalDiagnosticsDataType')
    schema.gen_header_type(out, 'ServerDiagnosticsSummaryDataType')
    schema.gen_header_type(out, 'ServerStatusDataType')
    schema.gen_header_type(out, 'SessionDiagnosticsDataType')
    schema.gen_header_type(out, 'SessionSecurityDiagnosticsDataType')
    schema.gen_header_type(out, 'StatusResult')
    schema.gen_header_type(out, 'SubscriptionDiagnosticsDataType')
    schema.gen_header_type(out, 'ModelChangeStructureVerbMask')
    schema.gen_header_type(out, 'ModelChangeStructureDataType')
    schema.gen_header_type(out, 'SemanticChangeStructureDataType')
    schema.gen_header_type(out, 'Range')
    schema.gen_header_type(out, 'EUInformation')
    schema.gen_header_type(out, 'AxisScaleEnumeration')
    schema.gen_header_type(out, 'ComplexNumberType')
    schema.gen_header_type(out, 'DoubleComplexNumberType')
    schema.gen_header_type(out, 'AxisInformation')
    schema.gen_header_type(out, 'XVType')
    schema.gen_header_type(out, 'ProgramDiagnosticDataType')
    schema.gen_header_type(out, 'Annotation')
    schema.gen_header_type(out, 'ExceptionDeviationFormat')


class BinarySchema:
    """Represents a Binary Schema Description (BSD) file"""

    OPC_NS = {
      'opc': "http://opcfoundation.org/BinarySchema/",
      'xsi': "http://www.w3.org/2001/XMLSchema-instance",
      'ua': "http://opcfoundation.org/UA/",
      'tns': "http://opcfoundation.org/UA/",
    }
    """BSD namespace shortcuts"""

    ROOT_TAG = ET.QName(OPC_NS['opc'], 'TypeDictionary')
    """Expected tag of the root element of a BSD file"""

    ENUM_TAG = ET.QName(OPC_NS['opc'], 'EnumeratedType')
    """Tag for enumerated types"""

    STRUC_TAG = ET.QName(OPC_NS['opc'], 'StructuredType')
    """Tag for structured types"""

    BUILTIN_TYPES = {
            'opc:Boolean': 'SOPC_Boolean',
            'opc:SByte': 'SOPC_SByte',
            'opc:Byte': 'SOPC_Byte',
            'opc:Int16': 'int16_t',
            'opc:UInt16': 'uint16_t',
            'opc:Int32': 'int32_t',
            'opc:UInt32': 'uint32_t',
            'opc:Int64': 'int64_t',
            'opc:UInt64': 'uint64_t',
            'opc:Float': 'float',
            'opc:Double': 'double',
            'opc:String': 'SOPC_String',
            'opc:DateTime': 'SOPC_DateTime',
            'opc:Guid': 'SOPC_Guid',
            'opc:ByteString': 'SOPC_ByteString',
            'ua:XmlElement': 'SOPC_XmlElement',
            'ua:NodeId': 'SOPC_NodeId',
            'ua:ExpandedNodeId': 'SOPC_ExpandedNodeId',
            'ua:StatusCode': 'SOPC_StatusCode',
            'ua:QualifiedName': 'SOPC_QualifiedName',
            'ua:LocalizedText': 'SOPC_LocalizedText',
            'ua:ExtensionObject': 'SOPC_ExtensionObject',
            'ua:DataValue': 'SOPC_DataValue',
            'ua:Variant': 'SOPC_Variant',
            'ua:DiagnosticInfo': 'SOPC_DiagnosticInfo',
    }

    def __init__(self, filename):
        self.tree = ET.parse(filename)
        root = self.tree.getroot()
        if root.tag != self.ROOT_TAG:
            fatal("Invalid root element in bsd file: %s" % root.tag)
        self.bsd2c = OrderedDict(self.BUILTIN_TYPES)
        self.enums = set()
        self.known_writer = KnownEncodeableTypeWriter()

    def gen_header_pair(self, out, basename):
        """
        Generates the declaration of request and response types in the header
        file.

        The type declarations are also protected by a ``#ifdef`` for their
        common name.
        """
        out.write(BLOCK_PROTECTION_START.format(name=basename))
        typename = basename + 'Request'
        self.gen_header_type(out, typename)
        self.known_writer.block_start[typename] = basename

        typename = basename + 'Response'
        self.gen_header_type(out, typename)
        self.known_writer.block_end[typename] = basename
        out.write(BLOCK_PROTECTION_END.format(name=basename) + '\n')

    def gen_header_type(self, out, typename):
        """
        Generates the declaration of typename in the header file.
        """
        typename = self.normalize_typename(typename)
        if typename in self.bsd2c:
            return

        barename = typename.split(':')[1]
        node = self._get_node(barename)
        if node.tag == self.STRUC_TAG:
            ctype = self._gen_struct_decl(out, node, barename)
            self.known_writer.encodeable_types.append(typename)
        elif node.tag == self.ENUM_TAG:
            ctype = self._gen_enum_decl(out, node, barename)
            self.enums.add(typename)
        else:
            fatal("Unknown node kind: %s" % node.tag)
        self.bsd2c[typename] = ctype

    def gen_type_index_decl(self, out):
        """
        Generates an enumerated type for indexing in SOPC_KnownEncodeableTypes

        We use an enumerated type without explicit values, so that the indexes
        are always compact, even when some type is excluded.  The purpose of
        these indexes is to access rapidly to the description of a type.
        """
        def writer(barename):
            out.write(TYPE_INDEX_DECL_ENCODEABLE_TYPE.format(name=barename))

        out.write(TYPE_INDEX_DECL_START)
        self.known_writer.gen_types(out, writer)
        out.write(TYPE_INDEX_DECL_END)

    def untranslated_typenames(self):
        """
        Returns the set of all names of types present in the schema that have
        not been translated.
        """
        nodes = (self.tree.findall('opc:StructuredType', self.OPC_NS) +
                 self.tree.findall('opc:EnumeratedType', self.OPC_NS))
        names = (self.normalize_typename(node.get('Name')) for node in nodes)
        return set(filter(lambda n: n not in self.bsd2c, names))

    def _gen_struct_decl(self, out, node, name):
        """
        Generates the declarations for a structured type.
        """
        children = node.findall('./opc:Field', self.OPC_NS)
        fields = [Field(self, child) for child in children]
        for field in fields:
            self.gen_header_type(out, field.type_name)
        self._check_array_fields(name, fields)

        out.write(STRUCT_DECL_START.format(name=name))

        for field in fields:
            if field.name != 'RequestHeader':
                self._gen_field_decl(out, field)

        out.write(STRUCT_DECL_END.format(name=name))
        return 'OpcUa_' + name

    def _gen_field_decl(self, out, field):
        """
        Generates the declaration of a structure field.
        """
        typename = self.normalize_typename(field.type_name)
        ctype = self.get_ctype(typename)
        if field.length_field:
            ctype += '*'
        out.write(STRUCT_DECL_FIELD.format(ctype=ctype,
                                           name=field.name))

    def _check_array_fields(self, name, fields):
        """
        Checks that array fields are always just after their length.
        Also check that the length is encoded as an Int32.
        """
        for i, field in enumerate(fields):
            if not field.length_field:
                continue
            if i == 0:
                fatal("array field %s at start of struct %s"
                      % (field.name, name))
            prev = fields[i - 1]
            if prev.name != field.length_field:
                fatal(("array field %s is not just after its length "
                      + "in struct %s") % (field.name, name))
            if prev.type_name != 'opc:Int32':
                fatal("invalid type %s for array length %s in struct %s"
                      % (prev.type_name, prev.name, name))

    def _gen_enum_decl(self, out, node, name):
        """
        Generates the declaration of an enumerated type.
        """
        out.write(ENUM_DECL_START.format(name=name))

        elem_decls = [
                ENUM_DECL_ELEM.format(name=name,
                                      elem=child.attrib['Name'],
                                      value=child.attrib['Value'])
                for child in node.findall('./opc:EnumeratedValue', self.OPC_NS)
        ]
        out.write(",\n".join(elem_decls) + '\n')

        out.write(ENUM_DECL_END.format(name=name))
        return 'OpcUa_' + name

    def _get_node(self, barename):
        """Returns the node describing the given type name."""
        nodes = self.tree.findall('./*[@Name="%s"]' % barename)
        if not nodes:
            fatal("Cannot find node for type: %s" % barename)
        elif len(nodes) != 1:
            fatal("Too many nodes for type: %s" % barename)
        return nodes[0]

    def get_ctype(self, typename):
        """
        Returns the C type for the given UA type.
        The UA type must have been normalized.
        """
        return self.bsd2c[typename]

    def is_enum(self, typename):
        """
        Tells whether the given UA type is an enumerated type.
        The UA type must have been normalized.
        """
        return typename in self.enums

    def normalize_typename(self, typename):
        """
        Normalize a typename for internal use.

        ``typename`` is either the local name of the type (e.g., ``Node``) or
        an XML like string with a namespace prefix (e.g., ``tns:Node``). In the
        case where the namespace prefix is absent, it is taken to be ``ua``.
        In the case where the namespace is ``tns``, it is replaced by ``ua``
        (which has the same URI in ``OPC_NS``.
        """
        if ':' not in typename:
            return 'ua:' + typename
        if typename.startswith('tns:'):
            return 'ua' + typename[3:]
        return typename


class Field:
    """Models a field in a BSD."""
    def __init__(self, schema, node):
        self.schema = schema
        self.name = node.get('Name')
        self.type_name = node.get('TypeName')
        self.length_field = node.get('LengthField')
        if not self.name:
            fatal("Missing name for field node: %s", node)
        if not self.type_name:
            fatal("Missing type name for field node: %s", node)


class KnownEncodeableTypeWriter:
    """
    Writer for the array of known encodeable types.

    Also used to generate the type of the indexes in that array, to ensure that
    the two are always in sync.
    """
    def __init__(self):
        self.encodeable_types = []
        self.block_start = {}
        self.block_end = {}

    def gen_types(self, out, write):
        """
        Generates declarations for all encodeable types.

        The ``write`` parameter is a function that takes the barename of a type
        and writes the piece of C code appropriate for that type.
        """
        for typename in self.encodeable_types:
            barename = typename.split(':')[1]
            blockname = self.block_start.get(barename, None)
            if blockname:
                out.write(BLOCK_PROTECTION_START.format(name=blockname))
            write(barename)
            blockname = self.block_end.get(barename, None)
            if blockname:
                out.write('\n' + BLOCK_PROTECTION_END.format(name=blockname))


def fatal(msg):
    """
    Prints an error message on stdout and exits in error.
    """
    print(msg, file=sys.stderr)
    sys.exit(1)


H_FILE_START = """
/* ========================================================================
 * Copyright (c) 2005-2016 The OPC Foundation, Inc. All rights reserved.
 *
 * OPC Foundation MIT License 1.00
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * The complete license agreement can be found here:
 * http://opcfoundation.org/License/MIT/1.00/
 *
 * Modifications: adaptation for S2OPC project
 * ======================================================================*/

#ifndef SOPC_Types_H_
#define SOPC_Types_H_ 1

#include "sopc_buffer.h"
#include "sopc_builtintypes.h"
#include "sopc_encodeabletype.h"
"""[1:]

H_FILE_ENUM_FUN_DECLS = """
void SOPC_Initialize_EnumeratedType(int32_t* enumerationValue);

void SOPC_Clear_EnumeratedType(int32_t* enumerationValue);

SOPC_ReturnStatus SOPC_Read_EnumeratedType(SOPC_Buffer* buf, \
int32_t* enumerationValue);

SOPC_ReturnStatus SOPC_Write_EnumeratedType(SOPC_Buffer* buf, \
const int32_t* enumerationValue);
"""

H_FILE_END = """
/*============================================================================
 * Table of known types.
 *===========================================================================*/
extern struct SOPC_EncodeableType** SOPC_KnownEncodeableTypes;

#endif
/* This is the last line of an autogenerated file. */
"""

BLOCK_PROTECTION_START = """
#ifndef OPCUA_EXCLUDE_{name}"""

BLOCK_PROTECTION_END = """#endif"""

STRUCT_DECL_START = """
#ifndef OPCUA_EXCLUDE_{name}
/*============================================================================
 * The {name} structure.
 *===========================================================================*/
extern struct SOPC_EncodeableType OpcUa_{name}_EncodeableType;

typedef struct _OpcUa_{name}
{{
    SOPC_EncodeableType* encodeableType;
    /* IMPORTANT NOTE: response header IN RESPONSE MSG BODY is kept only
     *  for giving a copy of the header to application.
     */
"""

STRUCT_DECL_FIELD = """
    {ctype} {name};
"""[1:]

STRUCT_DECL_END = """
}} OpcUa_{name};

void OpcUa_{name}_Initialize(void* pValue);

void OpcUa_{name}_Clear(void* pValue);

SOPC_ReturnStatus OpcUa_{name}_Encode(const void* pValue, SOPC_Buffer* buf);

SOPC_ReturnStatus OpcUa_{name}_Decode(void* pValue, SOPC_Buffer* buf);

#endif
"""[1:]

ENUM_DECL_START = """
#ifndef OPCUA_EXCLUDE_{name}
/*============================================================================
 * The {name} enumeration.
 *===========================================================================*/
typedef enum _OpcUa_{name}
{{
"""

ENUM_DECL_ELEM = "    OpcUa_{name}_{elem} = {value}"

ENUM_DECL_END = """
}} OpcUa_{name};

#endif
"""[1:]

TYPE_INDEX_DECL_START = """
/*============================================================================
 * Indexes in the table of known encodeable types.
 *
 * The enumerated values are indexes in the SOPC_KnownEncodeableTypes array.
 *===========================================================================*/
typedef enum SOPC_TypeInternalIndex
{"""

TYPE_INDEX_DECL_ENCODEABLE_TYPE = """
#ifndef OPCUA_EXCLUDE_{name}
    SOPC_TypeInternalIndex_{name},
#endif"""

TYPE_INDEX_DECL_END = """
    SOPC_TypeInternalIndex_SIZE
} SOPC_TypeInternalIndex;
"""

PREDEFINED_TYPES = [
        # Various encodings of NodeId
        'NodeIdType',
        'TwoByteNodeId',
        'FourByteNodeId',
        'NumericNodeId',
        'StringNodeId',
        'GuidNodeId',
        'ByteStringNodeId',
]
"""
Types that are locally used in module ``sopc_builtintypes`` and do not need to
be generated in module ``sopc_types``.
"""

ORPHANED_TYPES = [
        'NamingRuleType',
        'OpenFileMode',
        'TrustListMasks',
        'TrustListDataType',

        'IdentityMappingRuleType',
        'IdentityCriteriaType',

        'ApplicationPermissionRuleType',

        'DataSetMetaDataType',
        'ConfigurationVersionDataType',
        'DataTypeDescription',
        'EnumDescription',
        'EnumDefinition',
        'StructureDescription',
        'StructureDefinition',
        'StructureField',
        'StructureType',
        'FieldMetaData',
        'KeyValuePair',
        'DataTypeDefinition',
        'Union',

        'DataConnectionDataType',
        'OverrideValueHandling',

        'PublishedVariableDataType',

        'SecurityKeyServiceDataType',

        'DataSetContentMask',


        'PubSubState',

        # Empty base types of translated types
        'DiscoveryConfiguration',
        'FilterOperand',
        'HistoryReadDetails',
        'MonitoringFilter',
        'MonitoringFilterResult',
        'NotificationData',

]
"""
Types that are present in the binary schema but not used by any other type and
not translated to module ``sopc_types``.
"""

if __name__ == '__main__':
    main()
