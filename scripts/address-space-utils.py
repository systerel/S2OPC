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


# A small tool to merge address spaces in their XML forms,
#  remove some nodes and references,
#  generate reciprocal references,
#  build some consistency checks.


import argparse
import sys
import xml.etree.ElementTree as ET


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
    #if '' in ns_map:
    #    ns_map['default'] = ns_map['']
    return ns_map

# We should test another parser, such as lxml parser...
#NAMESPACEs = {
#    'xsd': 'http://www.w3.org/2001/XMLSchema',
#    'xsi': 'http://www.w3.org/2001/XMLSchema-instance',
#    'default': 'http://opcfoundation.org/UA/2011/03/UANodeSet.xsd',
#    'uax': 'http://opcfoundation.org/UA/2008/02/Types.xsd',
#    }


def remove_max_monit(tree):
    #<!-- delete references to MaxMonitoredItemsPerCall -->
    #<xsl:template match="Reference">
    #  <xsl:if test="not(current() = 'i=11714')">
    #    <xsl:copy>
    #        <xsl:apply-templates select="@*|node()"/>
    #    </xsl:copy>
    #  </xsl:if>
    #</xsl:template>

    #<!-- delete MaxMonitoredItemsPerCall -->
    #<xsl:template match="UAVariable">
    #  <xsl:if test="not(@NodeId = 'i=11714')">
    #    <xsl:copy>
    #        <xsl:apply-templates select="@*|node()"/>
    #    </xsl:copy>
    #  </xsl:if>
    #</xsl:template>
    pass

def remove_methods(tree):
    #<!-- delete all method with a methodDeclarationId (others are in type definition) -->
    #<xsl:template match="UAMethod[not(count(@MethodDeclarationId)=0)]"/>

    #<!-- delete all references to the method deleted + reference to MaxNodesPerMethodCall -->
    #<xsl:template match="Reference">
    #  <xsl:if test="not(current() = //UAMethod[not(count(@MethodDeclarationId)=0)]/@NodeId) and not(current() = 'i=11709')">
    #    <xsl:copy>
    #        <xsl:apply-templates select="@*|node()"/>
    #    </xsl:copy>
    #  </xsl:if>
    #</xsl:template>

    #<!-- delete children variables of method nodes deleted + MaxNodesPerMethodCall -->
    #<xsl:template match="UAVariable">
    #  <xsl:if test="not(@ParentNodeId = //UAMethod[not(count(@MethodDeclarationId)=0)]/@NodeId) and not(@NodeId = 'i=11709')">
    #    <xsl:copy>
    #        <xsl:apply-templates select="@*|node()"/>
    #    </xsl:copy>
    #  </xsl:if>
    #</xsl:template>
    pass

