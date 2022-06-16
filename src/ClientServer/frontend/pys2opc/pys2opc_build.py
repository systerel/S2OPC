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


from cffi import FFI
import os

ffibuilder = FFI()
# TODO: generate this file instead of concatenating manual copies of selected headers
header = open('./s2opc_expanded.h').read()
ffibuilder.cdef(header + r'''
    # 1 "cffi-cdef"
    /* Python callbacks that are callable from C */
    extern "Python"
    {
        void _callback_log(SOPC_Log_Level log_level, SOPC_LibSub_CstString text);
        void _callback_disconnected(SOPC_LibSub_ConnectionId c_id);
        void _callback_datachanged(SOPC_LibSub_ConnectionId c_id, SOPC_LibSub_DataId d_id, SOPC_LibSub_Value* value);
        void _callback_client_event(SOPC_LibSub_ConnectionId c_id, SOPC_LibSub_ApplicativeEvent event, SOPC_StatusCode status, const void* response, uintptr_t responseContext);
        void _callback_toolkit_event(SOPC_App_Com_Event event, uint32_t IdOrStatus, void* param, uintptr_t appContext);
        void _callback_address_space_event(const SOPC_CallContext* callCtxPtr, SOPC_App_AddSpace_Event event, void* opParam, SOPC_StatusCode opStatus);
        SOPC_ReturnStatus _callback_validate_user_identity(SOPC_UserAuthentication_Manager* authenticationManager,
                                                           const SOPC_ExtensionObject* pUser,
                                                           SOPC_UserAuthentication_Status* pUserAuthenticated);
        SOPC_ReturnStatus _callback_authorize_operation(SOPC_UserAuthorization_Manager* authorizationManager,
                                                        SOPC_UserAuthorization_OperationType operationType,
                                                        const SOPC_NodeId* nodeId,
                                                        uint32_t attributeId,
                                                        const SOPC_User* pUser,
                                                        bool* pbOperationAuthorized);
    }

    void SOPC_DataValue_Delete(SOPC_DataValue *datavalue);
''')

source = r'''
    #include "s2opc_expanded.h"

    const char* SOPC_SecurityPolicy_None_URI = "http://opcfoundation.org/UA/SecurityPolicy#None";
    const char* SOPC_SecurityPolicy_Basic128Rsa15 = "http://opcfoundation.org/UA/SecurityPolicy#Basic128Rsa15";
    const char* SOPC_SecurityPolicy_Basic256_URI = "http://opcfoundation.org/UA/SecurityPolicy#Basic256";
    const char* SOPC_SecurityPolicy_Basic256Sha256_URI = "http://opcfoundation.org/UA/SecurityPolicy#Basic256Sha256";
    const char* SOPC_SecurityPolicy_Aes128Sha256RsaOaep_URI = "http://opcfoundation.org/UA/SecurityPolicy#Aes128_Sha256_RsaOaep";
    const char* SOPC_SecurityPolicy_Aes256Sha256RsaPss_URI = "http://opcfoundation.org/UA/SecurityPolicy#Aes256_Sha256_RsaPss";
    const uint8_t SOPC_SecurityMode_None_Mask = 0x01;
    const uint8_t SOPC_SecurityMode_Sign_Mask = 0x02;
    const uint8_t SOPC_SecurityMode_SignAndEncrypt_Mask = 0x04;
    const uint8_t SOPC_SecurityMode_Any_Mask = 0x07;
    const uint8_t SOPC_MaxSecuPolicies_CFG = 5;

    void SOPC_DataValue_Delete(SOPC_DataValue *datavalue)
    {
        SOPC_DataValue_Clear(datavalue);
        free(datavalue);
    }
'''

# It (is said to) produces faster code with set_source, and checks what it can on the types.
# However, it requires a gcc.
# The other way, dlopen, loads the ABI, is less safe, slower, but only requires the .so/.dll
# TODO: automatize configuration through cmake

if os.name == 'nt':
# Windows
    ffibuilder.set_source('_pys2opc',
                      source,
                      extra_link_args=['Advapi32.lib', 'ws2_32.lib', 's2opc_clientserver-xml-loaders-expat.lib', 's2opc_clientwrapper.lib', 's2opc_commonwrapper.lib', 's2opc_clientserver.lib', 's2opc_common.lib', 'mbedcrypto.lib', 'mbedtls.lib', 'mbedx509.lib', 'libexpat.lib'],
                      include_dirs=['.'],
                      library_dirs=['../lib', # working dir should be located in build dir
                                    '.'],  # Ease compilation outside of the S2OPC project
                     )
else:
# Linux
    ffibuilder.set_source('_pys2opc',
                      source,
                      extra_link_args=['-ls2opc_clientserver-xml-loaders-expat', '-ls2opc_clientwrapper', '-ls2opc_commonwrapper', '-ls2opc_clientserver', '-ls2opc_common', '-lmbedcrypto', '-lmbedtls', '-lmbedx509', '-lexpat'],
                      include_dirs=['.'],
                      library_dirs=['../lib', # working dir should be located in build dir
                                    '.'],  # Ease compilation outside of the S2OPC project
                     )

if __name__ == '__main__':
    ffibuilder.compile(tmpdir='out')
