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

H_FILE_PATH = 'src/Common/opcua_types/sopc_types.h'
H_ENUM_FILE_PATH = 'src/Common/opcua_types/sopc_enum_types.h'
C_FILE_PATH = 'src/Common/opcua_types/sopc_types.c'


def main():
    """Main program"""
    ns = parse_args()
    schema = BinarySchema(ns.bsd)
    gen_header_file(schema)
    gen_implem_file(schema)

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
    with open(H_FILE_PATH, "w") as out, open(H_ENUM_FILE_PATH, "w") as out_enum:
        out.write(H_FILE_START)
        out_enum.write(H_ENUM_FILE_START)
        gen_header_types(out, out_enum, schema)
        out.write(H_FILE_ENUM_FUN_DECLS)
        schema.gen_type_index_decl(out)
        out.write(H_FILE_END)
        out_enum.write(H_ENUM_FILE_END)


def gen_implem_file(schema):
    """
    Generates the sopc_types.c file from the given schema.
    """
    with open(C_FILE_PATH, "w") as out:
        out.write(C_FILE_START)
        gen_implem_types(out, schema)
        out.write(C_FILE_END)


def gen_header_types(out, out_enum, schema):
    """
    Generates the types in the header file.
    """
    #
    # The list of types to generate is currently hard-coded below to match
    # exactly with the legacy version produced by the C# tool.  We can later
    # make it shorter by iterating only on the top-level types (generation is
    # recursive).
    #
    schema.gen_header_type(out, out_enum, 'IdType')
    schema.gen_header_type(out, out_enum, 'Node')
    schema.gen_header_type(out, out_enum, 'InstanceNode')
    schema.gen_header_type(out, out_enum, 'TypeNode')
    schema.gen_header_type(out, out_enum, 'ObjectNode')
    schema.gen_header_type(out, out_enum, 'ObjectTypeNode')
    schema.gen_header_type(out, out_enum, 'VariableNode')
    schema.gen_header_type(out, out_enum, 'VariableTypeNode')
    schema.gen_header_type(out, out_enum, 'ReferenceTypeNode')
    schema.gen_header_type(out, out_enum, 'MethodNode')
    schema.gen_header_type(out, out_enum, 'ViewNode')
    schema.gen_header_type(out, out_enum, 'DataTypeNode')
    schema.gen_header_type(out, out_enum, 'Argument')
    schema.gen_header_type(out, out_enum, 'EnumValueType')
    schema.gen_header_type(out, out_enum, 'EnumField')
    schema.gen_header_type(out, out_enum, 'OptionSet')
    schema.gen_header_type(out, out_enum, 'TimeZoneDataType')
    schema.gen_header_type(out, out_enum, 'ApplicationDescription')
    schema.gen_header_type(out, out_enum, 'RequestHeader')
    schema.gen_header_type(out, out_enum, 'ServiceFault')
    schema.gen_header_pair(out, out_enum, 'FindServers')
    schema.gen_header_type(out, out_enum, 'ServerOnNetwork')
    schema.gen_header_pair(out, out_enum, 'FindServersOnNetwork')
    schema.gen_header_type(out, out_enum, 'EndpointDescription')
    schema.gen_header_pair(out, out_enum, 'GetEndpoints')
    schema.gen_header_type(out, out_enum, 'RegisteredServer')
    schema.gen_header_pair(out, out_enum, 'RegisterServer')
    schema.gen_header_type(out, out_enum, 'MdnsDiscoveryConfiguration')
    schema.gen_header_pair(out, out_enum, 'RegisterServer2')
    schema.gen_header_type(out, out_enum, 'SecurityTokenRequestType')
    schema.gen_header_type(out, out_enum, 'ChannelSecurityToken')
    schema.gen_header_pair(out, out_enum, 'OpenSecureChannel')
    schema.gen_header_pair(out, out_enum, 'CloseSecureChannel')
    schema.gen_header_type(out, out_enum, 'SignedSoftwareCertificate')
    schema.gen_header_type(out, out_enum, 'SignatureData')
    schema.gen_header_pair(out, out_enum, 'CreateSession')
    schema.gen_header_type(out, out_enum, 'UserIdentityToken')
    schema.gen_header_type(out, out_enum, 'AnonymousIdentityToken')
    schema.gen_header_type(out, out_enum, 'UserNameIdentityToken')
    schema.gen_header_type(out, out_enum, 'X509IdentityToken')
    schema.gen_header_type(out, out_enum, 'KerberosIdentityToken')
    schema.gen_header_type(out, out_enum, 'IssuedIdentityToken')
    schema.gen_header_pair(out, out_enum, 'ActivateSession')
    schema.gen_header_pair(out, out_enum, 'CloseSession')
    schema.gen_header_pair(out, out_enum, 'Cancel')
    schema.gen_header_type(out, out_enum, 'NodeAttributesMask')
    schema.gen_header_type(out, out_enum, 'NodeAttributes')
    schema.gen_header_type(out, out_enum, 'ObjectAttributes')
    schema.gen_header_type(out, out_enum, 'VariableAttributes')
    schema.gen_header_type(out, out_enum, 'MethodAttributes')
    schema.gen_header_type(out, out_enum, 'ObjectTypeAttributes')
    schema.gen_header_type(out, out_enum, 'VariableTypeAttributes')
    schema.gen_header_type(out, out_enum, 'ReferenceTypeAttributes')
    schema.gen_header_type(out, out_enum, 'DataTypeAttributes')
    schema.gen_header_type(out, out_enum, 'ViewAttributes')
    schema.gen_header_type(out, out_enum, 'AddNodesItem')
    schema.gen_header_type(out, out_enum, 'AddNodesResult')
    schema.gen_header_pair(out, out_enum, 'AddNodes')
    schema.gen_header_type(out, out_enum, 'AddReferencesItem')
    schema.gen_header_pair(out, out_enum, 'AddReferences')
    schema.gen_header_type(out, out_enum, 'DeleteNodesItem')
    schema.gen_header_pair(out, out_enum, 'DeleteNodes')
    schema.gen_header_type(out, out_enum, 'DeleteReferencesItem')
    schema.gen_header_pair(out, out_enum, 'DeleteReferences')
    schema.gen_header_type(out, out_enum, 'AttributeWriteMask')
    schema.gen_header_type(out, out_enum, 'BrowseDirection')
    schema.gen_header_type(out, out_enum, 'ViewDescription')
    schema.gen_header_type(out, out_enum, 'BrowseDescription')
    schema.gen_header_type(out, out_enum, 'BrowseResultMask')
    schema.gen_header_type(out, out_enum, 'BrowseResult')
    schema.gen_header_pair(out, out_enum, 'Browse')
    schema.gen_header_pair(out, out_enum, 'BrowseNext')
    schema.gen_header_type(out, out_enum, 'BrowsePath')
    schema.gen_header_type(out, out_enum, 'BrowsePathResult')
    schema.gen_header_pair(out, out_enum, 'TranslateBrowsePathsToNodeIds')
    schema.gen_header_pair(out, out_enum, 'RegisterNodes')
    schema.gen_header_pair(out, out_enum, 'UnregisterNodes')
    schema.gen_header_type(out, out_enum, 'EndpointConfiguration')
    schema.gen_header_type(out, out_enum, 'ComplianceLevel')
    schema.gen_header_type(out, out_enum, 'SupportedProfile')
    schema.gen_header_type(out, out_enum, 'SoftwareCertificate')
    schema.gen_header_type(out, out_enum, 'NodeTypeDescription')
    schema.gen_header_type(out, out_enum, 'FilterOperator')
    schema.gen_header_type(out, out_enum, 'QueryDataSet')
    schema.gen_header_type(out, out_enum, 'NodeReference')
    schema.gen_header_type(out, out_enum, 'ContentFilter')
    schema.gen_header_type(out, out_enum, 'ElementOperand')
    schema.gen_header_type(out, out_enum, 'LiteralOperand')
    schema.gen_header_type(out, out_enum, 'AttributeOperand')
    schema.gen_header_type(out, out_enum, 'SimpleAttributeOperand')
    schema.gen_header_type(out, out_enum, 'ContentFilterElementResult')
    schema.gen_header_type(out, out_enum, 'ContentFilterResult')
    schema.gen_header_type(out, out_enum, 'ParsingResult')
    schema.gen_header_pair(out, out_enum, 'QueryFirst')
    schema.gen_header_pair(out, out_enum, 'QueryNext')
    schema.gen_header_type(out, out_enum, 'TimestampsToReturn')
    schema.gen_header_type(out, out_enum, 'ReadValueId')
    schema.gen_header_pair(out, out_enum, 'Read')
    schema.gen_header_type(out, out_enum, 'HistoryReadValueId')
    schema.gen_header_type(out, out_enum, 'HistoryReadResult')
    schema.gen_header_type(out, out_enum, 'ReadEventDetails')
    schema.gen_header_type(out, out_enum, 'ReadRawModifiedDetails')
    schema.gen_header_type(out, out_enum, 'ReadProcessedDetails')
    schema.gen_header_type(out, out_enum, 'ReadAtTimeDetails')
    schema.gen_header_type(out, out_enum, 'HistoryData')
    schema.gen_header_type(out, out_enum, 'HistoryModifiedData')
    schema.gen_header_type(out, out_enum, 'HistoryEvent')
    schema.gen_header_pair(out, out_enum, 'HistoryRead')
    schema.gen_header_type(out, out_enum, 'WriteValue')
    schema.gen_header_pair(out, out_enum, 'Write')
    schema.gen_header_type(out, out_enum, 'HistoryUpdateDetails')
    schema.gen_header_type(out, out_enum, 'PerformUpdateType')
    schema.gen_header_type(out, out_enum, 'UpdateDataDetails')
    schema.gen_header_type(out, out_enum, 'UpdateStructureDataDetails')
    schema.gen_header_type(out, out_enum, 'UpdateStructureDataDetails')
    schema.gen_header_type(out, out_enum, 'UpdateEventDetails')
    schema.gen_header_type(out, out_enum, 'DeleteRawModifiedDetails')
    schema.gen_header_type(out, out_enum, 'DeleteAtTimeDetails')
    schema.gen_header_type(out, out_enum, 'DeleteEventDetails')
    schema.gen_header_type(out, out_enum, 'HistoryUpdateResult')
    schema.gen_header_pair(out, out_enum, 'HistoryUpdate')
    schema.gen_header_type(out, out_enum, 'CallMethodRequest')
    schema.gen_header_type(out, out_enum, 'CallMethodResult')
    schema.gen_header_pair(out, out_enum, 'Call')
    schema.gen_header_type(out, out_enum, 'MonitoringMode')
    schema.gen_header_type(out, out_enum, 'DataChangeTrigger')
    schema.gen_header_type(out, out_enum, 'DeadbandType')
    schema.gen_header_type(out, out_enum, 'DataChangeFilter')
    schema.gen_header_type(out, out_enum, 'AggregateFilter')
    schema.gen_header_type(out, out_enum, 'EventFilterResult')
    schema.gen_header_type(out, out_enum, 'AggregateFilterResult')
    schema.gen_header_type(out, out_enum, 'MonitoredItemCreateRequest')
    schema.gen_header_type(out, out_enum, 'MonitoredItemCreateResult')
    schema.gen_header_pair(out, out_enum, 'CreateMonitoredItems')
    schema.gen_header_type(out, out_enum, 'MonitoredItemModifyRequest')
    schema.gen_header_type(out, out_enum, 'MonitoredItemModifyResult')
    schema.gen_header_pair(out, out_enum, 'ModifyMonitoredItems')
    schema.gen_header_pair(out, out_enum, 'SetMonitoringMode')
    schema.gen_header_pair(out, out_enum, 'SetTriggering')
    schema.gen_header_pair(out, out_enum, 'DeleteMonitoredItems')
    schema.gen_header_pair(out, out_enum, 'CreateSubscription')
    schema.gen_header_pair(out, out_enum, 'ModifySubscription')
    schema.gen_header_pair(out, out_enum, 'SetPublishingMode')
    schema.gen_header_type(out, out_enum, 'NotificationMessage')
    schema.gen_header_type(out, out_enum, 'DataChangeNotification')
    schema.gen_header_type(out, out_enum, 'EventNotificationList')
    schema.gen_header_type(out, out_enum, 'StatusChangeNotification')
    schema.gen_header_type(out, out_enum, 'SubscriptionAcknowledgement')
    schema.gen_header_pair(out, out_enum, 'Publish')
    schema.gen_header_pair(out, out_enum, 'Republish')
    schema.gen_header_type(out, out_enum, 'TransferResult')
    schema.gen_header_pair(out, out_enum, 'TransferSubscriptions')
    schema.gen_header_pair(out, out_enum, 'DeleteSubscriptions')
    schema.gen_header_type(out, out_enum, 'EnumeratedTestType')
    schema.gen_header_type(out, out_enum, 'BuildInfo')
    schema.gen_header_type(out, out_enum, 'RedundancySupport')
    schema.gen_header_type(out, out_enum, 'RedundantServerDataType')
    schema.gen_header_type(out, out_enum, 'NetworkGroupDataType')
    schema.gen_header_type(out, out_enum, 'SamplingIntervalDiagnosticsDataType')
    schema.gen_header_type(out, out_enum, 'ServerDiagnosticsSummaryDataType')
    schema.gen_header_type(out, out_enum, 'ServerStatusDataType')
    schema.gen_header_type(out, out_enum, 'SessionDiagnosticsDataType')
    schema.gen_header_type(out, out_enum, 'SessionSecurityDiagnosticsDataType')
    schema.gen_header_type(out, out_enum, 'StatusResult')
    schema.gen_header_type(out, out_enum, 'SubscriptionDiagnosticsDataType')
    schema.gen_header_type(out, out_enum, 'ModelChangeStructureVerbMask')
    schema.gen_header_type(out, out_enum, 'ModelChangeStructureDataType')
    schema.gen_header_type(out, out_enum, 'SemanticChangeStructureDataType')
    schema.gen_header_type(out, out_enum, 'Range')
    schema.gen_header_type(out, out_enum, 'EUInformation')
    schema.gen_header_type(out, out_enum, 'AxisScaleEnumeration')
    schema.gen_header_type(out, out_enum, 'ComplexNumberType')
    schema.gen_header_type(out, out_enum, 'DoubleComplexNumberType')
    schema.gen_header_type(out, out_enum, 'AxisInformation')
    schema.gen_header_type(out, out_enum, 'XVType')
    schema.gen_header_type(out, out_enum, 'ProgramDiagnosticDataType')
    schema.gen_header_type(out, out_enum, 'Annotation')
    schema.gen_header_type(out, out_enum, 'ExceptionDeviationFormat')


