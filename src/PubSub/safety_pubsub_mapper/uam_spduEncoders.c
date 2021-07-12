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

/*============================================================================
  INCLUDES
 *===========================================================================*/
#include <assert.h>

#include "sopc_builtintypes.h"
#include "sopc_dict.h"
#include "sopc_encodeable.h"
#include "sopc_logger.h"
#include "sopc_mem_alloc.h"

#include "uam_cache.h"
#include "uam_spduEncoders.h"

/*============================================================================
  LOCAL TYPES
 *===========================================================================*/

/*============================================================================
 This structure provides the content of the encodeable object (SOPC extension object)
 The first field is enforced by SOPC APIs, and the functional informations (which are actually encoded)
   are following. In this case this is SPDU request content.
 *===========================================================================*/
typedef struct
{
    SOPC_EncodeableType* encodeableType;
    UAS_RequestSpdu_type zSpdu;
} SPDURequest;

/*============================================================================
 This structure provides the content of the encodeable object (SOPC extension object)
 The first field is enforced by SOPC APIs, and the functional informations (which are actually encoded)
   are following. In this case this is SPDU request content.
 *===========================================================================*/
typedef struct
{
    SOPC_EncodeableType* encodeableType;
    /** Pointer to the serialized safety-related application data */
    SOPC_ByteString zSerializedSafetyData;
    /** Pointer to the serialized non-safety application data */
    SOPC_ByteString zSerializedNonSafetyData;
    /** Safety Flags from SafetyProvider. */
    UAS_UInt8 byFlags;
    /** Safety PDU Identifier. The SPDU_ID is used by the SafetyConsumer to check
     * whether the ResponseSPDU is coming from the correct SafetyProvider. */
    UAS_SPDUID_type zSpduId;
    /** The SafetyConsumerID in the ResponseSPDU shall be a copy of the SafetyConsumerID
     * received in the corresponding RequestSPDU. */
    UAS_UInt32 dwSafetyConsumerId;
    /** Monitoring Number (MNR) of the ResponseSPDU. The MNR in the ResponseSPDU
     * shall be a copy of the MNR received in the corresponding RequestSPDU. */
    UAS_UInt32 dwMonitoringNumber;
    /** This CRC-checksum shall be used to detect data corruption. */
    UAS_UInt32 dwCrc;
} SPDUResponse;

/*============================================================================
 This structure holds the content and configuration of a SPDU (Request or Response)
 *===========================================================================*/
typedef struct SPDU_struct
{
    /** Type of SPDU */
    bool isRequest;
    /** Length of SafetyData*/
    UAS_UInt16 wSafetyDataLength;
    /** Length of NonSafetyData*/
    UAS_UInt16 wNonSafetyDataLength;
    union {
        /** Relevant if isRequest is true */
        SPDURequest* pzRequest;
        /** Relevant if isRequest is false */
        SPDUResponse* pzResponse;
        /** VOID* mapping of SPDU data */
        void* pzData;
    } Data;
} SPDU_type;

/*============================================================================
 * Field descriptors of the SPDURequest encodeable type.
 *===========================================================================*/
static const SOPC_EncodeableType_FieldDescriptor SPDURequest_Fields[] = {
    {
        true,                                                      // isBuiltIn
        false,                                                     // isArrayLength
        true,                                                      // isToEncode
        (uint32_t) SOPC_UInt32_Id,                                 // typeIndex
        (uint32_t) offsetof(SPDURequest, zSpdu.dwSafetyConsumerId) // offset
    },
    {
        true,                                                      // isBuiltIn
        false,                                                     // isArrayLength
        true,                                                      // isToEncode
        (uint32_t) SOPC_UInt32_Id,                                 // typeIndex
        (uint32_t) offsetof(SPDURequest, zSpdu.dwMonitoringNumber) // offset
    },
    {
        true,                                           // isBuiltIn
        false,                                          // isArrayLength
        true,                                           // isToEncode
        (uint32_t) SOPC_Byte_Id,                        // typeIndex
        (uint32_t) offsetof(SPDURequest, zSpdu.byFlags) // offset
    },
};

/*============================================================================
  LOCAL PROTOTYPES
 *===========================================================================*/
/**
 * \brief Create and registers a SPDU type. This internal function realizes the common initializations to
 *      both \ref UAM_SpduEncoder_CreateSpduRequest and \ref UAM_SpduEncoder_CreateSpduResponse
 * \param dwNumericId Either UAM_SpduRequestHandle or UAM_SpduResponseHandle depending on request type.
 *                  This will be the request identifier.
 * \param pzEncodeabletype The definition of SPDU as encodeable extended object.
 * \param pType Must be allocated by caller. Will be freed at termination.
 * \return SOPC_STATUS_OK in case of success
 */
