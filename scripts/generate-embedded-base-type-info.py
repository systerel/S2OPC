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
import sys
import uuid
from xml.etree.ElementTree import iterparse, Element

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
UA_VALUE_TAG = UA_NODESET_NS + 'Value'

NODE_TAG_TO_NODE_CLASS = {
    UA_OBJECT_TAG : "OpcUa_NodeClass_Object",
    UA_VARIABLE_TAG : "OpcUa_NodeClass_Variable",
    UA_METHOD_TAG : "OpcUa_NodeClass_Method",
    UA_OBJECT_TYPE_TAG : "OpcUa_NodeClass_ObjectType",
    UA_VARIABLE_TYPE_TAG : "OpcUa_NodeClass_VariableType",
    UA_REFERENCE_TYPE_TAG : "OpcUa_NodeClass_ReferenceType",
    UA_DATA_TYPE_TAG : "OpcUa_NodeClass_DataType",
    UA_VIEW_TAG : "OpcUa_NodeClass_View"
}

ID_TYPE_NUMERIC = 0
ID_TYPE_STRING = 1
ID_TYPE_GUID = 2
ID_TYPE_BYTESTRING = 3

UA_TYPES_NS = '{http://opcfoundation.org/UA/2008/02/Types.xsd}'

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

#ifndef _sopc_embedded_nodeset2_h
#define _sopc_embedded_nodeset2_h

#include <stdbool.h>

#include "sopc_builtintypes.h"