def gen_implem_types(out, schema):
    schema.gen_encodeable_type_descs(out)
    schema.gen_user_token_policies_constants(out)
    schema.gen_encodeable_type_table(out)


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
        self.fields = dict()
        self.known_writer = KnownEncodeableTypeWriter()

    def gen_header_pair(self, out, out_enum, basename):
        """
        Generates the declaration of request and response types in the header
        file.

        The type declarations are also protected by a ``#ifdef`` for their
        common name.
        """
        out.write(BLOCK_PROTECTION_START.format(name=basename))
        typename = basename + 'Request'
        self.gen_header_type(out, out_enum, typename)
        self.known_writer.block_start[typename] = basename

        typename = basename + 'Response'
        self.gen_header_type(out, out_enum, typename)
        self.known_writer.block_end[typename] = basename
        out.write(BLOCK_PROTECTION_END.format(name=basename) + '\n')

    def gen_header_type(self, out, out_enum, typename):
        """
        Generates the declaration of typename in the header file.
        """
        typename = self.normalize_typename(typename)
        if typename in self.bsd2c:
            return

        barename = typename.split(':')[1]
        node = self._get_node(barename)
        if node.tag == self.STRUC_TAG:
            ctype = self._gen_struct_decl(out, out_enum, node, barename)
            self.known_writer.encodeable_types.append(typename)
        elif node.tag == self.ENUM_TAG:
            ctype = self._gen_enum_decl(out_enum, node, barename)
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
        def writer(typename, barename):
            out.write(TYPE_INDEX_DECL_ENCODEABLE_TYPE.format(name=barename))

        out.write(TYPE_INDEX_DECL_START)
        self.known_writer.gen_types(out, writer)
        out.write(TYPE_INDEX_DECL_END)

    def gen_encodeable_type_descs(self, out):
        """
        Generates the descriptors of all encodeable types.
        """
        def writer(typename, barename):
            self.gen_encodeable_type_desc(out, typename, barename)

        self.known_writer.gen_types(out, writer)

    def gen_encodeable_type_desc(self, out, typename, barename):
        """
        Generates the field descriptors of an encodeable type.
        """
        ctype = self.bsd2c[typename]
        out.write(ENCODEABLE_TYPE_DESC_START.format(name=barename))
        fields = self.fields[barename]
        if fields:
            out.write(ENCODEABLE_TYPE_FIELD_DESC_START.format(name=barename))
        for field in fields:
            is_built_in, type_index = self.get_type_index(field.type_name)
            out.write(ENCODEABLE_TYPE_FIELD_DESC.format(
                name=barename,
                is_built_in=c_bool_value(is_built_in),
                is_array_length=c_bool_value(field.is_array_length),
                is_to_encode=c_bool_value(field.is_to_encode),
                type_index=type_index,
                field_name=field.name
            ))
        if fields:
            out.write(ENCODEABLE_TYPE_FIELD_DESC_END.format())
            nof_fields = ENCODEABLE_TYPE_NOF_FIELDS.format(name=barename)
            field_desc = barename + '_Fields'
        else:
            nof_fields = '0'
            field_desc = 'NULL'
        out.write(ENCODEABLE_TYPE_DESC_END.format(
            name=barename,
            ctype=ctype,
            nof_fields=nof_fields,
            field_desc=field_desc))

    def get_type_index(self, typename):
        """
        Returns a pair (is_built_in, type_index) for a field type.
        """
        barename = typename.split(':')[1]
        is_built_in = typename in self.BUILTIN_TYPES
        if is_built_in:
            template = 'SOPC_{name}_Id'
        elif self.is_enum(self.normalize_typename(typename)):
            # Enumerated types are repainted to Int32
            is_built_in = True
            template = 'SOPC_Int32_Id'
        else:
            template = 'SOPC_TypeInternalIndex_{name}'
        return is_built_in, template.format(name=barename)

    def gen_user_token_policies_constants(self, out):
        """
        Generates example constants for UserTokenPolicies
        """
        out.write(CSTS_USER_TOKEN_POLICIES)

    def gen_encodeable_type_table(self, out):
        """
        Generates the table of all encodeable types.
        """
        def writer(typename, barename):
            out.write(TYPE_TABLE_ENTRY.format(name=barename))

        out.write(TYPE_TABLE_START)
        self.known_writer.gen_types(out, writer)
        out.write(TYPE_TABLE_END)

    def untranslated_typenames(self):
        """
        Returns the set of all names of types present in the schema that have
        not been translated.
        """
        nodes = (self.tree.findall('opc:StructuredType', self.OPC_NS) +
                 self.tree.findall('opc:EnumeratedType', self.OPC_NS))
        names = (self.normalize_typename(node.get('Name')) for node in nodes)
        return set(filter(lambda n: n not in self.bsd2c, names))

    def _gen_struct_decl(self, out, out_enum, node, name):
        """
        Generates the declarations for a structured type.
        """
        children = node.findall('./opc:Field', self.OPC_NS)
        fields = [Field(self, child) for child in children]
        for field in fields:
            self.gen_header_type(out, out_enum, field.type_name)
        fields = [field for field in fields if field.name != 'RequestHeader']
        self._check_array_fields(name, fields)

        out.write(STRUCT_DECL_START.format(name=name))

        for field in fields:
            self._gen_field_decl(out, field)

        out.write(STRUCT_DECL_END.format(name=name))
        self.fields[name] = fields
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
        Then mark the length field with the is_array_length attribute.
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
            prev.is_array_length = True

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
        # append an element that will size the enum as an int32
        elem_decls.append(ENUM_DECL_ELEM.format(name=name, elem="SizeOf", value="INT32_MAX"))

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
        # Will be updated later when checking arrays
        self.is_array_length = False
        self.is_to_encode = self.name != 'ResponseHeader'


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

        The ``write`` parameter is a function that takes the full name and
        barename of a type and writes the piece of C code appropriate for that
        type.
        """
        for typename in self.encodeable_types:
            barename = typename.split(':')[1]
            blockname = self.block_start.get(barename, None)
            if blockname:
                out.write(BLOCK_PROTECTION_START.format(name=blockname))
            write(typename, barename)
            blockname = self.block_end.get(barename, None)
            if blockname:
                out.write('\n' + BLOCK_PROTECTION_END.format(name=blockname))


def c_bool_value(value):
    """
    Returns the C representation of a Boolean value.
    """
    return 'true' if value else 'false'


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
#include "sopc_enum_types.h"

// s2opc_common_export.h is generated by CMake, when not using CMake, copy and include
// "src/Common/helpers_platform_dep/<platform>/s2opc_common_export.h_"
#include "s2opc_common_export.h"

"""[1:]

