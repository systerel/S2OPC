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

#include "sopc_encodeable.h"
#include "sopc_tcp_ua_low_level.h"

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

SOPC_StatusCode SOPC_Byte_Write(const SOPC_Byte* value, SOPC_MsgBuffer* msgBuffer)
{
    if(value == NULL){
        return STATUS_INVALID_PARAMETERS;
    }
    return TCP_UA_WriteMsgBuffer(msgBuffer, value, 1);
}

SOPC_StatusCode SOPC_Byte_Read(SOPC_Byte* value, SOPC_MsgBuffer* msgBuffer)
{
    if(value == NULL){
        return STATUS_INVALID_PARAMETERS;
    }

    return TCP_UA_ReadMsgBuffer(value, 1, msgBuffer, 1);
}

SOPC_StatusCode SOPC_Boolean_Write(const SOPC_Boolean* value, SOPC_MsgBuffer* msgBuffer)
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

    return SOPC_Byte_Write(&encodedValue, msgBuffer);
}

SOPC_StatusCode SOPC_Boolean_Read(SOPC_Boolean* value, SOPC_MsgBuffer* msgBuffer)
{
    SOPC_StatusCode status = STATUS_NOK;
    if(value == NULL){
        status = STATUS_INVALID_PARAMETERS;
    }else{
        status = SOPC_Byte_Read((SOPC_Byte*) value, msgBuffer);
        if(status == STATUS_OK){
            if(*value != FALSE){
                // Decoder should use 1 as True value
                *value = 1;
            }
        }
    }
    return status;
}

SOPC_StatusCode SOPC_SByte_Write(const SOPC_SByte* value, SOPC_MsgBuffer* msgBuffer)
{
    if(value == NULL){
        return STATUS_INVALID_PARAMETERS;
    }
    return TCP_UA_WriteMsgBuffer(msgBuffer, (SOPC_Byte*) value, 1);
}

SOPC_StatusCode SOPC_SByte_Read(SOPC_SByte* value, SOPC_MsgBuffer* msgBuffer)
{
    if(value == NULL){
        return STATUS_INVALID_PARAMETERS;
    }

    return TCP_UA_ReadMsgBuffer((SOPC_Byte*) value, 1, msgBuffer, 1);
}

SOPC_StatusCode SOPC_Int16_Write(const int16_t* value, SOPC_MsgBuffer* msgBuffer)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(value != NULL){
        int16_t encodedValue = *value;
        SOPC_EncodeDecode_Int16(&encodedValue);
        status = TCP_UA_WriteMsgBuffer(msgBuffer, (SOPC_Byte*) &encodedValue, 2);
    }
    return status;
}

SOPC_StatusCode SOPC_Int16_Read(int16_t* value, SOPC_MsgBuffer* msgBuffer)
{
    SOPC_StatusCode status = STATUS_NOK;
    int16_t readValue;
    if(value == NULL){
        status = STATUS_INVALID_PARAMETERS;
    }else{
        status = TCP_UA_ReadMsgBuffer((SOPC_Byte*)&readValue, 2, msgBuffer, 2);
        if(status == STATUS_OK){
            SOPC_EncodeDecode_Int16(&readValue);
            *value = readValue;
        }
    }
    return status;
}

SOPC_StatusCode SOPC_UInt16_Write(const uint16_t* value, SOPC_MsgBuffer* msgBuffer)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(value != NULL){
        uint16_t encodedValue = *value;
        SOPC_EncodeDecode_UInt16(&encodedValue);
        status = TCP_UA_WriteMsgBuffer(msgBuffer, (SOPC_Byte*) &encodedValue, 2);
    }
    return status;
}

SOPC_StatusCode SOPC_UInt16_Read(uint16_t* value, SOPC_MsgBuffer* msgBuffer)
{
    SOPC_StatusCode status = STATUS_NOK;
    if(value == NULL){
        status = STATUS_INVALID_PARAMETERS;
    }else{
        status = TCP_UA_ReadMsgBuffer((SOPC_Byte*)value, 2, msgBuffer, 2);
        if(status == STATUS_OK){
            SOPC_EncodeDecode_UInt16(value);
        }
    }
    return status;
}

SOPC_StatusCode SOPC_Int32_Write(const int32_t* value, SOPC_MsgBuffer* msgBuffer)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(value != NULL){
        int32_t encodedValue = *value;
        SOPC_EncodeDecode_Int32(&encodedValue);
        status = TCP_UA_WriteMsgBuffer(msgBuffer, (SOPC_Byte*) &encodedValue, 4);
    }
    return status;
}

SOPC_StatusCode SOPC_Int32_Read(int32_t* value, SOPC_MsgBuffer* msgBuffer)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(value != NULL){
        status = TCP_UA_ReadMsgBuffer((SOPC_Byte*)value, 4, msgBuffer, 4);
        if(status == STATUS_OK){
            SOPC_EncodeDecode_Int32(value);
        }
    }
    return status;
}

SOPC_StatusCode SOPC_UInt32_Write(const uint32_t* value, SOPC_MsgBuffer* msgBuffer)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(value != NULL){
        uint32_t encodedValue = *value;
        SOPC_EncodeDecode_UInt32(&encodedValue);
        status = TCP_UA_WriteMsgBuffer(msgBuffer, (SOPC_Byte*) &encodedValue, 4);
    }
    return status;
}

SOPC_StatusCode SOPC_UInt32_Read(uint32_t* value, SOPC_MsgBuffer* msgBuffer)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(value != NULL){
        status = TCP_UA_ReadMsgBuffer((SOPC_Byte*)value, 4, msgBuffer, 4);
        if(status == STATUS_OK){
            SOPC_EncodeDecode_UInt32(value);
        }
    }
    return status;
}

SOPC_StatusCode SOPC_Int64_Write(const int64_t* value, SOPC_MsgBuffer* msgBuffer)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(value != NULL){
        int64_t encodedValue = *value;
        SOPC_EncodeDecode_Int64(&encodedValue);
        status = TCP_UA_WriteMsgBuffer(msgBuffer, (SOPC_Byte*) &encodedValue, 8);
    }
    return status;
}

SOPC_StatusCode SOPC_Int64_Read(int64_t* value, SOPC_MsgBuffer* msgBuffer)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(value != NULL){
        status = TCP_UA_ReadMsgBuffer((SOPC_Byte*) value, 8, msgBuffer, 8);
        if(status == STATUS_OK){
            SOPC_EncodeDecode_Int64(value);
        }
    }
    return status;
}

SOPC_StatusCode SOPC_UInt64_Write(const uint64_t* value, SOPC_MsgBuffer* msgBuffer)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(value != NULL){
        uint64_t encodedValue = *value;
        SOPC_EncodeDecode_UInt64(&encodedValue);
        status = TCP_UA_WriteMsgBuffer(msgBuffer, (SOPC_Byte*) &encodedValue, 8);
    }
    return status;
}

SOPC_StatusCode SOPC_UInt64_Read(uint64_t* value, SOPC_MsgBuffer* msgBuffer)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(value != NULL){
        status = TCP_UA_ReadMsgBuffer((SOPC_Byte*) value, 8, msgBuffer, 8);
        if(status == STATUS_OK){
            SOPC_EncodeDecode_UInt64(value);
        }
    }
    return status;
}

SOPC_StatusCode SOPC_Float_Write(const float* value, SOPC_MsgBuffer* msgBuffer)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(value != NULL){
        float encodedValue = *value;
        SOPC_EncodeDecode_Float(&encodedValue);
        status = TCP_UA_WriteMsgBuffer(msgBuffer, (SOPC_Byte*) &encodedValue, 4);
    }
    return status;
}

SOPC_StatusCode SOPC_Float_Read(float* value, SOPC_MsgBuffer* msgBuffer)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(value != NULL){
        status = TCP_UA_ReadMsgBuffer((SOPC_Byte*)value, 4, msgBuffer, 4);
        if(status == STATUS_OK){
            SOPC_EncodeDecode_Float(value);
        }
    }
    return status;
}

