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
import base64
import binascii
import sys
import uuid
import os
from datetime import datetime, timezone, timedelta, MINYEAR, MAXYEAR
import re
from xml.etree.ElementTree import iterparse, Element
from functools import partial

UA_NODESET_NS = '{http://opcfoundation.org/UA/2011/03/UANodeSet.xsd}'
UA_NODESET_TAG = UA_NODESET_NS + 'UANodeSet'
UA_ALIASES_TAG = UA_NODESET_NS + 'Aliases'
UA_ALIAS_TAG = UA_NODESET_NS + 'Alias'
UA_VIEW_TAG = UA_NODESET_NS + 'UAView'
UA_OBJECT_TAG = UA_NODESET_NS + 'UAObject'
UA_VARIABLE_TAG = UA_NODESET_NS + 'UAVariable'
UA_VARIABLE_TYPE_TAG = UA_NODESET_NS + 'UAVariableType'
UA_OBJECT_TYPE_TAG = UA_NODESET_NS + 'UAObjectType'
UA_REFERENCE_TYPE_TAG = UA_NODESET_NS + 'UAReferenceType'
UA_DATA_TYPE_TAG = UA_NODESET_NS + 'UADataType'
UA_METHOD_TAG = UA_NODESET_NS + 'UAMethod'
UA_DESCRIPTION_TAG = UA_NODESET_NS + 'Description'
UA_DISPLAY_NAME_TAG = UA_NODESET_NS + 'DisplayName'
UA_REFERENCES_TAG = UA_NODESET_NS + 'References'
UA_REFERENCE_TAG = UA_NODESET_NS + 'Reference'
UA_ROLEPERMISSIONS_TAG = UA_NODESET_NS + 'RolePermissions'
UA_ROLEPERMISSION_TAG = UA_NODESET_NS + 'RolePermission'
UA_VALUE_TAG = UA_NODESET_NS + 'Value'
UA_DEFINITION_TAG = UA_NODESET_NS + 'Definition'
UA_FIELD_TAG = UA_NODESET_NS + 'Field'

ID_TYPE_NUMERIC = 0
ID_TYPE_STRING = 1
ID_TYPE_GUID = 2
ID_TYPE_BYTESTRING = 3

UA_TYPES_NS = '{http://opcfoundation.org/UA/2008/02/Types.xsd}'

UA_VALUE_TYPE_BOOL = UA_TYPES_NS + 'Boolean'
UA_VALUE_TYPE_BYTE = UA_TYPES_NS + 'Byte'
UA_VALUE_TYPE_INT16 = UA_TYPES_NS + 'Int16'
UA_VALUE_TYPE_INT32 = UA_TYPES_NS + 'Int32'
UA_VALUE_TYPE_INT64 = UA_TYPES_NS + 'Int64'
UA_VALUE_TYPE_GUID = UA_TYPES_NS + 'Guid'
UA_VALUE_TYPE_NODEID = UA_TYPES_NS + 'NodeId'
UA_VALUE_TYPE_SBYTE = UA_TYPES_NS + 'SByte'
UA_VALUE_TYPE_UINT16 = UA_TYPES_NS + 'UInt16'
UA_VALUE_TYPE_UINT32 = UA_TYPES_NS + 'UInt32'
UA_VALUE_TYPE_UINT64 = UA_TYPES_NS + 'UInt64'
UA_VALUE_TYPE_FLOAT = UA_TYPES_NS + 'Float'
UA_VALUE_TYPE_DOUBLE = UA_TYPES_NS + 'Double'
UA_VALUE_TYPE_STRING = UA_TYPES_NS + 'String'
UA_VALUE_TYPE_BYTESTRING = UA_TYPES_NS + 'ByteString'
UA_VALUE_TYPE_XMLELEMENT = UA_TYPES_NS + 'XmlElement'
UA_VALUE_TYPE_DATETIME = UA_TYPES_NS + 'DateTime'
UA_VALUE_TYPE_EXTENSIONOBJECT = UA_TYPES_NS + 'ExtensionObject'
UA_VALUE_TYPE_LOCALIZEDTEXT = UA_TYPES_NS + 'LocalizedText'
UA_VALUE_TYPE_QUALIFIEDNAME = UA_TYPES_NS + 'QualifiedName'

# NodeId tags
UA_VALUE_IDENTIFIER_TAG = UA_TYPES_NS + 'Identifier'
# LocalizedText tags
UA_VALUE_LOCALE_TAG = UA_TYPES_NS + 'Locale'
UA_VALUE_TEXT_TAG = UA_TYPES_NS + 'Text'
# Extension object tags
UA_VALUE_TYPEID_TAG = UA_TYPES_NS + 'TypeId'
UA_VALUE_BODY_TAG = UA_TYPES_NS + 'Body'
# Argument structure tags
UA_VALUE_ARGUMENT_TAG = UA_TYPES_NS + 'Argument'
UA_VALUE_NAME_TAG = UA_TYPES_NS + 'Name'
UA_VALUE_VALUE_TAG = UA_TYPES_NS + 'Value'
UA_VALUE_DISPLAY_NAME_TAG = UA_TYPES_NS + 'DisplayName'
UA_VALUE_DATATYPE_TAG = UA_TYPES_NS + 'DataType'
UA_VALUE_VALUERANK_TAG = UA_TYPES_NS + 'ValueRank'
UA_VALUE_ARRAYDIMENSIONS_TAG = UA_TYPES_NS + 'ArrayDimensions'
UA_VALUE_DESCRIPTION_TAG = UA_TYPES_NS + 'Description'
# EnumValueType structure tags
UA_VALUE_ENUM_TAG = UA_TYPES_NS + 'EnumValueType'
# EUInformation structure tags
UA_VALUE_EU_INFO_TAG = UA_TYPES_NS + 'EUInformation'
UA_VALUE_NS_URI_TAG = UA_TYPES_NS + 'NamespaceUri'
UA_VALUE_UNIT_ID_TAG = UA_TYPES_NS + 'UnitId'
UA_VALUE_NAMESPACE_INDEX_TAG = UA_TYPES_NS + 'NamespaceIndex'
# Range structure tags
UA_VALUE_RANGE_TAG = UA_TYPES_NS + 'Range'
UA_VALUE_LOW_TAG = UA_TYPES_NS + 'Low'
UA_VALUE_HIGH_TAG = UA_TYPES_NS + 'High'
# RolePermissionType structure tags
UA_VALUE_ROLEID_TAG = UA_TYPES_NS + 'RoleId'
UA_VALUE_PERMISSIONS_TAG = UA_TYPES_NS + 'Permissions'
# IdentityMappingRuleType structure tags
UA_VALUE_CRITERIA_TYPE_TAG = UA_TYPES_NS + 'CriteriaType'
UA_VALUE_CRITERIA_TAG = UA_TYPES_NS + 'Criteria'

VALUE_TYPE_BOOL = 0
VALUE_TYPE_BYTE = 1
VALUE_TYPE_INT16 = 2
VALUE_TYPE_INT32 = 3
VALUE_TYPE_INT64 = 4
VALUE_TYPE_GUID = 5
VALUE_TYPE_NODEID = 6
VALUE_TYPE_SBYTE = 7
VALUE_TYPE_UINT16 = 8
VALUE_TYPE_UINT32 = 9
VALUE_TYPE_UINT64 = 10
VALUE_TYPE_FLOAT = 11
VALUE_TYPE_DOUBLE = 12
VALUE_TYPE_STRING = 13
VALUE_TYPE_BYTESTRING = 14
VALUE_TYPE_XMLELEMENT = 15
VALUE_TYPE_DATETIME = 16
VALUE_TYPE_EXTENSIONOBJECT = 17
VALUE_TYPE_EXTENSIONOBJECT_ARGUMENT = 18
VALUE_TYPE_LOCALIZEDTEXT = 19
VALUE_TYPE_EXTENSIONOBJECT_ENUMVALUETYPE = 20
VALUE_TYPE_ENGINEERING_UNIT_INFO = 21
VALUE_TYPE_QUALIFIED_NAME = 22
VALUE_TYPE_RANGE = 23
VALUE_TYPE_ROLEPERMISSION = 24
VALUE_TYPE_IDENTITY_MAPPING_RULE_TYPE = 25

UNSUPPORTED_POINTER_VARIANT_TYPES = {VALUE_TYPE_DATETIME}

C_IDENTIFIER_TYPES = [
    'SOPC_IdentifierType_Numeric',
    'SOPC_IdentifierType_String',
    'SOPC_IdentifierType_Guid',
    'SOPC_IdentifierType_ByteString'
]

c_header = '''
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

#include "sopc_address_space.h"

#include <stdio.h>
#include <stdbool.h>

#include "opcua_statuscodes.h"

#include "sopc_builtintypes.h"
#include "sopc_enums.h"
#include "sopc_types.h"
#include "sopc_macros.h"

'''.lstrip()

ACCESSLEVEL_MASK_STATUSWRITE = 0b00100000
ACCESSLEVEL_MASK_TIMESTAMPWRITE = 0b01000000

PERMISSIONTYPE_MASK_BROWSE = 0b00000001

class ParseError(Exception):
    """
    Errors raised during XML parsing
    """
    pass


class CodeGenerationError(Exception):
    """
    Errors raised during C code generation
    """
    pass