def sanitize(tree):
    """
    Returns True if the sanitation is a success.
    Otherwise there is an unrecoverable error which requires more user input (two nodes with the same nodeid, ...).
    """
    # Prepare the common structures for find and check for uniqueness
    nodes = {}
    error = False
    for node in tree.iterfind('./*[@NodeId]'):
        nid = node.get('NodeId')
        if nid in nodes:
            print('Sanitize Error: NodeId {} found twice'.format(nid), file=sys.stderr)
            error = True
        nodes[nid] = node
    if error:
        return False

    # Add reciprocal References
    # References are a tuple (SourceNode, ReferenceType, TargetNode) (Part 3, §4.3.4)
    # Only the SourceNode is required to be in the address space.
    # All References should be unique.
    # Note that if there is (a, type0, b) and (a, type1, b), they describe the same Reference
    #  if type0 and type1 are subclasses of the same concrete ReferenceType.
    # When the reference a -> b exists, and when browsing b, b <- a also exists in the inverse direction.
    # We add reciprocal References to avoid their computations at browse-time.

    # First, compute the a -> b and b <- a sets of references
    # If no reference is missing, refs_fwd == refs_inv
    # In the Address Space, b <- a References are stored in b, hence the difficulty
    refs_fwd = {node: set() for node in nodes}  # {a: {(type, b), ...}}
    refs_inv = {node: set() for node in nodes}  # {a: {(type, b), ...}}, already existing inverse references b <- a are stored in refs_inv[a]
    for node in tree.iterfind('./*[References]'):
        nida = node.get('NodeId')  # The starting node of the references below
        refs, = node.iterfind('References')
        for ref in list(refs):  # Make a list so that we can remove elements while iterating
            type_ref = ref.get('ReferenceType')
            nidb = ref.text  # The destination node of this reference
            is_fwd = ref.get('IsForward') != 'false'
            if is_fwd:  # a -> b
                fwds = refs_fwd[nida]
                if (type_ref, nidb) in fwds:
                    print('Sanitize: duplicate forward Reference {} -> {} (type {})'.format(nida, nidb, type_ref), file=sys.stderr)
                    refs.remove(ref)
                fwds.add((type_ref, nidb))
            else:  # b <- a is stored in refs_inv[a]
                if nidb == 'None' or nidb is None:
                    print(nida)
                invs = refs_inv[nidb]
                if (type_ref, nida) in invs:
                    print('Sanitize: duplicate inverse Reference {} <- {} (type {})'.format(nidb, nida, type_ref), file=sys.stderr)
                    refs.remove(ref)
                invs.add((type_ref, nida))

    # Now add inverse refs b <- a for which a -> b exists
    trs_fwd = set((a,t,b) for a,ltr in refs_fwd.items() for t,b in ltr)
    trs_inv = set((a,t,b) for a,ltr in refs_inv.items() for t,b in ltr)
    for a, t, b in trs_fwd - trs_inv:
        if b not in nodes:
            print('Sanitize: Reference to unknown node, cannot add inverse reciprocal ({} -> {}, type {})'.format(a, b, t), file=sys.stderr)
        else:
            print('Sanitize: add inverse reciprocal Reference {} <- {} (type {})'.format(b, a, t), file=sys.stderr)
            node = nodes[b]
            # TODO: This is the "add_reference" function, do not copy paste it, factorize it
            refs_nodes = node.findall('References')
            if len(refs_nodes) < 1:
                refs = ET.Element('References')
                node.append(refs)
            else:
                refs, = refs_nodes
            elem = ET.Element('Reference', {'ReferenceType': t, 'IsForward': 'false'})
            elem.text = a
            refs.append(elem)
    # Add forward refs (there should be less)
    for a, t, b in trs_inv - trs_fwd:
        if a not in nodes:
            print('Sanitize: inverse Reference to unknown node, cannot add forward reciprocal ({} -> {}, type {})'.format(a, b, t), file=sys.stderr)
        else:
            print('Sanitize: add forward reciprocal Reference {} -> {} (type {})'.format(a, b, t), file=sys.stderr)
            node = nodes[a]
            # TODO: This is the "add_reference" function, do not copy paste it, factorize it
            refs_nodes = node.findall('References')
            if len(refs_nodes) < 1:
                refs = ET.Element('References')
                node.append(refs)
            else:
                refs, = refs_nodes
            elem = ET.Element('Reference', {'ReferenceType': t})
            elem.text = b
            refs.append(elem)

    # Note: ParentNodeId are optional. We add the inverse reference if it is mentioned.
    #  A ParentNodeId is an inverse reference typed "HasComponent"
    for node in tree.iterfind('./*[@ParentNodeId]'):
        # There may be no reference at all
        refs_nodes = node.findall('References')
        if len(refs_nodes) < 1:
            print('Sanitize: child Node without references (Node {} has an attribute ParentNodeId but no reference)'
                  .format(node.get('NodeId')), file=sys.stderr)
            # Note: the attrib member may be an interface, so this is not portable; however the ET lib does not provide other means to do this.
            del node.attrib['ParentNodeId']
            continue
        refs, = refs_nodes
        pnid = node.get('ParentNodeId')
        parent_refs = refs.findall('*[@IsForward="false"]')
        if not any(parent.text == pnid for parent in parent_refs):
            print('Sanitize: child Node without reference to its parent (Node {}, which parent is {})'
                  .format(node.get('NodeId'), pnid), file=sys.stderr)
            # Type is unknown in fact
            #refs.append(ET.Element('Reference', {'ReferenceType': 'HasComponent', 'IsForward': 'false'}, text=pnid))
            # Note: the attrib member may be an interface, so this is not portable; however the ET lib does not provide other means to do this.
            del node.attrib['ParentNodeId']

    # Note: we don't check that the Address Space Model specified in Part 3 is valid.

    return True


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='A tool to merge (and more) XMLs of OPC UA Address Spaces.')
    #parser.add_argument('--address-space', '-a', metavar='XML', dest='fns_adds', action='append', required=True,
    #                    help='Path the address spaces to merge. In case of conflicting elements, '+
    #                         'the element from the first address space in the argument order is kept.')
    parser.add_argument('fns_adds', nargs='+', metavar='XML',
                        help='Path (or - for stind) the address spaces to merge. '+
                             'In case of conflicting elements, '+
                             'the element from the first address space in the argument order is kept.')
    parser.add_argument('--output', '-o', metavar='XML', dest='fn_out', #required=True,
                        help='Path to the output file')# (default to stdout)')
    #parser.add_argument('--no-gen-reciprocal', action='store_false', dest='reciprocal',
    #                    help='Suppress the normal behavior which is to generate reciprocal references between nodes that only have one to the other.')
    parser.add_argument('--remove-max-monitored-items', action='store_true', dest='remove_max_monit',
                        help='Remove the MaxMonitoredItems node and references to it')
    parser.add_argument('--remove-methods', action='store_true', dest='remove_methods',
                        help='Remove nodes and references that enable the use of methods')
    parser.add_argument('--no-sanitize', action='store_false', dest='sanitize',
                        help='''
            Suppress the normal behavior which is to sanitize the model after merge/additions/removal.
            The normal behavior is to check for duplicate nodes,
            generate reciprocal references between nodes when there is a reference in only one direction,
            and remove attribute ParentNodeId when erroneous.
                             ''')
    args = parser.parse_args()

    # Load and merge all address spaces
    tree = None
    namespaces = {}
    for fname in args.fns_adds:
        # TODO: Merge
        source = fname if fname != '-' else sys.stdin
        new_ns = parse_xmlns(source)
        for k,v in new_ns.items():
            if k not in namespaces:
                # Keep first version of the namespaces
                ET.register_namespace(k, v)
                namespaces[k] = v
        tree = ET.parse(source)

    # Apply options afterwards
    if args.remove_max_monit:
        remove_max_monit(tree)

    if args.remove_methods:
        remove_methods(tree)

    if args.sanitize:
        res = sanitize(tree)

    if res:
        tree.write(args.fn_out or sys.stdout)
    else:
        print('There was some unrecoverable error{}'
              .format(', did not save to {}'.format(args.fn_out) if args.fn_out else ''),
              file=sys.stderr)
