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
The `pys2opc` module is a Python3 wrapper of the client library `client_subscription`, based on `s2opc`.
It uses CFFI to bind Python and C.
The wrapper provides Python classes for OPC UA concepts.

The `pys2opc.s2opc.PyS2OPC` represents the `SOPC_Toolkit` and gathers its top level functionalities
(see `pys2opc.s2opc.PyS2OPC.initialize`, `pys2opc.s2opc.PyS2OPC.add_configuration_unsecured`, ...).

Once a configuration is created and the toolkit is `pys2opc.s2opc.PyS2OPC.mark_configured`,
new connections are created with `pys2opc.s2opc.PyS2OPC.connect`.
Connection objects are instances of the `pys2opc.connection.BaseConnectionHandler`.

With connections, you can `pys2opc.connection.BaseConnectionHandler.read_nodes`,
`pys2opc.connection.BaseConnectionHandler.write_nodes` and `pys2opc.connection.BaseConnectionHandler.browse_nodes`.
You can also `pys2opc.connection.BaseConnectionHandler.add_nodes_to_subscription`,
and receive notifications through `pys2opc.connection.BaseConnectionHandler.on_datachanged`.

>>> PyS2OPC.get_version()
>>> with PyS2OPC.initialize():
>>>     config = PyS2OPC.add_configuration_unsecured()
>>>     PyS2OPC.mark_configured()
>>>     with PyS2OPC.connect(config, BaseConnectionHandler) as connection:
>>>         connection.read_nodes()


## NodeId concept

Throughout the module (e.g. `pys2opc.connection.BaseConnectionHandler.read_nodes`),
when the interface requires a NodeId, the following syntax is used:

>>> node_id = 'ns=1;x=qualifier'

The namespace part is optional (`ns=1;`), and its value is `0` by default.
The namespace is a 16-bits unsigned integer.
The NodeId type `x=` is either:

- `i=` followed by a 32-bits unsigned integer,
- `s=` followed by a zero-terminated string,
- `g=` followed by an OPC UA GUID (16 bytes hex string, e.g. "72962B91-FA75-4ae6-8D28-B404DC7DAF63"),
- `b=` followed by a bytestring.

"""

from .s2opc import PyS2OPC, ClientConfiguration, VERSION
from .connection import BaseConnectionHandler
from .types import Request, Variant, VariantType, DataValue, AttributeId, ReturnStatus, StatusCode, SecurityPolicy, SecurityMode, NodeClass