class NodeId(object):
    __slots__ = 'ns', 'ty', 'data'

    def __init__(self, ns, ty, data):
        self.ns = ns
        self.ty = ty
        self.data = data

    def __str__(self):
        s = ('ns=%d;' % self.ns) if self.ns else ''

        if self.ty == ID_TYPE_NUMERIC:
            s += ('i=%d' % self.data)
        elif self.ty == ID_TYPE_STRING:
            s += ('s=%s' % self.data)
        elif self.ty == ID_TYPE_GUID:
            s += ('g=%s' % str(self.data))
        elif self.ty == ID_TYPE_BYTESTRING:
            s += ('b=%s' % base64.standard_b64encode(self.data))

        return s

    @staticmethod
    def parse(nodeid):
        ty = None
        data = None
        cur = nodeid

        # Parse namespace
        if nodeid.startswith('ns='):
            ns_end_idx = nodeid.find(';')

            if ns_end_idx == -1:
                raise ParseError('Invalid NodeId: ' + nodeid)

            try:
                ns = int(nodeid[3:ns_end_idx])
            except ValueError:
                raise ParseError('Non integer namespace in NodeId: ' + nodeid)

            cur = cur[ns_end_idx+1:]
        else:
            ns = None

        if len(cur) < 3:
            raise ParseError('Truncated NodeId: ' + nodeid)

        if cur[0] == 'i':
            ty = ID_TYPE_NUMERIC

            try:
                data = int(cur[2:])
            except ValueError:
                raise ParseError('Invalid numeric NodeId: ' + nodeid)
        elif cur[0] == 's':
            ty = ID_TYPE_STRING
            data = cur[2:]
        elif cur[0] == 'g':
            ty = ID_TYPE_GUID

            try:
                data = uuid.UUID(cur[2:])
            except ValueError:
                raise ParseError('Invalid GUID NodeId: ' + nodeid)
        elif cur[0] == 'b':
            ty = ID_TYPE_BYTESTRING

            try:
                data = base64.standard_b64decode(cur[2:])
            except TypeError:
                raise ParseError('Invalid bytestring NodeId: ' + nodeid)

        if data == None:
            raise ParseError('Invalid NodeId: ' + nodeid)

        return NodeId(ns, ty, data)


class QName(object):
    __slots__ = 'ns', 'name'

    def __init__(self, ns, name):
        self.ns = ns
        self.name = name

    @staticmethod
    def parse(qname):
        idx = qname.find(':')

        if idx == -1:
            return QName(None, qname)

        try:
            ns = int(qname[0:idx])
        except ValueError:
            raise ParseError('Invalid namespace in qualified name: ' + qname)

        name = qname[1+idx:]

        if not name:
            raise ParseError('Missing name in qualified name: ' + qname)

        return QName(ns, name)

    def __str__(self):
        if self.ns is None:
            ns = 0
        else:
            ns = self.ns
        return f'{ns}:{self.name}'

class LocalizedText(object):
    __slots__ = 'locale', 'text'

    def __init__(self, locale, text):
        self.locale = locale
        self.text = text


class VariableValue(object):
    __slots__ = 'ty', 'val', 'is_array'

    def __init__(self, ty, val, is_array):
        self.ty = ty
        self.val = val
        self.is_array = is_array


class Node(object):
    __slots__ = 'tag', 'nodeid', 'browse_name', 'description', 'display_name', 'references', 'role_permissions',\
                'idonly', 'value', 'data_type', 'value_rank', 'accesslevel', 'executable', 'is_abstract', 'definition',\
                'event_notifier'

    def __init__(self, tag, nodeid, browse_name, description, display_name, references, role_permissions):
        self.tag = tag
        self.nodeid = nodeid
        self.browse_name = browse_name
        self.description = description
        self.display_name = display_name
        self.references = references
        self.role_permissions = role_permissions
        self.idonly = False
        self.value = None # For Variable node class only (next fields included)
        self.data_type = None
        self.value_rank = None
        self.accesslevel = None
        self.executable = None # For Method node class only
        self.is_abstract = False # For VariableType, ObjectType, ReferenceType and DataType
        self.definition = None # For DataType node class only
        self.event_notifier = None # For Object or View node class only


class Reference(object):
    __slots__ = 'ty', 'target', 'is_forward'

    def __init__(self, ty, target, is_forward):
        self.ty = ty
        self.target = target
        self.is_forward = is_forward

class RolePermission(object):
    __slots__ = 'permissions', 'roleid'

    def __init__(self, permissions, roleid):
        self.roleid = roleid
        self.permissions = permissions

# Extension objects (non-built in Structure) classes:

class ExtensionObject(object):
    __slots__ = 'extobj_typeid', 'extobj_objtype'

    def __init__(self, extobj_typeid, extobj_objtype):
        self.extobj_typeid = extobj_typeid
        self.extobj_objtype = extobj_objtype

class Argument(ExtensionObject):
    __slots__ = 'name', 'datatype', 'valuerank', 'arraydimensions', 'description'

    def __init__(self, name, datatype, valuerank, arraydimensions, description):
        self.extobj_typeid = NodeId.parse('i=298') # use binary encoding of datatype
        self.extobj_objtype = '&OpcUa_Argument_EncodeableType'
        self.name = name
        self.datatype = datatype
        self.valuerank = valuerank
        self.arraydimensions = arraydimensions
        self.description = description

class EnumValueType(ExtensionObject):
    __slots__ = 'value', 'displayname', 'description'

    def __init__(self, value, displayname, description):
        self.extobj_typeid = NodeId.parse('i=8251') # use binary encoding of datatype
        self.extobj_objtype = '&OpcUa_EnumValueType_EncodeableType'
        self.value = int(value)
        self.displayname = displayname
        self.description = description

class EUInformation(ExtensionObject):
    __slots__ = 'namespaceuri', 'unit_id', 'displayname', 'description'

    def __init__(self, namespaceuri, unit_id, displayname, description):
        self.extobj_typeid = NodeId.parse('i=889') # use binary encoding of datatype
        self.extobj_objtype = '&OpcUa_EUInformation_EncodeableType'
        self.namespaceuri = namespaceuri
        self.unit_id = int(unit_id)
        self.displayname = displayname
        self.description = description

class Range(ExtensionObject):
    __slots__ = 'low', 'high'

    def __init__(self, low, high):
        self.extobj_typeid = NodeId.parse('i=886') # use binary encoding of datatype
        self.extobj_objtype = '&OpcUa_Range_EncodeableType'
        self.low = low
        self.high = high

class ValueRolePermissionType(ExtensionObject):
    __slots__ = 'roleid', 'permissions'

    def __init__(self, roleid, permissions):
        self.extobj_typeid = NodeId.parse('i=128') # use binary encoding of datatype
        self.extobj_objtype = '&OpcUa_RolePermissionType_EncodeableType'
        self.roleid = roleid
        self.permissions = permissions

class ValueIdentityMappingRuleType(ExtensionObject):
    __slots__ = 'criteria', 'criteria_type'

    def __init__(self, criteria, criteria_type):
        self.extobj_typeid = NodeId.parse('i=15736') # use binary encoding of datatype
        self.extobj_objtype = '&OpcUa_IdentityMappingRuleType_EncodeableType'
        self.criteria = criteria
        self.criteria_type = criteria_type

class DataTypeDefinitionField(object):
    __slots__ = 'name'

    def __init__(self, name):
        self.name = name

class EnumField(DataTypeDefinitionField):
    __slots__ = 'value'

    def __init__(self, name, value):
        self.name = name
        # Set Value default value (based on XSD)
        self.value = -1
        if value is not None:
            self.value = int(value)

class StructureField(DataTypeDefinitionField):
    # note: 'isoptional' not supported
    __slots__ = 'datatype', 'valuerank', 'arraydimensions', 'maxstringlength', 'allowsubtypes'

    def __init__(self, name, datatype, valuerank, arraydimensions, maxstringlength, allowsubtypes):
        self.name = name
        if datatype is not None:
            self.datatype = NodeId.parse(datatype)
        else:
            self.datatype = NodeId.parse('i=24') # default value

        self.valuerank = -1
        if valuerank is not None:
            self.valuerank = int(valuerank)
            if self.valuerank <= 0 and self.valuerank != -1:
                raise ParseError('Expected a ValueRank shall be Scalar (-1) or a fixed rank Array (>=1).')

        self.arraydimensions = []
        if arraydimensions is not None:
            self.arraydimensions = [int(arraydim) for arraydim in arraydimensions.split(',')]
            unsigned_dimensions = [arraydim >= 0 for arraydim in self.arraydimensions]
            assert(False not in unsigned_dimensions)
            if len(self.arraydimensions) != self.valuerank:
                raise ParseError(f'Expected ArrayDimensions "{arraydimensions}" length to be equal to ValueRank "{valuerank}"')

        self.maxstringlength = 0
        if maxstringlength is not None:
            self.maxstringlength = int(maxstringlength)

        self.allowsubtypes = False
        if allowsubtypes is not None:
            self.allowsubtypes = bool(allowsubtypes)

