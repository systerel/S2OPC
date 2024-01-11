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

#include "sopc_builtintypes.h"
#include "sopc_encodeable.h"
#include "sopc_encoder.h"

#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "sopc_assert.h"
#include "sopc_hash.h"
#include "sopc_helper_string.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_time.h"

#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"
#include "sopc_common_constants.h"
#include "sopc_macros.h"
#include "sopc_protocol_constants.h"

bool SOPC_IsGoodStatus(SOPC_StatusCode status)
{
    return ((status & SOPC_GoodStatusOppositeMask) == 0);
}

// Converts if necessary the given status code to one of the tcp error code of spec part 6 (1.03) table 55
SOPC_StatusCode SOPC_StatusCode_ToTcpErrorCode(SOPC_StatusCode status)
{
    switch (status)
    {
    case OpcUa_BadTcpServerTooBusy:
    case OpcUa_BadTcpMessageTypeInvalid:
    case OpcUa_BadTcpSecureChannelUnknown:
    case OpcUa_BadTcpMessageTooLarge:
    case OpcUa_BadTimeout:
    case OpcUa_BadTcpNotEnoughResources:
    case OpcUa_BadTcpInternalError:
    case OpcUa_BadTcpEndpointUrlInvalid:
    case OpcUa_BadSecurityChecksFailed:
    case OpcUa_BadRequestInterrupted:
    case OpcUa_BadRequestTimeout:
    case OpcUa_BadSecureChannelClosed:
    case OpcUa_BadSecureChannelTokenUnknown:
    case OpcUa_BadCertificateUntrusted:
    case OpcUa_BadCertificateTimeInvalid:
    case OpcUa_BadCertificateIssuerTimeInvalid:
    case OpcUa_BadCertificateUseNotAllowed:
    case OpcUa_BadCertificateIssuerUseNotAllowed:
    case OpcUa_BadCertificateRevocationUnknown:
    case OpcUa_BadCertificateIssuerRevocationUnknown:
    case OpcUa_BadCertificateRevoked:
    case OpcUa_BadCertificateIssuerRevoked:
        // case OpcUa_BadCertificateUnknown: => not defined
        return status;
    case OpcUa_BadOutOfMemory:
        return OpcUa_BadTcpNotEnoughResources;
    case OpcUa_BadRequestTooLarge:
    case OpcUa_BadResponseTooLarge:
        return OpcUa_BadTcpMessageTooLarge;
    default:
        return OpcUa_BadTcpInternalError;
    }
}

const SOPC_NodeId SOPC_Null_Type = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 0};
const SOPC_NodeId SOPC_Boolean_Type = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = OpcUaId_Boolean};
const SOPC_NodeId SOPC_SByte_Type = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = OpcUaId_SByte};
const SOPC_NodeId SOPC_Byte_Type = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = OpcUaId_Byte};
const SOPC_NodeId SOPC_Int16_Type = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = OpcUaId_Int16};
const SOPC_NodeId SOPC_UInt16_Type = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = OpcUaId_UInt16};
const SOPC_NodeId SOPC_Int32_Type = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = OpcUaId_Int32};
const SOPC_NodeId SOPC_UInt32_Type = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = OpcUaId_UInt32};
const SOPC_NodeId SOPC_Int64_Type = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = OpcUaId_Int64};
const SOPC_NodeId SOPC_UInt64_Type = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = OpcUaId_UInt64};
const SOPC_NodeId SOPC_Float_Type = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = OpcUaId_Float};
const SOPC_NodeId SOPC_Double_Type = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = OpcUaId_Double};
const SOPC_NodeId SOPC_String_Type = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = OpcUaId_String};
const SOPC_NodeId SOPC_DateTime_Type = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = OpcUaId_DateTime};
const SOPC_NodeId SOPC_Guid_Type = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = OpcUaId_Guid};
const SOPC_NodeId SOPC_ByteString_Type = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = OpcUaId_ByteString};
const SOPC_NodeId SOPC_XmlElement_Type = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = OpcUaId_XmlElement};
const SOPC_NodeId SOPC_NodeId_Type = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = OpcUaId_NodeId};
const SOPC_NodeId SOPC_ExpandedNodeId_Type = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = OpcUaId_ExpandedNodeId};
const SOPC_NodeId SOPC_StatusCode_Type = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = OpcUaId_StatusCode};
const SOPC_NodeId SOPC_QualifiedName_Type = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = OpcUaId_QualifiedName};
const SOPC_NodeId SOPC_LocalizedText_Type = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = OpcUaId_LocalizedText};
const SOPC_NodeId SOPC_Structure_Type = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = OpcUaId_Structure};
const SOPC_NodeId SOPC_DataValue_Type = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = OpcUaId_DataValue};
const SOPC_NodeId SOPC_BaseData_Type = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = OpcUaId_BaseDataType};
const SOPC_NodeId SOPC_DiagnosticInfo_Type = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = OpcUaId_DiagnosticInfo};

const SOPC_NodeId* SOPC_BuiltInTypeId_To_DataTypeNodeId[26] = {
    &SOPC_Null_Type,          &SOPC_Boolean_Type,       &SOPC_SByte_Type,          &SOPC_Byte_Type,
    &SOPC_Int16_Type,         &SOPC_UInt16_Type,        &SOPC_Int32_Type,          &SOPC_UInt32_Type,
    &SOPC_Int64_Type,         &SOPC_UInt64_Type,        &SOPC_Float_Type,          &SOPC_Double_Type,
    &SOPC_String_Type,        &SOPC_DateTime_Type,      &SOPC_Guid_Type,           &SOPC_ByteString_Type,
    &SOPC_XmlElement_Type,    &SOPC_NodeId_Type,        &SOPC_ExpandedNodeId_Type, &SOPC_StatusCode_Type,
    &SOPC_QualifiedName_Type, &SOPC_LocalizedText_Type, &SOPC_Structure_Type,      &SOPC_DataValue_Type,
    &SOPC_BaseData_Type,      &SOPC_DiagnosticInfo_Type};

void SOPC_Boolean_InitializeAux(void* value)
{
    SOPC_Boolean_Initialize((SOPC_Boolean*) value);
}

void SOPC_Boolean_Initialize(SOPC_Boolean* b)
{
    if (b != NULL)
    {
        *b = false;
    }
}

SOPC_ReturnStatus SOPC_Boolean_CopyAux(void* dest, const void* src)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != dest && NULL != src)
    {
        *((SOPC_Boolean*) dest) = *((const SOPC_Boolean*) src);
        status = SOPC_STATUS_OK;
    }
    return status;
}

SOPC_ReturnStatus SOPC_Boolean_CompareAux(const void* left, const void* right, int32_t* comparison)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != left && NULL != right && NULL != comparison)
    {
        status = SOPC_STATUS_OK;
        const SOPC_Boolean cleft = *((const SOPC_Boolean*) left);
        const SOPC_Boolean cright = *((const SOPC_Boolean*) right);
        if (cleft < cright)
        {
            *comparison = -1;
        }
        else if (cright < cleft)
        {
            *comparison = 1;
        }
        else
        {
            *comparison = 0;
        }
    }
    return status;
}

void SOPC_Boolean_ClearAux(void* value)
{
    SOPC_Boolean_Clear((SOPC_Boolean*) value);
}

void SOPC_Boolean_Clear(SOPC_Boolean* b)
{
    if (b != NULL)
        *b = false;
}

void SOPC_SByte_InitializeAux(void* value)
{
    SOPC_SByte_Initialize((SOPC_SByte*) value);
}

void SOPC_SByte_Initialize(SOPC_SByte* sbyte)
{
    if (sbyte != NULL)
    {
        *sbyte = 0;
    }
}

SOPC_ReturnStatus SOPC_SByte_CopyAux(void* dest, const void* src)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != dest && NULL != src)
    {
        *((SOPC_SByte*) dest) = *((const SOPC_SByte*) src);
        status = SOPC_STATUS_OK;
    }
    return status;
}

SOPC_ReturnStatus SOPC_SByte_CompareAux(const void* left, const void* right, int32_t* comparison)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != left && NULL != right && NULL != comparison)
    {
        status = SOPC_STATUS_OK;
        const SOPC_SByte cleft = *((const SOPC_SByte*) left);
        const SOPC_SByte cright = *((const SOPC_SByte*) right);
        if (cleft < cright)
        {
            *comparison = -1;
        }
        else if (cright < cleft)
        {
            *comparison = 1;
        }
        else
        {
            *comparison = 0;
        }
    }
    return status;
}

void SOPC_SByte_ClearAux(void* value)
{
    SOPC_SByte_Clear((SOPC_SByte*) value);
}

void SOPC_SByte_Clear(SOPC_SByte* sbyte)
{
    if (sbyte != NULL)
    {
        *sbyte = 0;
    }
}

void SOPC_Byte_InitializeAux(void* value)
{
    SOPC_Byte_Initialize((SOPC_Byte*) value);
}

void SOPC_Byte_Initialize(SOPC_Byte* byte)
{
    if (byte != NULL)
    {
        *byte = 0;
    }
}

SOPC_ReturnStatus SOPC_Byte_CopyAux(void* dest, const void* src)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != dest && NULL != src)
    {
        *((SOPC_Byte*) dest) = *((const SOPC_Byte*) src);
        status = SOPC_STATUS_OK;
    }
    return status;
}

SOPC_ReturnStatus SOPC_Byte_CompareAux(const void* left, const void* right, int32_t* comparison)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != left && NULL != right && NULL != comparison)
    {
        status = SOPC_STATUS_OK;
        const SOPC_Byte cleft = *((const SOPC_Byte*) left);
        const SOPC_Byte cright = *((const SOPC_Byte*) right);
        if (cleft < cright)
        {
            *comparison = -1;
        }
        else if (cright < cleft)
        {
            *comparison = 1;
        }
        else
        {
            *comparison = 0;
        }
    }
    return status;
}

void SOPC_Byte_ClearAux(void* value)
{
    SOPC_Byte_Clear((SOPC_Byte*) value);
}

void SOPC_Byte_Clear(SOPC_Byte* byte)
{
    if (byte != NULL)
    {
        *byte = 0;
    }
}

void SOPC_Int16_InitializeAux(void* value)
{
    SOPC_Int16_Initialize((int16_t*) value);
}

void SOPC_Int16_Initialize(int16_t* intv)
{
    if (intv != NULL)
    {
        *intv = 0;
    }
}

SOPC_ReturnStatus SOPC_Int16_CopyAux(void* dest, const void* src)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != dest && NULL != src)
    {
        *((int16_t*) dest) = *((const int16_t*) src);
        status = SOPC_STATUS_OK;
    }
    return status;
}

SOPC_ReturnStatus SOPC_Int16_CompareAux(const void* left, const void* right, int32_t* comparison)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != left && NULL != right && NULL != comparison)
    {
        status = SOPC_STATUS_OK;
        const int16_t cleft = *((const int16_t*) left);
        const int16_t cright = *((const int16_t*) right);
        if (cleft < cright)
        {
            *comparison = -1;
        }
        else if (cright < cleft)
        {
            *comparison = 1;
        }
        else
        {
            *comparison = 0;
        }
    }
    return status;
}

void SOPC_Int16_ClearAux(void* value)
{
    SOPC_Int16_Clear((int16_t*) value);
}

void SOPC_Int16_Clear(int16_t* intv)
{
    if (intv != NULL)
    {
        *intv = 0;
    }
}

void SOPC_UInt16_InitializeAux(void* value)
{
    SOPC_UInt16_Initialize((uint16_t*) value);
}

void SOPC_UInt16_Initialize(uint16_t* uint)
{
    if (uint != NULL)
    {
        *uint = 0;
    }
}

SOPC_ReturnStatus SOPC_UInt16_CopyAux(void* dest, const void* src)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != dest && NULL != src)
    {
        *((uint16_t*) dest) = *((const uint16_t*) src);
        status = SOPC_STATUS_OK;
    }
    return status;
}

SOPC_ReturnStatus SOPC_UInt16_CompareAux(const void* left, const void* right, int32_t* comparison)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != left && NULL != right && NULL != comparison)
    {
        status = SOPC_STATUS_OK;
        const uint16_t cleft = *((const uint16_t*) left);
        const uint16_t cright = *((const uint16_t*) right);
        if (cleft < cright)
        {
            *comparison = -1;
        }
        else if (cright < cleft)
        {
            *comparison = 1;
        }
        else
        {
            *comparison = 0;
        }
    }
    return status;
}

void SOPC_UInt16_ClearAux(void* value)
{
    SOPC_UInt16_Clear((uint16_t*) value);
}
void SOPC_UInt16_Clear(uint16_t* uint)
{
    if (uint != NULL)
    {
        *uint = 0;
    }
}

void SOPC_Int32_InitializeAux(void* value)
{
    SOPC_Int32_Initialize((int32_t*) value);
}

void SOPC_Int32_Initialize(int32_t* intv)
{
    if (intv != NULL)
    {
        *intv = 0;
    }
}

SOPC_ReturnStatus SOPC_Int32_CopyAux(void* dest, const void* src)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != dest && NULL != src)
    {
        *((int32_t*) dest) = *((const int32_t*) src);
        status = SOPC_STATUS_OK;
    }
    return status;
}

SOPC_ReturnStatus SOPC_Int32_CompareAux(const void* left, const void* right, int32_t* comparison)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != left && NULL != right && NULL != comparison)
    {
        status = SOPC_STATUS_OK;
        const int32_t cleft = *((const int32_t*) left);
        const int32_t cright = *((const int32_t*) right);
        if (cleft < cright)
        {
            *comparison = -1;
        }
        else if (cright < cleft)
        {
            *comparison = 1;
        }
        else
        {
            *comparison = 0;
        }
    }
    return status;
}

void SOPC_Int32_ClearAux(void* value)
{
    SOPC_Int32_Clear((int32_t*) value);
}
void SOPC_Int32_Clear(int32_t* intv)
{
    if (intv != NULL)
    {
        *intv = 0;
    }
}

void SOPC_UInt32_InitializeAux(void* value)
{
    SOPC_UInt32_Initialize((uint32_t*) value);
}

void SOPC_UInt32_Initialize(uint32_t* uint)
{
    if (uint != NULL)
    {
        *uint = 0;
    }
}

SOPC_ReturnStatus SOPC_UInt32_CopyAux(void* dest, const void* src)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != dest && NULL != src)
    {
        *((uint32_t*) dest) = *((const uint32_t*) src);
        status = SOPC_STATUS_OK;
    }
    return status;
}

SOPC_ReturnStatus SOPC_UInt32_CompareAux(const void* left, const void* right, int32_t* comparison)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != left && NULL != right && NULL != comparison)
    {
        status = SOPC_STATUS_OK;
        const uint32_t cleft = *((const uint32_t*) left);
        const uint32_t cright = *((const uint32_t*) right);
        if (cleft < cright)
        {
            *comparison = -1;
        }
        else if (cright < cleft)
        {
            *comparison = 1;
        }
        else
        {
            *comparison = 0;
        }
    }
    return status;
}

void SOPC_UInt32_ClearAux(void* value)
{
    SOPC_UInt32_Clear((uint32_t*) value);
}
void SOPC_UInt32_Clear(uint32_t* uint)
{
    if (uint != NULL)
    {
        *uint = 0;
    }
}

void SOPC_Int64_InitializeAux(void* value)
{
    SOPC_Int64_Initialize((int64_t*) value);
}

void SOPC_Int64_Initialize(int64_t* intv)
{
    if (intv != NULL)
    {
        *intv = 0;
    }
}

SOPC_ReturnStatus SOPC_Int64_CopyAux(void* dest, const void* src)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != dest && NULL != src)
    {
        *((int64_t*) dest) = *((const int64_t*) src);
        status = SOPC_STATUS_OK;
    }
    return status;
}

SOPC_ReturnStatus SOPC_Int64_CompareAux(const void* left, const void* right, int32_t* comparison)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != left && NULL != right && NULL != comparison)
    {
        status = SOPC_STATUS_OK;
        const int64_t cleft = *((const int64_t*) left);
        const int64_t cright = *((const int64_t*) right);
        if (cleft < cright)
        {
            *comparison = -1;
        }
        else if (cright < cleft)
        {
            *comparison = 1;
        }
        else
        {
            *comparison = 0;
        }
    }
    return status;
}

void SOPC_Int64_ClearAux(void* value)
{
    SOPC_Int64_Clear((int64_t*) value);
}
void SOPC_Int64_Clear(int64_t* intv)
{
    if (intv != NULL)
    {
        *intv = 0;
    }
}

void SOPC_UInt64_InitializeAux(void* value)
{
    SOPC_UInt64_Initialize((uint64_t*) value);
}

void SOPC_UInt64_Initialize(uint64_t* uint)
{
    if (uint != NULL)
    {
        *uint = 0;
    }
}

SOPC_ReturnStatus SOPC_UInt64_CopyAux(void* dest, const void* src)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != dest && NULL != src)
    {
        *((uint64_t*) dest) = *((const uint64_t*) src);
        status = SOPC_STATUS_OK;
    }
    return status;
}

SOPC_ReturnStatus SOPC_UInt64_CompareAux(const void* left, const void* right, int32_t* comparison)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != left && NULL != right && NULL != comparison)
    {
        status = SOPC_STATUS_OK;
        const uint64_t cleft = *((const uint64_t*) left);
        const uint64_t cright = *((const uint64_t*) right);
        if (cleft < cright)
        {
            *comparison = -1;
        }
        else if (cright < cleft)
        {
            *comparison = 1;
        }
        else
        {
            *comparison = 0;
        }
    }
    return status;
}

void SOPC_UInt64_ClearAux(void* value)
{
    SOPC_UInt64_Clear((uint64_t*) value);
}
void SOPC_UInt64_Clear(uint64_t* uint)
{
    if (uint != NULL)
    {
        *uint = 0;
    }
}

void SOPC_Float_InitializeAux(void* value)
{
    SOPC_Float_Initialize((float*) value);
}

void SOPC_Float_Initialize(float* f)
{
    if (f != NULL)
    {
        *f = 0.0;
    }
}

SOPC_ReturnStatus SOPC_Float_CopyAux(void* dest, const void* src)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != dest && NULL != src)
    {
        *((float*) dest) = *((const float*) src);
        status = SOPC_STATUS_OK;
    }
    return status;
}

SOPC_ReturnStatus SOPC_Float_CompareAux(const void* left, const void* right, int32_t* comparison)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != left && NULL != right && NULL != comparison)
    {
        status = SOPC_STATUS_OK;
        const float cleft = *((const float*) left);
        const float cright = *((const float*) right);
        if (cleft < cright)
        {
            *comparison = -1;
        }
        else if (cright < cleft)
        {
            *comparison = 1;
        }
        else
        {
            *comparison = 0;
        }
    }
    return status;
}

void SOPC_Float_ClearAux(void* value)
{
    SOPC_Float_Clear((float*) value);
}

void SOPC_Float_Clear(float* f)
{
    if (f != NULL)
    {
        *f = 0.0;
    }
}

void SOPC_Double_InitializeAux(void* value)
{
    SOPC_Double_Initialize((double*) value);
}

void SOPC_Double_Initialize(double* d)
{
    if (d != NULL)
    {
        *d = 0.0;
    }
}

SOPC_ReturnStatus SOPC_Double_CopyAux(void* dest, const void* src)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != dest && NULL != src)
    {
        *((double*) dest) = *((const double*) src);
        status = SOPC_STATUS_OK;
    }
    return status;
}

SOPC_ReturnStatus SOPC_Double_CompareAux(const void* left, const void* right, int32_t* comparison)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != left && NULL != right && NULL != comparison)
    {
        status = SOPC_STATUS_OK;
        const double cleft = *((const double*) left);
        const double cright = *((const double*) right);
        if (cleft < cright)
        {
            *comparison = -1;
        }
        else if (cright < cleft)
        {
            *comparison = 1;
        }
        else
        {
            *comparison = 0;
        }
    }
    return status;
}

void SOPC_Double_ClearAux(void* value)
{
    SOPC_Double_Clear((double*) value);
}

void SOPC_Double_Clear(double* d)
{
    if (d != NULL)
    {
        *d = 0.0;
    }
}

void SOPC_ByteString_InitializeAux(void* value)
{
    SOPC_ByteString_Initialize((SOPC_ByteString*) value);
}

void SOPC_ByteString_Initialize(SOPC_ByteString* bstring)
{
    SOPC_String_Initialize((SOPC_String*) bstring);
}

SOPC_ByteString* SOPC_ByteString_Create(void)
{
    return (SOPC_ByteString*) SOPC_String_Create();
}

