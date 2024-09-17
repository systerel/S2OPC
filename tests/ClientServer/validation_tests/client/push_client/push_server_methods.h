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

#include "libs2opc_client.h"
#include "libs2opc_request_builder.h"

/* Test methods of the TrustList of the server */

/**
 * @brief Call the method Open on the TrustList of the server, in write or read mode.
 *
 * @param[in] secureConnection The client connection instance used to execute the service.
 * @param[in] write Wether the TrustList needs to be open in Write mode or in Read only.
 * True stands for Read/Write mode and False for Read only mode.
 * @param[out] pFileHandle The FileHandle identifying the open TrustList.
 * @return SOPC_ReturnStatus
 */
SOPC_ReturnStatus SOPC_TEST_TrustList_Open(SOPC_ClientConnection* secureConnection, bool write, uint32_t* pFileHandle);

/**
 * @brief Call the method Read on the TrustList of the server.
 *
 * @param[in] secureConnection The client connection instance used to execute the service.
 * @param[in] fileHandle The FileHandle of the TrustList to read.
 * @param[out] ppTrustList The TrustList returned by the server.
 * @return SOPC_ReturnStatus
 */
SOPC_ReturnStatus SOPC_TEST_TrustList_Read(SOPC_ClientConnection* secureConnection,
                                           uint32_t fileHandle,
                                           OpcUa_TrustListDataType** ppTrustList);

/**
 * @brief Call the method Close on the TrustList of the server.
 *
 * @param[in] secureConnection The client connection instance used to execute the service.
 * @param[in] fileHandle The FileHandle of the TrustList to close.
 * @return SOPC_ReturnStatus
 */
SOPC_ReturnStatus SOPC_TEST_TrustList_Close(SOPC_ClientConnection* secureConnection, uint32_t fileHandle);

/**
 * @brief Call the method Write on the TrustList of the server.
 *
 * @param[in] secureConnection The client connection instance used to execute the service.
 * @param[in] fileHandle The FileHandle of the TrustList to write.
 * @param[in] pTrustList The TrustList to write into the server's TrustList.
 * @return SOPC_ReturnStatus
 */
SOPC_ReturnStatus SOPC_TEST_TrustList_Write(SOPC_ClientConnection* secureConnection,
                                            uint32_t fileHandle,
                                            OpcUa_TrustListDataType* pTrustList);

/**
 * @brief Call the method OpenWithMasks on the TrustList of the server.
 * This method is used to read only a portion of the TrustList.
 *
 * @param[in] secureConnection The client connection instance used to execute the service.
 * @param[in] mask The mask telling which portion of the TrustList needs to be open.
 * @param[out] pFileHandle The FileHandle identifying the open TrustList.
 * @return SOPC_ReturnStatus
 */
SOPC_ReturnStatus SOPC_TEST_TrustList_OpenWithMasks(SOPC_ClientConnection* secureConnection,
                                                    OpcUa_TrustListMasks mask,
                                                    uint32_t* pFileHandle);

/**
 * @brief Call the method CloseAndUpdate on the TrustList of the server.
 * This method is used after writing a new TrustList in the server.
 *
 * @param[in] secureConnection The client connection instance used to execute the service.
 * @param[in] fileHandle The FileHandle of the TrustList to close and update.
 * @return SOPC_ReturnStatus
 */
SOPC_ReturnStatus SOPC_TEST_TrustList_CloseAndUpdate(SOPC_ClientConnection* secureConnection, uint32_t fileHandle);

/**
 * @brief Call the method AddCertificate on the TrustList of the server.
 * This method is used to add a certificate to the trusted certificates of the server's TrustList.
 * Only trusted certificates can be added with this method, and certificate that does not need a CRL
 * (ie not CA or with pathLen 0). Therefore, the server must return an error if the parameter
 * isTrustedCertificate is false.
 *
 * @param[in] secureConnection The client connection instance used to execute the service.
 * @param[in] certificate The certificate to add.
 * @param[in] isTrustedCertificate If the certificate is trusted or not.
 * @return SOPC_ReturnStatus
 */
