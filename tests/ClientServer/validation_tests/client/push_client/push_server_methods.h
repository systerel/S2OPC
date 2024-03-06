/*
 * Licensed to Systerel under one or more contributor license
 * agreements. See the NOTICE file distributed with this work
 * for additional information regarding copyright ownership.
 * Systerel licenses this file to you under the Apache
 * License, Version 2.0 (the "License"); you may not use this
 * file except in compliance with the License. You may obtain
 * a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#ifndef CHECK_PUSH_CLIENT_METHOD_H_
#define CHECK_PUSH_CLIENT_METHOD_H_

#include "libs2opc_new_client.h"
#include "libs2opc_request_builder.h"

/* Test methods of the TrustList (ServerConfiguration.CertificateGroups.DefaultApplicationGroup.TrustList) */
// 1) Methods inherited from FileType
SOPC_ReturnStatus SOPC_TEST_TrustList_Read(SOPC_ClientConnection* secureConnection,
                                           uint32_t fileHandle,
                                           OpcUa_TrustListDataType** ppTrustList);

SOPC_ReturnStatus SOPC_TEST_TrustList_Close(SOPC_ClientConnection* secureConnection, uint32_t fileHandle);

SOPC_ReturnStatus SOPC_TEST_TrustList_Open(SOPC_ClientConnection* secureConnection, bool write, uint32_t* pFileHandle);

SOPC_ReturnStatus SOPC_TEST_TrustList_Write(SOPC_ClientConnection* secureConnection,
                                            uint32_t fileHandle,
                                            OpcUa_TrustListDataType* pTrustList);

// SOPC_ReturnStatus SOPC_TEST_TrustList_GetPos();
// SOPC_ReturnStatus SOPC_TEST_TrustList_SetPos();
// TODO: test these two methods?

// 2) New methods
SOPC_ReturnStatus SOPC_TEST_TrustList_OpenWithMasks(SOPC_ClientConnection* secureConnection,
                                                    OpcUa_TrustListMasks mask,
                                                    uint32_t* pFileHandle);

SOPC_ReturnStatus SOPC_TEST_TrustList_CloseAndUpdate(SOPC_ClientConnection* secureConnection, uint32_t fileHandle);

SOPC_ReturnStatus SOPC_TEST_TrustList_AddCertificate(SOPC_ClientConnection* secureConnection,
                                                     SOPC_ByteString* certificate,
                                                     bool isTrustedCertificate);

SOPC_ReturnStatus SOPC_TEST_TrustList_RemoveCertificate(SOPC_ClientConnection* secureConnection,
                                                        SOPC_String pThumbprint,
                                                        bool isTrustedCertificate);

/* Methods of the ServerConfiguration. */
SOPC_ReturnStatus SOPC_TEST_ServerConfiguration_CreateSigningRequest_GetResponse(
    SOPC_ClientConnection* secureConnection,
    SOPC_NodeId* groupId,
    SOPC_NodeId* certificateTypeId,
    SOPC_String* subjectName,
    bool renewKey,
    SOPC_ByteString* nonce,
    OpcUa_CallResponse** resp);
SOPC_ReturnStatus SOPC_TEST_ServerConfiguration_CreateSigningRequest_FromResponse(OpcUa_CallResponse* resp,
                                                                                  SOPC_ByteString** csr);

SOPC_ReturnStatus SOPC_TEST_ServerConfiguration_UpdateCertificate(SOPC_ClientConnection* secureConnection,
                                                                  SOPC_ByteString* certificate,
                                                                  SOPC_ByteString* issuersArray,
                                                                  int32_t nbOfIssuers);

SOPC_ReturnStatus SOPC_TEST_ServerConfiguration_GetRejectedList(SOPC_ClientConnection* secureConnection);

#endif /* CHECK_PUSH_CLIENT_METHOD_H_ */