SOPC_StatusCode SOPC_Double_Write(const double* value, SOPC_MsgBuffer* msgBuffer)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(value != NULL){
        double encodedValue = *value;
        SOPC_EncodeDecode_Double(&encodedValue);
        status = TCP_UA_WriteMsgBuffer(msgBuffer, (SOPC_Byte*) &encodedValue, 8);
    }
    return status;
}

SOPC_StatusCode SOPC_Double_Read(double* value, SOPC_MsgBuffer* msgBuffer)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(value != NULL){
        status = TCP_UA_ReadMsgBuffer((SOPC_Byte*)value, 8, msgBuffer, 8);
        if(status == STATUS_OK){
            SOPC_EncodeDecode_Double(value);
        }
    }
    return status;
}

SOPC_StatusCode SOPC_ByteString_Write(const SOPC_ByteString* str, SOPC_MsgBuffer* msgBuffer)
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
        status = SOPC_Int32_Write(&length, msgBuffer);
        if(status == STATUS_OK &&
           str->Length > 0)
        {
            status = TCP_UA_WriteMsgBuffer(msgBuffer, str->Data, str->Length);
        }
    }
    return status;
}

SOPC_StatusCode SOPC_ByteString_Read(SOPC_ByteString* str, SOPC_MsgBuffer* msgBuffer)
{
    SOPC_StatusCode status = STATUS_NOK;
    int32_t length;
    if(str == NULL || (str != NULL && str->Data != NULL)){
        status = STATUS_INVALID_PARAMETERS;
    }else{
        status = TCP_UA_ReadMsgBuffer((SOPC_Byte*)&length, 4, msgBuffer, 4);
        if(status == STATUS_OK){
            SOPC_EncodeDecode_Int32(&length);
            if(length > 0){
                str->Length = length;
                str->Data = malloc(sizeof(SOPC_Byte) * length);
                if(str->Data != NULL){
                    status = TCP_UA_ReadMsgBuffer(str->Data, length, msgBuffer, length);
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

SOPC_StatusCode SOPC_String_Write(const SOPC_String* str, SOPC_MsgBuffer* msgBuffer)
{
    return SOPC_ByteString_Write((SOPC_ByteString*) str, msgBuffer);
}

SOPC_StatusCode SOPC_String_Read(SOPC_String* str, SOPC_MsgBuffer* msgBuffer)
{
    SOPC_StatusCode status = STATUS_NOK;
    int32_t length;
    if(str == NULL || (str != NULL && str->Data != NULL)){
        status = STATUS_INVALID_PARAMETERS;
    }else{
        status = TCP_UA_ReadMsgBuffer((SOPC_Byte*)&length, 4, msgBuffer, 4);
        if(status == STATUS_OK){
            SOPC_EncodeDecode_Int32(&length);
            if(length > 0){
                str->Length = length;
                // +1 to add '\0' character for CString compatibility
                str->Data = malloc(sizeof(SOPC_Byte) * (length + 1));
                if(str->Data != NULL){
                    status = TCP_UA_ReadMsgBuffer(str->Data, length, msgBuffer, length);
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

SOPC_StatusCode SOPC_XmlElement_Write(const SOPC_XmlElement* xml, SOPC_MsgBuffer* msgBuffer)
{
    // TODO: check XML validity ?
    return SOPC_ByteString_Write((SOPC_ByteString*) xml, msgBuffer);
}

SOPC_StatusCode SOPC_XmlElement_Read(SOPC_XmlElement* xml, SOPC_MsgBuffer* msgBuffer)
{
    // TODO: parse XML ?
    return SOPC_ByteString_Read((SOPC_ByteString*) xml, msgBuffer);
}

SOPC_StatusCode SOPC_DateTime_Write(const SOPC_DateTime* date, SOPC_MsgBuffer* msgBuffer)
{
    return SOPC_Int64_Write(date, msgBuffer);
}

SOPC_StatusCode SOPC_DateTime_Read(SOPC_DateTime* date, SOPC_MsgBuffer* msgBuffer)
{
    return SOPC_Int64_Read(date, msgBuffer);
}

SOPC_StatusCode SOPC_Guid_Write(const SOPC_Guid* guid, SOPC_MsgBuffer* msgBuffer)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(guid != NULL){
        status = SOPC_UInt32_Write(&guid->Data1, msgBuffer);
    }
    if(status == STATUS_OK){
        status = SOPC_UInt16_Write(&guid->Data2, msgBuffer);
    }
    if(status == STATUS_OK){
        status = SOPC_UInt16_Write(&guid->Data3, msgBuffer);
    }
    if(status == STATUS_OK){
        status = TCP_UA_WriteMsgBuffer(msgBuffer, &(guid->Data4[0]), 8);
    }
    return status;
}

SOPC_StatusCode SOPC_Guid_Read(SOPC_Guid* guid, SOPC_MsgBuffer* msgBuffer)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(guid != NULL){
        status = SOPC_UInt32_Read(&guid->Data1, msgBuffer);
    }
    if(status == STATUS_OK){
        status = SOPC_UInt16_Read(&guid->Data2, msgBuffer);
    }
    if(status == STATUS_OK){
        status = SOPC_UInt16_Read(&guid->Data3, msgBuffer);
    }
    if(status == STATUS_OK){
        status = TCP_UA_ReadMsgBuffer(&(guid->Data4[0]), 8, msgBuffer, 8);
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

SOPC_StatusCode Internal_NodeId_Write(SOPC_MsgBuffer* msgBuffer,
                                 SOPC_Byte encodingByte,
                                 const SOPC_NodeId* nodeId)
{
    assert(nodeId != NULL);
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_NodeId_DataEncoding encodingType = 0x0F & encodingByte; // Eliminate flags

    SOPC_Byte byte = 0;
    uint16_t twoBytes = 0;

    status = SOPC_Byte_Write(&encodingByte, msgBuffer);
    if(status == STATUS_OK){
        switch(encodingType){
            case NodeIdEncoding_Invalid:
                status = STATUS_INVALID_PARAMETERS;
                break;
            case NodeIdEncoding_TwoByte:
                byte = (SOPC_Byte) nodeId->Data.Numeric;
                status = SOPC_Byte_Write(&byte, msgBuffer);
                break;
            case  NodeIdEncoding_FourByte:
                twoBytes = (uint16_t) nodeId->Data.Numeric;
                if(nodeId->Namespace <= UINT8_MAX){
                    const SOPC_Byte namespace = nodeId->Namespace;
                    status = SOPC_Byte_Write(&namespace, msgBuffer);
                }else{
                    status = STATUS_INVALID_PARAMETERS;
                }
                if(status == STATUS_OK){
                    status = SOPC_UInt16_Write(&twoBytes, msgBuffer);
                }
                break;
            case  NodeIdEncoding_Numeric:
                status = SOPC_UInt16_Write(&nodeId->Namespace, msgBuffer);
                if(status == STATUS_OK){
                    status = SOPC_UInt32_Write(&nodeId->Data.Numeric, msgBuffer);
                }
                break;
            case  NodeIdEncoding_String:
                status = SOPC_UInt16_Write(&nodeId->Namespace, msgBuffer);
                if(status == STATUS_OK){
                    status = SOPC_String_Write(&nodeId->Data.String, msgBuffer);
                }
                break;
            case  NodeIdEncoding_Guid:
                status = SOPC_UInt16_Write(&nodeId->Namespace, msgBuffer);
                if(status == STATUS_OK){
                    status = SOPC_Guid_Write(nodeId->Data.Guid, msgBuffer);
                }
                break;
            case  NodeIdEncoding_ByteString:
                status = SOPC_UInt16_Write(&nodeId->Namespace, msgBuffer);
                if(status == STATUS_OK){
                    status = SOPC_ByteString_Write(&nodeId->Data.Bstring, msgBuffer);
                }
                break;
            default:
                status = STATUS_INVALID_PARAMETERS;
        }
    }

    return status;
}

SOPC_StatusCode SOPC_NodeId_Write(const SOPC_NodeId* nodeId, SOPC_MsgBuffer* msgBuffer)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(nodeId != NULL){
        status = Internal_NodeId_Write(msgBuffer, GetNodeIdDataEncoding(nodeId), nodeId);
    }
    return status;
}

SOPC_StatusCode Internal_NodeId_Read(SOPC_MsgBuffer* msgBuffer,
                                SOPC_NodeId* nodeId,
                                SOPC_Byte* encodingByte)
{
    assert(nodeId != NULL);
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_NodeId_DataEncoding encodingType = 0x0F;

    SOPC_Byte byte = 0;
    uint16_t twoBytes = 0;

    status = SOPC_Byte_Read(encodingByte, msgBuffer);

    if(status == STATUS_OK){
        encodingType = 0x0F & *encodingByte; // Eliminate flags
        switch(encodingType){
            case NodeIdEncoding_Invalid:
                status = STATUS_INVALID_RCV_PARAMETER;
                break;
            case NodeIdEncoding_TwoByte:
                nodeId->IdentifierType = IdentifierType_Numeric;
                nodeId->Namespace = 0;
                status = SOPC_Byte_Read(&byte, msgBuffer);
                nodeId->Data.Numeric = (uint32_t) byte;
                break;
            case  NodeIdEncoding_FourByte:
                nodeId->IdentifierType = IdentifierType_Numeric;
                status = SOPC_Byte_Read(&byte, msgBuffer);
                nodeId->Namespace = byte;
                if(status == STATUS_OK){
                    status = SOPC_UInt16_Read(&twoBytes, msgBuffer);
                    nodeId->Data.Numeric = twoBytes;
                }
                break;
            case  NodeIdEncoding_Numeric:
                nodeId->IdentifierType = IdentifierType_Numeric;
                status = SOPC_UInt16_Read(&nodeId->Namespace, msgBuffer);
                if(status == STATUS_OK){
                    status = SOPC_UInt32_Read(&nodeId->Data.Numeric, msgBuffer);
                }
                break;
            case  NodeIdEncoding_String:
                nodeId->IdentifierType = IdentifierType_String;
                status = SOPC_UInt16_Read(&nodeId->Namespace, msgBuffer);
                if(status == STATUS_OK){
                    status = SOPC_String_Read(&nodeId->Data.String, msgBuffer);
                }
                break;
            case  NodeIdEncoding_Guid:
                nodeId->IdentifierType = IdentifierType_Guid;
                status = SOPC_UInt16_Read(&nodeId->Namespace, msgBuffer);
                if(status == STATUS_OK){
                    nodeId->Data.Guid = malloc(sizeof(SOPC_Guid));
                    status = SOPC_Guid_Read(nodeId->Data.Guid, msgBuffer);
                }
                break;
            case  NodeIdEncoding_ByteString:
                nodeId->IdentifierType = IdentifierType_ByteString;
                status = SOPC_UInt16_Read(&nodeId->Namespace, msgBuffer);
                if(status == STATUS_OK){
                    status = SOPC_ByteString_Read(&nodeId->Data.Bstring, msgBuffer);
                }
                break;
            default:
                status = STATUS_INVALID_PARAMETERS;
        }
    }

    return status;
}

SOPC_StatusCode SOPC_NodeId_Read(SOPC_NodeId* nodeId, SOPC_MsgBuffer* msgBuffer)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_Byte encodingByte = 0;
    if(nodeId != NULL){
        status = Internal_NodeId_Read(msgBuffer, nodeId, &encodingByte);
    }
    return status;
}

SOPC_StatusCode SOPC_ExpandedNodeId_Write(const SOPC_ExpandedNodeId* expNodeId, SOPC_MsgBuffer* msgBuffer){
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
        status = Internal_NodeId_Write(msgBuffer, encodingByte, &expNodeId->NodeId);
    }
    if(status == STATUS_OK && expNodeId->NamespaceUri.Length > 0)
    {
        status = SOPC_String_Write(&expNodeId->NamespaceUri, msgBuffer);
    }
    if(status == STATUS_OK && expNodeId->ServerIndex > 0)
    {
        status = SOPC_UInt32_Write(&expNodeId->ServerIndex, msgBuffer);
    }

    return status;
}

SOPC_StatusCode SOPC_ExpandedNodeId_Read(SOPC_ExpandedNodeId* expNodeId, SOPC_MsgBuffer* msgBuffer){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_Byte encodingByte = 0;
    if(expNodeId != NULL){
        status = Internal_NodeId_Read(msgBuffer, &expNodeId->NodeId, &encodingByte);
    }
    if(status == STATUS_OK &&
       (encodingByte & NodeIdEncoding_NamespaceUriFlag) != 0x00)
    {
        status = SOPC_String_Read(&expNodeId->NamespaceUri, msgBuffer);
    }else{
        SOPC_String_Clear(&expNodeId->NamespaceUri);
    }
    if(status == STATUS_OK &&
        (encodingByte & NodeIdEncoding_ServerIndexFlag) != 0)
    {
        status = SOPC_UInt32_Read(&expNodeId->ServerIndex, msgBuffer);
    }else{
        SOPC_UInt32_Clear(&expNodeId->ServerIndex);
    }
    return status;
}

SOPC_StatusCode SOPC_StatusCode_Write(const SOPC_StatusCode* status, SOPC_MsgBuffer* msgBuffer){
    return SOPC_UInt32_Write(status, msgBuffer);
}

SOPC_StatusCode SOPC_StatusCode_Read(SOPC_StatusCode* status, SOPC_MsgBuffer* msgBuffer){
    return SOPC_UInt32_Read(status, msgBuffer);
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

SOPC_StatusCode SOPC_DiagnosticInfo_Write(const SOPC_DiagnosticInfo* diagInfo, SOPC_MsgBuffer* msgBuffer){
    SOPC_Byte encodingByte = 0x00;
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(diagInfo != NULL){
        status = STATUS_OK;
        encodingByte = GetDiagInfoEncodingByte(diagInfo);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_SymbolicId) != 0x00){
        status = SOPC_Int32_Write(&diagInfo->SymbolicId, msgBuffer);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_Namespace) != 0x00){
        status = SOPC_Int32_Write(&diagInfo->NamespaceUri, msgBuffer);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_Locale) != 0x00){
        status = SOPC_Int32_Write(&diagInfo->Locale, msgBuffer);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_LocalizedTest) != 0x00){
        status = SOPC_Int32_Write(&diagInfo->LocalizedText, msgBuffer);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_AdditionalInfo) != 0x00){
        status = SOPC_String_Write(&diagInfo->AdditionalInfo, msgBuffer);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_InnerStatusCode) != 0x00){
        status = SOPC_StatusCode_Write(&diagInfo->InnerStatusCode, msgBuffer);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_InnerDianosticInfo) != 0x00){
        status = SOPC_DiagnosticInfo_Write(diagInfo->InnerDiagnosticInfo, msgBuffer);
    }
    return status;
}

SOPC_StatusCode SOPC_DiagnosticInfo_Read(SOPC_DiagnosticInfo* diagInfo, SOPC_MsgBuffer* msgBuffer){
    SOPC_Byte encodingByte = 0x00;
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(diagInfo != NULL){
        status  = SOPC_Byte_Read(&encodingByte, msgBuffer);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_SymbolicId) != 0x00){
        status = SOPC_Int32_Read(&diagInfo->SymbolicId, msgBuffer);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_Namespace) != 0x00){
        status = SOPC_Int32_Read(&diagInfo->NamespaceUri, msgBuffer);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_Locale) != 0x00){
        status = SOPC_Int32_Read(&diagInfo->Locale, msgBuffer);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_LocalizedTest) != 0x00){
        status = SOPC_Int32_Read(&diagInfo->LocalizedText, msgBuffer);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_AdditionalInfo) != 0x00){
        status = SOPC_String_Read(&diagInfo->AdditionalInfo, msgBuffer);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_InnerStatusCode) != 0x00){
        status = SOPC_StatusCode_Read(&diagInfo->InnerStatusCode, msgBuffer);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_InnerDianosticInfo) != 0x00){
        status = SOPC_DiagnosticInfo_Read(diagInfo->InnerDiagnosticInfo, msgBuffer);
    }
    return status;
}

