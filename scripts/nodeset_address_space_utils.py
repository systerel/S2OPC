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
import re


NS_IDX_MATCHER = re.compile(r'ns=(\d+);(.+)')
NS_IDX_FORMATTER = 'ns={};{}'
PREFIX_IDX_MATCHER = re.compile(r'(\d+):(.+)')
PREFIX_IDX_FORMATTER = '{}:{}'


UA_URI = 'http://opcfoundation.org/UA/'
UA_NODESET_URI = 'http://opcfoundation.org/UA/2011/03/UANodeSet.xsd'
UA_TYPES_URI = 'http://opcfoundation.org/UA/2008/02/Types.xsd'
UA_TYPES_PREFIX = 'uax'
STRING_TAG = f'{{{UA_TYPES_URI}}}String'

HIERARCHICAL_REFERENCE_TYPES = frozenset([
    'HasEventSource',
    'HasChild',
    'Organizes',
    'HasNotifier,'
    'Aggregates',
    'HasSubtype',
    'HasProperty',
    'HasComponent',
    'HasOrderedComponent',
    "HasEncoding" # assimilated as hierarchical reference since target only depends on source node
    ])

TYPE_DEFINITION_REFERENCE_TYPE = 'HasTypeDefinition'


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


def is_forward(ref: ET.Element):
    return ref.get('IsForward') != 'false'


def get_ns0_version(tree_models):
    for model in tree_models:
        if model.get('ModelUri') == UA_URI:
            return model.get('Version')
    return None


def _check_declared_nid(nid, ns_count, matcher):
    m = matcher.match(nid)
    if m is not None:
        ns = int(m.group(1))
        if ns < 0:
            raise Exception (f"Invalid namespace usage ns={ns} in {nid}")
        if ns > ns_count:
            raise Exception(f"Missing namespace declaration for ns={ns} in {nid}, " +
                            f"whereas {ns_count} namespace{' is' if ns_count <2 else 's are'} declared")


def _is_hierarchical_ref(ref: ET.Element):
    ref_type = ref.get('ReferenceType')
    return ref_type in HIERARCHICAL_REFERENCE_TYPES

def _is_ns0(nid):
    return not nid.startswith('ns=') or nid.startswith('ns=0;')


def append_strings(parent_l_str: ET.Element, str_values):
    if len(str_values) == 0:
        # nothing to do
        return
    for str_val in str_values:
        str_elem = ET.SubElement(parent_l_str, STRING_TAG)
        str_elem.text = str_val


class NSFinder:

    def __init__(self, namespaces):
        self.namespaces = namespaces

    def _find_in(self, base, path):
        return base.find(path, self.namespaces)

    def _findall(self, base, path):
        return base.findall(path, self.namespaces)

    def _iterfind(self, base, path):
        yield from base.iterfind(path, self.namespaces)


class NSIndexReassigner(NSFinder):

    def __init__(self, namespaces):
        super(NSIndexReassigner, self).__init__(namespaces)
        self.ns_idx_reassign = dict()

    def compute_reassignment(self, tree_ns_uris: dict, new_ns_uris: ET.Element):
        """
        Compute the NS index reassignments and store it internally.
        Return the new NS URI nodes that shall be appended in the merged document.
        """
        # the new namespace URIs from the new address space need to be translated
        # but some of the namespace might already be in use
        ns_uris = dict()
        for idx, ns in enumerate(self._findall(tree_ns_uris, 'uanodeset:Uri')):
            ns_uris[ns.text] = idx + 1

        new_ns_nodes = list()
        for idx, ns in enumerate(self._findall(new_ns_uris, 'uanodeset:Uri')):
            if ns.text in ns_uris:
                tree_idx = ns_uris[ns.text]
                if tree_idx != idx + 1:
                    self.ns_idx_reassign[idx + 1] = tree_idx
            else:
                new_idx = len(ns_uris) + 1
                ns_uris[ns.text] = new_idx
                if new_idx != idx + 1:
                    self.ns_idx_reassign[idx + 1] = new_idx
                new_ns_nodes.append(ns)
        # print("Namespace URI reassignments:", self.ns_idx_reassign)
        return new_ns_nodes

    def get_ns_index(self, expr: str):
        return self.__reassigned_ns_index(expr)

    def reassign_node_ns_index(self, node: ET.Element):
        # Reassign namespace index for:
        #   @NodeId, @BrowseName, @ParentNodeId, @DataType,
        #   References/Reference/@ReferenceType and References/Reference/text
        for attr in ['NodeId', 'ParentNodeId', 'DataType']:
            self.__reassign_elem_attr(node, attr, self.__reassigned_ns_index)
        self.__reassign_elem_attr(node, 'BrowseName', self.__reassigned_prefix_index)

        for ref in self._iterfind(node, 'uanodeset:References/uanodeset:Reference'):
            self.__reassign_elem_attr(ref, 'ReferenceType', self.__reassigned_ns_index)
            ref.text = self.__reassigned_ns_index(ref.text)

    def __get_reassigned_expr(self, expr: str, matcher, formatter):
        m = matcher.match(expr)
        if m is not None:
            expr_idx = int(m.group(1))
            new_idx = self.ns_idx_reassign.get(expr_idx)
            if new_idx is not None:
                return formatter.format(new_idx, m.group(2))
        return expr

    def __reassigned_ns_index(self, expr: str):
        return self.__get_reassigned_expr(expr, NS_IDX_MATCHER, NS_IDX_FORMATTER)

    def __reassigned_prefix_index(self, expr: str):
        return self.__get_reassigned_expr(expr, PREFIX_IDX_MATCHER, PREFIX_IDX_FORMATTER)

    def __reassign_elem_attr(self, elem: ET.Element, attr: str, fun_reassigned):
        val = elem.get(attr)
        if val is not None:
            r_val = fun_reassigned(val)
            elem.set(attr, r_val)


