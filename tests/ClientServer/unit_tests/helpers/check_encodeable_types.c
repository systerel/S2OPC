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
 * \brief Entry point for encodeable types tests. Tests use libcheck.
 *
 * If you want to debug the exe, you should define env var CK_FORK=no
 * http://check.sourceforge.net/doc/check_html/check_4.html#No-Fork-Mode
 */

#include <check.h>

#include "check_helpers.h"

#include "custom2_types.h"
#include "custom_types.h"
#include "opcua_Custom2_identifiers.h"
#include "opcua_Custom_identifiers.h"
#include "opcua_identifiers.h"
#include "sopc_encodeabletype.h"
#include "sopc_encoder.h"
#include "sopc_helper_endianness_cfg.h"
#include "sopc_mem_alloc.h"
#include "sopc_types.h"

static void setup(void)
{
    SOPC_Helper_Endianness_Check();
}

/**
 * Checks that the status is OK.
 */
static void ck_assert_ok(SOPC_ReturnStatus status)
{
    ck_assert(SOPC_STATUS_OK == status);
}

/**
 * Checks that the object carries the expected encodeable type.
 */
#define ck_assert_encodeable_type(obj, type) ck_assert_ptr_eq((obj)->encodeableType, &type##_EncodeableType)

/******************************************************************************
 * Generic checker for encodeable types
 * Test of Initialize, Decode and Encode functions
 * The type and the checker function are received as parameters, as well as the
 * buffer to be decoded.
 ******************************************************************************/
static void checkEncodeableType(SOPC_EncodeableType* encType,
                                void (*encodeableTypeChecker)(const void*),
                                uint8_t* frame,
                                uint32_t frameSize)
{
    // Allocation
    void* obj = NULL;
    SOPC_Buffer* input = NULL;
    SOPC_Buffer* output = NULL;

    // Buffer initialization
    input = SOPC_Buffer_Create(frameSize);
    output = SOPC_Buffer_Create(frameSize);

    ck_assert_ok(SOPC_Buffer_Write(input, frame, frameSize));
    ck_assert_ok(SOPC_Buffer_SetPosition(input, 0));

    // Initialization
    SOPC_ReturnStatus status = SOPC_EncodeableObject_Create(encType, &obj);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_ptr_nonnull(obj);

    // Decode
    ck_assert_ok(SOPC_EncodeableObject_Decode(encType, obj, input, 0));

    // Check object content
    encodeableTypeChecker(obj);

    // Encode
    ck_assert_ok(SOPC_EncodeableObject_Encode(encType, obj, output, 0));

    // Check buffers
    ck_assert_uint_eq(output->position, frameSize);
    ck_assert_mem_eq(input->data, output->data, frameSize);

    // Clear all objects
    SOPC_EncodeableObject_Delete(encType, &obj);
    SOPC_Buffer_Delete(input);
    SOPC_Buffer_Delete(output);
}

/******************************************************************************
 * TimeZoneDataType unitary test
 ******************************************************************************/

static void check_TimeZoneDataType(const void* untypedObject)
{
    const OpcUa_TimeZoneDataType* obj = untypedObject;

    ck_assert_encodeable_type(obj, OpcUa_TimeZoneDataType);
    ck_assert_int_eq(obj->Offset, -1);
    ck_assert_int_eq(obj->DaylightSavingInOffset, true);
}

START_TEST(test_TimeZoneDataType)
{
    uint8_t frame[] = {
        0xFF, 0xFF, // Offset == -1
        0x01        // DaylightSavingInOffset == true
    };

    checkEncodeableType(&OpcUa_TimeZoneDataType_EncodeableType, check_TimeZoneDataType, frame, (uint32_t) sizeof frame);
}
END_TEST

/******************************************************************************
 * AggregateFilterResult unitary test
 ******************************************************************************/

static void check_AggregateFilterResult(const void* untypedObject)
{
    const OpcUa_AggregateFilterResult* obj = untypedObject;
    const OpcUa_AggregateConfiguration* config = NULL;

    ck_assert_encodeable_type(obj, OpcUa_AggregateFilterResult);
    ck_assert_int_eq(obj->RevisedStartTime, -1);
    ck_assert_double_eq(obj->RevisedProcessingInterval, 6.5);

    config = &obj->RevisedAggregateConfiguration;
    ck_assert_encodeable_type(config, OpcUa_AggregateConfiguration);
    ck_assert_int_eq(config->UseServerCapabilitiesDefaults, true);
    ck_assert_int_eq(config->TreatUncertainAsBad, false);
    ck_assert_uint_eq(config->PercentDataBad, 0x2A);
    ck_assert_uint_eq(config->PercentDataGood, 0x3A);
    ck_assert_int_eq(config->UseSlopedExtrapolation, true);
}

START_TEST(test_AggregateFilterResult)
{
    uint8_t frame[] = {
        // RevisedStartTime == -1
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,

        // RevisedProcessingInterval == 6.5
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1A, 0x40,

        // AggregateConfiguration:
        0x01, // UseServerCapabilitiesDefaults == true
        0x00, // TreatUncertainAsBad           == false
        0x2A, // PercentDataBad                == 0x2A
        0x3A, // PercentDataGood               == 0x3A
        0x01  // UseSlopedExtrapolation        == true
    };

    checkEncodeableType(&OpcUa_AggregateFilterResult_EncodeableType, check_AggregateFilterResult, frame,
                        (uint32_t) sizeof frame);
}
END_TEST

/******************************************************************************
 * BrowsePath unitary test
 ******************************************************************************/

static void check_BrowsePath(const void* untypedObject)
{
    const OpcUa_BrowsePath* obj = untypedObject;
    const OpcUa_RelativePath* path = NULL;
    const OpcUa_RelativePathElement* elem = NULL;

    ck_assert_encodeable_type(obj, OpcUa_BrowsePath);
    ck_assert_int_eq(obj->StartingNode.IdentifierType, SOPC_IdentifierType_Numeric);
    ck_assert_int_eq(obj->StartingNode.Namespace, 0x02);
    ck_assert_uint_eq(obj->StartingNode.Data.Numeric, 0x0403);

    path = &obj->RelativePath;
    ck_assert_encodeable_type(path, OpcUa_RelativePath);
    ck_assert_int_eq(path->NoOfElements, 2);
    ck_assert_ptr_nonnull(path->Elements);

    elem = &path->Elements[0];
    ck_assert_encodeable_type(elem, OpcUa_RelativePathElement);
    ck_assert_int_eq(elem->ReferenceTypeId.IdentifierType, SOPC_IdentifierType_Numeric);
    ck_assert_int_eq(elem->ReferenceTypeId.Namespace, 0x05);
    ck_assert_uint_eq(elem->ReferenceTypeId.Data.Numeric, 0x0706);
    ck_assert_int_eq(elem->IsInverse, false);
    ck_assert_int_eq(elem->IncludeSubtypes, true);
    ck_assert_uint_eq(elem->TargetName.NamespaceIndex, 0x0908);
    ck_assert_int_eq(elem->TargetName.Name.Length, -1);
    ck_assert_int_eq(elem->TargetName.Name.DoNotClear, false);
    ck_assert_ptr_null(elem->TargetName.Name.Data);

    elem = &path->Elements[1];
    ck_assert_encodeable_type(elem, OpcUa_RelativePathElement);
    ck_assert_int_eq(elem->ReferenceTypeId.IdentifierType, SOPC_IdentifierType_Numeric);
    ck_assert_int_eq(elem->ReferenceTypeId.Namespace, 0x0A);
    ck_assert_uint_eq(elem->ReferenceTypeId.Data.Numeric, 0x0C0B);
    ck_assert_int_eq(elem->IsInverse, true);
    ck_assert_int_eq(elem->IncludeSubtypes, false);
    ck_assert_uint_eq(elem->TargetName.NamespaceIndex, 0x0E0D);
    ck_assert_int_eq(elem->TargetName.Name.Length, 4);
    ck_assert_int_eq(elem->TargetName.Name.DoNotClear, false);
    ck_assert_str_eq((char*) elem->TargetName.Name.Data, "mugu");
}

