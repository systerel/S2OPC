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

INDENT_SPACES = '  '

def indent(level):
    return '\n' + INDENT_SPACES*level

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

def _remove_nids(tree, nids):
    # Remove nodes that matches all NodeIds in nids
    root = tree.getroot()
    for nid in nids:
        node = root.find('*[@NodeId="{}"]'.format(nid))
        if node is not None:
            if args.verbose:
                print('RemoveNode: {}'.format(nid), file=sys.stderr)
            root.remove(node)

def _remove_refs_to_nids(tree, nids, namespaces):
    # Remove Reference elements from all nodes that go to the NodeIds in nids
    root = tree.getroot()
    for node in tree.iterfind('./*[uanodeset:References]', namespaces):
        nida = node.get('NodeId')  # The starting node of the references below
        refs, = node.iterfind('uanodeset:References', namespaces)
        for ref in list(refs):  # Make a list so that we can remove elements while iterating
            if ref.text.strip() in nids:  # The destination node of this reference
                if args.verbose:
                    print('RemoveRef: {} -> {}'.format(nida, ref.text.strip()), file=sys.stderr)
                refs.remove(ref)

def _add_ref(node, ref_type, tgt, is_forward=True):
    # Add a reference from a node to the other NodeId nid in the given direction
    refs_nodes = node.findall('uanodeset:References', namespaces)
    if len(refs_nodes) < 1:
        refs = ET.Element('uanodeset:References', namespaces)
        # Manual identation with ET... This might not adjust well, we should also indent the latest brother
        refs.text = indent(2)
        refs.tail = indent(2)
        node.append(refs)
    else:
        refs, = refs_nodes
    attribs = {'ReferenceType': ref_type}
    if not is_forward:
        attribs['IsForward'] = 'false'
    elem = ET.Element('Reference', attribs)
    elem.text = tgt
    if len(refs) > 0:
        refs[-1].tail = indent(3)
    else:
        refs.text = indent(3)
    elem.tail = indent(2)
    refs.append(elem)

def merge(tree, new, namespaces):
    # Merge new tree into tree
    # The merge is restricted to tags for which we know the semantics
    # There are also some (maybe redundant) informations that are ignored by the S2OPC parser.

    # Merge Aliases
    tree_aliases = tree.find('uanodeset:Aliases', namespaces)
    if tree_aliases is None:
        print('Merge: Aliases expected to be present in first address space')
        return false
    tree_alias_dict = {alias.get('Alias'):alias.text for alias in tree_aliases}  # Assumes that the model does not have the same alias defined multiple times
    new_aliases = new.find('uanodeset:Aliases', namespaces)
    new_alias_dict = {}
    if new_aliases is not None:
        new_alias_dict = {alias.get('Alias'):alias.text for alias in new_aliases}
    # Assert existing aliases are the same
    res = True
    for alias in set(tree_alias_dict)&set(new_alias_dict):
        if tree_alias_dict[alias] != new_alias_dict[alias]:
            print('Merge: Alias used for different NodeId ({} is {} or {})'
                  .format(alias, tree_alias_dict[alias], new_alias_dict[alias]), file=sys.stderr)
            res = False
    if not res:
        return res

    # Add new aliases
    for alias in set(new_alias_dict)-set(tree_alias_dict):
        elem = ET.Element('Alias', {'Alias': alias})
        elem.text = new_alias_dict[alias]
        # Set correct indent level for current tag (tail of previous <Alias/> or text of <Aliases>)
        if len(tree_aliases) > 0:
            tree_aliases[-1].tail = indent(2)
        else:
            tree_aliases.text = indent(2)
        tree_aliases.append(elem)

    if len(tree_aliases) > 0:
        # Restore correct level for next tag which is </Aliases>
        tree_aliases[-1].tail = indent(1)

    # Merge Nodes
    tree_nodes = {node.get('NodeId'):node for node in tree.iterfind('*[@NodeId]')}
    new_nodes = {node.get('NodeId'):node for node in new.iterfind('*[@NodeId]')}
    # New nodes are copied
    tree_root = tree.getroot()
    for nid in set(new_nodes)&set(tree_nodes):
        print('Merged: skipped already known node {}'.format(nid), file=sys.stderr)
    # New unique nids
    new_nids = set(new_nodes)-set(tree_nodes)
    if len(new_nids) > 0 and len(tree_root) > 0:
        # indent for first node added
        tree_root[-1].tail = indent(1)
    for node in new.iterfind('*[@NodeId]'):
        nid = node.get('NodeId')
        if nid in new_nids:
            if args.verbose:
                print('Merge: add node {}'.format(nid), file=sys.stderr)
            tree_root.append(new_nodes[nid])
    # References of common nodes are merged
    for nid in set(tree_nodes)&set(new_nodes):
        nodeb = new_nodes[nid]
        refsb = nodeb.find('./uanodeset:References', namespaces)
        if refsb is None:
            continue
        nodea = tree_nodes[nid]
        for ref in refsb:
            _add_ref(nodea, ref.get('ReferenceType'), ref.text, is_forward=(ref.get('IsForward') != 'false'))

    return True