SOPC_ReturnStatus SOPC_ByteString_InitializeFixedSize(SOPC_ByteString* bstring, uint32_t size)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (bstring != NULL)
    {
        status = SOPC_STATUS_OK;
        SOPC_ByteString_Initialize(bstring);
        if ((uint64_t) size * sizeof(SOPC_Byte) <= SIZE_MAX && size <= INT32_MAX)
        {
            bstring->Data = SOPC_Malloc(sizeof(SOPC_Byte) * (size_t) size);
            bstring->Length = (int32_t) size;
        }
        else
        {
            bstring->Data = NULL;
        }
        if (bstring->Data != NULL)
        {
            memset(bstring->Data, 0, size);
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_ByteString_CopyFromBytes(SOPC_ByteString* dest, const SOPC_Byte* bytes, int32_t length)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (dest != NULL && bytes != NULL && length > 0 && NULL == dest->Data)
    {
        dest->Length = length;
        if (length > 0 && (uint64_t) length * sizeof(SOPC_Byte) <= SIZE_MAX)
        {
            dest->Data = SOPC_Malloc(sizeof(SOPC_Byte) * (size_t) length);
        }
        else
        {
            dest->Data = NULL;
        }
        if (dest->Data != NULL)
        {
            memcpy(dest->Data, bytes, (size_t) length);
            status = SOPC_STATUS_OK;
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_ByteString_Copy(SOPC_ByteString* dest, const SOPC_ByteString* src)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (dest != NULL && NULL == dest->Data && src != NULL)
    {
        dest->Length = src->Length;
        if (dest->Length > 0)
        {
            if ((uint64_t) dest->Length * sizeof(SOPC_Byte) <= SIZE_MAX)
            {
                dest->Data = SOPC_Malloc(sizeof(SOPC_Byte) * (size_t) dest->Length);
                if (dest->Data != NULL)
                {
                    status = SOPC_STATUS_OK;
                    // No need of secure copy, both have same size here
                    memcpy(dest->Data, src->Data, (size_t) dest->Length);
                }
                else
                {
                    status = SOPC_STATUS_NOK;
                }
            }
        }
        else
        {
            // NULL ByteString
            status = SOPC_STATUS_OK;
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_ByteString_CopyAux(void* dest, const void* src)
{
    return SOPC_ByteString_Copy((SOPC_ByteString*) dest, (const SOPC_ByteString*) src);
}

void SOPC_ByteString_ClearAux(void* value)
{
    SOPC_ByteString_Clear((SOPC_ByteString*) value);
}

void SOPC_ByteString_Clear(SOPC_ByteString* bstring)
{
    SOPC_String_Clear((SOPC_String*) bstring);
}

void SOPC_ByteString_Delete(SOPC_ByteString* bstring)
{
    SOPC_String_Delete((SOPC_String*) bstring);
}

SOPC_ReturnStatus SOPC_ByteString_Compare(const SOPC_ByteString* left,
                                          const SOPC_ByteString* right,
                                          int32_t* comparison)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;

    if (left != NULL && right != NULL)
    {
        status = SOPC_STATUS_OK;
    }

    if (SOPC_STATUS_OK == status)
    {
        if (left->Length == right->Length || (left->Length <= 0 && right->Length <= 0))
        {
            if (left->Length <= 0 && right->Length <= 0)
            {
                *comparison = 0;
            }
            else
            {
                *comparison = memcmp(left->Data, right->Data, (size_t) left->Length);
            }
        }
        else if (left->Length > right->Length)
        {
            *comparison = +1;
        }
        else
        {
            *comparison = -1;
        }
    }

    return status;
}

SOPC_ReturnStatus SOPC_ByteString_CompareAux(const void* left, const void* right, int32_t* comparison)
{
    return SOPC_ByteString_Compare((const SOPC_ByteString*) left, (const SOPC_ByteString*) right, comparison);
}

bool SOPC_ByteString_Equal(const SOPC_ByteString* left, const SOPC_ByteString* right)
{
    int32_t compare = 0;
    bool result = false;

    if (SOPC_ByteString_Compare(left, right, &compare) == SOPC_STATUS_OK)
    {
        result = compare == 0;
    }

    return result;
}

void SOPC_String_InitializeAux(void* value)
{
    SOPC_String_Initialize((SOPC_String*) value);
}

void SOPC_String_Initialize(SOPC_String* string)
{
    if (string != NULL)
    {
        string->Length = -1;
        string->Data = NULL;
        string->DoNotClear = false; // False unless characters attached
    }
}

SOPC_String* SOPC_String_Create(void)
{
    SOPC_String* string = NULL;
    string = SOPC_Malloc(sizeof(SOPC_String));
    if (NULL != string)
    {
        SOPC_String_Initialize(string);
    }
    return string;
}

SOPC_ReturnStatus SOPC_String_AttachFrom(SOPC_String* dest, SOPC_String* src)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (dest != NULL && src != NULL && src->Length >= 0 && src->Data != NULL)
    {
        status = SOPC_STATUS_OK;
        dest->Length = src->Length;
        dest->Data = src->Data;
        dest->DoNotClear = 1; // dest->characters will not be freed on clear
    }
    return status;
}

SOPC_ReturnStatus SOPC_String_AttachFromCstring(SOPC_String* dest, const char* src)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    size_t stringLength = 0;
    if (dest != NULL && NULL == dest->Data && src != NULL)
    {
        SOPC_ASSERT(CHAR_BIT == 8);
        stringLength = strlen(src);
        if (stringLength <= INT32_MAX)
        {
            status = SOPC_STATUS_OK;
            dest->Length = (int32_t) stringLength;
            SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
            dest->Data = (uint8_t*) src;
            SOPC_GCC_DIAGNOSTIC_RESTORE
            dest->DoNotClear = 1; // dest->characters will not be freed on clear
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_String_Copy(SOPC_String* dest, const SOPC_String* src)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (dest != NULL && NULL == dest->Data && src != NULL)
    {
        // Keep null terminator for C string compatibility
        dest->Length = src->Length;
        if (dest->Length > 0)
        {
            if ((uint64_t) dest->Length * sizeof(SOPC_Byte) <= SIZE_MAX)
            {
                status = SOPC_STATUS_OK;

                dest->Data = SOPC_Malloc(sizeof(SOPC_Byte) * (size_t) dest->Length + 1);
                if (dest->Data != NULL)
                {
                    // No need of secure copy, both have same size here
                    memcpy(dest->Data, src->Data, (size_t) dest->Length);
                    dest->Data[dest->Length] = '\0';
                    // Since it is a copy, be sure to clear bytes on clear
                    dest->DoNotClear = false;
                }
                else
                {
                    status = SOPC_STATUS_OUT_OF_MEMORY;
                }
            }
            else
            {
                status = SOPC_STATUS_OUT_OF_MEMORY;
            }
        }
        else
        {
            status = SOPC_STATUS_OK;
            dest->Data = NULL;
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_String_CopyAux(void* dest, const void* src)
{
    return SOPC_String_Copy((SOPC_String*) dest, (const SOPC_String*) src);
}

void SOPC_String_ClearAux(void* value)
{
    SOPC_String_Clear((SOPC_String*) value);
}

void SOPC_String_Clear(SOPC_String* string)
{
    if (string != NULL)
    {
        if (false == string->DoNotClear)
        {
            if (string->Data != NULL)
            {
                SOPC_Free(string->Data);
            }
        }
        SOPC_String_Initialize(string);
    }
}

void SOPC_String_Delete(SOPC_String* string)
{
    if (NULL != string)
    {
        SOPC_String_Clear(string);
        SOPC_Free(string);
    }
}

SOPC_ReturnStatus SOPC_String_CopyFromCString(SOPC_String* string, const char* cString)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    size_t stringLength = 0;
    size_t idx = 0;
    if (string != NULL && NULL == string->Data && cString != NULL)
    {
        status = SOPC_STATUS_OK;

        stringLength = strlen(cString);
        if (stringLength > 0 && stringLength <= INT32_MAX)
        {
            // length without null terminator
            string->Length = (int32_t) stringLength;
            // keep terminator for compatibility with char* strings
            string->Data = SOPC_Malloc(sizeof(SOPC_Byte) * (stringLength + 1));
            if (string->Data != NULL)
            {
                // Keep null terminator for compatibility with c strings !
                if (CHAR_BIT == 8)
                {
                    memcpy(string->Data, cString, stringLength + 1);
                }
                else
                {
                    // On systems for which char is not encoded on 1 byte
                    for (idx = 0; idx < stringLength + 1; idx++)
                    {
                        string->Data[idx] = (uint8_t) cString[idx];
                    }
                }
            }
            else
            {
                status = SOPC_STATUS_NOK;
            }
        }
        else if (0 == stringLength)
        {
            // Empty C string case is valid
            string->Length = 0;
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
    }

    return status;
}

SOPC_ReturnStatus SOPC_String_InitializeFromCString(SOPC_String* string, const char* cString)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;

    if (string != NULL)
    {
        SOPC_String_Initialize(string);
        status = SOPC_String_CopyFromCString(string, cString);
    }

    return status;
}

char* SOPC_String_GetCString(const SOPC_String* string)
{
    char* cString = NULL;
    int32_t idx = 0;
    if (string != NULL && string->Length > 0)
    {
        cString = SOPC_Malloc(sizeof(char) * ((size_t) string->Length + 1));
        if (cString != NULL)
        {
            if (CHAR_BIT == 8)
            {
                memcpy(cString, string->Data, (size_t) string->Length + 1);
            }
            else
            {
                // On systems for which char is not encoded on 1 byte
                for (idx = 0; idx < string->Length + 1; idx++)
                {
                    cString[idx] = (char) string->Data[idx];
                }
            }
        }
    }
    return cString;
}

const char* SOPC_String_GetRawCString(const SOPC_String* string)
{
    const char* cString = NULL;
    if (string == NULL)
    {
        return NULL;
    }
    if (string->Length > 0)
    {
        if (CHAR_BIT == 8)
        {
            cString = (const char*) string->Data;
            SOPC_ASSERT(string->Data[string->Length] == '\0');
        }
        else
        {
            SOPC_ASSERT(false);
        }
    }
    else
    {
        cString = "\0";
    }
    return cString;
}

SOPC_ReturnStatus SOPC_String_Compare(const SOPC_String* left,
                                      const SOPC_String* right,
                                      bool ignoreCase,
                                      int32_t* comparison)
{
    if (NULL == left || NULL == right || NULL == comparison)
        return SOPC_STATUS_INVALID_PARAMETERS;

    if (left->Length <= 0 && right->Length <= 0)
    {
        // Both NULL strings => considered equal
        *comparison = 0;
    }
    else if (left->Length == right->Length)
    {
        SOPC_ASSERT(CHAR_BIT == 8);
        if (false == ignoreCase)
        {
            *comparison = strcmp((char*) left->Data, (char*) right->Data);
        }
        else
        {
            *comparison = SOPC_strncmp_ignore_case((char*) left->Data, (char*) right->Data, (size_t) left->Length);
        }
    }
    else if (left->Length > right->Length)
    {
        *comparison = +1;
    }
    else
    {
        *comparison = -1;
    }

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_String_CompareAux(const void* left, const void* right, int32_t* comparison)
{
    return SOPC_String_Compare((const SOPC_String*) left, (const SOPC_String*) right, false, comparison);
}

bool SOPC_String_Equal(const SOPC_String* left, const SOPC_String* right)
{
    int32_t compare = 0;
    bool result = false;

    if (SOPC_String_Compare(left, right, false, &compare) == SOPC_STATUS_OK)
    {
        result = compare == 0;
    }

    return result;
}

const char** SOPC_String_GetRawCStringArray(int32_t nbOfStrings, SOPC_String* stringArray)
{
    if (nbOfStrings > 0 && NULL == stringArray)
    {
        return NULL;
    }

    if (nbOfStrings < 0)
    {
        nbOfStrings = 0;
    }

    const char** cStringArray = SOPC_Calloc((size_t) nbOfStrings + 1, sizeof(char*));

    if (cStringArray != NULL)
    {
        for (int32_t i = 0; i < nbOfStrings; i++)
        {
            cStringArray[i] = SOPC_String_GetRawCString(&stringArray[i]);
        }
        cStringArray[nbOfStrings] = NULL;
    }

    return cStringArray;
}

char** SOPC_String_GetCStringArray(int32_t nbOfStrings, SOPC_String* stringArray)
{
    if (nbOfStrings > 0 && NULL == stringArray)
    {
        return NULL;
    }

    if (nbOfStrings < 0)
    {
        nbOfStrings = 0;
    }

    char** cStringArray = SOPC_Calloc((size_t) nbOfStrings + 1, sizeof(char*));

    if (cStringArray != NULL)
    {
        for (int32_t i = 0; i < nbOfStrings; i++)
        {
            cStringArray[i] = SOPC_String_GetCString(&stringArray[i]);
        }
        cStringArray[nbOfStrings] = NULL;
    }

    return cStringArray;
}

void SOPC_XmlElement_InitializeAux(void* value)
{
    SOPC_XmlElement_Initialize((SOPC_XmlElement*) value);
}

void SOPC_XmlElement_Initialize(SOPC_XmlElement* xmlElt)
{
    if (xmlElt != NULL)
    {
        SOPC_ByteString_Initialize((SOPC_ByteString*) xmlElt);
    }
}

SOPC_ReturnStatus SOPC_XmlElement_Copy(SOPC_XmlElement* dest, const SOPC_XmlElement* src)
{
    return SOPC_ByteString_Copy((SOPC_ByteString*) dest, (const SOPC_ByteString*) src);
}

SOPC_ReturnStatus SOPC_XmlElement_CopyAux(void* dest, const void* src)
{
    return SOPC_XmlElement_Copy((SOPC_XmlElement*) dest, (const SOPC_XmlElement*) src);
}

SOPC_ReturnStatus SOPC_XmlElement_Compare(const SOPC_XmlElement* left,
                                          const SOPC_XmlElement* right,
                                          int32_t* comparison)
{
    return SOPC_ByteString_Compare((const SOPC_ByteString*) left, (const SOPC_ByteString*) right, comparison);
}

SOPC_ReturnStatus SOPC_XmlElement_CompareAux(const void* left, const void* right, int32_t* comparison)
{
    return SOPC_ByteString_Compare((const SOPC_ByteString*) left, (const SOPC_ByteString*) right, comparison);
}

void SOPC_XmlElement_ClearAux(void* value)
{
    SOPC_XmlElement_Clear((SOPC_XmlElement*) value);
}

void SOPC_XmlElement_Clear(SOPC_XmlElement* xmlElt)
{
    SOPC_ByteString_Clear((SOPC_ByteString*) xmlElt);
}

void SOPC_DateTime_InitializeAux(void* value)
{
    SOPC_DateTime_Initialize((SOPC_DateTime*) value);
}

void SOPC_DateTime_Initialize(SOPC_DateTime* dateTime)
{
    if (dateTime != NULL)
    {
        *dateTime = 0;
    }
}

SOPC_ReturnStatus SOPC_DateTime_CopyAux(void* dest, const void* src)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != dest && NULL != src)
    {
        *((SOPC_DateTime*) dest) = *((const SOPC_DateTime*) src);
        status = SOPC_STATUS_OK;
    }
    return status;
}

SOPC_ReturnStatus SOPC_DateTime_Compare(const SOPC_DateTime* left, const SOPC_DateTime* right, int32_t* comparison)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != left && NULL != right && NULL != comparison)
    {
        if (*left < *right)
        {
            *comparison = -1;
        }
        else if (*right < *left)
        {
            *comparison = 1;
        }
        else
        {
            *comparison = 0;
        }
        status = SOPC_STATUS_OK;
    }
    return status;
}

SOPC_ReturnStatus SOPC_DateTime_CompareAux(const void* left, const void* right, int32_t* comparison)
{
    return SOPC_DateTime_Compare((const SOPC_DateTime*) left, (const SOPC_DateTime*) right, comparison);
}

void SOPC_DateTime_ClearAux(void* value)
{
    SOPC_DateTime_Clear((SOPC_DateTime*) value);
}

void SOPC_DateTime_Clear(SOPC_DateTime* dateTime)
{
    SOPC_DateTime_Initialize(dateTime);
}

void SOPC_Guid_InitializeAux(void* value)
{
    SOPC_Guid_Initialize((SOPC_Guid*) value);
}

SOPC_ReturnStatus SOPC_Guid_FromCString(SOPC_Guid* guid, const char* str, size_t len)
{
    if (NULL == guid || NULL == str)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (len != 36 || '-' != str[8] || '-' != str[13] || '-' != str[18] || '-' != str[23])
    {
        return SOPC_STATUS_NOK;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_strtouint32_t(str, &guid->Data1, 16, '-');
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_strtouint16_t(str + 9, &guid->Data2, 16, '-');
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_strtouint16_t(str + 14, &guid->Data3, 16, '-');
    }

    char buf[4];
    memset(buf, 0, sizeof(buf));

    for (int i = 0; i < 2 && SOPC_STATUS_OK == status; ++i)
    {
        buf[0] = str[19 + 2 * i];
        buf[1] = str[20 + 2 * i];
        status = SOPC_strtouint8_t(buf, &guid->Data4[i], 16, '\0');
    }

    for (int i = 0; i < 6 && SOPC_STATUS_OK == status; ++i)
    {
        buf[0] = str[24 + 2 * i];
        buf[1] = str[25 + 2 * i];
        status = SOPC_strtouint8_t(buf, &guid->Data4[2 + i], 16, '\0');
    }

    return status;
}

static void SOPC_Guid_IntoCString(const SOPC_Guid* guid, char* dest)
{
    if (NULL == guid || NULL == dest)
    {
        return;
    }
    int res = sprintf(dest,
                      "g=%08" PRIX32 "-%04" PRIX16 "-%04" PRIX16 "-%02" PRIX8 "%02" PRIX8 "-%02" PRIX8 "%02" PRIX8
                      "%02" PRIX8 "%02" PRIX8 "%02" PRIX8 "%02" PRIX8,
                      guid->Data1, guid->Data2, guid->Data3, guid->Data4[0], guid->Data4[1], guid->Data4[2],
                      guid->Data4[3], guid->Data4[4], guid->Data4[5], guid->Data4[6], guid->Data4[7]);
    SOPC_ASSERT(res > 0);
}

char* SOPC_Guid_ToCString(const SOPC_Guid* guid)
{
    char* result = NULL;
    if (guid != NULL)
    {
        // e.g.: "g=09087E75-8E5E-499B-954F-F2A9603DB28A\0"
        result = SOPC_Calloc(39, sizeof(char));
        if (result != NULL)
        {
            SOPC_Guid_IntoCString(guid, result);
        }
    }
    return result;
}

void SOPC_Guid_Initialize(SOPC_Guid* guid)
{
    if (guid != NULL)
    {
        memset(guid, 0, sizeof(SOPC_Guid));
    }
}

SOPC_ReturnStatus SOPC_Guid_Copy(SOPC_Guid* dest, const SOPC_Guid* src)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != dest && NULL != src)
    {
        *dest = *src;
        status = SOPC_STATUS_OK;
    }
    return status;
}

SOPC_ReturnStatus SOPC_Guid_CopyAux(void* dest, const void* src)
{
    return SOPC_Guid_Copy((SOPC_Guid*) dest, (const SOPC_Guid*) src);
}

SOPC_ReturnStatus SOPC_Guid_Compare(const SOPC_Guid* left, const SOPC_Guid* right, int32_t* comparison)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != left && NULL != right && NULL != comparison)
    {
        status = SOPC_STATUS_OK;
        if (left->Data1 == right->Data1)
        {
            *comparison = 0;
        }
        else if (left->Data1 < right->Data1)
        {
            *comparison = -1;
        }
        else
        {
            *comparison = 1;
        }
        if (*comparison == 0)
        {
            if (left->Data2 < right->Data2)
            {
                *comparison = -1;
            }
            else if (left->Data2 > right->Data2)
            {
                *comparison = 1;
            }
        }
        if (*comparison == 0)
        {
            if (left->Data3 < right->Data3)
            {
                *comparison = -1;
            }
            else if (left->Data3 > right->Data3)
            {
                *comparison = 1;
            }
        }
        if (*comparison == 0)
        {
            /* No matter what the target endianness is, this function will return the same result */
            *comparison = memcmp(left->Data4, right->Data4, sizeof(uint64_t));
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_Guid_CompareAux(const void* left, const void* right, int32_t* comparison)
{
    return SOPC_Guid_Compare((const SOPC_Guid*) left, (const SOPC_Guid*) right, comparison);
}

void SOPC_Guid_ClearAux(void* value)
{
    SOPC_Guid_Clear((SOPC_Guid*) value);
}

void SOPC_Guid_Clear(SOPC_Guid* guid)
{
    if (guid != NULL)
    {
        memset(guid, 0, sizeof(SOPC_Guid));
    }
}

void SOPC_NodeId_InitializeAux(void* value)
{
    SOPC_NodeId_Initialize((SOPC_NodeId*) value);
}

void SOPC_NodeId_Initialize(SOPC_NodeId* nodeId)
{
    if (nodeId != NULL)
    {
        memset(nodeId, 0, sizeof(SOPC_NodeId));
    }
}

SOPC_ReturnStatus SOPC_NodeId_Copy(SOPC_NodeId* dest, const SOPC_NodeId* src)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != dest && NULL != src)
    {
        dest->Namespace = src->Namespace;
        dest->IdentifierType = src->IdentifierType;
        switch (src->IdentifierType)
        {
        case SOPC_IdentifierType_Numeric:
            dest->Data.Numeric = src->Data.Numeric;
            status = SOPC_STATUS_OK;
            break;
        case SOPC_IdentifierType_String:
            SOPC_String_Initialize(&dest->Data.String);
            status = SOPC_String_Copy(&dest->Data.String, &src->Data.String);
            break;
        case SOPC_IdentifierType_Guid:
            dest->Data.Guid = SOPC_Malloc(sizeof(SOPC_Guid));
            if (dest->Data.Guid != NULL)
            {
                status = SOPC_Guid_Copy(dest->Data.Guid, src->Data.Guid);
            }
            else
            {
                status = SOPC_STATUS_OUT_OF_MEMORY;
            }
            break;
        case SOPC_IdentifierType_ByteString:
            SOPC_ByteString_Initialize(&dest->Data.Bstring);
            status = SOPC_ByteString_Copy(&dest->Data.Bstring, &src->Data.Bstring);
            break;
        default:
            break;
        }
        if (SOPC_STATUS_OK != status)
        {
            SOPC_NodeId_Clear(dest);
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_NodeId_CopyAux(void* dest, const void* src)
{
    return SOPC_NodeId_Copy((SOPC_NodeId*) dest, (const SOPC_NodeId*) src);
}

void SOPC_NodeId_ClearAux(void* value)
{
    SOPC_NodeId_Clear((SOPC_NodeId*) value);
}

void SOPC_NodeId_Clear(SOPC_NodeId* nodeId)
{
    if (nodeId != NULL)
    {
        nodeId->Namespace = 0; // OPCUA namespace
        switch (nodeId->IdentifierType)
        {
        case SOPC_IdentifierType_Numeric:
            SOPC_UInt32_Clear(&nodeId->Data.Numeric);
            break;
        case SOPC_IdentifierType_String:
            SOPC_String_Clear(&nodeId->Data.String);
            break;
        case SOPC_IdentifierType_Guid:
            SOPC_Guid_Clear(nodeId->Data.Guid);
            if (nodeId->Data.Guid != NULL)
            {
                SOPC_Free(nodeId->Data.Guid);
            }
            nodeId->Data.Guid = NULL;
            break;
        case SOPC_IdentifierType_ByteString:
            SOPC_ByteString_Clear(&nodeId->Data.Bstring);
            break;
        default:
            break;
        }
        nodeId->IdentifierType = SOPC_IdentifierType_Numeric;
    }
}

SOPC_ReturnStatus SOPC_NodeId_Compare(const SOPC_NodeId* left, const SOPC_NodeId* right, int32_t* comparison)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != left && NULL != right && comparison != NULL)
    {
        if (left == right)
        {
            *comparison = 0;
            status = SOPC_STATUS_OK;
        }
        else if (left->Namespace == right->Namespace && left->IdentifierType == right->IdentifierType)
        {
            switch (left->IdentifierType)
            {
            case SOPC_IdentifierType_Numeric:
                if (left->Data.Numeric == right->Data.Numeric)
                {
                    *comparison = 0;
                }
                else if (left->Data.Numeric < right->Data.Numeric)
                {
                    *comparison = -1;
                }
                else
                {
                    *comparison = 1;
                }
                status = SOPC_STATUS_OK;
                break;
            case SOPC_IdentifierType_String:
                status = SOPC_String_Compare(&left->Data.String, &right->Data.String, false, comparison);
                break;
            case SOPC_IdentifierType_Guid:
                if (NULL != left->Data.Guid && NULL != right->Data.Guid)
                {
                    *comparison = memcmp(left->Data.Guid, right->Data.Guid, sizeof(SOPC_Guid));
                    status = SOPC_STATUS_OK;
                } // else invalid parameters
                break;
            case SOPC_IdentifierType_ByteString:
                status = SOPC_ByteString_Compare(&left->Data.Bstring, &right->Data.Bstring, comparison);
                break;
            default:
                break;
            }
        }
        else
        {
            if (left->IdentifierType == right->IdentifierType)
            {
                if (left->Namespace < right->Namespace)
                {
                    *comparison = -1;
                }
                else if (left->Namespace > right->Namespace)
                {
                    *comparison = 1;
                }
                else
                {
                    // Already verified in precedent conditions
                    SOPC_ASSERT(false);
                }
            }
            else if (left->IdentifierType < right->IdentifierType)
            {
                *comparison = -1;
            }
            else
            {
                *comparison = 1;
            }
            status = SOPC_STATUS_OK;
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_NodeId_CompareAux(const void* left, const void* right, int32_t* comparison)
{
    return SOPC_NodeId_Compare((const SOPC_NodeId*) left, (const SOPC_NodeId*) right, comparison);
}

bool SOPC_NodeId_Equal(const SOPC_NodeId* left, const SOPC_NodeId* right)
{
    if (NULL == left || NULL == right)
    {
        return false;
    }
    int32_t compare = -1;
    SOPC_ReturnStatus status = SOPC_NodeId_Compare(left, right, &compare);
    if (SOPC_STATUS_OK != status)
    {
        return false;
    }
    return compare == 0;
}

bool SOPC_NodeId_IsNull(const SOPC_NodeId* nodeId)
{
    if (NULL == nodeId)
    {
        return true;
    }

    if (0 != nodeId->Namespace)
    {
        return false;
    }

    switch (nodeId->IdentifierType)
    {
    case SOPC_IdentifierType_Numeric:
        return 0 == nodeId->Data.Numeric;

    case SOPC_IdentifierType_ByteString:
    case SOPC_IdentifierType_String:
        return 0 >= nodeId->Data.String.Length;

    case SOPC_IdentifierType_Guid:
        if (NULL == nodeId->Data.Guid)
            return true;
        return (0 == nodeId->Data.Guid->Data1 && 0 == nodeId->Data.Guid->Data2 && 0 == nodeId->Data.Guid->Data3 &&
                0 == nodeId->Data.Guid->Data4[0] && 0 == nodeId->Data.Guid->Data4[1] &&
                0 == nodeId->Data.Guid->Data4[2] && 0 == nodeId->Data.Guid->Data4[3] &&
                0 == nodeId->Data.Guid->Data4[4] && 0 == nodeId->Data.Guid->Data4[5] &&
                0 == nodeId->Data.Guid->Data4[6] && 0 == nodeId->Data.Guid->Data4[7]);
    default:
        return false;
    }
}

void SOPC_NodeId_Hash(const SOPC_NodeId* nodeId, uint64_t* hash)
{
    uint64_t h;

    SOPC_ASSERT(nodeId != NULL);

    h = SOPC_DJBHash((const uint8_t*) &nodeId->IdentifierType, sizeof(SOPC_IdentifierType));
    h = SOPC_DJBHash_Step(h, (const uint8_t*) &nodeId->Namespace, sizeof(uint16_t));

    switch (nodeId->IdentifierType)
    {
    case SOPC_IdentifierType_Numeric:
        h = SOPC_DJBHash_Step(h, (const uint8_t*) &nodeId->Data.Numeric, sizeof(uint32_t));
        break;
    case SOPC_IdentifierType_ByteString:
    case SOPC_IdentifierType_String:
        if (nodeId->Data.String.Length > 0)
        {
            h = SOPC_DJBHash_Step(h, nodeId->Data.String.Data, (size_t) nodeId->Data.String.Length);
        }
        break;
    case SOPC_IdentifierType_Guid:
        if (nodeId->Data.Guid != NULL)
        {
            h = SOPC_DJBHash_Step(h, (const uint8_t*) nodeId->Data.Guid, sizeof(SOPC_Guid));
        }
        break;
    default:
        SOPC_ASSERT(false && "Unknown IdentifierType");
    }

    *hash = h;
}

char* SOPC_NodeId_ToCString(const SOPC_NodeId* nodeId)
{
    // format part 6 §5.3.1.10
    char* result = NULL;
    int res = 0;
    size_t maxSize = 0;
    if (nodeId != NULL)
    {
        if (nodeId->Namespace != 0)
        {
            // with <nsId> = 5 digits max: "ns=<nsId>;"
            maxSize = 9;
        }
        switch (nodeId->IdentifierType)
        {
        case SOPC_IdentifierType_Numeric:
            // with <id> = 10 digits max: "i=<id>\0"
            maxSize += 13;
            break;
        case SOPC_IdentifierType_String:
            // "s=<string>\0"
            if (nodeId->Data.String.Length > 0)
            {
                maxSize += (uint32_t) nodeId->Data.String.Length + 3;
            }
            else
            {
                maxSize += 3;
            }
            break;
        case SOPC_IdentifierType_Guid:
            // ex: "g=09087e75-8e5e-499b-954f-f2a9603db28a\0"
            if (nodeId->Data.Guid != NULL)
            {
                maxSize += 39;
            }
            else
            {
                maxSize += 3;
            }
            break;
        case SOPC_IdentifierType_ByteString:
            // "b=<bstring>\0"
            if (nodeId->Data.Bstring.Length > 0)
            {
                maxSize += (uint32_t) nodeId->Data.Bstring.Length + 3;
            }
            else
            {
                maxSize += 3;
            }
            break;
        default:
            break;
        }
        result = SOPC_Calloc(maxSize, sizeof(char));
        if (result != NULL)
        {
            if (nodeId->Namespace != 0)
            {
                res = sprintf(result, "ns=%" PRIu16 ";", nodeId->Namespace);
            }
            if (res >= 0)
            {
                switch (nodeId->IdentifierType)
                {
                case SOPC_IdentifierType_Numeric:
                    // with <id> = 10 digits max: "i=<id>\0"
                    res = sprintf(&result[res], "i=%" PRIu32, nodeId->Data.Numeric);
                    SOPC_ASSERT(res > 0);
                    break;
                case SOPC_IdentifierType_String:
                    // "s=<string>\0"
                    if (nodeId->Data.String.Length > 0)
                    {
                        res = sprintf(&result[res], "s=%s", SOPC_String_GetRawCString(&nodeId->Data.String));
                    }
                    else
                    {
                        res = sprintf(&result[res], "s=");
                    }
                    SOPC_ASSERT(res > 0);
                    break;
                case SOPC_IdentifierType_Guid:
                    // ex: "g=09087e75-8e5e-499b-954f-f2a9603db28a\0"
                    if (nodeId->Data.Guid != NULL)
                    {
                        SOPC_Guid_IntoCString(nodeId->Data.Guid, &result[res]);
                    }
                    else
                    {
                        res = sprintf(&result[res], "g=");
                        SOPC_ASSERT(res > 0);
                    }
                    break;
                case SOPC_IdentifierType_ByteString:
                    // "b=<bstring>\0"
                    if (nodeId->Data.Bstring.Length > 0)
                    {
                        memcpy(&result[res], "b=", 2 * sizeof(char));
                        memcpy(&result[res + 2], nodeId->Data.Bstring.Data, (size_t) nodeId->Data.Bstring.Length);
                    }
                    else
                    {
                        res = sprintf(&result[res], "b=");
                        SOPC_ASSERT(res > 0);
                    }
                    break;
                default:
                    break;
                }
            }
            else
            {
                SOPC_Free(result);
                result = NULL;
            }
        }
    }
    return result;
}

SOPC_ReturnStatus SOPC_NodeId_InitializeFromCString(SOPC_NodeId* pNid, const char* cString, int32_t len)
{
    if (NULL == pNid || NULL == cString || len <= 0)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    /* Creates a new guid/string */
    char* sz = NULL; /* Safe copy of the cString */
    char* p = NULL;
    SOPC_IdentifierType type = SOPC_IdentifierType_Numeric;
    uint16_t ns = 0;
    uint32_t iid = 0;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_Guid* pGuid = NULL;

    /* Copy the string in a safe place and sscanf it */
    if (NULL != cString && len > 0)
    {
        sz = SOPC_Calloc((size_t) len + 1, sizeof(char));
    }
    if (NULL != sz)
    {
        strncpy(sz, cString, (size_t) len);
        /* Search for namespace, defaults to 0 */
        p = strchr(sz, ';');
        if (NULL != p)
        {
            if (memcmp(sz, "ns=", 3 * sizeof(char)) != 0)
            {
                status = SOPC_STATUS_NOK;
            }
            else
            {
                status = SOPC_strtouint16_t(&sz[3], &ns, 10, ';');
            }
            p += 1;
        }
        else
        {
            p = sz;
        }

        /* Search for identifier, followed by '=' */
        if (SOPC_STATUS_OK == status && len - (p - sz) < 2)
        {
            /* There is less than 2 chars left to read */
            status = SOPC_STATUS_NOK;
        }
        if (SOPC_STATUS_OK == status && p[1] != '=')
        {
            status = SOPC_STATUS_NOK;
        }
        if (SOPC_STATUS_OK == status)
        {
            switch (p[0])
            {
            case 'i':
                type = SOPC_IdentifierType_Numeric;
                p += 2;
                status = SOPC_strtouint32_t(p, &iid, 10, '\0');
                break;
            case 's':
                type = SOPC_IdentifierType_String;
                p += 2;
                break;
            case 'g':
                type = SOPC_IdentifierType_Guid;
                p += 2;
                pGuid = SOPC_Malloc(sizeof(SOPC_Guid));
                if (NULL == pGuid)
                {
                    status = SOPC_STATUS_NOK;
                }
                break;
            case 'b':
                type = SOPC_IdentifierType_ByteString;
                p += 2;
                break;
            default:
                status = SOPC_STATUS_NOK;
                break;
            }
        }
    }
    else
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }

    if (SOPC_STATUS_OK == status)
    {
        pNid->IdentifierType = type;
        pNid->Namespace = ns;
        switch (type)
        {
        case SOPC_IdentifierType_Numeric:
            pNid->Data.Numeric = iid;
            break;
        case SOPC_IdentifierType_String:
            status = SOPC_String_InitializeFromCString(&pNid->Data.String, p);
            break;
        case SOPC_IdentifierType_Guid:
            status = SOPC_Guid_FromCString(pGuid, p, (size_t)(len - (p - sz)));

            if (SOPC_STATUS_OK == status)
            {
                pNid->Data.Guid = pGuid;
            }
            break;
        case SOPC_IdentifierType_ByteString:
            SOPC_ByteString_Initialize(&pNid->Data.Bstring);
            status = SOPC_ByteString_CopyFromBytes(&pNid->Data.Bstring, (SOPC_Byte*) p, len - (int32_t)(p - sz));
            break;
        default:
            break;
        }
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_NodeId_Clear(pNid);
        SOPC_Free(pGuid);
    }

    SOPC_Free(sz);

    return status;
}

SOPC_NodeId* SOPC_NodeId_FromCString(const char* cString, int32_t len)
{
    SOPC_NodeId* pNid = SOPC_Malloc(sizeof(SOPC_NodeId));
    if (NULL == pNid)
    {
        return NULL;
    }

    SOPC_NodeId_Initialize(pNid);
    SOPC_ReturnStatus status = SOPC_NodeId_InitializeFromCString(pNid, cString, len);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Free(pNid);
        pNid = NULL;
    }

    return pNid;
}

static uint64_t nodeid_hash(const uintptr_t id)
{
    uint64_t hash;
    SOPC_NodeId_Hash((const SOPC_NodeId*) id, &hash);
    return hash;
}

static bool nodeid_equal(const uintptr_t a, const uintptr_t b)
{
    int32_t cmp = 0;

    SOPC_ReturnStatus status = SOPC_NodeId_Compare((const SOPC_NodeId*) a, (const SOPC_NodeId*) b, &cmp);
    SOPC_ASSERT(status == SOPC_STATUS_OK);

    return cmp == 0;
}

static void nodeid_free(uintptr_t id)
{
    SOPC_NodeId* nId = (SOPC_NodeId*) id;
    if (nId != NULL)
    {
        SOPC_NodeId_Clear(nId);
        SOPC_Free(nId);
    }
}

SOPC_Dict* SOPC_NodeId_Dict_Create(bool free_keys, SOPC_Dict_Free_Fct value_free)
{
    return SOPC_Dict_Create(0, nodeid_hash, nodeid_equal, free_keys ? nodeid_free : NULL, value_free);
}

void SOPC_ExpandedNodeId_InitializeAux(void* value)
{
    SOPC_ExpandedNodeId_Initialize((SOPC_ExpandedNodeId*) value);
}

void SOPC_ExpandedNodeId_Initialize(SOPC_ExpandedNodeId* expNodeId)
{
    if (expNodeId != NULL)
    {
        SOPC_String_Initialize(&expNodeId->NamespaceUri);
        SOPC_NodeId_Initialize(&expNodeId->NodeId);
        SOPC_UInt32_Initialize(&expNodeId->ServerIndex);
    }
}

SOPC_ReturnStatus SOPC_ExpandedNodeId_Copy(SOPC_ExpandedNodeId* dest, const SOPC_ExpandedNodeId* src)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != dest && NULL != src)
    {
        status = SOPC_NodeId_Copy(&dest->NodeId, &src->NodeId);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_String_Copy(&dest->NamespaceUri, &src->NamespaceUri);
        }
        if (SOPC_STATUS_OK == status)
        {
            dest->ServerIndex = src->ServerIndex;
        }
        if (SOPC_STATUS_OK != status)
        {
            SOPC_ExpandedNodeId_Clear(dest);
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_ExpandedNodeId_CopyAux(void* dest, const void* src)
{
    return SOPC_ExpandedNodeId_Copy((SOPC_ExpandedNodeId*) dest, (const SOPC_ExpandedNodeId*) src);
}

SOPC_ReturnStatus SOPC_ExpandedNodeId_Compare(const SOPC_ExpandedNodeId* left,
                                              const SOPC_ExpandedNodeId* right,
                                              int32_t* comparison)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != left && NULL != right && comparison != NULL)
    {
        status = SOPC_NodeId_Compare(&left->NodeId, &right->NodeId, comparison);
        if (SOPC_STATUS_OK == status && *comparison == 0)
        {
            status = SOPC_String_Compare(&left->NamespaceUri, &right->NamespaceUri, false, comparison);
            if (SOPC_STATUS_OK == status && *comparison == 0)
            {
                if (left->ServerIndex < right->ServerIndex)
                {
                    *comparison = -1;
                }
                else if (right->ServerIndex < left->ServerIndex)
                {
                    *comparison = 1;
                }
                else
                {
                    *comparison = 0;
                }
            }
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_ExpandedNodeId_CompareAux(const void* left, const void* right, int32_t* comparison)
{
    return SOPC_ExpandedNodeId_Compare((const SOPC_ExpandedNodeId*) left, (const SOPC_ExpandedNodeId*) right,
                                       comparison);
}

void SOPC_ExpandedNodeId_ClearAux(void* value)
{
    SOPC_ExpandedNodeId_Clear((SOPC_ExpandedNodeId*) value);
}

void SOPC_ExpandedNodeId_Clear(SOPC_ExpandedNodeId* expNodeId)
{
    if (expNodeId != NULL)
    {
        SOPC_String_Clear(&expNodeId->NamespaceUri);
        SOPC_NodeId_Clear(&expNodeId->NodeId);
        SOPC_UInt32_Clear(&expNodeId->ServerIndex);
    }
}

void SOPC_StatusCode_InitializeAux(void* value)
{
    SOPC_StatusCode_Initialize((SOPC_StatusCode*) value);
}

void SOPC_StatusCode_Initialize(SOPC_StatusCode* status)
{
    if (status != NULL)
    {
        *status = SOPC_GoodGenericStatus;
    }
}

SOPC_ReturnStatus SOPC_StatusCode_CopyAux(void* dest, const void* src)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != dest && NULL != src)
    {
        *((SOPC_StatusCode*) dest) = *((const SOPC_StatusCode*) src);
        status = SOPC_STATUS_OK;
    }
    return status;
}

SOPC_ReturnStatus SOPC_StatusCode_CompareAux(const void* left, const void* right, int32_t* comparison)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != left && NULL != right && NULL != comparison)
    {
        status = SOPC_STATUS_OK;
        const SOPC_ReturnStatus cleft = *((const SOPC_StatusCode*) left);
        const SOPC_ReturnStatus cright = *((const SOPC_StatusCode*) right);
        if (cleft < cright)
        {
            *comparison = -1;
        }
        else if (cright < cleft)
        {
            *comparison = 1;
        }
        else
        {
            *comparison = 0;
        }
    }
    return status;
}