H_FILE_ENUM_FUN_DECLS = """
void SOPC_Initialize_EnumeratedType(int32_t* enumerationValue);

void SOPC_Clear_EnumeratedType(int32_t* enumerationValue);

SOPC_ReturnStatus SOPC_Read_EnumeratedType(SOPC_Buffer* buf, \
int32_t* enumerationValue, uint32_t nestedStructLevel);

SOPC_ReturnStatus SOPC_Write_EnumeratedType(SOPC_Buffer* buf, \
const int32_t* enumerationValue, uint32_t nestedStructLevel);
"""

H_FILE_END = """
/*============================================================================
 * UserTokenPolicies example constant values
 *===========================================================================*/
#ifndef OPCUA_EXCLUDE_UserTokenPolicy
// UserTokenPolicyId for anonymous token type example
#define SOPC_UserTokenPolicy_Anonymous_ID "anonymous"
/** Example of anonymous user security policy supported configuration */
S2OPC_COMMON_EXPORT extern const OpcUa_UserTokenPolicy SOPC_UserTokenPolicy_Anonymous;

// UserTokenPolicyId for username token type with None SecurityPolicy example
#define SOPC_UserTokenPolicy_UserNameNone_ID "username_None"
/** Example username security policy supported and configured with security policy None.
 * With this security policy, the password will never be encrypted and this policy
 * shall not be used on unsecured and unencrypted secure channels.
 */
S2OPC_COMMON_EXPORT extern const OpcUa_UserTokenPolicy SOPC_UserTokenPolicy_UserName_NoneSecurityPolicy;

// UserTokenPolicyId for username token type with default SecurityPolicy example
#define SOPC_UserTokenPolicy_UserName_ID "username"
/** Example username security policy supported and configured with empty security policy.
 *  With this security policy, the password will be encrypted if Secure Channel security policy is not None.
 *  It shall not be used on secure channels using security policy None otherwise no encryption will be done.
 */
S2OPC_COMMON_EXPORT extern const OpcUa_UserTokenPolicy SOPC_UserTokenPolicy_UserName_DefaultSecurityPolicy;

// UserTokenPolicyId for username token type with Basic256Sha256 SecurityPolicy example
#define SOPC_UserTokenPolicy_UserNameBasic256Sha256_ID "username_Basic256Sha256"
/** Example username security policy supported and configured with Basic256Sha256 security policy.
 *  With this security policy, the password will be encrypted in any SecureChannel security policy and mode.
 */
S2OPC_COMMON_EXPORT extern const OpcUa_UserTokenPolicy SOPC_UserTokenPolicy_UserName_Basic256Sha256SecurityPolicy;

#endif

/*============================================================================
 * Table of known types.
 *===========================================================================*/
extern SOPC_EncodeableType** SOPC_KnownEncodeableTypes;

#endif
/* This is the last line of an autogenerated file. */
"""