'''.lstrip()


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

        return NodeId(ns, ty, data)

class Reference(object):
    __slots__ = 'ty', 'target', 'is_forward'

    def __init__(self, ty, target, is_forward):
        self.ty = ty
        self.target = target
        self.is_forward = is_forward


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


def get_node_reference_HasSubtype_Backward(node, aliases):
    ref = None

    refs_node = node.find(UA_REFERENCES_TAG)

    if refs_node is None:
        return ref

    for n in refs_node.findall(UA_REFERENCE_TAG):
        try:
            ref_type = n.attrib['ReferenceType']
        except KeyError:
            raise ParseError('Missing ReferenceType on Reference element for node ' + node.get('NodeId'))

        # In case there is an alias, resolve it
        ref_type = NodeId.parse(aliases.get(ref_type, ref_type))

        is_forward = (parse_boolean_value(n.get('IsForward', 'true')))

        # Note: ns=0;i=45 <=> HasSubtype reference
        if((ref_type.ns is None or ref_type.ns == 0) and ref_type.data == 45 and is_forward is False):
            if ref is None:
                ref = Reference(ref_type, NodeId.parse(n.text), is_forward)
            else:
                assert(False)

    return ref


def parse_boolean_value(text):
    return text.strip() == 'true'

def boolean_to_string(val):
    return 'true' if val else 'false'




def generate_string(data):
    assert data is None or isinstance(data, str), "Invalid string data: %r" % data

    if data is not None:
        return '{%d, 1, (SOPC_Byte*) "%s"}' % (len(data), data.replace('"', '\\"'))

    return '{0, 0, NULL}'


def generate_guid(data):
    assert isinstance(data, uuid.UUID)

    members = list(data.fields[0:5])

    for i in range(5, -1, -1):
        members.append((data.fields[5] >> (8*i)) & 0xFF)

    return '{0x%02x, 0x%02x, 0x%02x, {0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x}}' % tuple(members)


def generate_byte_string(data):
    return generate_string(data)


def generate_nodeid(nodeid):
    id_type = C_IDENTIFIER_TYPES[nodeid.ty]
    data_struct_field = '.Data.' + id_type[len('SOPC_IdentifierType_'):]

    if nodeid.ty == ID_TYPE_NUMERIC:
        data_struct_val = str(nodeid.data)
    elif nodeid.ty == ID_TYPE_STRING:
        data_struct_val = generate_string(nodeid.data)
    elif nodeid.ty == ID_TYPE_GUID:
        data_struct_val = generate_guid(nodeid.data)
    elif nodeid.ty == ID_TYPE_BYTESTRING:
        data_struct_val = generate_byte_string(nodeid.data)
    else:
        raise CodeGenerationError('Unknown NodeId data type')

    return '{%s, %d, %s = %s}' % (id_type, nodeid.ns or 0, data_struct_field, data_struct_val)

def parse_uanode_supertype(xml_node, source, aliases):
    parse_element(source, xml_node.tag)
    nodeid = NodeId.parse(xml_node.attrib['NodeId'])
    return nodeid, get_node_reference_HasSubtype_Backward(xml_node, aliases)

TYPE_TAGS = [UA_VARIABLE_TYPE_TAG, UA_OBJECT_TYPE_TAG, UA_REFERENCE_TYPE_TAG, UA_DATA_TYPE_TAG]

# Returns an array of Node objects
def generate_type_node_info(source, out, max_nodeId):
    aliases = {}
    subtypes_backward = dict()
    type_node_nodeclass = dict()

    out.write(c_header)
    out.write("typedef struct SOPC_AddressSpaceTypeInfo\n")
    out.write("{\n")
    out.write("    "+"OpcUa_NodeClass nodeClass;\n")
    out.write("    "+"bool hasSubtype;\n")
    out.write("    "+"SOPC_NodeId subtypeNodeId;\n")
    out.write("} SOPC_AddressSpaceTypeInfo;\n")
    out.write("// Generated from NodeSet2: integer nodeId --> SOPC_NodeId\n")
    out.write("#define SOPC_MAX_TYPE_INFO_NODE_ID "+str(max_nodeId)+"\n\n")
    out.write('const SOPC_AddressSpaceTypeInfo SOPC_Embedded_HasSubTypeBackward[SOPC_MAX_TYPE_INFO_NODE_ID + 1] = {\n')

    expect_element(source, UA_NODESET_TAG).clear()

    while True:
        try:
            ev, n = next(source)
        except StopIteration:
            raise ParseError('Unexpected end of document while parsing UANodeSet')

        if ev == 'end' and n.tag == UA_NODESET_TAG:
            index = 0
            # Ensures no reference defined from 'NULL' NodeId
            if 0 in subtypes_backward:
                del subtypes_backward[0]
            # Set NodeClass unspecified for 'NULL' NodeId
            type_node_nodeclass[0] = "OpcUa_NodeClass_Unspecified"

            while index <= max_nodeId:
                node_class = type_node_nodeclass.get(index, "OpcUa_NodeClass_Unspecified")
                if(index in subtypes_backward):
                    out.write("    {"+node_class+", true, "+generate_nodeid(subtypes_backward[index])+"},   // "+str(index)+"\n")
                else:
                    out.write("    {"+node_class+", false, "+generate_nodeid(NodeId(0, ID_TYPE_NUMERIC, 0))+"},   // "+str(index)+"\n")
                index += 1
            out.write('};\n\n#endif\n')
            return

        check_element(n)

        if n.tag in TYPE_TAGS:
            nodeid, ref = parse_uanode_supertype(n, source, aliases)
            if ref is not None:
                if not nodeid.ty == ID_TYPE_NUMERIC:
                    print("Warning: unexpected non-numeric source nodeId: "+str(nodeid))
                elif not ref.target.ty == ID_TYPE_NUMERIC:
                    print("Warning: unexpected non-numeric target nodeId: "+str(ref.target))
                elif nodeid.data in subtypes_backward:
                    print("Warning: already present source nodeId: "+str(nodeid))
                else:
                    subtypes_backward[nodeid.data] = ref.target
                    #child = n.find(UA_DISPLAY_NAME_TAG)
                    #print(str(n.tag[len(UA_NODESET_NS):])+": "+child.text+": "+str(nodeid)+" references")
                    #print(" - Reference: "+str(ref.ty)+", "+str(ref.target)+", "+str(ref.is_forward))

            if nodeid.ty == ID_TYPE_NUMERIC:
                type_node_nodeclass[nodeid.data] = NODE_TAG_TO_NODE_CLASS.get(n.tag)

        elif n.tag == UA_ALIASES_TAG:
            parse_element(source, n.tag)
            aliases.update(collect_aliases(n))
        else:
            skip_element(source, n.tag)

        n.clear()


def main():
    argparser = argparse.ArgumentParser(description='Extract HasSubtype hierarchy from address space')
    argparser.add_argument('xml_file', metavar='XML_FILE',
                           help='Path to the address space XML file')
    argparser.add_argument('h_file', metavar='H_FILE',
                           help='Path to the generated HEADER file')
    argparser.add_argument('--max_nodeId', metavar='MAX_NODEID', default=1000,
                           help='Maximum integer NodeId in NS 0 present in static array')
    args = argparser.parse_args()

    print('Generating C model...')
    with open(args.xml_file, 'rb') as xml_fd, open(args.h_file, 'w', encoding='utf8') as out_fd:
        try:
            generate_type_node_info(iterparse(xml_fd, events=('start', 'end')), out_fd, args.max_nodeId)
        except ParseError as e:
            sys.stderr.write('Woops, an error occurred: %s\n' % str(e))
            sys.exit(1)

    print('Done.')


if __name__ == '__main__':
    main()
