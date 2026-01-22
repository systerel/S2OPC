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

#ifndef SOPC_FX_AC_Types_H_
#define SOPC_FX_AC_Types_H_ 1

#include "fx_ac_enum_types.h"
#include "sopc_buffer.h"
#include "sopc_builtintypes.h"
#include "sopc_encodeabletype.h"

#define SOPC_FX_AC_NS_INDEX 4

#include "sopc_types.h"

#ifndef OPCUA_EXCLUDE_FX_AC_AggregatedHealthDataType
/*============================================================================
 * The FX_AC_AggregatedHealthDataType structure.
 *===========================================================================*/
extern SOPC_EncodeableType OpcUa_FX_AC_AggregatedHealthDataType_EncodeableType;

typedef struct _OpcUa_FX_AC_AggregatedHealthDataType
{
    SOPC_EncodeableType* encodeableType;
    /* IMPORTANT NOTE: response header IN RESPONSE MSG BODY is kept only
     *  for giving a copy of the header to application.
     */
    /* ::OpcUa_FX_AC_DeviceHealthOptionSet */ uint16_t AggregatedDeviceHealth;
    /* ::OpcUa_FX_AC_OperationalHealthOptionSet */ uint32_t AggregatedOperationalHealth;
} OpcUa_FX_AC_AggregatedHealthDataType;

void OpcUa_FX_AC_AggregatedHealthDataType_Initialize(void* pValue);

void OpcUa_FX_AC_AggregatedHealthDataType_Clear(void* pValue);

#endif

#ifndef OPCUA_EXCLUDE_FX_AC_ApplicationId
/*============================================================================
 * The FX_AC_ApplicationId structure.
 *===========================================================================*/
extern SOPC_EncodeableType OpcUa_FX_AC_ApplicationId_EncodeableType;

typedef struct _OpcUa_FX_AC_ApplicationId
{
    SOPC_EncodeableType* encodeableType;
    /* IMPORTANT NOTE: response header IN RESPONSE MSG BODY is kept only
     *  for giving a copy of the header to application.
     */
    uint32_t SwitchField;
    union
    {
        uint32_t IdNumeric;
        SOPC_String IdString;
        SOPC_Guid IdGuid;
        SOPC_ByteString IdByteString;
    } Value;
} OpcUa_FX_AC_ApplicationId;

void OpcUa_FX_AC_ApplicationId_Initialize(void* pValue);

void OpcUa_FX_AC_ApplicationId_Clear(void* pValue);

#endif

#ifndef OPCUA_EXCLUDE_FX_AC_ApplicationIdentifierDataType
/*============================================================================
 * The FX_AC_ApplicationIdentifierDataType structure.
 *===========================================================================*/
extern SOPC_EncodeableType OpcUa_FX_AC_ApplicationIdentifierDataType_EncodeableType;

typedef struct _OpcUa_FX_AC_ApplicationIdentifierDataType
{
    SOPC_EncodeableType* encodeableType;
    /* IMPORTANT NOTE: response header IN RESPONSE MSG BODY is kept only
     *  for giving a copy of the header to application.
     */
    SOPC_LocalizedText Name;
    OpcUa_FX_AC_ApplicationId UniqueIdentifier;
} OpcUa_FX_AC_ApplicationIdentifierDataType;

void OpcUa_FX_AC_ApplicationIdentifierDataType_Initialize(void* pValue);

void OpcUa_FX_AC_ApplicationIdentifierDataType_Clear(void* pValue);

#endif

#ifndef OPCUA_EXCLUDE_FX_AC_FxVersion
/*============================================================================
 * The FX_AC_FxVersion structure.
 *===========================================================================*/
extern SOPC_EncodeableType OpcUa_FX_AC_FxVersion_EncodeableType;

typedef struct _OpcUa_FX_AC_FxVersion
{
    SOPC_EncodeableType* encodeableType;
    /* IMPORTANT NOTE: response header IN RESPONSE MSG BODY is kept only
     *  for giving a copy of the header to application.
     */
    uint16_t Major;
    uint16_t Minor;
    uint16_t Build;
    uint16_t SubBuild;
} OpcUa_FX_AC_FxVersion;

void OpcUa_FX_AC_FxVersion_Initialize(void* pValue);

void OpcUa_FX_AC_FxVersion_Clear(void* pValue);

#endif

#ifndef OPCUA_EXCLUDE_FX_AC_IntervalRange
/*============================================================================
 * The FX_AC_IntervalRange structure.
 *===========================================================================*/