SOPC_StatusCode SOPC_QualifiedName_Write(const SOPC_QualifiedName* qname, SOPC_MsgBuffer* msgBuffer){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(qname != NULL){
        status = SOPC_UInt16_Write(&qname->NamespaceIndex, msgBuffer);
    }
    if(status == STATUS_OK){
        status = SOPC_String_Write(&qname->Name, msgBuffer);
    }
    return status;
}

SOPC_StatusCode SOPC_QualifiedName_Read(SOPC_QualifiedName* qname, SOPC_MsgBuffer* msgBuffer){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(qname != NULL){
        status = SOPC_UInt16_Read(&qname->NamespaceIndex, msgBuffer);
    }
    if(status == STATUS_OK){
        status = SOPC_String_Read(&qname->Name, msgBuffer);
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

SOPC_StatusCode SOPC_LocalizedText_Write(const SOPC_LocalizedText* localizedText, SOPC_MsgBuffer* msgBuffer){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_Byte encodingByte = 0;
    if(localizedText != NULL){
        status = STATUS_OK;
        encodingByte = GetLocalizedTextEncodingByte(localizedText);
        status = SOPC_Byte_Write(&encodingByte, msgBuffer);
    }
    if(status == STATUS_OK && (encodingByte & SOPC_LocalizedText_Locale) != 0){
        status = SOPC_String_Write(&localizedText->Locale, msgBuffer);
    }
    if(status == STATUS_OK && (encodingByte & SOPC_LocalizedText_Text) != 0){
        status = SOPC_String_Write(&localizedText->Text, msgBuffer);
    }
    return status;
}

SOPC_StatusCode SOPC_LocalizedText_Read(SOPC_LocalizedText* localizedText, SOPC_MsgBuffer* msgBuffer){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_Byte encodingByte = 0;
    if(localizedText != NULL){
        status = SOPC_Byte_Read(&encodingByte, msgBuffer);
    }
    if(status == STATUS_OK && (encodingByte & SOPC_LocalizedText_Locale) != 0){
        status = SOPC_String_Read(&localizedText->Locale, msgBuffer);
    }
    if(status == STATUS_OK && (encodingByte & SOPC_LocalizedText_Text) != 0){
        status = SOPC_String_Read(&localizedText->Text, msgBuffer);
    }
    return status;
}

SOPC_StatusCode SOPC_ExtensionObject_Write(const SOPC_ExtensionObject* extObj, SOPC_MsgBuffer* msgBuffer){
    const int32_t tmpLength = -1;
    SOPC_NodeId objNodeId = extObj->TypeId;
    uint32_t lengthPos;
    uint32_t curPos;
    int32_t length;
    uint16_t nsIndex = OPCUA_NAMESPACE_INDEX;
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_Byte encodingByte = 0;
    if(extObj != NULL){
        encodingByte = extObj->Encoding;
        status = STATUS_OK;
    }

    if(status == STATUS_OK &&
       encodingByte == SOPC_ExtObjBodyEncoding_Object)
    {
        encodingByte = SOPC_ExtObjBodyEncoding_ByteString;
        if(extObj->Body.Object.ObjType == NULL){
            status = STATUS_INVALID_PARAMETERS;
        }else{
            if(strncmp(extObj->Body.Object.ObjType->NamespaceUri,
                       OPCUA_NAMESPACE_NAME,
                       strlen(OPCUA_NAMESPACE_NAME))
               !=  0)
            {
                status = Namespace_GetIndex(&msgBuffer->nsTable, extObj->Body.Object.ObjType->NamespaceUri, &nsIndex);
            }

            objNodeId.IdentifierType = IdentifierType_Numeric;
            objNodeId.Namespace = nsIndex;
            objNodeId.Data.Numeric = extObj->Body.Object.ObjType->BinaryEncodingTypeId;
        }
    }

    if(status == STATUS_OK){
        status = SOPC_NodeId_Write(&objNodeId, msgBuffer);
    }

    if(status == STATUS_OK){
        status = SOPC_Byte_Write(&encodingByte, msgBuffer);
    }

    if(status == STATUS_OK){
        switch(extObj->Encoding){
            case SOPC_ExtObjBodyEncoding_None:
                break;
            case SOPC_ExtObjBodyEncoding_ByteString:
                status = SOPC_ByteString_Write(&extObj->Body.Bstring, msgBuffer);
                break;
            case SOPC_ExtObjBodyEncoding_XMLElement:
                status = SOPC_XmlElement_Write(&extObj->Body.Xml, msgBuffer);
                break;
            case SOPC_ExtObjBodyEncoding_Object:
                lengthPos = msgBuffer->buffers->position;
                status = SOPC_Int32_Write(&tmpLength, msgBuffer);
                if(status == STATUS_OK){
                    status = extObj->Body.Object.ObjType->Encode(extObj->Body.Object.Value, msgBuffer);
                }
                if(status == STATUS_OK){
                    // Go backward to write correct length value
                    curPos = msgBuffer->buffers->position;
                    length = curPos - (lengthPos + 4);
                    Buffer_SetPosition(msgBuffer->buffers, lengthPos);
                    SOPC_Int32_Write(&length, msgBuffer);
                    Buffer_SetPosition(msgBuffer->buffers, curPos);
                }
                break;
        }
    }

    return status;
}

SOPC_StatusCode SOPC_ExtensionObject_Read(SOPC_ExtensionObject* extObj, SOPC_MsgBuffer* msgBuffer){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_EncodeableType* encType;
    const char* nsName;
    SOPC_Byte encodingByte = 0;
    if(extObj != NULL){
        status = SOPC_NodeId_Read(&extObj->TypeId, msgBuffer);
    }
    if(status == STATUS_OK){
        status = SOPC_Byte_Read(&encodingByte, msgBuffer);
    }

    if(status == STATUS_OK &&
       encodingByte == SOPC_ExtObjBodyEncoding_ByteString){
        if(extObj->TypeId.IdentifierType == IdentifierType_Numeric){
            if(extObj->TypeId.Namespace != OPCUA_NAMESPACE_INDEX){
                nsName = Namespace_GetName(&msgBuffer->nsTable, extObj->TypeId.Namespace);
            }else{
                nsName = OPCUA_NAMESPACE_NAME;
            }
            if(nsName != NULL){
                encType = EncodeableType_GetEncodeableType(msgBuffer->encTypesTable,
                                                           nsName,
                                                           extObj->TypeId.Data.Numeric);
            }
            if(nsName == NULL || encType == NULL){
                status = STATUS_NOK;
            }else{
                encodingByte = SOPC_ExtObjBodyEncoding_Object;
                extObj->Body.Object.ObjType = encType;
            }
        }else{
            status = STATUS_INVALID_RCV_PARAMETER;
        }

    }

    if(status == STATUS_OK){
        switch(encodingByte){
            case SOPC_ExtObjBodyEncoding_None:
                extObj->Length = -1;
                break;
            case SOPC_ExtObjBodyEncoding_ByteString:
                status = SOPC_ByteString_Read(&extObj->Body.Bstring, msgBuffer);
                break;
            case SOPC_ExtObjBodyEncoding_XMLElement:
                status = SOPC_XmlElement_Read(&extObj->Body.Xml, msgBuffer);
                break;
            case SOPC_ExtObjBodyEncoding_Object:
                status = SOPC_Int32_Read(&extObj->Length, msgBuffer);
                if(status == STATUS_OK){
                    extObj->Body.Object.Value = malloc(extObj->Body.Object.ObjType->AllocationSize);
                    status = extObj->Body.Object.ObjType->Decode(&extObj->Body.Object.Value, msgBuffer);
                }
                break;
            default:
                status = STATUS_INVALID_RCV_PARAMETER;
        }
        if(status == STATUS_OK){
            extObj->Encoding = encodingByte;
        }
    }

    return status;
}

SOPC_Byte GetVariantEncodingMask(const SOPC_Variant* variant){
    assert(variant != NULL);
    SOPC_Byte encodingByte = variant->BuiltInTypeMask;
    if((variant->ArrayTypeMask & SOPC_VariantArrayMatrixFlag) != 0){
        encodingByte |= SOPC_VariantArrayMatrixFlag;
    }
    if((variant->ArrayTypeMask & SOPC_VariantArrayValueFlag) != 0){
        encodingByte |= SOPC_VariantArrayValueFlag;
    }
    return encodingByte;
}

SOPC_StatusCode WriteVariantNonArrayBuiltInType(SOPC_MsgBuffer* msgBuffer,
                                           SOPC_BuiltinId builtInTypeId,
                                           const SOPC_VariantValue *val){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    switch(builtInTypeId){
        case SOPC_Boolean_Id:
            status = SOPC_Boolean_Write(&val->Boolean, msgBuffer);
            break;
        case SOPC_SByte_Id:
            status = SOPC_SByte_Write(&val->Sbyte, msgBuffer);
            break;
        case SOPC_Byte_Id:
            status = SOPC_Byte_Write(&val->Byte, msgBuffer);
            break;
        case SOPC_Int16_Id:
            status = SOPC_Int16_Write(&val->Int16, msgBuffer);
            break;
        case SOPC_UInt16_Id:
            status = SOPC_UInt16_Write(&val->Uint16, msgBuffer);
            break;
        case SOPC_Int32_Id:
            status = SOPC_Int32_Write(&val->Int32, msgBuffer);
            break;
        case SOPC_UInt32_Id:
            status = SOPC_UInt32_Write(&val->Uint32, msgBuffer);
            break;
        case SOPC_Int64_Id:
            status = SOPC_Int64_Write(&val->Int64, msgBuffer);
            break;
        case SOPC_UInt64_Id:
            status = SOPC_UInt64_Write(&val->Uint64, msgBuffer);
            break;
        case SOPC_Float_Id:
            status = SOPC_Float_Write(&val->Floatv, msgBuffer);
            break;
        case SOPC_Double_Id:
            status = SOPC_Double_Write(&val->Doublev, msgBuffer);
            break;
        case SOPC_String_Id:
            status = SOPC_String_Write(&val->String, msgBuffer);
            break;
        case SOPC_DateTime_Id:
            status = SOPC_DateTime_Write(&val->Date, msgBuffer);
            break;
        case SOPC_Guid_Id:
            status = SOPC_Guid_Write(val->Guid, msgBuffer);
            break;
        case SOPC_ByteString_Id:
            status = SOPC_ByteString_Write(&val->Bstring, msgBuffer);
            break;
        case SOPC_XmlElement_Id:
            status = SOPC_XmlElement_Write(&val->XmlElt, msgBuffer);
            break;
        case SOPC_NodeId_Id:
            status = SOPC_NodeId_Write(val->NodeId, msgBuffer);
            break;
        case SOPC_ExpandedNodeId_Id:
            status = SOPC_ExpandedNodeId_Write(val->ExpNodeId, msgBuffer);
            break;
        case SOPC_StatusCode_Id:
            status = SOPC_StatusCode_Write(&val->Status, msgBuffer);
            break;
        case SOPC_QualifiedName_Id:
            status = SOPC_QualifiedName_Write(val->Qname, msgBuffer);
            break;
        case SOPC_LocalizedText_Id:
            status = SOPC_LocalizedText_Write(val->LocalizedText, msgBuffer);
            break;
        case SOPC_ExtensionObject_Id:
            status = SOPC_ExtensionObject_Write(val->ExtObject, msgBuffer);
            break;
        case SOPC_DataValue_Id:
            status = SOPC_DataValue_Write(val->DataValue, msgBuffer);
            break;
        case SOPC_Variant_Id:
            assert(FALSE);
            break;
        case SOPC_DiagnosticInfo_Id:
            status = SOPC_DiagnosticInfo_Write(val->DiagInfo, msgBuffer);
            break;
        default:
            break;
    }
    return status;
}

SOPC_StatusCode WriteVariantArrayBuiltInType(SOPC_MsgBuffer* msgBuffer,
                                        SOPC_BuiltinId builtInTypeId,
                                        const SOPC_VariantArrayValue* array,
                                        int32_t length)
{
    SOPC_StatusCode status = STATUS_OK;
    int32_t idx = 0;
    switch(builtInTypeId){
        case SOPC_Boolean_Id:
            for(idx = 0; idx < length; idx++){
                if(STATUS_OK == status) status = SOPC_Boolean_Write(&array->BooleanArr[idx], msgBuffer);
            }
            break;
        case SOPC_SByte_Id:
            for(idx = 0; idx < length; idx++){
                if(STATUS_OK == status) status = SOPC_SByte_Write(&array->SbyteArr[idx], msgBuffer);
            }
            break;
        case SOPC_Byte_Id:
            for(idx = 0; idx < length; idx++){
                if(STATUS_OK == status) status = SOPC_Byte_Write(&array->ByteArr[idx], msgBuffer);
            }
            break;
        case SOPC_Int16_Id:
            for(idx = 0; idx < length; idx++){
                if(STATUS_OK == status) status = SOPC_Int16_Write(&array->Int16Arr[idx], msgBuffer);
            }
            break;
        case SOPC_UInt16_Id:
            for(idx = 0; idx < length; idx++){
                if(STATUS_OK == status) status = SOPC_UInt16_Write(&array->Uint16Arr[idx], msgBuffer);
            }
            break;
        case SOPC_Int32_Id:
            for(idx = 0; idx < length; idx++){
                if(STATUS_OK == status) status = SOPC_Int32_Write(&array->Int32Arr[idx], msgBuffer);
            }
            break;
        case SOPC_UInt32_Id:
            for(idx = 0; idx < length; idx++){
                if(STATUS_OK == status) status = SOPC_UInt32_Write(&array->Uint32Arr[idx], msgBuffer);
            }
            break;
        case SOPC_Int64_Id:
            for(idx = 0; idx < length; idx++){
                if(STATUS_OK == status) status = SOPC_Int64_Write(&array->Int64Arr[idx], msgBuffer);
            }
            break;
        case SOPC_UInt64_Id:
            for(idx = 0; idx < length; idx++){
                if(STATUS_OK == status) status = SOPC_UInt64_Write(&array->Uint64Arr[idx], msgBuffer);
            }
            break;
        case SOPC_Float_Id:
            for(idx = 0; idx < length; idx++){
                if(STATUS_OK == status) status = SOPC_Float_Write(&array->FloatvArr[idx], msgBuffer);
            }
            break;
        case SOPC_Double_Id:
            for(idx = 0; idx < length; idx++){
                if(STATUS_OK == status) status = SOPC_Double_Write(&array->DoublevArr[idx], msgBuffer);
            }
            break;
        case SOPC_String_Id:
            for(idx = 0; idx < length; idx++){
                if(STATUS_OK == status) status = SOPC_String_Write(&array->StringArr[idx], msgBuffer);
            }
            break;
        case SOPC_DateTime_Id:
            for(idx = 0; idx < length; idx++){
                if(STATUS_OK == status) status = SOPC_DateTime_Write(&array->DateArr[idx], msgBuffer);
            }
            break;
        case SOPC_Guid_Id:
            for(idx = 0; idx < length; idx++){
                if(STATUS_OK == status) status = SOPC_Guid_Write(&array->GuidArr[idx], msgBuffer);
            }
            break;
        case SOPC_ByteString_Id:
            for(idx = 0; idx < length; idx++){
                if(STATUS_OK == status) status = SOPC_ByteString_Write(&array->BstringArr[idx], msgBuffer);
            }
            break;
        case SOPC_XmlElement_Id:
            for(idx = 0; idx < length; idx++){
                if(STATUS_OK == status) status = SOPC_XmlElement_Write(&array->XmlEltArr[idx], msgBuffer);
            }
            break;
        case SOPC_NodeId_Id:
            for(idx = 0; idx < length; idx++){
                if(STATUS_OK == status) status = SOPC_NodeId_Write(&array->NodeIdArr[idx], msgBuffer);
            }
            break;
        case SOPC_ExpandedNodeId_Id:
            for(idx = 0; idx < length; idx++){
                if(STATUS_OK == status) status = SOPC_ExpandedNodeId_Write(&array->ExpNodeIdArr[idx], msgBuffer);
            }
            break;
        case SOPC_StatusCode_Id:
            for(idx = 0; idx < length; idx++){
                if(STATUS_OK == status) status = SOPC_StatusCode_Write(&array->StatusArr[idx], msgBuffer);
            }
            break;
        case SOPC_QualifiedName_Id:
            for(idx = 0; idx < length; idx++){
                if(STATUS_OK == status) status = SOPC_QualifiedName_Write(&array->QnameArr[idx], msgBuffer);
            }
            break;
        case SOPC_LocalizedText_Id:
            for(idx = 0; idx < length; idx++){
                if(STATUS_OK == status) status = SOPC_LocalizedText_Write(&array->LocalizedTextArr[idx], msgBuffer);
            }
            break;
        case SOPC_ExtensionObject_Id:
            for(idx = 0; idx < length; idx++){
                if(STATUS_OK == status) status = SOPC_ExtensionObject_Write(&array->ExtObjectArr[idx], msgBuffer);
            }
            break;
        case SOPC_DataValue_Id:
            for(idx = 0; idx < length; idx++){
                if(STATUS_OK == status) status = SOPC_DataValue_Write(&array->DataValueArr[idx], msgBuffer);
            }
            break;
        case SOPC_Variant_Id:
            for(idx = 0; idx < length; idx++){
                if(STATUS_OK == status) status = SOPC_Variant_Write(&array->VariantArr[idx], msgBuffer);
            }
            break;
        case SOPC_DiagnosticInfo_Id:
            for(idx = 0; idx < length; idx++){
                if(STATUS_OK == status) status = SOPC_DiagnosticInfo_Write(&array->DiagInfoArr[idx], msgBuffer);
            }
            break;
        default:
            break;
    }
    return status;
}

SOPC_StatusCode SOPC_Variant_Write(const SOPC_Variant* variant, SOPC_MsgBuffer* msgBuffer){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_Byte encodingByte = 0;
    int32_t arrayLength = 0;
    if(variant != NULL){
        encodingByte = GetVariantEncodingMask(variant);
        status = SOPC_Byte_Write(&encodingByte, msgBuffer);
    }
    if(status == STATUS_OK){
        if((variant->ArrayTypeMask & SOPC_VariantArrayValueFlag) != 0){
            if((variant->ArrayTypeMask & SOPC_VariantArrayMatrixFlag) != 0){
                int32_t idx = 0;
                for(idx = 0; idx < variant->Value.Matrix.Dimensions; idx ++){
                    arrayLength *= variant->Value.Matrix.ArrayDimensions[idx];
                }
            }else{
                arrayLength = variant->Value.Array.Length;
            }
            status = SOPC_Int32_Write(&arrayLength, msgBuffer);
        }else if((variant->ArrayTypeMask & SOPC_VariantArrayMatrixFlag) != 0){
            status = STATUS_INVALID_PARAMETERS;
        }
    }
    if(status == STATUS_OK){
        if((variant->ArrayTypeMask & SOPC_VariantArrayValueFlag) != 0){
            if((variant->ArrayTypeMask & SOPC_VariantArrayMatrixFlag) != 0){
                status = WriteVariantArrayBuiltInType(msgBuffer,
                                                      variant->BuiltInTypeMask,
                                                      &variant->Value.Matrix.Content,
                                                      arrayLength);
                // Encode dimension array
                if(status == STATUS_OK){
                    // length
                    status = SOPC_Int32_Write(&variant->Value.Matrix.Dimensions, msgBuffer);
                }
                if(status == STATUS_OK){
                    // array
                    int32_t idx = 0;
                    for(idx = 0; idx < variant->Value.Matrix.Dimensions; idx ++){
                        status = SOPC_Int32_Write(&variant->Value.Matrix.ArrayDimensions[idx], msgBuffer);
                    }
                }
            }else{
                status = WriteVariantArrayBuiltInType(msgBuffer,
                                                      variant->BuiltInTypeMask,
                                                      &variant->Value.Array.Content,
                                                      arrayLength);
            }
        }else{
            // We already checked that matrix flag => array flag, here it's a single builtin type
            status = WriteVariantNonArrayBuiltInType(msgBuffer,
                                                     variant->BuiltInTypeMask,
                                                     &variant->Value);
        }
    }
    return status;
}

SOPC_StatusCode ReadVariantNonArrayBuiltInType(SOPC_MsgBuffer* msgBuffer,
                                           SOPC_BuiltinId builtInTypeId,
                                           SOPC_VariantValue *val){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    switch(builtInTypeId){
        case SOPC_Boolean_Id:
            status = SOPC_Boolean_Read(&val->Boolean, msgBuffer);
            break;
        case SOPC_SByte_Id:
            status = SOPC_SByte_Read(&val->Sbyte, msgBuffer);
            break;
        case SOPC_Byte_Id:
            status = SOPC_Byte_Read(&val->Byte, msgBuffer);
            break;
        case SOPC_Int16_Id:
            status = SOPC_Int16_Read(&val->Int16, msgBuffer);
            break;
        case SOPC_UInt16_Id:
            status = SOPC_UInt16_Read(&val->Uint16, msgBuffer);
            break;
        case SOPC_Int32_Id:
            status = SOPC_Int32_Read(&val->Int32, msgBuffer);
            break;
        case SOPC_UInt32_Id:
            status = SOPC_UInt32_Read(&val->Uint32, msgBuffer);
            break;
        case SOPC_Int64_Id:
            status = SOPC_Int64_Read(&val->Int64, msgBuffer);
            break;
        case SOPC_UInt64_Id:
            status = SOPC_UInt64_Read(&val->Uint64, msgBuffer);
            break;
        case SOPC_Float_Id:
            status = SOPC_Float_Read(&val->Floatv, msgBuffer);
            break;
        case SOPC_Double_Id:
            status = SOPC_Double_Read(&val->Doublev, msgBuffer);
            break;
        case SOPC_String_Id:
            status = SOPC_String_Read(&val->String, msgBuffer);
            break;
        case SOPC_DateTime_Id:
            status = SOPC_DateTime_Read(&val->Date, msgBuffer);
            break;
        case SOPC_Guid_Id:
            val->Guid = malloc(sizeof(SOPC_Guid));
            if(val->Guid != NULL){
                status = SOPC_Guid_Read(val->Guid, msgBuffer);
            }else{
                status = STATUS_NOK;
            }
            break;
        case SOPC_ByteString_Id:
            status = SOPC_ByteString_Read(&val->Bstring, msgBuffer);
            break;
        case SOPC_XmlElement_Id:
            status = SOPC_XmlElement_Read(&val->XmlElt, msgBuffer);
            break;
        case SOPC_NodeId_Id:
            val->NodeId = malloc(sizeof(SOPC_NodeId));
            if(val->NodeId != NULL){
                status = SOPC_NodeId_Read(val->NodeId, msgBuffer);
            }else{
                status = STATUS_NOK;
            }
            break;
        case SOPC_ExpandedNodeId_Id:
            val->ExpNodeId = malloc(sizeof(SOPC_ExpandedNodeId));
            if(val->ExpNodeId != NULL){
                status = SOPC_ExpandedNodeId_Read(val->ExpNodeId, msgBuffer);
            }else{
                status = STATUS_NOK;
            }
            break;
        case SOPC_StatusCode_Id:
            status = SOPC_StatusCode_Read(&val->Status, msgBuffer);
            break;
        case SOPC_QualifiedName_Id:
            val->Qname = malloc(sizeof(SOPC_QualifiedName));
            if(val->Qname != NULL){
                status = SOPC_QualifiedName_Read(val->Qname, msgBuffer);
            }else{
                status = STATUS_NOK;
            }
            break;
        case SOPC_LocalizedText_Id:
            val->LocalizedText = malloc(sizeof(SOPC_LocalizedText));
            if(val->LocalizedText != NULL){
                status = SOPC_LocalizedText_Read(val->LocalizedText, msgBuffer);
            }else{
                status = STATUS_NOK;
            }
            break;
        case SOPC_ExtensionObject_Id:
            val->ExtObject = malloc(sizeof(SOPC_ExtensionObject));
            if(val->ExtObject != NULL){
                status = SOPC_ExtensionObject_Read(val->ExtObject, msgBuffer);
            }else{
                status = STATUS_NOK;
            }
            break;
        case SOPC_DataValue_Id:
            val->DataValue = malloc(sizeof(SOPC_DataValue));
            if(val->DataValue != NULL){
                status = SOPC_DataValue_Read(val->DataValue, msgBuffer);
            }else{
                status = STATUS_NOK;
            }
            break;
        case SOPC_Variant_Id:
            assert(FALSE);
            break;
        case SOPC_DiagnosticInfo_Id:
            val->DiagInfo = malloc(sizeof(SOPC_DiagnosticInfo));
            if(val->DiagInfo != NULL){
                status = SOPC_DiagnosticInfo_Read(val->DiagInfo, msgBuffer);
            }else{
                status = STATUS_NOK;
            }
            break;
        default:
            break;
    }
    return status;
}

SOPC_StatusCode ReadVariantArrayBuiltInType(SOPC_MsgBuffer* msgBuffer,
                                        SOPC_BuiltinId builtInTypeId,
                                        SOPC_VariantArrayValue* array,
                                        int32_t length)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    int32_t idx = 0;
    switch(builtInTypeId){
        case SOPC_Boolean_Id:
            array->BooleanArr = malloc(sizeof(SOPC_Boolean) * length);
            if(array->BooleanArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= SOPC_Boolean_Read(&array->BooleanArr[idx], msgBuffer);
                }
            }
            break;
        case SOPC_SByte_Id:
            array->SbyteArr = malloc(sizeof(SOPC_SByte) * length);
            if(array->SbyteArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= SOPC_SByte_Read(&array->SbyteArr[idx], msgBuffer);
                }
            }
            break;
        case SOPC_Byte_Id:
            array->ByteArr = malloc(sizeof(SOPC_Byte) * length);
            if(array->ByteArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= SOPC_Byte_Read(&array->ByteArr[idx], msgBuffer);
                }
            }
            break;
        case SOPC_Int16_Id:
            array->Int16Arr = malloc(sizeof(int16_t) * length);
            if(array->Int16Arr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= SOPC_Int16_Read(&array->Int16Arr[idx], msgBuffer);
                }
            }
            break;
        case SOPC_UInt16_Id:
            array->SbyteArr = malloc(sizeof(uint16_t) * length);
            if(array->SbyteArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= SOPC_UInt16_Read(&array->Uint16Arr[idx], msgBuffer);
                }
            }
            break;
        case SOPC_Int32_Id:
            array->Int32Arr = malloc(sizeof(int32_t) * length);
            if(array->Int32Arr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= SOPC_Int32_Read(&array->Int32Arr[idx], msgBuffer);
                }
            }
            break;
        case SOPC_UInt32_Id:
            array->Uint32Arr = malloc(sizeof(uint32_t) * length);
            if(array->Uint32Arr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= SOPC_UInt32_Read(&array->Uint32Arr[idx], msgBuffer);
                }
            }
            break;
        case SOPC_Int64_Id:
            array->Int64Arr = malloc(sizeof(int64_t) * length);
            if(array->Int64Arr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= SOPC_Int64_Read(&array->Int64Arr[idx], msgBuffer);
                }
            }
            break;
        case SOPC_UInt64_Id:
            array->Uint64Arr = malloc(sizeof(uint64_t) * length);
            if(array->Uint64Arr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= SOPC_UInt64_Read(&array->Uint64Arr[idx], msgBuffer);
                }
            }
            break;
        case SOPC_Float_Id:
            array->FloatvArr = malloc(sizeof(float) * length);
            if(array->FloatvArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= SOPC_Float_Read(&array->FloatvArr[idx], msgBuffer);
                }
            }
            break;
        case SOPC_Double_Id:
            array->DoublevArr = malloc(sizeof(double) * length);
            if(array->DoublevArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= SOPC_Double_Read(&array->DoublevArr[idx], msgBuffer);
                }
            }
            break;
        case SOPC_String_Id:
            array->StringArr = malloc(sizeof(SOPC_String) * length);
            if(array->StringArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= SOPC_String_Read(&array->StringArr[idx], msgBuffer);
                }
            }
            break;
        case SOPC_DateTime_Id:
            array->DateArr = malloc(sizeof(SOPC_DateTime) * length);
            if(array->DateArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= SOPC_DateTime_Read(&array->DateArr[idx], msgBuffer);
                }
            }
            break;
        case SOPC_Guid_Id:
            array->GuidArr = malloc(sizeof(SOPC_Guid) * length);
            if(array->GuidArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= SOPC_Guid_Read(&array->GuidArr[idx], msgBuffer);
                }
            }
            break;
        case SOPC_ByteString_Id:
            array->BstringArr = malloc(sizeof(SOPC_ByteString) * length);
            if(array->BstringArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= SOPC_ByteString_Read(&array->BstringArr[idx], msgBuffer);
                }
            }
            break;
        case SOPC_XmlElement_Id:
            array->XmlEltArr = malloc(sizeof(SOPC_XmlElement) * length);
            if(array->XmlEltArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= SOPC_XmlElement_Read(&array->XmlEltArr[idx], msgBuffer);
                }
            }
            break;
        case SOPC_NodeId_Id:
            array->NodeIdArr = malloc(sizeof(SOPC_NodeId) * length);
            if(array->NodeIdArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= SOPC_NodeId_Read(&array->NodeIdArr[idx], msgBuffer);
                }
            }
            break;
        case SOPC_ExpandedNodeId_Id:
            array->ExpNodeIdArr = malloc(sizeof(SOPC_ExpandedNodeId) * length);
            if(array->ExpNodeIdArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= SOPC_ExpandedNodeId_Read(&array->ExpNodeIdArr[idx], msgBuffer);
                }
            }
            break;
        case SOPC_StatusCode_Id:
            array->StatusArr = malloc(sizeof(SOPC_StatusCode) * length);
            if(array->StatusArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= SOPC_StatusCode_Read(&array->StatusArr[idx], msgBuffer);
                }
            }
            break;
        case SOPC_QualifiedName_Id:
            array->QnameArr = malloc(sizeof(SOPC_QualifiedName) * length);
            if(array->QnameArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= SOPC_QualifiedName_Read(&array->QnameArr[idx], msgBuffer);
                }
            }
            break;
        case SOPC_LocalizedText_Id:
            array->LocalizedTextArr = malloc(sizeof(SOPC_LocalizedText) * length);
            if(array->LocalizedTextArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= SOPC_LocalizedText_Read(&array->LocalizedTextArr[idx], msgBuffer);
                }
            }
            break;
        case SOPC_ExtensionObject_Id:
            array->ExtObjectArr = malloc(sizeof(SOPC_ExtensionObject) * length);
            if(array->ExtObjectArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= SOPC_ExtensionObject_Read(&array->ExtObjectArr[idx], msgBuffer);
                }
            }
            break;
        case SOPC_DataValue_Id:
            array->DataValueArr = malloc(sizeof(SOPC_DataValue) * length);
            if(array->DataValueArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= SOPC_DataValue_Read(&array->DataValueArr[idx], msgBuffer);
                }
            }
            break;
        case SOPC_Variant_Id:
            array->VariantArr = malloc(sizeof(SOPC_Variant) * length);
            if(array->VariantArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= SOPC_Variant_Read(&array->VariantArr[idx], msgBuffer);
                }
            }
            break;
        case SOPC_DiagnosticInfo_Id:
            array->DiagInfoArr = malloc(sizeof(SOPC_DiagnosticInfo) * length);
            if(array->DiagInfoArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= SOPC_DiagnosticInfo_Read(&array->DiagInfoArr[idx], msgBuffer);
                }
            }
            break;
        default:
            break;
    }
    return status;
}

