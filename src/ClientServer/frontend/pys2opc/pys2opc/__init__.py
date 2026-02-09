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
The pys2opc package is a Python3 wrapper of the library `s2opc_clientserver`, based on S2OPC.
It uses Cython to bind Python with C.
The wrapper provides Python classes for OPC UA concepts.

The `pys2opc.PyS2OPC` represents the `SOPC_CommonHelper_*` API and gathers its top level functionalities,
which are split in `pys2opc.PyS2OPC_Client` represents the `SOPC_Client*Helper*`,
and `pys2opc.PyS2OPC_Server` represents the `SOPC_Server*Helper*`.

The module can be used simultaneously as a server and as a client, both with several connection configurations.


### Client use

First initialize the client with `PyS2OPC_Client.initialize`.
Then, configure it by passing an XML configuration file to `PyS2OPC_Client.load_client_configuration_from_file`.
For more details on XML configuration file, see [s2opc_clientserver_config.xsd](https://gitlab.com/systerel/S2OPC/-/blob/master/schemas/s2opc_clientserver_config.xsd?ref_type=heads)
Finally, the client can establish a connection to a server using the (`PyS2OPC_Client.connect`) function by selecting the user to be used in the previous XML file.

With connection, you can `pys2opc.BaseClientConnectionHandler.read_nodes`,
`pys2opc.BaseClientConnectionHandler.write_nodes` and `pys2opc.BaseClientConnectionHandler.browse_nodes`.
You can also `pys2opc.BaseClientConnectionHandler.add_nodes_to_subscription`,
and receive notifications through `pys2opc.BaseClientConnectionHandler.on_datachanged`.

>>> from pys2opc import PyS2OPC_Client
>>> PyS2OPC_Client.get_version()
>>> with PyS2OPC_Client.initialize():
>>>     configs = PyS2OPC_Client.load_client_configuration_from_file('client_config.xml')
>>>     with PyS2OPC_Client.connect(configs["read"]) as connection:
>>>         ReadResponse = connection.read_nodes(nodeIds=NODES_TO_READ)

*client_config.xml* extract: connection example with id = "read"
```
<Connection serverURL="opc.tcp://localhost:4841" id="read">
  <ServerCertificate path="server_public/server_4k_cert.der"/>
  <SecurityPolicy uri="http://opcfoundation.org/UA/SecurityPolicy#Basic256Sha256"/>
  <SecurityMode mode="Sign"/>
</Connection>
```

### Server use

Server is mainly configured by XML files: the first for the server endpoints configuration,
which specifies security parameter, session and users parameters,
the next file is the NodeSet describing the structure and content of the address space,
the last file to define allowed users and their authorization (read, write, ..).
The configuration function might also register callback to be notified on server (see `BaseAddressSpaceHandler`).

For more details on XML server endpoint configuration file, see [s2opc_clientserver_config.xsd](https://gitlab.com/systerel/S2OPC/-/blob/master/schemas/s2opc_clientserver_config.xsd?ref_type=heads).
For more details on XML address space configuration file, see [UANodeSet.xsd](https://github.com/OPCFoundation/UA-Nodeset/blob/v1.04/Schema/UANodeSet.xsd).
For more details on XML user authorization file, see [s2opc_clientserver_users_config.xsd](https://gitlab.com/systerel/S2OPC/-/blob/master/schemas/s2opc_clientserver_users_config.xsd?ref_type=heads).

There are two main ways to start the server once configured.
An "all-in-one" `PyS2OPC_Server.serve_forever`,
and a more permissive one using a `with` statement `PyS2OPC_Server.serve`.
In the `with` statement, the application code can be started alongside the S2OPC server.

>>> from pys2opc import PyS2OPC_Server
>>> PyS2OPC_Server.get_version()
>>> with PyS2OPC_Server.initialize():
>>>     PyS2OPC.load_server_configuration_from_files('server_config.xml', 'address_space.xml', 'user_config.xml')
>>>     with PyS2OPC_Server.serve():
>>>         # The main loop of the application

*server_config.xml* extract: endpoint configuration example
```
  <Endpoint url="opc.tcp://localhost:4841">
    <SecurityPolicies>
      <SecurityPolicy uri="http://opcfoundation.org/UA/SecurityPolicy#Basic256Sha256">
        <SecurityModes>
          <SecurityMode mode="Sign"/>
        </SecurityModes>
        <UserPolicies>
          <UserPolicy policyId="username_Basic256Sha256" tokenType="username" securityUri="http://opcfoundation.org/UA/SecurityPolicy#Basic256Sha256"/>
        </UserPolicies>
      </SecurityPolicy>
  </Endpoint>
```

*address_space.xml* extract: variable definition example
```
  <UAVariable AccessLevel="99" BrowseName="1:LK" DataType="Boolean" NodeId="ns=1;s=BALA_RDLS_W1.RM.LK">
    <DisplayName>LK</DisplayName>
    <Description>Switch Locally Locked</Description>
    <References>
      <Reference IsForward="false" ReferenceType="HasComponent">ns=1;s=BALA_RDLS_W1.RM</Reference>
      <Reference ReferenceType="HasTypeDefinition">i=63</Reference>
    </References>
    <Value>
      <uax:Boolean>false</uax:Boolean>
    </Value>
  </UAVariable>
```

*user_config.xml* extract: user authorization configuration example
```
  <UserPasswordConfiguration hash_iteration_count="10000" hash_length="32" salt_length="16">
    <!-- "me" pwd=1234 has all right accesses. -->
    <UserPassword user="me" hash="847d892ffaccb9822d417866f9d491389b29134b3c73c3a429ac95c627f9d40a" salt="17faf802f81c2503d3043042e79004b4">
      <UserAuthorization write="true" read="true" execute="true" addnode="true" receive_events="true" deletenode="true"/>
    </UserPassword>
  </UserPasswordConfiguration>
```

### NodeId concept

Throughout the module (e.g. `pys2opc.BaseClientConnectionHandler.read_nodes`),
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
The class `pys2opc.Variant` represent objects that have the properties of python values (e.g. addition/difference/multiplication/division of numbers),
but the types of OPC UA values (see `pys2opc.SOPC_BuiltinId`).

When creating `Variant`, there is no need to specify its type.
It can be changed later, and will be checked only when encoding the value.

DataValue is another OPC UA concept that encapsulates `Variant` to add timestamps and a quality (see `pys2opc.DataValue`).
It is DataValues that are used to store values (when reading or writing values).
There are helpers to convert Python values to OPC UA values (`pys2opc.DataValue.from_python`).
The other way around is simpler (just use the `pys2opc.DataValue` variant field as if it was a Python value).
"""

import pys2opc

from .pys2opc import *
