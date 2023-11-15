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
 * \brief Internal API to manage the TrustListType according the Push model.
 */

#ifndef SOPC_TRUSTLIST_
#define SOPC_TRUSTLIST_

#include "sopc_atomic.h"
#include "sopc_buffer.h"
#include "sopc_event_timer_manager.h"
#include "sopc_trustlist_itf.h"

#define SOPC_TRUSTLIST_ACTIVITY_TIMEOUT_MS 60000u // 1 minute

#define SOPC_TRUSTLIST_INVALID_HANDLE 0u

/* Open mode value (part 12 §7.8.2.1 v1.05) */
#define SOPC_TL_OPEN_MODE_UNKNOWN 0u
#define SOPC_TL_OPEN_MODE_READ 1u
#define SOPC_TL_OPEN_MODE_WRITE_ERASE_EXISTING 6u

/* TrustListMasks value (Table 29 - part 12 §7.8.2.7 v1.05) */
#define SOPC_TL_MASK_NONE 0u
#define SOPC_TL_MASK_TRUSTED_CERTS 1u
#define SOPC_TL_MASK_TRUSTED_CRLS 2u
#define SOPC_TL_MASK_ISSUER_CERTS 4u
#define SOPC_TL_MASK_ISSUER_CRLS 8u
#define SOPC_TL_MASK_ALL 15u

/**
 * \brief The TrustList handle
 */
typedef uint32_t SOPC_TrLst_Handle;

/**
 * \brief The Open mode
 */
typedef SOPC_Byte SOPC_TrLst_OpenMode;

/**
 * \brief The TrustListMasks
 */
typedef uint32_t SOPC_TrLst_Mask;

/**
 * \brief structure which gather all the variable nodIds belonging to the TrustList
 */
typedef struct SOPC_TrLst_VarCfg
{
    SOPC_NodeId* pSizeId;
    SOPC_NodeId* pWritableId;
    SOPC_NodeId* pUserWritableId;
    SOPC_NodeId* pOpenCountId;
    SOPC_NodeId* pLastUpdateTimeId;
} SOPC_TrLst_VarCfg;

typedef struct SOPC_TrLst_TimeEvent
{
    SOPC_Looper* pLooper;
    SOPC_EventHandler* pHandler;
    uint32_t activityTimeoutTimId;
    int32_t timeoutElapsed;
} SOPC_TrLst_TimeEvent;

/**
 * \brief Internal context for the TrustList
 */
typedef struct SOPC_TrustListContext
{
    uint32_t maxTrustListSize;           /*!< Defines the maximum size in byte for the TrustList */
    SOPC_NodeId* pObjectId;              /*!< The nodeId of the the TrustList */
    char* cStrObjectId;                  /*!< The C string nodeId of the the TrustList (it is used for logs) */
    SOPC_TrLst_Handle handle;            /*!< Defines the TrustList handle */
    SOPC_TrLst_OpenMode openingMode;     /*!< Defines the opening mode (read or write + erase if existing) */
    SOPC_TrLst_Mask openingMask;         /*!< Defines the opening mask to read only part of the TrustList */
    SOPC_TrustList_Type groupType;       /*!< Defines the TrustList type (associated to user or application) */
    SOPC_TrLst_VarCfg varIds;            /*!< The structure which gather all the variable nodIds
                                              belonging to the TrustList */
    SOPC_Buffer* pTrustListEncoded;      /*!< The buffer holding the instance of the TrustListDataType
                                              encoded in a UA Binary stream */
    SOPC_PKIProvider* pPKI;              /*!< A valid pointer to the PKI that belongs to the TrustList */
    SOPC_TrLst_Mask specifiedLists;      /*!< The parts of the TrustList to update with CloseAndUpdate */
    SOPC_CertificateList* pTrustedCerts; /*!< The trusted certificate list to be updated */
    SOPC_CertificateList* pIssuerCerts;  /*!< The issuer certificate list to be updated */
    SOPC_CRLList* pTrustedCRLs;          /*!< The trusted CRL list to be updated */
    SOPC_CRLList* pIssuerCRLs;           /*!< The issuer CRL list to be updated */
    SOPC_TrLst_TimeEvent event;
    SOPC_TrustList_UpdateCompleted_Fct* pFnUpdateCompleted; /*!< Calls when a new valid update has occurred */
    bool bDoNotDelete; /*!< Defined whatever the TrustList context shall be deleted */
} SOPC_TrustListContext;

/**
 * \brief Insert a new objectId key and TrustList context value.
 *
 * \param pObjectId The objectId of the TrustList.
 * \param pContext  A valid pointer on the TrustList context.
 *
 * \return \c TRUE in case of success.
 */
