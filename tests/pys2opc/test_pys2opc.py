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


from pys2opc import PyS2OPC, BaseConnectionHandler, DataValue, Variant, AttributeId
import time


class PrintSubs(BaseConnectionHandler):
    def on_datachanged(self, nodeId, dataValue):
        print('Data changed "{}" -> {}'.format(nodeId, dataValue.variant))

def read_browsenames(connection, nids):
    response = connection.read_nodes(nids, [AttributeId.BrowseName]*len(nids))
    for dv in response.results:
        yield dv.variant[1] if dv.variant != Variant(None) else '(no browse name)'

if __name__ == '__main__':
    print(PyS2OPC.get_version())
    print()

    with PyS2OPC.initialize():
        config = PyS2OPC.add_configuration_unsecured()
        PyS2OPC.configured()
        with PyS2OPC.connect(config, PrintSubs) as connection:
            nids = ['i=84', 's=Counter', 's=StatusString', 'i=2255']
            #connection.add_nodes_to_subscription(nids)
            #print(connection.read_nodes(nids).results)
            #if connection.write_nodes(['s=StatusString'], list(map(DataValue.from_python, ['Everything is Foobar.']))).is_ok():
            #    print('Write Ok')
            response = connection.browse_nodes(nids)
            browseNames = read_browsenames(connection, nids)
            for a, bwsr in zip(browseNames, response.results):
                sA = '  {}'.format(a)
                print(sA, end='' if bwsr.references else '\n')
                print((' '*len(sA)).join((' -> ' if ref.isForward else ' <- ') + ref.browseName[1] + '\n' for ref in bwsr.references), end='')
            time.sleep(2)