H_ENUM_FILE_START = """
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

#ifndef SOPC_Enum_Types_H_
#define SOPC_Enum_Types_H_ 1

/* DO NOT EDIT. THIS FILE IS GENERATED BY THE SCRIPT gen-sopc-types.py */

#include <stdint.h>
"""[1:]

H_ENUM_FILE_END = """
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
S2OPC_COMMON_EXPORT extern SOPC_EncodeableType OpcUa_{name}_EncodeableType;

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

SOPC_ReturnStatus OpcUa_{name}_Encode(const void* pValue, SOPC_Buffer* buf, uint32_t nestedStructLevel);

SOPC_ReturnStatus OpcUa_{name}_Decode(void* pValue, SOPC_Buffer* buf, uint32_t nestedStructLevel);

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

C_FILE_START = """
/*
 * Licensed to Systerel under one or more contributor license
 * agreements. See the NOTICE file distributed with this work
 * for additional information regarding copyright ownership.
 * Systerel licenses this file to you under the Apache
 * License, Version 2.0 (the "License"); you may not use this
 * file except in compliance with the License. You may obtain
 * a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include "opcua_identifiers.h"
#include "sopc_types.h"
#include "sopc_crypto_profiles.h"
#include "sopc_encoder.h"

/* DO NOT EDIT. THIS FILE IS GENERATED BY THE SCRIPT gen-sopc-types.py */
 