bool TrustList_DictInsert(SOPC_NodeId* pObjectId, SOPC_TrustListContext* pContext);

/**
 * \brief Get the TrustList context from the nodeId.
 *
 * \param pObjectId  The objectId of the TrustList.
 * \param bCheckActivityTimeout Defined whatever the activity timeout shall be check.
 * \param callContextPtr The method call context to set if \p bCheckActivityTimeout is TRUE (NULL if not used)
 * \param[out] bFound Defined whatever the TrustList is found that belongs to \p objectId .
 *
 * \return Return a valid ::SOPC_TrustListContext or NULL if error.
 */
SOPC_TrustListContext* TrustList_DictGet(const SOPC_NodeId* pObjectId,
                                         bool bCheckActivityTimeout,
                                         const SOPC_CallContext* callContextPtr,
                                         bool* bFound);

/**
 * \brief Removes a TrustList context from the nodeId.
 *
 * \param pObjectId The objectId of the TrustList.
 */
void TrustList_DictRemove(const SOPC_NodeId* pObjectId);

/**
 * \brief Start the activity timeout.
 *
 * \note If the period \c SOPC_TRUSTLIST_ACTIVITY_TIMEOUT_MS has elapsed then the trustlist is closed when calling
 *       ::TrustList_DictGet )
 *
 * \param pTrustList The TrustList context.
 *
 * \warning \p pTrustList shall be valid (!= NULL)
 */
void TrustList_StartActivityTimeout(SOPC_TrustListContext* pContext);

/**
 * \brief Reset the activity timeout.
 *
 * \note If the period \c SOPC_TRUSTLIST_ACTIVITY_TIMEOUT_MS has elapsed then the trustlist is closed when calling
 *       ::TrustList_DictGet )
 *
 * \param pTrustList The TrustList context.
 *
 * \warning \p pTrustList shall be valid (!= NULL)
 */
void TrustList_ResetActivityTimeout(SOPC_TrustListContext* pContext);

/**
 * \brief Generate a random handle and attach it to the TrustList.
 *
 * \param pTrustList The TrustList context.
 *
 * \warning \p pTrustList shall be valid (!= NULL)
 *
 * \return SOPC_STATUS_OK if successful
 */
SOPC_ReturnStatus TrustList_GenRandHandle(SOPC_TrustListContext* pTrustList);

/**
 * \brief Set the opening mode of the TrustList.
 *
 * \param pTrustList The TrustList context.
 * \param mode       The opening masks.
 *
 * \warning \p pTrustList shall be valid (!= NULL)
 *
 * \return True if \p mode is valid.
 */
bool TrustList_SetOpenMode(SOPC_TrustListContext* pTrustList, SOPC_TrLst_OpenMode mode);

/**
 * \brief Set the opening masks for the specifiedLists of the TrustList.
 *
 * \param pTrustList The TrustList context.
 * \param masks      The opening masks.
 *
 * \warning \p pTrustList shall be valid (!= NULL)
 *
 * \return True if \p masks is valid.
 */
bool TrustList_SetOpenMasks(SOPC_TrustListContext* pTrustList, SOPC_TrLst_Mask masks);

/**
 * \brief Set the TrustList position.
 *
 * \param pTrustList The TrustList context.
 * \param[out] pos  The TrustList position.
 *
 * \warning \p pTrustList shall be valid (!= NULL)
 *
 * \return SOPC_STATUS_OK if successful.
 */
SOPC_ReturnStatus TrustList_SetPosition(SOPC_TrustListContext* pTrustList, uint64_t pos);

/**
 * \brief Get the TrustList handle.
 *
 * \param pTrustList The TrustList context.
 *
 * \warning \p pTrustList shall be valid (!= NULL)
 *
 * \return The TrustList handle
 */
uint32_t TrustList_GetHandle(const SOPC_TrustListContext* pTrustList);

/**
 * \brief Get the TrustList position.
 *
 * \param pTrustList The TrustList context.
 *
 * \warning \p pTrustList shall be valid (!= NULL)
 *
 * \return The TrustList position.
 */
uint64_t TrustList_GetPosition(const SOPC_TrustListContext* pTrustList);

/**
 * \brief Get the TrustList length.
 *
 * \param pTrustList The TrustList context.
 *
 * \warning \p pTrustList shall be valid (!= NULL)
 *
 * \return The TrustList length.
 */
uint32_t TrustList_GetLength(const SOPC_TrustListContext* pTrustList);

/**
 * \brief Get the specifiedLists mask of the written TrustListDataType.
 *
 * \param pTrustList The TrustList context.
 *
 * \note If the trusted list is not opened in write mode, or if no write operation is performed,
 *       the function returns SOPC_TL_MASK_NONE.
 *
 * \warning \p pTrustList shall be valid (!= NULL)
 *
 * \return The specifiedLists mask
 */