static SOPC_ReturnStatus UAM_SpduEncoder_CreateSpdu(const UAS_UInt32 dwNumericId,
                                                    SOPC_EncodeableType* pzEncodeabletype,
                                                    SPDU_type* pType);

/*============================================================================
  LOCAL ENCODER/DECODER FUNCTIONS
 *===========================================================================*/
static SOPC_ReturnStatus SPDURequest_Encode(const void* pValue, SOPC_Buffer* msgBuffer, uint32_t nestedStructLevel);
static SOPC_ReturnStatus SPDURequest_Decode(void* pValue, SOPC_Buffer* buf, uint32_t nestedStructLevel);
static void SPDURequest_Initialize(void* pValue);
static void SPDURequest_Clear(void* pValue);
static SOPC_ReturnStatus SPDUResponse_Encode(const void* pValue, SOPC_Buffer* msgBuffer, uint32_t nestedStructLevel);
static SOPC_ReturnStatus SPDUResponse_Decode(void* pValue, SOPC_Buffer* buf, uint32_t nestedStructLevel);
static void SPDUResponse_Initialize(void* pValue);
static void SPDUResponse_Clear(void* pValue);

/*============================================================================
 * Structure descriptor of the SPDURequest encodeable type "SPDURequest".
 *===========================================================================*/
SOPC_EncodeableType SPDURequest_EncodeableType = {
    "SPDURequest",
    OpcUaId_SPDURequest,
    OpcUaId_SPDURequest_Encoding_DefaultBinary,
    OpcUaId_SPDURequest_Encoding_DefaultXml,
    NULL,
    sizeof(SPDURequest),
    SPDURequest_Initialize,
    SPDURequest_Clear,
    NULL,
    SPDURequest_Encode,
    SPDURequest_Decode,
    sizeof SPDURequest_Fields / sizeof(SOPC_EncodeableType_FieldDescriptor),
    SPDURequest_Fields};

/*===========================================================================*/
static SOPC_ReturnStatus SPDURequest_Encode(const void* pValue, SOPC_Buffer* msgBuffer, uint32_t nestedStructLevel)
{
    return SOPC_EncodeableObject_Encode(&SPDURequest_EncodeableType, pValue, msgBuffer, nestedStructLevel);
}

/*===========================================================================*/
static SOPC_ReturnStatus SPDURequest_Decode(void* pValue, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_EncodeableObject_Decode(&SPDURequest_EncodeableType, pValue, buf, nestedStructLevel);
}

/*===========================================================================*/
static void SPDURequest_Initialize(void* pValue)
{
    SOPC_EncodeableObject_Initialize(&SPDURequest_EncodeableType, pValue);
}

/*===========================================================================*/
static void SPDURequest_Clear(void* pValue)
{
    SOPC_EncodeableObject_Clear(&SPDURequest_EncodeableType, pValue);
}

/*============================================================================
 * Field descriptors of the SPDUResponse encodeable type.
 *===========================================================================*/
