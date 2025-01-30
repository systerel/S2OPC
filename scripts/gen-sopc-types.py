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
import os

H_FILE_PATH = 'src/Common/opcua_types/sopc_types.h'
H_ENUM_FILE_PATH = 'src/Common/opcua_types/sopc_enum_types.h'
C_FILE_PATH = 'src/Common/opcua_types/sopc_types.c'

# Namespaces are very hard to handle in XPath and the ET API does not help much
# {*} notations for namespace in XPath searches only appear in Py3.8
# Moreover, the original namespaces are not parsed by default
# (inspired from http://effbot.org/zone/element-namespaces.htm)
def parse_xmlns(source):
    # Note: the parser does not iterate over real prefixes of the ns...
    ns_map = {}
    for event, elem in ET.iterparse(source, ('start-ns',)):
        if event == "start-ns":
            prefix, uri = elem
            if prefix not in ns_map:
                ns_map[prefix] = uri
    
    return ns_map

# When non-NS0 types are generated, use the type prefix to normalize type files names
# Note: it is necessary to be able to reference this NS types into others if needed (see --imported_ns_prefixes)
def configure_custom_files_path_from_prefix(ns):
    lower_case_prefix = ns.types_prefix.lower()
    global H_FILE_PATH
    global C_FILE_PATH
    global H_ENUM_FILE_PATH
    H_FILE_PATH = ns.files_path + lower_case_prefix + '_types.h'
    C_FILE_PATH = ns.files_path + lower_case_prefix + '_types.c'
    H_ENUM_FILE_PATH = ns.files_path + lower_case_prefix + '_enum_types.h'

def main():
    """Main program"""
    ns = parse_args()
    if ns.types_prefix is None and ns.ns_index != 0:
        raise ValueError("--types_prefix shall be set when namespace is not NS0")
    elif ns.types_prefix is not None:
        configure_custom_files_path_from_prefix(ns)
    schema = BinarySchema(ns.bsd, ns.types_prefix, ns.ns_index, ns.imported_ns_prefixes)
    gen_header_file(H_FILE_PATH, H_ENUM_FILE_PATH, schema)
    ns_URI = None
    if ns.types_prefix is not None and ns.ns_index == 0:
        print("Warning: using NS URI field for EncodeableType instances since no NS index is defined for non-NS0: %s" % schema.targetNS)
    gen_implem_file(ns_URI, ns.ns_index, H_FILE_PATH, C_FILE_PATH, schema)

def parse_args():
    """
    Parse the command line arguments and return a dictionary of arguments.
    """
    parser = argparse.ArgumentParser(
        description='Generate the NS0 sopc_types.h and sopc_types.c files (or custom types).')
    parser.add_argument('--types_prefix',
                        metavar='prefix type name',
                        required=False,
                        help='Prefix used to generate global variables (only for non-NS0 namespace types) and types files with prefix in lower case: <prefix>_types.h/c and <prefix>_enum_types.c')
    parser.add_argument('--ns_index',
                        metavar='OPC UA namespace index',
                        type=int,
                        default=0,
                        required=False,
                        help='Local namespace index for the OPC UA types (if index unknown, set namespace URI). Default is NS0.')
    parser.add_argument('bsd',
                        metavar='file.bsd',
                        help='OPC UA type description')
    parser.add_argument('--files_path',
                        default='',
                        metavar='path',
                        required=False,
                        help='(optional) directory path for non-NS0 types files generation')
    parser.add_argument('--imported_ns_prefixes', default=[], nargs='+', dest='imported_ns_prefixes',
                        help='''
                        When other namespace types are imported 
                        (except for "http://opcfoundation.org/UA" and "http://opcfoundation.org/BinarySchema/")
                        the value used for --types_prefix used to generate those namespace types shall be provided 
                        to be able to reference those imported types.
                        ''')
    return parser.parse_args()