SOPC_StatusCode SOPC_Variant_Read(SOPC_Variant* variant, SOPC_MsgBuffer* msgBuffer){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_Byte encodingByte = 0;
    int32_t arrayLength = 0;
    if(variant != NULL){
        status = SOPC_Byte_Read(&encodingByte, msgBuffer);
    }
    if(status == STATUS_OK){
        // Retrieve array flags
        if((encodingByte & SOPC_VariantArrayValueFlag) != 0){
            variant->ArrayTypeMask = SOPC_VariantArrayValueFlag;
            if((encodingByte & SOPC_VariantArrayMatrixFlag) != 0){
                variant->ArrayTypeMask |= SOPC_VariantArrayMatrixFlag;
            }
            // Read array length
            status = SOPC_Int32_Read(&arrayLength, msgBuffer);
        }else if((encodingByte & SOPC_VariantArrayMatrixFlag) != 0){
            status = STATUS_INVALID_PARAMETERS;
        }
        // Retrieve builtin type id: avoid 2^7 and 2^6 which are array flags
        variant->BuiltInTypeMask = 0x3F & encodingByte;
    }

    if(status == STATUS_OK){
        if((variant->ArrayTypeMask & SOPC_VariantArrayValueFlag) != 0){
            status = ReadVariantArrayBuiltInType(msgBuffer,
                                                 variant->BuiltInTypeMask,
                                                 &variant->Value.Matrix.Content,
                                                 arrayLength);
            if(status == STATUS_OK && (variant->ArrayTypeMask & SOPC_VariantArrayMatrixFlag) != 0){
                // Decode dimension array
                if(status == STATUS_OK){
                    // length
                    status = SOPC_Int32_Read(&variant->Value.Matrix.Dimensions, msgBuffer);
                }
                if(status == STATUS_OK){
                    // array
                    variant->Value.Matrix.ArrayDimensions = malloc(sizeof(int32_t) * variant->Value.Matrix.Dimensions);
                    if(variant->Value.Matrix.ArrayDimensions != NULL){
                        int32_t idx = 0;
                        for(idx = 0; idx < variant->Value.Matrix.Dimensions; idx ++){
                            status = SOPC_Int32_Read(&variant->Value.Matrix.ArrayDimensions[idx], msgBuffer);
                        }
                    }else{
                        status = STATUS_NOK;
                    }
                }
            }else{
                status = ReadVariantArrayBuiltInType(msgBuffer,
                                                     variant->BuiltInTypeMask,
                                                     &variant->Value.Array.Content,
                                                     arrayLength);
            }
        }else{
            // We already checked that matrix flag => array flag, here it's a single builtin type
            status = ReadVariantNonArrayBuiltInType(msgBuffer,
                                                    variant->BuiltInTypeMask,
                                                    &variant->Value);
        }
    }
    return status;
}