class DataTypeDefinition(ExtensionObject):
    __slots__ = 'is_structure', 'structure_with_subtypes', 'default_encoding_id', 'base_data_type_id', 'fields'
    def __init__(self):
        self.is_structure = None
        self.structure_with_subtypes = False
        self.default_encoding_id = None
        self.base_data_type_id = None
        self.fields = []

    def add_field(self, field : DataTypeDefinitionField):
        assert (isinstance(field, StructureField) or isinstance(field, EnumField))
        if self.is_structure is None:
            self.is_structure = isinstance(field, StructureField)

        if self.is_structure and field.allowsubtypes:
            self.structure_with_subtypes = True

        self.fields.append(field)

    def finalize(self, dt_node, nodes):
        if self.is_structure:
            self.extobj_typeid = NodeId.parse('i=122') # use binary encoding of datatype StructureDefinition
            self.extobj_objtype = '&OpcUa_StructureDefinition_EncodeableType'
            hasEncodingId = 'i=38'
            hasSubtype = 'i=45'
            for ref in dt_node.references:
                if str(ref.ty) == hasEncodingId and ref.is_forward:
                    target_node = nodes.get(str(ref.target), None)
                    if target_node is not None and str(target_node.browse_name) == '0:Default Binary':
                        self.default_encoding_id = ref.target

                if str(ref.ty) == hasSubtype and not ref.is_forward:
                    self.base_data_type_id = ref.target

            if self.default_encoding_id is None:
                print(f'WARNING: DataType {dt_node.nodeid} StructureDefinition encoding node not found: use null NodeId instead')
                self.default_encoding_id = NodeId.parse('i=0')
            if self.base_data_type_id is None:
                print(f'WARNING: DataType {dt_node.nodeid} StructureDefinition base type node reference not found: use null NodeId instead')
                self.base_data_type_id = NodeId.parse('i=0')
        else:
            self.extobj_typeid = NodeId.parse('i=123') # use binary encoding of datatype EnumDefinition
            self.extobj_objtype = '&OpcUa_EnumDefinition_EncodeableType'


def expect_element(source, name=None):
    ev, n = next(source)

    if ev != 'start':
        n.clear()
        raise ParseError('Expected element start, got ' + ev)

    return check_element(n, name)


def check_element(n, name=None):
    if not isinstance(n, Element):
        n.clear()
        raise ParseError('Expected element, got %s' % str(n))

    if name is not None and n.tag != name:
        n.clear()
        raise ParseError('Expected element %s, got %s' % (name, str(n)))

    return n


def skip_element(source, name):
    while True:
        try:
            ev, n = next(source)
        except StopIteration:
            raise ParseError('Unexpected end of document while skipping to end of ' + name)

        finished = (ev == 'end' and isinstance(n, Element) and n.tag == name)
        n.clear()

        if finished:
            return


def parse_element(source, name):
    while True:
        try:
            ev, n = next(source)
        except StopIteration:
            raise ParseError('Unexpected end of document while parsing to end of ' + name)

        if ev == 'end' and isinstance(n, Element) and n.tag == name:
            return


def collect_aliases(node):
    aliases = {}

    for n in node.findall(UA_ALIAS_TAG):
        try:
            name = n.attrib['Alias']
        except KeyError:
            raise ParseError('Missing Alias for alias ' + n.text)

        aliases[name] = n.text

    return aliases


def localized_text_of_child(node, name):
    child = node.find(name)

    if child is None:
        return None

    return LocalizedText(child.get('Locale'), child.text)


def collect_node_references(node, aliases):
    refs = []

    refs_node = node.find(UA_REFERENCES_TAG)

    if refs_node is None:
        return refs

    for n in refs_node.findall(UA_REFERENCE_TAG):
        try:
            ref_type = n.attrib['ReferenceType']
        except KeyError:
            raise ParseError('Missing ReferenceType on Reference element for node ' + node.get('NodeId'))

        # In case there is an alias, resolve it
        ref_type = NodeId.parse(aliases.get(ref_type, ref_type))

        is_forward = (parse_boolean_value(n.get('IsForward', 'true')))

        refs.append(Reference(ref_type, NodeId.parse(n.text), is_forward))

    return refs

def parse_role_permissions(node):
    rolepermissions = []

    rolepermissions_node = node.find(UA_ROLEPERMISSIONS_TAG)

    if rolepermissions_node is None:
        return rolepermissions

    for n in rolepermissions_node.findall(UA_ROLEPERMISSION_TAG):
        try:
            permissions = n.attrib['Permissions']
            if 0 == int(permissions) & PERMISSIONTYPE_MASK_BROWSE:
                print("WARNING: permission Browse set to 0 will be ignored because this permission is not managed.")
        except KeyError:
            raise ParseError('Missing Permissions on RolePermission element for node ' + node.get('NodeId'))

        rolepermissions.append(RolePermission(permissions, NodeId.parse(n.text)))

    return rolepermissions


def parse_boolean_value(text : str) -> bool:
    return text.strip() == 'true'


def parse_guid(guid):
    try:
        string = guid.find(UA_VALUE_TYPE_STRING)
        if string is None:
            return uuid.UUID("")
        else:
            return uuid.UUID(string.text)
    except ValueError:
        raise ParseError('Invalid GUID: %s' % guid)

def boolean_to_string(val):
    return 'true' if val else 'false'


# Collects all the uax:{tag_name} items in a uax:ListOfXXX element
def collect_list_items(n, tag_name, is_simple_type, parse_func):
    if is_simple_type:
        return list(map(lambda x: parse_func(x.text), n.findall(tag_name)))
    else:
        return list(map(parse_func, n.findall(tag_name)))


# Parses the base type of a value tag name (eg. uax:Boolean, uax:ListOfString...)
# and returns the base type (eg. uax:Boolean, uax:String...) and whether the value
# is an array or not.
# Returns a (base_type_url, is_array) tuple.
def parse_value_tag(tag_name):
    LIST_PREFIX = 'ListOf'

    if not tag_name.startswith(UA_TYPES_NS):
        return tag_name, False

    base_name = tag_name[len(UA_TYPES_NS):]

    if base_name.startswith(LIST_PREFIX):
        return UA_TYPES_NS + base_name[len(LIST_PREFIX):], True
    else:
        return tag_name, False


def identity(x):
    return x

# Bytestrings are encoded as base4
def decode_bytestring(x):
    if x is None:
        return None
    else:
        return binascii.a2b_base64(x.encode('utf-8')).decode('utf-8')

# NodeId values are complex data types
def parse_node_id(n):
    identifier = n.find(UA_VALUE_IDENTIFIER_TAG)
    if identifier is None:
        return NodeId.parse("i=0")
    else:
        return NodeId.parse(identifier.text)

def parse_localized_text(n):
    locale = n.find(UA_VALUE_LOCALE_TAG)
    text = n.find(UA_VALUE_TEXT_TAG)
    locale_text = None if locale is None else locale.text
    text_text = None if text is None else text.text
    return LocalizedText(locale_text, text_text)

# Extension object values: manage some extensions objects

def parse_argument_body(n):
    argument = n.find(UA_VALUE_ARGUMENT_TAG)
    if argument is None:
        raise ParseError('Argument extension object without Argument tag')
    name = argument.find(UA_VALUE_NAME_TAG)
    datatype = argument.find(UA_VALUE_DATATYPE_TAG)
    valuerank = argument.find(UA_VALUE_VALUERANK_TAG)
    arraydimensions = argument.find(UA_VALUE_ARRAYDIMENSIONS_TAG)
    descriptions = argument.find(UA_VALUE_DESCRIPTION_TAG)
    if datatype is None:
        raise ParseError('Argument extension object without DataType')
    if valuerank is None:
        raise ParseError('Argument extension object without ValueRank')

    datatype_nodeid = parse_node_id(datatype)
    if datatype_nodeid is None:
        raise ParseError('Argument extension object with invalid DataType nodeId %s' % datatype_nodeid)
    if arraydimensions is None:
        arraydimensions_list = []
    else:
        arraydimensions_list = collect_list_items(arraydimensions, UA_VALUE_TYPE_UINT32, True, str)
    if descriptions is not None:
        descriptions = parse_localized_text(descriptions)

    return Argument(name.text, datatype_nodeid, int(valuerank.text), arraydimensions_list, descriptions)

def parse_enum_value_type_body(n):
    enum = n.find(UA_VALUE_ENUM_TAG)

    if enum is None:
        raise ParseError('EnumValueType extension object without EnumValueType tag')

    value = enum.find(UA_VALUE_VALUE_TAG)
    displayName = enum.find(UA_VALUE_DISPLAY_NAME_TAG)
    descriptions = enum.find(UA_VALUE_DESCRIPTION_TAG)

    if value is None:
        raise ParseError('EnumValueType extension object without Value')

    if displayName is None:
        raise ParseError('EnumValueType extension object without DisplayName')
    else:
        displayName = parse_localized_text(displayName)

    if descriptions is not None:
        descriptions = parse_localized_text(descriptions)

    return EnumValueType(value.text, displayName, descriptions)

def parse_engineering_unit_info_body(n):
    eu = n.find(UA_VALUE_EU_INFO_TAG)

    if eu is None:
        raise ParseError('EUInformation extension object without EUInformation tag')

    namespaceuri = eu.find(UA_VALUE_NS_URI_TAG)
    unit_id = eu.find(UA_VALUE_UNIT_ID_TAG)

    displayName = eu.find(UA_VALUE_DISPLAY_NAME_TAG)
    description= eu.find(UA_VALUE_DESCRIPTION_TAG)

    if namespaceuri is None:
        raise ParseError('EUInformation extension object without NamespaceUri')

    if unit_id is None:
        raise ParseError('EUInformation extension object without UnitId')

    if displayName is None:
        raise ParseError('EUInformation extension object without DisplayName')
    else:
        displayName = parse_localized_text(displayName)

    if description is not None:
        description = parse_localized_text(description)

    return EUInformation(namespaceuri.text, unit_id.text, displayName, description)