void SOPC_StatusCode_ClearAux(void* value)
{
    SOPC_StatusCode_Clear((SOPC_StatusCode*) value);
}

void SOPC_StatusCode_Clear(SOPC_StatusCode* status)
{
    if (status != NULL)
    {
        *status = SOPC_GoodGenericStatus;
    }
}

void SOPC_DiagnosticInfo_InitializeAux(void* value)
{
    SOPC_DiagnosticInfo_Initialize((SOPC_DiagnosticInfo*) value);
}

void SOPC_DiagnosticInfo_Initialize(SOPC_DiagnosticInfo* diagInfo)
{
    if (diagInfo != NULL)
    {
        diagInfo->SymbolicId = -1;
        diagInfo->NamespaceUri = -1;
        diagInfo->Locale = -1;
        diagInfo->LocalizedText = -1;
        SOPC_String_Initialize(&diagInfo->AdditionalInfo);
        diagInfo->InnerStatusCode = SOPC_STATUS_OK;
        diagInfo->InnerDiagnosticInfo = NULL;
    }
}

SOPC_ReturnStatus SOPC_DiagnosticInfo_Copy(SOPC_DiagnosticInfo* dest, const SOPC_DiagnosticInfo* src)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != dest && NULL != src)
    {
        if (NULL != dest->InnerDiagnosticInfo)
        {
            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
        else if (NULL == dest->InnerDiagnosticInfo && NULL != src->InnerDiagnosticInfo)
        {
            dest->InnerDiagnosticInfo = SOPC_Calloc(1, sizeof(SOPC_DiagnosticInfo));
            if (NULL == dest->InnerDiagnosticInfo)
            {
                status = SOPC_STATUS_OUT_OF_MEMORY;
            }
            else
            {
                status = SOPC_DiagnosticInfo_Copy(dest->InnerDiagnosticInfo, src->InnerDiagnosticInfo);
            }
        }
        else
        {
            status = SOPC_STATUS_OK;
        }
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_String_Copy(&dest->AdditionalInfo, &src->AdditionalInfo);
        }
        if (SOPC_STATUS_OK == status)
        {
            dest->Locale = src->Locale;
            dest->LocalizedText = src->LocalizedText;
            dest->NamespaceUri = src->NamespaceUri;
            dest->SymbolicId = src->SymbolicId;
        }
        if (SOPC_STATUS_OK != status)
        {
            SOPC_DiagnosticInfo_Clear(dest);
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_DiagnosticInfo_CopyAux(void* dest, const void* src)
{
    return SOPC_DiagnosticInfo_Copy((SOPC_DiagnosticInfo*) dest, (const SOPC_DiagnosticInfo*) src);
}

SOPC_ReturnStatus SOPC_DiagnosticInfo_Compare(const SOPC_DiagnosticInfo* left,
                                              const SOPC_DiagnosticInfo* right,
                                              int32_t* comparison)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != left && NULL != right && NULL != comparison)
    {
        status = SOPC_STATUS_OK;
        if (left->Locale < right->Locale)
        {
            *comparison = -1;
        }
        else if (right->Locale < left->Locale)
        {
            *comparison = 1;
        }
        else
        {
            *comparison = 0;
        }
        if (SOPC_STATUS_OK == status && *comparison == 0)
        {
            if (left->LocalizedText < right->LocalizedText)
            {
                *comparison = -1;
            }
            else if (right->LocalizedText < left->LocalizedText)
            {
                *comparison = 1;
            }
        }
        if (SOPC_STATUS_OK == status && *comparison == 0)
        {
            if (left->NamespaceUri < right->NamespaceUri)
            {
                *comparison = -1;
            }
            else if (right->NamespaceUri < left->NamespaceUri)
            {
                *comparison = 1;
            }
        }
        if (SOPC_STATUS_OK == status && *comparison == 0)
        {
            if (left->SymbolicId < right->SymbolicId)
            {
                *comparison = -1;
            }
            else if (right->SymbolicId < left->SymbolicId)
            {
                *comparison = 1;
            }
        }
        if (SOPC_STATUS_OK == status && *comparison == 0)
        {
            status = SOPC_String_Compare(&left->AdditionalInfo, &right->AdditionalInfo, false, comparison);
        }
        if (SOPC_STATUS_OK == status && *comparison == 0)
        {
            if (NULL == left->InnerDiagnosticInfo && NULL == right->InnerDiagnosticInfo)
            {
                *comparison = 0;
            }
            else
            {
                status = SOPC_DiagnosticInfo_Compare(left->InnerDiagnosticInfo, right->InnerDiagnosticInfo, comparison);
            }
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_DiagnosticInfo_CompareAux(const void* left, const void* right, int32_t* comparison)
{
    return SOPC_DiagnosticInfo_Compare((const SOPC_DiagnosticInfo*) left, (const SOPC_DiagnosticInfo*) right,
                                       comparison);
}

void SOPC_DiagnosticInfo_ClearAux(void* value)
{
    SOPC_DiagnosticInfo_Clear((SOPC_DiagnosticInfo*) value);
}

void SOPC_DiagnosticInfo_Clear(SOPC_DiagnosticInfo* diagInfo)
{
    if (diagInfo != NULL)
    {
        SOPC_String_Clear(&diagInfo->AdditionalInfo);
        if (diagInfo->InnerDiagnosticInfo != NULL)
        {
            SOPC_DiagnosticInfo_Clear(diagInfo->InnerDiagnosticInfo);
            SOPC_Free(diagInfo->InnerDiagnosticInfo);
        }
        diagInfo->SymbolicId = -1;
        diagInfo->NamespaceUri = -1;
        diagInfo->Locale = -1;
        diagInfo->LocalizedText = -1;
        diagInfo->InnerStatusCode = SOPC_STATUS_OK;
        diagInfo->InnerDiagnosticInfo = NULL;
    }
}

void SOPC_QualifiedName_InitializeAux(void* value)
{
    SOPC_QualifiedName_Initialize((SOPC_QualifiedName*) value);
}

void SOPC_QualifiedName_Initialize(SOPC_QualifiedName* qname)
{
    if (qname != NULL)
    {
        qname->NamespaceIndex = 0;
        SOPC_String_Initialize(&qname->Name);
    }
}

SOPC_ReturnStatus SOPC_QualifiedName_Copy(SOPC_QualifiedName* dest, const SOPC_QualifiedName* src)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != dest && NULL != src)
    {
        status = SOPC_String_Copy(&dest->Name, &src->Name);
        if (SOPC_STATUS_OK == status)
        {
            dest->NamespaceIndex = src->NamespaceIndex;
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_QualifiedName_CopyAux(void* dest, const void* src)
{
    return SOPC_QualifiedName_Copy((SOPC_QualifiedName*) dest, (const SOPC_QualifiedName*) src);
}

SOPC_ReturnStatus SOPC_QualifiedName_Compare(const SOPC_QualifiedName* left,
                                             const SOPC_QualifiedName* right,
                                             int32_t* comparison)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != left && NULL != right)
    {
        status = SOPC_String_Compare(&left->Name, &right->Name, false, comparison);
        if (SOPC_STATUS_OK == status && *comparison == 0)
        {
            if (left->NamespaceIndex < right->NamespaceIndex)
            {
                *comparison = -1;
            }
            else if (left->NamespaceIndex > right->NamespaceIndex)
            {
                *comparison = 1;
            }
            else
            {
                *comparison = 0;
            }
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_QualifiedName_CompareAux(const void* left, const void* right, int32_t* comparison)
{
    return SOPC_QualifiedName_Compare((const SOPC_QualifiedName*) left, (const SOPC_QualifiedName*) right, comparison);
}

void SOPC_QualifiedName_ClearAux(void* value)
{
    SOPC_QualifiedName_Clear((SOPC_QualifiedName*) value);
}

SOPC_ReturnStatus SOPC_QualifiedName_ParseCString(SOPC_QualifiedName* qname, const char* str)
{
    SOPC_StatusCode status = SOPC_STATUS_OK;

    if (NULL == qname || NULL == str)
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (status == SOPC_STATUS_OK)
    {
        char* colon = strchr(str, ':');
        qname->NamespaceIndex = 0;

        if (NULL == colon)
        {
            status = SOPC_String_CopyFromCString(&qname->Name, str);
        }
        else
        {
            status = SOPC_strtouint16_t(str, &qname->NamespaceIndex, 10, ':');

            if (status == SOPC_STATUS_OK)
            {
                status = SOPC_String_CopyFromCString(&qname->Name, colon + 1);
            }
            else
            {
                status = SOPC_String_CopyFromCString(&qname->Name, str);
            }
        }
    }

    return status;
}

void SOPC_QualifiedName_Clear(SOPC_QualifiedName* qname)
{
    if (qname != NULL)
    {
        qname->NamespaceIndex = 0;
        SOPC_String_Clear(&qname->Name);
    }
}

void SOPC_LocalizedText_InitializeAux(void* value)
{
    SOPC_LocalizedText_Initialize((SOPC_LocalizedText*) value);
}

void SOPC_LocalizedText_Initialize(SOPC_LocalizedText* localizedText)
{
    if (localizedText != NULL)
    {
        SOPC_String_Initialize(&localizedText->defaultLocale);
        SOPC_String_Initialize(&localizedText->defaultText);
        localizedText->localizedTextList = NULL;
    }
}

static SOPC_ReturnStatus SOPC_LocalizedText_Copy_Internal(int recursionLimit,
                                                          SOPC_LocalizedText* dest,
                                                          const SOPC_LocalizedText* src)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;

    if (recursionLimit < 0)
    {
        return status;
    }
    recursionLimit--;

    if (NULL != dest && NULL != src)
    {
        status = SOPC_String_Copy(&dest->defaultLocale, &src->defaultLocale);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_String_Copy(&dest->defaultText, &src->defaultText);
        }
        if (SOPC_STATUS_OK == status && NULL != src->localizedTextList)
        {
            dest->localizedTextList = SOPC_SLinkedList_Create(INT32_MAX);
            if (NULL == dest->localizedTextList)
            {
                status = SOPC_STATUS_OUT_OF_MEMORY;
            }
            SOPC_SLinkedListIterator it = SOPC_SLinkedList_GetIterator(src->localizedTextList);
            while (SOPC_SLinkedList_HasNext(&it) && SOPC_STATUS_OK == status)
            {
                SOPC_LocalizedText* lt = (SOPC_LocalizedText*) SOPC_SLinkedList_Next(&it);
                SOPC_ASSERT(NULL != lt);
                SOPC_LocalizedText* newLt = SOPC_Malloc(sizeof(*newLt));
                SOPC_LocalizedText_Initialize(newLt);
                status = SOPC_LocalizedText_Copy_Internal(recursionLimit, newLt, lt);
                if (SOPC_STATUS_OK == status)
                {
                    void* appended = (void*) SOPC_SLinkedList_Append(dest->localizedTextList, 0, (uintptr_t) newLt);
                    if (NULL == appended)
                    {
                        status = SOPC_STATUS_OUT_OF_MEMORY;
                    }
                }
                else
                {
                    status = SOPC_STATUS_OUT_OF_MEMORY;
                }
            }
        }
        if (SOPC_STATUS_OK != status)
        {
            SOPC_LocalizedText_Clear(dest);
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_LocalizedText_Copy(SOPC_LocalizedText* dest, const SOPC_LocalizedText* src)
{
    // We authorize only 1 level of LocalizedText contained in a LocalizedText
    return SOPC_LocalizedText_Copy_Internal(1, dest, src);
}

static SOPC_ReturnStatus SOPC_LocalizedText_Compare_Internal(int recursionLimit,
                                                             const SOPC_LocalizedText* left,
                                                             const SOPC_LocalizedText* right,
                                                             int32_t* comparison)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (recursionLimit < 0)
    {
        return status;
    }
    recursionLimit--;

    if (NULL != left && NULL != right)
    {
        status = SOPC_String_Compare(&left->defaultLocale, &right->defaultLocale, false, comparison);
        if (SOPC_STATUS_OK == status && *comparison == 0)
        {
            status = SOPC_String_Compare(&left->defaultText, &right->defaultText, false, comparison);
        }
        if (SOPC_STATUS_OK == status && *comparison == 0)
        {
            if (NULL != left->localizedTextList && NULL != right->localizedTextList)
            {
                const uint32_t lengthLeft = SOPC_SLinkedList_GetLength(left->localizedTextList);
                const uint32_t lengthRight = SOPC_SLinkedList_GetLength(right->localizedTextList);
                if (lengthLeft == lengthRight)
                {
                    SOPC_SLinkedListIterator itLeft = SOPC_SLinkedList_GetIterator(left->localizedTextList);
                    SOPC_SLinkedListIterator itRight = SOPC_SLinkedList_GetIterator(right->localizedTextList);
                    while (SOPC_SLinkedList_HasNext(&itLeft) && SOPC_STATUS_OK == status && *comparison == 0)
                    {
                        SOPC_LocalizedText* ltLeft = (SOPC_LocalizedText*) SOPC_SLinkedList_Next(&itLeft);
                        SOPC_LocalizedText* ltRight = (SOPC_LocalizedText*) SOPC_SLinkedList_Next(&itRight);
                        SOPC_ASSERT(NULL != ltLeft);
                        SOPC_ASSERT(NULL != ltRight);
                        status = SOPC_LocalizedText_Compare_Internal(recursionLimit, ltLeft, ltRight, comparison);
                    }
                }
                else if (lengthLeft > lengthRight)
                {
                    *comparison = +1;
                }
                else
                {
                    *comparison = -1;
                }
            }
            else if (NULL != left->localizedTextList)
            {
                *comparison = 0 == SOPC_SLinkedList_GetLength(left->localizedTextList) ? 0 : +1;
            }
            else if (NULL != right->localizedTextList)
            {
                *comparison = 0 == SOPC_SLinkedList_GetLength(right->localizedTextList) ? 0 : -1;
            }
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_LocalizedText_Compare(const SOPC_LocalizedText* left,
                                             const SOPC_LocalizedText* right,
                                             int32_t* comparison)
{
    // We authorize only 1 level of LocalizedText contained in a LocalizedText
    return SOPC_LocalizedText_Compare_Internal(1, left, right, comparison);
}

SOPC_ReturnStatus SOPC_LocalizedText_CompareAux(const void* left, const void* right, int32_t* comparison)
{
    return SOPC_LocalizedText_Compare((const SOPC_LocalizedText*) left, (const SOPC_LocalizedText*) right, comparison);
}

SOPC_ReturnStatus SOPC_LocalizedText_CopyAux(void* dest, const void* src)
{
    return SOPC_LocalizedText_Copy((SOPC_LocalizedText*) dest, (const SOPC_LocalizedText*) src);
}

void SOPC_LocalizedText_ClearAux(void* value)
{
    SOPC_LocalizedText_Clear((SOPC_LocalizedText*) value);
}

static void SOPC_LocalizedText_ListEltFree(uint32_t id, uintptr_t val)
{
    SOPC_UNUSED_ARG(id);
    SOPC_LocalizedText_ClearAux((void*) val);
    SOPC_Free((void*) val);
}

void SOPC_LocalizedText_Clear(SOPC_LocalizedText* localizedText)
{
    if (localizedText != NULL)
    {
        SOPC_String_Clear(&localizedText->defaultLocale);
        SOPC_String_Clear(&localizedText->defaultText);

        if (NULL != localizedText->localizedTextList)
        {
            SOPC_SLinkedList_Apply(localizedText->localizedTextList, SOPC_LocalizedText_ListEltFree);
            SOPC_SLinkedList_Delete(localizedText->localizedTextList);
            localizedText->localizedTextList = NULL;
        }
    }
}

SOPC_ReturnStatus SOPC_LocalizedText_CopyFromArray(SOPC_LocalizedText* destSetOfLt,
                                                   int32_t nbElts,
                                                   const SOPC_LocalizedText* srcArrayOfLt)
{
    if (NULL == destSetOfLt || NULL != destSetOfLt->localizedTextList || destSetOfLt->defaultLocale.Length > 0 ||
        destSetOfLt->defaultText.Length > 0 || nbElts <= 0 || NULL == srcArrayOfLt)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    // Do not use LocalizedText_Copy to avoid possible set of LT in the array:
    // array are considered to contain only single value
    SOPC_ReturnStatus status = SOPC_String_Copy(&destSetOfLt->defaultLocale, &srcArrayOfLt[0].defaultLocale);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_String_Copy(&destSetOfLt->defaultText, &srcArrayOfLt[0].defaultText);
    }
    if (nbElts > 1)
    {
        destSetOfLt->localizedTextList = SOPC_SLinkedList_Create(INT32_MAX);
        status = (NULL == destSetOfLt->localizedTextList ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK);

        for (int32_t i = 1; SOPC_STATUS_OK == status && i < nbElts; i++)
        {
            SOPC_LocalizedText* lt = SOPC_Calloc(1, sizeof(SOPC_LocalizedText));
            SOPC_LocalizedText_Initialize(lt);
            if (NULL == lt)
            {
                status = SOPC_STATUS_OUT_OF_MEMORY;
            }
            // Do not use LocalizedText_Copy to avoid possible set of LT in the array:
            // array are considered to contain only single value
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_String_Copy(&lt->defaultLocale, &srcArrayOfLt[i].defaultLocale);
            }
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_String_Copy(&lt->defaultText, &srcArrayOfLt[i].defaultText);
            }
            if (SOPC_STATUS_OK == status)
            {
                void* added = (void*) SOPC_SLinkedList_Append(destSetOfLt->localizedTextList, 0, (uintptr_t) lt);
                if (lt != added)
                {
                    status = SOPC_STATUS_OUT_OF_MEMORY;
                }
            }

            if (SOPC_STATUS_OK != status)
            {
                SOPC_LocalizedText_Clear(lt);
                SOPC_Free(lt);
            }
        }
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_LocalizedText_Clear(destSetOfLt);
    }
    return status;
}

SOPC_ReturnStatus SOPC_LocalizedText_CopyToArray(SOPC_LocalizedText** dstArray,
                                                 int32_t* nbElts,
                                                 const SOPC_LocalizedText* srcSetOfLt)
{
    if (NULL == dstArray || NULL == nbElts || NULL == srcSetOfLt)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    // There is always at least 1 element (default LT: it might be empty)
    *nbElts = 1;
    if (srcSetOfLt->localizedTextList != NULL)
    {
        // And there might be a set of LT in addition to the default LT
        *nbElts += (int32_t) SOPC_SLinkedList_GetLength(srcSetOfLt->localizedTextList);
    }
    *dstArray = SOPC_Calloc(sizeof(*nbElts), sizeof(SOPC_LocalizedText));

    SOPC_ReturnStatus status = (NULL == *dstArray ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK);
    if (SOPC_STATUS_OK == status)
    {
        for (int32_t i = 0; i < *nbElts; i++)
        {
            SOPC_LocalizedText_Initialize(&(*dstArray)[i]);
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        // Avoid copying the list in root LocalizedText
        status = SOPC_String_Copy(&(*dstArray)[0].defaultLocale, &srcSetOfLt->defaultLocale);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_String_Copy(&(*dstArray)[0].defaultText, &srcSetOfLt->defaultText);
        }
        if (*nbElts > 1)
        {
            SOPC_SLinkedListIterator it = SOPC_SLinkedList_GetIterator(srcSetOfLt->localizedTextList);
            for (int32_t i = 1; SOPC_STATUS_OK == status && i < *nbElts && SOPC_SLinkedList_HasNext(&it); i++)
            {
                SOPC_LocalizedText* lt = (SOPC_LocalizedText*) SOPC_SLinkedList_Next(&it);
                status = SOPC_LocalizedText_Copy(&(*dstArray)[i], lt);
            }
        }
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_Clear_Array(nbElts, (void**) dstArray, sizeof(SOPC_LocalizedText), SOPC_LocalizedText_ClearAux);
        *nbElts = 0;
    }
    return status;
}

static SOPC_ReturnStatus SOPC_LocalizedText_AddOrSetLocale_Internal_SetSupported(SOPC_LocalizedText* destSetOfLt,
                                                                                 const SOPC_LocalizedText* src)
{
    SOPC_ASSERT(NULL != destSetOfLt);
    SOPC_ASSERT(NULL != src);
    SOPC_ASSERT(src->defaultText.Length > 0);
    // Set a new localized text we know to be supported locale

    // Compare the default locale to the one to add/set
    bool addToList = true;
    int32_t comparison = -1;
    SOPC_ReturnStatus status = SOPC_String_Compare(&destSetOfLt->defaultLocale, &src->defaultLocale, true, &comparison);
    if (SOPC_STATUS_OK != status)
    {
        return status;
    }

    if (0 == comparison)
    {
        // Default localized text is the one to set
        SOPC_String_Clear(&destSetOfLt->defaultText);
        status = SOPC_String_Copy(&destSetOfLt->defaultText, &src->defaultText);
        addToList = false;
    }
    else if (NULL != destSetOfLt->localizedTextList)
    {
        // Search for same locale already defined
        SOPC_SLinkedListIterator it = SOPC_SLinkedList_GetIterator(destSetOfLt->localizedTextList);
        while (SOPC_STATUS_OK == status && addToList && SOPC_SLinkedList_HasNext(&it))
        {
            SOPC_LocalizedText* lt = (SOPC_LocalizedText*) SOPC_SLinkedList_Next(&it);
            status = SOPC_String_Compare(&lt->defaultLocale, &src->defaultLocale, true, &comparison);
            if (SOPC_STATUS_OK != status)
            {
                return status;
            }
            // If same locale found, set it with new value
            if (0 == comparison)
            {
                // Default localized text is the one to set
                SOPC_String_Clear(&lt->defaultText);
                status = SOPC_String_Copy(&lt->defaultText, &src->defaultText);
                addToList = false;
            }
        }
    }
    else
    {
        destSetOfLt->localizedTextList = SOPC_SLinkedList_Create(INT32_MAX);
        if (NULL == destSetOfLt->localizedTextList)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    // There is no localized text existent for this locale, create a new one
    if (SOPC_STATUS_OK == status && addToList)
    {
        SOPC_LocalizedText* newLT = SOPC_Malloc(sizeof(SOPC_LocalizedText));
        SOPC_LocalizedText_Initialize(newLT);
        if (NULL == newLT)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
        else
        {
            status = SOPC_LocalizedText_Copy(newLT, src);
        }

        if (SOPC_STATUS_OK == status)
        {
            void* appended = (void*) SOPC_SLinkedList_Append(destSetOfLt->localizedTextList, 0, (uintptr_t) newLT);
            status = NULL == appended ? SOPC_STATUS_NOK : SOPC_STATUS_OK;
        }
        if (SOPC_STATUS_OK != status)
        {
            SOPC_LocalizedText_Clear(newLT);
            SOPC_Free(newLT);
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    return status;
}

static SOPC_ReturnStatus SOPC_LocalizedText_AddOrSetLocale_Internal_RemoveSupported(SOPC_LocalizedText* destSetOfLt,
                                                                                    const SOPC_LocalizedText* src)
{
    SOPC_ASSERT(NULL != destSetOfLt);
    SOPC_ASSERT(NULL != src);
    // Removing a localized text is only triggered when set with LT containing empty text for the given locale
    SOPC_ASSERT(src->defaultText.Length <= 0);
    // If locale also empty all localized texts content is cleared at once and this function is not called
    SOPC_ASSERT(src->defaultLocale.Length > 0);

    // Remove a localized text if existing for supported locale

    // Compare the default locale to the one to add/set
    int32_t comparison = -1;
    SOPC_ReturnStatus status = SOPC_String_Compare(&destSetOfLt->defaultLocale, &src->defaultLocale, true, &comparison);
    if (SOPC_STATUS_OK != status)
    {
        return status;
    }

    SOPC_LocalizedText* ltToRemoveFromList = NULL;

    if (0 == comparison)
    {
        // Default localized text is the one to set
        SOPC_String_Clear(&destSetOfLt->defaultText);
        SOPC_String_Clear(&destSetOfLt->defaultLocale);
        if (NULL != destSetOfLt->localizedTextList && SOPC_SLinkedList_GetLength(destSetOfLt->localizedTextList) > 0)
        {
            // Replace default by first available in list
            SOPC_LocalizedText* lt = (SOPC_LocalizedText*) SOPC_SLinkedList_PopHead(destSetOfLt->localizedTextList);
            SOPC_ASSERT(NULL != lt);
            status = SOPC_String_Copy(&destSetOfLt->defaultLocale, &lt->defaultLocale);
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_String_Copy(&destSetOfLt->defaultText, &lt->defaultText);
            }

            // In any case clear the localized text popped from list
            SOPC_LocalizedText_Clear(lt);
            SOPC_Free(lt);
        }
    }
    else if (NULL != destSetOfLt->localizedTextList)
    {
        // Search for same locale already defined
        SOPC_SLinkedListIterator it = SOPC_SLinkedList_GetIterator(destSetOfLt->localizedTextList);
        while (SOPC_STATUS_OK == status && NULL == ltToRemoveFromList && SOPC_SLinkedList_HasNext(&it))
        {
            SOPC_LocalizedText* lt = (SOPC_LocalizedText*) SOPC_SLinkedList_Next(&it);
            status = SOPC_String_Compare(&lt->defaultLocale, &src->defaultLocale, true, &comparison);

            // If same locale found, set it with new value
            if (SOPC_STATUS_OK == status && 0 == comparison)
            {
                ltToRemoveFromList = lt;
            }
        }

        // There is localized text existent for this locale remove it
        if (SOPC_STATUS_OK == status && NULL != ltToRemoveFromList)
        {
            SOPC_LocalizedText* ltRemoved = (SOPC_LocalizedText*) SOPC_SLinkedList_RemoveFromValuePtr(
                destSetOfLt->localizedTextList, (uintptr_t) ltToRemoveFromList);
            if (ltRemoved == ltToRemoveFromList)
            {
                SOPC_LocalizedText_Clear(ltRemoved);
                SOPC_Free(ltRemoved);
            }
            else
            {
                status = SOPC_STATUS_NOK;
            }
        }
    }

    return status;
}

SOPC_ReturnStatus SOPC_LocalizedText_AddOrSetLocale(SOPC_LocalizedText* destSetOfLt,
                                                    char** supportedLocaleIds,
                                                    const SOPC_LocalizedText* src)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL == destSetOfLt || NULL == src || NULL == supportedLocaleIds || NULL != src->localizedTextList)
    {
        return status;
    }

    if (src->defaultLocale.Length <= 0 && src->defaultText.Length <= 0)
    {
        // It indicates all locales shall be erased:
        SOPC_LocalizedText_Clear(destSetOfLt);
        status = SOPC_STATUS_OK;
    }
    else
    {
        /*
         * Check if the locale to set is supported or not including invariant locale case:
         * Part 4 §5.10.4.1 (v1.03): "Writing a null String for the locale and
         *                            a non-null String for the text is setting the text for an invariant locale."
         * Erase all locales except this one since it is an invariant LocalizedText.
         */
        bool supportedLocale = src->defaultLocale.Length <= 0;
        int index = 0;
        const char* locale = supportedLocaleIds[index];
        const char* setLocale = SOPC_String_GetRawCString(&src->defaultLocale);
        while (!supportedLocale && NULL != locale)
        {
            int res = SOPC_strcmp_ignore_case(locale, setLocale);
            supportedLocale = (0 == res);
            index++;
            locale = supportedLocaleIds[index];
        }

        if (!supportedLocale)
        {
            status = SOPC_STATUS_NOT_SUPPORTED;
        }
        else
        {
            if (src->defaultText.Length > 0)
            {
                if (destSetOfLt->defaultLocale.Length <= 0 && destSetOfLt->defaultText.Length <= 0 &&
                    (NULL == destSetOfLt->localizedTextList ||
                     0 == SOPC_SLinkedList_GetLength(destSetOfLt->localizedTextList)))
                {
                    // The target localized text is empty, only copy it
                    status = SOPC_LocalizedText_Copy(destSetOfLt, src);
                }
                else
                {
                    // Define text for an existing or new supported locale
                    status = SOPC_LocalizedText_AddOrSetLocale_Internal_SetSupported(destSetOfLt, src);
                }
            }
            else
            {
                // Remove text supported locale
                status = SOPC_LocalizedText_AddOrSetLocale_Internal_RemoveSupported(destSetOfLt, src);
            }
        }
    }

    return status;
}

static int SOPC_LocalizedText_CompareLocales(const char* s1, const char* s2, bool includesCountryRegion)
{
    if (includesCountryRegion)
    {
        return SOPC_strcmp_ignore_case(s1, s2);
    }
    else
    {
        // Compare considering the language - coutry/region separator '-' as equivalent to '\0'
        return SOPC_strcmp_ignore_case_alt_end(s1, s2, '-');
    }
}

SOPC_ReturnStatus SOPC_LocalizedText_GetPreferredLocale(SOPC_LocalizedText* dest,
                                                        char** localeIds,
                                                        const SOPC_LocalizedText* srcSetOfLt)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL == dest || NULL != dest->localizedTextList || NULL == localeIds || NULL == srcSetOfLt)
    {
        return status;
    }

    int comparisonCounter = 0; // 2 types of comparison shall be done
    // First: attempt to find exact language+coutry/region locale match
    // Second: attempt to find language match only
    bool cmpWithCountryRegion = true;
    bool localeMatch = false;

    while (!localeMatch && comparisonCounter < 2)
    {
        int index = 0;
        const char* localeId = localeIds[index];

        while (NULL != localeId && !localeMatch)
        {
            // Check all available locales in source localized text

            // Check default localized text content
            int res = SOPC_strcmp_ignore_case(localeId, SOPC_String_GetRawCString(&srcSetOfLt->defaultLocale));
            localeMatch = (0 == res);

            if (localeMatch)
            {
                status = SOPC_String_Copy(&dest->defaultLocale, &srcSetOfLt->defaultLocale);
                if (SOPC_STATUS_OK == status)
                {
                    status = SOPC_String_Copy(&dest->defaultText, &srcSetOfLt->defaultText);
                }
            }
            else if (srcSetOfLt->localizedTextList != NULL)
            {
                // Check other localized texts content
                SOPC_SLinkedListIterator it = SOPC_SLinkedList_GetIterator(srcSetOfLt->localizedTextList);
                while (!localeMatch && SOPC_SLinkedList_HasNext(&it))
                {
                    const SOPC_LocalizedText* lt = (SOPC_LocalizedText*) SOPC_SLinkedList_Next(&it);
                    SOPC_ASSERT(NULL != lt);
                    res = SOPC_LocalizedText_CompareLocales(localeId, SOPC_String_GetRawCString(&lt->defaultLocale),
                                                            cmpWithCountryRegion);
                    localeMatch = (0 == res);
                    if (localeMatch)
                    {
                        status = SOPC_LocalizedText_Copy(dest, lt);
                    }
                }
            }
            index++;
            localeId = localeIds[index];
        }
        comparisonCounter++;
        cmpWithCountryRegion = false;
    }

    if (!localeMatch)
    {
        // Set default locale
        status = SOPC_String_Copy(&dest->defaultLocale, &srcSetOfLt->defaultLocale);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_String_Copy(&dest->defaultText, &srcSetOfLt->defaultText);
        }
    }

    return status;
}

SOPC_ReturnStatus SOPC_LocalizedTextArray_GetPreferredLocale(SOPC_LocalizedText* dest,
                                                             char** preferredLocaleIds,
                                                             int32_t nbLocalizedText,
                                                             const SOPC_LocalizedText* srcArray)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL == dest || NULL != dest->localizedTextList || NULL == preferredLocaleIds || NULL == srcArray ||
        0 >= nbLocalizedText)
    {
        return status;
    }

    int comparisonCounter = 0; // 2 types of comparison shall be done
    // First: attempt to find exact language+coutry/region locale match
    // Second: attempt to find language match only
    bool cmpWithCountryRegion = true;
    bool localeMatch = false;

    // Note: if there is only 1 locale (nbLocalizedText == 1) we only set the default locale (done after the loop)
    while (nbLocalizedText > 1 && !localeMatch && comparisonCounter < 2)
    {
        int index = 0;
        const char* localeId = preferredLocaleIds[index];

        while (NULL != localeId && !localeMatch)
        {
            // Check all available locales in source localized text

            // Check other localized texts content
            for (int32_t i = 0; !localeMatch && i < nbLocalizedText; i++)
            {
                const SOPC_LocalizedText* lt = &srcArray[i];
                SOPC_ASSERT(NULL != lt);
                int res = SOPC_LocalizedText_CompareLocales(localeId, SOPC_String_GetRawCString(&lt->defaultLocale),
                                                            cmpWithCountryRegion);
                localeMatch = (0 == res);
                if (localeMatch)
                {
                    status = SOPC_LocalizedText_Copy(dest, lt);
                }
            }
            index++;
            localeId = preferredLocaleIds[index];
        }
        comparisonCounter++;
        cmpWithCountryRegion = false;
    }

    if (!localeMatch)
    {
        // Set default locale
        status = SOPC_String_Copy(&dest->defaultLocale, &srcArray[0].defaultLocale);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_String_Copy(&dest->defaultText, &srcArray[0].defaultText);
        }
    }

    return status;
}