static const SOPC_EncodeableType_FieldDescriptor SPDUResponse_Fields[] = {
    {
        true,                                                    // isBuiltIn
        false,                                                   // isArrayLength
        true,                                                    // isToEncode
        (uint32_t) SOPC_ByteString_Id,                           // typeIndex
        (uint32_t) offsetof(SPDUResponse, zSerializedSafetyData) // offset
    },
    {
        true,                                                       // isBuiltIn
        false,                                                      // isArrayLength
        true,                                                       // isToEncode
        (uint32_t) SOPC_ByteString_Id,                              // typeIndex
        (uint32_t) offsetof(SPDUResponse, zSerializedNonSafetyData) // offset
    },
    {
        true,                                      // isBuiltIn
        false,                                     // isArrayLength
        true,                                      // isToEncode
        (uint32_t) SOPC_Byte_Id,                   // typeIndex
        (uint32_t) offsetof(SPDUResponse, byFlags) // offset
    },
    {
        true,                                              // isBuiltIn
        false,                                             // isArrayLength
        true,                                              // isToEncode
        (uint32_t) SOPC_UInt32_Id,                         // typeIndex
        (uint32_t) offsetof(SPDUResponse, zSpduId.dwPart1) // offset
    },
    {
        true,                                              // isBuiltIn
        false,                                             // isArrayLength
        true,                                              // isToEncode
        (uint32_t) SOPC_UInt32_Id,                         // typeIndex
        (uint32_t) offsetof(SPDUResponse, zSpduId.dwPart2) // offset
    },
    {
        true,                                              // isBuiltIn
        false,                                             // isArrayLength
        true,                                              // isToEncode
        (uint32_t) SOPC_UInt32_Id,                         // typeIndex
        (uint32_t) offsetof(SPDUResponse, zSpduId.dwPart3) // offset
    },
    {
        true,                                                 // isBuiltIn
        false,                                                // isArrayLength
        true,                                                 // isToEncode
        (uint32_t) SOPC_UInt32_Id,                            // typeIndex
        (uint32_t) offsetof(SPDUResponse, dwSafetyConsumerId) // offset
    },
    {
        true,                                                 // isBuiltIn
        false,                                                // isArrayLength
        true,                                                 // isToEncode
        (uint32_t) SOPC_UInt32_Id,                            // typeIndex
        (uint32_t) offsetof(SPDUResponse, dwMonitoringNumber) // offset},
    },
    {
        true,                                    // isBuiltIn
        false,                                   // isArrayLength
        true,                                    // isToEncode
        (uint32_t) SOPC_UInt32_Id,               // typeIndex
        (uint32_t) offsetof(SPDUResponse, dwCrc) // offset
    }};

/*===========================================================================*/
SOPC_EncodeableType SPDUResponse_EncodeableType = {
    "SPDUResponse",
    OpcUaId_SPDUResponse,
    OpcUaId_SPDUResponse_Encoding_DefaultBinary,
    OpcUaId_SPDUResponse_Encoding_DefaultXml,
    NULL,
    sizeof(SPDUResponse),
    SPDUResponse_Initialize,
    SPDUResponse_Clear,
    NULL,
    SPDUResponse_Encode,
    SPDUResponse_Decode,
    sizeof SPDUResponse_Fields / sizeof(SOPC_EncodeableType_FieldDescriptor),
    SPDUResponse_Fields};

/*===========================================================================*/
static SOPC_ReturnStatus SPDUResponse_Encode(const void* pValue, SOPC_Buffer* msgBuffer, uint32_t nestedStructLevel)
{
    return SOPC_EncodeableObject_Encode(&SPDUResponse_EncodeableType, pValue, msgBuffer, nestedStructLevel);
}

/*===========================================================================*/
static SOPC_ReturnStatus SPDUResponse_Decode(void* pValue, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_EncodeableObject_Decode(&SPDUResponse_EncodeableType, pValue, buf, nestedStructLevel);
}

/*===========================================================================*/
void SPDUResponse_Initialize(void* pValue)
{
    // SPDUResponse* pzSpdu = (SPDUResponse*)pValue;
    SOPC_EncodeableObject_Initialize(&SPDUResponse_EncodeableType, pValue);
}

/*===========================================================================*/
void SPDUResponse_Clear(void* pValue)
{
    SOPC_EncodeableObject_Clear(&SPDUResponse_EncodeableType, pValue);
}

/*============================================================================
 * LOCAL VARIABLES
 *===========================================================================*/

/** Indicates is the module is set-up */
static bool UAM_SpduEncoder_Initialized = false;

/**
 *  A dictionary { NodeiD* : SPDU_type* }
 *  Used to store configurations (Safe & non safe size) and to free all resources on termination.
 */
static SOPC_Dict* gRequests = NULL;

/*============================================================================
 * IMPLEMENTATION OF EXTERNAL SERVICES
 *===========================================================================*/

/*===========================================================================*/
SOPC_ReturnStatus UAM_SpduEncoder_Initialize(void)
{
    assert(!UAM_SpduEncoder_Initialized);
    SOPC_ReturnStatus bRetval = SOPC_STATUS_OK;

    // Create SPDU request object. Note that SPDUs are allocated and deallocated by
    // SOPC_Encodeable_xxx, so that pointers inside items shall not be freed by dictionary
    gRequests = SOPC_NodeId_Dict_Create(true, SOPC_Free);
    assert(NULL != gRequests);

    bRetval = SOPC_EncodeableType_AddUserType(&SPDURequest_EncodeableType);
    if (SOPC_STATUS_OK == bRetval)
    {
        bRetval = SOPC_EncodeableType_AddUserType(&SPDUResponse_EncodeableType);
    }

    UAM_SpduEncoder_Initialized = (SOPC_STATUS_OK == bRetval);

    return bRetval;
}