SOPC_TrLst_Mask TrustList_GetSpecifiedListsMask(const SOPC_TrustListContext* pTrustList);

/**
 * \brief Get the TOFU state status
 *
 * \param pTrustList The TrustList context.
 *
 * \warning \p pTrustList shall be valid (!= NULL).
 *
 * \return True if the server is in TOFU state.
 */
bool TrustList_isInTofuSate(const SOPC_TrustListContext* pTrustList);

/**
 * \brief Check the TrustList handle.
 *
 * \param pTrustList The TrustList context.
 * \param expected   The expected handle.
 * \param msg        The msg to log (NULL if not used)
 *
 * \warning \p pTrustList shall be valid (!= NULL)
 *
 * \return True if match.
 */
bool TrustList_CheckHandle(const SOPC_TrustListContext* pTrustList, SOPC_TrLst_Handle expected, const char* msg);

/**
 * \brief Check TrustList opening mode.
 *
 * \param pTrustList The TrustList context.
 * \param expected   The expected opening mode.
 * \param msg        The msg to log (NULL if not used)
 *
 * \warning \p pTrustList shall be valid (!= NULL)
 *
 * \return True if match.
 */
bool TrustList_CheckOpenMode(const SOPC_TrustListContext* pTrustList, SOPC_TrLst_OpenMode expected, const char* msg);

/**
 * \brief Check if the TrustList is open.
 *
 * \param pTrustList The TrustList context.
 *
 * \warning \p pTrustList shall be valid (!= NULL)
 *
 * \return True if open.
 */
bool TrustList_CheckIsOpen(const SOPC_TrustListContext* pTrustList);

/**
 * \brief Get the TrustList nodeId
 *
 * \param pTrustList The TrustList context.
 *
 * \warning \p pTrustList shall be valid (!= NULL)
 *
 *\return The TrustList nodeId
 */
const char* TrustList_GetStrNodeId(const SOPC_TrustListContext* pTrustList);

/**
 * \brief Read the PKI that belongs to the TrustList and encode it in a UA Binary encoded stream containing
 *        an instance of TrustListDataType.
 *
 * \param pTrustList The TrustList context.
 *
 * \note \p pTrustList is not encoded if it is opened in write mode.
 *
 * \warning \p pTrustList shall be valid (!= NULL).
 *
 * \return SOPC_STATUS_OK if successful.
 */
SOPC_ReturnStatus TrustList_Encode(SOPC_TrustListContext* pTrustList);

/**
 * \brief Read \p reqLength bytes from the current position of the encoded TrustListDataType.
 *        If \p reqLength is greater than the encoded data then
 *        the whole TrustListDataType is read otherwise the position is incremented by \p reqLength .
 *        The part read depends on the OpeningMask.
 *
 * \param pTrustList The TrustList context.
 * \param length     The length byte to read (!= 0)
 * \param[out] pDest A valid byte string to store the result.
 *
 * \warning \p pTrustList shall be valid (!= NULL) and open in read mode.
 *
 * \return SOPC_STATUS_OK if successful.
 */
SOPC_ReturnStatus TrustList_Read(SOPC_TrustListContext* pTrustList, int32_t reqLength, SOPC_ByteString* pDest);

/**
 * \brief Read and decode the buffer containing the TrustListDataType.
 *        The Certificate and CRL list result are append in \p pTrustList for later updates.
 *
 * \param pTrustList The TrustList context.
 * \param pSrc       The TrustListDataType to decode.
 *
 * \warning \p pTrustList shall be valid (!= NULL) and open in write mode.
 *
 * \return SOPC_STATUS_OK if successful.
 */
SOPC_ReturnStatus TrustList_Decode(SOPC_TrustListContext* pTrustList, const SOPC_ByteString* pSrc);

/**
 * \brief Remove a single Certificate from the TrustList.
 *        If the Certificate is a CA Certificate that has CRLs then all CRLs for that CA are removed as well.
 *
 * \param pTrustList      The TrustList context.
 * \param pThumbprint     The thumbprint of the certificate to remove formatted as a hexadecimal string.
 * \param bIsTrustedCert  If TRUE the certificate is removed from the trusted certificates list otherwise
 *                        the certificate is removed from the issuer certificates list.
 * \param secPolUri       The security policy.
 * \param[out] pbIsRemove A valid pointer indicating whether the certificate has been found and deleted.
 * \param[out] pbIsIssuer A valid pointer indicating whether the deleted certificate is an issuer.
 *
 * \warning \p pTrustList shall be valid (!= NULL) and not open.
 *
 * \return SOPC_GoodGenericStatus if successful.
 */