"""[1:]

ENCODEABLE_TYPE_DESC_START = """
#ifndef OPCUA_EXCLUDE_{name}
/*============================================================================
 * OpcUa_{name}_Initialize
 *===========================================================================*/
void OpcUa_{name}_Initialize(void* pValue)
{{
    SOPC_EncodeableObject_Initialize(&OpcUa_{name}_EncodeableType, pValue);
}}

/*============================================================================
 * OpcUa_{name}_Clear
 *===========================================================================*/
void OpcUa_{name}_Clear(void* pValue)
{{
    SOPC_EncodeableObject_Clear(&OpcUa_{name}_EncodeableType, pValue);
}}

/*============================================================================
 * OpcUa_{name}_Encode
 *===========================================================================*/
SOPC_ReturnStatus OpcUa_{name}_Encode(const void* pValue, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{{
    return SOPC_EncodeableObject_Encode(\
&OpcUa_{name}_EncodeableType, pValue, buf, nestedStructLevel);
}}

/*============================================================================
 * OpcUa_{name}_Decode
 *===========================================================================*/
SOPC_ReturnStatus OpcUa_{name}_Decode(void* pValue, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{{
    return SOPC_EncodeableObject_Decode(\
&OpcUa_{name}_EncodeableType, pValue, buf, nestedStructLevel);
}}
"""

ENCODEABLE_TYPE_FIELD_DESC_START = """
/*============================================================================
 * Field descriptors of the {name} encodeable type.
 *===========================================================================*/
static const SOPC_EncodeableType_FieldDescriptor {name}_Fields[] = {{
"""

ENCODEABLE_TYPE_FIELD_DESC = """
    {{
        {is_built_in},  // isBuiltIn
        {is_array_length}, // isArrayLength
        {is_to_encode}, // isToEncode
        (uint32_t) {type_index}, // typeIndex
        (uint32_t) offsetof(OpcUa_{name}, {field_name}) // offset
    }},
"""[1:]

ENCODEABLE_TYPE_FIELD_DESC_END = """
}};
"""[1:]

ENCODEABLE_TYPE_NOF_FIELDS = """
sizeof {name}_Fields / sizeof(SOPC_EncodeableType_FieldDescriptor)
"""[1:-1]

# TODO make encodeable type descriptor const
ENCODEABLE_TYPE_DESC_END = """
/*============================================================================
 * Descriptor of the {name} encodeable type.
 *===========================================================================*/
SOPC_EncodeableType OpcUa_{name}_EncodeableType =
{{
    "{name}",
    OpcUaId_{name},
    OpcUaId_{name}_Encoding_DefaultBinary,
    OpcUaId_{name}_Encoding_DefaultXml,
    NULL,
    sizeof({ctype}),
    OpcUa_{name}_Initialize,
    OpcUa_{name}_Clear,
    NULL,
    OpcUa_{name}_Encode,
    OpcUa_{name}_Decode,
    {nof_fields},
    {field_desc}
}};
#endif
"""

CSTS_USER_TOKEN_POLICIES = """
#ifndef OPCUA_EXCLUDE_UserTokenPolicy
/*============================================================================
 * UserTokenPolicies example constant values
 *===========================================================================*/

const OpcUa_UserTokenPolicy SOPC_UserTokenPolicy_Anonymous = {
    .TokenType = OpcUa_UserTokenType_Anonymous,
    .PolicyId = {sizeof(SOPC_UserTokenPolicy_Anonymous_ID) - 1, true,
                (SOPC_Byte*) SOPC_UserTokenPolicy_Anonymous_ID},
    .IssuedTokenType = {0, true, NULL},
    .IssuerEndpointUrl = {0, true, NULL},
    .SecurityPolicyUri = {0, true, NULL},
};

const OpcUa_UserTokenPolicy SOPC_UserTokenPolicy_UserName_NoneSecurityPolicy = {
    .TokenType = OpcUa_UserTokenType_UserName,
    .PolicyId = {sizeof(SOPC_UserTokenPolicy_UserNameNone_ID) - 1, true,
                (SOPC_Byte*) SOPC_UserTokenPolicy_UserNameNone_ID},
    .IssuedTokenType = {0, true, NULL},
    .IssuerEndpointUrl = {0, true, NULL},
    .SecurityPolicyUri = {sizeof(SOPC_SecurityPolicy_None_URI) - 1, true, (SOPC_Byte*) SOPC_SecurityPolicy_None_URI},
    /* None security policy shall be used only when
   secure channel security policy is non-None and with encryption since password will be non-encrypted */
};

const OpcUa_UserTokenPolicy SOPC_UserTokenPolicy_UserName_DefaultSecurityPolicy = {
    .TokenType = OpcUa_UserTokenType_UserName,
    .PolicyId = {sizeof(SOPC_UserTokenPolicy_UserName_ID) - 1, true,
                (SOPC_Byte*) SOPC_UserTokenPolicy_UserName_ID},
    .IssuedTokenType = {0, true, NULL},
    .IssuerEndpointUrl = {0, true, NULL},
    .SecurityPolicyUri = {0, true, NULL},
    /* Default security policy shall be used only when
   secure channel security policy is non-None since password will be non-encrypted */
};

const OpcUa_UserTokenPolicy SOPC_UserTokenPolicy_UserName_Basic256Sha256SecurityPolicy = {
    .TokenType = OpcUa_UserTokenType_UserName,
    .PolicyId = {sizeof(SOPC_UserTokenPolicy_UserNameBasic256Sha256_ID) - 1, true,
                 (SOPC_Byte*) SOPC_UserTokenPolicy_UserNameBasic256Sha256_ID},
    .IssuedTokenType = {0, true, NULL},
    .IssuerEndpointUrl = {0, true, NULL},
    .SecurityPolicyUri = {sizeof(SOPC_SecurityPolicy_Basic256Sha256_URI) - 1, true,
                          (SOPC_Byte*) SOPC_SecurityPolicy_Basic256Sha256_URI},
    /* Basic256Sha256 security policy might be used to ensure password is encrypted in any security policy and mode */
};
#endif
"""

# TODO make the whole array const
TYPE_TABLE_START = """
/*============================================================================
 * Table of known types.
 *===========================================================================*/
static SOPC_EncodeableType* g_KnownEncodeableTypes[\
SOPC_TypeInternalIndex_SIZE + 1] = {
"""[:-1]

TYPE_TABLE_ENTRY = """
#ifndef OPCUA_EXCLUDE_{name}
    &OpcUa_{name}_EncodeableType,
#endif"""

TYPE_TABLE_END = """
    NULL
};
"""

C_FILE_END = """
SOPC_EncodeableType** SOPC_KnownEncodeableTypes = g_KnownEncodeableTypes;

void SOPC_Initialize_EnumeratedType(int32_t* enumerationValue)
{
    *enumerationValue = 0;
}

void SOPC_Clear_EnumeratedType(int32_t* enumerationValue)
{
    *enumerationValue = 0;
}

SOPC_ReturnStatus SOPC_Read_EnumeratedType(SOPC_Buffer* buffer, int32_t* enumerationValue, uint32_t nestedStructLevel)
{
    return SOPC_Int32_Read(enumerationValue, buffer, nestedStructLevel);
}

SOPC_ReturnStatus SOPC_Write_EnumeratedType(SOPC_Buffer* buffer, const int32_t* enumerationValue, uint32_t nestedStructLevel)
{
    return SOPC_Int32_Write(enumerationValue, buffer, nestedStructLevel);
}

/* This is the last line of an autogenerated file. */
"""

if __name__ == '__main__':
    main()