def parse_range_body(n):
    vRange = n.find(UA_VALUE_RANGE_TAG)

    if vRange is None:
        raise ParseError('Range extension object without Range tag')

    low = vRange.find(UA_VALUE_LOW_TAG)
    high = vRange.find(UA_VALUE_HIGH_TAG)

    if low is None:
        raise ParseError('Range without Low tag')

    if high is None:
        raise ParseError('Range without High tag')

    return Range(low.text, high.text)

def parse_role_permission_type_body(n):
    roleid = n.find(UA_VALUE_ROLEID_TAG)
    permissions = n.find(UA_VALUE_PERMISSIONS_TAG)

    if roleid is None:
        raise ParseError('RolePermission without RoleId tag')

    if permissions is None:
        raise ParseError('RolePermission without Permissions tag')

    if 0 == int(permissions.text) & PERMISSIONTYPE_MASK_BROWSE:
        print("WARNING: DRP permission Browse set to 0 will be ignored because this permission is not managed.")

    roleid_nodeid = parse_node_id(roleid)
    if roleid_nodeid is None:
        raise ParseError('RolePermission extension object with invalid RoleId nodeId %s' % roleid_nodeid)

    return ValueRolePermissionType(roleid_nodeid, permissions.text)

def parse_identity_mapping_rule_type_body(n):
    criteria = n.find(UA_VALUE_CRITERIA_TAG)
    criteria_type = n.find(UA_VALUE_CRITERIA_TYPE_TAG)

    if criteria is None:
        raise ParseError('Identity without Criteria tag')

    if criteria_type is None:
        raise ParseError('Identity without CriteriaType tag')

    return ValueIdentityMappingRuleType(criteria.text, criteria_type.text)

def parse_qualified_name(n):

    namespace_index = n.find(UA_VALUE_NAMESPACE_INDEX_TAG)
    name = n.find(UA_VALUE_NAME_TAG)

    if namespace_index is None:
        raise ParseError('QualifiedName without NamespaceIndex')
    if name is None:
        raise ParseError('QualifiedName without Value')

    return QName(int(namespace_index.text), name.text)

EXTENSION_OBJECT_PARSERS_DICT = {
    # Argument XML encoding nodeId
    297: (parse_argument_body, VALUE_TYPE_EXTENSIONOBJECT_ARGUMENT),
    # Argument datatype nodeId
    296: (parse_argument_body, VALUE_TYPE_EXTENSIONOBJECT_ARGUMENT),

    # Engineering Unit XML encoding nodeId
    888: (parse_engineering_unit_info_body, VALUE_TYPE_ENGINEERING_UNIT_INFO),
    # Engineering Unit datatype nodeId
    887: (parse_engineering_unit_info_body, VALUE_TYPE_ENGINEERING_UNIT_INFO),

    # Range datatype and XML encoding ids
    884: (parse_range_body, VALUE_TYPE_RANGE),
    885: (parse_range_body, VALUE_TYPE_RANGE),

    # RolePermission XML encoding nodeId
    16126: (parse_role_permission_type_body, VALUE_TYPE_ROLEPERMISSION),
    # RolePermission datatype nodeId
    96: (parse_role_permission_type_body, VALUE_TYPE_ROLEPERMISSION),

    # IdentityMappingRuleType XML encoding nodeId
    15728: (parse_identity_mapping_rule_type_body, VALUE_TYPE_IDENTITY_MAPPING_RULE_TYPE),
    # IdentityMappingRuleType datatype nodeId
    15634: (parse_identity_mapping_rule_type_body, VALUE_TYPE_IDENTITY_MAPPING_RULE_TYPE),

    # EnumValueType datataype nodeId
    7616 : (parse_enum_value_type_body, VALUE_TYPE_EXTENSIONOBJECT_ENUMVALUETYPE),
    # EnumValueType datataype nodeId
    7594 : (parse_enum_value_type_body, VALUE_TYPE_EXTENSIONOBJECT_ENUMVALUETYPE),
}

def get_extension_object_parser_and_type(nodeid):
    # nodeid should either be the DataType nodeId, or the HasEncoding node nodeId associated
    if ID_TYPE_NUMERIC == nodeid.ty and nodeid.ns in (None, 0):
        # support only some of OPC UA namespace types
        nodeid_int =  int(nodeid.data)
        if nodeid_int in EXTENSION_OBJECT_PARSERS_DICT:
            return EXTENSION_OBJECT_PARSERS_DICT.get(nodeid_int, None)
    return (None, None)

def get_extension_object_type(n):
    typeid = n.find(UA_VALUE_TYPEID_TAG)
    body = n.find(UA_VALUE_BODY_TAG)
    if typeid is None or body is None:
        raise ParseError('TypeId or Body missing in ExtensionObject value')
    else:
        datatype_or_encoding_nodeid = parse_node_id(typeid)
        _ , ty = get_extension_object_parser_and_type(datatype_or_encoding_nodeid)
    return ty

def parse_extension_object(n):
    typeid = n.find(UA_VALUE_TYPEID_TAG)
    body = n.find(UA_VALUE_BODY_TAG)
    if typeid is None or body is None:
        raise ParseError('TypeId or Body missing in ExtensionObject value')
    else:
        datatype_or_encoding_nodeid = parse_node_id(typeid)
        extension_object_parser, _ = get_extension_object_parser_and_type(datatype_or_encoding_nodeid)
        if extension_object_parser is None:
            raise ParseError('TypeId or Body missing in ExtensionObject value')
        return extension_object_parser(body)

# Returns a VariableValue object
def collect_variable_value(n):

    value_node = n.find(UA_VALUE_TAG)

    if value_node is None:
        # print('No Value tag in Variable or VariableType for node %s' % n.attrib['NodeId'])
        return None

    if len(value_node) != 1:
        raise ParseError('Value tag should have exactly one children')

    value = list(value_node)[0]
    base_type, is_array = parse_value_tag(value.tag)

    # is_simple_type == True => value is just the text tag content
    is_simple_type = True

    if base_type == UA_VALUE_TYPE_BOOL:
        ty = VALUE_TYPE_BOOL
        parse_func = parse_boolean_value
    elif base_type == UA_VALUE_TYPE_BYTE:
        ty = VALUE_TYPE_BYTE
        parse_func = int
    elif base_type == UA_VALUE_TYPE_INT16:
        ty = VALUE_TYPE_INT16
        parse_func = int
    elif base_type == UA_VALUE_TYPE_INT32:
        ty = VALUE_TYPE_INT32
        parse_func = int
    elif base_type == UA_VALUE_TYPE_INT64:
        ty = VALUE_TYPE_INT64
        parse_func = int
    elif base_type == UA_VALUE_TYPE_GUID:
        is_simple_type = False
        ty = VALUE_TYPE_GUID
        parse_func = parse_guid
    elif base_type == UA_VALUE_TYPE_NODEID:
        is_simple_type = False
        ty = VALUE_TYPE_NODEID
        parse_func = parse_node_id
    elif base_type == UA_VALUE_TYPE_SBYTE:
        ty = VALUE_TYPE_SBYTE
        parse_func = int
    elif base_type == UA_VALUE_TYPE_UINT16:
        ty = VALUE_TYPE_UINT16
        parse_func = int
    elif base_type == UA_VALUE_TYPE_UINT32:
        ty = VALUE_TYPE_UINT32
        parse_func = int
    elif base_type == UA_VALUE_TYPE_UINT64:
        ty = VALUE_TYPE_UINT64
        parse_func = int
    elif base_type == UA_VALUE_TYPE_FLOAT:
        ty = VALUE_TYPE_FLOAT
        parse_func = float
    elif base_type == UA_VALUE_TYPE_DOUBLE:
        ty = VALUE_TYPE_DOUBLE
        parse_func = float
    elif base_type == UA_VALUE_TYPE_STRING:
        ty = VALUE_TYPE_STRING
        parse_func = identity
    elif base_type == UA_VALUE_TYPE_BYTESTRING:
        ty = VALUE_TYPE_BYTESTRING
        parse_func = decode_bytestring
    elif base_type == UA_VALUE_TYPE_XMLELEMENT:
        # TODO: is_simple_type = False => rest of XML shall be converted to string
        ty = VALUE_TYPE_XMLELEMENT
        parse_func = identity
    elif base_type == UA_VALUE_TYPE_DATETIME:
        ty = VALUE_TYPE_DATETIME
        parse_func = identity
    elif base_type == UA_VALUE_TYPE_LOCALIZEDTEXT:
        is_simple_type = False
        ty = VALUE_TYPE_LOCALIZEDTEXT
        parse_func = parse_localized_text
    elif base_type == UA_VALUE_TYPE_QUALIFIEDNAME:
        is_simple_type = False
        ty = VALUE_TYPE_QUALIFIED_NAME
        parse_func = parse_qualified_name
    elif base_type == UA_VALUE_TYPE_EXTENSIONOBJECT:
        is_simple_type = False
        # Define type of extension object (from TypeId)
        if is_array:
            # All extension objects shall be the same in case of an array
            ty_list = collect_list_items(value, base_type, is_simple_type, get_extension_object_type)
            for x in ty_list:
                if x != ty_list[0]:
                    raise ParseError('Different types in a ExtensionObject List of node %s' % n.attrib['NodeId'])
            ty = ty_list[0]
        else:
            ty = get_extension_object_type(value)
        parse_func = parse_extension_object
    else:
        raise ParseError('Unknown value type %s for node %s' % (value.tag, n.attrib['NodeId']))

    if is_array:
        val = collect_list_items(value, base_type, is_simple_type, parse_func)
    else:
        val_in = value.text if is_simple_type else value
        val = parse_func(val_in)

    return VariableValue(ty, val, is_array)

