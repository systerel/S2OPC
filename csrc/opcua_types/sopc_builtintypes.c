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

#include <assert.h>
#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sopc_hash.h"
#include "sopc_helper_string.h"
#include "sopc_macros.h"

#include "opcua_identifiers.h"
#include "sopc_toolkit_config_constants.h"

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

SOPC_ByteString* SOPC_ByteString_Create()
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
            bstring->Data = (SOPC_Byte*) malloc(sizeof(SOPC_Byte) * (size_t) size);
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
    if (dest != NULL && bytes != NULL && length > 0)
    {
        dest->Length = length;
        if (length > 0 && (uint64_t) length * sizeof(SOPC_Byte) <= SIZE_MAX)
        {
            dest->Data = malloc(sizeof(SOPC_Byte) * (size_t) length);
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
                dest->Data = malloc(sizeof(SOPC_Byte) * (size_t) dest->Length);
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

SOPC_String* SOPC_String_Create()
{
    SOPC_String* string = NULL;
    string = (SOPC_String*) malloc(sizeof(SOPC_String));
    if (NULL != string)
    {
        SOPC_String_Initialize(string);
    }
    return string;
}

SOPC_ReturnStatus SOPC_String_AttachFrom(SOPC_String* dest, SOPC_String* src)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (dest != NULL && src != NULL && src->Length > 0 && src->Data != NULL)
    {
        status = SOPC_STATUS_OK;
        dest->Length = src->Length;
        dest->Data = src->Data;
        dest->DoNotClear = 1; // dest->characters will not be freed on clear
    }
    return status;
}

SOPC_ReturnStatus SOPC_String_AttachFromCstring(SOPC_String* dest, char* src)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    size_t stringLength = 0;
    if (dest != NULL && NULL == dest->Data && src != NULL)
    {
        assert(CHAR_BIT == 8);
        stringLength = strlen(src);
        if (stringLength <= INT32_MAX)
        {
            status = SOPC_STATUS_OK;
            dest->Length = (int32_t) stringLength;
            dest->Data = (uint8_t*) src;
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

                dest->Data = (SOPC_Byte*) malloc(sizeof(SOPC_Byte) * (size_t) dest->Length + 1);
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
                free(string->Data);
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
        free(string);
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
            string->Data = (SOPC_Byte*) malloc(sizeof(SOPC_Byte) * (stringLength + 1));
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
        cString = (char*) malloc(sizeof(char) * ((size_t) string->Length + 1));
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
    char* cString = NULL;
    if (string != NULL && string->Length > 0)
    {
        if (CHAR_BIT == 8)
        {
            cString = (char*) string->Data;
            assert(string->Data[string->Length] == '\0');
        }
        else
        {
            assert(false);
        }
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
        assert(CHAR_BIT == 8);
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
    if (guid == NULL || str == NULL)
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
            dest->Data.Guid = malloc(sizeof(SOPC_Guid));
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
                free(nodeId->Data.Guid);
            }
            nodeId->Data.Guid = NULL;
            break;
        case SOPC_IdentifierType_ByteString:
            SOPC_ByteString_Clear(&nodeId->Data.Bstring);
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
                    assert(false);
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

void SOPC_NodeId_Hash(const SOPC_NodeId* nodeId, uint64_t* hash)
{
    uint64_t h;

    assert(nodeId != NULL);

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
        assert(false && "Unknown IdentifierType");
    }

    *hash = h;
}

char* SOPC_NodeId_ToCString(SOPC_NodeId* nodeId)
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
        result = calloc(maxSize, sizeof(char));
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
                    break;
                case SOPC_IdentifierType_Guid:
                    // ex: "g=09087e75-8e5e-499b-954f-f2a9603db28a\0"
                    if (nodeId->Data.Guid != NULL)
                    {
                        res = sprintf(&result[res], "g=%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                                      nodeId->Data.Guid->Data1, nodeId->Data.Guid->Data2, nodeId->Data.Guid->Data3,
                                      nodeId->Data.Guid->Data4[0], nodeId->Data.Guid->Data4[1],
                                      nodeId->Data.Guid->Data4[2], nodeId->Data.Guid->Data4[3],
                                      nodeId->Data.Guid->Data4[4], nodeId->Data.Guid->Data4[5],
                                      nodeId->Data.Guid->Data4[6], nodeId->Data.Guid->Data4[7]);
                    }
                    else
                    {
                        res = sprintf(&result[res], "g=");
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
                    }
                    break;
                default:
                    break;
                }
            }
            else
            {
                free(result);
                result = NULL;
            }
        }
    }
    return result;
}

SOPC_NodeId* SOPC_NodeId_FromCString(const char* cString, int32_t len)
{
    /* Creates a new NodeId (and guid/string) */
    SOPC_NodeId* pNid = NULL;
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
        sz = (char*) calloc((size_t) len + 1, sizeof(char));
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
                pGuid = (SOPC_Guid*) malloc(sizeof(SOPC_Guid));
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

    /* Now that it is valid and we now where to find the string/guid/bstring, create the NodeId */
    if (SOPC_STATUS_OK == status)
    {
        pNid = (SOPC_NodeId*) malloc(sizeof(SOPC_NodeId));
        SOPC_NodeId_Initialize(pNid);
    }

    if (NULL != pNid)
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
            pNid->Data.Guid = pGuid;

            status = SOPC_Guid_FromCString(pGuid, p, (size_t)(len - (p - sz)));

            /* May be failed, but pGuid is allocated */
            if (SOPC_STATUS_OK != status)
            {
                free(pGuid);
                pGuid = NULL;
                status = SOPC_STATUS_NOK;
            }
            break;
        case SOPC_IdentifierType_ByteString:
            SOPC_ByteString_Initialize(&pNid->Data.Bstring);
            status = SOPC_ByteString_CopyFromBytes(&pNid->Data.Bstring, (SOPC_Byte*) p, len - (int32_t)(p - sz));
            break;
        default:
            break;
        }

        /* Something could have failed but the NodeId is already allocated */
        if (SOPC_STATUS_OK != status)
        {
            free(pNid);
            pNid = NULL;
        }
    }
    else
    {
        free(pGuid);
    }

    free(sz);

    return pNid;
}

static uint64_t nodeid_hash(const void* id)
{
    uint64_t hash;
    SOPC_NodeId_Hash((const SOPC_NodeId*) id, &hash);
    return hash;
}

static bool nodeid_equal(const void* a, const void* b)
{
    int32_t cmp = 0;

    SOPC_ReturnStatus status = SOPC_NodeId_Compare((const SOPC_NodeId*) a, (const SOPC_NodeId*) b, &cmp);
    assert(status == SOPC_STATUS_OK);

    return cmp == 0;
}

static void nodeid_free(void* id)
{
    if (id != NULL)
    {
        SOPC_NodeId_Clear(id);
        free(id);
    }
}

SOPC_Dict* SOPC_NodeId_Dict_Create(bool free_keys, SOPC_Dict_Free_Fct value_free)
{
    return SOPC_Dict_Create(NULL, nodeid_hash, nodeid_equal, free_keys ? nodeid_free : NULL, value_free);
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
        *status = SOPC_STATUS_OK;
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
        *status = SOPC_STATUS_OK;
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
        status = SOPC_DiagnosticInfo_Copy(dest->InnerDiagnosticInfo, src->InnerDiagnosticInfo);
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
            free(diagInfo->InnerDiagnosticInfo);
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

    if (qname == NULL || str == NULL)
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (status == SOPC_STATUS_OK)
    {
        char* colon = strchr(str, ':');
        qname->NamespaceIndex = 0;

        if (colon == NULL)
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
        SOPC_String_Initialize(&localizedText->Locale);
        SOPC_String_Initialize(&localizedText->Text);
    }
}

SOPC_ReturnStatus SOPC_LocalizedText_Copy(SOPC_LocalizedText* dest, const SOPC_LocalizedText* src)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != dest && NULL != src)
    {
        status = SOPC_String_Copy(&dest->Locale, &src->Locale);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_String_Copy(&dest->Text, &src->Text);
        }
        if (SOPC_STATUS_OK != status)
        {
            SOPC_LocalizedText_Clear(dest);
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_LocalizedText_Compare(const SOPC_LocalizedText* left,
                                             const SOPC_LocalizedText* right,
                                             int32_t* comparison)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != left && NULL != right)
    {
        status = SOPC_String_Compare(&left->Locale, &right->Locale, false, comparison);
        if (SOPC_STATUS_OK == status && *comparison == 0)
        {
            status = SOPC_String_Compare(&left->Text, &right->Text, false, comparison);
        }
    }
    return status;
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

