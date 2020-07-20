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


from pys2opc import PyS2OPC_Client as PyS2OPC, BaseClientConnectionHandler, AttributeId
from _connection_configuration import configuration_parameters_no_subscription


ROOT_ID = 'i=84'

if __name__ == '__main__':
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
        sCandidates = {ROOT_ID}  # The Root node
        with PyS2OPC.connect(config, BaseClientConnectionHandler) as connection:
            try:
                while sCandidates:
                    lToBrowse = list(sCandidates)[:50]  # Make a request of max 50 nodes
                    # Make the request
                    print('.', end='', flush=True)
                    respBrowse = connection.browse_nodes(lToBrowse, maxReferencesPerNode=100)
                    # Put the browsed node face to its browse result
                    for a, bwsr in zip(lToBrowse, respBrowse.results):
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
                            if b not in dNames:
                                dNames[b] = ref.browseName[1]
                            # Add unexplored states to future candidates
                            if b not in dNodes:
                                sCandidates.add(b)
                # Finds missing names (mostly, the root node)
                lToName = [node for node in dNodes if node not in dNames]
                respRead = connection.read_nodes(lToName, [AttributeId.BrowseName]*len(lToName))
                for node, dvName in zip(lToName, respRead.results):
                    dNames[node] = dvName.variant[1]
            except:
                print('Connection failed, printing partial Address Space')

    def print_recurs(node, iIndent=0, sPrinted=set()):
        global dNodes, dNames
        """
        Prints subnodes indented, and keep in memory which nodes were printed, hence avoiding loops.
        """
        if node in dNames and node in dNodes:
            print('  '*iIndent + dNames[node] + ('   ({})'.format(node) if not dNodes[node] else ''))
        else:
            print('  '*iIndent + node)
        for subnode in dNodes.get(node, []):
            print_recurs(subnode, iIndent+1, sPrinted | {node})

    print_recurs(ROOT_ID)
    print('\nNombre d\'items affiches: {}'.format(sum(1 for v in dNodes.values() for _ in v)))