/*===========================================================================*/
void UAM_SpduEncoder_Clear(void)
{
    assert(UAM_SpduEncoder_Initialized);
    assert(NULL != gRequests);

    SOPC_Dict_Delete(gRequests);
    gRequests = NULL;

    SOPC_EncodeableType_RemoveUserType(&SPDURequest_EncodeableType);
    SOPC_EncodeableType_RemoveUserType(&SPDUResponse_EncodeableType);
    UAM_SpduEncoder_Initialized = false;
}

/*===========================================================================*/
static SOPC_ReturnStatus UAM_SpduEncoder_CreateSpdu(const UAS_UInt32 dwNumericId,
                                                    SOPC_EncodeableType* pzEncodeabletype,
                                                    SPDU_type* pType)
{
    assert(UAM_SpduEncoder_Initialized);
    assert(NULL != gRequests);
    assert(NULL != pType);
    SOPC_ReturnStatus bRetval = SOPC_STATUS_OK;
    SOPC_DataValue* dv = NULL;
    SOPC_ExtensionObject* ext = NULL;
    SOPC_NodeId* pNid = SOPC_Malloc(sizeof(SOPC_NodeId));
    assert(NULL != pNid);

    SOPC_NodeId_Initialize(pNid);
    pNid->Namespace = UAM_NAMESPACE;
    pNid->IdentifierType = SOPC_IdentifierType_Numeric;
    pNid->Data.Numeric = dwNumericId;

    UAM_Cache_Lock();

    // Set-up value in cache
    dv = UAM_Cache_Get(pNid);
    if (NULL == dv)
    {
        // The numeric value does not match configuration
        bRetval = SOPC_STATUS_INVALID_PARAMETERS;
    }
    else if (dv->Value.BuiltInTypeId != SOPC_ExtensionObject_Id)
    {
        bRetval = SOPC_STATUS_INVALID_PARAMETERS;
    }
    else if (SOPC_Dict_GetKey(gRequests, pNid, NULL) != NULL)
    {
        // The key has already been inserted.
        bRetval = SOPC_STATUS_NOT_SUPPORTED;
    }
    else
    {
        ext = dv->Value.Value.ExtObject;
        SOPC_ExtensionObject_Initialize(ext);

        // Associate the extension value of cache to an encoder
        bRetval = SOPC_Encodeable_CreateExtension(ext, pzEncodeabletype, &pType->Data.pzData);
        if (SOPC_STATUS_OK == bRetval)
        {
            SOPC_Dict_Insert(gRequests, pNid, (void*) pType);
        }
    }
    UAM_Cache_Unlock();

    return bRetval;
}

/*===========================================================================*/
SOPC_ReturnStatus UAM_SpduEncoder_CreateSpduRequest(const UAM_SpduRequestHandle dwNumericId)
{
    SPDU_type* pzSpdu = SOPC_Malloc(sizeof(*pzSpdu));
    assert(NULL != pzSpdu);
    pzSpdu->isRequest = true;
    pzSpdu->Data.pzData = NULL;
    pzSpdu->wNonSafetyDataLength = 0;
    pzSpdu->wSafetyDataLength = 0;
    return UAM_SpduEncoder_CreateSpdu((UAS_UInt32) dwNumericId, &SPDURequest_EncodeableType, pzSpdu);
}

/*===========================================================================*/
SOPC_ReturnStatus UAM_SpduEncoder_CreateSpduResponse(const UAM_SpduResponseHandle dwNumericId,
                                                     const UAS_UInt16 safeDataLength,
                                                     const UAS_UInt16 unsafeDataLength)
{
    SPDU_type* pzSpdu = SOPC_Malloc(sizeof(*pzSpdu));
    assert(NULL != pzSpdu);
    pzSpdu->isRequest = true;
    pzSpdu->Data.pzData = NULL;
    pzSpdu->wNonSafetyDataLength = unsafeDataLength;
    pzSpdu->wSafetyDataLength = safeDataLength;
    return UAM_SpduEncoder_CreateSpdu((UAS_UInt32) dwNumericId, &SPDUResponse_EncodeableType, pzSpdu);
}

