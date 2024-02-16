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

/** \file
 *
 * \brief Internal API to manage the ServerConfiguration according the Push model.
 */

#ifndef SOPC_PUSH_SERVER_CONFIG_
#define SOPC_PUSH_SERVER_CONFIG_

#include "sopc_builtintypes.h"
#include "sopc_certificate_group.h"

/**
 * \brief Internal structure to share context through method call.
 */
typedef struct PushServerContext
{
    bool bIsInit;                              /*!< Define if the API is initialized */
    bool bIsConfigured;                        /*!< Define if the API is configured. */
    const SOPC_NodeId* pServerConfigurationId; /*!< The nodeId of the ServerConfiguration object. */
    const SOPC_NodeId* pAppGroupId;            /*!< The nodeId of the application Group. */
    const SOPC_NodeId* pUsrGroupId;            /*!< The nodeId of the userToken group. */
} PushServerContext;

/**
 * \brief Internal structure to gather nodeIds.
 */
typedef struct PushServerConfig_NodeIds
{
    const SOPC_NodeId* pServerConfigurationId;  /*!< The nodeId of the ServerConfiguration object. */
    const SOPC_NodeId* pUpdateCertificateId;    /*!< The nodeId of the UpdateCertificate method. */
    const SOPC_NodeId* pApplyChangesId;         /*!< The nodeId of the ApplyChanges method.*/
    const SOPC_NodeId* pCreateSigningRequestId; /*!< The nodeId of the CreateSigningRequest method. */
    const SOPC_NodeId* pGetRejectedListId;      /*!< The nodeId of the CreateSigningRequest method. */
    const SOPC_NodeId* pAppGroupId;             /*!< The nodeId of the application group. */
    const SOPC_NodeId* pUsrGroupId;             /*!< The nodeId of the userToken group. */
} PushServerConfig_NodeIds;

/**
 * \brief Internal structure to gather method.
 */
typedef struct PushServerConfig_MethodFunc_Ptr
{
    SOPC_MethodCallFunc_Ptr* UpdateCertificate;
    SOPC_MethodCallFunc_Ptr* ApplyChanges;
    SOPC_MethodCallFunc_Ptr* CreateSigningRequest;
    SOPC_MethodCallFunc_Ptr* GetRejectedList;
} PushServerConfig_MethodFunc_Ptr;

/**
 * \brief Structure to gather the ServerConfiguration object data
 */
struct SOPC_PushServerConfig_Config
{
    PushServerConfig_NodeIds* pIds;                 /*!< Define all the nodeId of the ServerConfiguration. */
    SOPC_CertificateGroup_Config* pAppCertGroupCfg; /*!< Application certificate group configuration that
                                                         belongs to the CertificateGroups folder node */
    SOPC_CertificateGroup_Config* pUsrCertGroupCfg; /*!< User certificate group configuration that belongs to the
                                                         CertificateGroups folder node (NULL if not used) */
    bool bIsTOFUSate;                               /*!< When flag is set, the TOFU state is enabled. */
};

/**
 * \brief Get the list of certificates that have been rejected by the server,
 *        through the PKI attached to all group (user or application group)
 *
 * \param[out] ppBsCertArray A valid pointer to the newly created ByteString array.
 * \param[out] pLength The length of \p ppThumbprintArray
 *
 * \return Return SOPC_GoodGenericStatus if successful.
 */
SOPC_StatusCode PushServer_GetRejectedList(SOPC_ByteString** ppBsCertArray, uint32_t* pLengthArray);

/**
 * \brief Export the list of certificates that have been rejected by the server to the file system,
 *        through the PKI attached to all group (user or application group).
 *
 * \return SOPC_STATUS_OK if successful.
 */
SOPC_StatusCode PushServer_ExportRejectedList(void);

#endif /* SOPC_PUSH_SERVER_CONFIG_ */