def remove_max_monit(tree, namespaces):
    # Delete MaxMonitoredItemsPerCall
    _remove_nids(tree, ['i=11714'])

    # We have to remove references to MaxMonitoredItemsPerCall manually,
    #  as there may exist references to unknown nodes in an address space.
    _remove_refs_to_nids(tree, ['i=11714'], namespaces)

def remove_methods(tree, namespaces):
    # Delete methods that are instances of other methods.
    # For now, this difference between instantiated methods or not is solely based on the MethodDeclarationId.
    # See Part 3 §6 for more information.
    # (also delete MaxNodesPerMethodCall and its references)
    # (also delete properties of the methods)
    methods = []
    methods_properties = []
    for method_node in tree.findall('*[@MethodDeclarationId]'):
        methods.append(method_node.get('NodeId'))
        refs, = method_node.iterfind('uanodeset:References', namespaces)
        for ref in refs:
            ref_type = ref.get('ReferenceType')
            if ref.get('IsForward') != 'false' and (ref_type == 'HasProperty' or ref_type == 'i=46'):
                methods_properties.append(ref.text.strip())

    _remove_nids(tree, methods+methods_properties+['i=11709'])
    _remove_refs_to_nids(tree, methods+methods_properties+['i=11709'], namespaces)

def sanitize(tree, namespaces):
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
    # Set of forward reference (check existence)
    refs_fwd = set() # {(a,type, b), ...}
    # List of forward reference (keep refs order)
    refs_fwd_list = [] # [(a, type, b), ...]
    # Set of backward reference (check existence)
    refs_inv = set()  # {(a, type, b), ...}, already existing inverse references b <- a are stored in the forward direction (source to target)
    # List of backward reference (keep refs order)
    refs_inv_list = [] # [(a, type, b), ...], already existing inverse references b <- a
    for node in tree.iterfind('./*[uanodeset:References]', namespaces):
        nids = node.get('NodeId')  # The starting node of the references below
        refs, = node.iterfind('uanodeset:References', namespaces)
        for ref in list(refs):  # Make a list so that we can remove elements while iterating
            type_ref = ref.get('ReferenceType')
            nidt = ref.text.strip()  # The destination node of this reference
            is_fwd = ref.get('IsForward') != 'false'
            if is_fwd:
                # We are in the case a -> b,
                #  so a = nids, and b = nidt
                if (nids, type_ref, nidt) in refs_fwd:
                    print('Sanitize: duplicate forward Reference {} -> {} (type {})'.format(nids, nidt, type_ref), file=sys.stderr)
                else:
                    refs_fwd.add((nids, type_ref, nidt))
                    refs_fwd_list.append((nids, type_ref, nidt))
            else:
                # We are in the case b <- a,
                #  so b = nids, and a = nidt
                #  and nids <- nidt will be stored in forward order (nidt, type, nids)
                if nidt not in nodes:
                    print('Sanitize: inverse Reference from unknown node, cannot add forward reciprocal ({} -> {}, type {})'
                          .format(nids, nidt, type_ref), file=sys.stderr)
                    continue
                if (nidt, type_ref, nids) in refs_inv:
                    print('Sanitize: duplicate inverse Reference {} <- {} (type {})'.format(nidt, nids, type_ref), file=sys.stderr)
                else:
                    refs_inv.add((nidt, type_ref, nids))
                    refs_inv_list.append((nidt, type_ref, nids))

    # Add forward refs a -> b for which b <- a exists
    for a, t, b in refs_inv_list:
        if (a, t, b) in refs_fwd:
            # Already defined
            continue
        if a not in nodes:
            print('Sanitize: inverse Reference from unknown node, cannot add forward reciprocal ({} -> {}, type {})'.format(a, b, t), file=sys.stderr)
        else:
            if args.verbose:
                print('Sanitize: add forward reciprocal Reference {} -> {} (type {})'.format(a, b, t), file=sys.stderr)
            node = nodes[a]
            _add_ref(node, t, b, is_forward=True)

    # Now add inverse refs b <- a for which a -> b exists
    for a, t, b in refs_fwd_list:
        if (a, t, b) in refs_inv:
            # Already defined
            continue
        if b not in nodes:
            print('Sanitize: Reference to unknown node, cannot add inverse reciprocal ({} -> {}, type {})'.format(a, b, t), file=sys.stderr)
        else:
            if args.verbose:
                print('Sanitize: add inverse reciprocal Reference {} <- {} (type {})'.format(b, a, t), file=sys.stderr)
            node = nodes[b]
            _add_ref(node, t, a, is_forward=False)

    # Note: ParentNodeId is an optional attribute. It refers to the parent node.
    #  In case the ParentNodeId is present, but the reference to the parent is not, the attribute is removed.
    # The reference to the ParentNodeId should be typed "HasComponent" (not verified)
    for node in tree.iterfind('./*[@ParentNodeId]'):
        # There may be no reference at all
        refs_nodes = node.findall('uanodeset:References', namespaces)
        if len(refs_nodes) < 1:
            print('Sanitize: child Node without references (Node {} has an attribute ParentNodeId but no reference)'
                  .format(node.get('NodeId')), file=sys.stderr)
            # Note: the attrib member may be an interface, so this is not portable; however the ET lib does not provide other means to do this.
            del node.attrib['ParentNodeId']
            continue
        refs, = refs_nodes
        pnid = node.get('ParentNodeId')
        parent_refs = refs.findall('*[@IsForward="false"]')
        if not any(parent.text.strip() == pnid for parent in parent_refs):
            print('Sanitize: child Node without reference to its parent (Node {}, which parent is {})'
                  .format(node.get('NodeId'), pnid), file=sys.stderr)
            # Type is unknown in fact
            #refs.append(ET.Element('Reference', {'ReferenceType': 'HasComponent', 'IsForward': 'false'}, text=pnid))
            # Note: the attrib member may be an interface, so this is not portable; however the ET lib does not provide other means to do this.
            del node.attrib['ParentNodeId']

    # Note: we don't check that the Address Space Model specified in Part 3 is valid.

    # TODO: Remove empty <References />

    return True


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='A tool to merge (and more) XMLs of OPC UA Address Spaces.')
    parser.add_argument('fns_adds', nargs='+', metavar='XML',
                        help='''
            Path (or - for stdin) the address spaces to merge. In case of conflicting elements,
            the element from the first address space in the argument order is kept.
            The models must be for the same OPC UA version (e.g. 1.03).
                             ''')
    parser.add_argument('--output', '-o', metavar='XML', dest='fn_out', #required=True,
                        help='Path to the output file')# (default to stdout)')
    # TODO: if this feature is needed...
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
    parser.add_argument('--verbose', '-v', action='store_true',
                        help='Display information (reciprocal references added, merged nodes, removed nodes, etc.)')
    args = parser.parse_args()
    # Check that '-' is provided only once in input address spaces
    assert args.fns_adds.count('-') < 2, 'You can only take a single XML from the standard input'

    # Load and merge all address spaces
    tree = None
    namespaces = {}
    for fname in args.fns_adds:
        source = fname if fname != '-' else sys.stdin
        new_ns = parse_xmlns(source)
        for k,v in new_ns.items():
            if k not in namespaces:
                # Keep first version of the namespaces
                ET.register_namespace(k, v)
                namespaces[k] = v
        new_tree = ET.parse(source)
        if tree is None:
            tree = new_tree
            # ElementTree does not support XPath search with the default namespace.
            # We have to name it to be able to use it.
            if '' in namespaces:
                namespaces['uanodeset'] = namespaces['']
        else:
            merge(tree, new_tree, namespaces)

    # Apply options afterwards
    if args.remove_max_monit:
        remove_max_monit(tree, namespaces)

    if args.remove_methods:
        remove_methods(tree, namespaces)

    if args.sanitize:
        res = sanitize(tree, namespaces)
    else:
        res = True

    if res:
        tree.write(args.fn_out or sys.stdout.buffer, encoding="utf-8", xml_declaration=True)
    else:
        print('There was some unrecoverable error{}'
              .format(', did not save to {}'.format(args.fn_out) if args.fn_out else ''),
              file=sys.stderr)