extern SOPC_EncodeableType OpcUa_FX_AC_IntervalRange_EncodeableType;

typedef struct _OpcUa_FX_AC_IntervalRange
{
    SOPC_EncodeableType* encodeableType;
    /* IMPORTANT NOTE: response header IN RESPONSE MSG BODY is kept only
     *  for giving a copy of the header to application.
     */
    uint32_t Min;
    uint32_t Max;
    uint16_t Increment;
    uint16_t Multiplier;
    OpcUa_FX_AC_FxTimeUnitsEnum Unit;
} OpcUa_FX_AC_IntervalRange;

void OpcUa_FX_AC_IntervalRange_Initialize(void* pValue);

void OpcUa_FX_AC_IntervalRange_Clear(void* pValue);

#endif

#ifndef OPCUA_EXCLUDE_FX_AC_PublisherQosDataType
/*============================================================================
 * The FX_AC_PublisherQosDataType structure.
 *===========================================================================*/
extern SOPC_EncodeableType OpcUa_FX_AC_PublisherQosDataType_EncodeableType;

typedef struct _OpcUa_FX_AC_PublisherQosDataType
{
    SOPC_EncodeableType* encodeableType;
    /* IMPORTANT NOTE: response header IN RESPONSE MSG BODY is kept only
     *  for giving a copy of the header to application.
     */
    SOPC_String QosCategory;
    int32_t NoOfDatagramQos;
    SOPC_ExtensionObject* DatagramQos;
} OpcUa_FX_AC_PublisherQosDataType;

void OpcUa_FX_AC_PublisherQosDataType_Initialize(void* pValue);

void OpcUa_FX_AC_PublisherQosDataType_Clear(void* pValue);

#endif

#ifndef OPCUA_EXCLUDE_FX_AC_SubscriberQosDataType
/*============================================================================
 * The FX_AC_SubscriberQosDataType structure.
 *===========================================================================*/
extern SOPC_EncodeableType OpcUa_FX_AC_SubscriberQosDataType_EncodeableType;

typedef struct _OpcUa_FX_AC_SubscriberQosDataType
{
    SOPC_EncodeableType* encodeableType;
    /* IMPORTANT NOTE: response header IN RESPONSE MSG BODY is kept only
     *  for giving a copy of the header to application.
     */
    SOPC_String QosCategory;
    int32_t NoOfDatagramQos;
    SOPC_ExtensionObject* DatagramQos;
} OpcUa_FX_AC_SubscriberQosDataType;

void OpcUa_FX_AC_SubscriberQosDataType_Initialize(void* pValue);

void OpcUa_FX_AC_SubscriberQosDataType_Clear(void* pValue);

#endif

/*============================================================================
 * Indexes in the table of known encodeable types.
 *
 * The enumerated values are indexes in the sopc_FX_AC_KnownEncodeableTypes array.
 *===========================================================================*/
typedef enum _SOPC_FX_AC_TypeInternalIndex
{
#ifndef OPCUA_EXCLUDE_FX_AC_AggregatedHealthDataType
    SOPC_TypeInternalIndex_FX_AC_AggregatedHealthDataType,
#endif
#ifndef OPCUA_EXCLUDE_FX_AC_ApplicationId
    SOPC_TypeInternalIndex_FX_AC_ApplicationId,
#endif
#ifndef OPCUA_EXCLUDE_FX_AC_ApplicationIdentifierDataType
    SOPC_TypeInternalIndex_FX_AC_ApplicationIdentifierDataType,
#endif
#ifndef OPCUA_EXCLUDE_FX_AC_FxVersion
    SOPC_TypeInternalIndex_FX_AC_FxVersion,
#endif
#ifndef OPCUA_EXCLUDE_FX_AC_IntervalRange
    SOPC_TypeInternalIndex_FX_AC_IntervalRange,
#endif
#ifndef OPCUA_EXCLUDE_FX_AC_PublisherQosDataType
    SOPC_TypeInternalIndex_FX_AC_PublisherQosDataType,
#endif
#ifndef OPCUA_EXCLUDE_FX_AC_SubscriberQosDataType
    SOPC_TypeInternalIndex_FX_AC_SubscriberQosDataType,
#endif
    SOPC_FX_AC_TypeInternalIndex_SIZE
} SOPC_FX_AC_TypeInternalIndex;

/*============================================================================
 * Table of known types.
 *===========================================================================*/
extern SOPC_EncodeableType** sopc_FX_AC_KnownEncodeableTypes;

#endif
/* This is the last line of an autogenerated file. */
