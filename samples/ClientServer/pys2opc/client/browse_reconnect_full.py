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


"""
Example script: this script recursively browse the address space from the root and prints it.
Skips Types and Views subtrees. Skip inverse references.
Only displays the following reference types: "Organizes", "HasComponent", and "HasProperty".
"""


from collections import Counter
import random
import argparse
import re
import pickle

from pys2opc import PyS2OPC_Client as PyS2OPC, BaseClientConnectionHandler, AttributeId, NodeClass
from _connection_configuration import configuration_parameters_no_subscription
from utils import ReconnectingContext


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Configurable and resilient browse.')
    parser.add_argument('-d', '--depth', default=-1, type=int,
                        help='Max browse depth, in number of references from Root (Objects is 1).')
    parser.add_argument('-a', '--analyze', action='store_true',
                        help='Analyze browsed names (which analyze is done is not configurable, see the source).')
    parser.add_argument('root_nodes', nargs='*', default=['i=84'],
                        help='Browse from these nodes instead of the Root nodes.')
    args = parser.parse_args()

    with PyS2OPC.initialize():
        config = PyS2OPC.add_configuration_unsecured(**configuration_parameters_no_subscription)
        PyS2OPC.mark_configured()
        # The tree structure: stores explored nodes
        dNodes = {}  # {parent_node_id: [sub_node_id for each sub_node]}
        # The browse names are stored in a different structure
        dNames = {}  # {node_id: browse_name}
        # Adds nodes we don't want to explore.
        dNodes['i=86'] = []  # Types
        dNodes['i=87'] = []  # Views
        # The structure to store known-but-yet-to-be-explored nodes
        sCandidates = {node for node in args.root_nodes}
        dDubious = Counter()  # One of these states failed, but we don't know which one. They are still in sCandidates. Stores the number of times each was found in a failed request.
        dCls = {}  # Also stores the node class in a dict, which tells that maybe we should have created a struct for each node and store these instances in a single dict

        # Reconnecting browse
        nMaxRetry = 5
        nRetry = nMaxRetry
        connection = None
        nErrs = 0

        # Depth counter
        # It is used for the depth-limited exploration, not for display.
        dDepth = {node:0 for node in args.root_nodes}

        def add_response(lBwseNodes, lBwseResults):
            # Put the browsed node face to its browse result
            for a, bwsr in zip(lBwseNodes, lBwseResults):
                sCandidates.remove(a)
                assert a not in dNodes
                dNodes[a] = []
                # The browse result contains the list of references
                for ref in bwsr.references:
                    if not ref.isForward:
                        continue
                    if ref.referenceTypeId not in ('i=47',  # HasComponent
                                                   'i=46',  # HasProperty
                                                   'i=35'):  # Organizes
                        continue
                    b = ref.nodeId
                    dNodes[a].append(b)
                    dDepth[b] = dDepth[a]+1
                    if b not in dNames:
                        dNames[b] = ref.browseName[1]
                    # Add unexplored states to future candidates
                    if b not in dNodes and (args.depth < 0 or dDepth[b] <= args.depth):
                        sCandidates.add(b)
                    # Add node class
                    if b not in dCls:
                        dCls[b] = ref.nodeClass
                    # Compute node depth

        context = ReconnectingContext(config, BaseClientConnectionHandler)
        while sCandidates and nRetry:
            connection = context.get_connection()
            lToBrowse = list(random.sample(sCandidates, min(len(sCandidates), [3, 6, 12, 25, 50][nRetry-1])))  # Make a request of max 50 nodes
            # Make the request
            respBrowse = connection.browse_nodes(lToBrowse, maxReferencesPerNode=100)
            # Analyze the response
            if respBrowse is not None:
                print('.', end='', flush=True)
                nRetry = nMaxRetry
                add_response(lToBrowse, respBrowse.results)
            else:
                print(nRetry, end='', flush=True)
                nRetry -= 1
                nErrs += 1
                for a in lToBrowse:
                    dDubious[a] += 1
        if dDubious:
            print('\nMost dubious nodes:')
        else:
            print('\nNo dubious nodes.')
        for k,v in sorted(dDubious.items(), reverse=True):
            print('-', k, v)

        # # Finds missing names (mostly, the root node)
        # lToName = [node for node in dNodes if node not in dNames]
        # print(connection.connected)
        # respRead = connection.read_nodes(lToName, [AttributeId.BrowseName]*len(lToName))
        # print('x')
        # for node, dvName in zip(lToName, respRead.results):
        #     dNames[node] = dvName.variant[1]

        # Saves the content to somewhere
        pickle.dump({'dCls': dCls, 'dNodes': dNodes, 'dNames': dNames}, open('browse_dump', 'wb'))

        if args.analyze:
            patterns = Counter()
            lUnmatched = []
            def whole_match(s):
                match = re.match(r'[a-zA-Z0-9_]+', s)
                return match is not None and match.group(0) == s
            for node in dCls:
                bMatch = False
                if node.startswith('ns=4;s=F:Path:') and dCls[node] == NodeClass.Object:
                    for word in node[len('ns=4;s=F:Path:'):].split('//'):
                        if not whole_match(word):
                            break
                    else:
                        bMatch = True
                        patterns['Obj'] += 1
                elif node.startswith('ns=4;s=') and dCls[node] == NodeClass.Variable:
                    for word in node[len('ns=4;s='):].split('.'):
                        if not whole_match(word):
                            break
                    else:
                        bMatch = True
                        patterns['Var'] += 1
                if node.startswith('ns=4;') and not bMatch:
                    lUnmatched.append(node)
            if lUnmatched:
                print('Unmatched nodeid patterns:')
                for node in lUnmatched:
                    print('-', node)
            else:
                print('All nodeid were matched by a pattern')
            print('{} Objects with pattern F:Path:Obj//Obj, {} Variables with pattern Obj.Obj.Var\n'.format(patterns['Obj'], patterns['Var']))

    def print_recurs(node, iIndent=0, sPrinted=set()):
        global dNodes, dNames
        """
        Prints subnodes indented, and keep in memory which nodes were printed, hence avoiding loops.
        """
        print('  ' * iIndent, end='')
        if node in dCls:
            print('{}: '.format(NodeClass.get_name_from_id(dCls[node])[:3]), end='')
        if node in dNames:  # and node in dNodes:
            print(dNames[node] + ('   ({})'.format(node)))  # if not dNodes[node] else ''
        else:
            print(node)
        for subnode in dNodes.get(node, []):
            print_recurs(subnode, iIndent+1, sPrinted | {node})

    nBrowsed = sum(1 for v in dNodes.values() for _ in v)
    if nBrowsed < 10000:
        for node in args.root_nodes:
            print_recurs(node)
    print('\nNumber of browsed items: {}\nNumber of failed browse requests: {}'.format(nBrowsed, nErrs))