START_TEST(test_BrowsePath)
{
    uint8_t frame[] = {
        // BrowsePath->StartingNodeId
        0x01,       // Four byte encoding of NodeId
        0x02,       // Namespace == 2
        0x03, 0x04, // Identifier == 0x0403

        // BrowsePath->RelativePath
        0x02, 0x00, 0x00, 0x00, // NoOfElements == 2

        // BrowsePath->RelativePath.Element[0]
        // ReferenceTypeId
        0x01,       // Four byte encoding of NodeId
        0x05,       // Namespace == 5
        0x06, 0x07, // Identifier == 0x0706

        0x00, // IsInverse == false
        0x01, // IncludeSubtypes == true

        // TargetName
        0x08, 0x09,             // NamespaceIndex == 0x0908
        0xff, 0xff, 0xff, 0xff, // Name (null)

        // BrowsePath->RelativePath.Element[1]
        // ReferenceTypeId
        0x01,       // Four byte encoding of NodeId
        0x0A,       // Namespace == 0x0A
        0x0B, 0x0C, // Identifier == 0x0C0B

        0x01, // IsInverse == true
        0x00, // IncludeSubtypes == false

        // TargetName
        0x0D, 0x0E,             // NamespaceIndex = 0x0E0D
        0x04, 0x00, 0x00, 0x00, // Name length == 4
        0x6D, 0x75, 0x67, 0x75  // Name Data == "mugu"
    };

    checkEncodeableType(&OpcUa_BrowsePath_EncodeableType, check_BrowsePath, frame, (uint32_t) sizeof frame);
}
END_TEST

/******************************************************************************
 * DeleteSubscriptionsRequest unitary test
 ******************************************************************************/

static void check_DeleteSubscriptionsRequest(const void* untypedObject)
{
    const OpcUa_DeleteSubscriptionsRequest* obj = untypedObject;

    ck_assert_encodeable_type(obj, OpcUa_DeleteSubscriptionsRequest);
    ck_assert_int_eq(obj->NoOfSubscriptionIds, 2);
    ck_assert_uint_eq(obj->SubscriptionIds[0], 5);
    ck_assert_uint_eq(obj->SubscriptionIds[1], 10);
}

START_TEST(test_DeleteSubscriptionsRequest)
{
    // Test frame creation (with cursor position reset)
    uint8_t frame[] = {
        0x02, 0x00, 0x00, 0x00, // NoOfSubscriptionIds == 2
        0x05, 0x00, 0x00, 0x00, // SubscriptionsIds[0] == 5
        0x0A, 0x00, 0x00, 0x00  // SubscriptionsIds[1] == 10
    };

    checkEncodeableType(&OpcUa_DeleteSubscriptionsRequest_EncodeableType, check_DeleteSubscriptionsRequest, frame,
                        (uint32_t) sizeof frame);
}
END_TEST

/******************************************************************************
 * FindServersResponse unitary test
 ******************************************************************************/

static void check_TranslateBrowsePathsToNodeIdsRequest(const void* untypedObject)
{
    const OpcUa_TranslateBrowsePathsToNodeIdsRequest* obj = untypedObject;
    const OpcUa_BrowsePath* browsePath = NULL;
    const OpcUa_RelativePath* path = NULL;
    const OpcUa_RelativePathElement* elem = NULL;

    ck_assert_encodeable_type(obj, OpcUa_TranslateBrowsePathsToNodeIdsRequest);

    ck_assert_int_eq(obj->NoOfBrowsePaths, 2);
    ck_assert_ptr_nonnull(obj->BrowsePaths);

    // BrowsePaths[0]
    browsePath = &obj->BrowsePaths[0];
    ck_assert_encodeable_type(browsePath, OpcUa_BrowsePath);
    ck_assert_int_eq(browsePath->StartingNode.IdentifierType, SOPC_IdentifierType_Numeric);
    ck_assert_int_eq(browsePath->StartingNode.Namespace, 0x02);
    ck_assert_uint_eq(browsePath->StartingNode.Data.Numeric, 0x0605);

    // BrowsePaths[0].RelativePath
    path = &browsePath->RelativePath;
    ck_assert_encodeable_type(path, OpcUa_RelativePath);
    ck_assert_int_eq(path->NoOfElements, 2);
    ck_assert_ptr_nonnull(path->Elements);

    // BrowsePaths[0].RelativePath.Elements[0]
    elem = &path->Elements[0];
    ck_assert_encodeable_type(elem, OpcUa_RelativePathElement);
    ck_assert_int_eq(elem->ReferenceTypeId.IdentifierType, SOPC_IdentifierType_Numeric);
    ck_assert_int_eq(elem->ReferenceTypeId.Namespace, 0x05);
    ck_assert_uint_eq(elem->ReferenceTypeId.Data.Numeric, 0x0B0A);
    ck_assert_int_eq(elem->IsInverse, false);
    ck_assert_int_eq(elem->IncludeSubtypes, true);
    ck_assert_uint_eq(elem->TargetName.NamespaceIndex, 0x0B0C);
    ck_assert_int_eq(elem->TargetName.Name.Length, -1);
    ck_assert_int_eq(elem->TargetName.Name.DoNotClear, false);
    ck_assert_ptr_null(elem->TargetName.Name.Data);

    // BrowsePaths[0].RelativePath.Elements[1]
    elem = &path->Elements[1];
    ck_assert_encodeable_type(elem, OpcUa_RelativePathElement);
    ck_assert_int_eq(elem->ReferenceTypeId.IdentifierType, SOPC_IdentifierType_Numeric);
    ck_assert_int_eq(elem->ReferenceTypeId.Namespace, 0x0B);
    ck_assert_uint_eq(elem->ReferenceTypeId.Data.Numeric, 0x0D0C);
    ck_assert_int_eq(elem->IsInverse, true);
    ck_assert_int_eq(elem->IncludeSubtypes, false);
    ck_assert_uint_eq(elem->TargetName.NamespaceIndex, 0x0E0D);
    ck_assert_int_eq(elem->TargetName.Name.Length, 4);
    ck_assert_int_eq(elem->TargetName.Name.DoNotClear, false);
    ck_assert_str_eq((char*) elem->TargetName.Name.Data, "mugu");

    // BrowsePaths[1]
    browsePath = &obj->BrowsePaths[1];
    ck_assert_encodeable_type(browsePath, OpcUa_BrowsePath);
    ck_assert_int_eq(browsePath->StartingNode.IdentifierType, SOPC_IdentifierType_Numeric);
    ck_assert_int_eq(browsePath->StartingNode.Namespace, 0x02);
    ck_assert_uint_eq(browsePath->StartingNode.Data.Numeric, 0x0403);

    // BrowsePaths[1].RelativePath
    path = &browsePath->RelativePath;
    ck_assert_encodeable_type(path, OpcUa_RelativePath);
    ck_assert_int_eq(path->NoOfElements, 1);
    ck_assert_ptr_nonnull(path->Elements);

    // BrowsePaths[1].RelativePath.Elements[0]
    elem = &path->Elements[0];
    ck_assert_encodeable_type(elem, OpcUa_RelativePathElement);
    ck_assert_int_eq(elem->ReferenceTypeId.IdentifierType, SOPC_IdentifierType_Numeric);
    ck_assert_int_eq(elem->ReferenceTypeId.Namespace, 0x05);
    ck_assert_uint_eq(elem->ReferenceTypeId.Data.Numeric, 0x0706);
    ck_assert_int_eq(elem->IsInverse, false);
    ck_assert_int_eq(elem->IncludeSubtypes, true);
    ck_assert_uint_eq(elem->TargetName.NamespaceIndex, 0x0908);
    ck_assert_int_eq(elem->TargetName.Name.Length, 3);
    ck_assert_int_eq(elem->TargetName.Name.DoNotClear, false);
    ck_assert_str_eq((const char*) elem->TargetName.Name.Data, "foo");
}