SOPC_StatusCode TrustList_RemoveCert(SOPC_TrustListContext* pTrustList,
                                     const SOPC_String* pThumbprint,
                                     const bool bIsTrustedCert,
                                     const char* secPolUri,
                                     bool* pbIsRemove,
                                     bool* pbIsIssuer);

/**
 * \brief Validate the certificate and update the PKI that belongs to the TrustList.
 *        (AddCertificate method)
 *
 * \param pTrustList The TrustList context.
 * \param pBsCert    The ByteString certificate to update.
 * \param secPolUri  The security policy.
 *
 * \warning \p pTrustList shall be valid (!= NULL) and not open.
 *
 * \return SOPC_GoodGenericStatus if successful or a validation error.
 */
SOPC_StatusCode TrustList_UpdateWithAddCertificateMethod(SOPC_TrustListContext* pTrustList,
                                                         const SOPC_ByteString* pBsCert,
                                                         const char* secPolUri);

/**
 * \brief Validate the written TrustList and update the PKI.
 *        (CLoseAndUpdate method)
 *
 * \param pTrustList The TrustList context.
 * \param secPolUri  The security policy.
 *
 * \warning \p pTrustList shall be valid (!= NULL).
 *
 * \return SOPC_GoodGenericStatus if successful.
 */
SOPC_StatusCode TrustList_UpdateWithWriteMethod(SOPC_TrustListContext* pTrustList, const char* secPolUri);

/**
 * \brief Export the update
 *
 * \param pTrustList The TrustList context.
 * \param bEraseExiting Define if the existing certificate shall be deleted or include with the update.
 * \param bForcePush Force the export in case \p pTrustList is not open.
 *                   (cases of AddCertificate and RemoveCertificate)
 *
 * \warning \p pTrustList shall be valid (!= NULL).
 *
 * \return SOPC_STATUS_OK if successful.
 */
SOPC_ReturnStatus TrustList_Export(const SOPC_TrustListContext* pTrustList,
                                   const bool bEraseExiting,
                                   const bool bForcePush);

/**
 * \brief Raise an event to re-evaluate the certificate.
 *        The event depends on the certificate Group.
 *        Close all the secure channels for application group (SC layer).
 *        Make sessions orphaned for user group (Services layer).
 *
 * \param pTrustList The TrustList context.
 *
 * \warning \p pTrustList shall be valid (!= NULL).
 *
 * \return SOPC_STATUS_OK if successful.
 */
SOPC_ReturnStatus TrustList_RaiseEvent(const SOPC_TrustListContext* pTrustList);

/**
 * \brief Executes user callback to indicate the end of a valid update.
 *        Do nothing if the callback is not configured.
 *
 * \param pTrustList The TrustList context.
 *
 * \warning \p pTrustList shall be valid (!= NULL).
 */
void TrustList_UpdateCompleted(const SOPC_TrustListContext* pTrustList);

/**
 * \brief Write the value of the TrustList Size variable.
 *
 * \param pTrustList The TrustList context.
 * \param pAddSpAccess The address space access.
 *
 * \note If \p pTrustList is open in write mode then StatusCode of Bad_NotSupported is written.
 *
 * \warning \p pTrustList shall be valid (!= NULL).
 *
 */
void Trustlist_WriteVarSize(const SOPC_TrustListContext* pTrustList, SOPC_AddressSpaceAccess* pAddSpAccess);

/**
 * \brief Write the value of the TrustList OpenCount variable.
 *
 * \param pTrustList The TrustList context.
 * \param pAddSpAccess The address space access.
 *
 * \warning \p pTrustList shall be valid (!= NULL).
 *
 */
void Trustlist_WriteVarOpenCount(const SOPC_TrustListContext* pTrustList, SOPC_AddressSpaceAccess* pAddSpAccess);

/**
 * \brief Write the current time in the LastUpdateTime variable.
 *
 * \param pTrustList The TrustList context.
 * \param pAddSpAccess The address space access.
 *
 * \warning \p pTrustList shall be valid (!= NULL).
 *
 */
void Trustlist_WriteVarLastUpdateTime(const SOPC_TrustListContext* pTrustList, SOPC_AddressSpaceAccess* pAddSpAccess);

/**
 * \brief Reset the context of the TrustList but keep the user configuration.
 *
 * \param pTrustList The TrustList context.
 * \param pAddSpAccess The address space access to reset the value of variables (NULL if not used).
 *
 * \warning \p pTrustList shall be valid (!= NULL)
 */
void TrustList_Reset(SOPC_TrustListContext* pTrustList, SOPC_AddressSpaceAccess* pAddSpAccess);

#endif /* SOPC_TRUSTLIST_ */