def parse_datatype_attribute(datatype_attr, aliases):
    node = NodeId.parse(aliases.get(datatype_attr, datatype_attr))
    return node

def parse_value_rank_attribute(value_rank_node):
    value_rank = -1
    if 'ValueRank' in value_rank_node.attrib:
        try:
            value_rank = int(value_rank_node.attrib['ValueRank'])
        except:
            raise ParseError('Non integer ValueRank for node %s' % value_rank_node['NodeId'])
    return value_rank


def generate_string(data):
    assert data is None or isinstance(data, str), "Invalid string data: %r" % data

    if data is not None:
        # remove linebreaks
        data = data.replace('\n', '').replace('\r', '')
        return 'SOPC_STRING("%s")' % data.replace('"', '\\"')

    return 'SOPC_STRING_NULL'


def generate_guid(data):
    assert isinstance(data, uuid.UUID)

    members = list(data.fields[0:5])

    for i in range(5, -1, -1):
        members.append((data.fields[5] >> (8*i)) & 0xFF)

    return '{0x%02x, 0x%02x, 0x%02x, {0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x}}' % tuple(members)

def generate_byte_string(data):
    return generate_string(data)

def generate_qname(qname):
    assert isinstance(qname, QName)
    return 'SOPC_QUALIFIED_NAME(%d, "%s")' % (qname.ns or 0, qname.name or "")

def generate_qname_pointer(qname):
    return '(SOPC_QualifiedName[]) {%s}' % generate_qname(qname)

def generate_guid_pointer(val):
    return '(SOPC_Guid[]) {%s}' % generate_guid(val)

def generate_nodeid(nodeid):
    id_type = C_IDENTIFIER_TYPES[nodeid.ty]
    data_struct_field = '.Data.' + id_type[len('SOPC_IdentifierType_'):]

    if nodeid.ty == ID_TYPE_NUMERIC:
        if nodeid.ns:
            return 'SOPC_NODEID_NUMERIC(%d, %d)' % (nodeid.ns, nodeid.data)
        else:
            return 'SOPC_NODEID_NS0_NUMERIC(%d)' % nodeid.data
    elif nodeid.ty == ID_TYPE_STRING:
        return 'SOPC_NODEID_STRING(%d, "%s")' % (nodeid.ns or 0, nodeid.data)
    elif nodeid.ty == ID_TYPE_GUID:
        data_struct_val = generate_guid(nodeid.data)
    elif nodeid.ty == ID_TYPE_BYTESTRING:
        data_struct_val = generate_byte_string(nodeid.data)
    else:
        raise CodeGenerationError('Unknown NodeId data type')

    return '{%s, %d, %s = %s}' % (id_type, nodeid.ns or 0, data_struct_field, data_struct_val)

def generate_nodeid_pointer(val):
    return '(SOPC_NodeId[]) {%s}' % generate_nodeid(val)

def generate_localized_text(text):
    if text is None:
        return '{%s, %s}' % (generate_string(None), generate_string(None))

    if text.locale is not None:
        # remove linebreaks and whitespaces (should not exist in locale)
        text.locale = text.locale.replace('\n', '').replace('\r', '').replace(' ','')

    return 'SOPC_LOCALIZED_TEXT("%s", "%s")' % (text.locale or "", text.text or "")

def generate_localized_text_pointer(ltext):
    return '(SOPC_LocalizedText[]) {%s}' % generate_localized_text(ltext)

def generate_variant(type_id, c_type, field, val, is_array, generate_func):
    if is_array:
        field += 'Arr'
        c_array = '(%s[]){%s}' % (c_type, ','.join(map(generate_func, val))) if val else 'NULL'
        return '''
                  {true,
                   %s,
                   SOPC_VariantArrayType_Array,
                   {.Array =
                     {%d, {.%s = %s}}}}''' % (type_id, len(val), field, c_array)
    else:
        return '''
                  {true,
                   %s,
                   SOPC_VariantArrayType_SingleValue,
                   {.%s = %s}}''' % (type_id, field, generate_func(val))

# Generic extension object generator
def generate_extension_object(ext_obj, gen, is_array=False):
    type_id_field = '{%s, %s, 0}' % (generate_nodeid(ext_obj.extobj_typeid), generate_string(None))
    encoding = 'SOPC_ExtObjBodyEncoding_Object'
    body = '.Body.Object = {%s, %s}' % (gen(ext_obj), ext_obj.extobj_objtype)
    ext_obj = '{%s,%s,%s}' % (type_id_field, encoding, body)
    if is_array:
        return ext_obj
    else:
        # extension object is a pointer in variant, trick the compiler using a 1-element array
        return '''
                  (SOPC_ExtensionObject[])
                  {%s}
               ''' % ext_obj

# Specific extension object content generators

def generate_argument_ext_obj(obj):
    array_dimensions =('(uint32_t[]){%s}' % ','.join(obj.arraydimensions)
                       if obj.arraydimensions not in (None, []) else 'NULL')
    return ('''
               (OpcUa_Argument[])
               {{%s,
                 %s,
                 %s,
                 %d,
                 %d,
                 %s,
                 %s}}
            ''' %
            (obj.extobj_objtype,
             generate_string(obj.name),
             generate_nodeid(obj.datatype),
             obj.valuerank,
             len(obj.arraydimensions),
             array_dimensions,
             generate_localized_text(obj.description)
            )
           )

def generate_enum_value_type_ext_obj(obj):
    return ('''
               (OpcUa_EnumValueType[])
               {{%s,
                 %d,
                 %s,
                 %s}}
            ''' %
            (obj.extobj_objtype,
             obj.value,
             generate_localized_text(obj.displayname),
             generate_localized_text(obj.description)
            )
           )

def generate_engineering_unit_ext_obj(obj):
    return ('''
               (OpcUa_EUInformation[])
               {{%s,
                 %s,
                 %d,
                 %s,
                 %s}}
            ''' %
            (obj.extobj_objtype,
             generate_string(obj.namespaceuri),
             obj.unit_id,
             generate_localized_text(obj.displayname),
             generate_localized_text(obj.description)
            )
           )

def generate_range_ext_obj(obj):
    return ('''
               (OpcUa_Range[])
               {{%s,
                 %s,
                 %s}}
            ''' %
            (obj.extobj_objtype,
             obj.low,
             obj.high
            )
           )

def generate_rolepermissions_ext_obj(obj):
    return ('''
               (OpcUa_RolePermissionType[])
               {{%s,
                 %s,
                 %s}}
            ''' %
            (obj.extobj_objtype,
             generate_nodeid(obj.roleid),
             obj.permissions
            )
           )

def generate_identity_mapping_rule_type_ext_obj(obj):
    return ('''
               (OpcUa_IdentityMappingRuleType[])
               {{%s,
                 %s,
                 %s}}
            ''' %
            (obj.extobj_objtype,
             obj.criteria_type,
             generate_string(obj.criteria)
            )
           )

def gen_definition_enum_field(field):
    return ('''
            {%s,
              %d,
              %s,
              %s,
              %s}
            ''' %
            ('&OpcUa_EnumField_EncodeableType',
             field.value,
             generate_localized_text(None),
             generate_localized_text(None),
             generate_string(field.name)
            )
           )


def gen_definition_struct_field(field):
    array_dimensions =('(uint32_t[]){%s}' % ','.join([str(arraydim) for arraydim in field.arraydimensions])
                       if field.arraydimensions not in (None, []) else 'NULL')
    return ('''
            {%s,
              %s,
              %s,
              %s,
              %d,
              %d,
              %s,
              %d,
              %s}
            ''' %
            ('&OpcUa_StructureField_EncodeableType',
             generate_string(field.name),
             generate_localized_text(None),
             generate_nodeid(field.datatype),
             field.valuerank,
             len(field.arraydimensions),
             array_dimensions,
             field.maxstringlength,
             boolean_to_string(field.allowsubtypes))
           )

def generate_definition_fields(obj):
    if obj.is_structure:
        gen_type = 'OpcUa_StructureField'
        gen_func = gen_definition_struct_field
    else:
        gen_type = 'OpcUa_EnumField'
        gen_func = gen_definition_enum_field

    c_array = '(%s[]){%s}' % (gen_type, ','.join(map(gen_func, obj.fields))) if obj.fields not in (None, []) else 'NULL'
    return c_array

def generate_definition_ext_obj(obj):
    if obj.is_structure:
        structure_type = 'OpcUa_StructureType_Structure'
        if obj.structure_with_subtypes:
            structure_type = 'OpcUa_StructureType_StructureWithSubtypedValues'
        return ('''
            (%s[])
            {{%s,
             %s,
             %s,
             %s,
             %d,
             %s}}
            ''' %
            ('OpcUa_StructureDefinition',
             obj.extobj_objtype,
             generate_nodeid(obj.default_encoding_id),
             generate_nodeid(obj.base_data_type_id),
             structure_type,
             len(obj.fields),
             generate_definition_fields(obj),
            )
           )
    else:
        return ('''
            (%s[])
            {{%s,
             %d,
             %s}}
            ''' %
            ('OpcUa_EnumDefinition',
             obj.extobj_objtype,
             len(obj.fields),
             generate_definition_fields(obj),
            )
           )