START_TEST(test_TranslateBrowsePathsToNodeIdsRequest)
{
    uint8_t frame[] = {
        0x02, 0x00, 0x00, 0x00, // NoOfBrowsePaths == 2

        // BrowsePaths[0].StartingNodeId
        0x01,       // Four byte encoding of NodeId
        0x02,       // Namespace == 2
        0x05, 0x06, // Identifier == 0x0605

        // BrowsePaths[0].RelativePath
        0x02, 0x00, 0x00, 0x00, // NoOfElements == 2

        // BrowsePaths[0].RelativePath.Elements[0]
        // ...ReferenceTypeId
        0x01,       // Four byte encoding of NodeId
        0x05,       // Namespace == 5
        0x0A, 0x0B, // Identifier == 0x0B0A

        0x00, // IsInverse == false
        0x01, // IncludeSubtypes == true

        // ...TargetName
        0x0C, 0x0B,             // NamespaceIndex == 0x0B0C
        0xff, 0xff, 0xff, 0xff, // Name (null)

        // BrowsePaths[0].RelativePath.Elements[1]
        // ...ReferenceTypeId
        0x01,       // Four byte encoding of NodeId
        0x0B,       // Namespace == 0x0B
        0x0C, 0x0D, // Identifier == 0x0D0C

        0x01, // IsInverse == true
        0x00, // IncludeSubtypes == false

        // ...TargetName
        0x0D, 0x0E,             // NamespaceIndex == 0x0E0D
        0x04, 0x00, 0x00, 0x00, // Name length == 4
        0x6D, 0x75, 0x67, 0x75, // Name Data == "mugu"

        // BrowsePaths[1].StartingNodeId
        0x01,       // Four byte encoding of NodeId
        0x02,       // Namespace == 2
        0x03, 0x04, // Identifier == 0x0403

        // BrowsePaths[1].RelativePath
        0x01, 0x00, 0x00, 0x00, // NoOfElements == 1

        // BrowsePaths[1].RelativePath.Elements[0]
        // ...ReferenceTypeId
        0x01,       // Four byte encoding of NodeId
        0x05,       // Namespace == 5
        0x06, 0x07, // Identifier == 0x0706

        0x00, // IsInverse == false
        0x01, // IncludeSubtypes == true

        // ...TargetName
        0x08, 0x09,             // NamespaceIndex = 0x0908
        0x03, 0x00, 0x00, 0x00, // Name length == 3
        0x66, 0x6F, 0X6F        // Name Data == "foo"
    };

    checkEncodeableType(&OpcUa_TranslateBrowsePathsToNodeIdsRequest_EncodeableType,
                        check_TranslateBrowsePathsToNodeIdsRequest, frame, (uint32_t) sizeof frame);
}
END_TEST

/*===========================================================================*/
/*===========================================================================*/
/*    USER EBCODING TYPES */
/*===========================================================================*/
/*===========================================================================*/

#define OpcUaId_SPDURequest 10000001
#define OpcUaId_SPDURequest_Encoding_DefaultBinary 10000002
#define OpcUaId_SPDURequest_Encoding_DefaultXml 10000003

static void SPDURequest_Initialize(void* pValue);
static void SPDURequest_Clear(void* pValue);

typedef struct UAS_RequestSpdu_struct
{
    /** Safety Consumer Identifier - the identifier of the SafetyConsumer instance. */
    uint32_t dwSafetyConsumerId;
    /** Monitoring Number (MNR) of the RequestSPDU. The SafetyConsumer uses the MNR
     * to detect mistimed SPDUs, e.g. such SPDUs which are continuously repeated
     * by an erroneous network storing element. A different MNR is used in every
     * RequestSPDU of a given SafetyConsumer, and a ResponseSPDU will only be accepted,
     * if its MNR is identical to its matching RequestSPDU. */
    uint32_t dwMonitoringNumber;
    /** Non safety Flags from SafetyConsumer. */
    uint8_t byFlags;
} UAS_RequestSpdu_type;

typedef struct
{
    SOPC_EncodeableType* encodeableType;
    UAS_RequestSpdu_type zSpdu;
} SPDURequest;

/*============================================================================
 * Field descriptors of the SPDURequest encodeable type.
 *===========================================================================*/
static const SOPC_EncodeableType_FieldDescriptor SPDURequest_Fields[] = {
    {
        false,                                                     // isOptional
        true,                                                      // isBuiltIn
        false,                                                     // isArrayLength
        true,                                                      // isToEncode
        true,                                                      // isSameNs
        0,                                                         // nsIndex
        (uint32_t) SOPC_UInt32_Id,                                 // typeIndex
        (uint32_t) offsetof(SPDURequest, zSpdu.dwSafetyConsumerId) // offset
    },
    {
        false,                                                     // isOptional
        true,                                                      // isBuiltIn
        false,                                                     // isArrayLength
        true,                                                      // isToEncode
        true,                                                      // isSameNs
        0,                                                         // nsIndex
        (uint32_t) SOPC_UInt32_Id,                                 // typeIndex
        (uint32_t) offsetof(SPDURequest, zSpdu.dwMonitoringNumber) // offset
    },
    {
        false,                                          // isOptional
        true,                                           // isBuiltIn
        false,                                          // isArrayLength
        true,                                           // isToEncode
        true,                                           // isSameNs
        0,                                              // nsIndex
        (uint32_t) SOPC_Byte_Id,                        // typeIndex
        (uint32_t) offsetof(SPDURequest, zSpdu.byFlags) // offset
    },
};

SOPC_EncodeableType SPDURequest_EncodeableType = {
    "SPDURequest",
    OpcUaId_SPDURequest,
    OpcUaId_SPDURequest_Encoding_DefaultBinary,
    OpcUaId_SPDURequest_Encoding_DefaultXml,
    NULL,
    0,
    sizeof(SPDURequest),
    SPDURequest_Initialize,
    SPDURequest_Clear,
    SOPC_STRUCT_TYPE_CLASSIC,
    sizeof SPDURequest_Fields / sizeof(SOPC_EncodeableType_FieldDescriptor),
    SPDURequest_Fields,
    NULL};

SOPC_EncodeableType SPDURequest_EncodeableType2 = {
    "SPDURequest",
    OpcUaId_ReferenceNode,
    OpcUaId_SPDURequest_Encoding_DefaultBinary,
    OpcUaId_SPDURequest_Encoding_DefaultXml,
    NULL,
    0,
    sizeof(SPDURequest),
    SPDURequest_Initialize,
    SPDURequest_Clear,
    SOPC_STRUCT_TYPE_CLASSIC,
    sizeof SPDURequest_Fields / sizeof(SOPC_EncodeableType_FieldDescriptor),
    SPDURequest_Fields,
    NULL};

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

static void check_SpduRequestDataType(const void* untypedObject)
{
    const SPDURequest* obj = untypedObject;

    ck_assert_encodeable_type(obj, SPDURequest);
    ck_assert_int_eq(obj->zSpdu.byFlags, 0x11);
    ck_assert_int_eq(obj->zSpdu.dwSafetyConsumerId, 0x12345678);
    ck_assert_int_eq(obj->zSpdu.dwMonitoringNumber, 0xABCDEF05);
}

START_TEST(test_UserEncodeableType)
{
    SOPC_ReturnStatus res;
    SOPC_EncodeableType* pEncoder = NULL;
    uint8_t frame[] = {
        0x78, 0x56, 0x34, 0x12, // dwSafetyConsumerId
        0x05, 0xEF, 0xCD, 0xAB, // dwMonitoringNumber
        0x11,                   // byFlags
    };

    // Encoder is not known
    pEncoder = SOPC_EncodeableType_GetEncodeableType(OPCUA_NAMESPACE_INDEX, OpcUaId_SPDURequest);
    ck_assert(pEncoder == NULL);

    // Check default parameters
    res = SOPC_EncodeableType_AddUserType(NULL);
    ck_assert(res == SOPC_STATUS_INVALID_PARAMETERS);

    // Cannot register over a pre-defined type
    res = SOPC_EncodeableType_AddUserType(&SPDURequest_EncodeableType2);
    ck_assert(res == SOPC_STATUS_NOT_SUPPORTED);

    // Cannot unregister a pre-defined type
    res = SOPC_EncodeableType_RemoveUserType(&SPDURequest_EncodeableType2);
    ck_assert(res == SOPC_STATUS_INVALID_PARAMETERS);

    // Register type
    res = SOPC_EncodeableType_AddUserType(&SPDURequest_EncodeableType);
    ck_assert(res == SOPC_STATUS_OK);

    // Cannot unregister a pre-defined type
    res = SOPC_EncodeableType_RemoveUserType(&SPDURequest_EncodeableType2);
    ck_assert(res == SOPC_STATUS_INVALID_PARAMETERS);

    // Encoder is now known
    pEncoder = SOPC_EncodeableType_GetEncodeableType(OPCUA_NAMESPACE_INDEX, OpcUaId_SPDURequest);
    ck_assert(pEncoder == &SPDURequest_EncodeableType);

    checkEncodeableType(pEncoder, check_SpduRequestDataType, frame, (uint32_t) sizeof frame);

    // Cannot register twice the same ID
    res = SOPC_EncodeableType_AddUserType(&SPDURequest_EncodeableType);
    ck_assert(res == SOPC_STATUS_NOT_SUPPORTED);

    res = SOPC_EncodeableType_RemoveUserType(&SPDURequest_EncodeableType);
    ck_assert(res == SOPC_STATUS_OK);

    res = SOPC_EncodeableType_RemoveUserType(&SPDURequest_EncodeableType);
    ck_assert(res == SOPC_STATUS_INVALID_PARAMETERS);

    // Encoder is not known
    pEncoder = SOPC_EncodeableType_GetEncodeableType(OPCUA_NAMESPACE_INDEX, OpcUaId_SPDURequest);
    ck_assert(pEncoder == NULL);
}
END_TEST