SOPC_Byte GetDataValueEncodingMask(const SOPC_DataValue* dataValue){
    assert(dataValue != NULL);
    SOPC_Byte mask = 0;
    if(dataValue->Value.BuiltInTypeMask != SOPC_Null_Id && dataValue->Value.BuiltInTypeMask <= SOPC_BUILTINID_MAX){
        mask |= SOPC_DataValue_NotNullValue;
    }
    if(dataValue->Status != STATUS_OK){
        mask |= SOPC_DataValue_NotGoodStatusCode;
    }
    if(dataValue->SourceTimestamp > 0){
        mask |= SOPC_DataValue_NotMinSourceDate;
    }
    if(dataValue->SourcePicoSeconds > 0){
        mask |= SOPC_DataValue_NotZeroSourcePico;
    }
    if(dataValue->ServerTimestamp > 0){
            mask |= SOPC_DataValue_NotMinServerDate;
    }
    if(dataValue->ServerPicoSeconds > 0){
        mask |= SOPC_DataValue_NotZeroServerPico;
    }
    return mask;
}

SOPC_StatusCode SOPC_DataValue_Write(const SOPC_DataValue* dataValue, SOPC_MsgBuffer* msgBuffer)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_Byte encodingMask = 0;
    if(dataValue != NULL){
        encodingMask = GetDataValueEncodingMask(dataValue);
        status = SOPC_Byte_Write(&encodingMask, msgBuffer);
    }
    if(status == STATUS_OK && (encodingMask & SOPC_DataValue_NotNullValue) != 0){
        status = SOPC_Variant_Write(&dataValue->Value, msgBuffer);
    }
    if(status == STATUS_OK && (encodingMask & SOPC_DataValue_NotGoodStatusCode) != 0){
        status = SOPC_StatusCode_Write(&dataValue->Status, msgBuffer);
    }
    if(status == STATUS_OK && (encodingMask & SOPC_DataValue_NotMinSourceDate) != 0){
        status = SOPC_DateTime_Write(&dataValue->SourceTimestamp, msgBuffer);
    }
    if(status == STATUS_OK && (encodingMask & SOPC_DataValue_NotZeroSourcePico) != 0){
        status = SOPC_UInt16_Write(&dataValue->SourcePicoSeconds, msgBuffer);
    }
    if(status == STATUS_OK && (encodingMask & SOPC_DataValue_NotMinServerDate) != 0){
            status = SOPC_DateTime_Write(&dataValue->ServerTimestamp, msgBuffer);
    }
    if(status == STATUS_OK && (encodingMask & SOPC_DataValue_NotZeroServerPico) != 0){
        status = SOPC_UInt16_Write(&dataValue->ServerPicoSeconds, msgBuffer);
    }
    return status;
}