void SOPC_ExtensionObject_InitializeAux(void* value)
{
    SOPC_ExtensionObject_Initialize((SOPC_ExtensionObject*) value);
}

void SOPC_ExtensionObject_Initialize(SOPC_ExtensionObject* extObj)
{
    if (extObj != NULL)
    {
        memset(extObj, 0, sizeof(SOPC_ExtensionObject));
        SOPC_ExpandedNodeId_Initialize(&extObj->TypeId);
        extObj->Length = -1;
    }
}

SOPC_ReturnStatus SOPC_ExtensionObject_Copy(SOPC_ExtensionObject* dest, const SOPC_ExtensionObject* src)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;

    if (NULL == dest || NULL == src)
    {
        return status;
    }

    switch (src->Encoding)
    {
    case SOPC_ExtObjBodyEncoding_None:
        status = SOPC_STATUS_OK;
        break;
    case SOPC_ExtObjBodyEncoding_ByteString:
        status = SOPC_ByteString_Copy(&dest->Body.Bstring, &src->Body.Bstring);
        break;
    case SOPC_ExtObjBodyEncoding_XMLElement:
        status = SOPC_XmlElement_Copy(&dest->Body.Xml, &src->Body.Xml);
        break;
    case SOPC_ExtObjBodyEncoding_Object:
        if (NULL != src->Body.Object.ObjType && NULL != src->Body.Object.Value)
        {
            status = SOPC_Encodeable_Create(src->Body.Object.ObjType, &dest->Body.Object.Value);
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_EncodeableObject_Copy(src->Body.Object.ObjType, dest->Body.Object.Value,
                                                    src->Body.Object.Value);
                if (SOPC_STATUS_OK == status)
                {
                    dest->Body.Object.ObjType = src->Body.Object.ObjType;
                }
                else
                {
                    SOPC_Free(dest->Body.Object.Value);
                    dest->Body.Object.Value = NULL;
                }
            }
        }
        break;
    default:
        SOPC_ASSERT(false);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ExpandedNodeId_Copy(&dest->TypeId, &src->TypeId);
    }
    if (SOPC_STATUS_OK == status)
    {
        dest->Encoding = src->Encoding;
        dest->Length = src->Length;
    }
    else
    {
        SOPC_ExtensionObject_Clear(dest);
    }

    return status;
}

SOPC_ReturnStatus SOPC_ExtensionObject_Move(SOPC_ExtensionObject* dest, SOPC_ExtensionObject* src)
{
    if (NULL == dest || NULL == src)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    dest->Encoding = src->Encoding;
    dest->Length = src->Length;
    dest->TypeId = src->TypeId;
    switch (src->Encoding)
    {
    case SOPC_ExtObjBodyEncoding_None:
        break;
    case SOPC_ExtObjBodyEncoding_ByteString:
        dest->Body.Bstring = src->Body.Bstring;
        break;
    case SOPC_ExtObjBodyEncoding_XMLElement:
        dest->Body.Xml = src->Body.Xml;
        break;
    case SOPC_ExtObjBodyEncoding_Object:
        dest->Body.Object = src->Body.Object;
        break;
    default:
        SOPC_ASSERT(false);
    }
    SOPC_ExtensionObject_Initialize(src);

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_ExtensionObject_CopyAux(void* dest, const void* src)
{
    return SOPC_ExtensionObject_Copy((SOPC_ExtensionObject*) dest, (const SOPC_ExtensionObject*) src);
}

SOPC_ReturnStatus SOPC_ExtensionObject_Compare(const SOPC_ExtensionObject* left,
                                               const SOPC_ExtensionObject* right,
                                               int32_t* comparison)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (NULL == left || NULL == right || NULL == comparison)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (left->Encoding < right->Encoding)
    {
        *comparison = -1;
    }
    else if (right->Encoding < left->Encoding)
    {
        *comparison = 1;
    }
    else
    {
        status = SOPC_ExpandedNodeId_Compare(&left->TypeId, &right->TypeId, comparison);
        if (SOPC_STATUS_OK == status && *comparison == 0)
        {
            switch (right->Encoding)
            {
            case SOPC_ExtObjBodyEncoding_None:
                *comparison = 0;
                break;
            case SOPC_ExtObjBodyEncoding_ByteString:
                status = SOPC_ByteString_Compare(&left->Body.Bstring, &right->Body.Bstring, comparison);
                break;
            case SOPC_ExtObjBodyEncoding_XMLElement:
                status = SOPC_XmlElement_Compare(&left->Body.Xml, &right->Body.Xml, comparison);
                break;
            case SOPC_ExtObjBodyEncoding_Object:
                if (NULL != left->Body.Object.ObjType && NULL != left->Body.Object.Value &&
                    NULL != right->Body.Object.ObjType && NULL != right->Body.Object.Value)
                {
                    if (left->Body.Object.ObjType == right->Body.Object.ObjType)
                    {
                        status = SOPC_EncodeableObject_Compare(left->Body.Object.ObjType, left->Body.Object.Value,
                                                               right->Body.Object.Value, comparison);
                    }
                    else
                    {
                        *comparison = left->Body.Object.ObjType > right->Body.Object.ObjType ? +1 : -1;
                    }
                }
                else
                {
                    status = SOPC_STATUS_INVALID_PARAMETERS;
                }
                break;
            default:
                SOPC_ASSERT(false);
            }
        }
    }

    return status;
}

SOPC_ReturnStatus SOPC_ExtensionObject_CompareAux(const void* left, const void* right, int32_t* comparison)
{
    return SOPC_ExtensionObject_Compare((const SOPC_ExtensionObject*) left, (const SOPC_ExtensionObject*) right,
                                        comparison);
}

void SOPC_ExtensionObject_ClearAux(void* value)
{
    SOPC_ExtensionObject_Clear((SOPC_ExtensionObject*) value);
}

void SOPC_ExtensionObject_Clear(SOPC_ExtensionObject* extObj)
{
    if (extObj != NULL)
    {
        SOPC_ExpandedNodeId_Clear(&extObj->TypeId);
        switch (extObj->Encoding)
        {
        case SOPC_ExtObjBodyEncoding_None:
            break;
        case SOPC_ExtObjBodyEncoding_ByteString:
            SOPC_ByteString_Clear(&extObj->Body.Bstring);
            break;
        case SOPC_ExtObjBodyEncoding_XMLElement:
            SOPC_XmlElement_Clear(&extObj->Body.Xml);
            break;
        case SOPC_ExtObjBodyEncoding_Object:
            if (NULL != extObj->Body.Object.Value)
            {
                extObj->Body.Object.ObjType->Clear(extObj->Body.Object.Value);
                SOPC_Free(extObj->Body.Object.Value);
            }
            break;
        default:
            SOPC_ASSERT(false);
        }
        memset(extObj, 0, sizeof(SOPC_ExtensionObject));
        extObj->Length = -1;
    }
}

