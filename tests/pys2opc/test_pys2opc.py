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


from pys2opc import PyS2OPC, BaseConnectionHandler, DataValue
import time


class PrintSubs(BaseConnectionHandler):
    def on_datachanged(self, nodeId, dataValue):
        print('Data changed "{}" -> {}'.format(nodeId, dataValue.variant))


if __name__ == '__main__':
    print(PyS2OPC.get_version())
    print()

    with PyS2OPC.initialize():
        config = PyS2OPC.add_configuration_unsecured()
        PyS2OPC.configured()
        with PyS2OPC.connect(config, PrintSubs) as connection:
            #connection.add_nodes_to_subscription(['s=Counter', 's=StatusString', 'i=2255'])
            #print(connection.read_nodes(['s=Counter', 's=StatusString', 'i=2255']).results)
            if connection.write_nodes(['s=StatusString'], list(map(DataValue.from_python, ['Everything is Foobar.']))).is_ok():
                print('Write Ok')
            time.sleep(2)