def gen_header_file(h_types_path, h_enum_types_path, schema):
    """
    Generates the sopc_types.h file from the given schema.
    """
    with open(h_types_path, "w") as out, open(h_enum_types_path, "w") as out_enum:
        if schema.is_ns0_types():
            out.write(H_FILE_START)
            out_enum.write(H_ENUM_FILE_START)
        else:
            out.write(H_FILE_CUSTOM_START.format(enum_h_file=os.path.basename(h_enum_types_path), prefix=schema.types_prefix))
            # add includes for possible use of imported types in the current NS types
            for imported_include in schema.imported_includes:
                out.write(H_FILE_CUSTOM_INCLUDE.format(included_h_file=imported_include))
            out_enum.write(H_ENUM_FILE_CUSTOM_START.format(prefix=schema.types_prefix))
        schema.gen_header_types(out, out_enum)
        if schema.is_ns0_types():
            # Generates only for NS0 types
            out.write(H_FILE_ENUM_FUN_DECLS)
        schema.gen_type_index_decl(out)
        if schema.is_ns0_types():
            out.write(H_FILE_USER_TOKEN_POLICIES)
        out.write(H_FILE_END.format(prefix=schema.types_prefix))
        out_enum.write(H_ENUM_FILE_END)

def gen_implem_file(ns_uri, ns_index, h_types_path, c_types_path, schema):
    """
    Generates the sopc_types.c file from the given schema.
    """
    if ns_uri is None:
        ns_uri = 'NULL'
    else:
        # index will be ignored by code
        ns_index = 0

    with open(c_types_path, "w") as out:
        if schema.is_ns0_types():
            out.write(C_FILE_START)
        else:
            out.write(C_FILE_CUSTOM_START.format(prefix=schema.types_prefix, types_h_file=os.path.basename(h_types_path)))
        gen_implem_types(ns_uri, ns_index, out, schema)
        out.write(C_FILE_KNOWN_ENC_TYPES.format(prefix=schema.types_prefix))
        if schema.is_ns0_types():
            out.write(C_FILE_ENUM_FUN_DEFS)
        out.write(C_FILE_END)

def gen_implem_types(ns_uri, ns_index, out, schema):
    schema.gen_encodeable_type_table(out)
    schema.gen_encodeable_type_descs(ns_uri, ns_index, out)
    if schema.is_ns0_types():
        # Generates only for NS0 types
        schema.gen_user_token_policies_constants(out)

def get_enumerated_opcua_unsigned_type_and_id(enum_node):
    unsignedIntLength = enum_node.attrib['LengthInBits']
    assert unsignedIntLength is not None
    unsignedIntLength = int(unsignedIntLength)
    if 8 == unsignedIntLength:
        return ('SOPC_Byte', 'SOPC_Byte_Id')
    elif 16 == unsignedIntLength:
        return ('uint16_t', 'SOPC_UInt16_Id')
    elif 32 == unsignedIntLength:
        return ('uint32_t', 'SOPC_UInt32_Id')
    elif 64 == unsignedIntLength:
        return ('uint64_t', 'SOPC_UInt64_Id')
    assert False