class NodesetMerger(NSFinder):
    #TODO too big: split into various roles, encapsulate tree access

    def __init__(self, verbose):
        super(NodesetMerger, self).__init__(dict())
        self.verbose = verbose
        self.__source = None
        self.tree = None
        self.ns_idx_reassigner = NSIndexReassigner(self.namespaces)

    def _find(self, path):
        return self._find_in(self.tree, path)

    def _find_node_with_nid(self, nid):
        return self._find(f'*[@NodeId="{nid}"]')

    def _iter_nid_nodes_in(self, any_tree: ET.ElementTree):
        for node in self._iterfind(any_tree, '*[@NodeId]'):
            yield node, node.get('NodeId')

    def _iter_nid_nodes(self):
        yield from self._iter_nid_nodes_in(self.tree)

    def _create_elem(self, tag, attrib={}, xmlns='uanodeset'):
        ns_tag = '{' + self.namespaces[xmlns] + '}' + tag
        return ET.Element(ns_tag, attrib)

    def _add_ref(self, node, ref_type, tgt, is_forward=True):
        # Add a reference from a node to the other NodeId nid in the given direction
        attribs = {'ReferenceType': ref_type}
        if not is_forward:
            attribs['IsForward'] = 'false'

        refs_nodes = self._findall(node, 'uanodeset:References')
        if len(refs_nodes) == 0:
            refs = self._create_elem('References')
            node.append(refs)
        else:
            refs, = refs_nodes
            for ref in refs:
                if ref.text == tgt and ref.attrib == attribs:
                    # avoid duplicate reference
                    return

        elem = self._create_elem('Reference', attribs)
        elem.text = tgt
        refs.append(elem)

    def _remove_nids(self, nids):
        # Remove the nodes that match the NodeIds given in nids
        root = self.tree.getroot()
        for nid in nids:
            node = self._find_node_with_nid(nid)
            if node is not None:
                if self.verbose:
                    print('RemoveNode: {}'.format(nid), file=sys.stderr)
                root.remove(node)

    def _remove_refs_to_nids(self, nids):
        # Remove Reference elements from all nodes that go to the NodeIds in nids
        for node in self._iterfind(self.tree, './*[uanodeset:References]'):
            refs = self._find_in(node, 'uanodeset:References')
            if refs is None:
                return
            for ref in list(refs):  # Make a list so that we can remove elements while iterating
                if ref.text.strip() in nids:  # The destination node of this reference
                    if self.verbose:
                        print('RemoveRef: {} -> {}'.format(node.get('NodeId'), ref.text.strip()), file=sys.stderr)
                    refs.remove(ref)

    def _remove_nids_and_refs(self, nids):
        self._remove_nids(nids)
        self._remove_refs_to_nids(nids)

    def _check_all_namespaces_declared(self, any_tree):
        ns_count = 0
        ns_uris = self._find_in(any_tree, 'uanodeset:NamespaceUris')
        if ns_uris is not None:
            uris = self._findall(ns_uris, 'uanodeset:Uri')
            declarations = [uri.text for uri in uris]
            declared_ns = set(declarations)
            if len(declared_ns) != len(declarations):
                raise Exception("Duplicate Namespace URI declaration in: " + str(declarations))
            ns_count = len(declared_ns)
    
        for attr in ['NodeId', 'ParentNodeId', 'DataType']:
            for node in self._iterfind(any_tree, f'*[@{attr}]'):
                nid = node.get(attr)
                _check_declared_nid(nid, ns_count, NS_IDX_MATCHER)
        for node in self._iterfind(any_tree, f'*[@BrowseName]'):
            nid = node.get('BrowseName')
            _check_declared_nid(nid, ns_count, PREFIX_IDX_MATCHER)
        for ref in self._iterfind(any_tree, 'uanodeset:References/uanodeset:Reference'):
            _check_declared_nid(ref.get('ReferenceType'), ns_count, NS_IDX_MATCHER)
            _check_declared_nid(ref.text, ns_count, NS_IDX_MATCHER)
        for alias in self._iterfind(any_tree, 'uanodeset:Aliases/uanodeset:Alias'):
            _check_declared_nid(alias.text, ns_count, NS_IDX_MATCHER)

    def __merge_ns_uris(self, new: ET.ElementTree):
        self._check_all_namespaces_declared(new)
        new_ns_uris = self._find_in(new, 'uanodeset:NamespaceUris')
        if new_ns_uris is None:
            print(f"NamespaceUris is missing in {self.__source}, considering it as a NS0 address space extension")
            # TODO: check that the file does not contain any reference to ns=1 or more
            return True

        tree_ns_uris = self._find('uanodeset:NamespaceUris')
        if tree_ns_uris is None:
            tree_ns_uris = self._create_elem('NamespaceUris')
            self.tree.getroot().insert(0, new_ns_uris)

        new_ns_uri_nodes = self.ns_idx_reassigner.compute_reassignment(tree_ns_uris, new_ns_uris)
        for ns in new_ns_uri_nodes:
            tree_ns_uris.append(ns)
        return True

    def __merge_models(self, new: ET.ElementTree):
        tree_models = self._find('uanodeset:Models')
        if tree_models is None:
            print("Error: missing the <Models>")
            return False
        ns0_version = get_ns0_version(tree_models)
        if ns0_version is None:
            print("Error: Missing a NS0 <Model>")
            return False
        new_models = self._find_in(new, 'uanodeset:Models')
        for model in new_models:
            new_model_uri = model.get('ModelUri')
            already_model = self._find_in(tree_models, f'uanodeset:Model[@ModelUri="{new_model_uri}"]')
            if already_model is not None:
                already_model_version = already_model.get('Version')
                new_model_version = model.get('Version')
                if new_model_version == already_model_version:
                    # just skip the duplicate model
                    continue
                else:
                    raise Exception(f'Incompatible model version: {new_model_uri} provided with versions {already_model_version} and {new_model_version}')
            req_ns0 = self._find_in(model, f'uanodeset:RequiredModel[@ModelUri="{UA_URI}"]')
            if req_ns0 is not None:
                req_ns0_version = req_ns0.get('Version')
                if req_ns0_version != ns0_version:
                    raise Exception(f'Incompatible NS0 version: provided {ns0_version} but require {req_ns0_version}')
            tree_models.append(model)
        return True

    def __merge_aliases(self, new: ET.ElementTree):
        tree_aliases = self._find('uanodeset:Aliases')
        if tree_aliases is None:
            print('Merge: Aliases expected to be present in first address space')
            return False
        tree_alias_dict = {alias.get('Alias'):alias.text for alias in tree_aliases}
        new_aliases = self._find_in(new, 'uanodeset:Aliases')
        new_alias_dict = {}
        if new_aliases is not None:
            new_alias_dict = {alias.get('Alias'):self.ns_idx_reassigner.get_ns_index(alias.text) for alias in new_aliases}
        # Assert existing aliases are the same
        res = True
        for alias in sorted(set(tree_alias_dict) & set(new_alias_dict)):
            if tree_alias_dict[alias] != new_alias_dict[alias]:
                print('Merge: Alias used for different NodeId ({} is {} or {})'
                      .format(alias, tree_alias_dict[alias], new_alias_dict[alias]), file=sys.stderr)
                res = False
        if not res:
            return False

        # Add new aliases
        for alias in sorted(set(new_alias_dict) - set(tree_alias_dict)):
            elem = self._create_elem('Alias', {'Alias': alias})
            elem.text = new_alias_dict[alias]
            tree_aliases.append(elem)
    
        return True

    def __split_merged_nodes(self, new: ET.ElementTree):
        tree_nodes = dict()
        duplicates_source = set()
        for node, node_id in self._iter_nid_nodes():
            if node_id in tree_nodes:
                duplicates_source.add(node_id)
            else:
                tree_nodes[node_id] = node
        if len(duplicates_source) > 0:
            raise Exception(f"There are duplicate node IDs within the base file: {sorted(duplicates_source)}")

        new_nodes = dict()
        duplicates = set() # duplicates across merged files
        duplicates_target = set() # duplicates within the target file only, this is an error
        for node, _ in self._iter_nid_nodes_in(new):
            # Reassign namespace index for node attributes and subelements
            self.ns_idx_reassigner.reassign_node_ns_index(node)
            node_id = node.get('NodeId')
            if node_id in new_nodes:
                duplicates_target.add(node_id)
                continue
            if node_id in tree_nodes:
                duplicates.add(node_id)
            else:
                new_nodes[node_id] = node
        if len(duplicates_target) > 0:
            raise Exception(f"There are duplicate node IDs within the merged file: {sorted(duplicates_target)}")

        # ns0 duplicates across merged files are valid
        # for instance the Server, ServerArray, NamespaceArray nodes may appear in various files
        ns0_duplicates = filter(_is_ns0, duplicates)
        user_duplicates = duplicates - set(ns0_duplicates)
        if len(user_duplicates) > 0:
            raise Exception(f"There are duplicate Node IDs across merged files: {sorted(duplicates)}")
        return tree_nodes, new_nodes, ns0_duplicates

    def __merge_nodes(self, new: ET.ElementTree):
        tree_nodes, new_nodes, ns0_duplicate_nids = self.__split_merged_nodes(new)
        tree_root = self.tree.getroot()
        # New unique nids
        new_nids = set(new_nodes)
        for nid in sorted(new_nids):
            if self.verbose:
                print('Merge: add node {}'.format(nid), file=sys.stderr)
            tree_root.append(new_nodes[nid])

        # References of common nodes are merged
        for nid in sorted(ns0_duplicate_nids):
            nodeb = new_nodes.get(nid)
            assert nodeb is not None
            refsb = self._find_in(nodeb, './uanodeset:References')
            if refsb is None:
                continue
            nodea = tree_nodes.get(nid)
            assert nodea is not None
            for ref in refsb:
                self._add_ref(nodea, ref.get('ReferenceType'), ref.text, is_forward=is_forward(ref))

    def merge(self, source):
        # Merge new tree into tree
        # The merge is restricted to tags for which we know the semantics
        # There are also some (maybe redundant) informations that are ignored by the S2OPC parser.
        self.__source = source
        new_ns = parse_xmlns(source)
        for k,v in new_ns.items():
            if k not in self.namespaces:
                # Keep first version of the namespaces
                ET.register_namespace(k, v)
                self.namespaces[k] = v
        new = ET.parse(source)
        if self.tree is None:
            self.tree = new
            # ElementTree does not support XPath search with the default namespace.
            # We have to name it to be able to use it.
            if '' in self.namespaces:
                self.namespaces['uanodeset'] = self.namespaces['']
            if UA_TYPES_PREFIX not in self.namespaces:
                ET.register_namespace(UA_TYPES_PREFIX, UA_TYPES_URI)
                self.namespaces[UA_TYPES_PREFIX] = UA_TYPES_URI
            self.__fill_namespace_array()
            self._check_all_namespaces_declared(self.tree)
            return True

        # Merge NamespaceURIs
        if not self.__merge_ns_uris(new):
            return False

        # Merge Models
        if not self.__merge_models(new):
            return False

        # Merge ServerArray and NamespaceArray:
        self.__fill_namespace_array()
        self.__merge_server_array(new)

        # Merge Aliases
        if not self.__merge_aliases(new):
            return False

        # Merge Nodes, detect duplicate Node IDs (forbidden)
        self.__merge_nodes(new)

        return True

    def remove_max_monit(self):
        # Delete MaxMonitoredItemsPerCall
        self._remove_nids_and_refs(['i=11714'])

    def remove_max_node_mgt(self):
        # Delete MaxNodesPerNodeManagemeent
        self._remove_nids_and_refs(['i=11713'])

    def remove_methods(self):
        # Delete methods that are instances of other methods.
        # For now, this difference between instantiated methods or not is solely based on the MethodDeclarationId.
        # See Part 3 §6 for more information.
        # (also delete MaxNodesPerMethodCall and its references)
        # (also delete properties of the methods)
        methods = []
        methods_properties = []
        for method_node in self._iterfind(self.tree, '*[@MethodDeclarationId]'):
            methods.append(method_node.get('NodeId'))
            refs = self._find_in(method_node, 'uanodeset:References')
            if refs is None:
                continue
            for ref in refs:
                ref_type = ref.get('ReferenceType')
                if is_forward(ref) and (ref_type == 'HasProperty' or ref_type == 'i=46'):
                    methods_properties.append(ref.text.strip())

        self._remove_nids_and_refs(methods+methods_properties+['i=11709'])

    def remove_node_ids_greater_than(self, intMaxId):
        ns0nidPattern = re.compile('i=([0-9]+)')
        # Find Node Ids greater than intMaxId in NS 0
        nodes_to_remove = []
        for _, nid in self._iter_nid_nodes():
            match = ns0nidPattern.match(nid)
            if match:
                if int(match.group(1)) > intMaxId:
                    nodes_to_remove.append(nid)
        # Delete the concerned nodes and references associated
        self._remove_nids_and_refs(nodes_to_remove)

    def _iter_hierarchical(self, n: ET.Element, downwards=True):
        for ref in self._iterfind(n, "uanodeset:References/uanodeset:Reference"):
            if is_forward(ref) != downwards:
                continue
            if _is_hierarchical_ref(ref):
                child_nid = ref.text.strip()
                yield child_nid

    def _rec_compute_subtree(self, root_nid: str, subtree: dict):
        n = self._find_node_with_nid(root_nid)
        if n is None:
            return
        if root_nid not in subtree:
            subtree[root_nid] = set()
        for child_nid in self._iter_hierarchical(n):
            subtree[root_nid].add(child_nid)
            if child_nid not in subtree:
                subtree[child_nid] = set()
                self._rec_compute_subtree(child_nid, subtree)

    def _rec_bf_remove_subtree(self, remove_siblings: set, subtree: dict, is_root=False):
        # Breadth-First Removal of children with no parents outside the removed subtree
        children = set()
        removed_nids = set()
        for nid in remove_siblings:
            if nid not in subtree:
                # deleted, not to be removed
                continue
            n = self._find_node_with_nid(nid)
            outer_parents = [parent_nid for parent_nid in self._iter_hierarchical(n, downwards=False) if parent_nid not in subtree]
            if outer_parents and not is_root:
                # there is a parent not to be removed: don't remove this node and all its subtree
                # except for the 'root' nodes, that the user explicitly requested to remove
                print(f"Not removing {nid} because of outer parents: {outer_parents}")
                del subtree[nid]
            else:
                self.tree.getroot().remove(n)
                children.update(subtree[nid])
                removed_nids.add(nid)
        self._remove_refs_to_nids(removed_nids)
        if children:
            self._rec_bf_remove_subtree(children, subtree)
    
    def remove_subtree(self, remove_root_nid: str):
        subtree = dict()
        self._rec_compute_subtree(remove_root_nid, subtree)
        # retain nodes with a parent outside the subtree
        # Important note: due to the possibility of specifying node relations with cycles,
        # some of the parents of a given node may appear after discovering the node
        # even with a Breadth-First Search; so we start with the entire subtree, then
        # retain only the nodes with no outer parent
        self._rec_bf_remove_subtree({remove_root_nid}, subtree, is_root=True)

    def remove_orphans(self):
        orphans = list()
        for t in ["UAObject", "UAVariable"]:
            for n in self._iterfind(self.tree, ".//uanodeset:" + t):
                for ref in self._iterfind(n, "uanodeset:References/uanodeset:Reference"):
                    if _is_hierarchical_ref(ref) and not is_forward(ref):
                        break
                else:
                    nid = n.get('NodeId')
                    orphans.append(nid)
        for nid in orphans:
            if nid.replace(' ', '') in ['i=84', 'i=78', 'i=11510', 'i=80', 'i=11508']:
                # retain Root, Mandatory (and its placeholder), Optional (and its placeholder)
                continue
            self.remove_subtree(nid)

    def __get_aliases(self):
        tree_aliases = self._find('uanodeset:Aliases')
        if tree_aliases is None:
            return {}
        return {alias.text: alias.get('Alias') for alias in tree_aliases}

    def __exists_ref(self, search: str, nids_or_aliases: set):
        for ref_node in self._iterfind(self.tree, search):
            ref_nid = ref_node.text.strip()
            if ref_nid in nids_or_aliases:
                return True
        return False

    def __exists_typedef_ref(self, search: str, nids_or_aliases: set, inst_decl_nids: set):
        inst_decl_refs = list()
        # search in the text for nid or alias
        # (text value belonging to a list cannot be expressed in the reduced XPath language of Python 3.10)
        for node in self._iterfind(self.tree, search):
            nid = node.get('NodeId')
            for type_ref in self._iterfind(node, "uanodeset:References/uanodeset:Reference[@ReferenceType='HasTypeDefinition']"):
                if type_ref.text.strip() in nids_or_aliases:
                    # this is a reference
                    if nid in inst_decl_nids:
                        # but an instance declaration to be removed if the type is removed
                        inst_decl_refs.append(nid)
                    else:
                        return True, []
        # no reference found, potentially instance declaration nodes only
        return False, inst_decl_refs

    def remove_unused(self, retain_ns0: bool, retain_types: set):
        aliases = self.__get_aliases()
        for ty, search, is_full_request in [('UAObjectType', ".//uanodeset:UAObject", False),
                   ('UAVariableType', ".//uanodeset:UAVariable", False),
                   ('UADataType', ".//uanodeset:UAVariable[@DataType='{}']", True),
                   ('UAReferenceType', ".//uanodeset:References/uanodeset:Reference[@ReferenceType='{}']", True)]:
            while True:
                # loop while the removed types produce unused types
                removed_nids = set()

                # identify the 'instance declaration' nodes, which are not considered as references
                inst_decl_nids = set()
                if not is_full_request:
                    # these instance declaration nodes are intended to be removed in case the corresponding type is removed
                    for node in self._iterfind(self.tree, search):
                        if self._find_in(node, "uanodeset:References/uanodeset:Reference[@ReferenceType='HasModellingRule']") is not None:
                            inst_decl_nids.add(node.get('NodeId'))

                for ty_node in self._iterfind(self.tree, f"uanodeset:{ty}"):
                    nid = ty_node.get('NodeId')
                    if (retain_ns0 and _is_ns0(nid)) or nid in retain_types:
                        # retain this type
                        continue
                    alias = aliases.get(nid)
                    refs = set([nid])
                    if alias is not None:
                        refs.add(alias)
                    subtype_search = f".//uanodeset:{ty}/uanodeset:References/uanodeset:Reference[@ReferenceType='HasSubtype'][@IsForward='false']"
                    if self.__exists_ref(subtype_search, refs):
                        # this type is subtyped
                        continue
                    if is_full_request:
                        found = False
                        for ref in refs:
                            req = search.format(ref)
                            if self._find(req) is not None:
                                # this type is used by data
                                found = True
                                break
                        if found:
                            continue
                    else:
                        found, inst_decls_to_remove = self.__exists_typedef_ref(search, refs, inst_decl_nids)
                        if found:
                            # this type is used by data
                            continue
                    #FIXME: when a removed object is referenced in a 'HasModellingRule', its removal changes the status
                    #of the referencing node: it is no more considered an instance declaration !
                    for p in inst_decls_to_remove:
                        self.remove_subtree(p)
                        removed_nids.add(p)
                    self.remove_subtree(nid)
                    removed_nids.add(nid)
                if len(removed_nids) == 0:
                    break

    def sanitize(self):
        """
        Returns True if the sanitation is a success.
        Otherwise there is an unrecoverable error which requires more user input (two nodes with the same nodeid, ...).
        """
        # Prepare the common structures for find and check for uniqueness
        nodes = {}
        error = False
        for node, nid in self._iter_nid_nodes():
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
        for node in self._iterfind(self.tree, './*[uanodeset:References]'):
            nids = node.get('NodeId')  # The starting node of the references below
            refs, = self._iterfind(node, 'uanodeset:References')
            for ref in list(refs):  # Make a list so that we can remove elements while iterating
                type_ref = ref.get('ReferenceType')
                nidt = ref.text.strip()  # The destination node of this reference
                if is_forward(ref):
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
                if self.verbose:
                    print('Sanitize: add forward reciprocal Reference {} -> {} (type {})'.format(a, b, t), file=sys.stderr)
                node = nodes[a]
                self._add_ref(node, t, b, is_forward=True)
    
        # Now add inverse refs b <- a for which a -> b exists
        for a, t, b in refs_fwd_list:
            if (a, t, b) in refs_inv:
                # Already defined
                continue
            if b not in nodes:
                print('Sanitize: Reference to unknown node, cannot add inverse reciprocal ({} -> {}, type {})'.format(a, b, t), file=sys.stderr)
            else:
                if self.verbose:
                    print('Sanitize: add inverse reciprocal Reference {} <- {} (type {})'.format(b, a, t), file=sys.stderr)
                node = nodes[b]
                self._add_ref(node, t, a, is_forward=False)
    
        # Note: ParentNodeId is an optional attribute. It refers to the parent node.
        #  In case the ParentNodeId is present, but the reference to the parent is not, the attribute is removed.
        # The reference to the ParentNodeId should be typed "HasComponent" (not verified)
        for node in self._iterfind(self.tree, './*[@ParentNodeId]'):
            # There may be no reference at all
            refs_nodes = self._findall(node, 'uanodeset:References')
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
                #refs.append(self._create_elem('Reference', {'ReferenceType': 'HasComponent', 'IsForward': 'false'}, text=pnid))
                # Note: the attrib member may be an interface, so this is not portable; however the ET lib does not provide other means to do this.
                del node.attrib['ParentNodeId']
    
        # Note: we don't check that the Address Space Model specified in Part 3 is valid.
    
        # TODO: Remove empty <References />
    
        return True

    def __fetch_subelement(self, elem, subtag) -> ET.Element:
        subelem = self._find_in(elem, subtag)
        if subelem is None:
            subelem = ET.SubElement(elem, subtag)
        return subelem

    def __merge_server_array(self, new: ET.ElementTree):
        # The UAVariable corresponding to the server array is required
        # <NamespaceURIs> is assumed to be filled (and merged if needed) in the given tree
        tree_server_array = self._find(".//uanodeset:UAVariable[@NodeId='i=2254'][@BrowseName='ServerArray']")
        if tree_server_array is None:
            raise Exception("Missing UAVariable ServerArray (i=2254) in NS0")
        tree_value = self.__fetch_subelement(tree_server_array, f'{{{UA_NODESET_URI}}}Value')
        tree_l_str = self.__fetch_subelement(tree_value, f'{{{UA_TYPES_URI}}}ListOfString')
        tree_uris = [uri.text for uri in self._findall(tree_l_str, f'{{{UA_TYPES_URI}}}String')]
        new_uris = [uri.text for uri in self._findall(new, f"uanodeset:UAVariable[@NodeId='i=2254']/uanodeset:Value/{{{UA_TYPES_URI}}}ListOfString/{{{UA_TYPES_URI}}}String")]
        ns1_uri = self._find('uanodeset:NamespaceUris/uanodeset:Uri')
        if ns1_uri is None:
            # Nothing to merge, ns0 extension
            return
        if len(tree_uris) > 0 and tree_uris[0] != ns1_uri.text:
            raise Exception(f"Invalid local server URI in node id 2254: {tree_uris[0]}, expecting {ns1_uri} instead")
        new_uris.insert(0, ns1_uri.text)
        set_new_uris = set(new_uris) - set(tree_uris)
        unique_new_uris = list()
        for uri in new_uris:
            if uri in set_new_uris and uri not in unique_new_uris:
                unique_new_uris.append(uri)
        append_strings(tree_l_str, unique_new_uris)

    def __fill_namespace_array(self):
        # The UAVariable corresponding to the namespace array is required
        # <NamespaceURIs> is assumed to be filled (and merged if needed) in the given tree
        namespace_array_node = self._find(".//uanodeset:UAVariable[@NodeId='i=2255'][@BrowseName='NamespaceArray']")
        if namespace_array_node is None:
            raise Exception("Missing UAVariable NamespaceArray (i=2255) in NS0")
        value = self.__fetch_subelement(namespace_array_node, f'{{{UA_NODESET_URI}}}Value')
        l_str = self.__fetch_subelement(value, f'{{{UA_TYPES_URI}}}ListOfString')
        l_str.clear()
        ns_uris = self._findall(self.tree, 'uanodeset:NamespaceUris/uanodeset:Uri')
        append_strings(l_str, [UA_URI] + [uri.text for uri in ns_uris])

    def __get_all_retain_values(self, retain: set):
        nid_alias = self.__get_aliases()
        alias_nid =  {alias: nid for nid, alias in nid_alias.items()}
        corresp = set() # the corresponding alias or node id for every item in the retain set
        for r in retain:
            if r in nid_alias:
                corresp.add(nid_alias[r])
            if r in alias_nid:
                corresp.add(alias_nid[r])
        return retain | corresp

    def remove_backward_refs(self, retain: set):
        # HasSubtype backward refs should be kept since it might be necessary for type resolution. 
        retain |= {'HasSubtype'}
        all_retain = self.__get_all_retain_values(retain)
        for node, _ in self._iter_nid_nodes():
            for refs in self._iterfind(node, 'uanodeset:References'):
                back_refs = list()
                for ref in refs:
                    # oddly enough, filtering with a match expression fails
                    # (some back refs are not found, although identical to some that are found)
                    if not is_forward(ref) and ref.get('ReferenceType') not in all_retain:
                        # cannot remove while iterating
                        back_refs.append(ref)
                for ref in back_refs:
                    refs.remove(ref)

    def write_tree(self, file):
        ET.indent(self.tree)
        self.tree.write(file, encoding="utf-8", xml_declaration=True)