/*===========================================================================*/
SOPC_ReturnStatus UAM_SpduEncoder_SetRequest(const UAM_SpduRequestHandle dwNumericId,
                                             const UAS_RequestSpdu_type* pzSpdu)
{
    assert(UAM_SpduEncoder_Initialized);
    assert(NULL != pzSpdu);

    SOPC_DataValue* pDv = NULL;
    SOPC_NodeId nid;
    SOPC_ReturnStatus bRetval = SOPC_STATUS_INVALID_PARAMETERS;

    SOPC_NodeId_Initialize(&nid);
    nid.Namespace = UAM_NAMESPACE;
    nid.IdentifierType = SOPC_IdentifierType_Numeric;
    nid.Data.Numeric = (UAS_UInt32) dwNumericId;
    UAM_Cache_Lock();

    pDv = UAM_Cache_Get(&nid);
    if (pDv != NULL && pDv->Value.BuiltInTypeId == SOPC_ExtensionObject_Id &&
        pDv->Value.Value.ExtObject->Encoding == SOPC_ExtObjBodyEncoding_Object &&
        pDv->Value.Value.ExtObject->Body.Object.ObjType == &SPDURequest_EncodeableType)
    {
        SPDURequest* pReq = (SPDURequest*) (pDv->Value.Value.ExtObject->Body.Object.Value);
        assert(pReq != NULL);
        bRetval = SOPC_STATUS_OK;
        pReq->zSpdu = *pzSpdu;
    }

    UAM_Cache_Unlock();
    SOPC_NodeId_Clear(&nid);

    return bRetval;
}

/*===========================================================================*/
SOPC_ReturnStatus UAM_SpduEncoder_GetRequest(const UAM_SpduRequestHandle dwNumericId, UAS_RequestSpdu_type* pzSpdu)
{
    assert(UAM_SpduEncoder_Initialized);
    assert(NULL != pzSpdu);

    SOPC_DataValue* pDv = NULL;
    SOPC_NodeId nid;
    SOPC_ReturnStatus bRetval = SOPC_STATUS_INVALID_PARAMETERS;

    SOPC_NodeId_Initialize(&nid);
    nid.Namespace = UAM_NAMESPACE;
    nid.IdentifierType = SOPC_IdentifierType_Numeric;
    nid.Data.Numeric = dwNumericId;
    UAM_Cache_Lock();
    pDv = UAM_Cache_Get(&nid);
    if (pDv != NULL && pDv->Value.BuiltInTypeId == SOPC_ExtensionObject_Id &&
        pDv->Value.Value.ExtObject->Encoding == SOPC_ExtObjBodyEncoding_Object &&
        pDv->Value.Value.ExtObject->Body.Object.ObjType == &SPDURequest_EncodeableType)
    {
        SPDURequest* pReq = (SPDURequest*) (pDv->Value.Value.ExtObject->Body.Object.Value);
        *pzSpdu = pReq->zSpdu;
        bRetval = SOPC_STATUS_OK;
    }
    UAM_Cache_Unlock();
    SOPC_NodeId_Clear(&nid);

    return bRetval;
}

/*===========================================================================*/
SOPC_ReturnStatus UAM_SpduEncoder_SetResponse(const UAM_SpduResponseHandle dwNumericId,
                                              const UAS_ResponseSpdu_type* pzSpdu)
{
    assert(UAM_SpduEncoder_Initialized);
    assert(NULL != pzSpdu);

    SOPC_DataValue* pDv = NULL;
    SPDU_type* pzSpduType = NULL;
    SOPC_NodeId nid;
    SOPC_ReturnStatus bRetval = SOPC_STATUS_INVALID_PARAMETERS;

    SOPC_NodeId_Initialize(&nid);
    nid.Namespace = UAM_NAMESPACE;
    nid.IdentifierType = SOPC_IdentifierType_Numeric;
    nid.Data.Numeric = (UAS_UInt32) dwNumericId;
    UAM_Cache_Lock();

    pDv = UAM_Cache_Get(&nid);
    pzSpduType = SOPC_Dict_Get(gRequests, &nid, NULL);

    if (pzSpduType != NULL && pDv != NULL && pDv->Value.BuiltInTypeId == SOPC_ExtensionObject_Id &&
        pDv->Value.Value.ExtObject->Encoding == SOPC_ExtObjBodyEncoding_Object &&
        pDv->Value.Value.ExtObject->Body.Object.ObjType == &SPDUResponse_EncodeableType)
    {
        SPDUResponse* pReq = (SPDUResponse*) (pDv->Value.Value.ExtObject->Body.Object.Value);
        assert(pReq != NULL);
        bRetval = SOPC_STATUS_OK;

        // Retreive and copy content of request into user-level structure.
        pReq->byFlags = pzSpdu->byFlags;
        pReq->zSpduId = pzSpdu->zSpduId;
        pReq->dwSafetyConsumerId = pzSpdu->dwSafetyConsumerId;
        pReq->dwMonitoringNumber = pzSpdu->dwMonitoringNumber;
        pReq->dwCrc = pzSpdu->dwCrc;
        // Convert Safety Data ByteString
        SOPC_ByteString_Clear(&pReq->zSerializedSafetyData);
        bRetval = SOPC_ByteString_InitializeFixedSize(&pReq->zSerializedSafetyData, pzSpduType->wSafetyDataLength);
        assert(bRetval == SOPC_STATUS_OK);
        memcpy(pReq->zSerializedSafetyData.Data, pzSpdu->pbySerializedSafetyData, pzSpduType->wSafetyDataLength);

        // Convert NonSafety Data ByteString
        SOPC_ByteString_Clear(&pReq->zSerializedNonSafetyData);
        bRetval =
            SOPC_ByteString_InitializeFixedSize(&pReq->zSerializedNonSafetyData, pzSpduType->wNonSafetyDataLength);
        assert(bRetval == SOPC_STATUS_OK);
        memcpy(pReq->zSerializedNonSafetyData.Data, pzSpdu->pbySerializedNonSafetyData,
               pzSpduType->wNonSafetyDataLength);
    }

    UAM_Cache_Unlock();
    SOPC_NodeId_Clear(&nid);

    return bRetval;
}