void SOPC_LocalizedText_Clear(SOPC_LocalizedText* localizedText)
{
    if (localizedText != NULL)
    {
        SOPC_String_Clear(&localizedText->Locale);
        SOPC_String_Clear(&localizedText->Text);
    }
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
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_Buffer* encodedObject = NULL;
    SOPC_ExtObjectBodyEncoding encoding = SOPC_ExtObjBodyEncoding_None;

    if (NULL == dest || NULL == src)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    switch (src->Encoding)
    {
    case SOPC_ExtObjBodyEncoding_None:
        break;
    case SOPC_ExtObjBodyEncoding_ByteString:
        status = SOPC_ByteString_Copy(&dest->Body.Bstring, &src->Body.Bstring);
        encoding = SOPC_ExtObjBodyEncoding_ByteString;
        break;
    case SOPC_ExtObjBodyEncoding_XMLElement:
        status = SOPC_XmlElement_Copy(&dest->Body.Xml, &src->Body.Xml);
        encoding = SOPC_ExtObjBodyEncoding_XMLElement;
        break;
    case SOPC_ExtObjBodyEncoding_Object:
        if (NULL != src->Body.Object.ObjType && NULL != src->Body.Object.Value)
        {
            /* We do not have the copy method for the object but we can encode it */
            encoding = SOPC_ExtObjBodyEncoding_ByteString;
            encodedObject = SOPC_Buffer_Create(SOPC_MAX_STRING_LENGTH); /* String content + length */
            if (NULL == encodedObject)
            {
                status = SOPC_STATUS_OUT_OF_MEMORY;
            }

            if (SOPC_STATUS_OK == status)
            {
                status = src->Body.Object.ObjType->Encode(src->Body.Object.Value, encodedObject);
            }
            if (SOPC_STATUS_OK == status)
            {
                SOPC_ByteString_Initialize(&dest->Body.Bstring);
                assert(SOPC_MAX_STRING_LENGTH + 4 <= INT32_MAX); // Ensure conversion to int32_t is valid
                // Do a copy to keep only used data in buffer
                status = SOPC_ByteString_CopyFromBytes(&dest->Body.Bstring, encodedObject->data,
                                                       (int32_t) encodedObject->length);
                SOPC_Buffer_Delete(encodedObject);
                encodedObject = NULL;
            }
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ExpandedNodeId_Copy(&dest->TypeId, &src->TypeId);

        /* Since we have encoded Object into ByteString for copy, ensure the type NodeId is correct */
        if (SOPC_STATUS_OK == status && src->Encoding == SOPC_ExtObjBodyEncoding_Object)
        {
            dest->TypeId.NodeId.IdentifierType = SOPC_IdentifierType_Numeric;
            dest->TypeId.NodeId.Namespace = OPCUA_NAMESPACE_INDEX;
            dest->TypeId.NodeId.Data.Numeric = src->Body.Object.ObjType->BinaryEncodingTypeId;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        dest->Encoding = encoding;
        dest->Length = src->Length;
    }
    else
    {
        SOPC_ExtensionObject_Clear(dest);
    }

    return status;
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
                // TODO: compare encoded values or add a compare function in encodeable types
                status = SOPC_STATUS_NOT_SUPPORTED;
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
            extObj->Body.Object.ObjType->Clear(extObj->Body.Object.Value);
            free(extObj->Body.Object.Value);
            extObj->Body.Object.Value = NULL;
            break;
        }
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
        clearAuxFunction((void*) &val->Boolean);
        break;
    case SOPC_SByte_Id:
        clearAuxFunction((void*) &val->Sbyte);
        break;
    case SOPC_Byte_Id:
        clearAuxFunction((void*) &val->Byte);
        break;
    case SOPC_Int16_Id:
        clearAuxFunction((void*) &val->Int16);
        break;
    case SOPC_UInt16_Id:
        clearAuxFunction((void*) &val->Uint16);
        break;
    case SOPC_Int32_Id:
        clearAuxFunction((void*) &val->Int32);
        break;
    case SOPC_UInt32_Id:
        clearAuxFunction((void*) &val->Uint32);
        break;
    case SOPC_Int64_Id:
        clearAuxFunction((void*) &val->Int64);
        break;
    case SOPC_UInt64_Id:
        clearAuxFunction((void*) &val->Uint64);
        break;
    case SOPC_Float_Id:
        clearAuxFunction((void*) &val->Floatv);
        break;
    case SOPC_Double_Id:
        clearAuxFunction((void*) &val->Doublev);
        break;
    case SOPC_String_Id:
        clearAuxFunction((void*) &val->String);
        break;
    case SOPC_DateTime_Id:
        clearAuxFunction((void*) &val->Date);
        break;
    case SOPC_Guid_Id:
        clearAuxFunction((void*) val->Guid);
        break;
    case SOPC_ByteString_Id:
        clearAuxFunction((void*) &val->Bstring);
        break;
    case SOPC_XmlElement_Id:
        clearAuxFunction((void*) &val->XmlElt);
        break;
    case SOPC_NodeId_Id:
        clearAuxFunction((void*) val->NodeId);
        break;
    case SOPC_ExpandedNodeId_Id:
        clearAuxFunction((void*) val->ExpNodeId);
        break;
    case SOPC_StatusCode_Id:
        clearAuxFunction((void*) &val->Status);
        break;
    case SOPC_QualifiedName_Id:
        clearAuxFunction((void*) val->Qname);
        break;
    case SOPC_LocalizedText_Id:
        clearAuxFunction((void*) val->LocalizedText);
        break;
    case SOPC_ExtensionObject_Id:
        clearAuxFunction((void*) val->ExtObject);
        break;
    case SOPC_DataValue_Id:
        clearAuxFunction((void*) val->DataValue);
        break;
    case SOPC_DiagnosticInfo_Id:
        clearAuxFunction((void*) val->DiagInfo);
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
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
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
            array->BooleanArr = calloc(size, sizeof(SOPC_Boolean));
            if (NULL != array->BooleanArr)
                return SOPC_STATUS_OK;
            break;
        case SOPC_SByte_Id:
            array->SbyteArr = calloc(size, sizeof(SOPC_SByte));
            if (NULL != array->SbyteArr)
                return SOPC_STATUS_OK;
            break;
        case SOPC_Byte_Id:
            array->ByteArr = calloc(size, sizeof(SOPC_Byte));
            if (NULL != array->ByteArr)
                return SOPC_STATUS_OK;
            break;
        case SOPC_Int16_Id:
            array->Int16Arr = calloc(size, sizeof(int16_t));
            if (NULL != array->Int16Arr)
                return SOPC_STATUS_OK;
            break;
        case SOPC_UInt16_Id:
            array->Uint16Arr = calloc(size, sizeof(uint16_t));
            if (NULL != array->Uint16Arr)
                return SOPC_STATUS_OK;
            break;
        case SOPC_Int32_Id:
            array->Int32Arr = calloc(size, sizeof(int32_t));
            if (NULL != array->Int32Arr)
                return SOPC_STATUS_OK;
            break;
        case SOPC_UInt32_Id:
            array->Uint32Arr = calloc(size, sizeof(uint32_t));
            if (NULL != array->Uint32Arr)
                return SOPC_STATUS_OK;
            break;
        case SOPC_Int64_Id:
            array->Int64Arr = calloc(size, sizeof(int64_t));
            if (NULL != array->Int64Arr)
                return SOPC_STATUS_OK;
            break;
        case SOPC_UInt64_Id:
            array->Uint64Arr = calloc(size, sizeof(uint64_t));
            if (NULL != array->Uint64Arr)
                return SOPC_STATUS_OK;
            break;
        case SOPC_Float_Id:
            array->FloatvArr = calloc(size, sizeof(float));
            if (NULL != array->FloatvArr)
                return SOPC_STATUS_OK;
            break;
        case SOPC_Double_Id:
            array->DoublevArr = calloc(size, sizeof(double));
            if (NULL != array->DoublevArr)
                return SOPC_STATUS_OK;
            break;
        case SOPC_String_Id:
            array->StringArr = calloc(size, sizeof(SOPC_String));
            if (NULL != array->StringArr)
                return SOPC_STATUS_OK;
            break;
        case SOPC_DateTime_Id:
            array->DateArr = calloc(size, sizeof(SOPC_DateTime));
            if (NULL != array->DateArr)
                return SOPC_STATUS_OK;
            break;
        case SOPC_Guid_Id:
            array->GuidArr = calloc(size, sizeof(SOPC_Guid));
            if (NULL != array->GuidArr)
                return SOPC_STATUS_OK;
            break;
        case SOPC_ByteString_Id:
            array->BstringArr = calloc(size, sizeof(SOPC_ByteString));
            if (NULL != array->BstringArr)
                return SOPC_STATUS_OK;
            break;
        case SOPC_XmlElement_Id:
            array->XmlEltArr = calloc(size, sizeof(SOPC_XmlElement));
            if (NULL != array->XmlEltArr)
                return SOPC_STATUS_OK;
            break;
        case SOPC_NodeId_Id:
            array->NodeIdArr = calloc(size, sizeof(SOPC_NodeId));
            if (NULL != array->NodeIdArr)
                return SOPC_STATUS_OK;
            break;
        case SOPC_ExpandedNodeId_Id:
            array->ExpNodeIdArr = calloc(size, sizeof(SOPC_ExpandedNodeId));
            if (NULL != array->ExpNodeIdArr)
                return SOPC_STATUS_OK;
            break;
        case SOPC_StatusCode_Id:
            array->StatusArr = calloc(size, sizeof(SOPC_StatusCode));
            if (NULL != array->StatusArr)
                return SOPC_STATUS_OK;
            break;
        case SOPC_QualifiedName_Id:
            array->QnameArr = calloc(size, sizeof(SOPC_QualifiedName));
            if (NULL != array->QnameArr)
                return SOPC_STATUS_OK;
            break;
        case SOPC_LocalizedText_Id:
            array->LocalizedTextArr = calloc(size, sizeof(SOPC_LocalizedText));
            if (NULL != array->LocalizedTextArr)
                return SOPC_STATUS_OK;
            break;
        case SOPC_ExtensionObject_Id:
            array->ExtObjectArr = calloc(size, sizeof(SOPC_ExtensionObject));
            if (NULL != array->ExtObjectArr)
                return SOPC_STATUS_OK;
            break;
        case SOPC_DataValue_Id:
            array->DataValueArr = calloc(size, sizeof(SOPC_DataValue));
            if (NULL != array->DataValueArr)
                return SOPC_STATUS_OK;
            break;
        case SOPC_Variant_Id:
            array->VariantArr = calloc(size, sizeof(SOPC_Variant));
            if (NULL != array->VariantArr)
                return SOPC_STATUS_OK;
            break;
        case SOPC_DiagnosticInfo_Id:
            array->DiagInfoArr = calloc(size, sizeof(SOPC_DiagnosticInfo));
            if (NULL != array->DiagInfoArr)
                return SOPC_STATUS_OK;
            break;
        default:
            break;
        }
    }
    return status;
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
        SOPC_Clear_Array(length, (void**) &array->BooleanArr, sizeof(SOPC_Boolean), clearAuxFunction);
        break;
    case SOPC_SByte_Id:
        SOPC_Clear_Array(length, (void**) &array->SbyteArr, sizeof(SOPC_SByte), clearAuxFunction);
        break;
    case SOPC_Byte_Id:
        SOPC_Clear_Array(length, (void**) &array->ByteArr, sizeof(SOPC_Byte), clearAuxFunction);
        break;
    case SOPC_Int16_Id:
        SOPC_Clear_Array(length, (void**) &array->Int16Arr, sizeof(int16_t), clearAuxFunction);
        break;
    case SOPC_UInt16_Id:
        SOPC_Clear_Array(length, (void**) &array->Uint16Arr, sizeof(uint16_t), clearAuxFunction);
        break;
    case SOPC_Int32_Id:
        SOPC_Clear_Array(length, (void**) &array->Int32Arr, sizeof(int32_t), clearAuxFunction);
        break;
    case SOPC_UInt32_Id:
        SOPC_Clear_Array(length, (void**) &array->Uint32Arr, sizeof(uint32_t), clearAuxFunction);
        break;
    case SOPC_Int64_Id:
        SOPC_Clear_Array(length, (void**) &array->Int64Arr, sizeof(int64_t), clearAuxFunction);
        break;
    case SOPC_UInt64_Id:
        SOPC_Clear_Array(length, (void**) &array->Uint64Arr, sizeof(uint64_t), clearAuxFunction);
        break;
    case SOPC_Float_Id:
        SOPC_Clear_Array(length, (void**) &array->FloatvArr, sizeof(float), clearAuxFunction);
        break;
    case SOPC_Double_Id:
        SOPC_Clear_Array(length, (void**) &array->DoublevArr, sizeof(double), clearAuxFunction);
        break;
    case SOPC_String_Id:
        SOPC_Clear_Array(length, (void**) &array->StringArr, sizeof(SOPC_String), clearAuxFunction);
        break;
    case SOPC_DateTime_Id:
        SOPC_Clear_Array(length, (void**) &array->DateArr, sizeof(SOPC_DateTime), clearAuxFunction);
        break;
    case SOPC_Guid_Id:
        SOPC_Clear_Array(length, (void**) &array->GuidArr, sizeof(SOPC_Guid), clearAuxFunction);
        break;
    case SOPC_ByteString_Id:
        SOPC_Clear_Array(length, (void**) &array->BstringArr, sizeof(SOPC_ByteString), clearAuxFunction);
        break;
    case SOPC_XmlElement_Id:
        SOPC_Clear_Array(length, (void**) &array->XmlEltArr, sizeof(SOPC_XmlElement), clearAuxFunction);
        break;
    case SOPC_NodeId_Id:
        SOPC_Clear_Array(length, (void**) &array->NodeIdArr, sizeof(SOPC_NodeId), clearAuxFunction);
        break;
    case SOPC_ExpandedNodeId_Id:
        SOPC_Clear_Array(length, (void**) &array->ExpNodeIdArr, sizeof(SOPC_ExpandedNodeId), clearAuxFunction);
        break;
    case SOPC_StatusCode_Id:
        SOPC_Clear_Array(length, (void**) &array->StatusArr, sizeof(SOPC_StatusCode), clearAuxFunction);
        break;
    case SOPC_QualifiedName_Id:
        SOPC_Clear_Array(length, (void**) &array->QnameArr, sizeof(SOPC_QualifiedName), clearAuxFunction);
        break;
    case SOPC_LocalizedText_Id:
        SOPC_Clear_Array(length, (void**) &array->LocalizedTextArr, sizeof(SOPC_LocalizedText), clearAuxFunction);
        break;
    case SOPC_ExtensionObject_Id:
        SOPC_Clear_Array(length, (void**) &array->ExtObjectArr, sizeof(SOPC_ExtensionObject), clearAuxFunction);
        break;
    case SOPC_DataValue_Id:
        SOPC_Clear_Array(length, (void**) &array->DataValueArr, sizeof(SOPC_DataValue), clearAuxFunction);
        break;
    case SOPC_Variant_Id:
        SOPC_Clear_Array(length, (void**) &array->VariantArr, sizeof(SOPC_Variant), clearAuxFunction);
        break;
    case SOPC_DiagnosticInfo_Id:
        SOPC_Clear_Array(length, (void**) &array->DiagInfoArr, sizeof(SOPC_DiagnosticInfo), clearAuxFunction);
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
        status = opAuxFunction((void*) &left->Boolean, (const void*) &right->Boolean);
        break;
    case SOPC_SByte_Id:
        status = opAuxFunction((void*) &left->Sbyte, (const void*) &right->Sbyte);
        break;
    case SOPC_Byte_Id:
        status = opAuxFunction((void*) &left->Byte, (const void*) &right->Byte);
        break;
    case SOPC_Int16_Id:
        status = opAuxFunction((void*) &left->Int16, (const void*) &right->Int16);
        break;
    case SOPC_UInt16_Id:
        status = opAuxFunction((void*) &left->Uint16, (const void*) &right->Uint16);
        break;
    case SOPC_Int32_Id:
        status = opAuxFunction((void*) &left->Int32, (const void*) &right->Int32);
        break;
    case SOPC_UInt32_Id:
        status = opAuxFunction((void*) &left->Uint32, (const void*) &right->Uint32);
        break;
    case SOPC_Int64_Id:
        status = opAuxFunction((void*) &left->Int64, (const void*) &right->Int64);
        break;
    case SOPC_UInt64_Id:
        status = opAuxFunction((void*) &left->Uint64, (const void*) &right->Uint64);
        break;
    case SOPC_Float_Id:
        status = opAuxFunction((void*) &left->Floatv, (const void*) &right->Floatv);
        break;
    case SOPC_Double_Id:
        status = opAuxFunction((void*) &left->Doublev, (const void*) &right->Doublev);
        break;
    case SOPC_String_Id:
        status = opAuxFunction((void*) &left->String, (const void*) &right->String);
        break;
    case SOPC_DateTime_Id:
        status = opAuxFunction((void*) &left->Date, (const void*) &right->Date);
        break;
    case SOPC_Guid_Id:
        status = opAuxFunction((void*) left->Guid, (void*) right->Guid);
        break;
    case SOPC_ByteString_Id:
        status = opAuxFunction((void*) &left->Bstring, (const void*) &right->Bstring);
        break;
    case SOPC_XmlElement_Id:
        status = opAuxFunction((void*) &left->XmlElt, (const void*) &right->XmlElt);
        break;
    case SOPC_NodeId_Id:
        status = opAuxFunction((void*) left->NodeId, (void*) right->NodeId);
        break;
    case SOPC_ExpandedNodeId_Id:
        status = opAuxFunction((void*) left->ExpNodeId, (void*) right->ExpNodeId);
        break;
    case SOPC_StatusCode_Id:
        status = opAuxFunction((void*) &left->Status, (const void*) &right->Status);
        break;
    case SOPC_QualifiedName_Id:
        status = opAuxFunction((void*) left->Qname, (void*) right->Qname);
        break;
    case SOPC_LocalizedText_Id:
        status = opAuxFunction((void*) left->LocalizedText, (void*) right->LocalizedText);
        break;
    case SOPC_ExtensionObject_Id:
        status = opAuxFunction((void*) left->ExtObject, (void*) right->ExtObject);
        break;
    case SOPC_DataValue_Id:
        status = opAuxFunction((void*) left->DataValue, (void*) right->DataValue);
        break;
    case SOPC_DiagnosticInfo_Id:
        status = opAuxFunction((void*) left->DiagInfo, (void*) right->DiagInfo);
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