def run_merge(args):
    # check option compatibility and raise error
    sanitize = args.sanitize or args.remove_subtrees or args.remove_orphans or args.remove_unused or args.remove_backward_refs
    if not args.sanitize and sanitize:
        raise Exception("sanitization is required when removing a subtree, unused types or backward references")

    merger = NodesetMerger(args.verbose)
    for fname in args.fns_adds:
        source = fname if fname != '-' else sys.stdin
        res = merger.merge(source)
        if not res:
            print(f'Merge abandoned due to error with {source}')
            return

    # Apply options afterwards
    if args.remove_max_monit:
        merger.remove_max_monit()

    if args.remove_methods:
        merger.remove_methods()

    if args.remove_max_node_mgt:
        merger.remove_max_node_mgt()

    if args.remove_node_ids_gt > 0:
        merger.remove_node_ids_greater_than(args.remove_node_ids_gt)

    if sanitize:
        res = merger.sanitize()
    else:
        res = True

    if res and args.remove_subtrees:
        for node in args.remove_subtrees:
            merger.remove_subtree(node)

    if res and (args.remove_orphans or args.remove_unused):
        merger.remove_orphans()

    if res and args.remove_unused:
        merger.remove_unused(args.retain_ns0, frozenset(args.retain_types))

    if res and sanitize and args.remove_backward_refs:
        merger.remove_backward_refs(set(args.retain_nodes))

    if res:
        merger.write_tree(args.fn_out or sys.stdout.buffer)
    else:
        print('There was some unrecoverable error{}'
              .format(', did not save to {}'.format(args.fn_out) if args.fn_out else ''),
              file=sys.stderr)


