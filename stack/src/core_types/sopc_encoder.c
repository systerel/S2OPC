/*
 *  Copyright (C) 2016 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "sopc_encoder.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "platform_deps.h"
#include "sopc_encodeabletype.h"
#include "sopc_namespace_table.h"
#include "sopc_stack_config.h"

void SOPC_EncodeDecode_Int16(int16_t* intv)
{
    uint16_t* twoBytes = (uint16_t*) intv;
    assert(endianess != P_Endianess_Undefined);
    if(endianess == P_Endianess_BigEndian){
        *twoBytes = SWAP_2_BYTES(*twoBytes);
    }
}

void SOPC_EncodeDecode_UInt16(uint16_t* uintv)
{
    assert(endianess != P_Endianess_Undefined);
    if(endianess == P_Endianess_BigEndian){
        *uintv = SWAP_2_BYTES(*uintv);
    }
}

void SOPC_EncodeDecode_Int32(int32_t* intv)
{
    assert(endianess != P_Endianess_Undefined);
    uint32_t* fourBytes = (uint32_t*) intv;
    if(endianess == P_Endianess_BigEndian){
        *fourBytes = SWAP_4_BYTES(*fourBytes);
    }
}


void SOPC_EncodeDecode_UInt32(uint32_t* uintv)
{
    assert(endianess != P_Endianess_Undefined);
    if(endianess == P_Endianess_BigEndian){
        *uintv = SWAP_4_BYTES(*uintv);
    }
}

void SOPC_EncodeDecode_Int64(int64_t* intv)
{
    assert(endianess != P_Endianess_Undefined);
    uint64_t* eightBytes = (uint64_t*) intv;
    if(endianess == P_Endianess_BigEndian){
        *eightBytes = SWAP_8_BYTES(*eightBytes);
    }
}

void SOPC_EncodeDecode_UInt64(uint64_t* uintv)
{
    assert(endianess != P_Endianess_Undefined);
    if(endianess == P_Endianess_BigEndian){
        *uintv = SWAP_8_BYTES(*uintv);
    }
}

void SOPC_EncodeDecode_Float(float* floatv){
    assert(floatEndianess != P_Endianess_Undefined);
    uint32_t* fourBytes = (uint32_t*) floatv;
    if(floatEndianess == P_Endianess_BigEndian){
        *fourBytes = SWAP_4_BYTES(*fourBytes);
    }
}

void SOPC_EncodeDecode_Double(double* doublev){
    assert(floatEndianess != P_Endianess_Undefined);
    uint64_t* eightBytes = (uint64_t*) doublev;
    if(floatEndianess == P_Endianess_BigEndian){
        *eightBytes = SWAP_8_BYTES(*eightBytes);
    }
}


SOPC_StatusCode SOPC_Byte_WriteAux(void* value, SOPC_Buffer* buf){
    return SOPC_Byte_Write((SOPC_Byte*) value, buf);
}

SOPC_StatusCode SOPC_Byte_Write(const SOPC_Byte* value, SOPC_Buffer* buf)
{
    if(value == NULL || buf == NULL){
        return STATUS_INVALID_PARAMETERS;
    }
    return SOPC_Buffer_Write(buf, value, 1);
}


SOPC_StatusCode SOPC_Byte_ReadAux(void* value, SOPC_Buffer* buf)
{
    return SOPC_Byte_Read((SOPC_Byte*) value, buf);
}

SOPC_StatusCode SOPC_Byte_Read(SOPC_Byte* value, SOPC_Buffer* buf)
{
    if(value == NULL || buf == NULL){
        return STATUS_INVALID_PARAMETERS;
    }
    return SOPC_Buffer_Read(value, buf, 1);
}


SOPC_StatusCode SOPC_Boolean_WriteAux(void* value, SOPC_Buffer* buf){
    return SOPC_Boolean_Write((SOPC_Boolean*) value, buf);
}

SOPC_StatusCode SOPC_Boolean_Write(const SOPC_Boolean* value, SOPC_Buffer* buf)
{
    SOPC_Byte encodedValue;
    if(value == NULL){
        return STATUS_INVALID_PARAMETERS;
    }

    if(*value == FALSE){
        encodedValue = *value;
    }else{
        // Encoder should use 1 as True value
        encodedValue = 1;
    }

    return SOPC_Byte_Write(&encodedValue, buf);
}


SOPC_StatusCode SOPC_Boolean_ReadAux(void* value, SOPC_Buffer* buf)
{
    return SOPC_Boolean_Read((SOPC_Boolean*) value, buf);
}

SOPC_StatusCode SOPC_Boolean_Read(SOPC_Boolean* value, SOPC_Buffer* buf)
{
    SOPC_StatusCode status = STATUS_NOK;
    if(value == NULL){
        status = STATUS_INVALID_PARAMETERS;
    }else{
        status = SOPC_Byte_Read(value, buf);
        if(status == STATUS_OK){
            if(*value != FALSE){
                // Decoder should use 1 as True value
                *value = 1;
            }
        }
    }
    return status;
}


SOPC_StatusCode SOPC_SByte_WriteAux(void* value, SOPC_Buffer* buf){
    return SOPC_SByte_Write((SOPC_SByte*) value, buf);
}

SOPC_StatusCode SOPC_SByte_Write(const SOPC_SByte* value, SOPC_Buffer* buf)
{
    if(value == NULL){
        return STATUS_INVALID_PARAMETERS;
    }
    return SOPC_Buffer_Write(buf, (SOPC_Byte*) value, 1);
}


SOPC_StatusCode SOPC_SByte_ReadAux(void* value, SOPC_Buffer* buf)
{
    return SOPC_SByte_Read((SOPC_SByte*) value, buf);
}

SOPC_StatusCode SOPC_SByte_Read(SOPC_SByte* value, SOPC_Buffer* buf)
{
    if(value == NULL){
        return STATUS_INVALID_PARAMETERS;
    }

    return SOPC_Buffer_Read((SOPC_Byte*) value, buf, 1);
}


SOPC_StatusCode SOPC_Int16_WriteAux(void* value, SOPC_Buffer* buf){
    return SOPC_Int16_Write((int16_t*) value, buf);
}

SOPC_StatusCode SOPC_Int16_Write(const int16_t* value, SOPC_Buffer* buf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(value != NULL){
        int16_t encodedValue = *value;
        SOPC_EncodeDecode_Int16(&encodedValue);
        status = SOPC_Buffer_Write(buf, (SOPC_Byte*) &encodedValue, 2);
    }
    return status;
}


SOPC_StatusCode SOPC_Int16_ReadAux(void* value, SOPC_Buffer* buf)
{
    return SOPC_Int16_Read((int16_t*) value, buf);
}

SOPC_StatusCode SOPC_Int16_Read(int16_t* value, SOPC_Buffer* buf)
{
    SOPC_StatusCode status = STATUS_NOK;
    int16_t readValue;
    if(value == NULL){
        status = STATUS_INVALID_PARAMETERS;
    }else{
        status = SOPC_Buffer_Read((SOPC_Byte*)&readValue, buf, 2);
        if(status == STATUS_OK){
            SOPC_EncodeDecode_Int16(&readValue);
            *value = readValue;
        }
    }
    return status;
}


SOPC_StatusCode SOPC_UInt16_WriteAux(void* value, SOPC_Buffer* buf){
    return SOPC_UInt16_Write((uint16_t*) value, buf);
}

SOPC_StatusCode SOPC_UInt16_Write(const uint16_t* value, SOPC_Buffer* buf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(value != NULL){
        uint16_t encodedValue = *value;
        SOPC_EncodeDecode_UInt16(&encodedValue);
        status = SOPC_Buffer_Write(buf, (SOPC_Byte*) &encodedValue, 2);
    }
    return status;
}


SOPC_StatusCode SOPC_UInt16_ReadAux(void* value, SOPC_Buffer* buf)
{
    return SOPC_UInt16_Read((uint16_t*) value, buf);
}

SOPC_StatusCode SOPC_UInt16_Read(uint16_t* value, SOPC_Buffer* buf)
{
    SOPC_StatusCode status = STATUS_NOK;
    if(value == NULL){
        status = STATUS_INVALID_PARAMETERS;
    }else{
        status = SOPC_Buffer_Read((SOPC_Byte*)value, buf, 2);
        if(status == STATUS_OK){
            SOPC_EncodeDecode_UInt16(value);
        }
    }
    return status;
}


SOPC_StatusCode SOPC_Int32_WriteAux(void* value, SOPC_Buffer* buf){
    return SOPC_Int32_Write((int32_t*) value, buf);
}

SOPC_StatusCode SOPC_Int32_Write(const int32_t* value, SOPC_Buffer* buf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(value != NULL){
        int32_t encodedValue = *value;
        SOPC_EncodeDecode_Int32(&encodedValue);
        status = SOPC_Buffer_Write(buf, (SOPC_Byte*) &encodedValue, 4);
    }
    return status;
}


SOPC_StatusCode SOPC_Int32_ReadAux(void* value, SOPC_Buffer* buf)
{
    return SOPC_Int32_Read((int32_t*) value, buf);
}

SOPC_StatusCode SOPC_Int32_Read(int32_t* value, SOPC_Buffer* buf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(value != NULL){
        status = SOPC_Buffer_Read((SOPC_Byte*)value, buf, 4);
        if(status == STATUS_OK){
            SOPC_EncodeDecode_Int32(value);
        }
    }
    return status;
}


SOPC_StatusCode SOPC_UInt32_WriteAux(void* value, SOPC_Buffer* buf){
    return SOPC_UInt32_Write((uint32_t*) value, buf);
}

SOPC_StatusCode SOPC_UInt32_Write(const uint32_t* value, SOPC_Buffer* buf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(value != NULL){
        uint32_t encodedValue = *value;
        SOPC_EncodeDecode_UInt32(&encodedValue);
        status = SOPC_Buffer_Write(buf, (SOPC_Byte*) &encodedValue, 4);
    }
    return status;
}


SOPC_StatusCode SOPC_UInt32_ReadAux(void* value, SOPC_Buffer* buf)
{
    return SOPC_UInt32_Read((uint32_t*) value, buf);
}

SOPC_StatusCode SOPC_UInt32_Read(uint32_t* value, SOPC_Buffer* buf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(value != NULL){
        status = SOPC_Buffer_Read((SOPC_Byte*)value, buf, 4);
        if(status == STATUS_OK){
            SOPC_EncodeDecode_UInt32(value);
        }
    }
    return status;
}


SOPC_StatusCode SOPC_Int64_WriteAux(void* value, SOPC_Buffer* buf){
    return SOPC_Int64_Write((int64_t*) value, buf);
}

SOPC_StatusCode SOPC_Int64_Write(const int64_t* value, SOPC_Buffer* buf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(value != NULL){
        int64_t encodedValue = *value;
        SOPC_EncodeDecode_Int64(&encodedValue);
        status = SOPC_Buffer_Write(buf, (SOPC_Byte*) &encodedValue, 8);
    }
    return status;
}


SOPC_StatusCode SOPC_Int64_ReadAux(void* value, SOPC_Buffer* buf)
{
    return SOPC_Int64_Read((int64_t*) value, buf);
}

SOPC_StatusCode SOPC_Int64_Read(int64_t* value, SOPC_Buffer* buf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(value != NULL){
        status = SOPC_Buffer_Read((SOPC_Byte*) value, buf, 8);
        if(status == STATUS_OK){
            SOPC_EncodeDecode_Int64(value);
        }
    }
    return status;
}


SOPC_StatusCode SOPC_UInt64_WriteAux(void* value, SOPC_Buffer* buf){
    return SOPC_UInt64_Write((uint64_t*) value, buf);
}

SOPC_StatusCode SOPC_UInt64_Write(const uint64_t* value, SOPC_Buffer* buf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(value != NULL){
        uint64_t encodedValue = *value;
        SOPC_EncodeDecode_UInt64(&encodedValue);
        status = SOPC_Buffer_Write(buf, (SOPC_Byte*) &encodedValue, 8);
    }
    return status;
}


SOPC_StatusCode SOPC_UInt64_ReadAux(void* value, SOPC_Buffer* buf)
{
    return SOPC_UInt64_Read((uint64_t*) value, buf);
}

SOPC_StatusCode SOPC_UInt64_Read(uint64_t* value, SOPC_Buffer* buf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(value != NULL){
        status = SOPC_Buffer_Read((SOPC_Byte*) value, buf, 8);
        if(status == STATUS_OK){
            SOPC_EncodeDecode_UInt64(value);
        }
    }
    return status;
}


SOPC_StatusCode SOPC_Float_WriteAux(void* value, SOPC_Buffer* buf){
    return SOPC_Float_Write((float*) value, buf);
}

SOPC_StatusCode SOPC_Float_Write(const float* value, SOPC_Buffer* buf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(value != NULL){
        float encodedValue = *value;
        SOPC_EncodeDecode_Float(&encodedValue);
        status = SOPC_Buffer_Write(buf, (SOPC_Byte*) &encodedValue, 4);
    }
    return status;
}


SOPC_StatusCode SOPC_Float_ReadAux(void* value, SOPC_Buffer* buf)
{
    return SOPC_Float_Read((float*) value, buf);
}

SOPC_StatusCode SOPC_Float_Read(float* value, SOPC_Buffer* buf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(value != NULL){
        status = SOPC_Buffer_Read((SOPC_Byte*)value, buf, 4);
        if(status == STATUS_OK){
            SOPC_EncodeDecode_Float(value);
        }
    }
    return status;
}


SOPC_StatusCode SOPC_Double_WriteAux(void* value, SOPC_Buffer* buf){
    return SOPC_Double_Write((double*) value, buf);
}

SOPC_StatusCode SOPC_Double_Write(const double* value, SOPC_Buffer* buf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(value != NULL){
        double encodedValue = *value;
        SOPC_EncodeDecode_Double(&encodedValue);
        status = SOPC_Buffer_Write(buf, (SOPC_Byte*) &encodedValue, 8);
    }
    return status;
}


SOPC_StatusCode SOPC_Double_ReadAux(void* value, SOPC_Buffer* buf)
{
    return SOPC_Double_Read((double*) value, buf);
}

SOPC_StatusCode SOPC_Double_Read(double* value, SOPC_Buffer* buf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(value != NULL){
        status = SOPC_Buffer_Read((SOPC_Byte*)value, buf, 8);
        if(status == STATUS_OK){
            SOPC_EncodeDecode_Double(value);
        }
    }
    return status;
}


SOPC_StatusCode SOPC_ByteString_WriteAux(void* value, SOPC_Buffer* buf){
    return SOPC_ByteString_Write((SOPC_ByteString*) value, buf);
}

SOPC_StatusCode SOPC_ByteString_Write(const SOPC_ByteString* str, SOPC_Buffer* buf)
{
    SOPC_StatusCode status = STATUS_NOK;
    if(str == NULL){
        status = STATUS_INVALID_PARAMETERS;
    }else{
        int32_t length;
        if(str->Length > 0){
            length = str->Length;
        }else{
            length = -1;
        }
        status = SOPC_Int32_Write(&length, buf);
        if(status == STATUS_OK &&
           str->Length > 0)
        {
            status = SOPC_Buffer_Write(buf, str->Data, str->Length);
        }
    }
    return status;
}


SOPC_StatusCode SOPC_ByteString_ReadAux(void* value, SOPC_Buffer* buf)
{
    return SOPC_ByteString_Read((SOPC_ByteString*) value, buf);
}

SOPC_StatusCode SOPC_ByteString_Read(SOPC_ByteString* str, SOPC_Buffer* buf)
{
    SOPC_StatusCode status = STATUS_NOK;
    int32_t length;
    if(str == NULL || (str != NULL && str->Data != NULL)){
        status = STATUS_INVALID_PARAMETERS;
    }else{
        status = SOPC_Buffer_Read((SOPC_Byte*)&length, buf, 4);
        if(status == STATUS_OK){
            SOPC_EncodeDecode_Int32(&length);
            if(length > 0){
                str->Length = length;
                str->Data = malloc(sizeof(SOPC_Byte) * length);
                if(str->Data != NULL){
                    status = SOPC_Buffer_Read(str->Data, buf, length);
                    if(status != STATUS_OK){
                        status = STATUS_INVALID_STATE;
                        free(str->Data);
                        str->Data = NULL;
                        str->Length = -1;
                    }
                }
            }else{
                str->Length = -1;
            }
        }

    }
    return status;
}


SOPC_StatusCode SOPC_String_WriteAux(void* value, SOPC_Buffer* buf){
    return SOPC_String_Write((SOPC_String*) value, buf);
}

SOPC_StatusCode SOPC_String_Write(const SOPC_String* str, SOPC_Buffer* buf)
{
    SOPC_StatusCode status = STATUS_NOK;
    if(str == NULL){
        status = STATUS_INVALID_PARAMETERS;
    }else{
        int32_t length;
        if(str->Length > 0){
            length = str->Length;
        }else{
            length = -1;
        }
        status = SOPC_Int32_Write(&length, buf);
        if(status == STATUS_OK &&
           str->Length > 0)
        {
            status = SOPC_Buffer_Write(buf, str->Data, str->Length);
        }
    }
    return status;
}


SOPC_StatusCode SOPC_String_ReadAux(void* value, SOPC_Buffer* buf)
{
    return SOPC_String_Read((SOPC_String*) value, buf);
}

SOPC_StatusCode SOPC_String_Read(SOPC_String* str, SOPC_Buffer* buf)
{
    SOPC_StatusCode status = STATUS_NOK;
    int32_t length;
    if(str == NULL || (str != NULL && str->Data != NULL)){
        status = STATUS_INVALID_PARAMETERS;
    }else{
        status = SOPC_Buffer_Read((SOPC_Byte*)&length, buf, 4);
        if(status == STATUS_OK){
            SOPC_EncodeDecode_Int32(&length);
            if(length > 0){
                str->Length = length;
                // +1 to add '\0' character for CString compatibility
                str->Data = malloc(sizeof(SOPC_Byte) * (length + 1));
                if(str->Data != NULL){
                    status = SOPC_Buffer_Read(str->Data, buf, length);
                    if(status != STATUS_OK){
                        status = STATUS_INVALID_STATE;
                        free(str->Data);
                        str->Data = NULL;
                        str->Length = -1;
                    }else{
                        // Add '\0' character for CString compatibility
                        str->Data[str->Length] = '\0';
                    }
                }
            }else{
                str->Length = -1;
            }
        }

    }
    return status;
}


SOPC_StatusCode SOPC_XmlElement_WriteAux(void* value, SOPC_Buffer* buf){
    return SOPC_XmlElement_Write((SOPC_XmlElement*) value, buf);
}

SOPC_StatusCode SOPC_XmlElement_Write(const SOPC_XmlElement* xml, SOPC_Buffer* buf)
{
    // TODO: check XML validity ?
    return SOPC_ByteString_Write((SOPC_ByteString*) xml, buf);
}


SOPC_StatusCode SOPC_XmlElement_ReadAux(void* value, SOPC_Buffer* buf)
{
    return SOPC_XmlElement_Read((SOPC_XmlElement*) value, buf);
}

SOPC_StatusCode SOPC_XmlElement_Read(SOPC_XmlElement* xml, SOPC_Buffer* buf)
{
    // TODO: parse XML ?
    return SOPC_ByteString_Read((SOPC_ByteString*) xml, buf);
}


SOPC_StatusCode SOPC_DateTime_WriteAux(void* value, SOPC_Buffer* buf){
    return SOPC_DateTime_Write((SOPC_DateTime*) value, buf);
}

SOPC_StatusCode SOPC_DateTime_Write(const SOPC_DateTime* date, SOPC_Buffer* buf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    int64_t dateTime = 0;
    if(NULL != date){
        dateTime = SOPC_DateTime_ToInt64(date);
        status = SOPC_Int64_Write(&dateTime, buf);
    }
    return status;
}


SOPC_StatusCode SOPC_DateTime_ReadAux(void* value, SOPC_Buffer* buf)
{
    return SOPC_DateTime_Read((SOPC_DateTime*) value, buf);
}

SOPC_StatusCode SOPC_DateTime_Read(SOPC_DateTime* date, SOPC_Buffer* buf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    int64_t dateTime = 0;
    if(NULL != date){
        status = SOPC_Int64_Read(&dateTime, buf);
        SOPC_DateTime_FromInt64(date, dateTime);
    }
    return status;
}


SOPC_StatusCode SOPC_Guid_WriteAux(void* value, SOPC_Buffer* buf){
    return SOPC_Guid_Write((SOPC_Guid*) value, buf);
}

SOPC_StatusCode SOPC_Guid_Write(const SOPC_Guid* guid, SOPC_Buffer* buf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(guid != NULL){
        status = SOPC_UInt32_Write(&guid->Data1, buf);
    }
    if(status == STATUS_OK){
        status = SOPC_UInt16_Write(&guid->Data2, buf);
    }
    if(status == STATUS_OK){
        status = SOPC_UInt16_Write(&guid->Data3, buf);
    }
    if(status == STATUS_OK){
        status = SOPC_Buffer_Write(buf, &(guid->Data4[0]), 8);
    }
    return status;
}


SOPC_StatusCode SOPC_Guid_ReadAux(void* value, SOPC_Buffer* buf)
{
    return SOPC_Guid_Read((SOPC_Guid*) value, buf);
}

SOPC_StatusCode SOPC_Guid_Read(SOPC_Guid* guid, SOPC_Buffer* buf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(guid != NULL){
        status = SOPC_UInt32_Read(&guid->Data1, buf);
    }
    if(status == STATUS_OK){
        status = SOPC_UInt16_Read(&guid->Data2, buf);
    }
    if(status == STATUS_OK){
        status = SOPC_UInt16_Read(&guid->Data3, buf);
    }
    if(status == STATUS_OK){
        status = SOPC_Buffer_Read(&(guid->Data4[0]), buf, 8);
    }

    if(status != STATUS_OK && guid != NULL){
        SOPC_UInt32_Clear(&guid->Data1);
        SOPC_UInt16_Clear(&guid->Data2);
        SOPC_UInt16_Clear(&guid->Data3);
    }

    return status;
}

SOPC_NodeId_DataEncoding GetNodeIdDataEncoding(const SOPC_NodeId* nodeId){
    SOPC_NodeId_DataEncoding encodingEnum = NodeIdEncoding_Invalid;
    switch(nodeId->IdentifierType){
        case IdentifierType_Numeric:
            if(nodeId->Data.Numeric <= UINT8_MAX){
                encodingEnum = NodeIdEncoding_TwoByte;
            }else if(nodeId->Data.Numeric <= UINT16_MAX){
                encodingEnum = NodeIdEncoding_FourByte;
            }else{
                encodingEnum = NodeIdEncoding_Numeric;
            }
            break;
        case IdentifierType_String:
            encodingEnum = NodeIdEncoding_String;
            break;
        case IdentifierType_Guid:
            encodingEnum = NodeIdEncoding_Guid;
            break;
        case IdentifierType_ByteString:
            encodingEnum = NodeIdEncoding_ByteString;
            break;
    }
    return encodingEnum;
}

SOPC_StatusCode Internal_NodeId_Write(SOPC_Buffer* buf,
                                 SOPC_Byte encodingByte,
                                 const SOPC_NodeId* nodeId)
{
    assert(nodeId != NULL);
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_NodeId_DataEncoding encodingType = 0x0F & encodingByte; // Eliminate flags

    SOPC_Byte byte = 0;
    uint16_t twoBytes = 0;

    status = SOPC_Byte_Write(&encodingByte, buf);
    if(status == STATUS_OK){
        switch(encodingType){
            case NodeIdEncoding_Invalid:
                status = STATUS_INVALID_PARAMETERS;
                break;
            case NodeIdEncoding_TwoByte:
                byte = (SOPC_Byte) nodeId->Data.Numeric;
                status = SOPC_Byte_Write(&byte, buf);
                break;
            case  NodeIdEncoding_FourByte:
                twoBytes = (uint16_t) nodeId->Data.Numeric;
                if(nodeId->Namespace <= UINT8_MAX){
                    const SOPC_Byte namespace = (SOPC_Byte) nodeId->Namespace;
                    status = SOPC_Byte_Write(&namespace, buf);
                }else{
                    status = STATUS_INVALID_PARAMETERS;
                }
                if(status == STATUS_OK){
                    status = SOPC_UInt16_Write(&twoBytes, buf);
                }
                break;
            case  NodeIdEncoding_Numeric:
                status = SOPC_UInt16_Write(&nodeId->Namespace, buf);
                if(status == STATUS_OK){
                    status = SOPC_UInt32_Write(&nodeId->Data.Numeric, buf);
                }
                break;
            case  NodeIdEncoding_String:
                status = SOPC_UInt16_Write(&nodeId->Namespace, buf);
                if(status == STATUS_OK){
                    status = SOPC_String_Write(&nodeId->Data.String, buf);
                }
                break;
            case  NodeIdEncoding_Guid:
                status = SOPC_UInt16_Write(&nodeId->Namespace, buf);
                if(status == STATUS_OK){
                    status = SOPC_Guid_Write(nodeId->Data.Guid, buf);
                }
                break;
            case  NodeIdEncoding_ByteString:
                status = SOPC_UInt16_Write(&nodeId->Namespace, buf);
                if(status == STATUS_OK){
                    status = SOPC_ByteString_Write(&nodeId->Data.Bstring, buf);
                }
                break;
            default:
                status = STATUS_INVALID_PARAMETERS;
        }
    }

    return status;
}


SOPC_StatusCode SOPC_NodeId_WriteAux(void* value, SOPC_Buffer* buf){
    return SOPC_NodeId_Write((SOPC_NodeId*) value, buf);
}

SOPC_StatusCode SOPC_NodeId_Write(const SOPC_NodeId* nodeId, SOPC_Buffer* buf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(nodeId != NULL){
        status = Internal_NodeId_Write(buf, GetNodeIdDataEncoding(nodeId), nodeId);
    }
    return status;
}

SOPC_StatusCode Internal_NodeId_Read(SOPC_Buffer* buf,
                                SOPC_NodeId* nodeId,
                                SOPC_Byte* encodingByte)
{
    assert(nodeId != NULL);
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_NodeId_DataEncoding encodingType = 0x0F;

    SOPC_Byte byte = 0;
    uint16_t twoBytes = 0;

    status = SOPC_Byte_Read(encodingByte, buf);

    if(status == STATUS_OK){
        encodingType = 0x0F & *encodingByte; // Eliminate flags
        switch(encodingType){
            case NodeIdEncoding_Invalid:
                status = STATUS_INVALID_RCV_PARAMETER;
                break;
            case NodeIdEncoding_TwoByte:
                nodeId->IdentifierType = IdentifierType_Numeric;
                nodeId->Namespace = 0;
                status = SOPC_Byte_Read(&byte, buf);
                nodeId->Data.Numeric = (uint32_t) byte;
                break;
            case  NodeIdEncoding_FourByte:
                nodeId->IdentifierType = IdentifierType_Numeric;
                status = SOPC_Byte_Read(&byte, buf);
                nodeId->Namespace = byte;
                if(status == STATUS_OK){
                    status = SOPC_UInt16_Read(&twoBytes, buf);
                    nodeId->Data.Numeric = twoBytes;
                }
                break;
            case  NodeIdEncoding_Numeric:
                nodeId->IdentifierType = IdentifierType_Numeric;
                status = SOPC_UInt16_Read(&nodeId->Namespace, buf);
                if(status == STATUS_OK){
                    status = SOPC_UInt32_Read(&nodeId->Data.Numeric, buf);
                }
                break;
            case  NodeIdEncoding_String:
                nodeId->IdentifierType = IdentifierType_String;
                status = SOPC_UInt16_Read(&nodeId->Namespace, buf);
                if(status == STATUS_OK){
                    status = SOPC_String_Read(&nodeId->Data.String, buf);
                }
                break;
            case  NodeIdEncoding_Guid:
                nodeId->IdentifierType = IdentifierType_Guid;
                status = SOPC_UInt16_Read(&nodeId->Namespace, buf);
                if(status == STATUS_OK){
                    nodeId->Data.Guid = malloc(sizeof(SOPC_Guid));
                    if(nodeId->Data.Guid == NULL){
                        status = STATUS_NOK;
                    }else{
                        SOPC_Guid_Initialize(nodeId->Data.Guid);
                        status = SOPC_Guid_Read(nodeId->Data.Guid, buf);
                        if(status != STATUS_OK){
                            free(nodeId->Data.Guid);
                            nodeId->Data.Guid = NULL;
                        }
                    }
                }
                break;
            case  NodeIdEncoding_ByteString:
                nodeId->IdentifierType = IdentifierType_ByteString;
                status = SOPC_UInt16_Read(&nodeId->Namespace, buf);
                if(status == STATUS_OK){
                    status = SOPC_ByteString_Read(&nodeId->Data.Bstring, buf);
                }
                break;
            default:
                status = STATUS_INVALID_PARAMETERS;
        }
    }

    return status;
}


SOPC_StatusCode SOPC_NodeId_ReadAux(void* value, SOPC_Buffer* buf)
{
    return SOPC_NodeId_Read((SOPC_NodeId*) value, buf);
}

SOPC_StatusCode SOPC_NodeId_Read(SOPC_NodeId* nodeId, SOPC_Buffer* buf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_Byte encodingByte = 0;
    if(nodeId != NULL){
        status = Internal_NodeId_Read(buf, nodeId, &encodingByte);
    }
    return status;
}


SOPC_StatusCode SOPC_ExpandedNodeId_WriteAux(void* value, SOPC_Buffer* buf){
    return SOPC_ExpandedNodeId_Write((SOPC_ExpandedNodeId*) value, buf);
}

SOPC_StatusCode SOPC_ExpandedNodeId_Write(const SOPC_ExpandedNodeId* expNodeId, SOPC_Buffer* buf){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_Byte encodingByte = 0xFF;
    if(expNodeId != NULL){
        encodingByte = GetNodeIdDataEncoding(&expNodeId->NodeId);
        if(expNodeId->NamespaceUri.Length > 0){
            encodingByte |= NodeIdEncoding_NamespaceUriFlag;
        }
        if(expNodeId->ServerIndex > 0){
            encodingByte |= NodeIdEncoding_ServerIndexFlag;
        }
        status = Internal_NodeId_Write(buf, encodingByte, &expNodeId->NodeId);
    }
    if(status == STATUS_OK && expNodeId->NamespaceUri.Length > 0)
    {
        status = SOPC_String_Write(&expNodeId->NamespaceUri, buf);
    }
    if(status == STATUS_OK && expNodeId->ServerIndex > 0)
    {
        status = SOPC_UInt32_Write(&expNodeId->ServerIndex, buf);
    }

    return status;
}


SOPC_StatusCode SOPC_ExpandedNodeId_ReadAux(void* value, SOPC_Buffer* buf){
    return SOPC_ExpandedNodeId_Read((SOPC_ExpandedNodeId*) value, buf);
}

SOPC_StatusCode SOPC_ExpandedNodeId_Read(SOPC_ExpandedNodeId* expNodeId, SOPC_Buffer* buf){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_Byte encodingByte = 0;
    if(expNodeId != NULL){
        status = Internal_NodeId_Read(buf, &expNodeId->NodeId, &encodingByte);
    }

    if(status == STATUS_OK &&
       (encodingByte & NodeIdEncoding_NamespaceUriFlag) != 0x00)
    {
        status = SOPC_String_Read(&expNodeId->NamespaceUri, buf);
    }

    if(status == STATUS_OK &&
        (encodingByte & NodeIdEncoding_ServerIndexFlag) != 0)
    {
        status = SOPC_UInt32_Read(&expNodeId->ServerIndex, buf);
    }

    if(status != STATUS_OK && expNodeId != NULL){
        SOPC_NodeId_Clear(&expNodeId->NodeId);
        SOPC_String_Clear(&expNodeId->NamespaceUri);
        // No clear for last since it should manage it itself in case of failure
    }

    return status;
}


SOPC_StatusCode SOPC_StatusCode_WriteAux(void* value, SOPC_Buffer* buf){
    return SOPC_StatusCode_Write((SOPC_StatusCode*) value, buf);
}

SOPC_StatusCode SOPC_StatusCode_Write(const SOPC_StatusCode* status, SOPC_Buffer* buf){
    return SOPC_UInt32_Write(status, buf);
}


SOPC_StatusCode SOPC_StatusCode_ReadAux(void* value, SOPC_Buffer* buf){
    return SOPC_StatusCode_Read((SOPC_StatusCode*) value, buf);
}

SOPC_StatusCode SOPC_StatusCode_Read(SOPC_StatusCode* status, SOPC_Buffer* buf){
    return SOPC_UInt32_Read(status, buf);
}

SOPC_Byte GetDiagInfoEncodingByte(const SOPC_DiagnosticInfo* diagInfo){
    assert(diagInfo != NULL);
    SOPC_Byte encodingByte = 0x00;
    if(diagInfo->SymbolicId > -1){
        encodingByte |= DiagInfoEncoding_SymbolicId;
    }
    if(diagInfo->NamespaceUri > -1){
        encodingByte |= DiagInfoEncoding_Namespace;
    }
    if(diagInfo->Locale > -1){
        encodingByte |= DiagInfoEncoding_Locale;
    }
    if(diagInfo->LocalizedText > -1){
        encodingByte |= DiagInfoEncoding_LocalizedTest;
    }
    if(diagInfo->AdditionalInfo.Length > 0){
        encodingByte |= DiagInfoEncoding_AdditionalInfo;
    }
    if(diagInfo->InnerStatusCode > 0){ // OK status code does not provide information
        encodingByte |= DiagInfoEncoding_InnerStatusCode;
    }
    if(diagInfo->InnerDiagnosticInfo != NULL){
        encodingByte |= DiagInfoEncoding_InnerDianosticInfo;
    }
    return encodingByte;
}


SOPC_StatusCode SOPC_DiagnosticInfo_WriteAux(void* value, SOPC_Buffer* buf){
    return SOPC_DiagnosticInfo_Write((SOPC_DiagnosticInfo*) value, buf);
}

SOPC_StatusCode SOPC_DiagnosticInfo_Write(const SOPC_DiagnosticInfo* diagInfo, SOPC_Buffer* buf){
    SOPC_Byte encodingByte = 0x00;
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(diagInfo != NULL){
        status = STATUS_OK;
        encodingByte = GetDiagInfoEncodingByte(diagInfo);
    }
    if(status == STATUS_OK){
        SOPC_Byte_Write(&encodingByte, buf);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_SymbolicId) != 0x00){
        status = SOPC_Int32_Write(&diagInfo->SymbolicId, buf);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_Namespace) != 0x00){
        status = SOPC_Int32_Write(&diagInfo->NamespaceUri, buf);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_Locale) != 0x00){
        status = SOPC_Int32_Write(&diagInfo->Locale, buf);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_LocalizedTest) != 0x00){
        status = SOPC_Int32_Write(&diagInfo->LocalizedText, buf);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_AdditionalInfo) != 0x00){
        status = SOPC_String_Write(&diagInfo->AdditionalInfo, buf);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_InnerStatusCode) != 0x00){
        status = SOPC_StatusCode_Write(&diagInfo->InnerStatusCode, buf);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_InnerDianosticInfo) != 0x00){
        status = SOPC_DiagnosticInfo_Write(diagInfo->InnerDiagnosticInfo, buf);
    }
    return status;
}


SOPC_StatusCode SOPC_DiagnosticInfo_ReadAux(void* value, SOPC_Buffer* buf){
    return SOPC_DiagnosticInfo_Read((SOPC_DiagnosticInfo*) value, buf);
}

SOPC_StatusCode SOPC_DiagnosticInfo_Read(SOPC_DiagnosticInfo* diagInfo, SOPC_Buffer* buf){
    SOPC_Byte encodingByte = 0x00;
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(diagInfo != NULL){
        status  = SOPC_Byte_Read(&encodingByte, buf);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_SymbolicId) != 0x00){
        status = SOPC_Int32_Read(&diagInfo->SymbolicId, buf);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_Namespace) != 0x00){
        status = SOPC_Int32_Read(&diagInfo->NamespaceUri, buf);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_Locale) != 0x00){
        status = SOPC_Int32_Read(&diagInfo->Locale, buf);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_LocalizedTest) != 0x00){
        status = SOPC_Int32_Read(&diagInfo->LocalizedText, buf);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_AdditionalInfo) != 0x00){
        status = SOPC_String_Read(&diagInfo->AdditionalInfo, buf);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_InnerStatusCode) != 0x00){
        status = SOPC_StatusCode_Read(&diagInfo->InnerStatusCode, buf);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_InnerDianosticInfo) != 0x00){
        status = SOPC_DiagnosticInfo_Read(diagInfo->InnerDiagnosticInfo, buf);
    }
    if(status != STATUS_OK && diagInfo != NULL){
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


SOPC_StatusCode SOPC_QualifiedName_WriteAux(void* value, SOPC_Buffer* buf){
    return SOPC_QualifiedName_Write((SOPC_QualifiedName*) value, buf);
}

SOPC_StatusCode SOPC_QualifiedName_Write(const SOPC_QualifiedName* qname, SOPC_Buffer* buf){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(qname != NULL){
        status = SOPC_UInt16_Write(&qname->NamespaceIndex, buf);
    }
    if(status == STATUS_OK){
        status = SOPC_String_Write(&qname->Name, buf);
    }
    return status;
}


SOPC_StatusCode SOPC_QualifiedName_ReadAux(void* value, SOPC_Buffer* buf){
    return SOPC_QualifiedName_Read((SOPC_QualifiedName*) value, buf);
}

SOPC_StatusCode SOPC_QualifiedName_Read(SOPC_QualifiedName* qname, SOPC_Buffer* buf){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(qname != NULL){
        status = SOPC_UInt16_Read(&qname->NamespaceIndex, buf);
    }
    if(status == STATUS_OK){
        status = SOPC_String_Read(&qname->Name, buf);
    }

    if(status != STATUS_OK && qname != NULL){
        SOPC_UInt16_Clear(&qname->NamespaceIndex);
        // No clear for last since it should manage it itself in case of failure
    }

    return status;
}


SOPC_Byte GetLocalizedTextEncodingByte(const SOPC_LocalizedText* ltext){
    assert(ltext != NULL);
    SOPC_Byte encodingByte = 0;
    if(ltext->Locale.Length > 0){
        encodingByte |= SOPC_LocalizedText_Locale;
    }
    if(ltext->Text.Length > 0){
        encodingByte |= SOPC_LocalizedText_Text;
    }
    return encodingByte;
}


SOPC_StatusCode SOPC_LocalizedText_WriteAux(void* value, SOPC_Buffer* buf){
    return SOPC_LocalizedText_Write((SOPC_LocalizedText*) value, buf);
}

SOPC_StatusCode SOPC_LocalizedText_Write(const SOPC_LocalizedText* localizedText, SOPC_Buffer* buf){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_Byte encodingByte = 0;
    if(localizedText != NULL){
        status = STATUS_OK;
        encodingByte = GetLocalizedTextEncodingByte(localizedText);
        status = SOPC_Byte_Write(&encodingByte, buf);
    }
    if(status == STATUS_OK && (encodingByte & SOPC_LocalizedText_Locale) != 0){
        status = SOPC_String_Write(&localizedText->Locale, buf);
    }
    if(status == STATUS_OK && (encodingByte & SOPC_LocalizedText_Text) != 0){
        status = SOPC_String_Write(&localizedText->Text, buf);
    }
    return status;
}


SOPC_StatusCode SOPC_LocalizedText_ReadAux(void* value, SOPC_Buffer* buf){
    return SOPC_LocalizedText_Read((SOPC_LocalizedText*) value, buf);
}

SOPC_StatusCode SOPC_LocalizedText_Read(SOPC_LocalizedText* localizedText, SOPC_Buffer* buf){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_Byte encodingByte = 0;
    if(localizedText != NULL){
        status = SOPC_Byte_Read(&encodingByte, buf);
    }
    if(status == STATUS_OK && (encodingByte & SOPC_LocalizedText_Locale) != 0){
        status = SOPC_String_Read(&localizedText->Locale, buf);
    }
    if(status == STATUS_OK && (encodingByte & SOPC_LocalizedText_Text) != 0){
        status = SOPC_String_Read(&localizedText->Text, buf);
    }

    if(status != STATUS_OK && localizedText != NULL){
        SOPC_String_Clear(&localizedText->Locale);
        // No clear for last since it should manage it itself in case of failure
    }

    return status;
}


SOPC_StatusCode SOPC_ExtensionObject_WriteAux(void* value, SOPC_Buffer* buf){
    return SOPC_ExtensionObject_Write((SOPC_ExtensionObject*) value, buf);
}

SOPC_StatusCode SOPC_ExtensionObject_Write(const SOPC_ExtensionObject* extObj, SOPC_Buffer* buf){
    const int32_t tmpLength = -1;
    SOPC_NodeId nodeId;
    uint32_t lengthPos;
    uint32_t curPos;
    int32_t length;
    uint16_t nsIndex = OPCUA_NAMESPACE_INDEX;
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_NamespaceTable* nsTable = SOPC_StackConfiguration_GetNamespaces();
    SOPC_Byte encodingByte = 0;
    if(extObj != NULL){
        encodingByte = extObj->Encoding;
        nodeId = extObj->TypeId.NodeId;
        status = STATUS_OK;
    }

    if(status == STATUS_OK &&
       encodingByte == SOPC_ExtObjBodyEncoding_Object)
    {
        encodingByte = SOPC_ExtObjBodyEncoding_ByteString;
        if(extObj->Body.Object.ObjType == NULL){
            status = STATUS_INVALID_PARAMETERS;
        }else{
            status = Namespace_GetIndex(nsTable, extObj->Body.Object.ObjType->NamespaceUri, &nsIndex);

            nodeId.IdentifierType = IdentifierType_Numeric;
            nodeId.Namespace = nsIndex;
            nodeId.Data.Numeric = extObj->Body.Object.ObjType->BinaryEncodingTypeId;
        }
    }

    if(status == STATUS_OK){
        status = SOPC_NodeId_Write(&nodeId, buf);
    }

    if(status == STATUS_OK){
        status = SOPC_Byte_Write(&encodingByte, buf);
    }

    if(status == STATUS_OK){
        switch(extObj->Encoding){
            case SOPC_ExtObjBodyEncoding_None:
                break;
            case SOPC_ExtObjBodyEncoding_ByteString:
                status = SOPC_ByteString_Write(&extObj->Body.Bstring, buf);
                break;
            case SOPC_ExtObjBodyEncoding_XMLElement:
                status = SOPC_XmlElement_Write(&extObj->Body.Xml, buf);
                break;
            case SOPC_ExtObjBodyEncoding_Object:
                lengthPos = buf->position;
                status = SOPC_Int32_Write(&tmpLength, buf);
                if(status == STATUS_OK){
                    status = extObj->Body.Object.ObjType->Encode(extObj->Body.Object.Value, buf);
                }
                if(status == STATUS_OK){
                    // Go backward to write correct length value
                    curPos = buf->position;
                    length = curPos - (lengthPos + 4);
                    SOPC_Buffer_SetPosition(buf, lengthPos);
                    SOPC_Int32_Write(&length, buf);
                    SOPC_Buffer_SetPosition(buf, curPos);
                }
                break;
        }
    }

    return status;
}


SOPC_StatusCode SOPC_ExtensionObject_ReadAux(void* value, SOPC_Buffer* buf){
    return SOPC_ExtensionObject_Read((SOPC_ExtensionObject*) value, buf);
}

SOPC_StatusCode SOPC_ExtensionObject_Read(SOPC_ExtensionObject* extObj, SOPC_Buffer* buf){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_EncodeableType* encType = NULL;
    SOPC_NamespaceTable* nsTable = SOPC_StackConfiguration_GetNamespaces();
    SOPC_EncodeableType** encTypeTable = SOPC_StackConfiguration_GetEncodeableTypes();
    const char* nsName;
    uint8_t nsFound = FALSE;
    SOPC_Byte encodingByte = 0;
    if(extObj != NULL){
        status = SOPC_NodeId_Read(&extObj->TypeId.NodeId, buf);
    }
    if(status == STATUS_OK){
        status = SOPC_Byte_Read(&encodingByte, buf);
    }

    if(status == STATUS_OK){
        // Manage Object body decoding
        if(encodingByte == SOPC_ExtObjBodyEncoding_ByteString){
            // Object provided as a byte string, check if encoded object is a known type
            if(extObj->TypeId.NodeId.IdentifierType == IdentifierType_Numeric){
                if(extObj->TypeId.NodeId.Namespace != OPCUA_NAMESPACE_INDEX){
                    nsName = Namespace_GetName(nsTable, extObj->TypeId.NodeId.Namespace);
                    if(nsName != NULL){
                        nsFound = 1; // TRUE
                    }
                }else{
                    nsName = NULL; // <=> OPCUA_NAMESPACE_NAME in GetEncodeableType
                    nsFound = 1; // TRUE
                }
                if(nsFound != FALSE){
                    encType = SOPC_EncodeableType_GetEncodeableType(encTypeTable,
                                                                    nsName,
                                                                    extObj->TypeId.NodeId.Data.Numeric);
                }
                if(nsFound == FALSE || encType == NULL){
                    // Keep as a byte string since it is unknown object
                    encodingByte = SOPC_ExtObjBodyEncoding_ByteString;
                }else{
                    encodingByte = SOPC_ExtObjBodyEncoding_Object;
                    extObj->Body.Object.ObjType = encType;
                    SOPC_String_AttachFromCstring(&extObj->TypeId.NamespaceUri, encType->NamespaceUri);
                }
            }else{
                status = STATUS_INVALID_RCV_PARAMETER;
            }
        }else if(encodingByte == SOPC_ExtObjBodyEncoding_Object){
            // Invalid value encoded, it does not exist as a valid binary encoding value
            status = STATUS_INVALID_RCV_PARAMETER;
        }
    }

    if(status == STATUS_OK){
        switch(encodingByte){
            case SOPC_ExtObjBodyEncoding_None:
                extObj->Length = -1;
                break;
            case SOPC_ExtObjBodyEncoding_ByteString:
                status = SOPC_ByteString_Read(&extObj->Body.Bstring, buf);
                break;
            case SOPC_ExtObjBodyEncoding_XMLElement:
                status = SOPC_XmlElement_Read(&extObj->Body.Xml, buf);
                break;
            case SOPC_ExtObjBodyEncoding_Object:
                status = SOPC_Int32_Read(&extObj->Length, buf);
                if(status == STATUS_OK){
                    extObj->Body.Object.Value = malloc(extObj->Body.Object.ObjType->AllocationSize);
                    extObj->Body.Object.ObjType->Initialize(extObj->Body.Object.Value);
                    status = extObj->Body.Object.ObjType->Decode(extObj->Body.Object.Value, buf);
                    if(STATUS_OK != status){
                        free(extObj->Body.Object.Value);
                        extObj->Body.Object.Value = NULL;
                    }
                }
                break;
            default:
                status = STATUS_INVALID_RCV_PARAMETER;
        }
        if(status == STATUS_OK){
            extObj->Encoding = encodingByte;
        }
    }

    if(status != STATUS_OK && extObj != NULL){
        SOPC_NodeId_Clear(&extObj->TypeId.NodeId);
        // No clear for last since it should manage it itself in case of failure
    }

    return status;
}

SOPC_Byte GetVariantEncodingMask(const SOPC_Variant* variant){
    assert(variant != NULL);
    SOPC_Byte encodingByte = variant->BuiltInTypeId;
    if(variant->ArrayType == SOPC_VariantArrayType_Matrix){
        encodingByte |= SOPC_VariantArrayMatrixFlag;
    }
    if(variant->ArrayType == SOPC_VariantArrayType_Array){
        encodingByte |= SOPC_VariantArrayValueFlag;
    }
    return encodingByte;
}

SOPC_StatusCode WriteVariantNonArrayBuiltInType(SOPC_Buffer*         buf,
                                                SOPC_BuiltinId          builtInTypeId,
                                                const SOPC_VariantValue *val)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    switch(builtInTypeId){
        case SOPC_Boolean_Id:
            status = SOPC_Boolean_Write(&val->Boolean, buf);
            break;
        case SOPC_SByte_Id:
            status = SOPC_SByte_Write(&val->Sbyte, buf);
            break;
        case SOPC_Byte_Id:
            status = SOPC_Byte_Write(&val->Byte, buf);
            break;
        case SOPC_Int16_Id:
            status = SOPC_Int16_Write(&val->Int16, buf);
            break;
        case SOPC_UInt16_Id:
            status = SOPC_UInt16_Write(&val->Uint16, buf);
            break;
        case SOPC_Int32_Id:
            status = SOPC_Int32_Write(&val->Int32, buf);
            break;
        case SOPC_UInt32_Id:
            status = SOPC_UInt32_Write(&val->Uint32, buf);
            break;
        case SOPC_Int64_Id:
            status = SOPC_Int64_Write(&val->Int64, buf);
            break;
        case SOPC_UInt64_Id:
            status = SOPC_UInt64_Write(&val->Uint64, buf);
            break;
        case SOPC_Float_Id:
            status = SOPC_Float_Write(&val->Floatv, buf);
            break;
        case SOPC_Double_Id:
            status = SOPC_Double_Write(&val->Doublev, buf);
            break;
        case SOPC_String_Id:
            status = SOPC_String_Write(&val->String, buf);
            break;
        case SOPC_DateTime_Id:
            status = SOPC_DateTime_Write(&val->Date, buf);
            break;
        case SOPC_Guid_Id:
            status = SOPC_Guid_Write(val->Guid, buf);
            break;
        case SOPC_ByteString_Id:
            status = SOPC_ByteString_Write(&val->Bstring, buf);
            break;
        case SOPC_XmlElement_Id:
            status = SOPC_XmlElement_Write(&val->XmlElt, buf);
            break;
        case SOPC_NodeId_Id:
            status = SOPC_NodeId_Write(val->NodeId, buf);
            break;
        case SOPC_ExpandedNodeId_Id:
            status = SOPC_ExpandedNodeId_Write(val->ExpNodeId, buf);
            break;
        case SOPC_StatusCode_Id:
            status = SOPC_StatusCode_Write(&val->Status, buf);
            break;
        case SOPC_QualifiedName_Id:
            status = SOPC_QualifiedName_Write(val->Qname, buf);
            break;
        case SOPC_LocalizedText_Id:
            status = SOPC_LocalizedText_Write(val->LocalizedText, buf);
            break;
        case SOPC_ExtensionObject_Id:
            status = SOPC_ExtensionObject_Write(val->ExtObject, buf);
            break;
        case SOPC_DataValue_Id:
            status = SOPC_DataValue_Write(val->DataValue, buf);
            break;
        case SOPC_Variant_Id:
            // Part 6 Table 14 (v1.03): "The value shall not be a Variant
            //                           but it could be an array of Variants."
            status = STATUS_INVALID_PARAMETERS;
            break;
        case SOPC_DiagnosticInfo_Id:
            status = SOPC_DiagnosticInfo_Write(val->DiagInfo, buf);
            break;
        case SOPC_Null_Id:
            // mantis #0003682: errata for 1.03: NULL value encoding (no value to encode)
            status = STATUS_OK;
            break;
        default:
            // STATUS = INVALID PARAM
            break;
    }
    return status;
}

SOPC_StatusCode WriteVariantArrayBuiltInType(SOPC_Buffer*               buf,
                                             SOPC_BuiltinId                builtInTypeId,
                                             const SOPC_VariantArrayValue* array,
                                             int32_t*                      length)
{
    SOPC_StatusCode status = STATUS_OK;
    switch(builtInTypeId){
    case SOPC_Boolean_Id:
                status = SOPC_Write_Array(buf, length, (void**) &array->BooleanArr, sizeof(SOPC_Boolean), SOPC_Boolean_WriteAux);
                break;
            case SOPC_SByte_Id:
                status = SOPC_Write_Array(buf, length, (void**) &array->SbyteArr, sizeof(SOPC_SByte), SOPC_SByte_WriteAux);
                break;
            case SOPC_Byte_Id:
                status = SOPC_Write_Array(buf, length, (void**) &array->ByteArr, sizeof(SOPC_Byte), SOPC_Byte_WriteAux);
                break;
            case SOPC_Int16_Id:
                status = SOPC_Write_Array(buf, length, (void**) &array->Int16Arr, sizeof(int16_t), SOPC_Int16_WriteAux);
                break;
            case SOPC_UInt16_Id:
                status = SOPC_Write_Array(buf, length, (void**) &array->Uint16Arr, sizeof(uint16_t), SOPC_UInt16_WriteAux);
                break;
            case SOPC_Int32_Id:
                status = SOPC_Write_Array(buf, length, (void**) &array->Int32Arr, sizeof(int32_t), SOPC_Int32_WriteAux);
                break;
            case SOPC_UInt32_Id:
                status = SOPC_Write_Array(buf, length, (void**) &array->Uint32Arr, sizeof(uint32_t), SOPC_UInt32_WriteAux);
                break;
            case SOPC_Int64_Id:
                status = SOPC_Write_Array(buf, length, (void**) &array->Int64Arr, sizeof(int64_t), SOPC_Int64_WriteAux);
                break;
            case SOPC_UInt64_Id:
                status = SOPC_Write_Array(buf, length, (void**) &array->Uint64Arr, sizeof(uint64_t), SOPC_UInt64_WriteAux);
                break;
            case SOPC_Float_Id:
                status = SOPC_Write_Array(buf, length, (void**) &array->FloatvArr, sizeof(float), SOPC_Float_WriteAux);
                break;
            case SOPC_Double_Id:
                status = SOPC_Write_Array(buf, length, (void**) &array->DoublevArr, sizeof(double), SOPC_Double_WriteAux);
                break;
            case SOPC_String_Id:
                status = SOPC_Write_Array(buf, length, (void**) &array->StringArr, sizeof(SOPC_String), SOPC_String_WriteAux);
                break;
            case SOPC_DateTime_Id:
                status = SOPC_Write_Array(buf, length, (void**) &array->DateArr, sizeof(SOPC_DateTime), SOPC_DateTime_WriteAux);
                break;
            case SOPC_Guid_Id:
                status = SOPC_Write_Array(buf, length, (void**) &array->GuidArr, sizeof(SOPC_Guid), SOPC_Guid_WriteAux);
                break;
            case SOPC_ByteString_Id:
                status = SOPC_Write_Array(buf, length, (void**) &array->BstringArr, sizeof(SOPC_ByteString), SOPC_ByteString_WriteAux);
                break;
            case SOPC_XmlElement_Id:
                status = SOPC_Write_Array(buf, length, (void**) &array->XmlEltArr, sizeof(SOPC_XmlElement), SOPC_XmlElement_WriteAux);
                break;
            case SOPC_NodeId_Id:
                status = SOPC_Write_Array(buf, length, (void**) &array->NodeIdArr, sizeof(SOPC_NodeId), SOPC_NodeId_WriteAux);
                break;
            case SOPC_ExpandedNodeId_Id:
                status = SOPC_Write_Array(buf, length, (void**) &array->ExpNodeIdArr, sizeof(SOPC_ExpandedNodeId), SOPC_ExpandedNodeId_WriteAux);
                break;
            case SOPC_StatusCode_Id:
                status = SOPC_Write_Array(buf, length, (void**) &array->StatusArr, sizeof(SOPC_StatusCode), SOPC_StatusCode_WriteAux);
                break;
            case SOPC_QualifiedName_Id:
                status = SOPC_Write_Array(buf, length, (void**) &array->QnameArr, sizeof(SOPC_QualifiedName), SOPC_QualifiedName_WriteAux);
                break;
            case SOPC_LocalizedText_Id:
                status = SOPC_Write_Array(buf, length, (void**) &array->LocalizedTextArr, sizeof(SOPC_LocalizedText), SOPC_LocalizedText_WriteAux);
                break;
            case SOPC_ExtensionObject_Id:
                status = SOPC_Write_Array(buf, length, (void**) &array->ExtObjectArr, sizeof(SOPC_ExtensionObject), SOPC_ExtensionObject_WriteAux);
                break;
            case SOPC_DataValue_Id:
                status = SOPC_Write_Array(buf, length, (void**) &array->DataValueArr, sizeof(SOPC_DataValue), SOPC_DataValue_WriteAux);
                break;
            case SOPC_Variant_Id:
                status = SOPC_Write_Array(buf, length, (void**) &array->VariantArr, sizeof(SOPC_Variant), SOPC_Variant_WriteAux);
                break;
            case SOPC_DiagnosticInfo_Id:
                status = SOPC_Write_Array(buf, length, (void**) &array->DiagInfoArr, sizeof(SOPC_DiagnosticInfo), SOPC_DiagnosticInfo_WriteAux);
                break;
        default:
            status = STATUS_INVALID_PARAMETERS;
            break;
    }
    return status;
}


SOPC_StatusCode SOPC_Variant_WriteAux(void* value, SOPC_Buffer* buf){
    return SOPC_Variant_Write((SOPC_Variant*) value, buf);
}

SOPC_StatusCode SOPC_Variant_Write(const SOPC_Variant* variant, SOPC_Buffer* buf){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_Byte encodingByte = 0;
    int32_t arrayLength = 0;
    int64_t matrixLength = 1;
    int32_t idx = 0;

    if(variant != NULL){
        encodingByte = GetVariantEncodingMask(variant);
        status = SOPC_Byte_Write(&encodingByte, buf);
    }
    if(STATUS_OK == status){
        switch(variant->ArrayType){
            case SOPC_VariantArrayType_SingleValue:
                status = WriteVariantNonArrayBuiltInType(buf,
                                                         variant->BuiltInTypeId,
                                                         &variant->Value);
                break;
            case SOPC_VariantArrayType_Array:
                arrayLength = variant->Value.Array.Length;
                // Note: array length written in WriteVariantArrayBuiltInType
                if(STATUS_OK == status){
                    if(arrayLength < 0){
                        status = OpcUa_BadEncodingError;
                    }else{
                        status = WriteVariantArrayBuiltInType(buf,
                                                              variant->BuiltInTypeId,
                                                              &variant->Value.Array.Content,
                                                              &arrayLength);
                    }
                }
                break;
            case SOPC_VariantArrayType_Matrix:
                matrixLength = 1;
                if(variant->Value.Matrix.Dimensions == 0){
                    matrixLength = 0;
                }
                for(idx = 0; idx < variant->Value.Matrix.Dimensions && STATUS_OK == status; idx ++){
                    if(variant->Value.Matrix.ArrayDimensions[idx] > 0 &&
                       matrixLength * variant->Value.Matrix.ArrayDimensions[idx] <= INT32_MAX){
                        matrixLength *= variant->Value.Matrix.ArrayDimensions[idx];
                    }else{
                        status = OpcUa_BadEncodingError;
                    }
                }
                // Note: array length written in WriteVariantArrayBuiltInType
                if(STATUS_OK == status){
                    arrayLength = (int32_t) matrixLength;
                    status = WriteVariantArrayBuiltInType(buf,
                                                          variant->BuiltInTypeId,
                                                          &variant->Value.Matrix.Content,
                                                          &arrayLength);
                }
                // Encode dimension array
                if(STATUS_OK == status){
                    // length
                    status = SOPC_Int32_Write(&variant->Value.Matrix.Dimensions, buf);
                }
                if(STATUS_OK == status){
                    // array
                    int32_t idx = 0;
                    for(idx = 0; idx < variant->Value.Matrix.Dimensions; idx ++){
                        status = SOPC_Int32_Write(&variant->Value.Matrix.ArrayDimensions[idx], buf);
                    }
                }
                break;
        }
    }
    return status;
}

SOPC_StatusCode ReadVariantNonArrayBuiltInType(SOPC_Buffer*   buf,
                                               SOPC_BuiltinId    builtInTypeId,
                                               SOPC_VariantValue *val)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    switch(builtInTypeId){
        case SOPC_Boolean_Id:
            status = SOPC_Boolean_Read(&val->Boolean, buf);
            break;
        case SOPC_SByte_Id:
            status = SOPC_SByte_Read(&val->Sbyte, buf);
            break;
        case SOPC_Byte_Id:
            status = SOPC_Byte_Read(&val->Byte, buf);
            break;
        case SOPC_Int16_Id:
            status = SOPC_Int16_Read(&val->Int16, buf);
            break;
        case SOPC_UInt16_Id:
            status = SOPC_UInt16_Read(&val->Uint16, buf);
            break;
        case SOPC_Int32_Id:
            status = SOPC_Int32_Read(&val->Int32, buf);
            break;
        case SOPC_UInt32_Id:
            status = SOPC_UInt32_Read(&val->Uint32, buf);
            break;
        case SOPC_Int64_Id:
            status = SOPC_Int64_Read(&val->Int64, buf);
            break;
        case SOPC_UInt64_Id:
            status = SOPC_UInt64_Read(&val->Uint64, buf);
            break;
        case SOPC_Float_Id:
            status = SOPC_Float_Read(&val->Floatv, buf);
            break;
        case SOPC_Double_Id:
            status = SOPC_Double_Read(&val->Doublev, buf);
            break;
        case SOPC_String_Id:
            status = SOPC_String_Read(&val->String, buf);
            break;
        case SOPC_DateTime_Id:
            status = SOPC_DateTime_Read(&val->Date, buf);
            break;
        case SOPC_Guid_Id:
            val->Guid = malloc(sizeof(SOPC_Guid));
            if(val->Guid != NULL){
                SOPC_Guid_Initialize(val->Guid);
                status = SOPC_Guid_Read(val->Guid, buf);
                if(status != STATUS_OK){
                    free(val->Guid);
                    val->Guid = NULL;
                }
            }else{
                status = STATUS_NOK;
            }
            break;
        case SOPC_ByteString_Id:
            status = SOPC_ByteString_Read(&val->Bstring, buf);
            break;
        case SOPC_XmlElement_Id:
            status = SOPC_XmlElement_Read(&val->XmlElt, buf);
            break;
        case SOPC_NodeId_Id:
            val->NodeId = malloc(sizeof(SOPC_NodeId));
            if(val->NodeId != NULL){
                SOPC_NodeId_Initialize(val->NodeId);
                status = SOPC_NodeId_Read(val->NodeId, buf);
                if(status != STATUS_OK){
                    free(val->NodeId);
                    val->NodeId = NULL;
                }
            }else{
                status = STATUS_NOK;
            }
            break;
        case SOPC_ExpandedNodeId_Id:
            val->ExpNodeId = malloc(sizeof(SOPC_ExpandedNodeId));
            if(val->ExpNodeId != NULL){
                SOPC_ExpandedNodeId_Initialize(val->ExpNodeId);
                status = SOPC_ExpandedNodeId_Read(val->ExpNodeId, buf);
                if(status != STATUS_OK){
                    free(val->ExpNodeId);
                    val->ExpNodeId = NULL;
                }
            }else{
                status = STATUS_NOK;
            }
            break;
        case SOPC_StatusCode_Id:
            status = SOPC_StatusCode_Read(&val->Status, buf);
            break;
        case SOPC_QualifiedName_Id:
            val->Qname = malloc(sizeof(SOPC_QualifiedName));
            if(val->Qname != NULL){
                SOPC_QualifiedName_Initialize(val->Qname);
                status = SOPC_QualifiedName_Read(val->Qname, buf);
                if(status != STATUS_OK){
                    free(val->Qname);
                    val->Qname = NULL;
                }
            }else{
                status = STATUS_NOK;
            }
            break;
        case SOPC_LocalizedText_Id:
            val->LocalizedText = malloc(sizeof(SOPC_LocalizedText));
            if(val->LocalizedText != NULL){
                SOPC_LocalizedText_Initialize(val->LocalizedText);
                status = SOPC_LocalizedText_Read(val->LocalizedText, buf);
                if(status != STATUS_OK){
                    free(val->LocalizedText);
                    val->LocalizedText = NULL;
                }
            }else{
                status = STATUS_NOK;
            }
            break;
        case SOPC_ExtensionObject_Id:
            val->ExtObject = malloc(sizeof(SOPC_ExtensionObject));
            if(val->ExtObject != NULL){
                SOPC_ExtensionObject_Initialize(val->ExtObject);
                status = SOPC_ExtensionObject_Read(val->ExtObject, buf);
                if(status != STATUS_OK){
                    free(val->ExtObject);
                    val->ExtObject = NULL;
                }
            }else{
                status = STATUS_NOK;
            }
            break;
        case SOPC_DataValue_Id:
            val->DataValue = malloc(sizeof(SOPC_DataValue));
            if(val->DataValue != NULL){
                SOPC_DataValue_Initialize(val->DataValue);
                status = SOPC_DataValue_Read(val->DataValue, buf);
                if(status != STATUS_OK){
                    free(val->DataValue);
                    val->DataValue = NULL;
                }
            }else{
                status = STATUS_NOK;
            }
            break;
        case SOPC_Variant_Id:
            // Part 6 Table 14 (v1.03): "The value shall not be a Variant
            //                           but it could be an array of Variants."
            status = STATUS_INVALID_RCV_PARAMETER;
            break;
        case SOPC_DiagnosticInfo_Id:
            val->DiagInfo = malloc(sizeof(SOPC_DiagnosticInfo));
            if(val->DiagInfo != NULL){
                SOPC_DiagnosticInfo_Initialize(val->DiagInfo);
                status = SOPC_DiagnosticInfo_Read(val->DiagInfo, buf);
                if(status != STATUS_OK){
                    free(val->DiagInfo);
                    val->DiagInfo = NULL;
                }
            }else{
                status = STATUS_NOK;
            }
            break;
        case SOPC_Null_Id:
            // mantis #0003682: errata for 1.03: NULL value encoding (no value to decode)
            status = STATUS_OK;
            break;
        default:
            status = STATUS_NOK;
            break;
    }
    return status;
}

SOPC_StatusCode ReadVariantArrayBuiltInType(SOPC_Buffer*         buf,
                                            SOPC_BuiltinId          builtInTypeId,
                                            SOPC_VariantArrayValue* array,
                                            int32_t*                length)
{
    SOPC_StatusCode status = STATUS_OK;
    switch(builtInTypeId){
        case SOPC_Boolean_Id:
            status = SOPC_Read_Array(buf, length, (void**) &array->BooleanArr, sizeof(SOPC_Boolean), SOPC_Boolean_ReadAux, SOPC_Boolean_InitializeAux, SOPC_Boolean_ClearAux);
            break;
        case SOPC_SByte_Id:
            status = SOPC_Read_Array(buf, length, (void**) &array->SbyteArr, sizeof(SOPC_SByte), SOPC_SByte_ReadAux, SOPC_SByte_InitializeAux, SOPC_SByte_ClearAux);
            break;
        case SOPC_Byte_Id:
            status = SOPC_Read_Array(buf, length, (void**) &array->ByteArr, sizeof(SOPC_Byte), SOPC_Byte_ReadAux, SOPC_Byte_InitializeAux, SOPC_Byte_ClearAux);
            break;
        case SOPC_Int16_Id:
            status = SOPC_Read_Array(buf, length, (void**) &array->Int16Arr, sizeof(int16_t), SOPC_Int16_ReadAux, SOPC_Int16_InitializeAux, SOPC_Int16_ClearAux);
            break;
        case SOPC_UInt16_Id:
            status = SOPC_Read_Array(buf, length, (void**) &array->Uint16Arr, sizeof(uint16_t), SOPC_UInt16_ReadAux, SOPC_UInt16_InitializeAux, SOPC_UInt16_ClearAux);
            break;
        case SOPC_Int32_Id:
            status = SOPC_Read_Array(buf, length, (void**) &array->Int32Arr, sizeof(int32_t), SOPC_Int32_ReadAux, SOPC_Int32_InitializeAux, SOPC_Int32_ClearAux);
            break;
        case SOPC_UInt32_Id:
            status = SOPC_Read_Array(buf, length, (void**) &array->Uint32Arr, sizeof(uint32_t), SOPC_UInt32_ReadAux, SOPC_UInt32_InitializeAux, SOPC_UInt32_ClearAux);
            break;
        case SOPC_Int64_Id:
            status = SOPC_Read_Array(buf, length, (void**) &array->Int64Arr, sizeof(int64_t), SOPC_Int64_ReadAux, SOPC_Int64_InitializeAux, SOPC_Int64_ClearAux);
            break;
        case SOPC_UInt64_Id:
            status = SOPC_Read_Array(buf, length, (void**) &array->Uint64Arr, sizeof(uint64_t), SOPC_UInt64_ReadAux, SOPC_UInt64_InitializeAux, SOPC_UInt64_ClearAux);
            break;
        case SOPC_Float_Id:
            status = SOPC_Read_Array(buf, length, (void**) &array->FloatvArr, sizeof(float), SOPC_Float_ReadAux, SOPC_Float_InitializeAux, SOPC_Float_ClearAux);
            break;
        case SOPC_Double_Id:
            status = SOPC_Read_Array(buf, length, (void**) &array->DoublevArr, sizeof(double), SOPC_Double_ReadAux, SOPC_Double_InitializeAux, SOPC_Double_ClearAux);
            break;
        case SOPC_String_Id:
            status = SOPC_Read_Array(buf, length, (void**) &array->StringArr, sizeof(SOPC_String), SOPC_String_ReadAux, SOPC_String_InitializeAux, SOPC_String_ClearAux);
            break;
        case SOPC_DateTime_Id:
            status = SOPC_Read_Array(buf, length, (void**) &array->DateArr, sizeof(SOPC_DateTime), SOPC_DateTime_ReadAux, SOPC_DateTime_InitializeAux, SOPC_DateTime_ClearAux);
            break;
        case SOPC_Guid_Id:
            status = SOPC_Read_Array(buf, length, (void**) &array->GuidArr, sizeof(SOPC_Guid), SOPC_Guid_ReadAux, SOPC_Guid_InitializeAux, SOPC_Guid_ClearAux);
            break;
        case SOPC_ByteString_Id:
            status = SOPC_Read_Array(buf, length, (void**) &array->BstringArr, sizeof(SOPC_ByteString), SOPC_ByteString_ReadAux, SOPC_ByteString_InitializeAux, SOPC_ByteString_ClearAux);
            break;
        case SOPC_XmlElement_Id:
            status = SOPC_Read_Array(buf, length, (void**) &array->XmlEltArr, sizeof(SOPC_XmlElement), SOPC_XmlElement_ReadAux, SOPC_XmlElement_InitializeAux, SOPC_XmlElement_ClearAux);
            break;
        case SOPC_NodeId_Id:
            status = SOPC_Read_Array(buf, length, (void**) &array->NodeIdArr, sizeof(SOPC_NodeId), SOPC_NodeId_ReadAux, SOPC_NodeId_InitializeAux, SOPC_NodeId_ClearAux);
            break;
        case SOPC_ExpandedNodeId_Id:
            status = SOPC_Read_Array(buf, length, (void**) &array->ExpNodeIdArr, sizeof(SOPC_ExpandedNodeId), SOPC_ExpandedNodeId_ReadAux, SOPC_ExpandedNodeId_InitializeAux, SOPC_ExpandedNodeId_ClearAux);
            break;
        case SOPC_StatusCode_Id:
            status = SOPC_Read_Array(buf, length, (void**) &array->StatusArr, sizeof(SOPC_StatusCode), SOPC_StatusCode_ReadAux, SOPC_StatusCode_InitializeAux, SOPC_StatusCode_ClearAux);
            break;
        case SOPC_QualifiedName_Id:
            status = SOPC_Read_Array(buf, length, (void**) &array->QnameArr, sizeof(SOPC_QualifiedName), SOPC_QualifiedName_ReadAux, SOPC_QualifiedName_InitializeAux, SOPC_QualifiedName_ClearAux);
            break;
        case SOPC_LocalizedText_Id:
            status = SOPC_Read_Array(buf, length, (void**) &array->LocalizedTextArr, sizeof(SOPC_LocalizedText), SOPC_LocalizedText_ReadAux, SOPC_LocalizedText_InitializeAux, SOPC_LocalizedText_ClearAux);
            break;
        case SOPC_ExtensionObject_Id:
            status = SOPC_Read_Array(buf, length, (void**) &array->ExtObjectArr, sizeof(SOPC_ExtensionObject), SOPC_ExtensionObject_ReadAux, SOPC_ExtensionObject_InitializeAux, SOPC_ExtensionObject_ClearAux);
            break;
        case SOPC_DataValue_Id:
            status = SOPC_Read_Array(buf, length, (void**) &array->DataValueArr, sizeof(SOPC_DataValue), SOPC_DataValue_ReadAux, SOPC_DataValue_InitializeAux, SOPC_DataValue_ClearAux);
            break;
        case SOPC_Variant_Id:
            status = SOPC_Read_Array(buf, length, (void**) &array->VariantArr, sizeof(SOPC_Variant), SOPC_Variant_ReadAux, SOPC_Variant_InitializeAux, SOPC_Variant_ClearAux);
            break;
        case SOPC_DiagnosticInfo_Id:
            status = SOPC_Read_Array(buf, length, (void**) &array->DiagInfoArr, sizeof(SOPC_DiagnosticInfo), SOPC_DiagnosticInfo_ReadAux, SOPC_DiagnosticInfo_InitializeAux, SOPC_DiagnosticInfo_ClearAux);
            break;
        default:
	        status = STATUS_NOK;
            break;
    }
    return status;
}


SOPC_StatusCode SOPC_Variant_ReadAux(void* value, SOPC_Buffer* buf){
    return SOPC_Variant_Read((SOPC_Variant*) value, buf);
}

SOPC_StatusCode SOPC_Variant_Read(SOPC_Variant* variant, SOPC_Buffer* buf){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_Byte encodingByte = 0;
    int32_t arrayLength = 0;
    int64_t matrixLength = 1;
    if(variant != NULL){
        status = SOPC_Byte_Read(&encodingByte, buf);
    }
    if(STATUS_OK == status){
        // Retrieve array flags
        if((encodingByte & SOPC_VariantArrayValueFlag) != 0){
            if((encodingByte & SOPC_VariantArrayMatrixFlag) != 0){
                variant->ArrayType = SOPC_VariantArrayType_Matrix;
            }else{
                variant->ArrayType = SOPC_VariantArrayType_Array;
            }
            // Note array length read in ReadVariantArrayBuiltInType
        }else if((encodingByte & SOPC_VariantArrayMatrixFlag) != 0){
            status = STATUS_INVALID_PARAMETERS;
        }
        // Retrieve builtin type id: avoid 2^7 and 2^6 which are array flags
        variant->BuiltInTypeId = 0x3F & encodingByte;
    }

    if(STATUS_OK == status){
        switch(variant->ArrayType){
            case SOPC_VariantArrayType_SingleValue:
                status = ReadVariantNonArrayBuiltInType(buf,
                                                        variant->BuiltInTypeId,
                                                        &variant->Value);
                break;
            case SOPC_VariantArrayType_Array:
                status = ReadVariantArrayBuiltInType(buf,
                                                     variant->BuiltInTypeId,
                                                     &variant->Value.Array.Content,
                                                     &arrayLength);
                variant->Value.Array.Length = arrayLength;
                break;
            case SOPC_VariantArrayType_Matrix:
                status = ReadVariantArrayBuiltInType(buf,
                                                     variant->BuiltInTypeId,
                                                     &variant->Value.Matrix.Content,
                                                     &arrayLength);

                if(STATUS_OK == status && arrayLength < 0){
                    status = OpcUa_BadDecodingError;
                }

                // Decode dimension array
                if(STATUS_OK == status){
                    // length of array
                    status = SOPC_Int32_Read(&variant->Value.Matrix.Dimensions, buf);
                }

                if(STATUS_OK == status && variant->Value.Matrix.Dimensions < 0){
                    status = OpcUa_BadDecodingError;
                }

                if(STATUS_OK == status){
                    // array
                    variant->Value.Matrix.ArrayDimensions = malloc(sizeof(int32_t) * variant->Value.Matrix.Dimensions);
                    if(variant->Value.Matrix.Dimensions == 0){
                        matrixLength = 0;
                    }
                    if(variant->Value.Matrix.ArrayDimensions != NULL){
                        int32_t idx = 0;
                        for(idx = 0; idx < variant->Value.Matrix.Dimensions && STATUS_OK == status; idx ++){
                            status = SOPC_Int32_Read(&variant->Value.Matrix.ArrayDimensions[idx], buf);
                            if(variant->Value.Matrix.ArrayDimensions[idx] > 0){
                                // valid value
                                matrixLength *= variant->Value.Matrix.ArrayDimensions[idx];
                                if(matrixLength > arrayLength){
                                    status = OpcUa_BadDecodingError;
                                }
                            }else{
                                status = OpcUa_BadDecodingError;
                            }
                        }
                        if(STATUS_OK != status){
                            free(variant->Value.Matrix.ArrayDimensions);
                            variant->Value.Matrix.ArrayDimensions = NULL;
                        }
                    }else{
                        status = STATUS_NOK;
                    }
                }

                if(STATUS_OK == status && matrixLength != arrayLength){
                    // Dimensions length and total length are not equal
                    status = OpcUa_BadDecodingError;
                }

                break;
        }
    }

    if(status != STATUS_OK && variant != NULL){
        variant->BuiltInTypeId = 0; // encoding byte read assignement
        SOPC_Int32_Clear(&variant->Value.Matrix.Dimensions);
    }

    return status;
}

SOPC_Byte GetDataValueEncodingMask(const SOPC_DataValue* dataValue){
    assert(dataValue != NULL);
    SOPC_Byte mask = 0;
    if(dataValue->Value.BuiltInTypeId != SOPC_Null_Id && dataValue->Value.BuiltInTypeId <= SOPC_BUILTINID_MAX){
        mask |= SOPC_DataValue_NotNullValue;
    }
    if(dataValue->Status != STATUS_OK){
        mask |= SOPC_DataValue_NotGoodStatusCode;
    }
    if(dataValue->SourceTimestamp.Low32 > 0 || dataValue->SourceTimestamp.High32 > 0){
        mask |= SOPC_DataValue_NotMinSourceDate;
    }
    if(dataValue->SourcePicoSeconds > 0){
        mask |= SOPC_DataValue_NotZeroSourcePico;
    }
    if(dataValue->ServerTimestamp.Low32 > 0 || dataValue->ServerTimestamp.Low32 > 0){
            mask |= SOPC_DataValue_NotMinServerDate;
    }
    if(dataValue->ServerPicoSeconds > 0){
        mask |= SOPC_DataValue_NotZeroServerPico;
    }
    return mask;
}


SOPC_StatusCode SOPC_DataValue_WriteAux(void* value, SOPC_Buffer* buf){
    return SOPC_DataValue_Write((SOPC_DataValue*) value, buf);
}

SOPC_StatusCode SOPC_DataValue_Write(const SOPC_DataValue* dataValue, SOPC_Buffer* buf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_Byte encodingMask = 0;
    if(dataValue != NULL){
        encodingMask = GetDataValueEncodingMask(dataValue);
        status = SOPC_Byte_Write(&encodingMask, buf);
    }
    if(status == STATUS_OK && (encodingMask & SOPC_DataValue_NotNullValue) != 0){
        status = SOPC_Variant_Write(&dataValue->Value, buf);
    }
    if(status == STATUS_OK && (encodingMask & SOPC_DataValue_NotGoodStatusCode) != 0){
        status = SOPC_StatusCode_Write(&dataValue->Status, buf);
    }
    if(status == STATUS_OK && (encodingMask & SOPC_DataValue_NotMinSourceDate) != 0){
        status = SOPC_DateTime_Write(&dataValue->SourceTimestamp, buf);
    }
    if(status == STATUS_OK && (encodingMask & SOPC_DataValue_NotZeroSourcePico) != 0){
        status = SOPC_UInt16_Write(&dataValue->SourcePicoSeconds, buf);
    }
    if(status == STATUS_OK && (encodingMask & SOPC_DataValue_NotMinServerDate) != 0){
            status = SOPC_DateTime_Write(&dataValue->ServerTimestamp, buf);
    }
    if(status == STATUS_OK && (encodingMask & SOPC_DataValue_NotZeroServerPico) != 0){
        status = SOPC_UInt16_Write(&dataValue->ServerPicoSeconds, buf);
    }
    return status;
}


SOPC_StatusCode SOPC_DataValue_ReadAux(void* value, SOPC_Buffer* buf){
    return SOPC_DataValue_Read((SOPC_DataValue*) value, buf);
}

SOPC_StatusCode SOPC_DataValue_Read(SOPC_DataValue* dataValue, SOPC_Buffer* buf){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_Byte encodingMask = 0;
    if(dataValue != NULL){
        status = SOPC_Byte_Read(&encodingMask, buf);
        if(status == STATUS_OK && (encodingMask & SOPC_DataValue_NotNullValue) != 0){
            status = SOPC_Variant_Read(&dataValue->Value, buf);
        }else{
            dataValue->Value.BuiltInTypeId = SOPC_Null_Id;
        }
        if(status == STATUS_OK && (encodingMask & SOPC_DataValue_NotGoodStatusCode) != 0){
            status = SOPC_StatusCode_Read(&dataValue->Status, buf);
        }else{
            dataValue->Status = STATUS_OK;
        }
        if(status == STATUS_OK && (encodingMask & SOPC_DataValue_NotMinSourceDate) != 0){
            status = SOPC_DateTime_Read(&dataValue->SourceTimestamp, buf);
        }else{
            dataValue->SourceTimestamp.Low32 = 0;
            dataValue->SourceTimestamp.High32 = 0;
        }
        if(status == STATUS_OK && (encodingMask & SOPC_DataValue_NotZeroSourcePico) != 0){
            status = SOPC_UInt16_Read(&dataValue->SourcePicoSeconds, buf);
        }else{
            dataValue->SourcePicoSeconds = 0;
        }
        if(status == STATUS_OK && (encodingMask & SOPC_DataValue_NotMinServerDate) != 0){
            status = SOPC_DateTime_Read(&dataValue->ServerTimestamp, buf);
        }else{
            dataValue->ServerTimestamp.Low32 = 0;
            dataValue->ServerTimestamp.High32 = 0;
        }
        if(status == STATUS_OK && (encodingMask & SOPC_DataValue_NotZeroServerPico) != 0){
            status = SOPC_UInt16_Read(&dataValue->ServerPicoSeconds, buf);
        }else{
            dataValue->ServerPicoSeconds = 0;
        }

        if(status != STATUS_OK && dataValue != NULL){
            SOPC_Variant_Clear(&dataValue->Value);
            SOPC_StatusCode_Clear(&dataValue->Status);
            SOPC_DateTime_Clear(&dataValue->SourceTimestamp);
            SOPC_UInt16_Clear(&dataValue->SourcePicoSeconds);
            SOPC_DateTime_Clear(&dataValue->ServerTimestamp);
            // No clear for last read since it should manage it itself in case of failure
        }
    }

    return status;
}

SOPC_StatusCode SOPC_Read_Array(SOPC_Buffer* buf, int32_t* noOfElts, void** eltsArray,
                                size_t sizeOfElt, SOPC_EncodeableObject_PfnDecode* decodeFct,
                                SOPC_EncodeableObject_PfnInitialize* initializeFct,
                                SOPC_EncodeableObject_PfnClear* clearFct)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_Byte* byteArray = NULL;
    assert(buf != NULL && *eltsArray == NULL && noOfElts != NULL);

    if(buf != NULL && noOfElts != NULL && *noOfElts >= 0 &&
       (size_t) *noOfElts <= SIZE_MAX &&
       eltsArray != NULL  && *eltsArray == NULL &&
       decodeFct != NULL){
        status = STATUS_OK;
    }

    if(STATUS_OK == status){
        status = SOPC_Int32_Read(noOfElts, buf);
    }

    if(STATUS_OK == status && *noOfElts > 0){
        *eltsArray = malloc (sizeOfElt * (size_t) *noOfElts);
        if(*eltsArray == NULL){
            status = STATUS_NOK;
        }else{
            byteArray = (SOPC_Byte*) *eltsArray;
        }
    }

    if(STATUS_OK == status && *noOfElts > 0){
        size_t idx = 0;
        size_t pos = 0;
        for (idx = 0; status == STATUS_OK && idx < (size_t) *noOfElts; idx ++){
            pos = idx * sizeOfElt;
            initializeFct(&(byteArray[pos]));
            status = decodeFct(&(byteArray[pos]), buf);
        }

        if(STATUS_OK != status){
            size_t clearIdx = 0;
            // idx - 1 => clear only cases in which status was ok since we don't know
            //            the state in which byte array is in the last idx used (decode failed)
            for (clearIdx = 0; clearIdx < (idx - 1); clearIdx ++){
                pos = clearIdx * sizeOfElt;
                clearFct(&(byteArray[pos]));
            }
            free(*eltsArray);
            *eltsArray = NULL;
            *noOfElts = 0;
        }

    }

    return status;
}

SOPC_StatusCode SOPC_Write_Array(SOPC_Buffer* buf, int32_t* noOfElts, void** eltsArray,
                                 size_t sizeOfElt, SOPC_EncodeableObject_PfnEncode* encodeFct){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_Byte* byteArray = NULL;

    if(buf != NULL && noOfElts != NULL && eltsArray != NULL && encodeFct != NULL){
        if(*noOfElts > 0){
            if(*eltsArray != NULL){
                status = STATUS_OK;
                byteArray = *eltsArray;
            }
        }else{
            status = STATUS_OK;
        }
    }

    if(STATUS_OK == status){
        status = SOPC_Int32_Write(noOfElts, buf);
    }
    if(STATUS_OK == status && *noOfElts > 0){
        size_t idx = 0;
        size_t pos = 0;
        for (idx = 0; status == STATUS_OK && idx < (size_t) *noOfElts; idx ++){
            pos = idx * sizeOfElt;
            status = encodeFct(&(byteArray[pos]), buf);
        }
    }
    return status;
}