def parse_xml_datetime(val):
    dt = re.match('''^
      (?P<year>-?([1-9][0-9]{3,}|0[0-9]{3})) - (?P<month>[0-1][0-9]) - (?P<day>[0-3][0-9])
      T (?P<hour>[0-2][0-9]) : (?P<minute>[0-5][0-9]) : (?P<second>[0-5][0-9])
      (?P<sec_frac>\.[0-9]{1,})?
      (?P<tz>
        Z | (?P<tz_sign>[-+]) (?P<tz_hr>[0-1][0-9]) : (?P<tz_min>[0-5][0-9])
      )?
      $''', val, re.X)
    if dt is not None:
        values = dt.groupdict()

        # Retrieve sec_frac if present
        sec_frac = 0 if values['sec_frac'] is None else float(values['sec_frac'])

        # Compute UTC offset
        if not values['tz'] in ('Z', None):
            td = timedelta(hours=int(values['tz_hr']), minutes=int(values['tz_min']))
            tzinfo = timezone(td) if values['tz_sign'] == '+' else timezone(-td)
        else:
            tzinfo = timezone.utc

        del values['sec_frac']
        del values['tz']
        del values['tz_sign']
        del values['tz_hr']
        del values['tz_min']

        values = {k:int(v) for k,v in values.items()}
        values['tzinfo'] = tzinfo

        # Manage special cases
        delta = timedelta()
        # 24:00:00 case
        if values['hour'] == 24:
            if sec_frac != 0 or values['minute'] != 0 or values['second'] != 0:
                raise ValueError('Only time allowed with 24 hour is 24:00:00, "{}" not compliant'.format(val))
            values['hour'] = 23
            delta = timedelta(hours=1)
            # 14:00 utc offset is min/max
        if abs(values['tzinfo'].utcoffset(None)) > timedelta(hours=14, minutes=00):
            raise ValueError('Min/Max UTC offset is -14:00/14:00, "{}" not compliant'.format(values['tzinfo']))
        try:
            final_dt = datetime(**values) + delta
        except OverflowError:
            # Manage 24:00:00 on 9999-12-31 (no microseconds included)
            final_dt =  datetime(MAXYEAR, 12, 31, 23, 59, 59, tzinfo=values['tzinfo'])
        except ValueError:
            if values['year'] > MAXYEAR:
                # max datetime with tzinfo
                final_dt =  datetime(MAXYEAR, 12, 31, 23, 59, 59, tzinfo=timezone.utc)
                if values['year'] == MAXYEAR +1 and values['month'] == 1 and values['day'] == 1:
                    offset_dt = tzinfo.utcoffset(None)
                    time_dt = timedelta(hours=values['hour'], minutes=values['minute'], seconds=values['second'])
                    if offset_dt  > time_dt:
                        # apply the timezone offset to datetime data to keep year < MAXYEAR
                        values['year'] -= 1
                        values['month'] = 12
                        values['day'] = 31
                        values['hour'] = (values['hour'] - int(offset_dt.seconds / 3600)) % 24
                        values['minute'] = (values['minute'] - int(offset_dt.seconds % 3600 / 60)) % 60
                        values['tzinfo'] = timezone.utc
                        try:
                            final_dt = datetime(**values)
                        except:
                            # In case of error, keep max datetime
                            None
            elif values['year'] < MINYEAR:
                # Min datetime with tzinfo
                final_dt =  datetime(MINYEAR, 1, 1, 0, 0, 0, tzinfo=timezone.utc)
            else:
                raise ValueError('Unexpected datetime parameters: {}'.format(values))
        return final_dt, sec_frac
    else:
        raise ValueError('Parsed value {} does not match XML datetime format'.format(val))

def generate_datetime_str(val):
    # Parse xsd:datetime string to a Python datetime + seconds fraction (might be None)
    dt, sec_frac = parse_xml_datetime(val)
    dt_epoch_1601=datetime(1601, 1, 1, 0, 0, 0, tzinfo=timezone.utc)
    # Filter min/max values for binary format
    if dt <= dt_epoch_1601:
        # A date/time value is encoded as 0 if is equal to or earlier than 1601-01-01 12:00AM UTC
        return str(0)
    elif dt >= datetime(9999, 12, 31, 23, 59, 59, tzinfo=timezone.utc):
        ## A date/time is encoded as the maximum value for an Int64 if
        ## the value is equal to or greater than 9999-12-31 11:59:59PM UTC
        return str(2**63-1)
    # Convert POSIX timestamp to OPC UA DateTime (number of 100ns)
    # Offset the date to convert to OPC UA date format
    offset = datetime.fromtimestamp(0, tz=timezone.utc) - datetime(1601, 1, 1, 0, 0, 0, tzinfo=timezone.utc)
    sec_to_100ns = int(1e7)
    # Convert fraction of seconds into a number of 100ns if present
    sec_frac_to_100ns = int(sec_frac * sec_to_100ns)
    return str(int(dt.timestamp() + offset.total_seconds()) * sec_to_100ns + sec_frac_to_100ns)

def generate_value_variant(val):
    if val is None:
        return 'SOPC_VARIANT_NULL'

    if val.ty == VALUE_TYPE_BOOL:
        return generate_variant('SOPC_Boolean_Id', 'SOPC_Boolean', 'Boolean', val.val, val.is_array, boolean_to_string)
    elif val.ty == VALUE_TYPE_BYTE:
        return generate_variant('SOPC_Byte_Id', 'SOPC_Byte', 'Byte', val.val, val.is_array, str)
    elif val.ty == VALUE_TYPE_INT16:
        return generate_variant('SOPC_Int16_Id', 'int16_t', 'Int16', val.val, val.is_array, str)
    elif val.ty == VALUE_TYPE_INT32:
        return generate_variant('SOPC_Int32_Id', 'int32_t', 'Int32', val.val, val.is_array, str)
    elif val.ty == VALUE_TYPE_INT64:
        return generate_variant('SOPC_Int64_Id', 'int64_t', 'Int64', val.val, val.is_array, lambda x: str(x) + 'L')
    elif val.ty == VALUE_TYPE_SBYTE:
        return generate_variant('SOPC_SByte_Id', 'SOPC_SByte', 'Sbyte', val.val, val.is_array, str)
    elif val.ty == VALUE_TYPE_UINT16:
        return generate_variant('SOPC_UInt16_Id', 'uint16_t', 'Uint16', val.val, val.is_array, str)
    elif val.ty == VALUE_TYPE_UINT32:
        return generate_variant('SOPC_UInt32_Id', 'uint32_t', 'Uint32', val.val, val.is_array, str)
    elif val.ty == VALUE_TYPE_UINT64:
        return generate_variant('SOPC_UInt64_Id', 'uint64_t', 'Uint64', val.val, val.is_array, lambda x: str(x) + 'UL')
    elif val.ty == VALUE_TYPE_FLOAT:
        return generate_variant('SOPC_Float_Id', 'float', 'Floatv', val.val, val.is_array, lambda x : str(x) + 'f')
    elif val.ty == VALUE_TYPE_DOUBLE:
        return generate_variant('SOPC_Double_Id', 'double', 'Doublev', val.val, val.is_array, str)
    elif val.ty == VALUE_TYPE_STRING:
        return generate_variant('SOPC_String_Id', 'SOPC_String', 'String', val.val, val.is_array, generate_string)
    elif val.ty == VALUE_TYPE_BYTESTRING:
        return generate_variant('SOPC_ByteString_Id', 'SOPC_ByteString', 'Bstring', val.val, val.is_array, generate_string)
    elif val.ty == VALUE_TYPE_XMLELEMENT:
        return generate_variant('SOPC_XmlElement_Id', 'SOPC_XmlElement', 'XmlElt', val.val, val.is_array, generate_string)
    elif val.ty == VALUE_TYPE_LOCALIZEDTEXT:
        return generate_variant('SOPC_LocalizedText_Id', 'SOPC_LocalizedText', 'LocalizedText',
                                val.val, val.is_array,
                                generate_localized_text if val.is_array else generate_localized_text_pointer)
    elif val.ty == VALUE_TYPE_QUALIFIED_NAME:
        return generate_variant('SOPC_QualifiedName_Id', 'SOPC_QualifiedName' , 'Qname',
                                val.val, val.is_array, generate_qname if val.is_array else generate_qname_pointer)
    elif val.ty == VALUE_TYPE_GUID:
        return generate_variant('SOPC_Guid_Id', 'SOPC_Guid', 'Guid',
                                val.val, val.is_array,
                                generate_guid if val.is_array else generate_guid_pointer)
    elif val.ty == VALUE_TYPE_NODEID:
        return generate_variant('SOPC_NodeId_Id', 'SOPC_NodeId', 'NodeId',
                                val.val, val.is_array,
                                generate_nodeid if val.is_array else generate_nodeid_pointer)
    elif val.ty == VALUE_TYPE_EXTENSIONOBJECT_ARGUMENT:
        # Partial evaluation of generate_extension_object to make it compatible with generate_variant
        extension_object_generator = partial(generate_extension_object,
                                             gen=generate_argument_ext_obj, is_array=val.is_array)
        return generate_variant('SOPC_ExtensionObject_Id', 'SOPC_ExtensionObject' , 'ExtObject',
                                val.val, val.is_array, extension_object_generator)
    elif val.ty == VALUE_TYPE_EXTENSIONOBJECT_ENUMVALUETYPE:
        # Partial evaluation of generate_extension_object to make it compatible with generate_variant
        extension_object_generator = partial(generate_extension_object,
                                             gen=generate_enum_value_type_ext_obj, is_array=val.is_array)
        return generate_variant('SOPC_ExtensionObject_Id', 'SOPC_ExtensionObject' , 'ExtObject',
                                val.val, val.is_array, extension_object_generator)
    elif val.ty == VALUE_TYPE_ENGINEERING_UNIT_INFO:
        # Partial evaluation of generate_extension_object to make it compatible with generate_variant
        extension_object_generator = partial(generate_extension_object,
                                             gen=generate_engineering_unit_ext_obj, is_array=val.is_array)
        return generate_variant('SOPC_ExtensionObject_Id', 'SOPC_ExtensionObject' , 'ExtObject',
                                val.val, val.is_array, extension_object_generator)
    elif val.ty == VALUE_TYPE_RANGE:
        # Partial evaluation of generate_extension_object to make it compatible with generate_variant
        extension_object_generator = partial(generate_extension_object,
                                             gen=generate_range_ext_obj, is_array=val.is_array)
        return generate_variant('SOPC_ExtensionObject_Id', 'SOPC_ExtensionObject' , 'ExtObject',
                                val.val, val.is_array, extension_object_generator)
    elif val.ty == VALUE_TYPE_ROLEPERMISSION:
        # Partial evaluation of generate_extension_object to make it compatible with generate_variant
        extension_object_generator = partial(generate_extension_object,
                                             gen=generate_rolepermissions_ext_obj, is_array=val.is_array)
        return generate_variant('SOPC_ExtensionObject_Id', 'SOPC_ExtensionObject' , 'ExtObject',
                                val.val, val.is_array, extension_object_generator)
    elif val.ty == VALUE_TYPE_IDENTITY_MAPPING_RULE_TYPE:
        # Partial evaluation of generate_extension_object to make it compatible with generate_variant
        extension_object_generator = partial(generate_extension_object,
                                             gen=generate_identity_mapping_rule_type_ext_obj, is_array=val.is_array)
        return generate_variant('SOPC_ExtensionObject_Id', 'SOPC_ExtensionObject' , 'ExtObject',
                                val.val, val.is_array, extension_object_generator)
    elif val.ty == VALUE_TYPE_DATETIME:
        return generate_variant('SOPC_DateTime_Id', 'SOPC_DateTime', 'Date', val.val, val.is_array, generate_datetime_str)
    elif val.ty in UNSUPPORTED_POINTER_VARIANT_TYPES:
        # FIXME: The variant requires a pointer here, we need to wrap the value inside a 1-sized
        # array to trick the compiler.
        raise CodeGenerationError('This value type is not supported yet')
    else:
        raise CodeGenerationError('Unknown value type: %d' % val.ty)

