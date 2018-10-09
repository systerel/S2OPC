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


import pys2opc
import time

@pys2opc.ffi.def_extern()
def log_libsub(level, text):
    print(pys2opc.ffi.string(text))

@pys2opc.ffi.def_extern()
def disconnected(client_id):
    print('Disconnected:', client_id)

@pys2opc.ffi.def_extern()
def datachanged(connection_id, data_id, c_value):
    value = LibSub_Value(c_value)
    print('Data changed (connection {}, data_id {}), value {}'.format(connection_id, data_id, value.value))

@pys2opc.ffi.def_extern()
def generic_event(connection_id, event, status, response_struct, response_context):
    print('Event for request', response_context, 'received')

class LibSub_Value:
    def __init__(self, c_value):
        if c_value.type == pys2opc.lib.SOPC_LibSub_DataType_bool or c_value.type == pys2opc.lib.SOPC_LibSub_DataType_integer:
            self.value = pys2opc.ffi.cast('int64_t *', c_value.value)[0]
        elif c_value.type == pys2opc.lib.SOPC_LibSub_DataType_string or c_value.type == pys2opc.lib.SOPC_LibSub_DataType_bytestring:
            sopc_string = pys2opc.lib.cast('SOPC_String *', c_value.value)  # ByteString is also a string in SOPC
            self.value = pys2opc.ffi.buffer(sopc_string.Data, sopc_string.Length)[:]
        else:
            raise ValueError('Unknown value type')
        self.quality = c_value.quality
        self.source_timestamp = c_value.source_timestamp
        self.server_timestamp = c_value.server_timestamp


if __name__ == '__main__':
    print(pys2opc.ffi.string(pys2opc.lib.SOPC_LibSub_GetVersion()))

    # The LibSub API let the caller forget structures after passing them to the functions.
    # TODO: assert everything is STATUS_OK
    pys2opc.lib.SOPC_LibSub_Initialize([(pys2opc.lib.log_libsub, pys2opc.lib.disconnected)])

    p_cfgId = pys2opc.ffi.new('SOPC_LibSub_ConfigurationId *')
    NULL = pys2opc.ffi.NULL
    pys2opc.lib.SOPC_LibSub_ConfigureConnection([{'server_url': pys2opc.ffi.new('char[]', b"opc.tcp://localhost:4841"),
                                                   'security_policy': pys2opc.ffi.new('char[]', b"http://opcfoundation.org/UA/SecurityPolicy#None"),
                                                   'security_mode': pys2opc.lib.OpcUa_MessageSecurityMode_None,
                                                   'disable_certificate_verification': True,
                                                   'path_cert_auth': NULL,
                                                   'path_cert_srv': NULL,
                                                   'path_cert_cli': NULL,
                                                   'path_key_cli': NULL,
                                                   'path_crl': NULL,
                                                   'policyId': pys2opc.ffi.new('char[]', b"anonymous"),
                                                   'username': NULL,
                                                   'password': NULL,
                                                   'publish_period_ms': 500,
                                                   'n_max_keepalive': 3,
                                                   'n_max_lifetime': 10,
                                                   'data_change_callback': pys2opc.lib.datachanged,
                                                   'timeout_ms': 10000,
                                                   'sc_lifetime': 3600000,
                                                   'token_target': 3,
                                                   'generic_response_callback': pys2opc.lib.generic_event,
                                                   }],
                                                 p_cfgId)
    pys2opc.lib.SOPC_LibSub_Configured()

    p_conId = pys2opc.ffi.new('SOPC_LibSub_ConnectionId *')
    pys2opc.lib.SOPC_LibSub_Connect(p_cfgId[0], p_conId)

    p_did = pys2opc.ffi.new('SOPC_LibSub_DataId[1]')
    str_list = [pys2opc.ffi.new('char[]', b"s=Counter")]
    pys2opc.lib.SOPC_LibSub_AddToSubscription(p_conId[0],
            pys2opc.ffi.new('char **', str_list[0]),
            pys2opc.ffi.new('SOPC_LibSub_AttributeId*', pys2opc.lib.SOPC_LibSub_AttributeId_Value),
            1, p_did)
    print('Data_Id:', p_did[0])

    time.sleep(2)

    pys2opc.lib.SOPC_LibSub_Clear()