static void ApplyToVariantNonArrayBuiltInType(SOPC_BuiltinId builtInTypeId,
                                              SOPC_VariantValue* val,
                                              SOPC_EncodeableObject_PfnClear* clearAuxFunction)
{
    switch (builtInTypeId)
    {
    case SOPC_Null_Id:
        break;
    case SOPC_Boolean_Id:
    case SOPC_SByte_Id:
    case SOPC_Byte_Id:
    case SOPC_Int16_Id:
    case SOPC_UInt16_Id:
    case SOPC_Int32_Id:
    case SOPC_UInt32_Id:
    case SOPC_Int64_Id:
    case SOPC_UInt64_Id:
    case SOPC_Float_Id:
    case SOPC_Double_Id:
    case SOPC_String_Id:
    case SOPC_DateTime_Id:
    case SOPC_ByteString_Id:
    case SOPC_XmlElement_Id:
    case SOPC_StatusCode_Id:
        // Note union fields are aligned: provide pointer to data
        // (i.e. first first address for those types)
        clearAuxFunction((void*) val);
        break;
    case SOPC_Guid_Id:
    case SOPC_NodeId_Id:
    case SOPC_ExpandedNodeId_Id:
    case SOPC_QualifiedName_Id:
    case SOPC_LocalizedText_Id:
    case SOPC_ExtensionObject_Id:
    case SOPC_DataValue_Id:
    case SOPC_DiagnosticInfo_Id:
        // Note union fields content are aligned: provide pointer to data
        // (i.e. content of the first field for those types)
        clearAuxFunction((void*) val->Guid);
        break;
    case SOPC_Variant_Id:
        // Part 6 Table 14 (v1.03): "The value shall not be a Variant
        //                           but it could be an array of Variants."
        // Note: Variant is not encoded in S2OPC stack for this case
        break;
    default:
        break;
    }
}

static SOPC_ReturnStatus AllocVariantArrayBuiltInType(SOPC_BuiltinId builtInTypeId,
                                                      SOPC_VariantArrayValue* array,
                                                      int32_t length)
{
    if (NULL == array)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    size_t size = 0;
    if (length > 0)
    {
        size = (size_t) length;
        switch (builtInTypeId)
        {
        case SOPC_Null_Id:
            // mantis #0003682: errata for 1.03 but not confirmed NULL array forbidden
            break; // SOPC_STATUS_NOK since a NULL must not be an array
        case SOPC_Boolean_Id:
        case SOPC_SByte_Id:
        case SOPC_Byte_Id:
        case SOPC_Int16_Id:
        case SOPC_UInt16_Id:
        case SOPC_Int32_Id:
        case SOPC_UInt32_Id:
        case SOPC_Int64_Id:
        case SOPC_UInt64_Id:
        case SOPC_Float_Id:
        case SOPC_Double_Id:
        case SOPC_String_Id:
        case SOPC_DateTime_Id:
        case SOPC_Guid_Id:
        case SOPC_ByteString_Id:
        case SOPC_XmlElement_Id:
        case SOPC_NodeId_Id:
        case SOPC_ExpandedNodeId_Id:
        case SOPC_StatusCode_Id:
        case SOPC_QualifiedName_Id:
        case SOPC_LocalizedText_Id:
        case SOPC_ExtensionObject_Id:
        case SOPC_DataValue_Id:
        case SOPC_Variant_Id:
        case SOPC_DiagnosticInfo_Id:
            // Note union fields content are aligned: provide pointer to data
            // (i.e. content of the first field)
            array->BooleanArr = SOPC_Calloc(size, SOPC_BuiltInType_HandlingTable[builtInTypeId].size);
            if (NULL != array->BooleanArr)
                return SOPC_STATUS_OK;
            break;
        default:
            break;
        }
    }
    else
    {
        return SOPC_STATUS_OK;
    }
    return SOPC_STATUS_OUT_OF_MEMORY;
}

static void ClearToVariantArrayBuiltInType(SOPC_BuiltinId builtInTypeId,
                                           SOPC_VariantArrayValue* array,
                                           int32_t* length,
                                           SOPC_EncodeableObject_PfnClear* clearAuxFunction)
{
    switch (builtInTypeId)
    {
    case SOPC_Null_Id:
        // mantis #0003682: errata for 1.03 but not confirmed NULL array forbidden
        break;
    case SOPC_Boolean_Id:
    case SOPC_SByte_Id:
    case SOPC_Byte_Id:
    case SOPC_Int16_Id:
    case SOPC_UInt16_Id:
    case SOPC_Int32_Id:
    case SOPC_UInt32_Id:
    case SOPC_Int64_Id:
    case SOPC_UInt64_Id:
    case SOPC_Float_Id:
    case SOPC_Double_Id:
    case SOPC_String_Id:
    case SOPC_DateTime_Id:
    case SOPC_Guid_Id:
    case SOPC_ByteString_Id:
    case SOPC_XmlElement_Id:
    case SOPC_NodeId_Id:
    case SOPC_ExpandedNodeId_Id:
    case SOPC_StatusCode_Id:
    case SOPC_QualifiedName_Id:
    case SOPC_LocalizedText_Id:
    case SOPC_ExtensionObject_Id:
    case SOPC_DataValue_Id:
    case SOPC_Variant_Id:
    case SOPC_DiagnosticInfo_Id:
        // Note union fields content are aligned: provide pointer to data
        // (i.e. content of the first field)
        SOPC_Clear_Array(length, (void**) &array->BooleanArr, SOPC_BuiltInType_HandlingTable[builtInTypeId].size,
                         clearAuxFunction);
        break;
    default:
        break;
    }
}

static SOPC_ReturnStatus ApplyOpToVariantNonArrayBuiltInType(SOPC_BuiltinId builtInTypeId,
                                                             SOPC_VariantValue* left,
                                                             const SOPC_VariantValue* right,
                                                             SOPC_EncodeableObject_PfnCopy* opAuxFunction)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    switch (builtInTypeId)
    {
    case SOPC_Null_Id:
        break; // SOPC_STATUS_NOK since no operation could be applied to a NULL variant
    case SOPC_Boolean_Id:
    case SOPC_SByte_Id:
    case SOPC_Byte_Id:
    case SOPC_Int16_Id:
    case SOPC_UInt16_Id:
    case SOPC_Int32_Id:
    case SOPC_UInt32_Id:
    case SOPC_Int64_Id:
    case SOPC_UInt64_Id:
    case SOPC_Float_Id:
    case SOPC_Double_Id:
    case SOPC_String_Id:
    case SOPC_DateTime_Id:
    case SOPC_ByteString_Id:
    case SOPC_XmlElement_Id:
    case SOPC_StatusCode_Id:
        // Note union fields are aligned: provide pointer to data
        // (i.e. first first address for those types)
        status = opAuxFunction((void*) left, (const void*) right);
        break;
    case SOPC_Guid_Id:
    case SOPC_NodeId_Id:
    case SOPC_ExpandedNodeId_Id:
    case SOPC_QualifiedName_Id:
    case SOPC_LocalizedText_Id:
    case SOPC_ExtensionObject_Id:
    case SOPC_DataValue_Id:
    case SOPC_DiagnosticInfo_Id:
        // Note union fields content are aligned: provide pointer to data
        // (i.e. content of the first field for those types)
        status = opAuxFunction((void*) left->Guid, (const void*) right->Guid);
        break;
    case SOPC_Variant_Id:
        // Part 6 Table 14 (v1.03): "The value shall not be a Variant
        //                           but it could be an array of Variants."
        // Note: Variant is not encoded in S2OPC stack for this case
        break;
    default:
        break;
    }
    return status;
}

static SOPC_ReturnStatus ApplyCopyToVariantArrayBuiltInType(SOPC_BuiltinId builtInTypeId,
                                                            SOPC_VariantArrayValue* arrayLeft,
                                                            const SOPC_VariantArrayValue* arrayRight,
                                                            int32_t length,
                                                            SOPC_EncodeableObject_PfnCopy* opAuxFunction)
{
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    switch (builtInTypeId)
    {
    case SOPC_Null_Id:
        // mantis #0003682: errata for 1.03 but not confirmed NULL array forbidden
        break; // SOPC_STATUS_NOK since a NULL must not be an array
    case SOPC_Boolean_Id:
    case SOPC_SByte_Id:
    case SOPC_Byte_Id:
    case SOPC_Int16_Id:
    case SOPC_UInt16_Id:
    case SOPC_Int32_Id:
    case SOPC_UInt32_Id:
    case SOPC_Int64_Id:
    case SOPC_UInt64_Id:
    case SOPC_Float_Id:
    case SOPC_Double_Id:
    case SOPC_String_Id:
    case SOPC_DateTime_Id:
    case SOPC_Guid_Id:
    case SOPC_ByteString_Id:
    case SOPC_XmlElement_Id:
    case SOPC_NodeId_Id:
    case SOPC_ExpandedNodeId_Id:
    case SOPC_StatusCode_Id:
    case SOPC_QualifiedName_Id:
    case SOPC_LocalizedText_Id:
    case SOPC_ExtensionObject_Id:
    case SOPC_DataValue_Id:
    case SOPC_Variant_Id:
    case SOPC_DiagnosticInfo_Id:
        // Note union fields content are aligned: provide pointer to data
        // (i.e. content of the first field)
        return SOPC_Copy_Array(length, (void*) arrayLeft->BooleanArr, (void*) arrayRight->BooleanArr,
                               SOPC_BuiltInType_HandlingTable[builtInTypeId].size, opAuxFunction);
    default:
        break;
    }
    return status;
}

static SOPC_EncodeableObject_PfnComp* GetBuiltInTypeCompFunction(SOPC_BuiltinId builtInTypeId)
{
    SOPC_EncodeableObject_PfnComp* compFunction = NULL;
    if (0 <= builtInTypeId && builtInTypeId <= SOPC_BUILTINID_MAX)
    {
        compFunction = SOPC_BuiltInType_HandlingTable[builtInTypeId].compare;
    }
    return compFunction;
}

static SOPC_ReturnStatus CompareVariantValue_StandardBuiltInCompare(const void* customContext,
                                                                    SOPC_BuiltinId builtInTypeId,
                                                                    const void* left,
                                                                    const void* right,
                                                                    int32_t* compResult)
{
    SOPC_UNUSED_ARG(customContext);
    SOPC_EncodeableObject_PfnComp* compFunction = GetBuiltInTypeCompFunction(builtInTypeId);
    if (NULL == compFunction)
    {
        return SOPC_STATUS_NOT_SUPPORTED;
    }
    return compFunction(left, right, compResult);
}

static SOPC_ReturnStatus CompareVariantsNonArrayBuiltInType(SOPC_BuiltinId builtInTypeId,
                                                            const SOPC_VariantValue* left,
                                                            const SOPC_VariantValue* right,
                                                            SOPC_VariantValue_PfnCompCustom* compAuxFunction,
                                                            const void* customCompContext,
                                                            int32_t* comparison)
{
    SOPC_ReturnStatus status = SOPC_STATUS_NOT_SUPPORTED;
    switch (builtInTypeId)
    {
    case SOPC_Null_Id:
        status = SOPC_STATUS_OK;
        *comparison = 0;
        break;
    case SOPC_Boolean_Id:
    case SOPC_SByte_Id:
    case SOPC_Byte_Id:
    case SOPC_Int16_Id:
    case SOPC_UInt16_Id:
    case SOPC_Int32_Id:
    case SOPC_UInt32_Id:
    case SOPC_Int64_Id:
    case SOPC_UInt64_Id:
    case SOPC_Float_Id:
    case SOPC_Double_Id:
    case SOPC_String_Id:
    case SOPC_DateTime_Id:
    case SOPC_ByteString_Id:
    case SOPC_XmlElement_Id:
    case SOPC_StatusCode_Id:
        // Note union fields are aligned: provide pointer to data
        // (i.e. first first address for those types)
        status = compAuxFunction(customCompContext, builtInTypeId, (const void*) left, (const void*) right, comparison);
        break;
    case SOPC_Guid_Id:
    case SOPC_NodeId_Id:
    case SOPC_ExpandedNodeId_Id:
    case SOPC_QualifiedName_Id:
    case SOPC_LocalizedText_Id:
    case SOPC_ExtensionObject_Id:
    case SOPC_DataValue_Id:
    case SOPC_DiagnosticInfo_Id:
        // Note union fields content are aligned: provide pointer to data
        // (i.e. content of the first field for those types)
        status = compAuxFunction(customCompContext, builtInTypeId, (const void*) left->Guid, (const void*) right->Guid,
                                 comparison);
        break;
    case SOPC_Variant_Id:
        // Part 6 Table 14 (v1.03): "The value shall not be a Variant
        //                           but it could be an array of Variants."
        // Note: Variant is not encoded in S2OPC stack for this case
        break;
    default:
        break;
    }
    return status;
}

static SOPC_ReturnStatus CompareVariantArrayBuiltInType(SOPC_BuiltinId builtInTypeId,
                                                        const SOPC_VariantArrayValue* arrayLeft,
                                                        const SOPC_VariantArrayValue* arrayRight,
                                                        int32_t length,
                                                        SOPC_VariantValue_PfnCompCustom* compAuxFunction,
                                                        const void* customCompContext,
                                                        int32_t* comparison)
{
    SOPC_ReturnStatus status = SOPC_STATUS_NOT_SUPPORTED;
    switch (builtInTypeId)
    {
    case SOPC_Null_Id:
        // mantis #0003682: errata for 1.03 but not confirmed NULL array forbidden
        break; // SOPC_STATUS_NOK since a NULL must not be an array
    case SOPC_Boolean_Id:
    case SOPC_SByte_Id:
    case SOPC_Byte_Id:
    case SOPC_Int16_Id:
    case SOPC_UInt16_Id:
    case SOPC_Int32_Id:
    case SOPC_UInt32_Id:
    case SOPC_Int64_Id:
    case SOPC_UInt64_Id:
    case SOPC_Float_Id:
    case SOPC_Double_Id:
    case SOPC_String_Id:
    case SOPC_DateTime_Id:
    case SOPC_Guid_Id:
    case SOPC_ByteString_Id:
    case SOPC_XmlElement_Id:
    case SOPC_NodeId_Id:
    case SOPC_ExpandedNodeId_Id:
    case SOPC_StatusCode_Id:
    case SOPC_QualifiedName_Id:
    case SOPC_LocalizedText_Id:
    case SOPC_ExtensionObject_Id:
    case SOPC_DataValue_Id:
    case SOPC_Variant_Id:
    case SOPC_DiagnosticInfo_Id:
        // Note union fields content are aligned: provide pointer to data
        // (i.e. content of the first field)
        return SOPC_CompCustom_Array(length, (const void*) arrayLeft->BooleanArr, (const void*) arrayRight->BooleanArr,
                                     SOPC_BuiltInType_HandlingTable[builtInTypeId].size, compAuxFunction,
                                     customCompContext, builtInTypeId, comparison);
    default:
        break;
    }
    return status;
}

SOPC_Variant* SOPC_Variant_Create(void)
{
    SOPC_Variant* variant = SOPC_Calloc(1, sizeof(SOPC_Variant));

    if (NULL == variant)
    {
        return NULL;
    }

    SOPC_Variant_Initialize(variant);
    return variant;
}

bool SOPC_Variant_Initialize_Array(SOPC_Variant* var, SOPC_BuiltinId builtInId, int32_t length)
{
    SOPC_ReturnStatus status = AllocVariantArrayBuiltInType(builtInId, &var->Value.Array.Content, length);
    if (SOPC_STATUS_OK != status)
    {
        return false;
    }
    var->ArrayType = SOPC_VariantArrayType_Array;
    var->Value.Array.Length = length;
    var->BuiltInTypeId = builtInId;
    return true;
}

void SOPC_Variant_InitializeAux(void* value)
{
    SOPC_Variant_Initialize((SOPC_Variant*) value);
}

void SOPC_Variant_Initialize(SOPC_Variant* variant)
{
    if (variant != NULL)
    {
        memset(variant, 0, sizeof(SOPC_Variant));
    }
}

void SOPC_Null_ClearAux(void* value)
{
    SOPC_UNUSED_ARG(value);
}

static SOPC_EncodeableObject_PfnClear* GetBuiltInTypeClearFunction(SOPC_BuiltinId builtInTypeId)
{
    SOPC_EncodeableObject_PfnClear* clearFunction = NULL;
    if (0 <= builtInTypeId && builtInTypeId <= SOPC_BUILTINID_MAX)
    {
        clearFunction = SOPC_BuiltInType_HandlingTable[builtInTypeId].clear;
    }
    return clearFunction;
}

SOPC_ReturnStatus SOPC_Null_CopyAux(void* dest, const void* src)
{
    SOPC_UNUSED_ARG(dest);
    SOPC_UNUSED_ARG(src);
    return SOPC_STATUS_OK;
}

static SOPC_EncodeableObject_PfnCopy* GetBuiltInTypeCopyFunction(SOPC_BuiltinId builtInTypeId)
{
    SOPC_EncodeableObject_PfnCopy* copyFunction = NULL;
    if (0 <= builtInTypeId && builtInTypeId <= SOPC_BUILTINID_MAX)
    {
        copyFunction = SOPC_BuiltInType_HandlingTable[builtInTypeId].copy;
    }
    return copyFunction;
}

SOPC_ReturnStatus SOPC_Null_CompareAux(const void* dest, const void* src, int32_t* comparison)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != dest && NULL != src && NULL != comparison)
    {
        *comparison = 0;
        status = SOPC_STATUS_OK;
    }
    return status;
}

static SOPC_ReturnStatus AllocVariantNonArrayBuiltInType(SOPC_BuiltinId builtInTypeId, SOPC_VariantValue* val)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    switch (builtInTypeId)
    {
    case SOPC_Null_Id:
    case SOPC_Boolean_Id:
    case SOPC_SByte_Id:
    case SOPC_Byte_Id:
    case SOPC_Int16_Id:
    case SOPC_UInt16_Id:
    case SOPC_Int32_Id:
    case SOPC_UInt32_Id:
    case SOPC_Int64_Id:
    case SOPC_UInt64_Id:
    case SOPC_Float_Id:
    case SOPC_Double_Id:
    case SOPC_String_Id:
    case SOPC_DateTime_Id:
    case SOPC_ByteString_Id:
    case SOPC_XmlElement_Id:
    case SOPC_StatusCode_Id:
        break;
    case SOPC_Guid_Id:
    case SOPC_NodeId_Id:
    case SOPC_ExpandedNodeId_Id:
    case SOPC_QualifiedName_Id:
    case SOPC_LocalizedText_Id:
    case SOPC_ExtensionObject_Id:
    case SOPC_DataValue_Id:
    case SOPC_DiagnosticInfo_Id:
        // Note union fields content are aligned: provide pointer to data
        // (i.e. content of the first field for those types)
        val->Guid = SOPC_Calloc(1, SOPC_BuiltInType_HandlingTable[builtInTypeId].size);
        if (NULL == val->Guid)
        {
            status = SOPC_STATUS_NOK;
        }
        break;
    case SOPC_Variant_Id:
        // Part 6 Table 14 (v1.03): "The value shall not be a Variant
        //                           but it could be an array of Variants."
        // Note: Variant is not encoded in S2OPC stack for this case
        break;
    default:
        status = SOPC_STATUS_INVALID_PARAMETERS;
        break;
    }
    return status;
}

static void FreeVariantNonArrayBuiltInType(SOPC_BuiltinId builtInTypeId, SOPC_VariantValue* val)
{
    switch (builtInTypeId)
    {
    case SOPC_Null_Id:
    case SOPC_Boolean_Id:
    case SOPC_SByte_Id:
    case SOPC_Byte_Id:
    case SOPC_Int16_Id:
    case SOPC_UInt16_Id:
    case SOPC_Int32_Id:
    case SOPC_UInt32_Id:
    case SOPC_Int64_Id:
    case SOPC_UInt64_Id:
    case SOPC_Float_Id:
    case SOPC_Double_Id:
    case SOPC_String_Id:
    case SOPC_DateTime_Id:
    case SOPC_ByteString_Id:
    case SOPC_XmlElement_Id:
    case SOPC_StatusCode_Id:
        break;
    case SOPC_Guid_Id:
    case SOPC_NodeId_Id:
    case SOPC_ExpandedNodeId_Id:
    case SOPC_QualifiedName_Id:
    case SOPC_LocalizedText_Id:
    case SOPC_ExtensionObject_Id:
    case SOPC_DataValue_Id:
    case SOPC_DiagnosticInfo_Id:
        // Note union fields content are aligned: provide pointer to data
        // (i.e. content of the first field for those types)
        if (NULL != val->Guid)
        {
            SOPC_Free((void*) val->Guid);
        }
        val->Guid = NULL;
        break;
    case SOPC_Variant_Id:
        // Part 6 Table 14 (v1.03): "The value shall not be a Variant
        //                           but it could be an array of Variants."
        // Note: Variant is not encoded in S2OPC stack for this case
        break;
    default:
        break;
    }
}

SOPC_ReturnStatus SOPC_Variant_Copy(SOPC_Variant* dest, const SOPC_Variant* src)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    bool error = false;
    int64_t matrixLength = 1; // For multiplication to compute from dimensions values
    int32_t idx = 0;
    if (dest != NULL && src != NULL)
    {
        SOPC_EncodeableObject_PfnCopy* copyFunction = GetBuiltInTypeCopyFunction(src->BuiltInTypeId);
        if (NULL == copyFunction)
            return SOPC_STATUS_NOK;

        switch (src->ArrayType)
        {
        case SOPC_VariantArrayType_SingleValue:
            status = AllocVariantNonArrayBuiltInType(src->BuiltInTypeId, &dest->Value);
            if (SOPC_STATUS_OK == status)
            {
                status =
                    ApplyOpToVariantNonArrayBuiltInType(src->BuiltInTypeId, &dest->Value, &src->Value, copyFunction);
                if (SOPC_STATUS_OK != status)
                {
                    FreeVariantNonArrayBuiltInType(src->BuiltInTypeId, &dest->Value);
                }
            }
            break;
        case SOPC_VariantArrayType_Array:
            if (src->Value.Array.Length >= 0)
            {
                status = AllocVariantArrayBuiltInType(src->BuiltInTypeId, &dest->Value.Array.Content,
                                                      src->Value.Array.Length);
                if (SOPC_STATUS_OK == status)
                {
                    dest->Value.Array.Length = src->Value.Array.Length;
                    status = ApplyCopyToVariantArrayBuiltInType(src->BuiltInTypeId, &dest->Value.Array.Content,
                                                                &src->Value.Array.Content, src->Value.Array.Length,
                                                                copyFunction);
                }
                if (SOPC_STATUS_OK != status)
                {
                    ClearToVariantArrayBuiltInType(src->BuiltInTypeId, &dest->Value.Array.Content,
                                                   &dest->Value.Array.Length,
                                                   GetBuiltInTypeClearFunction(src->BuiltInTypeId));
                }
            }
            else if (src->Value.Array.Length == -1)
            {
                // <null> array is equal to empty array. Implement result as empty array
                status = SOPC_STATUS_OK;
                dest->Value.Array.Length = 0;
                // dest->Value.Array.Content is not significant in that case
            }
            break;
        case SOPC_VariantArrayType_Matrix:
            if (src->Value.Matrix.Dimensions == 0)
            {
                matrixLength = 0;
                status = SOPC_STATUS_OK;
            }
            else
            {
                for (idx = 0; idx < src->Value.Matrix.Dimensions && false == error; idx++)
                {
                    if (src->Value.Matrix.ArrayDimensions[idx] > 0 &&
                        matrixLength * src->Value.Matrix.ArrayDimensions[idx] <= INT32_MAX)
                    {
                        matrixLength *= src->Value.Matrix.ArrayDimensions[idx];
                    }
                    else
                    {
                        error = true;
                    }
                }
                if (false == error && src->Value.Matrix.Dimensions > 0 &&
                    (uint64_t) src->Value.Matrix.Dimensions * sizeof(int32_t) <= SIZE_MAX)
                {
                    dest->Value.Matrix.ArrayDimensions =
                        SOPC_Malloc((size_t) src->Value.Matrix.Dimensions * sizeof(int32_t));
                    if (NULL != dest->Value.Matrix.ArrayDimensions)
                    {
                        dest->Value.Matrix.Dimensions = src->Value.Matrix.Dimensions;
                        memcpy(dest->Value.Matrix.ArrayDimensions, src->Value.Matrix.ArrayDimensions,
                               (size_t) src->Value.Matrix.Dimensions * sizeof(int32_t));
                        status = AllocVariantArrayBuiltInType(src->BuiltInTypeId, &dest->Value.Matrix.Content,
                                                              (int32_t) matrixLength);
                        if (SOPC_STATUS_OK == status)
                        {
                            status = ApplyCopyToVariantArrayBuiltInType(src->BuiltInTypeId, &dest->Value.Matrix.Content,
                                                                        &src->Value.Matrix.Content,
                                                                        (int32_t) matrixLength, copyFunction);
                            if (SOPC_STATUS_OK != status)
                            {
                                ClearToVariantArrayBuiltInType(src->BuiltInTypeId, &dest->Value.Matrix.Content,
                                                               (int32_t*) &matrixLength,
                                                               GetBuiltInTypeClearFunction(src->BuiltInTypeId));
                                SOPC_Free(dest->Value.Matrix.ArrayDimensions);
                                dest->Value.Matrix.ArrayDimensions = NULL;
                            }
                        }
                        else
                        {
                            SOPC_Free(dest->Value.Matrix.ArrayDimensions);
                            dest->Value.Matrix.ArrayDimensions = NULL;
                        }
                    }
                }
            }
            break;
        default:
            break;
        }
        if (SOPC_STATUS_OK == status)
        {
            dest->BuiltInTypeId = src->BuiltInTypeId;
            dest->ArrayType = src->ArrayType;
        }
        else
        {
            SOPC_Variant_Clear(dest);
        }

        return status;
    }

    return status;
}

