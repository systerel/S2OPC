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

#include <math.h>
#include <string.h>

#include "opcua_identifiers.h"
#include "sopc_assert.h"
#include "sopc_common_constants.h"
#include "sopc_encodeabletype.h"
#include "sopc_encoder.h"
#include "sopc_helper_endianness_cfg.h"
#include "sopc_mem_alloc.h"
#include "sopc_types.h"

SOPC_ReturnStatus SOPC_Byte_WriteAux(const void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_Byte_Write((const SOPC_Byte*) value, buf, nestedStructLevel);
}

SOPC_ReturnStatus SOPC_Byte_Write(const SOPC_Byte* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    if (NULL == value || NULL == buf)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    else if (nestedStructLevel >= SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    nestedStructLevel++;
    SOPC_ReturnStatus status = SOPC_Buffer_Write(buf, value, 1);
    if (SOPC_STATUS_OK != status)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY == status ? SOPC_STATUS_WOULD_BLOCK : SOPC_STATUS_ENCODING_ERROR;
    }

    return status;
}

SOPC_ReturnStatus SOPC_Byte_ReadAux(void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_Byte_Read((SOPC_Byte*) value, buf, nestedStructLevel);
}

SOPC_ReturnStatus SOPC_Byte_Read(SOPC_Byte* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    if (NULL == value || NULL == buf)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    else if (nestedStructLevel >= SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    nestedStructLevel++;
    SOPC_ReturnStatus status = SOPC_Buffer_Read(value, buf, 1);
    if (status != SOPC_STATUS_OK)
    {
        status = SOPC_STATUS_ENCODING_ERROR;
    }

    return status;
}

SOPC_ReturnStatus SOPC_Boolean_WriteAux(const void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_Boolean_Write((const SOPC_Boolean*) value, buf, nestedStructLevel);
}

SOPC_ReturnStatus SOPC_Boolean_Write(const SOPC_Boolean* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    SOPC_Byte encodedValue;
    if (NULL == value || NULL == buf)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    else if (nestedStructLevel >= SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    nestedStructLevel++;

    if (false == *value)
    {
        encodedValue = *value;
    }
    else
    {
        // Encoder should use 1 as True value
        encodedValue = 1;
    }
    return SOPC_Byte_Write(&encodedValue, buf, nestedStructLevel);
}

SOPC_ReturnStatus SOPC_Boolean_ReadAux(void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_Boolean_Read((SOPC_Boolean*) value, buf, nestedStructLevel);
}

SOPC_ReturnStatus SOPC_Boolean_Read(SOPC_Boolean* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    if (NULL == value)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    else if (nestedStructLevel >= SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    nestedStructLevel++;
    SOPC_ReturnStatus status = SOPC_Byte_Read(value, buf, nestedStructLevel);
    if (SOPC_STATUS_OK == status)
    {
        if (false != *value)
        {
            // Decoder should use 1 as True value
            *value = 1;
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_SByte_WriteAux(const void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_SByte_Write((const SOPC_SByte*) value, buf, nestedStructLevel);
}

SOPC_ReturnStatus SOPC_SByte_Write(const SOPC_SByte* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    if (NULL == value || NULL == buf)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    else if (nestedStructLevel >= SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    nestedStructLevel++;
    SOPC_ReturnStatus status = SOPC_Buffer_Write(buf, (const SOPC_Byte*) value, 1);

    if (SOPC_STATUS_OK != status)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY == status ? SOPC_STATUS_WOULD_BLOCK : SOPC_STATUS_ENCODING_ERROR;
    }
    return status;
}

SOPC_ReturnStatus SOPC_SByte_ReadAux(void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_SByte_Read((SOPC_SByte*) value, buf, nestedStructLevel);
}

SOPC_ReturnStatus SOPC_SByte_Read(SOPC_SByte* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    if (NULL == value || NULL == buf)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    else if (nestedStructLevel >= SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    nestedStructLevel++;
    SOPC_ReturnStatus status = SOPC_Buffer_Read((SOPC_Byte*) value, buf, 1);

    if (SOPC_STATUS_OK != status)
    {
        status = SOPC_STATUS_ENCODING_ERROR;
    }
    return status;
}

SOPC_ReturnStatus SOPC_Int16_WriteAux(const void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_Int16_Write((const int16_t*) value, buf, nestedStructLevel);
}

SOPC_ReturnStatus SOPC_Int16_Write(const int16_t* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    if (NULL == value || NULL == buf)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    else if (nestedStructLevel >= SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    nestedStructLevel++;

    int16_t encodedValue = *value;
    SOPC_TO_LITTLE_ENDIAN_16BITS(encodedValue);
    SOPC_ReturnStatus status = SOPC_Buffer_Write(buf, (SOPC_Byte*) &encodedValue, 2);

    if (SOPC_STATUS_OK != status)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY == status ? SOPC_STATUS_WOULD_BLOCK : SOPC_STATUS_ENCODING_ERROR;
    }
    return status;
}

SOPC_ReturnStatus SOPC_Int16_ReadAux(void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_Int16_Read((int16_t*) value, buf, nestedStructLevel);
}

SOPC_ReturnStatus SOPC_Int16_Read(int16_t* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    if (NULL == value || NULL == buf)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    else if (nestedStructLevel >= SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    nestedStructLevel++;

    SOPC_ReturnStatus status = SOPC_Buffer_Read((SOPC_Byte*) value, buf, 2);
    if (SOPC_STATUS_OK == status)
    {
        SOPC_TO_LITTLE_ENDIAN_16BITS(*value);
    }
    else
    {
        status = SOPC_STATUS_ENCODING_ERROR;
    }
    return status;
}

SOPC_ReturnStatus SOPC_UInt16_WriteAux(const void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_UInt16_Write((const uint16_t*) value, buf, nestedStructLevel);
}

SOPC_ReturnStatus SOPC_UInt16_Write(const uint16_t* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    if (NULL == value || NULL == buf)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    else if (nestedStructLevel >= SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    nestedStructLevel++;
    uint16_t encodedValue = *value;
    SOPC_TO_LITTLE_ENDIAN_16BITS(encodedValue);
    SOPC_ReturnStatus status = SOPC_Buffer_Write(buf, (SOPC_Byte*) &encodedValue, 2);

    if (SOPC_STATUS_OK != status)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY == status ? SOPC_STATUS_WOULD_BLOCK : SOPC_STATUS_ENCODING_ERROR;
    }
    return status;
}

SOPC_ReturnStatus SOPC_UInt16_ReadAux(void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_UInt16_Read((uint16_t*) value, buf, nestedStructLevel);
}

SOPC_ReturnStatus SOPC_UInt16_Read(uint16_t* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    if (NULL == value || NULL == buf)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    else if (nestedStructLevel >= SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    nestedStructLevel++;

    SOPC_ReturnStatus status = SOPC_Buffer_Read((SOPC_Byte*) value, buf, 2);
    if (SOPC_STATUS_OK == status)
    {
        SOPC_TO_LITTLE_ENDIAN_16BITS(*value);
    }
    else
    {
        status = SOPC_STATUS_ENCODING_ERROR;
    }
    return status;
}

SOPC_ReturnStatus SOPC_Int32_WriteAux(const void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_Int32_Write((const int32_t*) value, buf, nestedStructLevel);
}

SOPC_ReturnStatus SOPC_Int32_Write(const int32_t* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    if (NULL == value || NULL == buf)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    else if (nestedStructLevel >= SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct)
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    nestedStructLevel++;
    int32_t encodedValue = *value;
    SOPC_TO_LITTLE_ENDIAN_32BITS(encodedValue);
    SOPC_ReturnStatus status = SOPC_Buffer_Write(buf, (SOPC_Byte*) &encodedValue, 4);

    if (SOPC_STATUS_OK != status)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY == status ? SOPC_STATUS_WOULD_BLOCK : SOPC_STATUS_ENCODING_ERROR;
    }

    return status;
}

SOPC_ReturnStatus SOPC_Int32_ReadAux(void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_Int32_Read((int32_t*) value, buf, nestedStructLevel);
}

SOPC_ReturnStatus SOPC_Int32_Read(int32_t* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    if (NULL == value || NULL == buf)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    else if (nestedStructLevel >= SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    nestedStructLevel++;
    SOPC_ReturnStatus status = SOPC_Buffer_Read((SOPC_Byte*) value, buf, 4);
    if (SOPC_STATUS_OK == status)
    {
        SOPC_TO_LITTLE_ENDIAN_32BITS(*value);
    }
    else
    {
        status = SOPC_STATUS_ENCODING_ERROR;
    }

    return status;
}

SOPC_ReturnStatus SOPC_UInt32_WriteAux(const void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_UInt32_Write((const uint32_t*) value, buf, nestedStructLevel);
}

SOPC_ReturnStatus SOPC_UInt32_Write(const uint32_t* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    if (NULL == value || NULL == buf)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    else if (nestedStructLevel >= SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct)
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    nestedStructLevel++;
    uint32_t encodedValue = *value;
    SOPC_TO_LITTLE_ENDIAN_32BITS(encodedValue);
    SOPC_ReturnStatus status = SOPC_Buffer_Write(buf, (SOPC_Byte*) &encodedValue, 4);

    if (SOPC_STATUS_OK != status)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY == status ? SOPC_STATUS_WOULD_BLOCK : SOPC_STATUS_ENCODING_ERROR;
    }
    return status;
}

SOPC_ReturnStatus SOPC_UInt32_ReadAux(void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_UInt32_Read((uint32_t*) value, buf, nestedStructLevel);
}

SOPC_ReturnStatus SOPC_UInt32_Read(uint32_t* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    if (NULL == value || NULL == buf)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    else if (nestedStructLevel >= SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    nestedStructLevel++;
    SOPC_ReturnStatus status = SOPC_Buffer_Read((SOPC_Byte*) value, buf, 4);
    if (SOPC_STATUS_OK == status)
    {
        SOPC_TO_LITTLE_ENDIAN_32BITS(*value);
    }
    else
    {
        status = SOPC_STATUS_ENCODING_ERROR;
    }

    return status;
}

SOPC_ReturnStatus SOPC_Int64_WriteAux(const void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_Int64_Write((const int64_t*) value, buf, nestedStructLevel);
}

SOPC_ReturnStatus SOPC_Int64_Write(const int64_t* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    if (NULL == value || NULL == buf)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    else if (nestedStructLevel >= SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct)
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    nestedStructLevel++;
    int64_t encodedValue = *value;
    SOPC_TO_LITTLE_ENDIAN_64BITS(encodedValue);
    SOPC_ReturnStatus status = SOPC_Buffer_Write(buf, (SOPC_Byte*) &encodedValue, 8);

    if (SOPC_STATUS_OK != status)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY == status ? SOPC_STATUS_WOULD_BLOCK : SOPC_STATUS_ENCODING_ERROR;
    }

    return status;
}

SOPC_ReturnStatus SOPC_Int64_ReadAux(void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_Int64_Read((int64_t*) value, buf, nestedStructLevel);
}

SOPC_ReturnStatus SOPC_Int64_Read(int64_t* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    if (NULL == value || NULL == buf)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    else if (nestedStructLevel >= SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    nestedStructLevel++;
    SOPC_ReturnStatus status = SOPC_Buffer_Read((SOPC_Byte*) value, buf, 8);
    if (SOPC_STATUS_OK == status)
    {
        SOPC_TO_LITTLE_ENDIAN_64BITS(*value);
    }
    else
    {
        status = SOPC_STATUS_ENCODING_ERROR;
    }

    return status;
}

SOPC_ReturnStatus SOPC_UInt64_WriteAux(const void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_UInt64_Write((const uint64_t*) value, buf, nestedStructLevel);
}

SOPC_ReturnStatus SOPC_UInt64_Write(const uint64_t* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    if (NULL == value || NULL == buf)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    else if (nestedStructLevel >= SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct)
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    nestedStructLevel++;
    uint64_t encodedValue = *value;
    SOPC_TO_LITTLE_ENDIAN_64BITS(encodedValue);
    SOPC_ReturnStatus status = SOPC_Buffer_Write(buf, (SOPC_Byte*) &encodedValue, 8);

    if (SOPC_STATUS_OK != status)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY == status ? SOPC_STATUS_WOULD_BLOCK : SOPC_STATUS_ENCODING_ERROR;
    }

    return status;
}

SOPC_ReturnStatus SOPC_UInt64_ReadAux(void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_UInt64_Read((uint64_t*) value, buf, nestedStructLevel);
}

SOPC_ReturnStatus SOPC_UInt64_Read(uint64_t* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    if (NULL == value || NULL == buf)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    else if (nestedStructLevel >= SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    nestedStructLevel++;
    SOPC_ReturnStatus status = SOPC_Buffer_Read((SOPC_Byte*) value, buf, 8);
    if (SOPC_STATUS_OK == status)
    {
        SOPC_TO_LITTLE_ENDIAN_64BITS(*value);
    }
    else
    {
        status = SOPC_STATUS_ENCODING_ERROR;
    }

    return status;
}

static float normalize_float(float value)
{
    switch (fpclassify(value))
    {
    case FP_INFINITE:
    case FP_NORMAL:
    case FP_ZERO:
        // Nothing to do
        return value;
    default:
        break;
    }
    // Try to normalize to Quiet NaN value
    static bool once = true;
    static bool nanUnsupported = false;
    static float result;
    if (once)
    {
        once = false;
        result = nanf("");
        if (fpclassify(result) == FP_ZERO)
        {
            nanUnsupported = true;
        }
    }
    if (nanUnsupported)
    {
        // Quiet NaN unsupported
        return value;
    }
    return result;
}

SOPC_ReturnStatus SOPC_Float_WriteAux(const void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_Float_Write((const float*) value, buf, nestedStructLevel);
}

SOPC_ReturnStatus SOPC_Float_Write(const float* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    if (NULL == value || NULL == buf)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    else if (nestedStructLevel >= SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    nestedStructLevel++;
    float encodedValue = *value;
    // Normalize NaN value if quiet NaN available
    encodedValue = normalize_float(encodedValue);

    SOPC_TO_LITTLE_ENDIAN_FLOAT(encodedValue);
    SOPC_ReturnStatus status = SOPC_Buffer_Write(buf, (SOPC_Byte*) &encodedValue, 4);

    if (SOPC_STATUS_OK != status)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY == status ? SOPC_STATUS_WOULD_BLOCK : SOPC_STATUS_ENCODING_ERROR;
    }

    return status;
}

SOPC_ReturnStatus SOPC_Float_ReadAux(void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_Float_Read((float*) value, buf, nestedStructLevel);
}

SOPC_ReturnStatus SOPC_Float_Read(float* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    if (NULL == value || NULL == buf)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    else if (nestedStructLevel >= SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    nestedStructLevel++;
    SOPC_ReturnStatus status = SOPC_Buffer_Read((SOPC_Byte*) value, buf, 4);
    if (SOPC_STATUS_OK == status)
    {
        SOPC_TO_LITTLE_ENDIAN_FLOAT(*value);
    }
    else
    {
        status = SOPC_STATUS_ENCODING_ERROR;
    }
    // Normalize NaN value if quiet NaN available
    *value = normalize_float(*value);

    return status;
}

SOPC_ReturnStatus SOPC_Double_WriteAux(const void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_Double_Write((const double*) value, buf, nestedStructLevel);
}

static double normalize_double(double value)
{
    switch (fpclassify(value))
    {
    case FP_INFINITE:
    case FP_NORMAL:
    case FP_ZERO:
        // Nothing to do
        return value;
    default:
        break;
    }
    // Try to normalize to Quiet NaN value
    static bool once = true;
    static bool nanUnsupported = false;
    static double result;
    if (once)
    {
        once = false;
        result = nan("");
        if (fpclassify(result) == FP_ZERO)
        {
            nanUnsupported = true;
        }
    }
    if (nanUnsupported)
    {
        // Quiet NaN unsupported
        return value;
    }
    return result;
}

SOPC_ReturnStatus SOPC_Double_Write(const double* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    if (NULL == value || NULL == buf)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    else if (nestedStructLevel >= SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    nestedStructLevel++;
    double encodedValue = *value;
    // Normalize NaN/denormalized value if quiet NaN available
    encodedValue = normalize_double(encodedValue);
    SOPC_TO_LITTLE_ENDIAN_DOUBLE(encodedValue);
    SOPC_ReturnStatus status = SOPC_Buffer_Write(buf, (SOPC_Byte*) &encodedValue, 8);

    if (SOPC_STATUS_OK != status)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY == status ? SOPC_STATUS_WOULD_BLOCK : SOPC_STATUS_ENCODING_ERROR;
    }
    return status;
}

SOPC_ReturnStatus SOPC_Double_ReadAux(void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_Double_Read((double*) value, buf, nestedStructLevel);
}

SOPC_ReturnStatus SOPC_Double_Read(double* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    if (NULL == value || NULL == buf)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    else if (nestedStructLevel >= SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct)
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    nestedStructLevel++;

    SOPC_ReturnStatus status = SOPC_Buffer_Read((SOPC_Byte*) value, buf, 8);
    if (SOPC_STATUS_OK == status)
    {
        SOPC_TO_LITTLE_ENDIAN_DOUBLE(*value);
    }
    else
    {
        status = SOPC_STATUS_ENCODING_ERROR;
    }
    // Normalize NaN/denormalized value if quiet NaN available
    *value = normalize_double(*value);
    return status;
}

SOPC_ReturnStatus SOPC_ByteString_WriteAux(const void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_ByteString_Write((const SOPC_ByteString*) value, buf, nestedStructLevel);
}

SOPC_ReturnStatus SOPC_ByteString_Write(const SOPC_ByteString* str, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    if (NULL == str || NULL == buf)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    else if (nestedStructLevel >= SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct)
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    nestedStructLevel++;
    int32_t length;
    if (str->Length > 0)
    {
        length = str->Length;
    }
    else
    {
        length = -1;
    }
    SOPC_ReturnStatus status = SOPC_Int32_Write(&length, buf, nestedStructLevel);
    if (SOPC_STATUS_OK == status && str->Length > 0)
    {
        status = SOPC_Buffer_Write(buf, str->Data, (uint32_t) str->Length);

        if (SOPC_STATUS_OK != status)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY == status ? SOPC_STATUS_WOULD_BLOCK : SOPC_STATUS_ENCODING_ERROR;
        }
    }
    else if (SOPC_STATUS_OK != status)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY == status ? SOPC_STATUS_WOULD_BLOCK : SOPC_STATUS_ENCODING_ERROR;
    }

    return status;
}

SOPC_ReturnStatus SOPC_ByteString_ReadAux(void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_ByteString_Read((SOPC_ByteString*) value, buf, nestedStructLevel);
}

SOPC_ReturnStatus SOPC_ByteString_Read(SOPC_ByteString* str, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    int32_t length;
    if (NULL == str || NULL != str->Data || str->Length > 0 || NULL == buf)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    else if (nestedStructLevel >= SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    nestedStructLevel++;
    SOPC_ReturnStatus status = SOPC_Int32_Read(&length, buf, nestedStructLevel);

    if (SOPC_STATUS_OK == status)
    {
        if (length > 0)
        {
            if (length <= SOPC_Internal_Common_GetEncodingConstants()->max_string_length &&
                (uint64_t) length * sizeof(SOPC_Byte) <= SIZE_MAX)
            {
                str->Length = length;
                str->Data = SOPC_Malloc(sizeof(SOPC_Byte) * (size_t) length);
                if (NULL != str->Data)
                {
                    status = SOPC_Buffer_Read(str->Data, buf, (uint32_t) length);
                    if (SOPC_STATUS_OK != status)
                    {
                        status = SOPC_STATUS_ENCODING_ERROR;
                        SOPC_Free(str->Data);
                        str->Data = NULL;
                        str->Length = -1;
                    }
                }
                else
                {
                    status = SOPC_STATUS_OUT_OF_MEMORY;
                    str->Length = -1;
                }
            }
            else
            {
                status = SOPC_STATUS_OUT_OF_MEMORY;
            }
        }
        else
        {
            str->Length = -1;
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_String_WriteAux(const void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_String_Write((const SOPC_String*) value, buf, nestedStructLevel);
}

SOPC_ReturnStatus SOPC_String_Write(const SOPC_String* str, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    if (NULL == str || NULL == buf)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    else if (nestedStructLevel >= SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    nestedStructLevel++;
    int32_t length;
    if (str->Length > 0)
    {
        length = str->Length;
    }
    else
    {
        length = -1;
    }
    SOPC_ReturnStatus status = SOPC_Int32_Write(&length, buf, nestedStructLevel);
    if (SOPC_STATUS_OK == status && str->Length > 0)
    {
        status = SOPC_Buffer_Write(buf, str->Data, (uint32_t) str->Length);

        if (SOPC_STATUS_OK != status)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY == status ? SOPC_STATUS_WOULD_BLOCK : SOPC_STATUS_ENCODING_ERROR;
        }
    }

    return status;
}

SOPC_ReturnStatus SOPC_String_ReadAux(void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_String_Read((SOPC_String*) value, buf, nestedStructLevel);
}

SOPC_ReturnStatus SOPC_String_ReadWithLimitedLength(SOPC_String* str,
                                                    int32_t maxLength,
                                                    SOPC_Buffer* buf,
                                                    uint32_t nestedStructLevel)
{
    int32_t length;
    if (NULL == str || NULL != str->Data || str->Length > 0 || NULL == buf)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    else if (nestedStructLevel >= SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    nestedStructLevel++;
    SOPC_ReturnStatus status = SOPC_Int32_Read(&length, buf, nestedStructLevel);

    if (SOPC_STATUS_OK == status)
    {
        if (length > 0)
        {
            if (length <= SOPC_Internal_Common_GetEncodingConstants()->max_string_length &&
                (uint64_t) length + 1 <= SIZE_MAX)
            {
                if (0 == maxLength || length <= maxLength)
                {
                    str->Length = length;
                    // +1 to add '\0' character for CString compatibility
                    str->Data = SOPC_Malloc(sizeof(SOPC_Byte) * (size_t)(length + 1));
                    if (NULL != str->Data)
                    {
                        status = SOPC_Buffer_Read(str->Data, buf, (uint32_t) length);
                        if (SOPC_STATUS_OK != status)
                        {
                            status = SOPC_STATUS_ENCODING_ERROR;
                            SOPC_Free(str->Data);
                            str->Data = NULL;
                            str->Length = -1;
                        }
                        else
                        {
                            // Add '\0' character for CString compatibility
                            str->Data[str->Length] = '\0';
                        }
                    }
                    else
                    {
                        str->Length = -1;
                        status = SOPC_STATUS_OUT_OF_MEMORY;
                    }
                }
                else
                {
                    str->Length = -1;
                    status = SOPC_STATUS_WOULD_BLOCK;
                }
            }
            else
            {
                status = SOPC_STATUS_OUT_OF_MEMORY;
            }
        }
        else
        {
            str->Length = -1;
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_String_Read(SOPC_String* str, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_String_ReadWithLimitedLength(str, 0, buf, nestedStructLevel);
}

SOPC_ReturnStatus SOPC_XmlElement_WriteAux(const void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_XmlElement_Write((const SOPC_XmlElement*) value, buf, nestedStructLevel);
}

SOPC_ReturnStatus SOPC_XmlElement_Write(const SOPC_XmlElement* xml, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    // TODO: check XML validity ?
    return SOPC_ByteString_Write((const SOPC_ByteString*) xml, buf, nestedStructLevel);
}

SOPC_ReturnStatus SOPC_XmlElement_ReadAux(void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_XmlElement_Read((SOPC_XmlElement*) value, buf, nestedStructLevel);
}

SOPC_ReturnStatus SOPC_XmlElement_Read(SOPC_XmlElement* xml, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    // TODO: parse XML ?
    return SOPC_ByteString_Read((SOPC_ByteString*) xml, buf, nestedStructLevel);
}

SOPC_ReturnStatus SOPC_DateTime_WriteAux(const void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_DateTime_Write((const SOPC_DateTime*) value, buf, nestedStructLevel);
}

SOPC_ReturnStatus SOPC_DateTime_Write(const SOPC_DateTime* date, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    if (NULL == date || NULL == buf)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    else if (nestedStructLevel >= SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    nestedStructLevel++;
    return SOPC_Int64_Write(date, buf, nestedStructLevel);
}

SOPC_ReturnStatus SOPC_DateTime_ReadAux(void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_DateTime_Read((SOPC_DateTime*) value, buf, nestedStructLevel);
}

SOPC_ReturnStatus SOPC_DateTime_Read(SOPC_DateTime* date, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    if (NULL == date)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    else if (nestedStructLevel >= SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    nestedStructLevel++;
    return SOPC_Int64_Read(date, buf, nestedStructLevel);
}

SOPC_ReturnStatus SOPC_Guid_WriteAux(const void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_Guid_Write((const SOPC_Guid*) value, buf, nestedStructLevel);
}

SOPC_ReturnStatus SOPC_Guid_Write(const SOPC_Guid* guid, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    if (NULL == guid || NULL == buf)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    else if (nestedStructLevel >= SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    nestedStructLevel++;
    SOPC_ReturnStatus status = SOPC_UInt32_Write(&guid->Data1, buf, nestedStructLevel);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_UInt16_Write(&guid->Data2, buf, nestedStructLevel);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_UInt16_Write(&guid->Data3, buf, nestedStructLevel);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Buffer_Write(buf, &(guid->Data4[0]), 8);
    }
    if (SOPC_STATUS_OK != status)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY == status ? SOPC_STATUS_WOULD_BLOCK : SOPC_STATUS_ENCODING_ERROR;
    }

    return status;
}

SOPC_ReturnStatus SOPC_Guid_ReadAux(void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_Guid_Read((SOPC_Guid*) value, buf, nestedStructLevel);
}

SOPC_ReturnStatus SOPC_Guid_Read(SOPC_Guid* guid, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    if (NULL == guid || NULL == buf)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    else if (nestedStructLevel >= SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    nestedStructLevel++;

    SOPC_ReturnStatus status = SOPC_UInt32_Read(&guid->Data1, buf, nestedStructLevel);

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_UInt16_Read(&guid->Data2, buf, nestedStructLevel);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_UInt16_Read(&guid->Data3, buf, nestedStructLevel);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Buffer_Read(&(guid->Data4[0]), buf, 8);
    }

    if (SOPC_STATUS_OK != status)
    {
        status = SOPC_STATUS_ENCODING_ERROR;
        SOPC_UInt32_Clear(&guid->Data1);
        SOPC_UInt16_Clear(&guid->Data2);
        SOPC_UInt16_Clear(&guid->Data3);
    }
    return status;
}

static SOPC_NodeId_DataEncoding GetNodeIdDataEncoding(const SOPC_NodeId* nodeId)
{
    SOPC_NodeId_DataEncoding encodingEnum = SOPC_NodeIdEncoding_Invalid;
    switch (nodeId->IdentifierType)
    {
    case SOPC_IdentifierType_Numeric:
        if (OPCUA_NAMESPACE_INDEX == nodeId->Namespace && nodeId->Data.Numeric <= UINT8_MAX)
        {
            // Default namespace and Id : [0..255]
            encodingEnum = SOPC_NodeIdEncoding_TwoBytes;
        }
        else if (nodeId->Namespace <= UINT8_MAX && nodeId->Data.Numeric <= UINT16_MAX)
        {
            // Namespace : [0..255] and Id : [0..65535]
            encodingEnum = SOPC_NodeIdEncoding_FourBytes;
        }
        else
        {
            // Other numeric cases: Namespace on 2 bytes and Id on 4 bytes
            encodingEnum = SOPC_NodeIdEncoding_Numeric;
        }
        break;
    case SOPC_IdentifierType_String:
        encodingEnum = SOPC_NodeIdEncoding_String;
        break;
    case SOPC_IdentifierType_Guid:
        encodingEnum = SOPC_NodeIdEncoding_Guid;
        break;
    case SOPC_IdentifierType_ByteString:
        encodingEnum = SOPC_NodeIdEncoding_ByteString;
        break;
    default:
        break;
    }
    return encodingEnum;
}

static SOPC_ReturnStatus Internal_NodeId_Write(SOPC_Buffer* buf,
                                               SOPC_Byte encodingByte,
                                               const SOPC_NodeId* nodeId,
                                               uint32_t nestedStructLevel)
{
    SOPC_ASSERT(NULL != nodeId);
    SOPC_NodeId_DataEncoding encodingType = 0x0F & encodingByte; // Eliminate flags

    SOPC_Byte byte = 0;
    uint16_t twoBytes = 0;

    SOPC_ReturnStatus status = SOPC_Byte_Write(&encodingByte, buf, nestedStructLevel);
    if (SOPC_STATUS_OK == status)
    {
        switch (encodingType)
        {
        case SOPC_NodeIdEncoding_Invalid:
            status = SOPC_STATUS_INVALID_PARAMETERS;
            break;
        case SOPC_NodeIdEncoding_TwoBytes:
            SOPC_ASSERT(OPCUA_NAMESPACE_INDEX == nodeId->Namespace);
            SOPC_ASSERT(nodeId->Data.Numeric <= UINT8_MAX);
            byte = (SOPC_Byte) nodeId->Data.Numeric;

            status = SOPC_Byte_Write(&byte, buf, nestedStructLevel);
            break;
        case SOPC_NodeIdEncoding_FourBytes:
            SOPC_ASSERT(nodeId->Namespace <= UINT8_MAX);
            SOPC_ASSERT(nodeId->Data.Numeric <= UINT16_MAX);
            twoBytes = (uint16_t) nodeId->Data.Numeric;
            byte = (SOPC_Byte) nodeId->Namespace;

            status = SOPC_Byte_Write(&byte, buf, nestedStructLevel);
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_UInt16_Write(&twoBytes, buf, nestedStructLevel);
            }
            break;
        case SOPC_NodeIdEncoding_Numeric:
            status = SOPC_UInt16_Write(&nodeId->Namespace, buf, nestedStructLevel);
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_UInt32_Write(&nodeId->Data.Numeric, buf, nestedStructLevel);
            }
            break;
        case SOPC_NodeIdEncoding_String:
            status = SOPC_UInt16_Write(&nodeId->Namespace, buf, nestedStructLevel);
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_String_Write(&nodeId->Data.String, buf, nestedStructLevel);
            }
            break;
        case SOPC_NodeIdEncoding_Guid:
            status = SOPC_UInt16_Write(&nodeId->Namespace, buf, nestedStructLevel);
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_Guid_Write(nodeId->Data.Guid, buf, nestedStructLevel);
            }
            break;
        case SOPC_NodeIdEncoding_ByteString:
            status = SOPC_UInt16_Write(&nodeId->Namespace, buf, nestedStructLevel);
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_ByteString_Write(&nodeId->Data.Bstring, buf, nestedStructLevel);
            }
            break;
        default:
            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
    }

    return status;
}

SOPC_ReturnStatus SOPC_NodeId_WriteAux(const void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_NodeId_Write((const SOPC_NodeId*) value, buf, nestedStructLevel);
}

SOPC_ReturnStatus SOPC_NodeId_Write(const SOPC_NodeId* nodeId, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    if (NULL == nodeId)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    else if (nestedStructLevel >= SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    nestedStructLevel++;
    return Internal_NodeId_Write(buf, (SOPC_Byte) GetNodeIdDataEncoding(nodeId), nodeId, nestedStructLevel);
}

static SOPC_ReturnStatus Internal_NodeId_Read(SOPC_Buffer* buf,
                                              SOPC_NodeId* nodeId,
                                              SOPC_Byte* encodingByte,
                                              uint32_t nestedStructLevel)
{
    SOPC_ASSERT(NULL != nodeId);
    SOPC_NodeId_DataEncoding encodingType = 0x0F;

    SOPC_Byte byte = 0;
    uint16_t twoBytes = 0;

    if (nestedStructLevel >= SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    SOPC_ReturnStatus status = SOPC_Byte_Read(encodingByte, buf, nestedStructLevel);

    if (SOPC_STATUS_OK == status)
    {
        encodingType = 0x0F & *encodingByte; // Eliminate flags
        switch (encodingType)
        {
        case SOPC_NodeIdEncoding_Invalid:
            status = SOPC_STATUS_ENCODING_ERROR;
            break;
        case SOPC_NodeIdEncoding_TwoBytes:
            nodeId->IdentifierType = SOPC_IdentifierType_Numeric;
            nodeId->Namespace = 0;
            status = SOPC_Byte_Read(&byte, buf, nestedStructLevel);
            nodeId->Data.Numeric = (uint32_t) byte;
            break;
        case SOPC_NodeIdEncoding_FourBytes:
            nodeId->IdentifierType = SOPC_IdentifierType_Numeric;
            status = SOPC_Byte_Read(&byte, buf, nestedStructLevel);
            nodeId->Namespace = byte;
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_UInt16_Read(&twoBytes, buf, nestedStructLevel);
                nodeId->Data.Numeric = twoBytes;
            }
            break;
        case SOPC_NodeIdEncoding_Numeric:
            nodeId->IdentifierType = SOPC_IdentifierType_Numeric;
            status = SOPC_UInt16_Read(&nodeId->Namespace, buf, nestedStructLevel);
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_UInt32_Read(&nodeId->Data.Numeric, buf, nestedStructLevel);
            }
            break;
        case SOPC_NodeIdEncoding_String:
            nodeId->IdentifierType = SOPC_IdentifierType_String;
            status = SOPC_UInt16_Read(&nodeId->Namespace, buf, nestedStructLevel);
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_String_Read(&nodeId->Data.String, buf, nestedStructLevel);
            }
            break;
        case SOPC_NodeIdEncoding_Guid:
            nodeId->IdentifierType = SOPC_IdentifierType_Guid;
            status = SOPC_UInt16_Read(&nodeId->Namespace, buf, nestedStructLevel);
            if (SOPC_STATUS_OK == status)
            {
                nodeId->Data.Guid = SOPC_Malloc(sizeof(SOPC_Guid));
                if (NULL == nodeId->Data.Guid)
                {
                    status = SOPC_STATUS_NOK;
                }
                else
                {
                    SOPC_Guid_Initialize(nodeId->Data.Guid);
                    status = SOPC_Guid_Read(nodeId->Data.Guid, buf, nestedStructLevel);
                    if (SOPC_STATUS_OK != status)
                    {
                        SOPC_Free(nodeId->Data.Guid);
                        nodeId->Data.Guid = NULL;
                    }
                }
            }
            break;
        case SOPC_NodeIdEncoding_ByteString:
            nodeId->IdentifierType = SOPC_IdentifierType_ByteString;
            status = SOPC_UInt16_Read(&nodeId->Namespace, buf, nestedStructLevel);
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_ByteString_Read(&nodeId->Data.Bstring, buf, nestedStructLevel);
            }
            break;
        default:
            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
    }

    return status;
}

SOPC_ReturnStatus SOPC_NodeId_ReadAux(void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_NodeId_Read((SOPC_NodeId*) value, buf, nestedStructLevel);
}

SOPC_ReturnStatus SOPC_NodeId_Read(SOPC_NodeId* nodeId, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    SOPC_Byte encodingByte = 0;
    if (NULL == nodeId)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    else if (nestedStructLevel >= SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    nestedStructLevel++;
    return Internal_NodeId_Read(buf, nodeId, &encodingByte, nestedStructLevel);
}

SOPC_ReturnStatus SOPC_ExpandedNodeId_WriteAux(const void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_ExpandedNodeId_Write((const SOPC_ExpandedNodeId*) value, buf, nestedStructLevel);
}

SOPC_ReturnStatus SOPC_ExpandedNodeId_Write(const SOPC_ExpandedNodeId* expNodeId,
                                            SOPC_Buffer* buf,
                                            uint32_t nestedStructLevel)
{
    SOPC_Byte encodingByte = 0xFF;
    if (NULL == expNodeId)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    else if (nestedStructLevel >= SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    nestedStructLevel++;
    encodingByte = (SOPC_Byte) GetNodeIdDataEncoding(&expNodeId->NodeId);
    if (expNodeId->NamespaceUri.Length > 0)
    {
        encodingByte |= SOPC_NodeIdEncoding_NamespaceUriFlag;
    }
    if (expNodeId->ServerIndex > 0)
    {
        encodingByte |= SOPC_NodeIdEncoding_ServerIndexFlag;
    }
    SOPC_ReturnStatus status = Internal_NodeId_Write(buf, encodingByte, &expNodeId->NodeId, nestedStructLevel);

    if (SOPC_STATUS_OK == status && expNodeId->NamespaceUri.Length > 0)
    {
        status = SOPC_String_Write(&expNodeId->NamespaceUri, buf, nestedStructLevel);
    }
    if (SOPC_STATUS_OK == status && expNodeId->ServerIndex > 0)
    {
        status = SOPC_UInt32_Write(&expNodeId->ServerIndex, buf, nestedStructLevel);
    }

    return status;
}

SOPC_ReturnStatus SOPC_ExpandedNodeId_ReadAux(void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_ExpandedNodeId_Read((SOPC_ExpandedNodeId*) value, buf, nestedStructLevel);
}

SOPC_ReturnStatus SOPC_ExpandedNodeId_Read(SOPC_ExpandedNodeId* expNodeId, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    SOPC_Byte encodingByte = 0;
    if (NULL == expNodeId)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    else if (nestedStructLevel >= SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    nestedStructLevel++;
    SOPC_ReturnStatus status = Internal_NodeId_Read(buf, &expNodeId->NodeId, &encodingByte, nestedStructLevel);

    if (SOPC_STATUS_OK == status && (encodingByte & SOPC_NodeIdEncoding_NamespaceUriFlag) != 0x00)
    {
        status = SOPC_String_Read(&expNodeId->NamespaceUri, buf, nestedStructLevel);
    }

    if (SOPC_STATUS_OK == status && (encodingByte & SOPC_NodeIdEncoding_ServerIndexFlag) != 0)
    {
        status = SOPC_UInt32_Read(&expNodeId->ServerIndex, buf, nestedStructLevel);
    }

    if (SOPC_STATUS_OK != status && NULL != expNodeId)
    {
        SOPC_NodeId_Clear(&expNodeId->NodeId);
        SOPC_String_Clear(&expNodeId->NamespaceUri);
        // No clear for last since it should manage it itself in case of failure
    }

    return status;
}

SOPC_ReturnStatus SOPC_StatusCode_WriteAux(const void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_StatusCode_Write((const SOPC_StatusCode*) value, buf, nestedStructLevel);
}

SOPC_ReturnStatus SOPC_StatusCode_Write(const SOPC_StatusCode* status, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_UInt32_Write(status, buf, nestedStructLevel);
}

SOPC_ReturnStatus SOPC_StatusCode_ReadAux(void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_StatusCode_Read((SOPC_StatusCode*) value, buf, nestedStructLevel);
}

SOPC_ReturnStatus SOPC_StatusCode_Read(SOPC_StatusCode* status, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_UInt32_Read(status, buf, nestedStructLevel);
}

static SOPC_Byte GetDiagInfoEncodingByte(const SOPC_DiagnosticInfo* diagInfo)
{
    SOPC_ASSERT(NULL != diagInfo);
    SOPC_Byte encodingByte = 0x00;
    if (diagInfo->SymbolicId > -1)
    {
        encodingByte |= SOPC_DiagInfoEncoding_SymbolicId;
    }
    if (diagInfo->NamespaceUri > -1)
    {
        encodingByte |= SOPC_DiagInfoEncoding_Namespace;
    }
    if (diagInfo->Locale > -1)
    {
        encodingByte |= SOPC_DiagInfoEncoding_Locale;
    }
    if (diagInfo->LocalizedText > -1)
    {
        encodingByte |= SOPC_DiagInfoEncoding_LocalizedTest;
    }
    if (diagInfo->AdditionalInfo.Length > 0)
    {
        encodingByte |= SOPC_DiagInfoEncoding_AdditionalInfo;
    }
    if (diagInfo->InnerStatusCode > 0)
    { // OK status code does not provide information
        encodingByte |= SOPC_DiagInfoEncoding_InnerStatusCode;
    }
    if (NULL != diagInfo->InnerDiagnosticInfo)
    {
        encodingByte |= SOPC_DiagInfoEncoding_InnerDianosticInfo;
    }
    return encodingByte;
}

SOPC_ReturnStatus SOPC_DiagnosticInfo_WriteAux(const void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_DiagnosticInfo_Write((const SOPC_DiagnosticInfo*) value, buf, nestedStructLevel);
}

static SOPC_ReturnStatus SOPC_DiagnosticInfo_Write_Internal(const SOPC_DiagnosticInfo* diagInfo,
                                                            SOPC_Buffer* buf,
                                                            uint32_t nestedLevel)
{
    SOPC_Byte encodingByte = 0x00;
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != diagInfo)
    {
        status = SOPC_STATUS_OK;
        encodingByte = GetDiagInfoEncodingByte(diagInfo);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Byte_Write(&encodingByte, buf, 0);
    }
    if (SOPC_STATUS_OK == status && (encodingByte & SOPC_DiagInfoEncoding_SymbolicId) != 0x00)
    {
        status = SOPC_Int32_Write(&diagInfo->SymbolicId, buf, 0);
    }
    if (SOPC_STATUS_OK == status && (encodingByte & SOPC_DiagInfoEncoding_Namespace) != 0x00)
    {
        status = SOPC_Int32_Write(&diagInfo->NamespaceUri, buf, 0);
    }
    if (SOPC_STATUS_OK == status && (encodingByte & SOPC_DiagInfoEncoding_Locale) != 0x00)
    {
        status = SOPC_Int32_Write(&diagInfo->Locale, buf, 0);
    }
    if (SOPC_STATUS_OK == status && (encodingByte & SOPC_DiagInfoEncoding_LocalizedTest) != 0x00)
    {
        status = SOPC_Int32_Write(&diagInfo->LocalizedText, buf, 0);
    }
    if (SOPC_STATUS_OK == status && (encodingByte & SOPC_DiagInfoEncoding_AdditionalInfo) != 0x00)
    {
        status = SOPC_String_Write(&diagInfo->AdditionalInfo, buf, 0);
    }
    if (SOPC_STATUS_OK == status && (encodingByte & SOPC_DiagInfoEncoding_InnerStatusCode) != 0x00)
    {
        status = SOPC_StatusCode_Write(&diagInfo->InnerStatusCode, buf, 0);
    }
    if (SOPC_STATUS_OK == status && (encodingByte & SOPC_DiagInfoEncoding_InnerDianosticInfo) != 0x00)
    {
        nestedLevel++;
        if (nestedLevel > SOPC_Internal_Common_GetEncodingConstants()->max_nested_diag_info)
        {
            status = SOPC_STATUS_ENCODING_ERROR;
        }
        else
        {
            status = SOPC_DiagnosticInfo_Write_Internal(diagInfo->InnerDiagnosticInfo, buf, nestedLevel);
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_DiagnosticInfo_Write(const SOPC_DiagnosticInfo* diagInfo,
                                            SOPC_Buffer* buf,
                                            uint32_t nestedStructLevel)
{
    if (nestedStructLevel >= SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    nestedStructLevel++;

    // Manage diagnostic information nested level
    return SOPC_DiagnosticInfo_Write_Internal(diagInfo, buf, 0);
}

SOPC_ReturnStatus SOPC_DiagnosticInfo_ReadAux(void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_DiagnosticInfo_Read((SOPC_DiagnosticInfo*) value, buf, nestedStructLevel);
}

static SOPC_ReturnStatus SOPC_DiagnosticInfo_Read_Internal(SOPC_DiagnosticInfo* diagInfo,
                                                           SOPC_Buffer* buf,
                                                           uint32_t nestedLevel)
{
    SOPC_Byte encodingByte = 0x00;
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != diagInfo)
    {
        status = SOPC_Byte_Read(&encodingByte, buf, 0);
    }
    if (SOPC_STATUS_OK == status && (encodingByte & SOPC_DiagInfoEncoding_SymbolicId) != 0x00)
    {
        status = SOPC_Int32_Read(&diagInfo->SymbolicId, buf, 0);
    }
    if (SOPC_STATUS_OK == status && (encodingByte & SOPC_DiagInfoEncoding_Namespace) != 0x00)
    {
        status = SOPC_Int32_Read(&diagInfo->NamespaceUri, buf, 0);
    }
    if (SOPC_STATUS_OK == status && (encodingByte & SOPC_DiagInfoEncoding_Locale) != 0x00)
    {
        status = SOPC_Int32_Read(&diagInfo->Locale, buf, 0);
    }
    if (SOPC_STATUS_OK == status && (encodingByte & SOPC_DiagInfoEncoding_LocalizedTest) != 0x00)
    {
        status = SOPC_Int32_Read(&diagInfo->LocalizedText, buf, 0);
    }
    if (SOPC_STATUS_OK == status && (encodingByte & SOPC_DiagInfoEncoding_AdditionalInfo) != 0x00)
    {
        status = SOPC_String_Read(&diagInfo->AdditionalInfo, buf, 0);
    }
    if (SOPC_STATUS_OK == status && (encodingByte & SOPC_DiagInfoEncoding_InnerStatusCode) != 0x00)
    {
        status = SOPC_StatusCode_Read(&diagInfo->InnerStatusCode, buf, 0);
    }
    if (SOPC_STATUS_OK == status && (encodingByte & SOPC_DiagInfoEncoding_InnerDianosticInfo) != 0x00)
    {
        nestedLevel++;
        if (nestedLevel > SOPC_Internal_Common_GetEncodingConstants()->max_nested_diag_info)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
        else
        {
            diagInfo->InnerDiagnosticInfo = SOPC_Malloc(sizeof(SOPC_DiagnosticInfo));
            if (NULL == diagInfo->InnerDiagnosticInfo)
            {
                status = SOPC_STATUS_OUT_OF_MEMORY;
                SOPC_Free(diagInfo->InnerDiagnosticInfo);
                diagInfo->InnerDiagnosticInfo = NULL;
            }
            else
            {
                SOPC_DiagnosticInfo_Initialize(diagInfo->InnerDiagnosticInfo);
                status = SOPC_DiagnosticInfo_Read_Internal(diagInfo->InnerDiagnosticInfo, buf, nestedLevel);
                if (SOPC_STATUS_OK != status)
                {
                    SOPC_Free(diagInfo->InnerDiagnosticInfo);
                    diagInfo->InnerDiagnosticInfo = NULL;
                }
            }
        }
    }
    if (SOPC_STATUS_OK != status && NULL != diagInfo)
    {
        SOPC_Int32_Clear(&diagInfo->SymbolicId);
        SOPC_Int32_Clear(&diagInfo->NamespaceUri);
        SOPC_Int32_Clear(&diagInfo->Locale);
        SOPC_Int32_Clear(&diagInfo->LocalizedText);
        SOPC_String_Clear(&diagInfo->AdditionalInfo);
        SOPC_StatusCode_Clear(&diagInfo->InnerStatusCode);
        // No clear for last since it should manage it itself in case of failure
    }
    return status;
}

SOPC_ReturnStatus SOPC_DiagnosticInfo_Read(SOPC_DiagnosticInfo* diagInfo, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    // Manage diagnostic information nested level
    if (nestedStructLevel >= SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    nestedStructLevel++;
    return SOPC_DiagnosticInfo_Read_Internal(diagInfo, buf, 0);
}

SOPC_ReturnStatus SOPC_QualifiedName_WriteAux(const void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_QualifiedName_Write((const SOPC_QualifiedName*) value, buf, nestedStructLevel);
}

SOPC_ReturnStatus SOPC_QualifiedName_Write(const SOPC_QualifiedName* qname,
                                           SOPC_Buffer* buf,
                                           uint32_t nestedStructLevel)
{
    if (NULL == qname)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    else if (nestedStructLevel >= SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    nestedStructLevel++;
    SOPC_ReturnStatus status = SOPC_UInt16_Write(&qname->NamespaceIndex, buf, nestedStructLevel);

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_String_Write(&qname->Name, buf, nestedStructLevel);
    }
    return status;
}

SOPC_ReturnStatus SOPC_QualifiedName_ReadAux(void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_QualifiedName_Read((SOPC_QualifiedName*) value, buf, nestedStructLevel);
}

SOPC_ReturnStatus SOPC_QualifiedName_Read(SOPC_QualifiedName* qname, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    if (NULL == qname)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    else if (nestedStructLevel >= SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    nestedStructLevel++;
    SOPC_ReturnStatus status = SOPC_UInt16_Read(&qname->NamespaceIndex, buf, nestedStructLevel);

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_String_Read(&qname->Name, buf, nestedStructLevel);
    }

    if (SOPC_STATUS_OK != status && NULL != qname)
    {
        SOPC_UInt16_Clear(&qname->NamespaceIndex);
        // No clear for last since it should manage it itself in case of failure
    }

    return status;
}

static SOPC_Byte GetLocalizedTextEncodingByte(const SOPC_LocalizedText* ltext)
{
    SOPC_ASSERT(NULL != ltext);
    SOPC_Byte encodingByte = 0;
    if (ltext->defaultLocale.Length > 0)
    {
        encodingByte |= SOPC_LocalizedText_Locale;
    }
    if (ltext->defaultText.Length > 0)
    {
        encodingByte |= SOPC_LocalizedText_Text;
    }
    return encodingByte;
}

SOPC_ReturnStatus SOPC_LocalizedText_WriteAux(const void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_LocalizedText_Write((const SOPC_LocalizedText*) value, buf, nestedStructLevel);
}

SOPC_ReturnStatus SOPC_LocalizedText_Write(const SOPC_LocalizedText* localizedText,
                                           SOPC_Buffer* buf,
                                           uint32_t nestedStructLevel)
{
    SOPC_Byte encodingByte = 0;
    if (NULL == localizedText)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    else if (nestedStructLevel >= SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    nestedStructLevel++;
    encodingByte = GetLocalizedTextEncodingByte(localizedText);
    SOPC_ReturnStatus status = SOPC_Byte_Write(&encodingByte, buf, nestedStructLevel);

    if (SOPC_STATUS_OK == status && (encodingByte & SOPC_LocalizedText_Locale) != 0)
    {
        status = SOPC_String_Write(&localizedText->defaultLocale, buf, nestedStructLevel);
    }
    if (SOPC_STATUS_OK == status && (encodingByte & SOPC_LocalizedText_Text) != 0)
    {
        status = SOPC_String_Write(&localizedText->defaultText, buf, nestedStructLevel);
    }
    return status;
}

SOPC_ReturnStatus SOPC_LocalizedText_ReadAux(void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_LocalizedText_Read((SOPC_LocalizedText*) value, buf, nestedStructLevel);
}

SOPC_ReturnStatus SOPC_LocalizedText_Read(SOPC_LocalizedText* localizedText,
                                          SOPC_Buffer* buf,
                                          uint32_t nestedStructLevel)
{
    SOPC_Byte encodingByte = 0;
    if (NULL == localizedText)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    else if (nestedStructLevel >= SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    nestedStructLevel++;
    SOPC_ReturnStatus status = SOPC_Byte_Read(&encodingByte, buf, nestedStructLevel);

    if (SOPC_STATUS_OK == status && (encodingByte & SOPC_LocalizedText_Locale) != 0)
    {
        status = SOPC_String_Read(&localizedText->defaultLocale, buf, nestedStructLevel);
    }
    if (SOPC_STATUS_OK == status && (encodingByte & SOPC_LocalizedText_Text) != 0)
    {
        status = SOPC_String_Read(&localizedText->defaultText, buf, nestedStructLevel);
    }

    if (SOPC_STATUS_OK != status && NULL != localizedText)
    {
        SOPC_String_Clear(&localizedText->defaultLocale);
        // No clear for last since it should manage it itself in case of failure
    }

    return status;
}

SOPC_ReturnStatus SOPC_ExtensionObject_WriteAux(const void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_ExtensionObject_Write((const SOPC_ExtensionObject*) value, buf, nestedStructLevel);
}

SOPC_ReturnStatus SOPC_ExtensionObject_Write(const SOPC_ExtensionObject* extObj,
                                             SOPC_Buffer* buf,
                                             uint32_t nestedStructLevel)
{
    const int32_t tmpLength = -1;
    SOPC_NodeId nodeId;
    uint32_t lengthPos;
    uint32_t curPos;
    int32_t length;
    SOPC_Byte encodingByte = 0;
    if (NULL == extObj)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    else if (nestedStructLevel >= SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    nestedStructLevel++;
    encodingByte = (SOPC_Byte) extObj->Encoding;
    nodeId = extObj->TypeId.NodeId;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (SOPC_STATUS_OK == status && encodingByte == SOPC_ExtObjBodyEncoding_Object)
    {
        encodingByte = SOPC_ExtObjBodyEncoding_ByteString;
        if (NULL == extObj->Body.Object.ObjType)
        {
            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
        else
        {
            nodeId.IdentifierType = SOPC_IdentifierType_Numeric;
            SOPC_ASSERT(NULL == extObj->Body.Object.ObjType->NamespaceUri &&
                        "EncType Namespace URI translation unsupported");
            nodeId.Namespace = extObj->Body.Object.ObjType->NamespaceIndex;
            nodeId.Data.Numeric = extObj->Body.Object.ObjType->BinaryEncodingTypeId;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_NodeId_Write(&nodeId, buf, nestedStructLevel);
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Byte_Write(&encodingByte, buf, nestedStructLevel);
    }

    if (SOPC_STATUS_OK == status)
    {
        switch (extObj->Encoding)
        {
        case SOPC_ExtObjBodyEncoding_None:
            break;
        case SOPC_ExtObjBodyEncoding_ByteString:
            status = SOPC_ByteString_Write(&extObj->Body.Bstring, buf, nestedStructLevel);
            break;
        case SOPC_ExtObjBodyEncoding_XMLElement:
            status = SOPC_XmlElement_Write(&extObj->Body.Xml, buf, nestedStructLevel);
            break;
        case SOPC_ExtObjBodyEncoding_Object:
            lengthPos = buf->position;
            status = SOPC_Int32_Write(&tmpLength, buf, nestedStructLevel);
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_EncodeableObject_Encode(extObj->Body.Object.ObjType, extObj->Body.Object.Value, buf,
                                                      nestedStructLevel);
            }
            if (SOPC_STATUS_OK == status)
            {
                curPos = buf->position;
                if (INT32_MAX >= curPos - (lengthPos + 4))
                {
                    // Go backward to write correct length value
                    length = (int32_t)(curPos - (lengthPos + 4));
                    SOPC_Buffer_SetPosition(buf, lengthPos);
                    SOPC_Int32_Write(&length, buf, nestedStructLevel);
                    SOPC_Buffer_SetPosition(buf, curPos);
                }
            }
            break;
        default:
            break;
        }
    }

    return status;
}

SOPC_ReturnStatus SOPC_ExtensionObject_ReadAux(void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_ExtensionObject_Read((SOPC_ExtensionObject*) value, buf, nestedStructLevel);
}

SOPC_ReturnStatus SOPC_ExtensionObject_Read(SOPC_ExtensionObject* extObj, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    SOPC_EncodeableType* encType = NULL;
    SOPC_Byte encodingByte = 0;
    if (NULL == extObj)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    else if (nestedStructLevel >= SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    nestedStructLevel++;
    SOPC_ReturnStatus status = SOPC_NodeId_Read(&extObj->TypeId.NodeId, buf, nestedStructLevel);

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Byte_Read(&encodingByte, buf, nestedStructLevel);
    }

    if (SOPC_STATUS_OK == status)
    {
        // Manage Object body decoding
        if (encodingByte == SOPC_ExtObjBodyEncoding_ByteString)
        {
            // Object provided as a byte string, check if encoded object is a known type
            if (extObj->TypeId.NodeId.IdentifierType == SOPC_IdentifierType_Numeric)
            {
                encType = SOPC_EncodeableType_GetEncodeableType(extObj->TypeId.NodeId.Namespace,
                                                                extObj->TypeId.NodeId.Data.Numeric);
                if (NULL != encType)
                {
                    encodingByte = SOPC_ExtObjBodyEncoding_Object;
                    extObj->Body.Object.ObjType = encType;
                    SOPC_String_AttachFromCstring(&extObj->TypeId.NamespaceUri, encType->NamespaceUri);
                } // else: Keep it as a byte string since we do not know its type
            }     // else: Keep it as a byte string since we do not know its type
        }
        else if (encodingByte == SOPC_ExtObjBodyEncoding_Object)
        {
            // Invalid value encoded, it does not exist as a valid binary encoding value
            status = SOPC_STATUS_ENCODING_ERROR;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        switch (encodingByte)
        {
        case SOPC_ExtObjBodyEncoding_None:
            extObj->Length = -1;
            break;
        case SOPC_ExtObjBodyEncoding_XMLElement:
            status = SOPC_XmlElement_Read(&extObj->Body.Xml, buf, nestedStructLevel);
            break;
        case SOPC_ExtObjBodyEncoding_ByteString:
            status = SOPC_ByteString_Read(&extObj->Body.Bstring, buf, nestedStructLevel);
            break;
        case SOPC_ExtObjBodyEncoding_Object:
            status = SOPC_Int32_Read(&extObj->Length, buf, nestedStructLevel);
            if (SOPC_STATUS_OK == status)
            {
                /* Allocation size value comes from types defined in Toolkit and is considered as not excessive
                 * value */
                extObj->Body.Object.Value = SOPC_Malloc(extObj->Body.Object.ObjType->AllocationSize);
                if (NULL != extObj->Body.Object.Value)
                {
                    extObj->Body.Object.ObjType->Initialize(extObj->Body.Object.Value);
                    status = SOPC_EncodeableObject_Decode(extObj->Body.Object.ObjType, extObj->Body.Object.Value, buf,
                                                          nestedStructLevel);
                    if (SOPC_STATUS_OK != status)
                    {
                        SOPC_Free(extObj->Body.Object.Value);
                        extObj->Body.Object.Value = NULL;
                    }
                }
                else
                {
                    status = SOPC_STATUS_OUT_OF_MEMORY;
                }
            }
            break;
        default:
            status = SOPC_STATUS_ENCODING_ERROR;
        }
        if (SOPC_STATUS_OK == status)
        {
            extObj->Encoding = encodingByte;
        }
    }

    if (SOPC_STATUS_OK != status && NULL != extObj)
    {
        SOPC_NodeId_Clear(&extObj->TypeId.NodeId);
        // No clear for last since it should manage it itself in case of failure
    }

    return status;
}

static SOPC_Byte GetVariantEncodingMask(const SOPC_Variant* variant)
{
    SOPC_ASSERT(NULL != variant);
    SOPC_Byte encodingByte = (SOPC_Byte) variant->BuiltInTypeId;
    if (variant->ArrayType == SOPC_VariantArrayType_Matrix)
    {
        encodingByte |= SOPC_VariantArrayValueFlag;
        encodingByte |= SOPC_VariantArrayDimensionsFlag;
    }
    if (variant->ArrayType == SOPC_VariantArrayType_Array)
    {
        encodingByte |= SOPC_VariantArrayValueFlag;
    }
    return encodingByte;
}

static SOPC_ReturnStatus WriteVariantNonArrayBuiltInType(SOPC_Buffer* buf,
                                                         SOPC_BuiltinId builtInTypeId,
                                                         const SOPC_VariantValue* val,
                                                         uint32_t nestedStructLevel)
{
    if (nestedStructLevel >= SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    // nestedStructLevel is not incremented here
    // it is incremented in sub-functions
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;

    switch (builtInTypeId)
    {
    case SOPC_Boolean_Id:
        status = SOPC_Boolean_Write(&val->Boolean, buf, nestedStructLevel);
        break;
    case SOPC_SByte_Id:
        status = SOPC_SByte_Write(&val->Sbyte, buf, nestedStructLevel);
        break;
    case SOPC_Byte_Id:
        status = SOPC_Byte_Write(&val->Byte, buf, nestedStructLevel);
        break;
    case SOPC_Int16_Id:
        status = SOPC_Int16_Write(&val->Int16, buf, nestedStructLevel);
        break;
    case SOPC_UInt16_Id:
        status = SOPC_UInt16_Write(&val->Uint16, buf, nestedStructLevel);
        break;
    case SOPC_Int32_Id:
        status = SOPC_Int32_Write(&val->Int32, buf, nestedStructLevel);
        break;
    case SOPC_UInt32_Id:
        status = SOPC_UInt32_Write(&val->Uint32, buf, nestedStructLevel);
        break;
    case SOPC_Int64_Id:
        status = SOPC_Int64_Write(&val->Int64, buf, nestedStructLevel);
        break;
    case SOPC_UInt64_Id:
        status = SOPC_UInt64_Write(&val->Uint64, buf, nestedStructLevel);
        break;
    case SOPC_Float_Id:
        status = SOPC_Float_Write(&val->Floatv, buf, nestedStructLevel);
        break;
    case SOPC_Double_Id:
        status = SOPC_Double_Write(&val->Doublev, buf, nestedStructLevel);
        break;
    case SOPC_String_Id:
        status = SOPC_String_Write(&val->String, buf, nestedStructLevel);
        break;
    case SOPC_DateTime_Id:
        status = SOPC_DateTime_Write(&val->Date, buf, nestedStructLevel);
        break;
    case SOPC_Guid_Id:
        status = SOPC_Guid_Write(val->Guid, buf, nestedStructLevel);
        break;
    case SOPC_ByteString_Id:
        status = SOPC_ByteString_Write(&val->Bstring, buf, nestedStructLevel);
        break;
    case SOPC_XmlElement_Id:
        status = SOPC_XmlElement_Write(&val->XmlElt, buf, nestedStructLevel);
        break;
    case SOPC_NodeId_Id:
        status = SOPC_NodeId_Write(val->NodeId, buf, nestedStructLevel);
        break;
    case SOPC_ExpandedNodeId_Id:
        status = SOPC_ExpandedNodeId_Write(val->ExpNodeId, buf, nestedStructLevel);
        break;
    case SOPC_StatusCode_Id:
        status = SOPC_StatusCode_Write(&val->Status, buf, nestedStructLevel);
        break;
    case SOPC_QualifiedName_Id:
        status = SOPC_QualifiedName_Write(val->Qname, buf, nestedStructLevel);
        break;
    case SOPC_LocalizedText_Id:
        status = SOPC_LocalizedText_Write(val->LocalizedText, buf, nestedStructLevel);
        break;
    case SOPC_ExtensionObject_Id:
        status = SOPC_ExtensionObject_Write(val->ExtObject, buf, nestedStructLevel);
        break;
    case SOPC_DataValue_Id:
        status = SOPC_DataValue_WriteAux_Nested((void*) val->DataValue, buf, nestedStructLevel);
        break;
    case SOPC_Variant_Id:
        // Part 6 Table 14 (v1.03): "The value shall not be a Variant
        //                           but it could be an array of Variants."
        status = SOPC_STATUS_INVALID_PARAMETERS;
        break;
    case SOPC_DiagnosticInfo_Id:
        status = SOPC_DiagnosticInfo_Write(val->DiagInfo, buf, nestedStructLevel);
        break;
    case SOPC_Null_Id:
        // mantis #0003682: errata for 1.03: NULL value encoding (no value to encode)
        status = SOPC_STATUS_OK;
        break;
    default:
        // SOPC_STATUS = INVALID PARAM
        break;
    }
    return status;
}

// 0 Means not optimizable and other values represent the size in byte of each element to copy
static size_t getBuiltinOptimizableSize(SOPC_BuiltinId builtInTypeId)
{
    if (builtInTypeId <= 0 || builtInTypeId > SOPC_BUILTINID_MAX)
    {
        return 0;
    }
    switch (builtInTypeId)
    {
    case SOPC_Byte_Id:
    case SOPC_SByte_Id:
    case SOPC_Boolean_Id:
    case SOPC_UInt16_Id:
    case SOPC_Int16_Id:
    case SOPC_Int32_Id:
    case SOPC_UInt32_Id:
    case SOPC_Int64_Id:
    case SOPC_UInt64_Id:
    case SOPC_DateTime_Id:
    case SOPC_StatusCode_Id:
    case SOPC_Float_Id:
        return (SOPC_IS_LITTLE_ENDIAN ? SOPC_BuiltInType_HandlingTable[builtInTypeId].size : 0);
    case SOPC_Double_Id:
        return (SOPC_IS_LITTLE_ENDIAN && (!SOPC_IS_DOUBLE_MIDDLE_ENDIAN)
                    ? SOPC_BuiltInType_HandlingTable[builtInTypeId].size
                    : 0);
    default:
        return 0;
    }
}

static SOPC_ReturnStatus WriteVariantArrayBuiltInType(SOPC_Buffer* buf,
                                                      SOPC_BuiltinId builtInTypeId,
                                                      const SOPC_VariantArrayValue* array,
                                                      int32_t* length,
                                                      uint32_t nestedStructLevel)
{
    if (nestedStructLevel >= SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct)
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    else if (builtInTypeId <= 0 || builtInTypeId > SOPC_BUILTINID_MAX)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    const size_t eltOptimSize = getBuiltinOptimizableSize(builtInTypeId);
    if (eltOptimSize > 0)
    {
        // Note : union fields content are all pointing to the same address.
        // using array->BooleanArr to point array, but any other field would be possible
        if (NULL == buf || NULL == length || NULL == array || (*length > 0 && NULL == array->BooleanArr))
        {
            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
        else
        {
            nestedStructLevel++;
            status = SOPC_Int32_Write(length, buf, nestedStructLevel);
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_Buffer_Write(buf, array->BooleanArr, (uint32_t)((uint32_t)(*length) * eltOptimSize));
            }
        }
    }
    else
    {
        // Note : union fields content are all pointing to the same address.
        // using array->BooleanArr to point array, but any other field would be possible
        status = SOPC_Write_Array(buf, length, (const void* const*) &array->BooleanArr,
                                  SOPC_BuiltInType_HandlingTable[builtInTypeId].size,
                                  SOPC_BuiltInType_EncodingTable[builtInTypeId].encode, nestedStructLevel);
    }
    return status;
}

SOPC_ReturnStatus SOPC_Variant_WriteAux(const void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_Variant_Write((const SOPC_Variant*) value, buf, nestedStructLevel);
}

static SOPC_ReturnStatus SOPC_Variant_Write_Internal(const SOPC_Variant* variant,
                                                     SOPC_Buffer* buf,
                                                     uint32_t nestedStructLevel)
{
    SOPC_Byte encodingByte = 0;
    int32_t arrayLength = 0;
    int64_t matrixLength = 1;
    int32_t idx = 0;

    const uint32_t max_nested_struct_level = SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct;

    if (NULL == variant)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    else if (nestedStructLevel >= max_nested_struct_level)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    nestedStructLevel++;
    encodingByte = GetVariantEncodingMask(variant);
    SOPC_ReturnStatus status = SOPC_Byte_Write(&encodingByte, buf, nestedStructLevel);

    if (SOPC_STATUS_OK == status)
    {
        switch (variant->ArrayType)
        {
        case SOPC_VariantArrayType_SingleValue:
            status = WriteVariantNonArrayBuiltInType(buf, variant->BuiltInTypeId, &variant->Value, nestedStructLevel);
            break;
        case SOPC_VariantArrayType_Array:
            arrayLength = variant->Value.Array.Length;
            // Note: array length written in WriteVariantArrayBuiltInType
            if (SOPC_STATUS_OK == status)
            {
                if (arrayLength == -1)
                {
                    // Encode NULL arrays as empty
                    arrayLength = 0;
                }
                if (arrayLength < 0)
                {
                    status = SOPC_STATUS_ENCODING_ERROR;
                }
                else
                {
                    status = WriteVariantArrayBuiltInType(buf, variant->BuiltInTypeId, &variant->Value.Array.Content,
                                                          &arrayLength, nestedStructLevel);
                }
            }
            break;
        case SOPC_VariantArrayType_Matrix:
            matrixLength = 1;
            if (variant->Value.Matrix.Dimensions == 0)
            {
                matrixLength = 0;
            }
            for (idx = 0; idx < variant->Value.Matrix.Dimensions && SOPC_STATUS_OK == status; idx++)
            {
                if (variant->Value.Matrix.ArrayDimensions[idx] > 0 &&
                    matrixLength * variant->Value.Matrix.ArrayDimensions[idx] <= INT32_MAX)
                {
                    matrixLength *= variant->Value.Matrix.ArrayDimensions[idx];
                }
                else
                {
                    status = SOPC_STATUS_ENCODING_ERROR;
                }
            }
            // Note: array length written in WriteVariantArrayBuiltInType
            if (SOPC_STATUS_OK == status)
            {
                arrayLength = (int32_t) matrixLength;
                status = WriteVariantArrayBuiltInType(buf, variant->BuiltInTypeId, &variant->Value.Matrix.Content,
                                                      &arrayLength, nestedStructLevel);
            }
            // Encode dimension array
            if (SOPC_STATUS_OK == status)
            {
                // length
                status = SOPC_Int32_Write(&variant->Value.Matrix.Dimensions, buf, nestedStructLevel);
            }
            if (SOPC_STATUS_OK == status)
            {
                // array
                for (idx = 0; idx < variant->Value.Matrix.Dimensions; idx++)
                {
                    status = SOPC_Int32_Write(&variant->Value.Matrix.ArrayDimensions[idx], buf, nestedStructLevel);
                }
            }
            break;
        default:
            status = SOPC_STATUS_ENCODING_ERROR;
            break;
        }
    }

    return status;
}

SOPC_ReturnStatus SOPC_Variant_WriteAux_Nested(const void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_Variant_Write_Internal((const SOPC_Variant*) value, buf, nestedStructLevel);
}

SOPC_ReturnStatus SOPC_Variant_Write(const SOPC_Variant* variant, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    if (nestedStructLevel >= SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    // Manage variant nested level (data value could be contained in a variant)
    return SOPC_Variant_Write_Internal(variant, buf, nestedStructLevel);
}

static SOPC_ReturnStatus ReadVariantNonArrayBuiltInType(SOPC_Buffer* buf,
                                                        SOPC_BuiltinId builtInTypeId,
                                                        SOPC_VariantValue* val,
                                                        uint32_t nestedStructLevel)
{
    if (nestedStructLevel >= SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    // nestedStructLevel is handled by subcalls
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;

    switch (builtInTypeId)
    {
    case SOPC_Boolean_Id:
        status = SOPC_Boolean_Read(&val->Boolean, buf, nestedStructLevel);
        break;
    case SOPC_SByte_Id:
        status = SOPC_SByte_Read(&val->Sbyte, buf, nestedStructLevel);
        break;
    case SOPC_Byte_Id:
        status = SOPC_Byte_Read(&val->Byte, buf, nestedStructLevel);
        break;
    case SOPC_Int16_Id:
        status = SOPC_Int16_Read(&val->Int16, buf, nestedStructLevel);
        break;
    case SOPC_UInt16_Id:
        status = SOPC_UInt16_Read(&val->Uint16, buf, nestedStructLevel);
        break;
    case SOPC_Int32_Id:
        status = SOPC_Int32_Read(&val->Int32, buf, nestedStructLevel);
        break;
    case SOPC_UInt32_Id:
        status = SOPC_UInt32_Read(&val->Uint32, buf, nestedStructLevel);
        break;
    case SOPC_Int64_Id:
        status = SOPC_Int64_Read(&val->Int64, buf, nestedStructLevel);
        break;
    case SOPC_UInt64_Id:
        status = SOPC_UInt64_Read(&val->Uint64, buf, nestedStructLevel);
        break;
    case SOPC_Float_Id:
        status = SOPC_Float_Read(&val->Floatv, buf, nestedStructLevel);
        break;
    case SOPC_Double_Id:
        status = SOPC_Double_Read(&val->Doublev, buf, nestedStructLevel);
        break;
    case SOPC_String_Id:
        status = SOPC_String_Read(&val->String, buf, nestedStructLevel);
        break;
    case SOPC_DateTime_Id:
        status = SOPC_DateTime_Read(&val->Date, buf, nestedStructLevel);
        break;
    case SOPC_Guid_Id:
        val->Guid = SOPC_Malloc(sizeof(SOPC_Guid));
        if (NULL != val->Guid)
        {
            SOPC_Guid_Initialize(val->Guid);
            status = SOPC_Guid_Read(val->Guid, buf, nestedStructLevel);
            if (SOPC_STATUS_OK != status)
            {
                SOPC_Free(val->Guid);
                val->Guid = NULL;
            }
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
        break;
    case SOPC_ByteString_Id:
        status = SOPC_ByteString_Read(&val->Bstring, buf, nestedStructLevel);
        break;
    case SOPC_XmlElement_Id:
        status = SOPC_XmlElement_Read(&val->XmlElt, buf, nestedStructLevel);
        break;
    case SOPC_NodeId_Id:
        val->NodeId = SOPC_Malloc(sizeof(SOPC_NodeId));
        if (NULL != val->NodeId)
        {
            SOPC_NodeId_Initialize(val->NodeId);
            status = SOPC_NodeId_Read(val->NodeId, buf, nestedStructLevel);
            if (SOPC_STATUS_OK != status)
            {
                SOPC_Free(val->NodeId);
                val->NodeId = NULL;
            }
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
        break;
    case SOPC_ExpandedNodeId_Id:
        val->ExpNodeId = SOPC_Malloc(sizeof(SOPC_ExpandedNodeId));
        if (NULL != val->ExpNodeId)
        {
            SOPC_ExpandedNodeId_Initialize(val->ExpNodeId);
            status = SOPC_ExpandedNodeId_Read(val->ExpNodeId, buf, nestedStructLevel);
            if (SOPC_STATUS_OK != status)
            {
                SOPC_Free(val->ExpNodeId);
                val->ExpNodeId = NULL;
            }
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
        break;
    case SOPC_StatusCode_Id:
        status = SOPC_StatusCode_Read(&val->Status, buf, nestedStructLevel);
        break;
    case SOPC_QualifiedName_Id:
        val->Qname = SOPC_Malloc(sizeof(SOPC_QualifiedName));
        if (NULL != val->Qname)
        {
            SOPC_QualifiedName_Initialize(val->Qname);
            status = SOPC_QualifiedName_Read(val->Qname, buf, nestedStructLevel);
            if (SOPC_STATUS_OK != status)
            {
                SOPC_Free(val->Qname);
                val->Qname = NULL;
            }
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
        break;
    case SOPC_LocalizedText_Id:
        val->LocalizedText = SOPC_Malloc(sizeof(SOPC_LocalizedText));
        if (NULL != val->LocalizedText)
        {
            SOPC_LocalizedText_Initialize(val->LocalizedText);
            status = SOPC_LocalizedText_Read(val->LocalizedText, buf, nestedStructLevel);
            if (SOPC_STATUS_OK != status)
            {
                SOPC_Free(val->LocalizedText);
                val->LocalizedText = NULL;
            }
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
        break;
    case SOPC_ExtensionObject_Id:
        val->ExtObject = SOPC_Malloc(sizeof(SOPC_ExtensionObject));
        if (NULL != val->ExtObject)
        {
            SOPC_ExtensionObject_Initialize(val->ExtObject);
            status = SOPC_ExtensionObject_Read(val->ExtObject, buf, nestedStructLevel);
            if (SOPC_STATUS_OK != status)
            {
                SOPC_Free(val->ExtObject);
                val->ExtObject = NULL;
            }
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
        break;
    case SOPC_DataValue_Id:
        val->DataValue = SOPC_Malloc(sizeof(SOPC_DataValue));
        if (NULL != val->DataValue)
        {
            SOPC_DataValue_Initialize(val->DataValue);
            status = SOPC_DataValue_ReadAux_Nested((void*) val->DataValue, buf, nestedStructLevel);
            if (SOPC_STATUS_OK != status)
            {
                SOPC_Free(val->DataValue);
                val->DataValue = NULL;
            }
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
        break;
    case SOPC_Variant_Id:
        // Part 6 Table 14 (v1.03): "The value shall not be a Variant
        //                           but it could be an array of Variants."
        status = SOPC_STATUS_ENCODING_ERROR;
        break;
    case SOPC_DiagnosticInfo_Id:
        val->DiagInfo = SOPC_Malloc(sizeof(SOPC_DiagnosticInfo));
        if (NULL != val->DiagInfo)
        {
            SOPC_DiagnosticInfo_Initialize(val->DiagInfo);
            status = SOPC_DiagnosticInfo_Read(val->DiagInfo, buf, nestedStructLevel);
            if (SOPC_STATUS_OK != status)
            {
                SOPC_DiagnosticInfo_Clear(val->DiagInfo);
                SOPC_Free(val->DiagInfo);
                val->DiagInfo = NULL;
            }
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
        break;
    case SOPC_Null_Id:
        // mantis #0003682: errata for 1.03: NULL value encoding (no value to decode)
        status = SOPC_STATUS_OK;
        break;
    default:
        status = SOPC_STATUS_NOK;
        break;
    }
    return status;
}

static SOPC_ReturnStatus ReadVariantArrayBuiltInType(SOPC_Buffer* buf,
                                                     SOPC_BuiltinId builtInTypeId,
                                                     SOPC_VariantArrayValue* array,
                                                     int32_t* length,
                                                     uint32_t nestedStructLevel)
{
    if (nestedStructLevel >= SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct)
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    else if (builtInTypeId <= 0 || builtInTypeId > SOPC_BUILTINID_MAX)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    const size_t eltOptimSize = getBuiltinOptimizableSize(builtInTypeId);
    if (eltOptimSize > 0)
    {
        if (NULL == buf || NULL == length || NULL == array || NULL != array->BooleanArr)
        {
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
        else if (nestedStructLevel >= SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct)
        {
            return SOPC_STATUS_INVALID_STATE;
        }

        nestedStructLevel++;
        status = SOPC_Int32_Read(length, buf, nestedStructLevel);

        if (SOPC_STATUS_OK == status && *length < 0)
        {
            *length = 0;
        }

        if (SOPC_STATUS_OK == status && *length > SOPC_Internal_Common_GetEncodingConstants()->max_array_length)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }

        if (SOPC_STATUS_OK == status && *length > 0 && (uint64_t) *length <= SIZE_MAX / eltOptimSize)
        {
            array->BooleanArr = SOPC_Calloc((size_t) *length, eltOptimSize);

            if (NULL == array->BooleanArr)
            {
                status = SOPC_STATUS_OUT_OF_MEMORY;
            }
            else
            {
                status = SOPC_Buffer_Read(array->BooleanArr, buf, (uint32_t)((uint32_t)(*length) * eltOptimSize));
                if (SOPC_STATUS_OK != status)
                {
                    status = SOPC_STATUS_ENCODING_ERROR;
                }
            }

            if (SOPC_STATUS_OK != status)
            {
                SOPC_Free(array->BooleanArr);
                array->BooleanArr = NULL;
                *length = 0;
            }
        }
    }
    else
    {
        // Note : union fields content are all pointing to the same address.
        // using array->BooleanArr to point array, but any other field would be possible
        status = SOPC_Read_Array(buf, length, (void**) &array->BooleanArr,
                                 SOPC_BuiltInType_HandlingTable[builtInTypeId].size,
                                 SOPC_BuiltInType_EncodingTable[builtInTypeId].decode,
                                 SOPC_BuiltInType_HandlingTable[builtInTypeId].initialize,
                                 SOPC_BuiltInType_HandlingTable[builtInTypeId].clear, nestedStructLevel);
    }
    return status;
}

SOPC_ReturnStatus SOPC_Variant_ReadAux(void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_Variant_Read((SOPC_Variant*) value, buf, nestedStructLevel);
}

static SOPC_ReturnStatus SOPC_Variant_Read_Internal(SOPC_Variant* variant, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    SOPC_Byte encodingByte = 0;
    int32_t arrayLength = 0;
    int64_t matrixLength = 1;

    const SOPC_Common_EncodingConstants* encCfg = SOPC_Internal_Common_GetEncodingConstants();

    if (NULL == variant)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    else if (nestedStructLevel >= encCfg->max_nested_struct)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    nestedStructLevel++;
    SOPC_ReturnStatus status = SOPC_Byte_Read(&encodingByte, buf, nestedStructLevel);

    if (SOPC_STATUS_OK == status)
    {
        // Retrieve array flags
        if ((encodingByte & SOPC_VariantArrayValueFlag) != 0)
        {
            if ((encodingByte & SOPC_VariantArrayDimensionsFlag) != 0)
            {
                variant->ArrayType = SOPC_VariantArrayType_Matrix;
            }
            else
            {
                variant->ArrayType = SOPC_VariantArrayType_Array;
            }
            // Note array length read in ReadVariantArrayBuiltInType
        }
        else if ((encodingByte & SOPC_VariantArrayDimensionsFlag) != 0)
        {
            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
        else
        {
            variant->ArrayType = SOPC_VariantArrayType_SingleValue;
        }
        // Retrieve builtin type id: avoid 2^7 and 2^6 which are array flags
        variant->BuiltInTypeId = 0x3F & encodingByte;
    }

    if (SOPC_STATUS_OK == status)
    {
        switch (variant->ArrayType)
        {
        case SOPC_VariantArrayType_SingleValue:
            status = ReadVariantNonArrayBuiltInType(buf, variant->BuiltInTypeId, &variant->Value, nestedStructLevel);
            break;
        case SOPC_VariantArrayType_Array:
            status = ReadVariantArrayBuiltInType(buf, variant->BuiltInTypeId, &variant->Value.Array.Content,
                                                 &arrayLength, nestedStructLevel);
            variant->Value.Array.Length = arrayLength;
            break;
        case SOPC_VariantArrayType_Matrix:
            status = ReadVariantArrayBuiltInType(buf, variant->BuiltInTypeId, &variant->Value.Matrix.Content,
                                                 &arrayLength, nestedStructLevel);

            if (SOPC_STATUS_OK == status && arrayLength < 0)
            {
                status = SOPC_STATUS_ENCODING_ERROR;
            }

            // Decode dimension array
            if (SOPC_STATUS_OK == status)
            {
                // length of array
                status = SOPC_Int32_Read(&variant->Value.Matrix.Dimensions, buf, nestedStructLevel);
            }

            if (SOPC_STATUS_OK == status &&
                (variant->Value.Matrix.Dimensions < 0 || variant->Value.Matrix.Dimensions > encCfg->max_array_length ||
                 (uint64_t) variant->Value.Matrix.Dimensions * 1 > SIZE_MAX))
            {
                status = SOPC_STATUS_OUT_OF_MEMORY;
            }

            if (SOPC_STATUS_OK == status)
            {
                // array
                variant->Value.Matrix.ArrayDimensions =
                    SOPC_Malloc(sizeof(int32_t) * (size_t) variant->Value.Matrix.Dimensions);
                if (variant->Value.Matrix.Dimensions == 0)
                {
                    matrixLength = 0;
                }
                if (NULL != variant->Value.Matrix.ArrayDimensions)
                {
                    int32_t idx = 0;
                    for (idx = 0; idx < variant->Value.Matrix.Dimensions && SOPC_STATUS_OK == status; idx++)
                    {
                        status = SOPC_Int32_Read(&variant->Value.Matrix.ArrayDimensions[idx], buf, nestedStructLevel);
                        if (SOPC_STATUS_OK == status && variant->Value.Matrix.ArrayDimensions[idx] > 0)
                        {
                            // valid value
                            matrixLength *= variant->Value.Matrix.ArrayDimensions[idx];
                            if (matrixLength > arrayLength)
                            {
                                status = SOPC_STATUS_ENCODING_ERROR;
                            }
                        }
                        else
                        {
                            status = SOPC_STATUS_ENCODING_ERROR;
                        }
                    }
                    if (SOPC_STATUS_OK != status)
                    {
                        SOPC_Free(variant->Value.Matrix.ArrayDimensions);
                        variant->Value.Matrix.ArrayDimensions = NULL;
                    }
                }
                else
                {
                    status = SOPC_STATUS_NOK;
                }
            }

            if (SOPC_STATUS_OK == status && matrixLength != arrayLength)
            {
                // Dimensions length and total length are not equal
                status = SOPC_STATUS_ENCODING_ERROR;
            }

            if (SOPC_STATUS_OK != status)
            {
                if (arrayLength > 0)
                {
                    // manage cases in which Dimensions or ArrayDimensions are not correctly
                    // decoded. Force to use only arrayLength value to ensure correct cleaning
                    // of the matrix content.
                    variant->Value.Matrix.Dimensions = 1;
                    int32_t* arrayDim = SOPC_Calloc(1, sizeof(int32_t));
                    if (NULL != arrayDim)
                    {
                        arrayDim[0] = arrayLength;
                        /* free if memory was previously allocated */
                        if (NULL != variant->Value.Matrix.ArrayDimensions)
                        {
                            SOPC_Free(variant->Value.Matrix.ArrayDimensions);
                        }
                        variant->Value.Matrix.ArrayDimensions = arrayDim;
                    }
                    else
                    {
                        status = SOPC_STATUS_OUT_OF_MEMORY;
                    }
                }
                else
                {
                    variant->Value.Matrix.Dimensions = 0;
                }
            }

            if (SOPC_STATUS_OK != status)
            {
                SOPC_Variant_Clear(variant);
            }

            break;
        default:
            status = SOPC_STATUS_ENCODING_ERROR;
            break;
        }
    }

    if (SOPC_STATUS_OK != status && NULL != variant)
    {
        variant->BuiltInTypeId = SOPC_Null_Id; // encoding byte read assignement
        SOPC_Int32_Clear(&variant->Value.Matrix.Dimensions);
    }

    return status;
}

SOPC_ReturnStatus SOPC_Variant_ReadAux_Nested(void* value, SOPC_Buffer* buf, uint32_t nestedLevel)
{
    return SOPC_Variant_Read_Internal((SOPC_Variant*) value, buf, nestedLevel);
}

SOPC_ReturnStatus SOPC_Variant_Read(SOPC_Variant* variant, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_Variant_Read_Internal(variant, buf, nestedStructLevel);
}

static SOPC_Byte GetDataValueEncodingMask(const SOPC_DataValue* dataValue)
{
    SOPC_ASSERT(NULL != dataValue);
    SOPC_Byte mask = 0;
    if (dataValue->Value.BuiltInTypeId != SOPC_Null_Id && dataValue->Value.BuiltInTypeId <= SOPC_BUILTINID_MAX)
    {
        mask |= SOPC_DataValue_NotNullValue;
    }
    if (SOPC_STATUS_OK != dataValue->Status)
    {
        mask |= SOPC_DataValue_NotGoodStatusCode;
    }
    if (dataValue->SourceTimestamp > 0)
    {
        mask |= SOPC_DataValue_NotMinSourceDate;
    }
    if (dataValue->SourcePicoSeconds > 0)
    {
        mask |= SOPC_DataValue_NotZeroSourcePico;
    }
    if (dataValue->ServerTimestamp > 0)
    {
        mask |= SOPC_DataValue_NotMinServerDate;
    }
    if (dataValue->ServerPicoSeconds > 0)
    {
        mask |= SOPC_DataValue_NotZeroServerPico;
    }
    return mask;
}

SOPC_ReturnStatus SOPC_DataValue_WriteAux(const void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_DataValue_Write((const SOPC_DataValue*) value, buf, nestedStructLevel);
}

static SOPC_ReturnStatus SOPC_DataValue_Write_Internal(const SOPC_DataValue* dataValue,
                                                       SOPC_Buffer* buf,
                                                       uint32_t nestedStructLevel)
{
    SOPC_Byte encodingMask = 0;
    if (NULL == dataValue)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    else if (nestedStructLevel >= SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    nestedStructLevel++;
    encodingMask = GetDataValueEncodingMask(dataValue);
    SOPC_ReturnStatus status = SOPC_Byte_Write(&encodingMask, buf, nestedStructLevel);

    if (SOPC_STATUS_OK == status && (encodingMask & SOPC_DataValue_NotNullValue) != 0)
    {
        status = SOPC_Variant_Write_Internal(&dataValue->Value, buf, nestedStructLevel);
    }
    if (SOPC_STATUS_OK == status && (encodingMask & SOPC_DataValue_NotGoodStatusCode) != 0)
    {
        status = SOPC_StatusCode_Write(&dataValue->Status, buf, nestedStructLevel);
    }
    if (SOPC_STATUS_OK == status && (encodingMask & SOPC_DataValue_NotMinSourceDate) != 0)
    {
        status = SOPC_DateTime_Write(&dataValue->SourceTimestamp, buf, nestedStructLevel);
    }
    if (SOPC_STATUS_OK == status && (encodingMask & SOPC_DataValue_NotZeroSourcePico) != 0)
    {
        status = SOPC_UInt16_Write(&dataValue->SourcePicoSeconds, buf, nestedStructLevel);
    }
    if (SOPC_STATUS_OK == status && (encodingMask & SOPC_DataValue_NotMinServerDate) != 0)
    {
        status = SOPC_DateTime_Write(&dataValue->ServerTimestamp, buf, nestedStructLevel);
    }
    if (SOPC_STATUS_OK == status && (encodingMask & SOPC_DataValue_NotZeroServerPico) != 0)
    {
        status = SOPC_UInt16_Write(&dataValue->ServerPicoSeconds, buf, nestedStructLevel);
    }
    return status;
}

SOPC_ReturnStatus SOPC_DataValue_WriteAux_Nested(const void* value, SOPC_Buffer* buf, uint32_t nestedLevel)
{
    return SOPC_DataValue_Write_Internal((const SOPC_DataValue*) value, buf, nestedLevel);
}

SOPC_ReturnStatus SOPC_DataValue_Write(const SOPC_DataValue* dataValue, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    // Manage variant nested level (data value could be contained in a variant)
    return SOPC_DataValue_Write_Internal(dataValue, buf, nestedStructLevel);
}

SOPC_ReturnStatus SOPC_DataValue_ReadAux(void* value, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_DataValue_Read((SOPC_DataValue*) value, buf, nestedStructLevel);
}

static SOPC_ReturnStatus SOPC_DataValue_Read_Internal(SOPC_DataValue* dataValue,
                                                      SOPC_Buffer* buf,
                                                      uint32_t nestedStructLevel)
{
    SOPC_Byte encodingMask = 0;
    if (NULL == dataValue)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    else if (nestedStructLevel >= SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    nestedStructLevel++;
    SOPC_ReturnStatus status = SOPC_Byte_Read(&encodingMask, buf, nestedStructLevel);
    if (SOPC_STATUS_OK == status && (encodingMask & SOPC_DataValue_NotNullValue) != 0)
    {
        status = SOPC_Variant_Read_Internal(&dataValue->Value, buf, nestedStructLevel);
    }
    else
    {
        dataValue->Value.BuiltInTypeId = SOPC_Null_Id;
    }
    if (SOPC_STATUS_OK == status && (encodingMask & SOPC_DataValue_NotGoodStatusCode) != 0)
    {
        status = SOPC_StatusCode_Read(&dataValue->Status, buf, nestedStructLevel);
    }
    else
    {
        dataValue->Status = SOPC_STATUS_OK;
    }
    if (SOPC_STATUS_OK == status && (encodingMask & SOPC_DataValue_NotMinSourceDate) != 0)
    {
        status = SOPC_DateTime_Read(&dataValue->SourceTimestamp, buf, nestedStructLevel);
    }
    else
    {
        dataValue->SourceTimestamp = 0;
    }
    if (SOPC_STATUS_OK == status && (encodingMask & SOPC_DataValue_NotZeroSourcePico) != 0)
    {
        status = SOPC_UInt16_Read(&dataValue->SourcePicoSeconds, buf, nestedStructLevel);
    }
    else
    {
        dataValue->SourcePicoSeconds = 0;
    }
    if (SOPC_STATUS_OK == status && (encodingMask & SOPC_DataValue_NotMinServerDate) != 0)
    {
        status = SOPC_DateTime_Read(&dataValue->ServerTimestamp, buf, nestedStructLevel);
    }
    else
    {
        dataValue->ServerTimestamp = 0;
    }
    if (SOPC_STATUS_OK == status && (encodingMask & SOPC_DataValue_NotZeroServerPico) != 0)
    {
        status = SOPC_UInt16_Read(&dataValue->ServerPicoSeconds, buf, nestedStructLevel);
    }
    else
    {
        dataValue->ServerPicoSeconds = 0;
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_Variant_Clear(&dataValue->Value);
        SOPC_StatusCode_Clear(&dataValue->Status);
        SOPC_DateTime_Clear(&dataValue->SourceTimestamp);
        SOPC_UInt16_Clear(&dataValue->SourcePicoSeconds);
        SOPC_DateTime_Clear(&dataValue->ServerTimestamp);
        // No clear for last read since it should manage it itself in case of failure
    }

    return status;
}

SOPC_ReturnStatus SOPC_DataValue_ReadAux_Nested(void* value, SOPC_Buffer* buf, uint32_t nestedLevel)
{
    return SOPC_DataValue_Read_Internal((SOPC_DataValue*) value, buf, nestedLevel);
}

SOPC_ReturnStatus SOPC_DataValue_Read(SOPC_DataValue* dataValue, SOPC_Buffer* buf, uint32_t nestedStructLevel)
{
    return SOPC_DataValue_Read_Internal(dataValue, buf, nestedStructLevel);
}

SOPC_ReturnStatus SOPC_Read_Array(SOPC_Buffer* buf,
                                  int32_t* noOfElts,
                                  void** eltsArray,
                                  size_t sizeOfElt,
                                  SOPC_EncodeableObject_PfnDecode* decodeFct,
                                  SOPC_EncodeableObject_PfnInitialize* initializeFct,
                                  SOPC_EncodeableObject_PfnClear* clearFct,
                                  uint32_t nestedStructLevel)
{
    SOPC_Byte* byteArray = NULL;

    if (NULL == buf || NULL == noOfElts || NULL == eltsArray || NULL != *eltsArray || NULL == decodeFct ||
        0 == sizeOfElt)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    else if (nestedStructLevel >= SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    nestedStructLevel++;
    SOPC_ReturnStatus status = SOPC_Int32_Read(noOfElts, buf, nestedStructLevel);

    if (SOPC_STATUS_OK != status)
    {
        return status;
    }

    if (*noOfElts < 0)
    {
        *noOfElts = 0;
    }

    if (*noOfElts > SOPC_Internal_Common_GetEncodingConstants()->max_array_length)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    if (*noOfElts > 0 && (uint64_t) *noOfElts <= SIZE_MAX / sizeOfElt)
    {
        *eltsArray = SOPC_Calloc((size_t) *noOfElts, sizeOfElt);

        if (NULL == *eltsArray)
        {
            return SOPC_STATUS_NOK;
        }

        size_t idx;
        byteArray = (SOPC_Byte*) *eltsArray;

        for (idx = 0; SOPC_STATUS_OK == status && idx < (size_t) *noOfElts; idx++)
        {
            size_t pos = idx * sizeOfElt;
            initializeFct(&(byteArray[pos]));
            status = decodeFct(&(byteArray[pos]), buf, nestedStructLevel);
        }

        if (SOPC_STATUS_OK != status)
        {
            size_t clearIdx = 0;
            // idx - 1 => clear only cases in which status was ok since we don't know
            //            the state in which byte array is in the last idx used (decode failed)
            for (clearIdx = 0; clearIdx < (idx - 1); clearIdx++)
            {
                size_t pos = clearIdx * sizeOfElt;
                clearFct(&(byteArray[pos]));
            }
            SOPC_Free(*eltsArray);
            *eltsArray = NULL;
            *noOfElts = 0;
        }
    }

    return status;
}

SOPC_ReturnStatus SOPC_Write_Array(SOPC_Buffer* msgBuf,
                                   const int32_t* const noOfElts,
                                   const void* const* eltsArray,
                                   size_t sizeOfElt,
                                   SOPC_EncodeableObject_PfnEncode* encodeFct,
                                   uint32_t nestedStructLevel)
{
    if (NULL == msgBuf || NULL == noOfElts || NULL == eltsArray || NULL == encodeFct ||
        (*noOfElts > 0 && NULL == *eltsArray))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    else if (nestedStructLevel >= SOPC_Internal_Common_GetEncodingConstants()->max_nested_struct)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    nestedStructLevel++;
    SOPC_ReturnStatus status = SOPC_Int32_Write(noOfElts, msgBuf, nestedStructLevel);

    if (SOPC_STATUS_OK == status && *noOfElts > 0)
    {
        const SOPC_Byte* byteArray = *eltsArray;
        size_t idx = 0;
        size_t pos = 0;
        for (idx = 0; SOPC_STATUS_OK == status && idx < (size_t) *noOfElts; idx++)
        {
            pos = idx * sizeOfElt;
            status = encodeFct(&(byteArray[pos]), msgBuf, nestedStructLevel);
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_EncodeMsg_Type_Header_Body(SOPC_Buffer* buf,
                                                  SOPC_EncodeableType* encType,
                                                  SOPC_EncodeableType* headerType,
                                                  void* msgHeader,
                                                  void* msgBody)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    SOPC_NodeId nodeId;
    SOPC_NodeId_Initialize(&nodeId);

    if (NULL != buf &&
        // Body cannot be null except in case of service fault message
        NULL != encType && (NULL != msgBody || encType->TypeId == OpcUaId_ServiceFault) && NULL != headerType &&
        NULL != msgHeader)
    {
        nodeId.IdentifierType = SOPC_IdentifierType_Numeric;
        if (NULL == encType->NamespaceUri)
        {
            nodeId.Namespace = 0;
        }
        else
        {
            // TODO: find namespace Id
        }
        nodeId.Data.Numeric = encType->BinaryEncodingTypeId;

        status = SOPC_NodeId_Write(&nodeId, buf, 0);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_EncodeableObject_Encode(headerType, msgHeader, buf, 0);
    }
    if (SOPC_STATUS_OK == status && encType->TypeId != OpcUaId_ServiceFault)
    {
        status = SOPC_EncodeableObject_Encode(encType, msgBody, buf, 0);
    }
    return status;
}

SOPC_ReturnStatus SOPC_MsgBodyType_Read(SOPC_Buffer* buf, SOPC_EncodeableType** receivedEncType)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    SOPC_EncodeableType* recEncType = NULL;
    SOPC_NodeId nodeId;
    SOPC_NodeId_Initialize(&nodeId);
    if (NULL != buf)
    {
        status = SOPC_NodeId_Read(&nodeId, buf, 0);
    }

    if (SOPC_STATUS_OK == status && nodeId.IdentifierType == SOPC_IdentifierType_Numeric)
    {
        // Must be the case in which we cannot know the type before decoding it
        if (OPCUA_NAMESPACE_INDEX == nodeId.Namespace)
        {
            recEncType = SOPC_EncodeableType_GetEncodeableType(OPCUA_NAMESPACE_INDEX, nodeId.Data.Numeric);
        }
        else
        {
            status = SOPC_STATUS_ENCODING_ERROR;
        }
        *receivedEncType = recEncType;
    }

    SOPC_NodeId_Clear(&nodeId);
    return status;
}

SOPC_ReturnStatus SOPC_DecodeMsg_HeaderOrBody(SOPC_Buffer* buffer,
                                              SOPC_EncodeableType* msgEncType,
                                              void** encodeableObj)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != buffer && NULL != encodeableObj && NULL != msgEncType)
    {
        *encodeableObj = SOPC_Malloc(msgEncType->AllocationSize);
        if (NULL != *encodeableObj)
        {
            status = SOPC_EncodeableObject_Decode(msgEncType, *encodeableObj, buffer, 0);
            if (SOPC_STATUS_OK != status)
            {
                SOPC_Free(*encodeableObj);
                *encodeableObj = NULL;
            }
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
    }
    return status;
}

const SOPC_BuiltInType_Encoding SOPC_BuiltInType_EncodingTable[SOPC_BUILTINID_MAX + 1] = {
    {NULL, NULL},
    {SOPC_Boolean_WriteAux, SOPC_Boolean_ReadAux},
    {SOPC_SByte_WriteAux, SOPC_SByte_ReadAux},
    {SOPC_Byte_WriteAux, SOPC_Byte_ReadAux},
    {SOPC_Int16_WriteAux, SOPC_Int16_ReadAux},
    {SOPC_UInt16_WriteAux, SOPC_UInt16_ReadAux},
    {SOPC_Int32_WriteAux, SOPC_Int32_ReadAux},
    {SOPC_UInt32_WriteAux, SOPC_UInt32_ReadAux},
    {SOPC_Int64_WriteAux, SOPC_Int64_ReadAux},
    {SOPC_UInt64_WriteAux, SOPC_UInt64_ReadAux},
    {SOPC_Float_WriteAux, SOPC_Float_ReadAux},
    {SOPC_Double_WriteAux, SOPC_Double_ReadAux},
    {SOPC_String_WriteAux, SOPC_String_ReadAux},
    {SOPC_DateTime_WriteAux, SOPC_DateTime_ReadAux},
    {SOPC_Guid_WriteAux, SOPC_Guid_ReadAux},
    {SOPC_ByteString_WriteAux, SOPC_ByteString_ReadAux},
    {SOPC_XmlElement_WriteAux, SOPC_XmlElement_ReadAux},
    {SOPC_NodeId_WriteAux, SOPC_NodeId_ReadAux},
    {SOPC_ExpandedNodeId_WriteAux, SOPC_ExpandedNodeId_ReadAux},
    {SOPC_StatusCode_WriteAux, SOPC_StatusCode_ReadAux},
    {SOPC_QualifiedName_WriteAux, SOPC_QualifiedName_ReadAux},
    {SOPC_LocalizedText_WriteAux, SOPC_LocalizedText_ReadAux},
    {SOPC_ExtensionObject_WriteAux, SOPC_ExtensionObject_ReadAux},
    {SOPC_DataValue_WriteAux, SOPC_DataValue_ReadAux},
    {SOPC_Variant_WriteAux, SOPC_Variant_ReadAux},
    {SOPC_DiagnosticInfo_WriteAux, SOPC_DiagnosticInfo_ReadAux}};