def collect_dt_definition(aliases, dt_node, dt_xml_node):

    definition_node = dt_xml_node.find(UA_DEFINITION_TAG)

    if definition_node is not None:
        # Union are not managed for now => skip
        is_union = parse_boolean_value(definition_node.get('IsUnion', 'false'))


    if definition_node is None or is_union:
        return None

    definition = DataTypeDefinition()

    for field_node in definition_node:
        if field_node.tag != UA_FIELD_TAG:
            raise ParseError('Unexpected child tag %s instead of Field tag in Definition of DataType node %s' % (field_node.tag, dt_xml_node.attrib['NodeId']))
        name = field_node.get('Name', None)
        if name is None:
             raise ParseError('Name attribute missing in Field tag of Definition of DataType node %s' % (dt_xml_node.attrib['NodeId']))

        value = field_node.get('Value', None)
        if definition.is_structure is None:
            if value is not None:
                is_struct = False
            else:
                is_struct = True
        else:
            is_struct = definition.is_structure

        if is_struct and value is not None:
            raise ParseError('Value attribute unexpected in structure definition of DataType node %s.'
                             ' If it is an enum definition the Value attribute is expected in first field.' % (dt_xml_node.attrib['NodeId']))

        datatype = field_node.get('DataType', None)
        valuerank = field_node.get('ValueRank', None)
        arraydimensions = field_node.get('ArrayDimensions', None)
        maxstringlength = field_node.get('MaxStringLength', None)
        isoptional = field_node.get('IsOptional', None)
        allowsubtypes = field_node.get('AllowSubTypes', None)
        struct_fields = [datatype, valuerank, arraydimensions, maxstringlength, isoptional, allowsubtypes]

        if not is_struct and True in [attr is not None for attr in struct_fields]:
            raise ParseError('Structure field attributes "%s" staticmethod unexpected in enum definition of DataType node %s.' % (struct_fields, dt_xml_node.attrib['NodeId']))

        if isoptional is not None:
            # Optional fields are not managed for now => skip
            return None

        if is_struct:
            field = StructureField(name, aliases.get(datatype, datatype), valuerank, arraydimensions, maxstringlength, allowsubtypes)
        else:
            field = EnumField(name, value)
        definition.add_field(field)

    if len(definition.fields) == 0:
        definition = None

    return definition

def parse_uanode(no_dt_definition, xml_node, source, aliases):
    parse_element(source, xml_node.tag)
    nodeid = NodeId.parse(xml_node.attrib['NodeId'])
    browse_name = QName.parse(xml_node.attrib['BrowseName'])
    description = localized_text_of_child(xml_node, UA_DESCRIPTION_TAG)
    display_name = localized_text_of_child(xml_node, UA_DISPLAY_NAME_TAG)
    references = collect_node_references(xml_node, aliases)
    role_permissions = parse_role_permissions(xml_node)

    node = Node(
        xml_node.tag,
        nodeid,
        browse_name,
        description,
        display_name,
        references,
        role_permissions
    )

    if (xml_node.tag == UA_VARIABLE_TAG) or (xml_node.tag == UA_VARIABLE_TYPE_TAG):
        node.value = collect_variable_value(xml_node)

        if 'DataType' in xml_node.attrib:
            node.data_type = parse_datatype_attribute(xml_node.attrib['DataType'], aliases)
        else:
            # TODO: DataType = "i=24" (to be done in SOPC_AddressSpace_Node_Initialize too)
            node.data_type = NodeId(0,0,0)

        node.value_rank = parse_value_rank_attribute(xml_node)

        accesslevel = xml_node.get('AccessLevel', None)

        if accesslevel is not None:
            try:
                node.accesslevel = int(accesslevel)
            except ValueError:
                raise ParseError('Non integer AccessLevel for node %s' % xml_node['NodeId'])

    if xml_node.tag == UA_METHOD_TAG:
        node.executable = (parse_boolean_value(xml_node.get('Executable', 'true')))

    if xml_node.tag == UA_DATA_TYPE_TAG and not no_dt_definition:
        node.definition = collect_dt_definition(aliases, node, xml_node)

    if xml_node.tag == UA_OBJECT_TAG or xml_node.tag == UA_VIEW_TAG:
        event_notifier = xml_node.get('EventNotifier', None)
        if event_notifier is not None:
            try:
                node.event_notifier = int(event_notifier)
            except ValueError:
                raise ParseError('Non integer EventNotifier for node %s' % xml_node['NodeId'])

    if (xml_node.tag == UA_VARIABLE_TYPE_TAG or xml_node.tag == UA_OBJECT_TYPE_TAG or
        xml_node.tag == UA_REFERENCE_TYPE_TAG or xml_node.tag == UA_DATA_TYPE_TAG):
        node.is_abstract = (parse_boolean_value(xml_node.get('IsAbstract', 'false')))

    return node


def generate_reference(ref):
    return '''                {
                    &OpcUa_ReferenceNode_EncodeableType,
                    %s,
                    %s,
                    {%s, %s, 0},
                }''' % (
        generate_nodeid(ref.ty),
        'false' if ref.is_forward else 'true',
        generate_nodeid(ref.target),
        generate_string(None)
    )

def generate_role_permission(role_permission):
    return '''                {
                    &OpcUa_RolePermissionType_EncodeableType,
                    %s,
                    %d,
                }''' % (
        generate_nodeid(role_permission.roleid),
        int(role_permission.permissions)
    )

def generate_item(is_const_addspace, ua_node, ty, variant_field, value_status='OpcUa_UncertainInitialValue', **kwargs):
    extra = ''

    for k, v in kwargs.items():
        extra += ('\n            .%s = %s,' % (k, v))

    references = list(map(generate_reference, ua_node.references))
    references_qualifier = "const " if is_const_addspace else ""
    references_str = ('(' + references_qualifier + 'OpcUa_ReferenceNode[]) {\n' +
                      ',\n'.join(references) +
                      '\n            }') if references else 'NULL'

    role_permissions = list(map(generate_role_permission, ua_node.role_permissions))
    role_permissions_str = ('(' + references_qualifier + 'OpcUa_RolePermissionType[]) {\n' +
                      ',\n'.join(role_permissions) +
                      '\n            }') if role_permissions else 'NULL'

    begin_decl = '''    {
        OpcUa_NodeClass_%s,
        %s,
        {0, 0},
        {.%s={
            .encodeableType = &OpcUa_%sNode_EncodeableType,
            .NodeId = %s,
            .NodeClass = OpcUa_NodeClass_%s,
            .BrowseName = %s,
            .DisplayName = %s,
            .Description = %s,
            .NoOfReferences = %d,
            .References = %s,''' % (
        ty,
        value_status,
        variant_field,
        ty,
        generate_nodeid(ua_node.nodeid),
        ty,
        generate_qname(ua_node.browse_name),
        generate_localized_text(ua_node.display_name),
        generate_localized_text(ua_node.description),
        len(ua_node.references),
        references_str
        )

    role_permissions_final = ""
    if 'NULL' != role_permissions_str:
        role_permissions_final = '''
            .NoOfRolePermissions = %d,
            .RolePermissions = %s,''' % (
            len(ua_node.role_permissions),
            role_permissions_str
        )

    end_decl = '''%s
        }}\n    },\n''' % (
        extra)

    return begin_decl + role_permissions_final + end_decl