/*===========================================================================*/
SOPC_ReturnStatus UAM_SpduEncoder_GetResponse(const UAM_SpduResponseHandle dwNumericId, UAS_ResponseSpdu_type* pzSpdu)
{
    assert(UAM_SpduEncoder_Initialized);
    assert(NULL != pzSpdu);

    SOPC_DataValue* pDv = NULL;
    SPDU_type* pzSpduType = NULL;
    SOPC_NodeId nid;
    SOPC_ReturnStatus bRetval = SOPC_STATUS_INVALID_PARAMETERS;

    SOPC_NodeId_Initialize(&nid);
    nid.Namespace = UAM_NAMESPACE;
    nid.IdentifierType = SOPC_IdentifierType_Numeric;
    nid.Data.Numeric = dwNumericId;
    UAM_Cache_Lock();
    pDv = UAM_Cache_Get(&nid);
    pzSpduType = SOPC_Dict_Get(gRequests, &nid, NULL);

    if (pzSpduType != NULL && pDv != NULL && pDv->Value.BuiltInTypeId == SOPC_ExtensionObject_Id &&
        pDv->Value.Value.ExtObject->Encoding == SOPC_ExtObjBodyEncoding_Object &&
        pDv->Value.Value.ExtObject->Body.Object.ObjType == &SPDUResponse_EncodeableType &&
        NULL != pzSpdu->pbySerializedSafetyData && NULL != pzSpdu->pbySerializedNonSafetyData)
    {
        SPDUResponse* pResp = (SPDUResponse*) (pDv->Value.Value.ExtObject->Body.Object.Value);

        pzSpdu->byFlags = pResp->byFlags;
        pzSpdu->zSpduId = pResp->zSpduId;
        pzSpdu->dwSafetyConsumerId = pResp->dwSafetyConsumerId;
        pzSpdu->dwMonitoringNumber = pResp->dwMonitoringNumber;
        pzSpdu->dwCrc = pResp->dwCrc;

        bRetval = SOPC_STATUS_OK;

        // Also check that the received buffer lengths match the expected value provided in
        // UAM_SpduEncoder_CreateSpduResponse
        if (pResp->zSerializedSafetyData.Length == pzSpduType->wSafetyDataLength)
        {
            memcpy(pzSpdu->pbySerializedSafetyData, pResp->zSerializedSafetyData.Data,
                   (size_t) pResp->zSerializedSafetyData.Length);
        }
        else
        {
            bRetval = SOPC_STATUS_NOK;
        }
        if (pResp->zSerializedNonSafetyData.Length == pzSpduType->wNonSafetyDataLength)
        {
            // TODO : allow a dynamic-length feature for non-safe data?
            memcpy(pzSpdu->pbySerializedNonSafetyData, pResp->zSerializedNonSafetyData.Data,
                   (size_t) pResp->zSerializedNonSafetyData.Length);
        }
        else
        {
            bRetval = SOPC_STATUS_NOK;
        }
    }
    UAM_Cache_Unlock();
    SOPC_NodeId_Clear(&nid);

    return bRetval;
}