SOPC_StatusCode SOPC_DataValue_Read(SOPC_DataValue* dataValue, SOPC_MsgBuffer* msgBuffer){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_Byte encodingMask = 0;
    if(dataValue != NULL){
        status = SOPC_Byte_Read(&encodingMask, msgBuffer);
    }
    if(status == STATUS_OK && (encodingMask & SOPC_DataValue_NotNullValue) != 0){
        status = SOPC_Variant_Read(&dataValue->Value, msgBuffer);
    }else{
        dataValue->Value.BuiltInTypeMask = SOPC_Null_Id;
    }
    if(status == STATUS_OK && (encodingMask & SOPC_DataValue_NotGoodStatusCode) != 0){
        status = SOPC_StatusCode_Read(&dataValue->Status, msgBuffer);
    }else{
        dataValue->Status = STATUS_OK;
    }
    if(status == STATUS_OK && (encodingMask & SOPC_DataValue_NotMinSourceDate) != 0){
        status = SOPC_DateTime_Read(&dataValue->SourceTimestamp, msgBuffer);
    }else{
        dataValue->SourceTimestamp = 0;
    }
    if(status == STATUS_OK && (encodingMask & SOPC_DataValue_NotZeroSourcePico) != 0){
        status = SOPC_UInt16_Read(&dataValue->SourcePicoSeconds, msgBuffer);
    }else{
        dataValue->SourcePicoSeconds = 0;
    }
    if(status == STATUS_OK && (encodingMask & SOPC_DataValue_NotMinServerDate) != 0){
            status = SOPC_DateTime_Read(&dataValue->ServerTimestamp, msgBuffer);
    }else{
        dataValue->ServerTimestamp = 0;
    }
    if(status == STATUS_OK && (encodingMask & SOPC_DataValue_NotZeroServerPico) != 0){
        status = SOPC_UInt16_Read(&dataValue->ServerPicoSeconds, msgBuffer);
    }else{
        dataValue->ServerPicoSeconds = 0;
    }
    return status;
}