static SOPC_ReturnStatus ApplyOpToVariantArrayBuiltInType(SOPC_BuiltinId builtInTypeId,
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
        return SOPC_Op_Array(length, (void*) arrayLeft->BooleanArr, (void*) arrayRight->BooleanArr,
                             sizeof(SOPC_Boolean), opAuxFunction);
    case SOPC_SByte_Id:
        return SOPC_Op_Array(length, (void*) arrayLeft->SbyteArr, (void*) arrayRight->SbyteArr, sizeof(SOPC_SByte),
                             opAuxFunction);
    case SOPC_Byte_Id:
        return SOPC_Op_Array(length, (void*) arrayLeft->ByteArr, (void*) arrayRight->ByteArr, sizeof(SOPC_Byte),
                             opAuxFunction);
    case SOPC_Int16_Id:
        return SOPC_Op_Array(length, (void*) arrayLeft->Int16Arr, (void*) arrayRight->Int16Arr, sizeof(int16_t),
                             opAuxFunction);
    case SOPC_UInt16_Id:
        return SOPC_Op_Array(length, (void*) arrayLeft->Uint16Arr, (void*) arrayRight->Uint16Arr, sizeof(uint16_t),
                             opAuxFunction);
    case SOPC_Int32_Id:
        return SOPC_Op_Array(length, (void*) arrayLeft->Int32Arr, (void*) arrayRight->Int32Arr, sizeof(int32_t),
                             opAuxFunction);
    case SOPC_UInt32_Id:
        return SOPC_Op_Array(length, (void*) arrayLeft->Uint32Arr, (void*) arrayRight->Uint32Arr, sizeof(uint32_t),
                             opAuxFunction);
    case SOPC_Int64_Id:
        return SOPC_Op_Array(length, (void*) arrayLeft->Int64Arr, (void*) arrayRight->Int64Arr, sizeof(int64_t),
                             opAuxFunction);
    case SOPC_UInt64_Id:
        return SOPC_Op_Array(length, (void*) arrayLeft->Uint64Arr, (void*) arrayRight->Uint64Arr, sizeof(uint64_t),
                             opAuxFunction);
    case SOPC_Float_Id:
        return SOPC_Op_Array(length, (void*) arrayLeft->FloatvArr, (void*) arrayRight->FloatvArr, sizeof(float),
                             opAuxFunction);
    case SOPC_Double_Id:
        return SOPC_Op_Array(length, (void*) arrayLeft->DoublevArr, (void*) arrayRight->DoublevArr, sizeof(double),
                             opAuxFunction);
    case SOPC_String_Id:
        return SOPC_Op_Array(length, (void*) arrayLeft->StringArr, (void*) arrayRight->StringArr, sizeof(SOPC_String),
                             opAuxFunction);
    case SOPC_DateTime_Id:
        return SOPC_Op_Array(length, (void*) arrayLeft->DateArr, (void*) arrayRight->DateArr, sizeof(SOPC_DateTime),
                             opAuxFunction);
    case SOPC_Guid_Id:
        return SOPC_Op_Array(length, (void*) arrayLeft->GuidArr, (void*) arrayRight->GuidArr, sizeof(SOPC_Guid),
                             opAuxFunction);
    case SOPC_ByteString_Id:
        return SOPC_Op_Array(length, (void*) arrayLeft->BstringArr, (void*) arrayRight->BstringArr,
                             sizeof(SOPC_ByteString), opAuxFunction);
    case SOPC_XmlElement_Id:
        return SOPC_Op_Array(length, (void*) arrayLeft->XmlEltArr, (void*) arrayRight->XmlEltArr,
                             sizeof(SOPC_XmlElement), opAuxFunction);
    case SOPC_NodeId_Id:
        return SOPC_Op_Array(length, (void*) arrayLeft->NodeIdArr, (void*) arrayRight->NodeIdArr, sizeof(SOPC_NodeId),
                             opAuxFunction);
    case SOPC_ExpandedNodeId_Id:
        return SOPC_Op_Array(length, (void*) arrayLeft->ExpNodeIdArr, (void*) arrayRight->ExpNodeIdArr,
                             sizeof(SOPC_ExpandedNodeId), opAuxFunction);
    case SOPC_StatusCode_Id:
        return SOPC_Op_Array(length, (void*) arrayLeft->StatusArr, (void*) arrayRight->StatusArr,
                             sizeof(SOPC_StatusCode), opAuxFunction);
    case SOPC_QualifiedName_Id:
        return SOPC_Op_Array(length, (void*) arrayLeft->QnameArr, (void*) arrayRight->QnameArr,
                             sizeof(SOPC_QualifiedName), opAuxFunction);
    case SOPC_LocalizedText_Id:
        return SOPC_Op_Array(length, (void*) arrayLeft->LocalizedTextArr, (void*) arrayRight->LocalizedTextArr,
                             sizeof(SOPC_LocalizedText), opAuxFunction);
    case SOPC_ExtensionObject_Id:
        return SOPC_Op_Array(length, (void*) arrayLeft->ExtObjectArr, (void*) arrayRight->ExtObjectArr,
                             sizeof(SOPC_ExtensionObject), opAuxFunction);
    case SOPC_DataValue_Id:
        return SOPC_Op_Array(length, (void*) arrayLeft->DataValueArr, (void*) arrayRight->DataValueArr,
                             sizeof(SOPC_DataValue), opAuxFunction);
    case SOPC_Variant_Id:
        return SOPC_Op_Array(length, (void*) arrayLeft->VariantArr, (void*) arrayRight->VariantArr,
                             sizeof(SOPC_Variant), opAuxFunction);
    case SOPC_DiagnosticInfo_Id:
        return SOPC_Op_Array(length, (void*) arrayLeft->DiagInfoArr, (void*) arrayRight->DiagInfoArr,
                             sizeof(SOPC_DiagnosticInfo), opAuxFunction);
    default:
        break;
    }
    return status;
}

