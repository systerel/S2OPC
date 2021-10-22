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
The `pys2opc` module is a Python3 wrapper of the client library `s2opc_clientwrapper`, based on `s2opc`.
It uses CFFI to bind Python with C.
The wrapper provides Python classes for OPC UA concepts.

The `pys2opc.s2opc.PyS2OPC` represents the `SOPC_Toolkit*_*` API and gathers its top level functionalities,
which are split in `pys2opc.s2opc.PyS2OPC_Client` and `pys2opc.s2opc.PyS2OPC_Server`.

For now, the module can be used exclusively either as a Server or one or more Clients.


### Client use

Once a configuration is created and the toolkit is `pys2opc.s2opc.PyS2OPC.mark_configured`,
new connections are created with `pys2opc.s2opc.PyS2OPC_Client.connect`.
Connection objects are instances of the `pys2opc.connection.BaseClientConnectionHandler`.

With connections, you can `pys2opc.connection.BaseClientConnectionHandler.read_nodes`,
`pys2opc.connection.BaseClientConnectionHandler.write_nodes` and `pys2opc.connection.BaseClientConnectionHandler.browse_nodes`.
You can also `pys2opc.connection.BaseClientConnectionHandler.add_nodes_to_subscription`,
and receive notifications through `pys2opc.connection.BaseClientConnectionHandler.on_datachanged`.

>>> from pys2opc import PyS2OPC_Client as PyS2OPC
>>> PyS2OPC.get_version()
>>> with PyS2OPC.initialize():
>>>     config = PyS2OPC.add_configuration_unsecured()
>>>     PyS2OPC.mark_configured()
>>>     with PyS2OPC.connect(config, BaseClientConnectionHandler) as connection:
>>>         connection.read_nodes()


### Server use

Servers are mainly configured by XML files: one for the structure and content of the address space,
the other for the endpoint configuration, which specifies who can connect, and which security keys are used.
The configuration is also the place to register callbacks
(see `pys2opc.server_callbacks.BaseAddressSpaceHandler`).

There are two main ways to start the server once configured.
An "all-in-one" `pys2opc.s2opc.PyS2OPC_Server.serve_forever`,
and a more permissive one using a `with` statement `pys2opc.s2opc.PyS2OPC_Server.serve`.
In the `with` statement, the application code can be started alongside the S2OPC server.

>>> from pys2opc import PyS2OPC_Server as PyS2OPC
>>> PyS2OPC.get_version()
>>> with PyS2OPC.initialize():
>>>     PyS2OPC.load_address_space('address_space.xml')
>>>     PyS2OPC.load_configuration('server_configuration.xml')
>>>     PyS2OPC.mark_configured()
>>>     with PyS2OPC.serve():
>>>         # The main loop of the application
>>>         while PyS2OPC.serving(): pass


### NodeId concept

Throughout the module (e.g. `pys2opc.connection.BaseClientConnectionHandler.read_nodes`),
when the interface requires a NodeId, the following syntax is used:

>>> node_id = 'ns=1;x=qualifier'

The namespace part is optional (`ns=1;`), and its value is `0` by default.
The namespace is a 16-bits unsigned integer.
The NodeId type `x=` is either:

- `i=` followed by a 32-bits unsigned integer,
- `s=` followed by a zero-terminated string,
- `g=` followed by an OPC UA GUID (16 bytes hex string, e.g. "72962B91-FA75-4ae6-8D28-B404DC7DAF63"),
- `b=` followed by a bytestring.


### Types in OPC UA

All values in OPC UA are typed.
You cannot write a value of a variable if you don't send the correctly typed value to write.
Mostly used types are signed or unsigned integers of various sizes (8, 16, 32, 64 bits), single or double precision floating point values,
and string or bytes (bytes may contain \\0 values).

Also, arrays are handled at the value level, and values can be array of integers (or other types).

However, values in Python are more flexible.
Integers are unbound, floating point are always double, arrays may contain different types of values.
The class `pys2opc.types.Variant` represent objects that have the properties of python values (e.g. addition/difference/multiplication/division of numbers),
but the types of OPC UA values (see `pys2opc.types.VariantType`).

When creating `Variant`, there is no need to specify its type.
It can be changed later, and will be checked only when encoding the value.

DataValue is another OPC UA concept that encapsulates `Variant` to add timestamps and a quality (see `pys2opc.types.DataValue`).
It is DataValues that are used to store values (when reading or writing values).
There are helpers to convert Python values to OPC UA values (`pys2opc.types.DataValue.from_python`).
The other way around is simpler (just use the `pys2opc.types.DataValue.variant` as if it was a Python value).

When writing nodes to a server, PyS2OPC is able to compute the type of a Variant by making a read beforehand
(see `pys2opc.connection.BaseClientConnectionHandler.write_nodes`) and deducing the right types.
Once the types are deduced, values may be modified and they will keep their type, so that it is not necessary to make the deduction again.
"""

from _pys2opc import ffi, lib as libsub
from .s2opc import VERSION, PyS2OPC_Client, PyS2OPC_Server, ClientConfiguration, ServerConfiguration, BaseAddressSpaceHandler
from .connection import BaseClientConnectionHandler
from .types import Variant, VariantType, DataValue, AttributeId, ReturnStatus, StatusCode, SecurityPolicy, SecurityMode, NodeClass, LogLevel
from .request import Request

#del ffi, libsub  # This makes pdoc bug