START_TEST(test_UserEncodeableTypeNS1)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_EncodeableType* pEncoder = NULL;

    const uint16_t NS_INDEX = 1;

    // Encoder is not registered
    pEncoder = SOPC_EncodeableType_GetEncodeableType(NS_INDEX, OpcUaId_Custom_CustomDataType);
    ck_assert(pEncoder == NULL);
    pEncoder = SOPC_EncodeableType_GetEncodeableType(NS_INDEX, OpcUaId_Custom_CustomDataType_Encoding_DefaultBinary);
    ck_assert(pEncoder == NULL);

    // Create an extension object of an unreferenced encodeable type
    SOPC_ExtensionObject extObj;
    SOPC_ExtensionObject_Initialize(&extObj);

    OpcUa_Custom_CustomDataType* instCDT = NULL;

    status = SOPC_ExtensionObject_CreateObject(&extObj, &OpcUa_Custom_CustomDataType_EncodeableType, (void**) &instCDT);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    instCDT->fieldb = true;
    instCDT->fieldu = 1000;

    // Encode the extension object into a buffer (success since it contains direct reference to encType)
    SOPC_Buffer* buf = SOPC_Buffer_Create(1024);
    ck_assert_ptr_nonnull(buf);
    status = SOPC_ExtensionObject_Write(&extObj, buf, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    // Clear extension object content
    SOPC_ExtensionObject_Clear(&extObj);

    // Try to decode an extension object for unregistered encType: undecoded object retrieved (ByteString)
    status = SOPC_Buffer_SetPosition(buf, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_ExtensionObject_Read(&extObj, buf, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(extObj.Encoding == SOPC_ExtObjBodyEncoding_ByteString);
    // Binary type identifier is the one expected but was not found in known encTypes
    ck_assert_int_le(extObj.TypeId.NamespaceUri.Length, 0);
    ck_assert_int_eq(NS_INDEX, extObj.TypeId.NodeId.Namespace);
    ck_assert_int_eq(SOPC_IdentifierType_Numeric, extObj.TypeId.NodeId.IdentifierType);
    ck_assert_int_eq(OpcUaId_Custom_CustomDataType_Encoding_DefaultBinary, extObj.TypeId.NodeId.Data.Numeric);

    // Get_DataType returns the generic structure type
    SOPC_Variant variant;
    SOPC_NodeId outNodeId;
    SOPC_NodeId_Initialize(&outNodeId);
    SOPC_Variant_Initialize(&variant);
    variant.BuiltInTypeId = SOPC_ExtensionObject_Id;
    variant.Value.ExtObject = &extObj;
    const SOPC_NodeId* typeId = SOPC_Variant_Get_DataType(&variant, &outNodeId);
    ck_assert_ptr_nonnull(typeId);
    ck_assert_int_eq(0, typeId->Namespace);
    ck_assert_int_eq(SOPC_IdentifierType_Numeric, typeId->IdentifierType);
    ck_assert_int_eq(OpcUaId_Structure, typeId->Data.Numeric);
    SOPC_NodeId_Clear(&outNodeId);
    // Record the encodeable type encoder
    for (uint32_t i = 0; SOPC_STATUS_OK == status && i < SOPC_Custom_TypeInternalIndex_SIZE; i++)
    {
        SOPC_EncodeableType* userType = sopc_Custom_KnownEncodeableTypes[i];
        status = SOPC_EncodeableType_AddUserType(userType);
        ck_assert_int_eq(SOPC_STATUS_OK, status);
    }

    // Encoder is registered (both DataType id and binary encoding type id)
    pEncoder = SOPC_EncodeableType_GetEncodeableType(NS_INDEX, OpcUaId_Custom_CustomDataType);
    ck_assert(pEncoder == &OpcUa_Custom_CustomDataType_EncodeableType);
    pEncoder = SOPC_EncodeableType_GetEncodeableType(NS_INDEX, OpcUaId_Custom_CustomDataType_Encoding_DefaultBinary);
    ck_assert(pEncoder == &OpcUa_Custom_CustomDataType_EncodeableType);

    // Reset the buffer position, clear ExtObj and decode buffer again
    status = SOPC_Buffer_SetPosition(buf, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    SOPC_ExtensionObject_Clear(&extObj);
    status = SOPC_ExtensionObject_Read(&extObj, buf, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    // Decoded object retrieved
    ck_assert(extObj.Encoding == SOPC_ExtObjBodyEncoding_Object);
    ck_assert_ptr_eq(&OpcUa_Custom_CustomDataType_EncodeableType, extObj.Body.Object.ObjType);
    // Get_DataType returns the type id
    typeId = SOPC_Variant_Get_DataType(&variant, &outNodeId);
    ck_assert_ptr_nonnull(typeId);
    ck_assert_int_eq(NS_INDEX, typeId->Namespace);
    ck_assert_int_eq(SOPC_IdentifierType_Numeric, typeId->IdentifierType);
    ck_assert_int_eq(OpcUaId_Custom_CustomDataType, typeId->Data.Numeric);
    SOPC_NodeId_Clear(&outNodeId);

    // Remove custom type from recorded encoders
    status = SOPC_EncodeableType_RemoveUserType(&OpcUa_Custom_CustomDataType_EncodeableType);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    SOPC_ExtensionObject_Clear(&extObj);

    /* TC 2 : CustomDataType2 which references CustomDataType (same NS / custom type file) */
    OpcUa_Custom_CustomDataType2 instCDT2;
    OpcUa_Custom_CustomDataType2_Initialize(&instCDT2);

    instCDT2.fieldb = false;
    instCDT2.fieldcdt.fieldb = true;
    instCDT2.fieldcdt.fieldu = 1000;

    // Reset the buffer position and encode into buffer
    status = SOPC_Buffer_SetPosition(buf, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_EncodeableObject_Encode(&OpcUa_Custom_CustomDataType2_EncodeableType, &instCDT2, buf, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    // Reset the buffer position and type instance, then decode from buffer
    status = SOPC_Buffer_SetPosition(buf, 0);
    OpcUa_Custom_CustomDataType2_Clear(&instCDT2);
    ck_assert_uint_eq(0, instCDT2.fieldcdt.fieldu);

    status = SOPC_EncodeableObject_Decode(&OpcUa_Custom_CustomDataType2_EncodeableType, &instCDT2, buf, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_uint_eq(1000, instCDT2.fieldcdt.fieldu);
    ck_assert(true == instCDT2.fieldcdt.fieldb);
    ck_assert(false == instCDT2.fieldb);

    OpcUa_Custom_CustomDataType2_Clear(&instCDT2);

    /* TC 3 : CustomWithNS0DataType which references CustomDataType (same types file) and NS 0 (well known types file)
     */
    OpcUa_Custom_CustomWithNS0DataType instCDTNS0;
    OpcUa_Custom_CustomWithNS0DataType_Initialize(&instCDTNS0);

    instCDTNS0.fieldb = true;
    status = SOPC_String_AttachFromCstring(&instCDTNS0.keyPair.Key.Name, "CustomKey");
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    instCDTNS0.keyPair.Value.BuiltInTypeId = SOPC_Boolean_Id;
    instCDTNS0.keyPair.Value.Value.Boolean = true;

    // Reset the buffer position and encode into buffer
    status = SOPC_Buffer_SetPosition(buf, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_EncodeableObject_Encode(&OpcUa_Custom_CustomWithNS0DataType_EncodeableType, &instCDTNS0, buf, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    // Reset the buffer position and type instance, then decode from buffer
    status = SOPC_Buffer_SetPosition(buf, 0);
    OpcUa_Custom_CustomWithNS0DataType_Clear(&instCDTNS0);
    ck_assert(false == instCDTNS0.fieldb);

    status = SOPC_EncodeableObject_Decode(&OpcUa_Custom_CustomWithNS0DataType_EncodeableType, &instCDTNS0, buf, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(true == instCDTNS0.fieldb);

    int compareRes = strcmp(SOPC_String_GetRawCString(&instCDTNS0.keyPair.Key.Name), "CustomKey");
    ck_assert_int_eq(0, compareRes);
    ck_assert(SOPC_Boolean_Id == instCDTNS0.keyPair.Value.BuiltInTypeId);
    ck_assert(true == instCDTNS0.keyPair.Value.Value.Boolean);

    OpcUa_Custom_CustomWithNS0DataType_Clear(&instCDTNS0);
    SOPC_Buffer_Delete(buf);

    // Unrecord the encodeable type encoders
    SOPC_EncodeableType_RemoveAllUserTypes();
}
END_TEST

START_TEST(test_UserEncodeableTypeNS2)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_EncodeableType* pEncoder = NULL;

    const uint16_t NS_1 = 1;
    const uint16_t NS_2 = 2;
    ck_assert_uint_eq(NS_2, OpcUa_Custom2_CustomWithNS1DataType_EncodeableType.NamespaceIndex);

    // Encoder is not registered
    pEncoder = SOPC_EncodeableType_GetEncodeableType(NS_1, OpcUaId_Custom_CustomDataType);
    ck_assert_ptr_null(pEncoder);

    // Create an extension object of an unreferenced encodeable type
    SOPC_ExtensionObject extObj;
    SOPC_ExtensionObject_Initialize(&extObj);

    OpcUa_Custom2_CustomWithNS1DataType* instCNS1DT = NULL;

    // It is NOT possible to create such object only with the encodeable type
    // because it depends on UNREGISTERED NS1 encodeable types
    status = SOPC_ExtensionObject_CreateObject(&extObj, &OpcUa_Custom2_CustomWithNS1DataType_EncodeableType,
                                               (void**) &instCNS1DT);

    ck_assert_int_eq(SOPC_STATUS_NOT_SUPPORTED, status);
    // Clear extension object content
    SOPC_ExtensionObject_Clear(&extObj);

    // Now registers the whole table of encodeable types for NS1 Custom Types (on which depends NS2)
    status =
        SOPC_EncodeableType_RegisterTypesArray(SOPC_Custom_TypeInternalIndex_SIZE, sopc_Custom_KnownEncodeableTypes);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    // Encoder is registered
    pEncoder = SOPC_EncodeableType_GetEncodeableType(NS_1, OpcUaId_Custom_CustomDataType);
    ck_assert_ptr_nonnull(pEncoder);

    // It is possible to create such object only with the encodeable type
    // because it depends on REGISTERED NS1 encodeable types
    status = SOPC_ExtensionObject_CreateObject(&extObj, &OpcUa_Custom2_CustomWithNS1DataType_EncodeableType,
                                               (void**) &instCNS1DT);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    instCNS1DT->custom2.fieldb = true;
    instCNS1DT->custom2.fieldu = 42;
    instCNS1DT->custom1.fieldb = false;
    status = SOPC_String_AttachFromCstring(&instCNS1DT->custom1.keyPair.Key.Name, "CustomKey");
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    instCNS1DT->custom1.keyPair.Value.BuiltInTypeId = SOPC_Boolean_Id;
    instCNS1DT->custom1.keyPair.Value.Value.Boolean = true;

    // Encode the extension object into a buffer (success since it contains direct reference to encType)
    SOPC_Buffer* buf = SOPC_Buffer_Create(1024);
    ck_assert_ptr_nonnull(buf);
    status = SOPC_ExtensionObject_Write(&extObj, buf, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    // Clear extension object content
    SOPC_ExtensionObject_Clear(&extObj);

    // Try to decode an extension object for UNREGISTERED encType of NS2
    status = SOPC_Buffer_SetPosition(buf, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_ExtensionObject_Read(&extObj, buf, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    // It has not been decoded since unregistered
    ck_assert(extObj.Encoding == SOPC_ExtObjBodyEncoding_ByteString);
    // Binary type identifier is the one expected but was not found in known encTypes
    ck_assert_int_le(extObj.TypeId.NamespaceUri.Length, 0);
    ck_assert_int_eq(NS_2, extObj.TypeId.NodeId.Namespace);
    ck_assert_int_eq(SOPC_IdentifierType_Numeric, extObj.TypeId.NodeId.IdentifierType);
    ck_assert_int_eq(OpcUaId_Custom2_CustomWithNS1DataType_Encoding_DefaultBinary, extObj.TypeId.NodeId.Data.Numeric);

    // Encodeable type is not registered
    pEncoder = SOPC_EncodeableType_GetEncodeableType(NS_2, OpcUaId_Custom2_CustomWithNS1DataType);
    ck_assert_ptr_null(pEncoder);
    pEncoder =
        SOPC_EncodeableType_GetEncodeableType(NS_2, OpcUaId_Custom2_CustomWithNS1DataType_Encoding_DefaultBinary);
    ck_assert_ptr_null(pEncoder);

    // Now registers the whole table of encodeable types for NS2 Custom Types to be able to decode it
    status =
        SOPC_EncodeableType_RegisterTypesArray(SOPC_Custom2_TypeInternalIndex_SIZE, sopc_Custom2_KnownEncodeableTypes);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    pEncoder = SOPC_EncodeableType_GetEncodeableType(NS_2, OpcUaId_Custom2_CustomWithNS1DataType);
    ck_assert_ptr_nonnull(pEncoder);

    // Try to decode an extension object for REGISTERED encType
    status = SOPC_Buffer_SetPosition(buf, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    SOPC_ExtensionObject_Clear(&extObj);
    status = SOPC_ExtensionObject_Read(&extObj, buf, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    // It has been decoded since it is now registered
    ck_assert(extObj.Encoding == SOPC_ExtObjBodyEncoding_Object);
    ck_assert_ptr_eq(&OpcUa_Custom2_CustomWithNS1DataType_EncodeableType, extObj.Body.Object.ObjType);

    instCNS1DT = (OpcUa_Custom2_CustomWithNS1DataType*) extObj.Body.Object.Value;
    ck_assert(true == instCNS1DT->custom2.fieldb);
    ck_assert_int_eq(instCNS1DT->custom2.fieldu, 42);
    ck_assert(false == instCNS1DT->custom1.fieldb);
    int compareRes = strcmp(SOPC_String_GetRawCString(&instCNS1DT->custom1.keyPair.Key.Name), "CustomKey");
    ck_assert_int_eq(0, compareRes);
    ck_assert(SOPC_Boolean_Id == instCNS1DT->custom1.keyPair.Value.BuiltInTypeId);
    ck_assert(true == instCNS1DT->custom1.keyPair.Value.Value.Boolean);
    // Clear the extension object prior to unregister NS1 custom type,
    // otherwise clear on nested custom NS1 type will be ignored after unregister.
    SOPC_ExtensionObject_Clear(&extObj);

    // Now unregisters the whole table of encodeable types for NS1 Custom Types
    status =
        SOPC_EncodeableType_UnRegisterTypesArray(SOPC_Custom_TypeInternalIndex_SIZE, sopc_Custom_KnownEncodeableTypes);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    // Reset the buffer position and decode buffer again but with NS1 unregistered
    status = SOPC_Buffer_SetPosition(buf, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_ExtensionObject_Read(&extObj, buf, 0);

    // The decoding failed because NS2 is recorded but not NS1 !!!
    ck_assert_int_eq(SOPC_STATUS_NOT_SUPPORTED, status);

    // Now unregisters the whole table of encodeable types for NS2 Custom Types
    status = SOPC_EncodeableType_UnRegisterTypesArray(SOPC_Custom2_TypeInternalIndex_SIZE,
                                                      sopc_Custom2_KnownEncodeableTypes);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    SOPC_Buffer_Delete(buf);
}
END_TEST

START_TEST(test_EncodeAndDecode_ToAndFrom_ByteString)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    OpcUa_EnumField enumFieldSrc = {0};
    OpcUa_EnumField enumFieldDest = {0};
    OpcUa_EnumField_Initialize(&enumFieldSrc);
    OpcUa_EnumField_Initialize(&enumFieldDest);
    enumFieldSrc.Value = 64;
    SOPC_ByteString* destByteString = SOPC_ByteString_Create();
    ck_assert_ptr_nonnull(destByteString);

    status = SOPC_EncodeableObject_EncodeToByteString(enumFieldSrc.encodeableType, (const void*) &enumFieldSrc,
                                                      destByteString);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_ptr_nonnull(destByteString->Data);

    status = SOPC_EncodeableObject_DecodeFromByteString(enumFieldDest.encodeableType, destByteString,
                                                        (void*) &enumFieldDest);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq(enumFieldSrc.Value, enumFieldDest.Value);

    /* Decode again to compare memory (to check all fields) */
    SOPC_ByteString* newByteString = SOPC_ByteString_Create();
    ck_assert_ptr_nonnull(newByteString);

    status = SOPC_EncodeableObject_EncodeToByteString(enumFieldDest.encodeableType, (const void*) &enumFieldDest,
                                                      newByteString);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_ptr_nonnull(destByteString->Data);
    ck_assert_int_eq(destByteString->Length, newByteString->Length);

    int cmpRes = memcmp(destByteString->Data, newByteString->Data, (size_t) destByteString->Length);
    ck_assert_int_eq(0, cmpRes);

    SOPC_EncodeableObject_Clear(enumFieldDest.encodeableType, &enumFieldDest);
    SOPC_ByteString_Delete(destByteString);
    SOPC_ByteString_Delete(newByteString);
}
END_TEST

static bool CheckEncodedBuffer(SOPC_Buffer* bufToCheck, uint32_t bufLengthExpected, const uint8_t* bufExpected)
{
    SOPC_Buffer_SetPosition(bufToCheck, 0);
    uint8_t buf[100] = {0};
    SOPC_Buffer_Read((uint8_t*) buf, bufToCheck, bufToCheck->length);
    bool res = bufToCheck->length == bufLengthExpected ? true : false;
    for (int i = 0; i < (int) bufToCheck->length && res; i++)
    {
        res = (buf[i] == bufExpected[i]);
    }
    return res;
}

START_TEST(test_UserEncodeableType_Union)
{
    SOPC_ReturnStatus status =
        SOPC_EncodeableType_RegisterTypesArray(SOPC_Custom_TypeInternalIndex_SIZE, sopc_Custom_KnownEncodeableTypes);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    /*** CREATE/INIT and CLEAR ***/
    SOPC_ExtensionObject extObjUnion;
    SOPC_ExtensionObject_Initialize(&extObjUnion);
    OpcUa_Custom_CustomUnionDataType* instCDT_Union = NULL;
    status = SOPC_ExtensionObject_CreateObject(&extObjUnion, &OpcUa_Custom_CustomUnionDataType_EncodeableType,
                                               (void**) &instCDT_Union);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    SOPC_ExtensionObject_Clear(&extObjUnion);

    /*** ENCODE/DECODE***/
    SOPC_Buffer* buf = SOPC_Buffer_Create(1024);
    OpcUa_Custom_CustomUnionDataType instCDT1_Union, instCDT2_Union;
    OpcUa_Custom_CustomUnionDataType_Initialize(&instCDT1_Union);
    OpcUa_Custom_CustomUnionDataType_Initialize(&instCDT2_Union);
    instCDT1_Union.SwitchField = 2;
    instCDT1_Union.Value.FieldUint16 = UINT16_MAX;

    // Encode
    status = SOPC_EncodeableObject_Encode(&OpcUa_Custom_CustomUnionDataType_EncodeableType, &instCDT1_Union, buf, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    // Union SwitchField = 2 (field 2 selected)
    uint8_t expectedEncode[6] = {0x2, 0x0, 0x0, 0x0, 0xff, 0xff};
    ck_assert(CheckEncodedBuffer(buf, 6, expectedEncode));

    // Reset the buffer position and type instance, then decode from buffer
    OpcUa_Custom_CustomUnionDataType_Clear(&instCDT1_Union);
    ck_assert_uint_eq(0, instCDT1_Union.Value.FieldUint16);
    status = SOPC_Buffer_SetPosition(buf, 0);

    // Decode
    status = SOPC_EncodeableObject_Decode(&OpcUa_Custom_CustomUnionDataType_EncodeableType, &instCDT1_Union, buf, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_uint_eq(2, instCDT1_Union.SwitchField);
    ck_assert_uint_eq(UINT16_MAX, instCDT1_Union.Value.FieldUint16);

    // Encode cleared Union
    OpcUa_Custom_CustomUnionDataType_Clear(&instCDT1_Union);
    ck_assert_uint_eq(0, instCDT1_Union.SwitchField);
    ck_assert_uint_eq(0, instCDT1_Union.Value.FieldUint16);
    SOPC_Buffer_Reset(buf);
    status = SOPC_EncodeableObject_Encode(&OpcUa_Custom_CustomUnionDataType_EncodeableType, &instCDT1_Union, buf, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    uint8_t expectedClearedUnion[4] = {0x0, 0x0, 0x0, 0x0}; // Only switch field set to 0
    ck_assert(CheckEncodedBuffer(buf, 4, expectedClearedUnion));

    // Encode Union with an overly large switch field (= 3 for only 2 fields in the union)
    instCDT1_Union.SwitchField = 3;
    status = SOPC_EncodeableObject_Encode(&OpcUa_Custom_CustomUnionDataType_EncodeableType, &instCDT1_Union, buf, 0);
    ck_assert_int_eq(SOPC_STATUS_ENCODING_ERROR, status);

    /*** COPY ***/
    instCDT1_Union.SwitchField = 2;
    instCDT1_Union.Value.FieldUint16 = UINT16_MAX;
    OpcUa_Custom_CustomUnionDataType_Initialize(&instCDT2_Union);
    status =
        SOPC_EncodeableObject_Copy(&OpcUa_Custom_CustomUnionDataType_EncodeableType, &instCDT2_Union, &instCDT1_Union);
    // Check
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_uint_eq(2, instCDT1_Union.SwitchField);
    ck_assert_uint_eq(UINT16_MAX, instCDT2_Union.Value.FieldUint16);
    ck_assert_uint_eq(2, instCDT2_Union.SwitchField);
    ck_assert_uint_eq(UINT16_MAX, instCDT2_Union.Value.FieldUint16);

    /*** MOVE ***/
    OpcUa_Custom_CustomUnionDataType_Initialize(&instCDT2_Union);
    status = SOPC_EncodeableObject_Move(&instCDT2_Union, &instCDT1_Union);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    // Check
    ck_assert_uint_eq(0, instCDT1_Union.SwitchField);
    ck_assert_uint_eq(0, instCDT1_Union.Value.FieldUint16);
    ck_assert_uint_eq(2, instCDT2_Union.SwitchField);
    ck_assert_uint_eq(UINT16_MAX, instCDT2_Union.Value.FieldUint16);

    /*** COMPARE ***/
    int32_t resCmp = 0;
    status = SOPC_EncodeableObject_Compare(&OpcUa_Custom_CustomUnionDataType_EncodeableType, &instCDT1_Union,
                                           &instCDT2_Union, &resCmp);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq(-1, resCmp);

    instCDT1_Union.SwitchField = 2;
    instCDT1_Union.Value.FieldUint16 = UINT16_MAX;
    status = SOPC_EncodeableObject_Compare(&OpcUa_Custom_CustomUnionDataType_EncodeableType, &instCDT1_Union,
                                           &instCDT2_Union, &resCmp);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq(0, resCmp);

    instCDT2_Union.Value.FieldUint16 = UINT16_MAX - 1;
    status = SOPC_EncodeableObject_Compare(&OpcUa_Custom_CustomUnionDataType_EncodeableType, &instCDT1_Union,
                                           &instCDT2_Union, &resCmp);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq(1, resCmp);

    // Clear
    SOPC_Buffer_Delete(buf);
    // Unrecord the encodeable type encoders
    SOPC_EncodeableType_RemoveAllUserTypes();
}

START_TEST(test_UserEncodeableType_OptionFields)
{
    SOPC_ReturnStatus status =
        SOPC_EncodeableType_RegisterTypesArray(SOPC_Custom_TypeInternalIndex_SIZE, sopc_Custom_KnownEncodeableTypes);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    /*** CREATE/INIT and CLEAR ***/
    SOPC_ExtensionObject extObjOptFields;
    SOPC_ExtensionObject_Initialize(&extObjOptFields);
    OpcUa_Custom_CustomOptFieldsDataType* instCDT_OptFields = NULL;
    status = SOPC_ExtensionObject_CreateObject(&extObjOptFields, &OpcUa_Custom_CustomOptFieldsDataType_EncodeableType,
                                               (void**) &instCDT_OptFields);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    SOPC_ExtensionObject_Clear(&extObjOptFields);

    /*** ENCODE/DECODE***/
    SOPC_Buffer* buf = SOPC_Buffer_Create(1024);
    OpcUa_Custom_CustomOptFieldsDataType instCDT1_OptFields, instCDT2_OptFields;
    OpcUa_Custom_CustomOptFieldsDataType_Initialize(&instCDT1_OptFields);
    OpcUa_Custom_CustomOptFieldsDataType_Initialize(&instCDT2_OptFields);
    ck_assert_ptr_null(instCDT1_OptFields.FieldUint32_2);
    ck_assert_ptr_null(instCDT1_OptFields.FieldUint32_3);
    ck_assert_ptr_null(instCDT1_OptFields.NoOfArrayUint16);
    ck_assert_ptr_null(instCDT1_OptFields.ArrayUint16);
    ck_assert_ptr_null(instCDT1_OptFields.Fieldcdt);
    instCDT1_OptFields.FieldBool = true;
    instCDT1_OptFields.FieldUint32_1 = UINT32_MAX;
    instCDT1_OptFields.FieldUint32_2 = SOPC_Calloc(1, sizeof(*instCDT1_OptFields.FieldUint32_2));
    *instCDT1_OptFields.FieldUint32_2 = 0xff0000ff;
    instCDT1_OptFields.NoOfArrayUint16 = SOPC_Calloc(1, sizeof(*instCDT1_OptFields.NoOfArrayUint16));
    *instCDT1_OptFields.NoOfArrayUint16 = 2;
    instCDT1_OptFields.ArrayUint16 =
        SOPC_Calloc((size_t) *instCDT1_OptFields.NoOfArrayUint16, sizeof(*instCDT1_OptFields.ArrayUint16));
    instCDT1_OptFields.ArrayUint16[0] = 5;
    instCDT1_OptFields.ArrayUint16[1] = 6;
    OpcUa_Custom_CustomDataType* instCDT = SOPC_Calloc(1, sizeof(*instCDT));
    OpcUa_Custom_CustomDataType_Initialize(instCDT);
    instCDT->fieldb = true;
    instCDT->fieldu = 7;
    instCDT1_OptFields.Fieldcdt = instCDT;

    // Encode
    status =
        SOPC_EncodeableObject_Encode(&OpcUa_Custom_CustomOptFieldsDataType_EncodeableType, &instCDT1_OptFields, buf, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    /*
     * OptMask : 0b00001101 = 0x0d (field opt 1, 3, and 4), 0x00, 0x00, 0x00. (4 Bytes)
     * SOPC_Boolean FieldBool = true : 0x01
     * uint32_t FieldUint32_1 = UINT32_MAX : 0xff, 0xff, 0xff, 0xff
     * uint32_t* FieldUint32_2 = 0xff0000ff : 0xff, 0x00, 0x00, 0xff
     * uint32_t* FieldUint32_3 = NULL
     * int32_t* NoOfArrayUint16 = 2 : 0x02, 0x00, 0x00, 0x00
     * uint16_t* ArrayUint16 = [5, 6] : 0x05, 0x00, 0x06, 0x00
     * OpcUa_Custom_CustomDataType* Fieldcdt = {SOPC_Boolean: true, uint16_t: 7} : 0x01, 0x07, 0x00
     */
    uint8_t expectedEncode[24] = {0x0d, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00,
                                  0xff, 0x02, 0x00, 0x00, 0x00, 0x05, 0x00, 0x06, 0x00, 0x01, 0x07, 0x00};
    ck_assert(CheckEncodedBuffer(buf, 24, expectedEncode));

    // Reset the buffer position and type instance, then decode from buffer
    OpcUa_Custom_CustomOptFieldsDataType_Clear(&instCDT1_OptFields);
    ck_assert(false == instCDT1_OptFields.FieldBool);
    ck_assert_uint_eq(0, instCDT1_OptFields.FieldUint32_1);
    ck_assert_ptr_null(instCDT1_OptFields.FieldUint32_2);
    ck_assert_ptr_null(instCDT1_OptFields.FieldUint32_3);
    ck_assert_ptr_null(instCDT1_OptFields.NoOfArrayUint16);
    ck_assert_ptr_null(instCDT1_OptFields.ArrayUint16);
    ck_assert_ptr_null(instCDT1_OptFields.Fieldcdt);

    status = SOPC_Buffer_SetPosition(buf, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    // Decode
    status =
        SOPC_EncodeableObject_Decode(&OpcUa_Custom_CustomOptFieldsDataType_EncodeableType, &instCDT1_OptFields, buf, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(true == instCDT1_OptFields.FieldBool);
    ck_assert_uint_eq(UINT32_MAX, instCDT1_OptFields.FieldUint32_1);
    ck_assert_uint_eq(0xff0000ff, *instCDT1_OptFields.FieldUint32_2);
    ck_assert_ptr_null(instCDT1_OptFields.FieldUint32_3);
    ck_assert_int_eq(2, *instCDT1_OptFields.NoOfArrayUint16);
    ck_assert_uint_eq(5, instCDT1_OptFields.ArrayUint16[0]);
    ck_assert_uint_eq(6, instCDT1_OptFields.ArrayUint16[1]);
    ck_assert_ptr_nonnull(instCDT1_OptFields.Fieldcdt);
    ck_assert(true == instCDT1_OptFields.Fieldcdt->fieldb);
    ck_assert_uint_eq(7, instCDT1_OptFields.Fieldcdt->fieldu);

    // Encode cleared OptFields
    OpcUa_Custom_CustomOptFieldsDataType_Clear(&instCDT1_OptFields);
    ck_assert(false == instCDT1_OptFields.FieldBool);
    ck_assert_uint_eq(0, instCDT1_OptFields.FieldUint32_1);
    ck_assert_ptr_null(instCDT1_OptFields.FieldUint32_2);
    ck_assert_ptr_null(instCDT1_OptFields.FieldUint32_3);
    ck_assert_ptr_null(instCDT1_OptFields.NoOfArrayUint16);
    ck_assert_ptr_null(instCDT1_OptFields.ArrayUint16);
    ck_assert_ptr_null(instCDT1_OptFields.Fieldcdt);
    SOPC_Buffer_Reset(buf);
    status =
        SOPC_EncodeableObject_Encode(&OpcUa_Custom_CustomOptFieldsDataType_EncodeableType, &instCDT1_OptFields, buf, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    // mask set to 0 and non-optional fields set to 0.
    uint8_t expectedClearedOptFields[9] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
    ck_assert(CheckEncodedBuffer(buf, 9, expectedClearedOptFields));

    /*** COPY ***/
    instCDT1_OptFields.FieldBool = true;
    instCDT1_OptFields.FieldUint32_1 = UINT32_MAX;
    instCDT1_OptFields.FieldUint32_2 = SOPC_Calloc(1, sizeof(*instCDT1_OptFields.FieldUint32_2));
    *instCDT1_OptFields.FieldUint32_2 = 0xff0000ff;
    instCDT1_OptFields.NoOfArrayUint16 = SOPC_Calloc(1, sizeof(*instCDT1_OptFields.NoOfArrayUint16));
    *instCDT1_OptFields.NoOfArrayUint16 = 2;
    instCDT1_OptFields.ArrayUint16 =
        SOPC_Calloc((size_t) *instCDT1_OptFields.NoOfArrayUint16, sizeof(*instCDT1_OptFields.ArrayUint16));
    instCDT1_OptFields.ArrayUint16[0] = 5;
    instCDT1_OptFields.ArrayUint16[1] = 6;
    OpcUa_Custom_CustomDataType* instCDTcpy = SOPC_Calloc(1, sizeof(*instCDTcpy));
    OpcUa_Custom_CustomDataType_Initialize(instCDTcpy);
    instCDTcpy->fieldb = true;
    instCDTcpy->fieldu = 7;
    instCDT1_OptFields.Fieldcdt = instCDTcpy;
    OpcUa_Custom_CustomOptFieldsDataType_Initialize(&instCDT2_OptFields);
    status = SOPC_EncodeableObject_Copy(&OpcUa_Custom_CustomOptFieldsDataType_EncodeableType, &instCDT2_OptFields,
                                        &instCDT1_OptFields);
    // Check
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(true == instCDT2_OptFields.FieldBool);
    ck_assert_uint_eq(UINT32_MAX, instCDT2_OptFields.FieldUint32_1);
    ck_assert_uint_eq(0xff0000ff, *instCDT2_OptFields.FieldUint32_2);
    ck_assert_ptr_null(instCDT2_OptFields.FieldUint32_3);
    ck_assert_int_eq(2, *instCDT2_OptFields.NoOfArrayUint16);
    ck_assert_uint_eq(5, instCDT2_OptFields.ArrayUint16[0]);
    ck_assert_uint_eq(6, instCDT2_OptFields.ArrayUint16[1]);
    ck_assert_ptr_nonnull(instCDT1_OptFields.Fieldcdt);
    ck_assert(true == instCDT1_OptFields.Fieldcdt->fieldb);
    ck_assert_uint_eq(7, instCDT1_OptFields.Fieldcdt->fieldu);

    /*** MOVE ***/
    OpcUa_Custom_CustomOptFieldsDataType_Clear(&instCDT2_OptFields);
    status = SOPC_EncodeableObject_Move(&instCDT2_OptFields, &instCDT1_OptFields);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    // Check
    ck_assert_uint_eq(0, instCDT1_OptFields.FieldUint32_1);
    ck_assert_ptr_null(instCDT1_OptFields.FieldUint32_2);
    ck_assert_ptr_null(instCDT1_OptFields.FieldUint32_3);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(true == instCDT2_OptFields.FieldBool);
    ck_assert_uint_eq(UINT32_MAX, instCDT2_OptFields.FieldUint32_1);
    ck_assert_uint_eq(0xff0000ff, *instCDT2_OptFields.FieldUint32_2);
    ck_assert_ptr_null(instCDT2_OptFields.FieldUint32_3);
    ck_assert_int_eq(2, *instCDT2_OptFields.NoOfArrayUint16);
    ck_assert_uint_eq(5, instCDT2_OptFields.ArrayUint16[0]);
    ck_assert_uint_eq(6, instCDT2_OptFields.ArrayUint16[1]);
    ck_assert_ptr_nonnull(instCDT2_OptFields.Fieldcdt);
    ck_assert(true == instCDT2_OptFields.Fieldcdt->fieldb);
    ck_assert_uint_eq(7, instCDT2_OptFields.Fieldcdt->fieldu);

    /*** COMPARE ***/
    int32_t resCmp = 0;
    status = SOPC_EncodeableObject_Compare(&OpcUa_Custom_CustomOptFieldsDataType_EncodeableType, &instCDT1_OptFields,
                                           &instCDT2_OptFields, &resCmp);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq(-1, resCmp);

    status = SOPC_EncodeableObject_Copy(&OpcUa_Custom_CustomOptFieldsDataType_EncodeableType, &instCDT1_OptFields,
                                        &instCDT2_OptFields);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_EncodeableObject_Compare(&OpcUa_Custom_CustomOptFieldsDataType_EncodeableType, &instCDT1_OptFields,
                                           &instCDT2_OptFields, &resCmp);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq(0, resCmp);

    instCDT1_OptFields.ArrayUint16[0] = 777;
    status = SOPC_EncodeableObject_Compare(&OpcUa_Custom_CustomOptFieldsDataType_EncodeableType, &instCDT1_OptFields,
                                           &instCDT2_OptFields, &resCmp);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq(1, resCmp);

    instCDT1_OptFields.ArrayUint16[0] = 5;
    SOPC_Free(instCDT1_OptFields.FieldUint32_2);
    instCDT1_OptFields.FieldUint32_2 = NULL;
    status = SOPC_EncodeableObject_Compare(&OpcUa_Custom_CustomOptFieldsDataType_EncodeableType, &instCDT1_OptFields,
                                           &instCDT2_OptFields, &resCmp);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq(-1, resCmp);

    instCDT1_OptFields.FieldUint32_2 = SOPC_Calloc(1, sizeof(*instCDT1_OptFields.FieldUint32_2));
    *instCDT1_OptFields.FieldUint32_2 = 0xff0000ff;
    SOPC_Free(instCDT1_OptFields.NoOfArrayUint16);
    instCDT1_OptFields.NoOfArrayUint16 = NULL;
    SOPC_Free(instCDT1_OptFields.ArrayUint16);
    instCDT1_OptFields.ArrayUint16 = NULL;
    status = SOPC_EncodeableObject_Compare(&OpcUa_Custom_CustomOptFieldsDataType_EncodeableType, &instCDT1_OptFields,
                                           &instCDT2_OptFields, &resCmp);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq(-1, resCmp);

    SOPC_Free(instCDT2_OptFields.NoOfArrayUint16);
    instCDT2_OptFields.NoOfArrayUint16 = NULL;
    SOPC_Free(instCDT2_OptFields.ArrayUint16);
    instCDT2_OptFields.ArrayUint16 = NULL;
    status = SOPC_EncodeableObject_Compare(&OpcUa_Custom_CustomOptFieldsDataType_EncodeableType, &instCDT1_OptFields,
                                           &instCDT2_OptFields, &resCmp);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq(0, resCmp);

    instCDT1_OptFields.Fieldcdt->fieldu = 8;
    status = SOPC_EncodeableObject_Compare(&OpcUa_Custom_CustomOptFieldsDataType_EncodeableType, &instCDT1_OptFields,
                                           &instCDT2_OptFields, &resCmp);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq(1, resCmp);

    OpcUa_Custom_CustomDataType_Clear(instCDT1_OptFields.Fieldcdt);
    SOPC_Free(instCDT1_OptFields.Fieldcdt);
    instCDT1_OptFields.Fieldcdt = NULL;
    status = SOPC_EncodeableObject_Compare(&OpcUa_Custom_CustomOptFieldsDataType_EncodeableType, &instCDT1_OptFields,
                                           &instCDT2_OptFields, &resCmp);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq(-1, resCmp);

    /*** CLEAR ***/
    OpcUa_Custom_CustomOptFieldsDataType_Clear(&instCDT1_OptFields);
    OpcUa_Custom_CustomOptFieldsDataType_Clear(&instCDT2_OptFields);
    SOPC_Buffer_Delete(buf);

    // Unrecord the encodeable type encoders
    SOPC_EncodeableType_RemoveAllUserTypes();
}
END_TEST

Suite* tests_make_suite_encodeable_types(void)
{
    Suite* s;
    TCase* tc_encodeable_types;

    s = suite_create("Tests for encodeable types");

    tc_encodeable_types = tcase_create("Encodeable_Types");

    tcase_add_checked_fixture(tc_encodeable_types, setup, NULL);
    tcase_add_test(tc_encodeable_types, test_TimeZoneDataType);
    tcase_add_test(tc_encodeable_types, test_AggregateFilterResult);
    tcase_add_test(tc_encodeable_types, test_BrowsePath);
    tcase_add_test(tc_encodeable_types, test_DeleteSubscriptionsRequest);
    tcase_add_test(tc_encodeable_types, test_TranslateBrowsePathsToNodeIdsRequest);
    tcase_add_test(tc_encodeable_types, test_UserEncodeableType);
    tcase_add_test(tc_encodeable_types, test_UserEncodeableTypeNS1);
    tcase_add_test(tc_encodeable_types, test_UserEncodeableTypeNS2);
    tcase_add_test(tc_encodeable_types, test_EncodeAndDecode_ToAndFrom_ByteString);
    tcase_add_test(tc_encodeable_types, test_UserEncodeableType_Union);
    tcase_add_test(tc_encodeable_types, test_UserEncodeableType_OptionFields);
    suite_add_tcase(s, tc_encodeable_types);

    return s;
}