static SOPC_ReturnStatus CompareVariantsNonArrayBuiltInType(SOPC_BuiltinId builtInTypeId,
                                                            const SOPC_VariantValue* left,
                                                            const SOPC_VariantValue* right,
                                                            int32_t* comparison,
                                                            SOPC_EncodeableObject_PfnComp* compAuxFunction)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    switch (builtInTypeId)
    {
    case SOPC_Null_Id:
        break; // SOPC_STATUS_NOK since no operation could be applied to a NULL variant
    case SOPC_Boolean_Id:
        status = compAuxFunction((const void*) &left->Boolean, (const void*) &right->Boolean, comparison);
        break;
    case SOPC_SByte_Id:
        status = compAuxFunction((const void*) &left->Sbyte, (const void*) &right->Sbyte, comparison);
        break;
    case SOPC_Byte_Id:
        status = compAuxFunction((const void*) &left->Byte, (const void*) &right->Byte, comparison);
        break;
    case SOPC_Int16_Id:
        status = compAuxFunction((const void*) &left->Int16, (const void*) &right->Int16, comparison);
        break;
    case SOPC_UInt16_Id:
        status = compAuxFunction((const void*) &left->Uint16, (const void*) &right->Uint16, comparison);
        break;
    case SOPC_Int32_Id:
        status = compAuxFunction((const void*) &left->Int32, (const void*) &right->Int32, comparison);
        break;
    case SOPC_UInt32_Id:
        status = compAuxFunction((const void*) &left->Uint32, (const void*) &right->Uint32, comparison);
        break;
    case SOPC_Int64_Id:
        status = compAuxFunction((const void*) &left->Int64, (const void*) &right->Int64, comparison);
        break;
    case SOPC_UInt64_Id:
        status = compAuxFunction((const void*) &left->Uint64, (const void*) &right->Uint64, comparison);
        break;
    case SOPC_Float_Id:
        status = compAuxFunction((const void*) &left->Floatv, (const void*) &right->Floatv, comparison);
        break;
    case SOPC_Double_Id:
        status = compAuxFunction((const void*) &left->Doublev, (const void*) &right->Doublev, comparison);
        break;
    case SOPC_String_Id:
        status = compAuxFunction((const void*) &left->String, (const void*) &right->String, comparison);
        break;
    case SOPC_DateTime_Id:
        status = compAuxFunction((const void*) &left->Date, (const void*) &right->Date, comparison);
        break;
    case SOPC_Guid_Id:
        status = compAuxFunction((void*) left->Guid, (void*) right->Guid, comparison);
        break;
    case SOPC_ByteString_Id:
        status = compAuxFunction((const void*) &left->Bstring, (const void*) &right->Bstring, comparison);
        break;
    case SOPC_XmlElement_Id:
        status = compAuxFunction((const void*) &left->XmlElt, (const void*) &right->XmlElt, comparison);
        break;
    case SOPC_NodeId_Id:
        status = compAuxFunction((void*) left->NodeId, (const void*) right->NodeId, comparison);
        break;
    case SOPC_ExpandedNodeId_Id:
        status = compAuxFunction((void*) left->ExpNodeId, (const void*) right->ExpNodeId, comparison);
        break;
    case SOPC_StatusCode_Id:
        status = compAuxFunction((const void*) &left->Status, (const void*) &right->Status, comparison);
        break;
    case SOPC_QualifiedName_Id:
        status = compAuxFunction((void*) left->Qname, (void*) right->Qname, comparison);
        break;
    case SOPC_LocalizedText_Id:
        status = compAuxFunction((void*) left->LocalizedText, (void*) right->LocalizedText, comparison);
        break;
    case SOPC_ExtensionObject_Id:
        status = compAuxFunction((void*) left->ExtObject, (void*) right->ExtObject, comparison);
        break;
    case SOPC_DataValue_Id:
        status = compAuxFunction((void*) left->DataValue, (void*) right->DataValue, comparison);
        break;
    case SOPC_DiagnosticInfo_Id:
        status = compAuxFunction((void*) left->DiagInfo, (void*) right->DiagInfo, comparison);
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
                                                        SOPC_EncodeableObject_PfnComp* compAuxFunction,
                                                        int32_t* comparison)
{
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    switch (builtInTypeId)
    {
    case SOPC_Null_Id:
        // mantis #0003682: errata for 1.03 but not confirmed NULL array forbidden
        break; // SOPC_STATUS_NOK since a NULL must not be an array
    case SOPC_Boolean_Id:
        return SOPC_Comp_Array(length, (void*) arrayLeft->BooleanArr, (void*) arrayRight->BooleanArr,
                               sizeof(SOPC_Boolean), compAuxFunction, comparison);
    case SOPC_SByte_Id:
        return SOPC_Comp_Array(length, (void*) arrayLeft->SbyteArr, (void*) arrayRight->SbyteArr, sizeof(SOPC_SByte),
                               compAuxFunction, comparison);
    case SOPC_Byte_Id:
        return SOPC_Comp_Array(length, (void*) arrayLeft->ByteArr, (void*) arrayRight->ByteArr, sizeof(SOPC_Byte),
                               compAuxFunction, comparison);
    case SOPC_Int16_Id:
        return SOPC_Comp_Array(length, (void*) arrayLeft->Int16Arr, (void*) arrayRight->Int16Arr, sizeof(int16_t),
                               compAuxFunction, comparison);
    case SOPC_UInt16_Id:
        return SOPC_Comp_Array(length, (void*) arrayLeft->Uint16Arr, (void*) arrayRight->Uint16Arr, sizeof(uint16_t),
                               compAuxFunction, comparison);
    case SOPC_Int32_Id:
        return SOPC_Comp_Array(length, (void*) arrayLeft->Int32Arr, (void*) arrayRight->Int32Arr, sizeof(int32_t),
                               compAuxFunction, comparison);
    case SOPC_UInt32_Id:
        return SOPC_Comp_Array(length, (void*) arrayLeft->Uint32Arr, (void*) arrayRight->Uint32Arr, sizeof(uint32_t),
                               compAuxFunction, comparison);
    case SOPC_Int64_Id:
        return SOPC_Comp_Array(length, (void*) arrayLeft->Int64Arr, (void*) arrayRight->Int64Arr, sizeof(int64_t),
                               compAuxFunction, comparison);
    case SOPC_UInt64_Id:
        return SOPC_Comp_Array(length, (void*) arrayLeft->Uint64Arr, (void*) arrayRight->Uint64Arr, sizeof(uint64_t),
                               compAuxFunction, comparison);
    case SOPC_Float_Id:
        return SOPC_Comp_Array(length, (void*) arrayLeft->FloatvArr, (void*) arrayRight->FloatvArr, sizeof(float),
                               compAuxFunction, comparison);
    case SOPC_Double_Id:
        return SOPC_Comp_Array(length, (void*) arrayLeft->DoublevArr, (void*) arrayRight->DoublevArr, sizeof(double),
                               compAuxFunction, comparison);
    case SOPC_String_Id:
        return SOPC_Comp_Array(length, (void*) arrayLeft->StringArr, (void*) arrayRight->StringArr, sizeof(SOPC_String),
                               compAuxFunction, comparison);
    case SOPC_DateTime_Id:
        return SOPC_Comp_Array(length, (void*) arrayLeft->DateArr, (void*) arrayRight->DateArr, sizeof(SOPC_DateTime),
                               compAuxFunction, comparison);
    case SOPC_Guid_Id:
        return SOPC_Comp_Array(length, (void*) arrayLeft->GuidArr, (void*) arrayRight->GuidArr, sizeof(SOPC_Guid),
                               compAuxFunction, comparison);
    case SOPC_ByteString_Id:
        return SOPC_Comp_Array(length, (void*) arrayLeft->BstringArr, (void*) arrayRight->BstringArr,
                               sizeof(SOPC_ByteString), compAuxFunction, comparison);
    case SOPC_XmlElement_Id:
        return SOPC_Comp_Array(length, (void*) arrayLeft->XmlEltArr, (void*) arrayRight->XmlEltArr,
                               sizeof(SOPC_XmlElement), compAuxFunction, comparison);
    case SOPC_NodeId_Id:
        return SOPC_Comp_Array(length, (void*) arrayLeft->NodeIdArr, (void*) arrayRight->NodeIdArr, sizeof(SOPC_NodeId),
                               compAuxFunction, comparison);
    case SOPC_ExpandedNodeId_Id:
        return SOPC_Comp_Array(length, (void*) arrayLeft->ExpNodeIdArr, (void*) arrayRight->ExpNodeIdArr,
                               sizeof(SOPC_ExpandedNodeId), compAuxFunction, comparison);
    case SOPC_StatusCode_Id:
        return SOPC_Comp_Array(length, (void*) arrayLeft->StatusArr, (void*) arrayRight->StatusArr,
                               sizeof(SOPC_StatusCode), compAuxFunction, comparison);
    case SOPC_QualifiedName_Id:
        return SOPC_Comp_Array(length, (void*) arrayLeft->QnameArr, (void*) arrayRight->QnameArr,
                               sizeof(SOPC_QualifiedName), compAuxFunction, comparison);
    case SOPC_LocalizedText_Id:
        return SOPC_Comp_Array(length, (void*) arrayLeft->LocalizedTextArr, (void*) arrayRight->LocalizedTextArr,
                               sizeof(SOPC_LocalizedText), compAuxFunction, comparison);
    case SOPC_ExtensionObject_Id:
        return SOPC_Comp_Array(length, (void*) arrayLeft->ExtObjectArr, (void*) arrayRight->ExtObjectArr,
                               sizeof(SOPC_ExtensionObject), compAuxFunction, comparison);
    case SOPC_DataValue_Id:
        return SOPC_Comp_Array(length, (void*) arrayLeft->DataValueArr, (void*) arrayRight->DataValueArr,
                               sizeof(SOPC_DataValue), compAuxFunction, comparison);
    case SOPC_Variant_Id:
        return SOPC_Comp_Array(length, (void*) arrayLeft->VariantArr, (void*) arrayRight->VariantArr,
                               sizeof(SOPC_Variant), compAuxFunction, comparison);
    case SOPC_DiagnosticInfo_Id:
        return SOPC_Comp_Array(length, (void*) arrayLeft->DiagInfoArr, (void*) arrayRight->DiagInfoArr,
                               sizeof(SOPC_DiagnosticInfo), compAuxFunction, comparison);
    default:
        break;
    }
    return status;
}