def number_coalesce(n, default):
    return n if n is not None else default

def generate_item_object(is_const_addspace, nodes, ua_node):
    return generate_item(is_const_addspace, ua_node, 'Object', 'object',
                         EventNotifier=str(number_coalesce(ua_node.event_notifier, 0)))

def default_variable_status(ua_node):
    value_status = '0x00' # Good status
    if ua_node.value is None and ua_node.nodeid.ns not in (None,0):
        # Keep OPC UA default namespace nodes with a Good status, necessary to pass UACTT
        # othewise keep Good status only if a value is defined
        value_status='OpcUa_UncertainInitialValue'
    return value_status

def generate_item_variable(is_const_addspace, nodes, ua_node):
    value_status = default_variable_status(ua_node)
    return generate_item(is_const_addspace, ua_node, 'Variable', 'variable', value_status,
                         Value=generate_value_variant(ua_node.value),
                         DataType=generate_nodeid(ua_node.data_type),
                         ValueRank="(%d)" % ua_node.value_rank,
                         AccessLevel=str(number_coalesce(ua_node.accesslevel, 1)))


def generate_item_variable_type(is_const_addspace, nodes, ua_node):
    value_status = default_variable_status(ua_node)
    return generate_item(is_const_addspace, ua_node, 'VariableType', 'variable_type', value_status,
                         Value=generate_value_variant(ua_node.value),
                         DataType=generate_nodeid(ua_node.data_type),
                         ValueRank="(%d)" % ua_node.value_rank,
                         IsAbstract= 'true' if ua_node.is_abstract else 'false')

def generate_item_object_type(is_const_addspace, nodes, ua_node):
    return generate_item(is_const_addspace, ua_node, 'ObjectType', 'object_type',
                         IsAbstract= 'true' if ua_node.is_abstract else 'false')


def generate_item_reference_type(is_const_addspace, nodes, ua_node):
    return generate_item(is_const_addspace, ua_node, 'ReferenceType', 'reference_type',
                         IsAbstract= 'true' if ua_node.is_abstract else 'false')

def generate_item_data_type(is_const_addspace, nodes, ua_node):
    dt_definition = None
    if ua_node.definition is not None:
        ua_node.definition.finalize(ua_node, nodes)
        is_not_pointer = True
        dt_definition = generate_extension_object(ua_node.definition, generate_definition_ext_obj, is_array=is_not_pointer)
    if dt_definition is None:
        return generate_item(is_const_addspace, ua_node, 'DataType', 'data_type',
                             IsAbstract= 'true' if ua_node.is_abstract else 'false')
    else:
        return generate_item(is_const_addspace, ua_node, 'DataType', 'data_type',
                             IsAbstract= 'true' if ua_node.is_abstract else 'false',
                             DataTypeDefinition = dt_definition)


def generate_item_method(is_const_addspace, nodes, ua_node):
    return generate_item(is_const_addspace, ua_node, 'Method', 'method',
                         Executable= 'true' if ua_node.executable else 'false')


GEN_ITEM_FUNCS = {
    UA_OBJECT_TAG: generate_item_object,
    UA_VARIABLE_TAG: generate_item_variable,
    UA_VARIABLE_TYPE_TAG: generate_item_variable_type,
    UA_OBJECT_TYPE_TAG: generate_item_object_type,
    UA_REFERENCE_TYPE_TAG: generate_item_reference_type,
    UA_DATA_TYPE_TAG: generate_item_data_type,
    UA_METHOD_TAG: generate_item_method,
}

def no_metadata_write_in_accesslevel(ua_node, access_level):
    if (ACCESSLEVEL_MASK_TIMESTAMPWRITE | ACCESSLEVEL_MASK_STATUSWRITE) & access_level == 0:
        # no metadata write allowed => OK
        return access_level
    else:
        # metadata write allowed => warning + forbids it since metadata is read only
        new_access_level = ~(ACCESSLEVEL_MASK_TIMESTAMPWRITE | ACCESSLEVEL_MASK_STATUSWRITE) & access_level
        print('WARNING: AccessLevel %d of NodeId %s changed to %d'
              % (access_level, str(ua_node.nodeid), new_access_level))
        return new_access_level

DEBUG_CURRENT_LINE = None
# Returns an array of Node objects
def generate_address_space(is_const_addspace, no_dt_definition, source, out):
    global DEBUG_CURRENT_LINE
    aliases = {}
    node_id_to_node = {}
    variables = list()
    n_items = 0

    out.write(c_header)

    out.write("const bool sopc_embedded_is_const_addspace = %s;\n\n" % ("true" if is_const_addspace else "false"))
    if is_const_addspace:
        out.write('SOPC_GCC_DIAGNOSTIC_IGNORE_DISCARD_QUALIFIER\n')
        out.write('SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST\n')
        out.write('const SOPC_AddressSpace_Node SOPC_Embedded_AddressSpace_Nodes[] = {\n')
    else:
        out.write('SOPC_AddressSpace_Node SOPC_Embedded_AddressSpace_Nodes[] = {\n')

    expect_element(source, UA_NODESET_TAG).clear()

    end = False
    while not end:
        try:
            ev, n = next(source)
        except StopIteration:
            raise ParseError('Unexpected end of document while parsing UANodeSet')
        try:
            DEBUG_CURRENT_LINE = "%s : %s"%(str(n.tag), str(n.attrib))
        except:
            DEBUG_CURRENT_LINE = str(n)
        if ev == 'end' and n.tag == UA_NODESET_TAG:
            end = True
            continue

        check_element(n)

        gen_func = GEN_ITEM_FUNCS.get(n.tag, None)
        if gen_func:
            ua_node = parse_uanode(no_dt_definition, n, source, aliases)
            node_id_to_node[str(ua_node.nodeid)] = ua_node
        elif n.tag == UA_ALIASES_TAG:
            parse_element(source, n.tag)
            aliases.update(collect_aliases(n))
        else:
            skip_element(source, n.tag)

        n.clear()

    for ua_node in node_id_to_node.values():
        gen_func = GEN_ITEM_FUNCS.get(ua_node.tag, None)
        if is_const_addspace and ua_node.tag == UA_VARIABLE_TAG:
            # Add specific case for Variables to store the non constant part into array of Variants
            access_level= no_metadata_write_in_accesslevel(ua_node, number_coalesce(ua_node.accesslevel, 1))
            out.write(generate_item(is_const_addspace, ua_node, 'Variable', 'variable', '0x00',
                                    # Set Variant index as Variant value in constant item
                                    Value='SOPC_VARIANT_UINT32(%d)' % len(variables),
                                    DataType=generate_nodeid(ua_node.data_type),
                                    ValueRank="(%d)" % ua_node.value_rank,
                                    AccessLevel=str(access_level)))
            # Set Variant value in variable Variants array
            variables.append(generate_value_variant(ua_node.value))
            n_items += 1
        elif gen_func:
            out.write(gen_func(is_const_addspace, node_id_to_node, ua_node))
            n_items += 1
        else:
            assert(False)

    if end:
            out.write('};\n')
            out.write('const uint32_t SOPC_Embedded_AddressSpace_nNodes = %d;\n' % n_items)
            out.write('\n');
            if len(variables) > 0:
                out.write('// Index is provided by the corresponding Variable UInt32 Variant in SOPC_Embedded_AddressSpace_Nodes\n');
                out.write('SOPC_Variant SOPC_Embedded_VariableVariant[%d] = {\n' % len(variables))
                for i in range(0, len(variables)):
                    out.write(variables[i])
                    out.write(',\n')
                out.write('};\n')
            else:
                out.write('// Unused variable but it is still necessary to link the loader of embedded address space\n')
                out.write('SOPC_Variant* SOPC_Embedded_VariableVariant = NULL;\n')

            if is_const_addspace :
                out.write('SOPC_GCC_DIAGNOSTIC_RESTORE\n')
                out.write('SOPC_GCC_DIAGNOSTIC_RESTORE\n')

            out.write('const uint32_t SOPC_Embedded_VariableVariant_nb = %d;\n' % len(variables))

def main():
    global DEBUG_CURRENT_LINE
    argparser = argparse.ArgumentParser(description='Generate the S2OPC address space from an OPC UA NodeSet')
    argparser.add_argument('xml_file', metavar='XML_FILE',
                           help='Path to the address space XML file')
    argparser.add_argument('c_file', metavar='C_FILE',
                           help='Path to the generated C file')
    argparser.add_argument('--const_addspace', action='store_true', default=False,
                           help='Flag to set the generated address space as a const (default: False)')
    argparser.add_argument('--no_dt_definition', action='store_true', default=False,
                           help='Flag to avoid parsing the DataType definition (default: False)')

    args = argparser.parse_args()

    print('Generating C address space (const=%s)...' % str(args.const_addspace))
    with open(args.xml_file, 'rb') as xml_fd, open(args.c_file, 'w', encoding='utf8') as out_fd:
        try:
            generate_address_space(args.const_addspace, args.no_dt_definition, iterparse(xml_fd, events=('start', 'end')), out_fd)
        except ParseError as e:
            sys.stderr.write('Woops, an error occurred: %s \n' % str(e))
            if DEBUG_CURRENT_LINE :
                sys.stderr.write('Context: %s \n' % DEBUG_CURRENT_LINE)
            sys.exit(1)

    print('Done.')


if __name__ == '__main__':
    main()