SOPC_ReturnStatus SOPC_Variant_ShallowCopy(SOPC_Variant* dest, const SOPC_Variant* src)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (dest != NULL && src != NULL)
    {
        *dest = *src;
        dest->DoNotClear = true;
        status = SOPC_STATUS_OK;
    }
    return status;
}

void SOPC_Variant_Move(SOPC_Variant* dst, SOPC_Variant* src)
{
    SOPC_ASSERT(src != NULL);
    SOPC_ASSERT(dst != NULL);

    *dst = *src;
    src->DoNotClear = true;
}

SOPC_ReturnStatus SOPC_Variant_CompareCustom(SOPC_VariantValue_PfnCompCustom* compCustom,
                                             const void* compCustomContext,
                                             const SOPC_Variant* left,
                                             const SOPC_Variant* right,
                                             int32_t* comparison)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    bool error = false;
    int64_t matrixLength = 1; // For multiplication to compute from dimensions values
    int32_t idx = 0;
    if (left != NULL && right != NULL && comparison != NULL)
    {
        if (left->BuiltInTypeId == right->BuiltInTypeId)
        {
            if (left->ArrayType == right->ArrayType)
            {
                switch (right->ArrayType)
                {
                case SOPC_VariantArrayType_SingleValue:
                    status = CompareVariantsNonArrayBuiltInType(right->BuiltInTypeId, &left->Value, &right->Value,
                                                                compCustom, compCustomContext, comparison);
                    break;
                case SOPC_VariantArrayType_Array:
                    if (left->Value.Array.Length < right->Value.Array.Length)
                    {
                        status = SOPC_STATUS_OK;
                        *comparison = -1;
                    }
                    else if (left->Value.Array.Length > right->Value.Array.Length)
                    {
                        status = SOPC_STATUS_OK;
                        *comparison = 1;
                    }
                    else
                    {
                        status = CompareVariantArrayBuiltInType(left->BuiltInTypeId, &left->Value.Array.Content,
                                                                &right->Value.Array.Content, left->Value.Array.Length,
                                                                compCustom, compCustomContext, comparison);
                    }
                    break;
                case SOPC_VariantArrayType_Matrix:
                    if (left->Value.Matrix.Dimensions < right->Value.Matrix.Dimensions)
                    {
                        status = SOPC_STATUS_OK;
                        *comparison = -1;
                    }
                    else if (left->Value.Matrix.Dimensions > right->Value.Matrix.Dimensions)
                    {
                        status = SOPC_STATUS_OK;
                        *comparison = 1;
                    }
                    else
                    {
                        *comparison = 0;
                        for (idx = 0; idx < left->Value.Matrix.Dimensions && false == error && *comparison == 0; idx++)
                        {
                            if (left->Value.Matrix.ArrayDimensions[idx] > 0 &&
                                matrixLength * left->Value.Matrix.ArrayDimensions[idx] <= INT32_MAX)
                            {
                                if (left->Value.Matrix.ArrayDimensions[idx] < right->Value.Matrix.ArrayDimensions[idx])
                                {
                                    status = SOPC_STATUS_OK;
                                    *comparison = -1;
                                }
                                else if (left->Value.Matrix.ArrayDimensions[idx] >
                                         right->Value.Matrix.ArrayDimensions[idx])
                                {
                                    status = SOPC_STATUS_OK;
                                    *comparison = 1;
                                }
                                else
                                {
                                    matrixLength *= left->Value.Matrix.ArrayDimensions[idx];
                                }
                            }
                            else
                            {
                                error = true;
                            }
                        }
                        if (matrixLength <= 0 || matrixLength > INT32_MAX)
                        {
                            error = true;
                        }
                        if (!error && *comparison == 0)
                        {
                            status = CompareVariantArrayBuiltInType(
                                left->BuiltInTypeId, &left->Value.Matrix.Content, &right->Value.Matrix.Content,
                                (int32_t) matrixLength, compCustom, compCustomContext, comparison);
                        }
                    }
                    break;
                default:
                    break;
                }
            }
            else
            {
                status = SOPC_STATUS_OK;
                *comparison = (int32_t) left->ArrayType - (int32_t) right->ArrayType;
            }
        }
        else
        {
            status = SOPC_STATUS_OK;
            *comparison = (int32_t) left->BuiltInTypeId - (int32_t) right->BuiltInTypeId;
        }
    }

    return status;
}

SOPC_ReturnStatus SOPC_Variant_Compare(const SOPC_Variant* left, const SOPC_Variant* right, int32_t* comparison)
{
    return SOPC_Variant_CompareCustom(&CompareVariantValue_StandardBuiltInCompare, NULL, left, right, comparison);
}

SOPC_ReturnStatus SOPC_Variant_CompareAux(const void* left, const void* right, int32_t* comparison)
{
    return SOPC_Variant_Compare((const SOPC_Variant*) left, (const SOPC_Variant*) right, comparison);
}

SOPC_ReturnStatus SOPC_Variant_CompareCustomRange(SOPC_VariantValue_PfnCompCustom* compCustom,
                                                  const void* compCustomContext,
                                                  const SOPC_Variant* left,
                                                  const SOPC_Variant* right,
                                                  const SOPC_NumericRange* range,
                                                  int32_t* comparison)
{
    if (NULL == left || NULL == right)
    {
        // To be consistent with SOPC_Variant_Compare
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (NULL == range)
    {
        return SOPC_Variant_CompareCustom(compCustom, compCustomContext, left, right, comparison);
    }

    SOPC_Variant *left_sub = SOPC_Variant_Create(), *right_sub = SOPC_Variant_Create();

    if (NULL == left_sub || NULL == right_sub)
    {
        SOPC_Variant_Delete(left_sub);
        SOPC_Variant_Delete(right_sub);
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    SOPC_ReturnStatus status = SOPC_Variant_GetRange(left_sub, left, range);

    if (status == SOPC_STATUS_OK)
    {
        status = SOPC_Variant_GetRange(right_sub, right, range);
    }

    if (status == SOPC_STATUS_OK)
    {
        status = SOPC_Variant_CompareCustom(compCustom, compCustomContext, left_sub, right_sub, comparison);
    }

    SOPC_Variant_Delete(left_sub);
    SOPC_Variant_Delete(right_sub);

    return status;
}

SOPC_ReturnStatus SOPC_Variant_CompareRange(const SOPC_Variant* left,
                                            const SOPC_Variant* right,
                                            const SOPC_NumericRange* range,
                                            int32_t* comparison)
{
    return SOPC_Variant_CompareCustomRange(&CompareVariantValue_StandardBuiltInCompare, NULL, left, right, range,
                                           comparison);
}

SOPC_ReturnStatus SOPC_Variant_CopyAux(void* dest, const void* src)
{
    return SOPC_Variant_Copy((SOPC_Variant*) dest, (const SOPC_Variant*) src);
}

void SOPC_Variant_ClearAux(void* value)
{
    SOPC_Variant_Clear((SOPC_Variant*) value);
}

void SOPC_Variant_Clear(SOPC_Variant* variant)
{
    bool error = false;
    int64_t matrixLength = 1; // For multiplication to compute from dimensions values
    int32_t idx = 0;
    if (variant != NULL)
    {
        if (false == variant->DoNotClear)
        {
            SOPC_EncodeableObject_PfnClear* clearFunction = GetBuiltInTypeClearFunction(variant->BuiltInTypeId);
            if (NULL == clearFunction)
                return;

            switch (variant->ArrayType)
            {
            case SOPC_VariantArrayType_SingleValue:
                ApplyToVariantNonArrayBuiltInType(variant->BuiltInTypeId, &variant->Value, clearFunction);
                FreeVariantNonArrayBuiltInType(variant->BuiltInTypeId, &variant->Value);
                break;
            case SOPC_VariantArrayType_Array:
                ClearToVariantArrayBuiltInType(variant->BuiltInTypeId, &variant->Value.Array.Content,
                                               &variant->Value.Array.Length, clearFunction);
                break;
            case SOPC_VariantArrayType_Matrix:
                if (variant->Value.Matrix.Dimensions == 0)
                {
                    matrixLength = 0;
                }
                for (idx = 0; idx < variant->Value.Matrix.Dimensions && false == error; idx++)
                {
                    if (variant->Value.Matrix.ArrayDimensions[idx] > 0 &&
                        matrixLength * variant->Value.Matrix.ArrayDimensions[idx] <= INT32_MAX)
                    {
                        matrixLength *= variant->Value.Matrix.ArrayDimensions[idx];
                    }
                    else
                    {
                        error = true;
                    }
                }
                if (false == error)
                {
                    SOPC_Free(variant->Value.Matrix.ArrayDimensions);
                    variant->Value.Matrix.ArrayDimensions = NULL;
                    ClearToVariantArrayBuiltInType(variant->BuiltInTypeId, &variant->Value.Matrix.Content,
                                                   (int32_t*) &matrixLength, clearFunction);
                    variant->Value.Matrix.Dimensions = 0;
                }
                break;
            default:
                break;
            }
        }

        // Reset internal properties
        SOPC_Variant_Initialize(variant);
    }
}

void SOPC_Variant_Delete(SOPC_Variant* variant)
{
    if (NULL == variant)
    {
        return;
    }

    SOPC_Variant_Clear(variant);
    SOPC_Free(variant);
}

void SOPC_Variant_Print(SOPC_Variant* pvar)
{
    size_t i;
    uint8_t c;
    SOPC_CONSOLE_PRINTF("Variant @%p", (void*) pvar);
    char* s = NULL;

    if (NULL == pvar)
        return;

    int32_t dimensions = -1;
    switch (pvar->ArrayType)
    {
    case SOPC_VariantArrayType_SingleValue:
        dimensions = 0;
        break;
    case SOPC_VariantArrayType_Array:
        dimensions = 1;
        break;
    case SOPC_VariantArrayType_Matrix:
        dimensions = pvar->Value.Matrix.Dimensions;
        break;
    default:
        break;
    }

    SOPC_CONSOLE_PRINTF(":\n  TypeId %i [dim=%" PRIi32 "]: ", (int) pvar->BuiltInTypeId, dimensions);

    if (0 != dimensions)
    {
        SOPC_CONSOLE_PRINTF("[...]\n");
        return;
    }

    switch (pvar->BuiltInTypeId)
    {
    case SOPC_Null_Id:
        SOPC_CONSOLE_PRINTF("Null\n");
        break;
    case SOPC_Boolean_Id:
        SOPC_CONSOLE_PRINTF("Boolean\n  Value: %" PRIu8 "\n", pvar->Value.Boolean);
        break;
    case SOPC_SByte_Id:
        SOPC_CONSOLE_PRINTF("SByte\n  Value: %" PRIi8 "\n", pvar->Value.Sbyte);
        break;
    case SOPC_Byte_Id:
        SOPC_CONSOLE_PRINTF("Byte\n  Value: %" PRIu8 "\n", pvar->Value.Byte);
        break;
    case SOPC_Int16_Id:
        SOPC_CONSOLE_PRINTF("Int16\n  Value: %" PRIi16 "\n", pvar->Value.Int16);
        break;
    case SOPC_UInt16_Id:
        SOPC_CONSOLE_PRINTF("UInt16\n  Value: %" PRIu16 "\n", pvar->Value.Uint16);
        break;
    case SOPC_Int32_Id:
        SOPC_CONSOLE_PRINTF("Int32\n  Value: %" PRIi32 "\n", pvar->Value.Int32);
        break;
    case SOPC_UInt32_Id:
        SOPC_CONSOLE_PRINTF("UInt32\n  Value: %" PRIu32 "\n", pvar->Value.Uint32);
        break;
    case SOPC_Int64_Id:
        SOPC_CONSOLE_PRINTF("Int64\n  Value: %" PRIi64 "\n", pvar->Value.Int64);
        break;
    case SOPC_UInt64_Id:
        SOPC_CONSOLE_PRINTF("UInt64\n  Value: %" PRIu64 "\n", pvar->Value.Uint64);
        break;
    case SOPC_Float_Id:
        SOPC_CONSOLE_PRINTF("Float\n  Value: %g\n", pvar->Value.Floatv);
        break;
    case SOPC_Double_Id:
        SOPC_CONSOLE_PRINTF("Double\n  Value: %g\n", pvar->Value.Doublev);
        break;
    case SOPC_String_Id:
        SOPC_CONSOLE_PRINTF("String\n  Value: \"%*.*s\"\n", (int) pvar->Value.String.Length,
                            (int) pvar->Value.String.Length, pvar->Value.String.Data);
        break;
    case SOPC_ByteString_Id:
        SOPC_CONSOLE_PRINTF("ByteString\n  Length: %" PRIi32 "\n  Value: \"", pvar->Value.Bstring.Length);
        /* Pretty print */
        for (i = 0; i < (size_t) pvar->Value.Bstring.Length; ++i)
        {
            c = pvar->Value.Bstring.Data[i];
            if (0x20 <= c && c < 0x80)
                /* Displayable ascii range */
                SOPC_CONSOLE_PRINTF("%c", c);
            else
                /* Special char */
                SOPC_CONSOLE_PRINTF("\\x%02" PRIX8, c);
        }
        SOPC_CONSOLE_PRINTF("\"\n");
        break;
    case SOPC_XmlElement_Id:
        SOPC_CONSOLE_PRINTF("XmlElement\n  Length: %" PRIi32 "\n  Value: \"", pvar->Value.XmlElt.Length);
        /* Pretty print */
        for (i = 0; i < (size_t) pvar->Value.XmlElt.Length; ++i)
        {
            c = pvar->Value.XmlElt.Data[i];
            if (0x20 <= c && c < 0x80)
                /* Displayable ascii range */
                SOPC_CONSOLE_PRINTF("%c", c);
            else
                /* Special char */
                SOPC_CONSOLE_PRINTF("\\x%02" PRIX8, c);
        }
        SOPC_CONSOLE_PRINTF("\"\n");
        break;
    case SOPC_NodeId_Id:
        s = SOPC_NodeId_ToCString(pvar->Value.NodeId);
        SOPC_CONSOLE_PRINTF("NodeId\n  Value: %s\n", s);
        SOPC_Free(s);
        s = NULL;
        break;
    case SOPC_StatusCode_Id:
        SOPC_CONSOLE_PRINTF("StatusCode\n  Value: %" PRIX32 "\n", pvar->Value.Status);
        break;
    case SOPC_DateTime_Id:
        s = SOPC_Time_GetString(pvar->Value.Date, true, false);
        SOPC_CONSOLE_PRINTF("DateTime = %s\n", s);
        SOPC_Free(s);
        s = NULL;
        break;
    case SOPC_Guid_Id:
        s = SOPC_Guid_ToCString(pvar->Value.Guid);
        SOPC_CONSOLE_PRINTF("Guid = '%s'\n", s);
        SOPC_Free(s);
        s = NULL;
        break;
    case SOPC_ExpandedNodeId_Id:
        s = SOPC_NodeId_ToCString(&pvar->Value.ExpNodeId->NodeId);
        SOPC_CONSOLE_PRINTF("ExpandedNodeId\n  NsURI: %s\n  NodeId: %s\n  ServerIdx: %" PRIu32 "\n",
                            SOPC_String_GetRawCString(&pvar->Value.ExpNodeId->NamespaceUri), s,
                            pvar->Value.ExpNodeId->ServerIndex);
        SOPC_Free(s);
        s = NULL;
        break;
    case SOPC_QualifiedName_Id:
        SOPC_CONSOLE_PRINTF("QualifiedName = %" PRIu16 ":%s\n", pvar->Value.Qname->NamespaceIndex,
                            SOPC_String_GetRawCString(&pvar->Value.Qname->Name));
        break;
    case SOPC_LocalizedText_Id:
        SOPC_CONSOLE_PRINTF("LocalizedText (default only) = [%s] %s\n",
                            SOPC_String_GetRawCString(&pvar->Value.LocalizedText->defaultLocale),
                            SOPC_String_GetRawCString(&pvar->Value.LocalizedText->defaultText));
        break;
    case SOPC_ExtensionObject_Id:
        SOPC_CONSOLE_PRINTF("ExtensionObject: <print not implemented>\n");
        break;
    case SOPC_DataValue_Id:
        SOPC_CONSOLE_PRINTF("DataValue: <print not implemented>\n");
        break;
    case SOPC_Variant_Id: /* This one does not have an implementation at all */
        SOPC_CONSOLE_PRINTF("Variant: <print not implemented>\n");
        break;
    case SOPC_DiagnosticInfo_Id:
        SOPC_CONSOLE_PRINTF("DiagnosticInfo: <print not implemented>\n");
        break;
    default:
        SOPC_CONSOLE_PRINTF("<print not implemented>\n");
        break;
    }
}

void SOPC_DataValue_InitializeAux(void* value)
{
    SOPC_DataValue_Initialize((SOPC_DataValue*) value);
}

void SOPC_DataValue_Initialize(SOPC_DataValue* dataValue)
{
    if (dataValue != NULL)
    {
        memset(dataValue, 0, sizeof(SOPC_DataValue));
    }
}

SOPC_ReturnStatus SOPC_DataValue_Copy(SOPC_DataValue* dest, const SOPC_DataValue* src)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != dest && NULL != src)
    {
        status = SOPC_Variant_Copy(&dest->Value, &src->Value);
        if (SOPC_STATUS_OK == status)
        {
            dest->Status = src->Status;
            dest->ServerPicoSeconds = src->ServerPicoSeconds;
            dest->ServerTimestamp = src->ServerTimestamp;
            dest->SourcePicoSeconds = src->SourcePicoSeconds;
            dest->SourceTimestamp = src->SourceTimestamp;
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_DataValue_Compare(const SOPC_DataValue* left, const SOPC_DataValue* right, int32_t* comparison)
{
    return SOPC_DataValue_CompareRange(left, right, NULL, comparison);
}

SOPC_ReturnStatus SOPC_DataValue_CompareAux(const void* left, const void* right, int32_t* comparison)
{
    return SOPC_DataValue_Compare((const SOPC_DataValue*) left, (const SOPC_DataValue*) right, comparison);
}

SOPC_ReturnStatus SOPC_DataValue_CompareRange(const SOPC_DataValue* left,
                                              const SOPC_DataValue* right,
                                              const SOPC_NumericRange* range,
                                              int32_t* comparison)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != left && NULL != right && NULL != comparison)
    {
        if (left->Status < right->Status)
        {
            status = SOPC_STATUS_OK;
            *comparison = -1;
        }
        else if (right->Status < left->Status)
        {
            status = SOPC_STATUS_OK;
            *comparison = 1;
        }
        else
        {
            status = SOPC_DateTime_Compare(&left->ServerTimestamp, &right->ServerTimestamp, comparison);
        }
        if (SOPC_STATUS_OK == status && *comparison == 0)
        {
            if (left->ServerPicoSeconds < right->ServerPicoSeconds)
            {
                status = SOPC_STATUS_OK;
                *comparison = -1;
            }
            else if (right->ServerPicoSeconds < left->ServerPicoSeconds)
            {
                status = SOPC_STATUS_OK;
                *comparison = 1;
            }
        }
        if (SOPC_STATUS_OK == status && *comparison == 0)
        {
            status = SOPC_DateTime_Compare(&left->SourceTimestamp, &right->SourceTimestamp, comparison);
        }
        if (SOPC_STATUS_OK == status && *comparison == 0)
        {
            if (left->SourcePicoSeconds < right->SourcePicoSeconds)
            {
                status = SOPC_STATUS_OK;
                *comparison = -1;
            }
            else if (right->SourcePicoSeconds < left->SourcePicoSeconds)
            {
                status = SOPC_STATUS_OK;
                *comparison = 1;
            }
        }
        if (SOPC_STATUS_OK == status && *comparison == 0)
        {
            status = SOPC_Variant_CompareRange(&left->Value, &right->Value, range, comparison);
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_DataValue_CopyAux(void* dest, const void* src)
{
    return SOPC_DataValue_Copy((SOPC_DataValue*) dest, (const SOPC_DataValue*) src);
}

void SOPC_DataValue_ClearAux(void* value)
{
    SOPC_DataValue_Clear((SOPC_DataValue*) value);
}
void SOPC_DataValue_Clear(SOPC_DataValue* dataValue)
{
    if (dataValue != NULL)
    {
        SOPC_Variant_Clear(&dataValue->Value);
        SOPC_StatusCode_Clear(&dataValue->Status);
        SOPC_DateTime_Clear(&dataValue->SourceTimestamp);
        SOPC_DateTime_Clear(&dataValue->ServerTimestamp);
        dataValue->SourcePicoSeconds = 0;
        dataValue->ServerPicoSeconds = 0;
    }
}

void SOPC_Initialize_Array(int32_t noOfElts,
                           void* eltsArray,
                           size_t sizeOfElt,
                           SOPC_EncodeableObject_PfnInitialize* initFct)
{
    SOPC_ASSERT(NULL != eltsArray);
    size_t pos = 0;
    SOPC_Byte* byteArray = eltsArray;

    for (size_t idx = 0; idx < (size_t) noOfElts; idx++)
    {
        pos = idx * sizeOfElt;
        initFct(&(byteArray[pos]));
    }
}

SOPC_ReturnStatus SOPC_Copy_Array(int32_t noOfElts,
                                  void* eltsArrayDest,
                                  const void* eltsArraySrc,
                                  size_t sizeOfElt,
                                  SOPC_EncodeableObject_PfnCopy* opFct)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    size_t idx = 0;
    size_t pos = 0;
    SOPC_Byte* byteArrayLeft = eltsArrayDest;
    const SOPC_Byte* byteArrayRight = eltsArraySrc;
    if (noOfElts > 0 && byteArrayLeft != NULL && byteArrayRight != NULL)
    {
        for (idx = 0; idx < (size_t) noOfElts && SOPC_STATUS_OK == status; idx++)
        {
            pos = idx * sizeOfElt;
            status = opFct(&(byteArrayLeft[pos]), &(byteArrayRight[pos]));
        }
    }
    else if (0 == noOfElts && NULL == byteArrayLeft && NULL == byteArrayRight)
    {
        status = SOPC_STATUS_OK;
    }
    else
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }
    return status;
}

SOPC_ReturnStatus SOPC_Comp_Array(int32_t noOfElts,
                                  const void* eltsArrayLeft,
                                  const void* eltsArrayRight,
                                  size_t sizeOfElt,
                                  SOPC_EncodeableObject_PfnComp* compFct,
                                  int32_t* comparisonResult)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;

    if (0 == sizeOfElt || NULL == compFct || NULL == comparisonResult)
    {
        return status;
    }

    size_t idx = 0;
    size_t pos = 0;
    const SOPC_Byte* byteArrayLeft = eltsArrayLeft;
    const SOPC_Byte* byteArrayRight = eltsArrayRight;
    if (noOfElts > 0 && byteArrayLeft != NULL && byteArrayRight != NULL)
    {
        *comparisonResult = 0;
        status = SOPC_STATUS_OK;
        for (idx = 0; idx < (size_t) noOfElts && SOPC_STATUS_OK == status && *comparisonResult == 0; idx++)
        {
            pos = idx * sizeOfElt;
            status = compFct(&(byteArrayLeft[pos]), &(byteArrayRight[pos]), comparisonResult);
        }
    }
    else if (0 == noOfElts)
    {
        *comparisonResult = 0;
        status = SOPC_STATUS_OK;
    }
    return status;
}

SOPC_ReturnStatus SOPC_CompCustom_Array(int32_t noOfElts,
                                        const void* eltsArrayLeft,
                                        const void* eltsArrayRight,
                                        size_t sizeOfElt,
                                        SOPC_VariantValue_PfnCompCustom* compCustomFct,
                                        const void* customCompContext,
                                        SOPC_BuiltinId builtInId,
                                        int32_t* comparisonResult)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;

    if (0 == sizeOfElt || NULL == compCustomFct || NULL == comparisonResult)
    {
        return status;
    }

    size_t idx = 0;
    size_t pos = 0;
    const SOPC_Byte* byteArrayLeft = eltsArrayLeft;
    const SOPC_Byte* byteArrayRight = eltsArrayRight;
    if (noOfElts > 0 && byteArrayLeft != NULL && byteArrayRight != NULL)
    {
        *comparisonResult = 0;
        status = SOPC_STATUS_OK;
        for (idx = 0; idx < (size_t) noOfElts && SOPC_STATUS_OK == status && *comparisonResult == 0; idx++)
        {
            pos = idx * sizeOfElt;
            status = compCustomFct(customCompContext, builtInId, &(byteArrayLeft[pos]), &(byteArrayRight[pos]),
                                   comparisonResult);
        }
    }
    else if (0 == noOfElts)
    {
        *comparisonResult = 0;
        status = SOPC_STATUS_OK;
    }
    return status;
}

void SOPC_Clear_Array(int32_t* noOfElts, void** eltsArray, size_t sizeOfElt, SOPC_EncodeableObject_PfnClear* clearFct)
{
    size_t idx = 0;
    size_t pos = 0;
    SOPC_Byte* byteArray = NULL;
    if (noOfElts != NULL && *noOfElts >= 0 && eltsArray != NULL)
    {
        byteArray = *eltsArray;
        if (byteArray != NULL)
        {
            for (idx = 0; idx < (size_t) *noOfElts; idx++)
            {
                pos = idx * sizeOfElt;
                clearFct(&(byteArray[pos]));
            }

            SOPC_Free(*eltsArray);
        }
        *noOfElts = 0;
        *eltsArray = NULL;
    }
}

/**
 * \brief Check if a range is valid on an array.
 * \note:
 * - Arrays always start at index 0
 * - ::SOPC_Dimension objects are always valid (implying that start &tl;= end is already asserted)
 *
 * \param arrayLength The array length
 * \param dimension   A (non-NULL) dimension. Contains the range to check.
 * \param fullRange   When true, the function checks that at least one element of the array is valid for \p dimension
 *                    (typical for a "Read" array check).
 *                    When  false, the function checks that all elements of the array are valid for \p dimension
 *                    (typical for a "Write" array check).
 *
 * \return true if the range is valid, false otherwise.
 */