def make_argparser():
    parser = argparse.ArgumentParser(description='A tool to merge (and more) XMLs of OPC UA Address Spaces.')
    parser.add_argument('fns_adds', nargs='+', metavar='XML',
                        help='''
            Path (or - for stdin) the address spaces to merge. In case of conflicting elements,
            the element from the first address space in the argument order is kept.
            The models must be for the same OPC UA version (e.g. 1.03).
            The first address space shall contain the OPC UA namespace NS0.
            The following address spaces, if any, will be the namespaces 1 to N.
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
    parser.add_argument('--remove-max-node-management', action='store_true', dest='remove_max_node_mgt',
                        help='Remove the MaxNodesPerNodeManagement node and references to it')
    parser.add_argument('--remove-node-ids-greater-than', default=0, type=int, dest='remove_node_ids_gt',
                        help='Remove the nodes of NS 0 with a NodeId greater than the given value')
    parser.add_argument('--remove-subtrees', default=[], nargs='+', dest='remove_subtrees',
                        help='''
                        For each given nodeId, remove the node along with all its descendants,
                        except for those with another ancestry.
                        Note: HasEncoding is assimilated as a hierarchical link (target node existence depends on source node).
                        This  option forces the creation of reciprocal references (sanitize).
                        ''')
    parser.add_argument('--no-sanitize', action='store_false', dest='sanitize',
                        help='''
            Suppress the normal behavior which is to sanitize the model after merge/additions/removal.
            The normal behavior is to check for duplicate nodes,
            generate reciprocal references between nodes when there is a reference in only one direction,
            and remove attribute ParentNodeId when erroneous.
                             ''')
    parser.add_argument('--verbose', '-v', action='store_true',
                        help='Display information (reciprocal references added, merged nodes, removed nodes, etc.)')
    rm_unused = parser.add_argument_group('Remove unused nodes')
    rm_unused.add_argument('--remove-orphans', action='store_true', dest='remove_orphans',
                        help='''
                        Remove all orphan nodes, i.e. nodes with no hierarchical parent, except for Root node "i=84".
                        Note: HasEncoding is assimilated as a hierarchical link (target node existence depends on source node).
                        This option forces the creation of reciprocal references (sanitize).
                        ''')
    rm_unused.add_argument('--remove-unused', action='store_true', dest='remove_unused',
                        help='''
                        Remove all of the type definitions which are not used by the model, as well as unused nodes. 
                        This  option forces the creation of reciprocal references (sanitize).
                        This option starts with the removal of orphan nodes (see option --remove-orphans),
                        then removes the unused type definitions.
                        To retain some of the type definitions, see options --retain-ns0 and --retain-types.
                        ''')
    rm_unused.add_argument('--retain-ns0', action='store_true', dest='retain_ns0',
                        help='''
                        Retain all NS0 types.
                        ''')
    rm_unused.add_argument('--retain-types', nargs='+', metavar='TYPE', default=[], dest='retain_types',
                        help='''
                        Retain the types given by their NS and NodeID
                        ''')
    rm_back_refs = parser.add_argument_group('Remove backward references')
    rm_back_refs.add_argument('--remove-backward-refs', action='store_true', dest='remove_backward_refs',
                        help='''
                        Remove the backward references, i.e. reference elements with @IsForward="false".
                        Retain only the "HasSubtype" backward references, necessary for type resolution.
                        This option requires a sanitized model, else information would be lost.
                        Thus, it is not compatible with --no-sanitize.
                        ''')
    rm_back_refs.add_argument('--retain-nodes', nargs='+', metavar='ID_OR_ALIAS', dest='retain_nodes',
                        default=[],
                        help='''
                        When removing the backward references, retain those with the given ReferenceType.
                        The reference types may be given either as node ID or alias.
                        ''')

    return parser


if __name__ == '__main__':
    parser = make_argparser()
    args = parser.parse_args()
    # Check that '-' is provided only once in input address spaces
    assert args.fns_adds.count('-') < 2, 'You can only take a single XML from the standard input'

    run_merge(args)