SOPC_ReturnStatus SOPC_TEST_TrustList_AddCertificate(SOPC_ClientConnection* secureConnection,
                                                     SOPC_ByteString* certificate,
                                                     bool isTrustedCertificate);

/**
 * @brief Call the method RemoveCertificate on the TrustList of the server.
 *
 * @param[in] secureConnection The client connection instance used to execute the service.
 * @param[in] pThumbprint The thumbprint of the certificate to remove.
 * @param[in] isTrustedCertificate If the certifiate is trusted or not.
 * @return SOPC_ReturnStatus
 */
SOPC_ReturnStatus SOPC_TEST_TrustList_RemoveCertificate(SOPC_ClientConnection* secureConnection,
                                                        SOPC_String pThumbprint,
                                                        bool isTrustedCertificate);

/* Test methods of the ServerConfiguration. */

/**
 * @brief Call the method CreateSigningRequest of the server and get the OPC UA response.
 *
 * @param[in] secureConnection The client connection instance used to execute the service.
 * @param[in] groupId The NodeId of the Certificate Group which is affected by the request.
 * @param[in] certificateTypeId The type of Certificate being requested.
 * @param[in] subjectName The subjectName of the CSR.
 * @param[in] renewKey If the server uses its actual key or a new one for signing the CSR.
 * @param[in] nonce The nonce that will be used to create the CSR.
 * @param[out] resp The OPC UA response of the server.
 * @return SOPC_ReturnStatus
 */
SOPC_ReturnStatus SOPC_TEST_ServerConfiguration_CreateSigningRequest_GetResponse(
    SOPC_ClientConnection* secureConnection,
    SOPC_NodeId* groupId,
    SOPC_NodeId* certificateTypeId,
    SOPC_String* subjectName,
    bool renewKey,
    SOPC_ByteString* nonce,
    OpcUa_CallResponse** resp);

/**
 * @brief Extract the CSR from the OPC UA response of the CreateSigningRequest request.
 *
 * @param[in] resp The OPC UA response.
 * @param[out] csr The extracted csr.
 * @return SOPC_ReturnStatus
 */
SOPC_ReturnStatus SOPC_TEST_ServerConfiguration_CreateSigningRequest_FromResponse(OpcUa_CallResponse* resp,
                                                                                  SOPC_ByteString** csr);

/**
 * @brief Call the method UpdateCertificate of the server. This method is used after obtaining a signed
 * certificate with the CSR provided by the server.
 *
 * @param[in] secureConnection The client connection instance used to execute the service.
 * @param[in] certificate The new certificate.
 * @param[in] issuersArray The issuers of the certificate.
 * @param[in] nbOfIssuers The number of issuers.
 * @return SOPC_ReturnStatus
 */
SOPC_ReturnStatus SOPC_TEST_ServerConfiguration_UpdateCertificate(SOPC_ClientConnection* secureConnection,
                                                                  SOPC_ByteString* certificate,
                                                                  SOPC_ByteString* issuersArray,
                                                                  int32_t nbOfIssuers);

/**
 * @brief Call the method GetRejectedList of the server. This method has no parameter, it will create a folder
 * "rejected" in the server PKI folder that will contain all the non-validated certificates of the server
 *
 * @param[in] secureConnection The client connection instance used to execute the service.
 * @return SOPC_ReturnStatus
 */
SOPC_ReturnStatus SOPC_TEST_ServerConfiguration_GetRejectedList(SOPC_ClientConnection* secureConnection);

extern const SOPC_NodeId gServerDefaultApplicationGroupId;
extern const SOPC_NodeId gRsaSha256ApplicationCertificateTypeId;

#endif /* CHECK_PUSH_CLIENT_METHOD_H_ */