static inline bool is_array_valid_range(int32_t arrayLength, const SOPC_Dimension* dimension, bool fullRange)
{
    SOPC_ASSERT(NULL != dimension);
    if (arrayLength <= 0)
    {
        return false;
    }
    const uint32_t uLen = (uint32_t) arrayLength;

    return dimension->start < uLen && (!fullRange || dimension->end < uLen);
}

static bool has_range_array(const SOPC_Variant* variant, const SOPC_NumericRange* range, bool fullRange)
{
    SOPC_ASSERT(range->n_dimensions == 1);

    if (variant->ArrayType == SOPC_VariantArrayType_SingleValue)
    {
        // Dereferencing scalars is allowed for strings and bytestrings
        if (variant->BuiltInTypeId == SOPC_String_Id)
        {
            return is_array_valid_range(variant->Value.String.Length, &range->dimensions[0], fullRange);
        }
        else if (variant->BuiltInTypeId == SOPC_ByteString_Id)
        {
            return is_array_valid_range(variant->Value.Bstring.Length, &range->dimensions[0], fullRange);
        }
    }

    if (variant->ArrayType != SOPC_VariantArrayType_Array)
    {
        return false;
    }

    if (variant->Value.Array.Length <= 0)
    {
        return false;
    }
    /* Note: we might also detect static range invalid cases but we need the ArrayDimension attribute of Variable node.
     *       if (ArrayDimension[0] != 0 && range->dimensions[0].start >= ArrayDimension[0]) return false
     */

    return is_array_valid_range(variant->Value.Array.Length, &range->dimensions[0], fullRange);
}

static bool has_range_matrix(const SOPC_Variant* variant, const SOPC_NumericRange* range, bool fullRange)
{
    SOPC_ASSERT(range->n_dimensions > 1);
    if (range->n_dimensions > INT32_MAX)
    {
        return false;
    }

    bool has_range = true;
    int32_t n_dimensions = (int32_t) range->n_dimensions;

    if (variant->ArrayType == SOPC_VariantArrayType_Array && 2 == range->n_dimensions)
    {
        SOPC_String* strArray = NULL;
        // Dereferencing 2-dimension array is allowed for strings and bytestrings
        if (variant->BuiltInTypeId == SOPC_String_Id)
        {
            strArray = variant->Value.Array.Content.StringArr;
        }
        else if (variant->BuiltInTypeId == SOPC_ByteString_Id)
        {
            strArray = variant->Value.Array.Content.BstringArr;
        }
        else
        {
            return false;
        }
        // First range dimension limits the strings concerned in the string array
        has_range = is_array_valid_range(variant->Value.Array.Length, &range->dimensions[0], false);
        for (uint32_t i = range->dimensions[0].start;
             i <= range->dimensions[0].end && i < (uint32_t) variant->Value.Array.Length && has_range; i++)
        {
            // Second range dimension limits the part of the string concerned
            has_range &= is_array_valid_range(strArray[i].Length, &range->dimensions[1], fullRange);
        }
    }
    else
    {
        if (variant->ArrayType != SOPC_VariantArrayType_Matrix)
        {
            return false;
        }

        if (variant->Value.Matrix.Dimensions != n_dimensions)
        {
            return false;
        }

        for (size_t i = 0; i < range->n_dimensions && has_range; i++)
        {
            has_range &=
                is_array_valid_range(variant->Value.Matrix.ArrayDimensions[i], &range->dimensions[i], fullRange);
        }
    }
    /* Note: we might also detect static range invalid cases but we need the ArrayDimension attribute of Variable node.
     *       if (ArrayDimension[i] != 0 && range->dimensions[i].start >= ArrayDimension[i]) return false
     */

    return has_range;
}

SOPC_ReturnStatus SOPC_Variant_HasRange(const SOPC_Variant* variant,
                                        const SOPC_NumericRange* range,
                                        bool fullRange,
                                        bool* hasRange)
{
    if (NULL == variant || NULL == range || NULL == hasRange)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    switch (range->n_dimensions)
    {
    case 0:
        return SOPC_STATUS_INVALID_PARAMETERS;
    case 1:
        *hasRange = has_range_array(variant, range, fullRange);
        return SOPC_STATUS_OK;
    default:
        *hasRange = has_range_matrix(variant, range, fullRange);
        return SOPC_STATUS_OK;
    }
}

/**
 * A flattened range defines a range [start;end] in flattened 1-dimension array
 */
typedef struct _SOPC_FlattenedRange
{
    uint32_t start; // Inclusive
    uint32_t end;   // Inclusive
} SOPC_FlattenedRange;

/**
 * Flattened ranges provides <N> ranges in the flattened 1-dimension array.
 * It is the flattened representation in a 1-dimension array of the ::SOPC_NumericRange in a matrix.
 */
typedef struct _SOPC_FlattenedRanges
{
    size_t n_ranges;
    SOPC_FlattenedRange* ranges;
} SOPC_FlattenedRanges;

static inline uint32_t SOPC_MIN_INDEX(uint32_t left, uint32_t right)
{
    return (left > right ? right : left);
}

// Common treatment for slicing both strings and bytestrings
static SOPC_ReturnStatus get_range_string_helper(SOPC_String* dst,
                                                 const SOPC_String* src,
                                                 const SOPC_Dimension* dimension)
{
    SOPC_ASSERT(src->Length >= 0);
    SOPC_String_Initialize(dst);

    const uint32_t src_length = (uint32_t) src->Length;
    const uint32_t start = dimension->start;

    if (start >= src_length)
    {
        // Nothing to copy
        dst->Length = 0;
        return SOPC_STATUS_OK;
    }

    const uint32_t end = SOPC_MIN_INDEX(dimension->end, src_length - 1);
    SOPC_ASSERT(end >= start);

    const uint32_t dst_len = end - start + 1;

    dst->Data = SOPC_Calloc(1 + dst_len, sizeof(SOPC_Byte));

    if (NULL == dst->Data)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    memcpy(dst->Data, src->Data + start, (size_t) dst_len);
    dst->Length = (int32_t) dst_len;

    return SOPC_STATUS_OK;
}

static SOPC_ReturnStatus get_range_string(SOPC_Variant* dst, const SOPC_String* src, const SOPC_Dimension* dimension)
{
    dst->ArrayType = SOPC_VariantArrayType_SingleValue;
    dst->BuiltInTypeId = SOPC_String_Id;
    dst->DoNotClear = false;

    return get_range_string_helper(&dst->Value.String, src, dimension);
}

static SOPC_ReturnStatus get_range_bytestring(SOPC_Variant* dst,
                                              const SOPC_String* src,
                                              const SOPC_Dimension* dimension)
{
    dst->ArrayType = SOPC_VariantArrayType_SingleValue;
    dst->BuiltInTypeId = SOPC_ByteString_Id;
    dst->DoNotClear = false;

    return get_range_string_helper(&dst->Value.Bstring, src, dimension);
}

static size_t size_of_builtin_type(SOPC_BuiltinId builtInTypeId)
{
    if (0 <= builtInTypeId && builtInTypeId <= SOPC_BUILTINID_MAX)
    {
        return SOPC_BuiltInType_HandlingTable[builtInTypeId].size;
    }

    SOPC_ASSERT(false);
    return 0;
}

static SOPC_ReturnStatus get_range_array(SOPC_Variant* dst, const SOPC_Variant* src, const SOPC_NumericRange* range)
{
    SOPC_ASSERT(range->n_dimensions == 1);

    if (src->ArrayType == SOPC_VariantArrayType_SingleValue)
    {
        // Dereferencing scalars is allowed for strings and bytestrings
        if (src->BuiltInTypeId == SOPC_String_Id)
        {
            return get_range_string(dst, &src->Value.String, &range->dimensions[0]);
        }
        else if (src->BuiltInTypeId == SOPC_ByteString_Id)
        {
            return get_range_bytestring(dst, &src->Value.Bstring, &range->dimensions[0]);
        }
    }

    if (src->ArrayType != SOPC_VariantArrayType_Array)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    const SOPC_Dimension* dim = &range->dimensions[0];
    SOPC_ASSERT(src->Value.Array.Length >= 0);
    const uint32_t start = dim->start;
    const uint32_t array_length = (uint32_t) src->Value.Array.Length;

    dst->BuiltInTypeId = src->BuiltInTypeId;
    dst->DoNotClear = false;
    dst->Value.Array.Length = 0;

    if (start >= array_length)
    {
        // Nothing to copy
        return SOPC_STATUS_OK;
    }

    const uint32_t end = SOPC_MIN_INDEX(dim->end, array_length - 1);
    SOPC_ASSERT(end >= start);

    const uint32_t dst_len = end - start + 1;

    SOPC_EncodeableObject_PfnCopy* copyFunction = GetBuiltInTypeCopyFunction(src->BuiltInTypeId);

    if (NULL == copyFunction)
    {
        return SOPC_STATUS_NOK;
    }

    const size_t type_size = size_of_builtin_type(src->BuiltInTypeId);

    // Untyped pointer to the source array data at the correct offset
    const uint8_t* src_data = *((const uint8_t* const*) &src->Value.Array.Content) + start * type_size;

    SOPC_ReturnStatus status =
        AllocVariantArrayBuiltInType(src->BuiltInTypeId, &dst->Value.Array.Content, (int32_t) dst_len);

    if (status != SOPC_STATUS_OK)
    {
        return status;
    }

    const uint8_t* src_i = src_data;
    uint8_t* dst_i = *((uint8_t**) &dst->Value.Array.Content);

    for (uint32_t i = 0; i < dst_len; ++i)
    {
        status = copyFunction(dst_i, src_i);

        if (status != SOPC_STATUS_OK)
        {
            return status;
        }

        src_i += type_size;
        dst_i += type_size;

        // Update array length so that SOPC_Variant_Clear clears the copied
        // items in case of failure
        dst->Value.Array.Length = (int32_t)(i + 1);
    }

    dst->ArrayType = SOPC_VariantArrayType_Array;

    return SOPC_STATUS_OK;
}