SOPC_Variant* SOPC_Variant_Create()
{
    SOPC_Variant* variant = calloc(1, sizeof(SOPC_Variant));

    if (variant == NULL)
    {
        return NULL;
    }

    SOPC_Variant_Initialize(variant);
    return variant;
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
    (void) value;
}

static SOPC_EncodeableObject_PfnClear* GetBuiltInTypeClearFunction(SOPC_BuiltinId builtInTypeId)
{
    SOPC_EncodeableObject_PfnClear* clearFunction = NULL;
    switch (builtInTypeId)
    {
    case SOPC_Null_Id:
        clearFunction = SOPC_Null_ClearAux;
        break;
    case SOPC_Boolean_Id:
        clearFunction = SOPC_Boolean_ClearAux;
        break;
    case SOPC_SByte_Id:
        clearFunction = SOPC_SByte_ClearAux;
        break;
    case SOPC_Byte_Id:
        clearFunction = SOPC_Byte_ClearAux;
        break;
    case SOPC_Int16_Id:
        clearFunction = SOPC_Int16_ClearAux;
        break;
    case SOPC_UInt16_Id:
        clearFunction = SOPC_UInt16_ClearAux;
        break;
    case SOPC_Int32_Id:
        clearFunction = SOPC_Int32_ClearAux;
        break;
    case SOPC_UInt32_Id:
        clearFunction = SOPC_UInt32_ClearAux;
        break;
    case SOPC_Int64_Id:
        clearFunction = SOPC_Int64_ClearAux;
        break;
    case SOPC_UInt64_Id:
        clearFunction = SOPC_UInt64_ClearAux;
        break;
    case SOPC_Float_Id:
        clearFunction = SOPC_Float_ClearAux;
        break;
    case SOPC_Double_Id:
        clearFunction = SOPC_Double_ClearAux;
        break;
    case SOPC_String_Id:
        clearFunction = SOPC_String_ClearAux;
        break;
    case SOPC_DateTime_Id:
        clearFunction = SOPC_DateTime_ClearAux;
        break;
    case SOPC_Guid_Id:
        clearFunction = SOPC_Guid_ClearAux;
        break;
    case SOPC_ByteString_Id:
        clearFunction = SOPC_ByteString_ClearAux;
        break;
    case SOPC_XmlElement_Id:
        clearFunction = SOPC_XmlElement_ClearAux;
        break;
    case SOPC_NodeId_Id:
        clearFunction = SOPC_NodeId_ClearAux;
        break;
    case SOPC_ExpandedNodeId_Id:
        clearFunction = SOPC_ExpandedNodeId_ClearAux;
        break;
    case SOPC_StatusCode_Id:
        clearFunction = SOPC_StatusCode_ClearAux;
        break;
    case SOPC_QualifiedName_Id:
        clearFunction = SOPC_QualifiedName_ClearAux;
        break;
    case SOPC_LocalizedText_Id:
        clearFunction = SOPC_LocalizedText_ClearAux;
        break;
    case SOPC_ExtensionObject_Id:
        clearFunction = SOPC_ExtensionObject_ClearAux;
        break;
    case SOPC_DataValue_Id:
        clearFunction = SOPC_DataValue_ClearAux;
        break;
    case SOPC_Variant_Id:
        clearFunction = SOPC_Variant_ClearAux;
        break;
    case SOPC_DiagnosticInfo_Id:
        clearFunction = SOPC_DiagnosticInfo_ClearAux;
        break;
    default:
        break;
    }
    return clearFunction;
}