class BinarySchema:
    """Represents a Binary Schema Description (BSD) file"""

    OPC_NS = {
      'opc': "http://opcfoundation.org/BinarySchema/",
      'xsi': "http://www.w3.org/2001/XMLSchema-instance",
      'ua': "http://opcfoundation.org/UA/",
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
            'opc:CharArray': 'SOPC_String',
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

    BUILTIN_CONVERT_TYPE_BARENAME = {
            'CharArray' : 'String'
    }
    
    # Compute self.xmlns_indexes which provide computed NS index for each imported XML NS, and prefixes of imported NS
    # Note: current types NS index <N> is considered as maximum NS index, X dependencies are computed to be <N-X>, <N-X+1>, etc. in the order of Import tags except for NS0.
    def check_and_compute_ns_indexes(self, types_ns_index, imported_ns_prefixes):
        # Check default NS are the expected ones
        for ns in self.OPC_NS:
            if self.xmlns[ns] != self.OPC_NS[ns]:
                raise Exception("Unexpected value for XML NS %s: found %s expecting %s" % (ns, self.xmlns[ns], self.OPC_NS[ns]))

        # If target NS is NS0: no NS import to deal with, ignore specific treatment
        if self.targetNS != self.OPC_NS['ua']:
            # Compute reverse xmlns: URI => XML NS alias
            xmlns_rev = dict()
            for ns in self.xmlns:
                nsURI = self.xmlns[ns]
                if nsURI in xmlns_rev and types_ns_index != 0:
                    raise Exception("Several XML NS aliases for same URI %s: %s and %s" % (nsURI, self.xmlns[ns], xmlns_rev[nsURI]))
                xmlns_rev[nsURI] = ns
            # Compute the NS index for NS dependencies if existing: considering incremental values with {types_ns_index} as maximum value
            nsImportNodes = self.tree.findall('opc:Import', self.xmlns)
            # Compute NS URI imported eliminating the well-known NS0 and ~built-in-NS
            nsImported = [nsImport.get('Namespace') for nsImport in nsImportNodes
                          if xmlns_rev[nsImport.get('Namespace')] not in self.OPC_NS]
            nbNsImported = len(nsImported)
            if nbNsImported != len(imported_ns_prefixes):
                raise Exception("Incoherent number of imported namespaces %d and provided NS types prefixes %d" % (nbNsImported, len(imported_ns_prefixes))) 
            if types_ns_index - nbNsImported <= 0:
                raise Exception("Number of imported NS %d greater than current types NS %d" % (nbNsImported, types_ns_index)) 
            # Compute starting NS index to used based on max types_ns_index
            nsImportedIndex = types_ns_index - nbNsImported
            nsImportedCounter = 0
            # Populate self.xmlns_indexes, self.xmln_types_prefixes and self.imported_ns_prefixes
            for nsURI in nsImported:
                ns = xmlns_rev[nsURI]
                self.xmlns_indexes[ns] = nsImportedIndex
                self.xmlns_types_prefixes[ns] = imported_ns_prefixes[nsImportedCounter] + '_'
                self.imported_includes.append(imported_ns_prefixes[nsImportedCounter].lower() + '_types.h')
                nsImportedIndex = nsImportedIndex + 1
                nsImportedCounter = nsImportedCounter + 1
            # NS 0 has no prefix in type name
            self.xmlns_indexes['ua'] = 0
            self.xmlns_types_prefixes['ua'] = ''
            self.imported_includes.insert(0, 'sopc_types.h')

    def __init__(self, filename, types_prefix, types_ns_index, imported_ns_prefixes):
        self.tree = ET.parse(filename)
        root = self.tree.getroot()
        if root.tag != self.ROOT_TAG:
            fatal("Invalid root element in bsd file: %s" % root.tag)
        self.targetNS = root.get('TargetNamespace') # the target NS of types
        self.xmlns = parse_xmlns(filename)
        self.xmlns_indexes = dict() # NS indexes for the imported OPC UA NS
        self.xmlns_types_prefixes = dict() # the types prefixes for the imported OPC UA NS when generated
        self.imported_includes = [] # the includes necessary for the imported OPC UA NS types
        self.check_and_compute_ns_indexes(types_ns_index, imported_ns_prefixes)
        self.bsd2c = OrderedDict(self.BUILTIN_TYPES)
        self.enums = dict()
        self.fields = dict()
        self.known_writer = KnownEncodeableTypeWriter()
        if types_prefix is None:
            self.types_prefix = ''
        else:
            self.types_prefix = types_prefix + '_'

    def is_ns0_types(self):
        return self.types_prefix == ''

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
        barename = typename.split(':')[1]
        if typename in self.bsd2c:
            return
        node = self._get_node(barename)
        barename = self.types_prefix + barename

        if node.tag == self.STRUC_TAG:
            if node.get('BaseType') == 'ua:Union':
                fatal("Unsupported BaseType %s for type %s" % (node.get('BaseType'), node.get('Name')))
            ctype = self._gen_struct_decl(out, out_enum, node, barename)
            self.known_writer.encodeable_types.append((typename, barename))
        elif node.tag == self.ENUM_TAG:
            utype, utype_id = get_enumerated_opcua_unsigned_type_and_id(node)
            self.enums[barename] = utype
            self.enums[typename] = utype_id
            ctype = self._gen_enum_decl(out_enum, node, barename)
        else:
            fatal("Unknown node kind: %s" % node.tag)
        self.bsd2c[typename] = ctype

    def gen_header_types(self, out, out_enum):
        types = (self.tree.findall('opc:EnumeratedType', self.xmlns) +
                 self.tree.findall('opc:StructuredType', self.xmlns))
        for typ in types:
            name = typ.get('Name')
            if name in PREDEFINED_TYPES:
                continue
            elif 0 == len(list(typ)):
                # Type is empty (no fields)
                continue
            elif (name.endswith('Request') and
                  self.tree.findall('./*[@Name="%sResponse"]' % name[:-len('Request')])):
                self.gen_header_pair(out, out_enum, name[:-len('Request')])
            elif name.endswith('Response'):
                # Already treated with Request
                continue
            else:
                self.gen_header_type(out, out_enum, name)

    def gen_type_index_decl(self, out):
        """
        Generates an enumerated type for indexing in sopc_KnownEncodeableTypes

        We use an enumerated type without explicit values, so that the indexes
        are always compact, even when some type is excluded.  The purpose of
        these indexes is to access rapidly to the description of a type.
        """
        def writer(typename, barename):
            out.write(TYPE_INDEX_DECL_ENCODEABLE_TYPE.format(prefix=self.types_prefix, name=barename))

        out.write(TYPE_INDEX_DECL_START.format(prefix=self.types_prefix))
        self.known_writer.gen_types(out, writer)
        out.write(TYPE_INDEX_DECL_END.format(prefix=self.types_prefix))

    def gen_encodeable_type_descs(self, ns_uri, ns_index, out):
        """
        Generates the descriptors of all encodeable types.
        """
        def writer(typename, barename):
            self.gen_encodeable_type_desc(ns_uri, ns_index, out, typename, barename)

        out.write(ENCODEABLE_TYPES_DESC_START)
        self.known_writer.gen_types(out, writer)

    def gen_encodeable_type_desc(self, ns_uri, ns_index,  out, typename, barename):
        """
        Generates the field descriptors of an encodeable type.
        """
        ctype = self.bsd2c[typename]
        out.write(ENCODEABLE_TYPE_DESC_START.format(name=barename))
        fields = self.fields[barename]
        if fields:
            out.write(ENCODEABLE_TYPE_FIELD_DESC_START.format(name=barename))
        for field in fields:
            field_ns_index = 0
            is_built_in, type_index = self.get_type_index(field.type_name)
            # When the field type is in another NS, the NS index shall be referenced 
            # and type index in this other NS types array is referenced by type index enum name
            if not is_built_in and not field.is_same_ns:
                field_barename = field.type_name.split(':')[1]
                field_ns_index = self.xmlns_indexes[field.type_ns]
                type_index = 'SOPC_TypeInternalIndex_' + self.xmlns_types_prefixes[field.type_ns] + field_barename
            out.write(ENCODEABLE_TYPE_FIELD_DESC.format(
                name=barename,
                is_built_in=c_bool_value(is_built_in),
                is_array_length=c_bool_value(field.is_array_length),
                is_to_encode=c_bool_value(field.is_to_encode),
                is_same_ns=c_bool_value(is_built_in or field.is_same_ns),
                ns_index=field_ns_index,
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
            ns_URI=ns_uri, 
            ns_index=ns_index,  
            ctype=ctype,
            nof_fields=nof_fields,
            field_desc=field_desc,
            prefix=self.types_prefix))

    def get_type_index(self, typename):
        """
        Returns a pair (is_built_in, type_index) for a field type.
        """
        barename = typename.split(':')[1]
        is_built_in = typename in self.BUILTIN_TYPES
        if is_built_in:
            template = 'SOPC_{name}_Id'
            # check if conversion needed for built in types
            if barename in self.BUILTIN_CONVERT_TYPE_BARENAME:
                barename = self.BUILTIN_CONVERT_TYPE_BARENAME[barename]
        elif self.is_enum(self.normalize_typename(typename)):
            # Enumerated types are unsigned integer (TypeId computed in enums dict)
            is_built_in = True
            template = self.enums[self.normalize_typename(typename)]
        else:
            if self.types_prefix is not None:
                barename = self.types_prefix + barename
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
        out.write(TYPE_TABLE_START.format(prefix=self.types_prefix))
        self.known_writer.gen_types(out, writer)
        out.write(TYPE_TABLE_END)

    def untranslated_typenames(self):
        """
        Returns the set of all names of types present in the schema that have
        not been translated.
        """
        nodes = (self.tree.findall('opc:StructuredType', self.xmlns) +
                 self.tree.findall('opc:EnumeratedType', self.xmlns))
        names = (self.normalize_typename(node.get('Name')) for node in nodes)
        return set(filter(lambda n: n not in self.bsd2c, names))

    def _gen_struct_decl(self, out, out_enum, node, name):
        """
        Generates the declarations for a structured type.
        """
        children = node.findall('./opc:Field', self.xmlns)
        fields = [Field(self, child) for child in children]
        for field in fields:
            # Generate field type if not already done and if defined in target/current NS
            if field.is_same_ns:
                self.gen_header_type(out, out_enum, field.type_name)
        fields = [field for field in fields if field.name != 'RequestHeader']
        self._check_array_fields(name, fields)

        if self.is_ns0_types():
            out.write(STRUCT_DECL_START.format(export="S2OPC_COMMON_EXPORT ", name=name))
        else:
            out.write(STRUCT_DECL_START.format(export="", name=name))

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
        unsignedIntLength = node.attrib['LengthInBits']
        if unsignedIntLength is None:
            raise Exception('Mandatory attribute LengthInBits missing in EnumeratedType %s' % name)
        unsignedIntLength = int(unsignedIntLength)
        if 8 == unsignedIntLength:
            maxUint="UINT8_MAX"
        elif 16 == unsignedIntLength:
            maxUint="UINT16_MAX"
        elif 32 == unsignedIntLength:
            # ISO C restricts enumerator values to range of 'int'
            maxUint="INT32_MAX"
        elif 64 == unsignedIntLength:
            # ISO C restricts enumerator values to range of 'int'
            maxUint="INT32_MAX"
        else:
            raise Exception('Unexpected attribute LengthInBits value "%d" missing in EnumeratedType %s' % (unsignedIntLength, name))

        # If optionSet is active the type is not an enum but a mask
        optionSet = node.attrib.get('IsOptionSet', False)


        elem_decls = [
                ENUM_DECL_ELEM.format(name=name,
                                      elem=child.attrib['Name'],
                                      value=child.attrib['Value'])
                for child in node.findall('./opc:EnumeratedValue', self.xmlns)
        ]
        # append an element that will size the enum as an int32
        elem_decls.append(ENUM_DECL_ELEM.format(name=name, elem="SizeOf", value=maxUint))

        out.write(",\n".join(elem_decls) + '\n')

        out.write(ENUM_DECL_END.format(name=name))

        if optionSet:
            # If the type is not an enum, keep the built it type
            return '/* ::OpcUa_' + name + ' */ ' + self.enums[name]
        else:
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
        # Check if known type
        if typename in self.bsd2c:
            return self.bsd2c[typename]
        xmlNS, barename = typename.split(':')
        # Check if the type is from an imported NS types that can be referenced
        if xmlNS in self.xmlns_types_prefixes and self.xmlns[xmlNS] != self.targetNS:
            return 'OpcUa_' + self.xmlns_types_prefixes[xmlNS] + barename
        else:
            raise Exception("Unexpected unknown type referenced %s" % typename)

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
        self.type_ns =  self.type_name.split(':')[0]
        self.is_same_ns = schema.xmlns[self.type_ns] == schema.targetNS
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
        for typename, barename in self.encodeable_types:
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

H_FILE_CUSTOM_START = """
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

#ifndef SOPC_{prefix}Types_H_
#define SOPC_{prefix}Types_H_ 1

#include "sopc_buffer.h"
#include "sopc_builtintypes.h"
#include "sopc_encodeabletype.h"
#include "{enum_h_file}"

"""[1:]

H_FILE_CUSTOM_INCLUDE = """
#include "{included_h_file}"

"""[1:]

H_FILE_ENUM_FUN_DECLS = """
void SOPC_Initialize_EnumeratedType(int32_t* enumerationValue);

void SOPC_Clear_EnumeratedType(int32_t* enumerationValue);

SOPC_ReturnStatus SOPC_Read_EnumeratedType(SOPC_Buffer* buf, \
int32_t* enumerationValue, uint32_t nestedStructLevel);

SOPC_ReturnStatus SOPC_Write_EnumeratedType(SOPC_Buffer* buf, \
const int32_t* enumerationValue, uint32_t nestedStructLevel);
"""

H_FILE_USER_TOKEN_POLICIES="""
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

// UserTokenPolicyId for x509 token type with Basic256Sha256 SecurityPolicy example
#define SOPC_UserTokenPolicy_X509Basic256Sha256_ID "X509_Basic256Sha256"
/** Example x509 security policy supported and configured with Basic256Sha256 security policy.
 */
S2OPC_COMMON_EXPORT extern const OpcUa_UserTokenPolicy SOPC_UserTokenPolicy_X509_Basic256Sha256SecurityPolicy;

// UserTokenPolicyId for x509 token type with default SecurityPolicy example
#define SOPC_UserTokenPolicy_X509_ID "X509"
S2OPC_COMMON_EXPORT extern const OpcUa_UserTokenPolicy SOPC_UserTokenPolicy_X509_DefaultSecurityPolicy;

#endif
"""

H_FILE_END ="""
/*============================================================================
 * Table of known types.
 *===========================================================================*/
extern SOPC_EncodeableType** sopc_{prefix}KnownEncodeableTypes;

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

H_ENUM_FILE_CUSTOM_START = """
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

#ifndef SOPC_{prefix}Enum_Types_H_
#define SOPC_{prefix}Enum_Types_H_ 1

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
{export}extern SOPC_EncodeableType OpcUa_{name}_EncodeableType;

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
 * The enumerated values are indexes in the sopc_{prefix}KnownEncodeableTypes array.
 *===========================================================================*/
typedef enum _SOPC_{prefix}TypeInternalIndex
{{"""

TYPE_INDEX_DECL_ENCODEABLE_TYPE = """
#ifndef OPCUA_EXCLUDE_{name}
    SOPC_TypeInternalIndex_{name},
#endif"""

TYPE_INDEX_DECL_END = """
    SOPC_{prefix}TypeInternalIndex_SIZE
}} SOPC_{prefix}TypeInternalIndex;
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

C_FILE_CUSTOM_START = """
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

#include "opcua_{prefix}identifiers.h"
#include "{types_h_file}"
#include "sopc_encoder.h"

/* DO NOT EDIT. THIS FILE IS GENERATED BY THE SCRIPT gen-sopc-types.py */

"""[1:]

ENCODEABLE_TYPES_DESC_START = """

/*============================================================================
 * Encodeable types descriptions
 *===========================================================================*/

"""

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
        {is_same_ns}, // isSameNs
        (uint16_t) {ns_index}, // nsIndex
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
    {ns_URI},
    {ns_index},
    sizeof({ctype}),
    OpcUa_{name}_Initialize,
    OpcUa_{name}_Clear,
    {nof_fields},
    {field_desc},
    g_{prefix}KnownEncodeableTypes
}};
#endif
"""

CSTS_USER_TOKEN_POLICIES = """
#ifndef OPCUA_EXCLUDE_UserTokenPolicy
/*============================================================================
 * UserTokenPolicies example constant values
 *===========================================================================*/

const OpcUa_UserTokenPolicy SOPC_UserTokenPolicy_Anonymous = {
    .encodeableType = &OpcUa_UserTokenPolicy_EncodeableType,
    .PolicyId = SOPC_STRING(SOPC_UserTokenPolicy_Anonymous_ID),
    .TokenType = OpcUa_UserTokenType_Anonymous,
    .IssuedTokenType = SOPC_STRING_NULL,
    .IssuerEndpointUrl = SOPC_STRING_NULL,
    .SecurityPolicyUri = SOPC_STRING_NULL,
};

const OpcUa_UserTokenPolicy SOPC_UserTokenPolicy_UserName_NoneSecurityPolicy = {
    .encodeableType = &OpcUa_UserTokenPolicy_EncodeableType,
    .PolicyId = SOPC_STRING(SOPC_UserTokenPolicy_UserNameNone_ID),
    .TokenType = OpcUa_UserTokenType_UserName,
    .IssuedTokenType = SOPC_STRING_NULL,
    .IssuerEndpointUrl = SOPC_STRING_NULL,
    .SecurityPolicyUri = SOPC_STRING(SOPC_SecurityPolicy_None_URI),
    /* None security policy shall be used only when
   secure channel security policy is non-None and with encryption since password will be non-encrypted */
};

const OpcUa_UserTokenPolicy SOPC_UserTokenPolicy_UserName_DefaultSecurityPolicy = {
    .encodeableType = &OpcUa_UserTokenPolicy_EncodeableType,
    .PolicyId = SOPC_STRING(SOPC_UserTokenPolicy_UserName_ID),
    .TokenType = OpcUa_UserTokenType_UserName,
    .IssuedTokenType = SOPC_STRING_NULL,
    .IssuerEndpointUrl = SOPC_STRING_NULL,
    .SecurityPolicyUri = SOPC_STRING_NULL,
    /* Default security policy shall be used only when
   secure channel security policy is non-None since password will be non-encrypted */
};

const OpcUa_UserTokenPolicy SOPC_UserTokenPolicy_UserName_Basic256Sha256SecurityPolicy = {
    .encodeableType = &OpcUa_UserTokenPolicy_EncodeableType,
    .PolicyId = SOPC_STRING(SOPC_UserTokenPolicy_UserNameBasic256Sha256_ID),
    .TokenType = OpcUa_UserTokenType_UserName,
    .IssuedTokenType = SOPC_STRING_NULL,
    .IssuerEndpointUrl = SOPC_STRING_NULL,
    .SecurityPolicyUri = SOPC_STRING(SOPC_SecurityPolicy_Basic256Sha256_URI),
    /* Basic256Sha256 security policy might be used to ensure password is encrypted in any security policy and mode */
};

const OpcUa_UserTokenPolicy SOPC_UserTokenPolicy_X509_Basic256Sha256SecurityPolicy = {
    .encodeableType = &OpcUa_UserTokenPolicy_EncodeableType,
    .PolicyId = SOPC_STRING(SOPC_UserTokenPolicy_X509Basic256Sha256_ID),
    .TokenType = OpcUa_UserTokenType_Certificate,
    .IssuedTokenType = SOPC_STRING_NULL,
    .IssuerEndpointUrl = SOPC_STRING_NULL,
    .SecurityPolicyUri = SOPC_STRING(SOPC_SecurityPolicy_Basic256Sha256_URI),
};

const OpcUa_UserTokenPolicy SOPC_UserTokenPolicy_X509_DefaultSecurityPolicy = {
    .encodeableType = &OpcUa_UserTokenPolicy_EncodeableType,
    .PolicyId = SOPC_STRING(SOPC_UserTokenPolicy_X509_ID),
    .TokenType = OpcUa_UserTokenType_Certificate,
    .IssuedTokenType = SOPC_STRING_NULL,
    .IssuerEndpointUrl = SOPC_STRING_NULL,
    .SecurityPolicyUri = SOPC_STRING_NULL,
};

#endif
"""

TYPE_TABLE_START = """
/*============================================================================
 * Table of known types.
 *===========================================================================*/
static SOPC_EncodeableType* g_{prefix}KnownEncodeableTypes[\
SOPC_{prefix}TypeInternalIndex_SIZE + 1] = {{
"""[:-1]

TYPE_TABLE_ENTRY = """
#ifndef OPCUA_EXCLUDE_{name}
    &OpcUa_{name}_EncodeableType,
#endif"""

TYPE_TABLE_END = """
    NULL
};
"""

C_FILE_KNOWN_ENC_TYPES = """
SOPC_EncodeableType** sopc_{prefix}KnownEncodeableTypes = g_{prefix}KnownEncodeableTypes;
"""

C_FILE_ENUM_FUN_DEFS = """
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
"""

C_FILE_END = """
/* This is the last line of an autogenerated file. */
"""

if __name__ == '__main__':
    main()