static SOPC_ReturnStatus get_range_matrix_on_string_array(SOPC_Variant* dst,
                                                          const SOPC_Variant* src,
                                                          const SOPC_NumericRange* range)
{
    SOPC_ASSERT(src->ArrayType == SOPC_VariantArrayType_Array);
    SOPC_ASSERT(2 == range->n_dimensions);

    if (range->dimensions[0].start >= (uint32_t) src->Value.Array.Length)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    const uint32_t array_end_index = (range->dimensions[0].end < (uint32_t) src->Value.Array.Length)
                                         ? range->dimensions[0].end
                                         : (uint32_t) src->Value.Array.Length - 1;
    const uint32_t array_length = array_end_index - range->dimensions[0].start + 1;

    SOPC_String* srcStrArray = NULL;
    SOPC_String* dstStrArray = SOPC_Calloc(array_length, sizeof(*dstStrArray));
    if (NULL == dstStrArray)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    dst->ArrayType = SOPC_VariantArrayType_Array;
    dst->BuiltInTypeId = src->BuiltInTypeId;
    dst->DoNotClear = false;
    dst->Value.Array.Length = (int32_t) array_length;
    if (src->BuiltInTypeId == SOPC_String_Id)
    {
        srcStrArray = src->Value.Array.Content.StringArr;
        dst->Value.Array.Content.StringArr = dstStrArray;
    }
    else if (src->BuiltInTypeId == SOPC_ByteString_Id)
    {
        srcStrArray = src->Value.Array.Content.BstringArr;
        dst->Value.Array.Content.BstringArr = dstStrArray;
    }
    else
    {
        SOPC_Free(dstStrArray);
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus grStatus = SOPC_STATUS_OK;
    // First range dimension limit the strings concerned in the string array
    for (uint32_t i = 0; i < array_length && SOPC_STATUS_OK == grStatus; i++)
    {
        // Second range dimension limit the part of the string concerned
        grStatus = get_range_string_helper(&dstStrArray[i], &srcStrArray[range->dimensions[0].start + i],
                                           &range->dimensions[1]);
    }

    return grStatus;
}

// Compute the flattened Ranges to apply on flattened representation
static SOPC_ReturnStatus flatten_matrix_numeric_ranges(const SOPC_Variant* variant,
                                                       const SOPC_NumericRange* numRanges,
                                                       SOPC_FlattenedRanges* flatRanges)
{
    SOPC_ASSERT(SOPC_VariantArrayType_Matrix == variant->ArrayType);
    SOPC_ASSERT(variant->Value.Matrix.Dimensions > 0);
    SOPC_ASSERT(numRanges->n_dimensions == (size_t) variant->Value.Matrix.Dimensions);

    // Number of ranges to set on the flattened representation
    size_t n_ranges = 1;
    // Number of elements for an index of each dimension
    uint32_t* numberOfElementsPerDimensionIndex =
        SOPC_Calloc((size_t) variant->Value.Matrix.Dimensions, sizeof(*numberOfElementsPerDimensionIndex));
    if (NULL == numberOfElementsPerDimensionIndex)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    for (int64_t i = (int64_t) numRanges->n_dimensions - 1; i >= 0; i--)
    {
        const SOPC_Dimension* dim = &numRanges->dimensions[i];
        const uint32_t start_in_dim = dim->start;
        const uint32_t end_in_dim = dim->end;
        SOPC_ASSERT(end_in_dim >= start_in_dim);
        const uint32_t elts_in_range = end_in_dim - start_in_dim + 1;

        /* Note: multi-dimensional arrays are encoded as a one-dimensional array,
         * Higher rank dimensions are serialized first.
         * For example, an array with dimensions [3,2] is written in this order:
         * [0,0], [0,1], [0,2], [1,0], [1,1]
         */
        // Numbers of elements included in a change of only last dimension index is only one
        // For other dimensions it is the product of the next dimension: length and number of elements per index
        // numberOfElementsPerDimensionIndex(dim_<N_MAX>) = 1
        // numberOfElementsPerDimensionIndex(dim_<n-1>) = Length of dim_<n> * numberOfElementsPerDimensionIndex(dim_<n>)
        if ((size_t) i < numRanges->n_dimensions - 1)
        {
            SOPC_ASSERT(variant->Value.Matrix.ArrayDimensions[i] > 0);
            numberOfElementsPerDimensionIndex[i] =
                numberOfElementsPerDimensionIndex[i + 1] * (uint32_t) variant->Value.Matrix.ArrayDimensions[i + 1];

            if (SIZE_MAX / n_ranges > elts_in_range)
            {
                // Number of ranges is multiplied by the number of elements in current range
                n_ranges *= elts_in_range;
            }
            else
            {
                SOPC_Free(numberOfElementsPerDimensionIndex);
                return SOPC_STATUS_OUT_OF_MEMORY;
            }
        }
        else
        {
            // Last dimension is serialized first: 1 element per index in this dimension
            numberOfElementsPerDimensionIndex[i] = 1;

            // Last dimension range will remain a range because flattened indexes remain consecutive (n_range *= 1)
        }
    }

    SOPC_FlattenedRanges result_flat_index_ranges = {
        .n_ranges = n_ranges, .ranges = SOPC_Calloc(n_ranges, sizeof(*result_flat_index_ranges.ranges))};

    size_t previous_number_of_flat_indexes = 0;
    uint32_t* previous_flat_indexes = SOPC_Calloc(n_ranges, sizeof(*previous_flat_indexes));
    size_t next_number_of_flat_indexes = 0;
    uint32_t* next_flat_indexes = SOPC_Calloc(n_ranges, sizeof(*next_flat_indexes));

    if (NULL == result_flat_index_ranges.ranges || NULL == previous_flat_indexes || NULL == next_flat_indexes)
    {
        SOPC_Free(numberOfElementsPerDimensionIndex);
        SOPC_Free(result_flat_index_ranges.ranges);
        SOPC_Free(previous_flat_indexes);
        SOPC_Free(next_flat_indexes);
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    // Start with flattened index value 0 for each ranges of each dimension
    // previous_flat_indexes => content is already 0
    // Set number to 1 to obtain number of range elements in first iteration
    previous_number_of_flat_indexes = 1;

    // For each dimension except last dimension
    for (size_t i = 0; i < numRanges->n_dimensions - 1; i++)
    {
        const SOPC_Dimension* dim = &numRanges->dimensions[i];
        const uint32_t start_in_dim = dim->start;
        const uint32_t end_in_dim = dim->end;
        const uint32_t elts_in_range = end_in_dim - start_in_dim + 1;

        if (i < numRanges->n_dimensions - 1)
        {
            next_number_of_flat_indexes = elts_in_range * previous_number_of_flat_indexes;

            size_t next_i = 0;
            // Iterate on current dimension range and compute partially flattened indexes
            for (uint32_t i_in_dim = start_in_dim; i_in_dim <= end_in_dim; i_in_dim++)
            {
                const uint32_t numberOfElementForDimensionIndex = i_in_dim * numberOfElementsPerDimensionIndex[i];
                // Transform previous flattened ranges with current dimension range
                for (size_t j = 0; j < previous_number_of_flat_indexes; j++)
                {
                    // Flattened ranges have unique index until last dimension (ignore end until then)
                    next_flat_indexes[next_i] = previous_flat_indexes[j] + numberOfElementForDimensionIndex;
                    next_i++;
                }
            }
            SOPC_ASSERT(next_i == next_number_of_flat_indexes);
        }

        // Exchange previous with next flattened indexes for next iteration
        uint32_t* tmp_indexes = previous_flat_indexes;
        previous_flat_indexes = next_flat_indexes;
        next_flat_indexes = tmp_indexes;
        previous_number_of_flat_indexes = next_number_of_flat_indexes;
    }
    // Last dimension treatment
    {
        // Last dimension range will remain a range because flattened indexes remain consecutive (n_range *= 1)
        SOPC_ASSERT(previous_number_of_flat_indexes == result_flat_index_ranges.n_ranges);

        // Keep last dimension ranges and compute final flattened ranges
        const SOPC_Dimension* dim = &numRanges->dimensions[numRanges->n_dimensions - 1];
        const uint32_t start_in_dim = dim->start;
        const uint32_t end_in_dim = dim->end;

        // Iterate on current dimension range and compute partially flattened indexes
        for (uint32_t i_in_dim = start_in_dim; i_in_dim <= end_in_dim; i_in_dim++)
        {
            // Transform previous flattened ranges with current dimension range
            for (size_t j = 0; j < previous_number_of_flat_indexes; j++)
            {
                result_flat_index_ranges.ranges[j].start = previous_flat_indexes[j] + start_in_dim;
                result_flat_index_ranges.ranges[j].end = previous_flat_indexes[j] + end_in_dim;
            }
        }
    }
    SOPC_Free(numberOfElementsPerDimensionIndex);
    SOPC_Free(previous_flat_indexes);
    SOPC_Free(next_flat_indexes);
    *flatRanges = result_flat_index_ranges;

    return SOPC_STATUS_OK;
}

static SOPC_ReturnStatus get_range_matrix(SOPC_Variant* dst, const SOPC_Variant* src, const SOPC_NumericRange* range)
{
    /*
     * Destination matrix: new matrix to get the specified ranges: dimensions lengths will be length of intersection
     *                                                             of range and source matrix data
     * Source matrix: data source for the specified ranges to read, dimensions lengths might differ from ranges
     * lengths
     */
    SOPC_ASSERT(range->n_dimensions > 1);
    if (range->n_dimensions > INT32_MAX)
    {
        return false;
    }

    const int32_t n_dimensions = (int32_t) range->n_dimensions;

    if (src->ArrayType == SOPC_VariantArrayType_Array && 2 == range->n_dimensions)
    {
        return get_range_matrix_on_string_array(dst, src, range);
    }

    if (src->ArrayType != SOPC_VariantArrayType_Matrix)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_EncodeableObject_PfnCopy* copyFunction = GetBuiltInTypeCopyFunction(src->BuiltInTypeId);
    const size_t type_size = size_of_builtin_type(src->BuiltInTypeId);

    dst->ArrayType = SOPC_VariantArrayType_Matrix;
    dst->BuiltInTypeId = src->BuiltInTypeId;
    dst->DoNotClear = false;
    dst->Value.Matrix.Dimensions = n_dimensions;
    dst->Value.Matrix.ArrayDimensions = SOPC_Calloc(range->n_dimensions, sizeof(*dst->Value.Matrix.ArrayDimensions));
    if (NULL == dst->Value.Matrix.ArrayDimensions)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    SOPC_NumericRange truncatedRange = {.n_dimensions = range->n_dimensions,
                                        .dimensions = SOPC_Calloc(range->n_dimensions, sizeof(*range->dimensions))};
    if (NULL == truncatedRange.dimensions)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    /* Compute total length of destination array and truncated dimension ranges:
     * limited by each dimension range and source array length */
    uint32_t total_dst_len = 1;
    for (size_t i = 0; i < range->n_dimensions && SOPC_STATUS_OK == status; i++)
    {
        const SOPC_Dimension* dim = &range->dimensions[i];
        SOPC_ASSERT(src->Value.Array.Length >= 0);
        const int32_t array_length = src->Value.Matrix.ArrayDimensions[i];
        if (!is_array_valid_range(array_length, dim, false))
        {
            /* Start index shall be valid in the source variant matrix */
            status = SOPC_STATUS_INVALID_PARAMETERS;
            break;
        }
        SOPC_Dimension* truncatedDim = &truncatedRange.dimensions[i];
        truncatedDim->start = dim->start;
        // Truncate range end if origin end index is not in the source variant matrix
        truncatedDim->end = SOPC_MIN_INDEX(dim->end, (uint32_t) array_length - 1);
        const uint32_t length_in_dim = truncatedDim->end - truncatedDim->start + 1;
        if (length_in_dim > INT32_MAX)
        {
            status = SOPC_STATUS_INVALID_PARAMETERS;
            break;
        }
        dst->Value.Matrix.ArrayDimensions[i] = (int32_t) length_in_dim;
        total_dst_len *= length_in_dim;
    }
    if (SOPC_STATUS_OK == status && total_dst_len > INT32_MAX)
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Free(truncatedRange.dimensions);
        return status;
    }

    // Allocate array (flattened matrix)
    status = AllocVariantArrayBuiltInType(dst->BuiltInTypeId, &dst->Value.Matrix.Content, (int32_t) total_dst_len);

    if (SOPC_STATUS_OK != status)
    {
        return status;
    }

    SOPC_Initialize_Array((int32_t) total_dst_len, *((void**) &dst->Value.Matrix.Content),
                          SOPC_BuiltInType_HandlingTable[dst->BuiltInTypeId].size,
                          SOPC_BuiltInType_HandlingTable[dst->BuiltInTypeId].initialize);

    // Compute corresponding flattened ranges that shall be applied to flattened source array
    SOPC_FlattenedRanges franges = {.n_ranges = 0, .ranges = NULL};
    status = flatten_matrix_numeric_ranges(src, &truncatedRange, &franges);
    SOPC_Free(truncatedRange.dimensions);
    memset(&truncatedRange, 0, sizeof(truncatedRange));
    if (SOPC_STATUS_OK != status)
    {
        return status;
    }

    // Untyped pointer to the source array data at the correct offset
    const uint8_t* const src_array_origin = *((const uint8_t* const*) &src->Value.Matrix.Content);
    uint8_t* dst_array_index_k = *((uint8_t**) &dst->Value.Matrix.Content);

    // Now we just have to do copies between the two flattened arrays using flattened ranges
    for (uint32_t i = 0; i < franges.n_ranges && SOPC_STATUS_OK == status; i++)
    {
        const SOPC_FlattenedRange* frange = &franges.ranges[i];
        for (uint32_t j = frange->start; j <= frange->end && SOPC_STATUS_OK == status; j++)
        {
            // j represents the index in the source flattened array
            const uint8_t* src_array_index_j = src_array_origin + j * type_size;
            status = copyFunction(dst_array_index_k, src_array_index_j);

            // We always increment by 1 for next element in destination flattened array
            dst_array_index_k += type_size;
        }
    }

    if (SOPC_STATUS_OK != status)
    {
        // Clear destination variant
        SOPC_Variant_Clear(dst);
    }
    SOPC_Free(franges.ranges);
    return status;
}

SOPC_ReturnStatus SOPC_Variant_GetRange(SOPC_Variant* dst, const SOPC_Variant* src, const SOPC_NumericRange* range)
{
    switch (range->n_dimensions)
    {
    case 0:
        return SOPC_STATUS_INVALID_PARAMETERS;
    case 1:
        return get_range_array(dst, src, range);
    default:
        // Matrix will come later
        return get_range_matrix(dst, src, range);
    }
}

static SOPC_ReturnStatus set_range_string(SOPC_String* dst, const SOPC_String* src, const SOPC_Dimension* dimension)
{
    const uint32_t start = dimension->start;
    const uint32_t end = dimension->end;
    SOPC_ASSERT(end >= start);

    if (((uint32_t) src->Length) != (end - start + 1))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (!is_array_valid_range(dst->Length, dimension, true))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    const size_t range_len = (size_t)(end - start + 1);
    memcpy(dst->Data + ((size_t) start), src->Data, range_len * sizeof(SOPC_Byte));

    return SOPC_STATUS_OK;
}

static SOPC_ReturnStatus set_range_array(SOPC_Variant* dst, const SOPC_Variant* src, const SOPC_NumericRange* range)
{
    SOPC_ASSERT(dst->BuiltInTypeId == src->BuiltInTypeId);
    SOPC_ASSERT(range->n_dimensions == 1);

    if (src->ArrayType == SOPC_VariantArrayType_SingleValue)
    {
        // Dereferencing scalars is allowed for strings and bytestrings
        if (src->BuiltInTypeId == SOPC_String_Id)
        {
            return set_range_string(&dst->Value.String, &src->Value.String, &range->dimensions[0]);
        }
        else if (src->BuiltInTypeId == SOPC_ByteString_Id)
        {
            return set_range_string(&dst->Value.Bstring, &src->Value.Bstring, &range->dimensions[0]);
        }
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (src->ArrayType != SOPC_VariantArrayType_Array)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    const uint32_t start = range->dimensions[0].start;
    const uint32_t end = range->dimensions[0].end;
    SOPC_ASSERT(end >= start);

    if (((uint32_t) src->Value.Array.Length) != (end - start + 1))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (!is_array_valid_range(dst->Value.Array.Length, &range->dimensions[0], true))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_EncodeableObject_PfnCopy* copyFunction = GetBuiltInTypeCopyFunction(src->BuiltInTypeId);
    SOPC_EncodeableObject_PfnClear* clearFunction = GetBuiltInTypeClearFunction(src->BuiltInTypeId);

    if (NULL == copyFunction || NULL == clearFunction)
    {
        return SOPC_STATUS_NOK;
    }

    const size_t type_size = size_of_builtin_type(src->BuiltInTypeId);

    /* Note: fix to ensure we will free the variant content at the end.
     * If the variant was a static definition it has DoNotClear flag
     * but since we will do partial modification (to be cleared) on it we need to change that.
     * Make the variant "clearable" by copying itself before the partial modification.
     */
    if (dst->DoNotClear)
    {
        SOPC_Variant tmp;
        SOPC_Variant_Initialize(&tmp);
        SOPC_ReturnStatus status = SOPC_Variant_Copy(&tmp, dst);
        if (status != SOPC_STATUS_OK)
        {
            return status;
        }
        *dst = tmp;
    }

    // Untyped pointer to the source array data at the correct offset
    const uint8_t* src_i = *((const uint8_t* const*) &src->Value.Array.Content);
    uint8_t* dst_i = *((uint8_t**) &dst->Value.Array.Content) + start * type_size;

    for (uint32_t i = 0; i < (end - start + 1); ++i)
    {
        clearFunction(dst_i);
        SOPC_ReturnStatus status = copyFunction(dst_i, src_i);

        if (status != SOPC_STATUS_OK)
        {
            return status;
        }

        src_i += type_size;
        dst_i += type_size;
    }

    return SOPC_STATUS_OK;
}

static SOPC_ReturnStatus set_range_matrix_on_string_array(SOPC_Variant* dst,
                                                          const SOPC_Variant* src,
                                                          const SOPC_NumericRange* range)
{
    SOPC_ASSERT(dst->ArrayType == SOPC_VariantArrayType_Array);
    SOPC_ASSERT(2 == range->n_dimensions);

    if (!is_array_valid_range(dst->Value.Array.Length, &range->dimensions[0], true))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_String* strArray = NULL;
    if (dst->BuiltInTypeId == SOPC_String_Id)
    {
        strArray = dst->Value.Array.Content.StringArr;
    }
    else if (dst->BuiltInTypeId == SOPC_ByteString_Id)
    {
        strArray = dst->Value.Array.Content.BstringArr;
    }
    else
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    const uint32_t array_end_index = range->dimensions[0].end;
    const uint32_t array_length = array_end_index - range->dimensions[0].start + 1;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    // First range dimension limit the strings concerned in the string array
    for (uint32_t i = 0; i < array_length && SOPC_STATUS_OK == status; i++)
    {
        // Second range dimension limit the part of the string concerned
        status = set_range_string(&strArray[range->dimensions[0].start + i], &src->Value.Array.Content.StringArr[i],
                                  &range->dimensions[1]);
    }

    return status;
}

static SOPC_ReturnStatus set_range_matrix(SOPC_Variant* dst,
                                          const SOPC_Variant* src,
                                          const SOPC_NumericRange* numRanges)
{
    /*
     * Destination matrix: existing matrix to be modify: dimensions lengths might differ from ranges lengths
     * Source matrix: data source for the specified ranges to write, dimensions lengths shall match ranges lengths
     */

    SOPC_ASSERT(dst->BuiltInTypeId == src->BuiltInTypeId);
    SOPC_ASSERT(numRanges->n_dimensions > 1);
    if (numRanges->n_dimensions > INT32_MAX)
    {
        return false;
    }

    if (src->ArrayType == SOPC_VariantArrayType_Array && 2 == numRanges->n_dimensions)
    {
        return set_range_matrix_on_string_array(dst, src, numRanges);
    }

    if (src->ArrayType != SOPC_VariantArrayType_Matrix)
    {
        return SOPC_STATUS_NOK;
    }

    SOPC_EncodeableObject_PfnCopy* copyFunction = GetBuiltInTypeCopyFunction(src->BuiltInTypeId);
    SOPC_EncodeableObject_PfnClear* clearFunction = GetBuiltInTypeClearFunction(src->BuiltInTypeId);

    if (NULL == copyFunction || NULL == clearFunction)
    {
        return SOPC_STATUS_NOK;
    }

    const size_t type_size = size_of_builtin_type(src->BuiltInTypeId);

    /* Note: fix to ensure we will free the variant content at the end.
     * If the variant was a static definition it has DoNotClear flag
     * but since we will do partial modification (to be cleared) on it we need to change that.
     * Make the variant "clearable" by copying itself before the partial modification.
     */
    if (dst->DoNotClear)
    {
        SOPC_Variant tmp;
        SOPC_Variant_Initialize(&tmp);
        SOPC_ReturnStatus status = SOPC_Variant_Copy(&tmp, dst);
        if (status != SOPC_STATUS_OK)
        {
            return status;
        }
        *dst = tmp;
    }

    /* Check constraints */
    for (size_t i = 0; i < numRanges->n_dimensions; i++)
    {
        const SOPC_Dimension* dim = &numRanges->dimensions[i];
        const uint32_t start_in_dim = dim->start;
        const uint32_t end_in_dim = dim->end;
        SOPC_ASSERT(end_in_dim >= start_in_dim);

        /* Source array dimension shall match the range length (values to write in dimension) */
        if (((uint32_t) src->Value.Matrix.ArrayDimensions[i]) != (end_in_dim - start_in_dim + 1))
        {
            return SOPC_STATUS_INVALID_PARAMETERS;
        }

        if (!is_array_valid_range(dst->Value.Matrix.ArrayDimensions[i], dim, true))
        {
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
    }

    /* Compute corresponding flattened ranges that shall be applied to flattened destination array */
    SOPC_FlattenedRanges franges = {.n_ranges = 0, .ranges = NULL};
    SOPC_ReturnStatus status = flatten_matrix_numeric_ranges(dst, numRanges, &franges);
    if (SOPC_STATUS_OK != status)
    {
        return status;
    }

    // Untyped pointer to the source array data at the correct offset
    const uint8_t* src_array_index_j = *((const uint8_t* const*) &src->Value.Matrix.Content);
    uint8_t* const dst_array_origin = *((uint8_t**) &dst->Value.Matrix.Content);

    // Now we just have to do copies between the two flattened arrays using flattened ranges
    for (uint32_t i = 0; i < franges.n_ranges && SOPC_STATUS_OK == status; i++)
    {
        const SOPC_FlattenedRange* frange = &franges.ranges[i];
        for (uint32_t j = frange->start; j <= frange->end && SOPC_STATUS_OK == status; j++)
        {
            // j represents the index in the destination flattened array
            uint8_t* dst_array_index_j = dst_array_origin + j * type_size;
            clearFunction(dst_array_index_j);
            status = copyFunction(dst_array_index_j, src_array_index_j);

            // We always increment by 1 for next element in source flattened array
            src_array_index_j += type_size;
        }
    }

    SOPC_Free(franges.ranges);
    return status;
}

SOPC_ReturnStatus SOPC_Variant_SetRange(SOPC_Variant* dst, const SOPC_Variant* src, const SOPC_NumericRange* range)
{
    if (dst->BuiltInTypeId != src->BuiltInTypeId)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    switch (range->n_dimensions)
    {
    case 0:
        return SOPC_STATUS_INVALID_PARAMETERS;
    case 1:
        return set_range_array(dst, src, range);
    default:
        return set_range_matrix(dst, src, range);
    }
}

const void* SOPC_Variant_Get_SingleValue(const SOPC_Variant* var, SOPC_BuiltinId builtInTypeId)
{
    SOPC_ASSERT(SOPC_VariantArrayType_SingleValue == var->ArrayType);
    SOPC_ASSERT(builtInTypeId == var->BuiltInTypeId);

    switch (builtInTypeId)
    {
    case SOPC_Null_Id:
        return NULL;
    case SOPC_Boolean_Id:
    case SOPC_SByte_Id:
    case SOPC_Byte_Id:
    case SOPC_Int16_Id:
    case SOPC_UInt16_Id:
    case SOPC_Int32_Id:
    case SOPC_UInt32_Id:
    case SOPC_Int64_Id:
    case SOPC_UInt64_Id:
    case SOPC_Float_Id:
    case SOPC_Double_Id:
    case SOPC_String_Id:
    case SOPC_DateTime_Id:
    case SOPC_ByteString_Id:
    case SOPC_XmlElement_Id:
    case SOPC_StatusCode_Id:
        // Note union fields are aligned: provide pointer to data
        // (i.e. first first address for those types)
        return (const void*) &var->Value.Boolean;
    case SOPC_Guid_Id:
    case SOPC_NodeId_Id:
    case SOPC_ExpandedNodeId_Id:
    case SOPC_QualifiedName_Id:
    case SOPC_LocalizedText_Id:
    case SOPC_ExtensionObject_Id:
    case SOPC_DataValue_Id:
    case SOPC_DiagnosticInfo_Id:
        // Note union fields content are aligned: provide pointer to data
        // (i.e. content of the first field for those types)
        return (const void*) var->Value.Guid;
    case SOPC_Variant_Id:
        // Part 6 Table 14 (v1.03): "The value shall not be a Variant
        //                           but it could be an array of Variants."
        // Note: Variant is not encoded in S2OPC stack for this case
        return NULL;
    default:
        SOPC_ASSERT(false);
        return NULL;
    }
}

const void* SOPC_Variant_Get_ArrayValue(const SOPC_Variant* var, SOPC_BuiltinId builtInTypeId, int32_t index)
{
    SOPC_ASSERT(SOPC_VariantArrayType_Array == var->ArrayType);
    SOPC_ASSERT(builtInTypeId == var->BuiltInTypeId);
    SOPC_ASSERT(var->Value.Array.Length > index);

    if (index < 0 || (uint64_t) index > (uint64_t) SIZE_MAX)
    {
        return NULL;
    }

    switch (builtInTypeId)
    {
    case SOPC_Null_Id:
        // mantis #0003682: errata for 1.03 but not confirmed NULL array forbidden
        return NULL;
    case SOPC_Boolean_Id:
    case SOPC_SByte_Id:
    case SOPC_Byte_Id:
    case SOPC_Int16_Id:
    case SOPC_UInt16_Id:
    case SOPC_Int32_Id:
    case SOPC_UInt32_Id:
    case SOPC_Int64_Id:
    case SOPC_UInt64_Id:
    case SOPC_Float_Id:
    case SOPC_Double_Id:
    case SOPC_String_Id:
    case SOPC_DateTime_Id:
    case SOPC_Guid_Id:
    case SOPC_ByteString_Id:
    case SOPC_XmlElement_Id:
    case SOPC_NodeId_Id:
    case SOPC_ExpandedNodeId_Id:
    case SOPC_StatusCode_Id:
    case SOPC_QualifiedName_Id:
    case SOPC_LocalizedText_Id:
    case SOPC_ExtensionObject_Id:
    case SOPC_DataValue_Id:
    case SOPC_Variant_Id:
    case SOPC_DiagnosticInfo_Id:
        return (char*) var->Value.Array.Content.BooleanArr +
               SOPC_BuiltInType_HandlingTable[builtInTypeId].size * (size_t) index;
    default:
        return NULL;
    }
}

bool SOPC_Variant_CopyInto_ArrayValueAt(const SOPC_Variant* var,
                                        SOPC_BuiltinId builtInTypeId,
                                        int32_t index,
                                        const void* value)
{
    SOPC_ASSERT(SOPC_VariantArrayType_Array == var->ArrayType);
    SOPC_ASSERT(builtInTypeId == var->BuiltInTypeId && SOPC_Null_Id != builtInTypeId);
    SOPC_ASSERT(var->Value.Array.Length > index);
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    if (index < 0 || (uint64_t) index > (uint64_t) SIZE_MAX)
    {
        return false;
    }

    SOPC_EncodeableObject_PfnCopy* copyFct = SOPC_BuiltInType_HandlingTable[builtInTypeId].copy;
    switch (builtInTypeId)
    {
    case SOPC_Boolean_Id:
    case SOPC_SByte_Id:
    case SOPC_Byte_Id:
    case SOPC_Int16_Id:
    case SOPC_UInt16_Id:
    case SOPC_Int32_Id:
    case SOPC_UInt32_Id:
    case SOPC_Int64_Id:
    case SOPC_UInt64_Id:
    case SOPC_Float_Id:
    case SOPC_Double_Id:
    case SOPC_String_Id:
    case SOPC_DateTime_Id:
    case SOPC_Guid_Id:
    case SOPC_ByteString_Id:
    case SOPC_XmlElement_Id:
    case SOPC_NodeId_Id:
    case SOPC_ExpandedNodeId_Id:
    case SOPC_StatusCode_Id:
    case SOPC_QualifiedName_Id:
    case SOPC_LocalizedText_Id:
    case SOPC_ExtensionObject_Id:
    case SOPC_DataValue_Id:
    case SOPC_Variant_Id:
    case SOPC_DiagnosticInfo_Id:
        status = copyFct((char*) var->Value.Array.Content.BooleanArr +
                             SOPC_BuiltInType_HandlingTable[builtInTypeId].size * (size_t) index,
                         value);
        break;
    default:
        SOPC_ASSERT(false);
        break;
    }

    if (SOPC_STATUS_OK == status)
    {
        return true;
    }
    return false;
}

const SOPC_NodeId* SOPC_Variant_Get_DataType(const SOPC_Variant* var)
{
    switch (var->BuiltInTypeId)
    {
    case SOPC_Null_Id:
    case SOPC_Boolean_Id:
    case SOPC_SByte_Id:
    case SOPC_Byte_Id:
    case SOPC_Int16_Id:
    case SOPC_UInt16_Id:
    case SOPC_Int32_Id:
    case SOPC_UInt32_Id:
    case SOPC_Int64_Id:
    case SOPC_UInt64_Id:
    case SOPC_Float_Id:
    case SOPC_Double_Id:
    case SOPC_String_Id:
    case SOPC_DateTime_Id:
    case SOPC_Guid_Id:
    case SOPC_ByteString_Id:
    case SOPC_XmlElement_Id:
    case SOPC_NodeId_Id:
    case SOPC_ExpandedNodeId_Id:
    case SOPC_StatusCode_Id:
    case SOPC_QualifiedName_Id:
    case SOPC_LocalizedText_Id:
    case SOPC_DiagnosticInfo_Id:
    case SOPC_DataValue_Id:
    case SOPC_Variant_Id: /* Corresponds to BaseDataType: could only be an array of Variant */
        return SOPC_BuiltInTypeId_To_DataTypeNodeId[var->BuiltInTypeId];
    case SOPC_ExtensionObject_Id:

        if (var->ArrayType == SOPC_VariantArrayType_SingleValue && var->Value.ExtObject->TypeId.ServerIndex == 0 &&
            var->Value.ExtObject->TypeId.NamespaceUri.Length <= 0)
        {
            if (var->Value.ExtObject->Encoding == SOPC_ExtObjBodyEncoding_Object &&
                NULL != var->Value.ExtObject->Body.Object.ObjType)
            {
                SOPC_ASSERT(NULL == var->Value.ExtObject->Body.Object.ObjType->NamespaceUri &&
                            "EncType Namespace URI translation unsupported");
                // Restore the DataType type id if it was the encoding object node
                var->Value.ExtObject->TypeId.NodeId.Namespace =
                    var->Value.ExtObject->Body.Object.ObjType->NamespaceIndex;
                var->Value.ExtObject->TypeId.NodeId.Data.Numeric = var->Value.ExtObject->Body.Object.ObjType->TypeId;
                return &var->Value.ExtObject->TypeId.NodeId;
            }
            else
            {
                // TODO / Note: if the type is unknown we cannot guarantee here the NodeId is a DataType, since it
                // could be the DefaultEncoding Object instead. Returns the generic Structure type instead
                return &SOPC_Structure_Type;
            }
        }
        else
        {
            /* If type defined in another server or variant is an array, no guarantee that all are of same type.
             * Keep "Structure" generic type. */
            return &SOPC_Structure_Type;
        }
    default:
        SOPC_ASSERT(false); // Invalid type
        return &SOPC_Null_Type;
    }
}

int32_t SOPC_Variant_Get_ValueRank(const SOPC_Variant* var)
{
    switch (var->ArrayType)
    {
    case SOPC_VariantArrayType_SingleValue:
        return -1; // Scalar
    case SOPC_VariantArrayType_Array:
        return 1; // One dimension array
    case SOPC_VariantArrayType_Matrix:
        return var->Value.Matrix.Dimensions;
    default:
        SOPC_ASSERT(false); // Invalid value
        return -3;
    }
}

const SOPC_BuiltInType_Handling SOPC_BuiltInType_HandlingTable[SOPC_BUILTINID_MAX + 1] = {
    {
        0,                   // size
        NULL,                // initialize
        SOPC_Null_ClearAux,  // clear
        SOPC_Null_CopyAux,   // copy
        SOPC_Null_CompareAux // compare
    },
    {
        sizeof(SOPC_Boolean),       // size
        SOPC_Boolean_InitializeAux, // initialize
        SOPC_Boolean_ClearAux,      // clear
        SOPC_Boolean_CopyAux,       // copy
        SOPC_Boolean_CompareAux     // compare
    },
    {
        sizeof(SOPC_SByte),       // size
        SOPC_SByte_InitializeAux, // initialize
        SOPC_SByte_ClearAux,      // clear
        SOPC_SByte_CopyAux,       // copy
        SOPC_SByte_CompareAux     // compare
    },
    {
        sizeof(SOPC_Byte),       // size
        SOPC_Byte_InitializeAux, // initialize
        SOPC_Byte_ClearAux,      // clear
        SOPC_Byte_CopyAux,       // copy
        SOPC_Byte_CompareAux     // compare
    },
    {
        sizeof(int16_t),          // size
        SOPC_Int16_InitializeAux, // initialize
        SOPC_Int16_ClearAux,      // clear
        SOPC_Int16_CopyAux,       // copy
        SOPC_Int16_CompareAux     // compare
    },
    {
        sizeof(uint16_t),          // size
        SOPC_UInt16_InitializeAux, // initialize
        SOPC_UInt16_ClearAux,      // clear
        SOPC_UInt16_CopyAux,       // copy
        SOPC_UInt16_CompareAux     // compare
    },
    {
        sizeof(int32_t),          // size
        SOPC_Int32_InitializeAux, // initialize
        SOPC_Int32_ClearAux,      // clear
        SOPC_Int32_CopyAux,       // copy
        SOPC_Int32_CompareAux     // compare
    },
    {
        sizeof(uint32_t),          // size
        SOPC_UInt32_InitializeAux, // initialize
        SOPC_UInt32_ClearAux,      // clear
        SOPC_UInt32_CopyAux,       // copy
        SOPC_UInt32_CompareAux     // compare
    },
    {
        sizeof(int64_t),          // size
        SOPC_Int64_InitializeAux, // initialize
        SOPC_Int64_ClearAux,      // clear
        SOPC_Int64_CopyAux,       // copy
        SOPC_Int64_CompareAux     // compare
    },
    {
        sizeof(uint64_t),          // size
        SOPC_UInt64_InitializeAux, // initialize
        SOPC_UInt64_ClearAux,      // clear
        SOPC_UInt64_CopyAux,       // copy
        SOPC_UInt64_CompareAux     // compare
    },
    {
        sizeof(float),            // size
        SOPC_Float_InitializeAux, // initialize
        SOPC_Float_ClearAux,      // clear
        SOPC_Float_CopyAux,       // copy
        SOPC_Float_CompareAux     // compare
    },
    {
        sizeof(double),            // size
        SOPC_Double_InitializeAux, // initialize
        SOPC_Double_ClearAux,      // clear
        SOPC_Double_CopyAux,       // copy
        SOPC_Double_CompareAux     // compare
    },
    {
        sizeof(SOPC_String),       // size
        SOPC_String_InitializeAux, // initialize
        SOPC_String_ClearAux,      // clear
        SOPC_String_CopyAux,       // copy
        SOPC_String_CompareAux     // compare
    },
    {
        sizeof(SOPC_DateTime),       // size
        SOPC_DateTime_InitializeAux, // initialize
        SOPC_DateTime_ClearAux,      // clear
        SOPC_DateTime_CopyAux,       // copy
        SOPC_DateTime_CompareAux     // compare
    },
    {
        sizeof(SOPC_Guid),       // size
        SOPC_Guid_InitializeAux, // initialize
        SOPC_Guid_ClearAux,      // clear
        SOPC_Guid_CopyAux,       // copy
        SOPC_Guid_CompareAux     // compare
    },
    {
        sizeof(SOPC_ByteString),       // size
        SOPC_ByteString_InitializeAux, // initialize
        SOPC_ByteString_ClearAux,      // clear
        SOPC_ByteString_CopyAux,       // copy
        SOPC_ByteString_CompareAux     // compare
    },
    {
        sizeof(SOPC_XmlElement),       // size
        SOPC_XmlElement_InitializeAux, // initialize
        SOPC_XmlElement_ClearAux,      // clear
        SOPC_XmlElement_CopyAux,       // copy
        SOPC_XmlElement_CompareAux     // compare
    },
    {
        sizeof(SOPC_NodeId),       // size
        SOPC_NodeId_InitializeAux, // initialize
        SOPC_NodeId_ClearAux,      // clear
        SOPC_NodeId_CopyAux,       // copy
        SOPC_NodeId_CompareAux     // compare
    },
    {
        sizeof(SOPC_ExpandedNodeId),       // size
        SOPC_ExpandedNodeId_InitializeAux, // initialize
        SOPC_ExpandedNodeId_ClearAux,      // clear
        SOPC_ExpandedNodeId_CopyAux,       // copy
        SOPC_ExpandedNodeId_CompareAux     // compare
    },
    {
        sizeof(SOPC_StatusCode),       // size
        SOPC_StatusCode_InitializeAux, // initialize
        SOPC_StatusCode_ClearAux,      // clear
        SOPC_StatusCode_CopyAux,       // copy
        SOPC_StatusCode_CompareAux     // compare
    },
    {
        sizeof(SOPC_QualifiedName),       // size
        SOPC_QualifiedName_InitializeAux, // initialize
        SOPC_QualifiedName_ClearAux,      // clear
        SOPC_QualifiedName_CopyAux,       // copy
        SOPC_QualifiedName_CompareAux     // compare
    },
    {
        sizeof(SOPC_LocalizedText),       // size
        SOPC_LocalizedText_InitializeAux, // initialize
        SOPC_LocalizedText_ClearAux,      // clear
        SOPC_LocalizedText_CopyAux,       // copy
        SOPC_LocalizedText_CompareAux     // compare
    },
    {
        sizeof(SOPC_ExtensionObject),       // size
        SOPC_ExtensionObject_InitializeAux, // initialize
        SOPC_ExtensionObject_ClearAux,      // clear
        SOPC_ExtensionObject_CopyAux,       // copy
        SOPC_ExtensionObject_CompareAux     // compare
    },
    {
        sizeof(SOPC_DataValue),       // size
        SOPC_DataValue_InitializeAux, // initialize
        SOPC_DataValue_ClearAux,      // clear
        SOPC_DataValue_CopyAux,       // copy
        SOPC_DataValue_CompareAux     // compare
    },
    {
        sizeof(SOPC_Variant),       // size
        SOPC_Variant_InitializeAux, // initialize
        SOPC_Variant_ClearAux,      // clear
        SOPC_Variant_CopyAux,       // copy
        SOPC_Variant_CompareAux     // compare
    },
    {
        sizeof(SOPC_DiagnosticInfo),       // size
        SOPC_DiagnosticInfo_InitializeAux, // initialize
        SOPC_DiagnosticInfo_ClearAux,      // clear
        SOPC_DiagnosticInfo_CopyAux,       // copy
        SOPC_DiagnosticInfo_CompareAux     // compare
    }};

bool SOPC_ValueRank_IsAssignableInto(int32_t dest_ValueRank, int32_t src_valueRank)
{
    SOPC_ASSERT(dest_ValueRank > -4);
    SOPC_ASSERT(src_valueRank > -4);
    if (dest_ValueRank == src_valueRank)
    {
        // Covers compatible abstract value rank (but src always concrete when computed from variant value)
        return true;
    }

    switch (dest_ValueRank)
    {
    case -3:
        // ScalarOrOneDimension (−3): src can be scalar (-1) or one dimension array (1)
        return (src_valueRank == -1 || src_valueRank == 1);
    case -2:
        // Any (−2): src can be have any value rank
        return true;
    case -1:
        // Scalar (−1): src can be scalar (-1) only
        return src_valueRank == -1;
    case 0:
        // OneOrMoreDimensions (0): src can have <n> dimensions (1..n)
        return src_valueRank > 0;
    default:
        return false; // value rank equality already tested prior to switch/case, otherwise false
    }
}