SOPC_ReturnStatus SOPC_Null_CopyAux(void* dest, const void* src)
{
    (void) dest;
    (void) src;
    return SOPC_STATUS_OK;
}

static SOPC_EncodeableObject_PfnCopy* GetBuiltInTypeCopyFunction(SOPC_BuiltinId builtInTypeId)
{
    SOPC_EncodeableObject_PfnCopy* copyFunction = NULL;
    switch (builtInTypeId)
    {
    case SOPC_Null_Id:
        copyFunction = SOPC_Null_CopyAux;
        break;
    case SOPC_Boolean_Id:
        copyFunction = SOPC_Boolean_CopyAux;
        break;
    case SOPC_SByte_Id:
        copyFunction = SOPC_SByte_CopyAux;
        break;
    case SOPC_Byte_Id:
        copyFunction = SOPC_Byte_CopyAux;
        break;
    case SOPC_Int16_Id:
        copyFunction = SOPC_Int16_CopyAux;
        break;
    case SOPC_UInt16_Id:
        copyFunction = SOPC_UInt16_CopyAux;
        break;
    case SOPC_Int32_Id:
        copyFunction = SOPC_Int32_CopyAux;
        break;
    case SOPC_UInt32_Id:
        copyFunction = SOPC_UInt32_CopyAux;
        break;
    case SOPC_Int64_Id:
        copyFunction = SOPC_Int64_CopyAux;
        break;
    case SOPC_UInt64_Id:
        copyFunction = SOPC_UInt64_CopyAux;
        break;
    case SOPC_Float_Id:
        copyFunction = SOPC_Float_CopyAux;
        break;
    case SOPC_Double_Id:
        copyFunction = SOPC_Double_CopyAux;
        break;
    case SOPC_String_Id:
        copyFunction = SOPC_String_CopyAux;
        break;
    case SOPC_DateTime_Id:
        copyFunction = SOPC_DateTime_CopyAux;
        break;
    case SOPC_Guid_Id:
        copyFunction = SOPC_Guid_CopyAux;
        break;
    case SOPC_ByteString_Id:
        copyFunction = SOPC_ByteString_CopyAux;
        break;
    case SOPC_XmlElement_Id:
        copyFunction = SOPC_XmlElement_CopyAux;
        break;
    case SOPC_NodeId_Id:
        copyFunction = SOPC_NodeId_CopyAux;
        break;
    case SOPC_ExpandedNodeId_Id:
        copyFunction = SOPC_ExpandedNodeId_CopyAux;
        break;
    case SOPC_StatusCode_Id:
        copyFunction = SOPC_StatusCode_CopyAux;
        break;
    case SOPC_QualifiedName_Id:
        copyFunction = SOPC_QualifiedName_CopyAux;
        break;
    case SOPC_LocalizedText_Id:
        copyFunction = SOPC_LocalizedText_CopyAux;
        break;
    case SOPC_ExtensionObject_Id:
        copyFunction = SOPC_ExtensionObject_CopyAux;
        break;
    case SOPC_DataValue_Id:
        copyFunction = SOPC_DataValue_CopyAux;
        break;
    case SOPC_Variant_Id:
        copyFunction = SOPC_Variant_CopyAux;
        break;
    case SOPC_DiagnosticInfo_Id:
        copyFunction = SOPC_DiagnosticInfo_CopyAux;
        break;
    default:
        break;
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

static SOPC_EncodeableObject_PfnComp* GetBuiltInTypeCompFunction(SOPC_BuiltinId builtInTypeId)
{
    SOPC_EncodeableObject_PfnComp* compFunction = NULL;
    switch (builtInTypeId)
    {
    case SOPC_Null_Id:
        compFunction = SOPC_Null_CompareAux;
        break;
    case SOPC_Boolean_Id:
        compFunction = SOPC_Boolean_CompareAux;
        break;
    case SOPC_SByte_Id:
        compFunction = SOPC_SByte_CompareAux;
        break;
    case SOPC_Byte_Id:
        compFunction = SOPC_Byte_CompareAux;
        break;
    case SOPC_Int16_Id:
        compFunction = SOPC_Int16_CompareAux;
        break;
    case SOPC_UInt16_Id:
        compFunction = SOPC_UInt16_CompareAux;
        break;
    case SOPC_Int32_Id:
        compFunction = SOPC_Int32_CompareAux;
        break;
    case SOPC_UInt32_Id:
        compFunction = SOPC_UInt32_CompareAux;
        break;
    case SOPC_Int64_Id:
        compFunction = SOPC_Int64_CompareAux;
        break;
    case SOPC_UInt64_Id:
        compFunction = SOPC_UInt64_CompareAux;
        break;
    case SOPC_Float_Id:
        compFunction = SOPC_Float_CompareAux;
        break;
    case SOPC_Double_Id:
        compFunction = SOPC_Double_CompareAux;
        break;
    case SOPC_String_Id:
        compFunction = SOPC_String_CompareAux;
        break;
    case SOPC_DateTime_Id:
        compFunction = SOPC_DateTime_CompareAux;
        break;
    case SOPC_Guid_Id:
        compFunction = SOPC_Guid_CompareAux;
        break;
    case SOPC_ByteString_Id:
        compFunction = SOPC_ByteString_CompareAux;
        break;
    case SOPC_XmlElement_Id:
        compFunction = SOPC_XmlElement_CompareAux;
        break;
    case SOPC_NodeId_Id:
        compFunction = SOPC_NodeId_CompareAux;
        break;
    case SOPC_ExpandedNodeId_Id:
        compFunction = SOPC_ExpandedNodeId_CompareAux;
        break;
    case SOPC_StatusCode_Id:
        compFunction = SOPC_StatusCode_CompareAux;
        break;
    case SOPC_QualifiedName_Id:
        compFunction = SOPC_QualifiedName_CompareAux;
        break;
    case SOPC_LocalizedText_Id:
        compFunction = SOPC_LocalizedText_CompareAux;
        break;
    case SOPC_ExtensionObject_Id:
        compFunction = SOPC_ExtensionObject_CompareAux;
        break;
    case SOPC_DataValue_Id:
        compFunction = SOPC_DataValue_CompareAux;
        break;
    case SOPC_Variant_Id:
        compFunction = SOPC_Variant_CompareAux;
        break;
    case SOPC_DiagnosticInfo_Id:
        compFunction = SOPC_DiagnosticInfo_CompareAux;
        break;
    default:
        break;
    }
    return compFunction;
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
        val->Guid = malloc(sizeof(SOPC_Guid));
        if (NULL != val->Guid)
        {
            memset(val->Guid, 0, sizeof(SOPC_Guid));
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
        break;
    case SOPC_NodeId_Id:
        val->NodeId = malloc(sizeof(SOPC_NodeId));
        if (NULL != val->NodeId)
        {
            memset(val->NodeId, 0, sizeof(SOPC_NodeId));
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
        break;
    case SOPC_ExpandedNodeId_Id:
        val->ExpNodeId = malloc(sizeof(SOPC_ExpandedNodeId));
        if (NULL != val->ExpNodeId)
        {
            memset(val->ExpNodeId, 0, sizeof(SOPC_ExpandedNodeId));
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
        break;
    case SOPC_QualifiedName_Id:
        val->Qname = malloc(sizeof(SOPC_QualifiedName));
        if (NULL != val->Qname)
        {
            memset(val->Qname, 0, sizeof(SOPC_QualifiedName));
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
        break;
    case SOPC_LocalizedText_Id:
        val->LocalizedText = malloc(sizeof(SOPC_LocalizedText));
        if (NULL != val->LocalizedText)
        {
            memset(val->LocalizedText, 0, sizeof(SOPC_LocalizedText));
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
        break;
    case SOPC_ExtensionObject_Id:
        val->ExtObject = malloc(sizeof(SOPC_ExtensionObject));
        if (NULL != val->ExtObject)
        {
            memset(val->ExtObject, 0, sizeof(SOPC_ExtensionObject));
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
        break;
    case SOPC_DataValue_Id:
        val->DataValue = malloc(sizeof(SOPC_DataValue));
        if (NULL != val->DataValue)
        {
            memset(val->DataValue, 0, sizeof(SOPC_DataValue));
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
        break;
    case SOPC_DiagnosticInfo_Id:
        val->DiagInfo = malloc(sizeof(SOPC_DiagnosticInfo));
        if (NULL != val->DiagInfo)
        {
            memset(val->DiagInfo, 0, sizeof(SOPC_DiagnosticInfo));
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
        break;
    case SOPC_Variant_Id:
        // Part 6 Table 14 (v1.03): "The value shall not be a Variant
        //                           but it could be an array of Variants."
        // Note: Variant is not encoded in S2OPC stack for this case
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
        if (NULL != val->Guid)
        {
            free(val->Guid);
        }
        val->Guid = NULL;
        break;
    case SOPC_NodeId_Id:
        if (NULL != val->NodeId)
        {
            free(val->NodeId);
        }
        val->NodeId = NULL;
        break;
    case SOPC_ExpandedNodeId_Id:
        if (NULL != val->ExpNodeId)
        {
            free(val->ExpNodeId);
        }
        val->ExpNodeId = NULL;
        break;
    case SOPC_QualifiedName_Id:
        if (NULL != val->Qname)
        {
            free(val->Qname);
        }
        val->Qname = NULL;
        break;
    case SOPC_LocalizedText_Id:
        if (NULL != val->LocalizedText)
        {
            free(val->LocalizedText);
        }
        val->LocalizedText = NULL;
        break;
    case SOPC_ExtensionObject_Id:
        if (NULL != val->ExtObject)
        {
            free(val->ExtObject);
        }
        val->ExtObject = NULL;
        break;
    case SOPC_DataValue_Id:
        if (NULL != val->DataValue)
        {
            free(val->DataValue);
        }
        val->DataValue = NULL;
        break;
    case SOPC_DiagnosticInfo_Id:
        if (NULL != val->DiagInfo)
        {
            free(val->DiagInfo);
        }
        val->DiagInfo = NULL;
        break;
    case SOPC_Variant_Id:
        // Part 6 Table 14 (v1.03): "The value shall not be a Variant
        //                           but it could be an array of Variants."
        // Note: Variant is not encoded in S2OPC stack for this case
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
                    status = ApplyOpToVariantArrayBuiltInType(src->BuiltInTypeId, &dest->Value.Array.Content,
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
                        malloc((size_t) src->Value.Matrix.Dimensions * sizeof(int32_t));
                    if (NULL != dest->Value.Matrix.ArrayDimensions)
                    {
                        dest->Value.Matrix.Dimensions = src->Value.Matrix.Dimensions;
                        memcpy(dest->Value.Matrix.ArrayDimensions, src->Value.Matrix.ArrayDimensions,
                               (size_t) src->Value.Matrix.Dimensions * sizeof(int32_t));
                        status = AllocVariantArrayBuiltInType(src->BuiltInTypeId, &dest->Value.Matrix.Content,
                                                              (int32_t) matrixLength);
                        if (SOPC_STATUS_OK == status)
                        {
                            status = ApplyOpToVariantArrayBuiltInType(src->BuiltInTypeId, &dest->Value.Matrix.Content,
                                                                      &src->Value.Matrix.Content,
                                                                      (int32_t) matrixLength, copyFunction);
                            if (SOPC_STATUS_OK != status)
                            {
                                ClearToVariantArrayBuiltInType(src->BuiltInTypeId, &dest->Value.Matrix.Content,
                                                               (int32_t*) &matrixLength,
                                                               GetBuiltInTypeClearFunction(src->BuiltInTypeId));
                                free(dest->Value.Matrix.ArrayDimensions);
                                dest->Value.Matrix.ArrayDimensions = NULL;
                            }
                        }
                        else
                        {
                            free(dest->Value.Matrix.ArrayDimensions);
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
    assert(src != NULL);
    assert(dst != NULL);

    *dst = *src;
    src->DoNotClear = true;
}

SOPC_ReturnStatus SOPC_Variant_Compare(const SOPC_Variant* left, const SOPC_Variant* right, int32_t* comparison)
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
                SOPC_EncodeableObject_PfnComp* compFunction = GetBuiltInTypeCompFunction(left->BuiltInTypeId);
                if (NULL == compFunction)
                    return SOPC_STATUS_NOK;

                switch (right->ArrayType)
                {
                case SOPC_VariantArrayType_SingleValue:
                    status = CompareVariantsNonArrayBuiltInType(right->BuiltInTypeId, &left->Value, &right->Value,
                                                                comparison, compFunction);
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
                                                                compFunction, comparison);
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
                        if (false == error && *comparison == 0)
                        {
                            status = CompareVariantArrayBuiltInType(left->BuiltInTypeId, &left->Value.Matrix.Content,
                                                                    &right->Value.Matrix.Content,
                                                                    left->Value.Array.Length, compFunction, comparison);
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

SOPC_ReturnStatus SOPC_Variant_CompareAux(const void* left, const void* right, int32_t* comparison)
{
    return SOPC_Variant_Compare((const SOPC_Variant*) left, (const SOPC_Variant*) right, comparison);
}

SOPC_ReturnStatus SOPC_Variant_CompareRange(const SOPC_Variant* left,
                                            const SOPC_Variant* right,
                                            const SOPC_NumericRange* range,
                                            int32_t* comparison)
{
    if (left == NULL || right == NULL)
    {
        // To be consistent with SOPC_Variant_Compare
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (range == NULL)
    {
        return SOPC_Variant_Compare(left, right, comparison);
    }

    SOPC_Variant *left_sub = SOPC_Variant_Create(), *right_sub = SOPC_Variant_Create();

    if (left_sub == NULL || right_sub == NULL)
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
        status = SOPC_Variant_Compare(left_sub, right_sub, comparison);
    }

    SOPC_Variant_Delete(left_sub);
    SOPC_Variant_Delete(right_sub);

    return status;
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
                    free(variant->Value.Matrix.ArrayDimensions);
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
    if (variant == NULL)
    {
        return;
    }

    SOPC_Variant_Clear(variant);
    free(variant);
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

void SOPC_Initialize_Array(int32_t* noOfElts,
                           void** eltsArray,
                           size_t sizeOfElt,
                           SOPC_EncodeableObject_PfnInitialize* initFct)
{
    (void) initFct;
    (void) sizeOfElt;
    *noOfElts = 0;
    *eltsArray = NULL;
}

SOPC_ReturnStatus SOPC_Op_Array(int32_t noOfElts,
                                void* eltsArrayLeft,
                                void* eltsArrayRight,
                                size_t sizeOfElt,
                                SOPC_EncodeableObject_PfnCopy* opFct)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    size_t idx = 0;
    size_t pos = 0;
    SOPC_Byte* byteArrayLeft = eltsArrayLeft;
    SOPC_Byte* byteArrayRight = eltsArrayRight;
    if (noOfElts >= 0 && byteArrayLeft != NULL && byteArrayRight != NULL)
    {
        for (idx = 0; idx < (size_t) noOfElts && SOPC_STATUS_OK == status; idx++)
        {
            pos = idx * sizeOfElt;
            status = opFct(&(byteArrayLeft[pos]), &(byteArrayRight[pos]));
        }
    }
    else
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }
    return status;
}

SOPC_ReturnStatus SOPC_Comp_Array(int32_t noOfElts,
                                  void* eltsArrayLeft,
                                  void* eltsArrayRight,
                                  size_t sizeOfElt,
                                  SOPC_EncodeableObject_PfnComp* compFct,
                                  int32_t* comparison)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    size_t idx = 0;
    size_t pos = 0;
    SOPC_Byte* byteArrayLeft = eltsArrayLeft;
    SOPC_Byte* byteArrayRight = eltsArrayRight;
    if (noOfElts >= 0 && byteArrayLeft != NULL && byteArrayRight != NULL)
    {
        *comparison = 0;
        for (idx = 0; idx < (size_t) noOfElts && SOPC_STATUS_OK == status && *comparison == 0; idx++)
        {
            pos = idx * sizeOfElt;
            status = compFct(&(byteArrayLeft[pos]), &(byteArrayRight[pos]), comparison);
        }
    }
    else
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
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

            free(*eltsArray);
        }
        *noOfElts = 0;
        *eltsArray = NULL;
    }
}

static bool has_range_string(const SOPC_String* str, const SOPC_NumericRange* range)
{
    assert(range->n_dimensions == 1);

    if (str->Length <= 0)
    {
        return false;
    }

    return range->dimensions[0].start < ((uint32_t) str->Length);
}

static bool has_range_array(const SOPC_Variant* variant, const SOPC_NumericRange* range)
{
    assert(range->n_dimensions == 1);

    if (variant->ArrayType == SOPC_VariantArrayType_SingleValue)
    {
        // Dereferencing scalars is allowed for strings and bytestrings
        if (variant->BuiltInTypeId == SOPC_String_Id)
        {
            return has_range_string(&variant->Value.String, range);
        }
        else if (variant->BuiltInTypeId == SOPC_ByteString_Id)
        {
            return has_range_string(&variant->Value.Bstring, range);
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

    return range->dimensions[0].start < ((uint32_t) variant->Value.Array.Length);
}

SOPC_ReturnStatus SOPC_Variant_HasRange(const SOPC_Variant* variant, const SOPC_NumericRange* range, bool* has_range)
{
    switch (range->n_dimensions)
    {
    case 0:
        return SOPC_STATUS_INVALID_PARAMETERS;
    case 1:
        *has_range = has_range_array(variant, range);
        return SOPC_STATUS_OK;
    default:
        // Matrix will come later
        return SOPC_STATUS_NOT_SUPPORTED;
    }
}

// Common treatment for slicing both strings and bytestrings
static SOPC_ReturnStatus get_range_string_helper(SOPC_String* dst,
                                                 const SOPC_String* src,
                                                 const SOPC_NumericRange* range)
{
    assert(range->n_dimensions == 1);

    SOPC_Dimension* dim = &range->dimensions[0];
    assert(src->Length >= 0);
    uint32_t src_length = (uint32_t) src->Length;
    uint32_t start = dim->start;

    if (start >= src_length)
    {
        // Nothing to copy
        dst->Length = 0;
        return SOPC_STATUS_OK;
    }

    uint32_t end = (dim->end >= src_length) ? (src_length - 1) : dim->end;
    assert(end >= start);

    uint32_t dst_len = end - start + 1;

    dst->Data = calloc(1 + dst_len, sizeof(SOPC_Byte));

    if (dst->Data == NULL)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    memcpy(dst->Data, src->Data + start, (size_t) dst_len);
    dst->Length = (int32_t) dst_len;

    return SOPC_STATUS_OK;
}

static SOPC_ReturnStatus get_range_string(SOPC_Variant* dst, const SOPC_String* src, const SOPC_NumericRange* range)
{
    dst->ArrayType = SOPC_VariantArrayType_SingleValue;
    dst->BuiltInTypeId = SOPC_String_Id;
    dst->DoNotClear = false;

    SOPC_String_Initialize(&dst->Value.String);

    return get_range_string_helper(&dst->Value.String, src, range);
}

static SOPC_ReturnStatus get_range_bytestring(SOPC_Variant* dst, const SOPC_String* src, const SOPC_NumericRange* range)
{
    dst->ArrayType = SOPC_VariantArrayType_SingleValue;
    dst->BuiltInTypeId = SOPC_ByteString_Id;
    dst->DoNotClear = false;

    SOPC_ByteString_Initialize(&dst->Value.Bstring);

    return get_range_string_helper(&dst->Value.Bstring, src, range);
}

static size_t size_of_builtin_type(SOPC_BuiltinId type_id)
{
    switch (type_id)
    {
    case SOPC_Null_Id:
        return 0;
    case SOPC_Boolean_Id:
        return sizeof(SOPC_Boolean);
    case SOPC_SByte_Id:
        return sizeof(SOPC_SByte);
    case SOPC_Byte_Id:
        return sizeof(SOPC_Byte);
    case SOPC_Int16_Id:
        return sizeof(int16_t);
    case SOPC_UInt16_Id:
        return sizeof(uint16_t);
    case SOPC_Int32_Id:
        return sizeof(int32_t);
    case SOPC_UInt32_Id:
        return sizeof(uint32_t);
    case SOPC_Int64_Id:
        return sizeof(int64_t);
    case SOPC_UInt64_Id:
        return sizeof(uint64_t);
    case SOPC_Float_Id:
        return sizeof(float);
    case SOPC_Double_Id:
        return sizeof(double);
    case SOPC_String_Id:
        return sizeof(SOPC_String);
    case SOPC_DateTime_Id:
        return sizeof(SOPC_DateTime);
    case SOPC_Guid_Id:
        return sizeof(SOPC_Guid);
    case SOPC_ByteString_Id:
        return sizeof(SOPC_ByteString);
    case SOPC_XmlElement_Id:
        return sizeof(SOPC_XmlElement);
    case SOPC_NodeId_Id:
        return sizeof(SOPC_NodeId);
    case SOPC_ExpandedNodeId_Id:
        return sizeof(SOPC_ExpandedNodeId);
    case SOPC_StatusCode_Id:
        return sizeof(SOPC_StatusCode);
    case SOPC_QualifiedName_Id:
        return sizeof(SOPC_QualifiedName);
    case SOPC_LocalizedText_Id:
        return sizeof(SOPC_LocalizedText);
    case SOPC_ExtensionObject_Id:
        return sizeof(SOPC_ExtensionObject);
    case SOPC_DataValue_Id:
        return sizeof(SOPC_DataValue);
    case SOPC_Variant_Id:
        return sizeof(SOPC_Variant);
    case SOPC_DiagnosticInfo_Id:
        return sizeof(SOPC_DiagnosticInfo);
    }

    assert(false);
    return 0;
}

static SOPC_ReturnStatus get_range_array(SOPC_Variant* dst, const SOPC_Variant* src, const SOPC_NumericRange* range)
{
    assert(range->n_dimensions == 1);

    if (src->ArrayType == SOPC_VariantArrayType_SingleValue)
    {
        // Dereferencing scalars is allowed for strings and bytestrings
        if (src->BuiltInTypeId == SOPC_String_Id)
        {
            return get_range_string(dst, &src->Value.String, range);
        }
        else if (src->BuiltInTypeId == SOPC_ByteString_Id)
        {
            return get_range_bytestring(dst, &src->Value.Bstring, range);
        }
    }

    if (src->ArrayType != SOPC_VariantArrayType_Array)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_Dimension* dim = &range->dimensions[0];
    assert(src->Value.Array.Length >= 0);
    uint32_t start = dim->start;
    uint32_t array_length = (uint32_t) src->Value.Array.Length;

    dst->BuiltInTypeId = src->BuiltInTypeId;
    dst->DoNotClear = false;
    dst->Value.Array.Length = 0;

    if (start >= array_length)
    {
        // Nothing to copy
        return SOPC_STATUS_OK;
    }

    uint32_t end = (dim->end >= array_length) ? (array_length - 1) : dim->end;
    assert(end >= start);

    uint32_t dst_len = end - start + 1;

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
        return SOPC_STATUS_NOT_SUPPORTED;
    }
}

static SOPC_ReturnStatus set_range_string(SOPC_String* variant, const SOPC_String* src, const SOPC_NumericRange* range)
{
    assert(range->n_dimensions == 1);

    uint32_t start = range->dimensions[0].start;
    uint32_t end = range->dimensions[0].end;
    assert(end >= start);

    if (((uint32_t) src->Length) != (end - start + 1))
    {
        return SOPC_STATUS_NOK;
    }

    if (variant->Length <= 0 || ((uint32_t) variant->Length) <= start)
    {
        // Nothing to copy
        return SOPC_STATUS_OK;
    }

    if (((uint32_t) variant->Length) <= end)
    {
        end = (uint32_t)(variant->Length - 1);
    }

    size_t range_len = (size_t)(end - start + 1);
    memcpy(variant->Data + ((size_t) start), src->Data, range_len * sizeof(SOPC_Byte));

    return SOPC_STATUS_OK;
}

static SOPC_ReturnStatus set_range_array(SOPC_Variant* variant, const SOPC_Variant* src, const SOPC_NumericRange* range)
{
    assert(range->n_dimensions == 1);

    if (variant->BuiltInTypeId != src->BuiltInTypeId)
    {
        return SOPC_STATUS_NOK;
    }

    if (src->ArrayType == SOPC_VariantArrayType_SingleValue)
    {
        // Dereferencing scalars is allowed for strings and bytestrings
        if (src->BuiltInTypeId == SOPC_String_Id)
        {
            return set_range_string(&variant->Value.String, &src->Value.String, range);
        }
        else if (src->BuiltInTypeId == SOPC_ByteString_Id)
        {
            return set_range_string(&variant->Value.Bstring, &src->Value.Bstring, range);
        }
    }

    if (src->ArrayType != SOPC_VariantArrayType_Array)
    {
        return SOPC_STATUS_NOK;
    }

    uint32_t start = range->dimensions[0].start;
    uint32_t end = range->dimensions[0].end;
    assert(end >= start);

    if (((uint32_t) src->Value.Array.Length) != (end - start + 1))
    {
        return SOPC_STATUS_NOK;
    }

    if (variant->Value.Array.Length <= 0 || ((uint32_t) variant->Value.Array.Length) <= start)
    {
        // Nothing to copy
        return SOPC_STATUS_OK;
    }

    if (((uint32_t) variant->Value.Array.Length) <= end)
    {
        end = (uint32_t)(variant->Value.Array.Length - 1);
    }

    SOPC_EncodeableObject_PfnCopy* copyFunction = GetBuiltInTypeCopyFunction(src->BuiltInTypeId);
    SOPC_EncodeableObject_PfnClear* clearFunction = GetBuiltInTypeClearFunction(src->BuiltInTypeId);

    if (copyFunction == NULL || clearFunction == NULL)
    {
        return SOPC_STATUS_NOK;
    }

    const size_t type_size = size_of_builtin_type(src->BuiltInTypeId);

    // Untyped pointer to the source array data at the correct offset
    const uint8_t* src_i = *((const uint8_t* const*) &src->Value.Array.Content);
    uint8_t* dst_i = *((uint8_t**) &variant->Value.Array.Content) + start * type_size;

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

SOPC_ReturnStatus SOPC_Variant_SetRange(SOPC_Variant* variant, const SOPC_Variant* src, const SOPC_NumericRange* range)
{
    switch (range->n_dimensions)
    {
    case 0:
        return SOPC_STATUS_INVALID_PARAMETERS;
    case 1:
        return set_range_array(variant, src, range);
    default:
        // Matrix will come later
        return SOPC_STATUS_NOT_SUPPORTED;
    }
}

const SOPC_NodeId* SOPC_Variant_Get_DataType(SOPC_Variant* var)
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
                // Restore the DataType type id if it was the encoding object node
                var->Value.ExtObject->TypeId.NodeId.Data.Numeric = var->Value.ExtObject->Body.Object.ObjType->TypeId;
                return &var->Value.ExtObject->TypeId.NodeId;
            }
            else
            {
                // TODO / Note: if the type is unknown we cannot guarantee here the NodeId is a DataType, since it could
                // be the DefaultEncoding Object instead.
                // Returns the generic Structure type instead
                return &SOPC_Structure_Type;
            }
        }
        else
        {
            /* If type defined in another server or variant is an array, no guarantee that all are of same type. Keep
             * "Structure" generic type. */
            return &SOPC_Structure_Type;
        }
    default:
        assert(false); // Invalid type
        return &SOPC_Null_Type;
    }
}

int32_t SOPC_Variant_Get_ValueRank(SOPC_Variant* var)
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
        assert(false); // Invalid value
        return -3;
    }
}
