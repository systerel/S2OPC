/*
 * ua_encoder.c
 *
 *  Created on: Aug 3, 2016
 *      Author: vincent
 */

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "ua_encoder.h"
#include <ua_encodeable.h>
#include <ua_tcp_ua_low_level.h>

void EncodeDecode_Int16(int16_t* intv)
{
    uint16_t* twoBytes = (uint16_t*) intv;
    assert(endianess != P_Endianess_Undefined);
    if(endianess == P_Endianess_BigEndian){
        *twoBytes = SWAP_2_BYTES(*twoBytes);
    }
}

void EncodeDecode_UInt16(uint16_t* uintv)
{
    assert(endianess != P_Endianess_Undefined);
    if(endianess == P_Endianess_BigEndian){
        *uintv = SWAP_2_BYTES(*uintv);
    }
}

void EncodeDecode_Int32(int32_t* intv)
{
    assert(endianess != P_Endianess_Undefined);
    uint32_t* fourBytes = (uint32_t*) intv;
    if(endianess == P_Endianess_BigEndian){
        *fourBytes = SWAP_4_BYTES(*fourBytes);
    }
}


void EncodeDecode_UInt32(uint32_t* uintv)
{
    assert(endianess != P_Endianess_Undefined);
    if(endianess == P_Endianess_BigEndian){
        *uintv = SWAP_4_BYTES(*uintv);
    }
}

void EncodeDecode_Int64(int64_t* intv)
{
    assert(endianess != P_Endianess_Undefined);
    uint64_t* eightBytes = (uint64_t*) intv;
    if(endianess == P_Endianess_BigEndian){
        *eightBytes = SWAP_8_BYTES(*eightBytes);
    }
}

void EncodeDecode_UInt64(uint64_t* uintv)
{
    assert(endianess != P_Endianess_Undefined);
    if(endianess == P_Endianess_BigEndian){
        *uintv = SWAP_8_BYTES(*uintv);
    }
}

void EncodeDecode_Float(float* floatv){
    assert(floatEndianess != P_Endianess_Undefined);
    uint32_t* fourBytes = (uint32_t*) floatv;
    if(floatEndianess == P_Endianess_BigEndian){
        *fourBytes = SWAP_4_BYTES(*fourBytes);
    }
}

void EncodeDecode_Double(double* doublev){
    assert(floatEndianess != P_Endianess_Undefined);
    uint64_t* eightBytes = (uint64_t*) doublev;
    if(floatEndianess == P_Endianess_BigEndian){
        *eightBytes = SWAP_4_BYTES(*eightBytes);
    }
}

StatusCode Byte_Write(const UA_Byte* value, UA_MsgBuffer* msgBuffer)
{
    if(value == NULL){
        return STATUS_INVALID_PARAMETERS;
    }
    return TCP_UA_WriteMsgBuffer(msgBuffer, value, 1);
}

StatusCode Byte_Read(UA_Byte* value, UA_MsgBuffer* msgBuffer)
{
    if(value == NULL){
        return STATUS_INVALID_PARAMETERS;
    }

    return TCP_UA_ReadMsgBuffer(value, 1, msgBuffer, 1);
}

StatusCode Boolean_Write(const UA_Boolean* value, UA_MsgBuffer* msgBuffer)
{
    UA_Byte encodedValue;
    if(value == NULL){
        return STATUS_INVALID_PARAMETERS;
    }

    if(*value == UA_FALSE){
        encodedValue = *value;
    }else{
        // Encoder should use 1 as True value
        encodedValue = 1;
    }

    return Byte_Write(&encodedValue, msgBuffer);
}

StatusCode Boolean_Read(UA_Boolean* value, UA_MsgBuffer* msgBuffer)
{
    StatusCode status = STATUS_NOK;
    if(value == NULL){
        status = STATUS_INVALID_PARAMETERS;
    }else{
        status = Byte_Read((UA_Byte*) value, msgBuffer);
        if(status == STATUS_OK){
            if(*value != UA_FALSE){
                // Decoder should use 1 as True value
                *value = 1;
            }
        }
    }
    return status;
}

StatusCode SByte_Write(const UA_SByte* value, UA_MsgBuffer* msgBuffer)
{
    if(value == NULL){
        return STATUS_INVALID_PARAMETERS;
    }
    return TCP_UA_WriteMsgBuffer(msgBuffer, (UA_Byte*) value, 1);
}

StatusCode SByte_Read(UA_SByte* value, UA_MsgBuffer* msgBuffer)
{
    if(value == NULL){
        return STATUS_INVALID_PARAMETERS;
    }

    return TCP_UA_ReadMsgBuffer((UA_Byte*) value, 1, msgBuffer, 1);
}

StatusCode Int16_Write(const int16_t* value, UA_MsgBuffer* msgBuffer)
{
    int16_t encodedValue = *value;
    StatusCode status = STATUS_NOK;
    EncodeDecode_Int16(&encodedValue);
    status = TCP_UA_WriteMsgBuffer(msgBuffer, (UA_Byte*) &encodedValue, 2);
    return status;
}

StatusCode Int16_Read(int16_t* value, UA_MsgBuffer* msgBuffer)
{
    StatusCode status = STATUS_NOK;
    int16_t readValue;
    if(value == NULL){
        status = STATUS_INVALID_PARAMETERS;
    }else{
        status = TCP_UA_ReadMsgBuffer((UA_Byte*)&readValue, 2, msgBuffer, 2);
        if(status == STATUS_OK){
            EncodeDecode_Int16(&readValue);
            *value = readValue;
        }
    }
    return status;
}

StatusCode UInt16_Write(const uint16_t* value, UA_MsgBuffer* msgBuffer)
{
    StatusCode status = STATUS_NOK;
    uint16_t encodedValue = *value;
    EncodeDecode_UInt16(&encodedValue);
    status = TCP_UA_WriteMsgBuffer(msgBuffer, (UA_Byte*) &encodedValue, 2);
    return status;
}

StatusCode UInt16_Read(uint16_t* value, UA_MsgBuffer* msgBuffer)
{
    StatusCode status = STATUS_NOK;
    if(value == NULL){
        status = STATUS_INVALID_PARAMETERS;
    }else{
        status = TCP_UA_ReadMsgBuffer((UA_Byte*)value, 2, msgBuffer, 2);
        if(status == STATUS_OK){
            EncodeDecode_UInt16(value);
        }
    }
    return status;
}

StatusCode Int32_Write(const int32_t* value, UA_MsgBuffer* msgBuffer)
{
    StatusCode status = STATUS_NOK;
    int32_t encodedValue = *value;
    if(value == NULL){
            status = STATUS_INVALID_PARAMETERS;
    }else{
        EncodeDecode_Int32(&encodedValue);
        status = TCP_UA_WriteMsgBuffer(msgBuffer, (UA_Byte*) &encodedValue, 4);
    }
    return status;
}

StatusCode Int32_Read(int32_t* value, UA_MsgBuffer* msgBuffer)
{
    StatusCode status = STATUS_NOK;
    if(value == NULL){
        status = STATUS_INVALID_PARAMETERS;
    }else{
        status = TCP_UA_ReadMsgBuffer((UA_Byte*)value, 4, msgBuffer, 4);
        if(status == STATUS_OK){
            EncodeDecode_Int32(value);
        }
    }
    return status;
}

StatusCode UInt32_Write(const uint32_t* value, UA_MsgBuffer* msgBuffer)
{
    StatusCode status = STATUS_NOK;
    uint32_t encodedValue = *value;
    EncodeDecode_UInt32(&encodedValue);
    status = TCP_UA_WriteMsgBuffer(msgBuffer, (UA_Byte*) &encodedValue, 4);
    return status;
}

StatusCode UInt32_Read(uint32_t* value, UA_MsgBuffer* msgBuffer)
{
    StatusCode status = STATUS_NOK;
    if(value == NULL){
        status = STATUS_INVALID_PARAMETERS;
    }else{
        status = TCP_UA_ReadMsgBuffer((UA_Byte*)value, 4, msgBuffer, 4);
        if(status == STATUS_OK){
            EncodeDecode_UInt32(value);
        }
    }
    return status;
}

StatusCode Int64_Write(const int64_t* value, UA_MsgBuffer* msgBuffer)
{
    StatusCode status = STATUS_NOK;
    int64_t encodedValue = *value;
    if(value == NULL){
            status = STATUS_INVALID_PARAMETERS;
    }else{
        EncodeDecode_Int64(&encodedValue);
        status = TCP_UA_WriteMsgBuffer(msgBuffer, (UA_Byte*) &encodedValue, 8);
    }
    return status;
}

StatusCode Int64_Read(int64_t* value, UA_MsgBuffer* msgBuffer)
{
    StatusCode status = STATUS_NOK;
    if(value == NULL){
        status = STATUS_INVALID_PARAMETERS;
    }else{
        status = TCP_UA_ReadMsgBuffer((UA_Byte*) value, 8, msgBuffer, 8);
        if(status == STATUS_OK){
            EncodeDecode_Int64(value);
        }
    }
    return status;
}

StatusCode UInt64_Write(const uint64_t* value, UA_MsgBuffer* msgBuffer)
{
    StatusCode status = STATUS_NOK;
    uint64_t encodedValue = *value;
    if(value == NULL){
            status = STATUS_INVALID_PARAMETERS;
    }else{
        EncodeDecode_UInt64(&encodedValue);
        status = TCP_UA_WriteMsgBuffer(msgBuffer, (UA_Byte*) &encodedValue, 8);
    }
    return status;
}

StatusCode UInt64_Read(uint64_t* value, UA_MsgBuffer* msgBuffer)
{
    StatusCode status = STATUS_NOK;
    if(value == NULL){
        status = STATUS_INVALID_PARAMETERS;
    }else{
        status = TCP_UA_ReadMsgBuffer((UA_Byte*) value, 8, msgBuffer, 8);
        if(status == STATUS_OK){
            EncodeDecode_UInt64(value);
        }
    }
    return status;
}

StatusCode Float_Write(const float* value, UA_MsgBuffer* msgBuffer)
{
    StatusCode status = STATUS_NOK;
    float encodedValue = *value;
    EncodeDecode_Float(&encodedValue);
    status = TCP_UA_WriteMsgBuffer(msgBuffer, (UA_Byte*) &encodedValue, 4);
    return status;
}

StatusCode Float_Read(float* value, UA_MsgBuffer* msgBuffer)
{
    StatusCode status = STATUS_NOK;
    if(value == NULL){
        status = STATUS_INVALID_PARAMETERS;
    }else{
        status = TCP_UA_ReadMsgBuffer((UA_Byte*)value, 4, msgBuffer, 4);
        if(status == STATUS_OK){
            EncodeDecode_Float(value);
        }
    }
    return status;
}

StatusCode Double_Write(const double* value, UA_MsgBuffer* msgBuffer)
{
    StatusCode status = STATUS_NOK;
    double encodedValue = *value;
    EncodeDecode_Double(&encodedValue);
    status = TCP_UA_WriteMsgBuffer(msgBuffer, (UA_Byte*) &encodedValue, 4);
    return status;
}

StatusCode Double_Read(double* value, UA_MsgBuffer* msgBuffer)
{
    StatusCode status = STATUS_NOK;
    if(value == NULL){
        status = STATUS_INVALID_PARAMETERS;
    }else{
        status = TCP_UA_ReadMsgBuffer((UA_Byte*)value, 4, msgBuffer, 4);
        if(status == STATUS_OK){
            EncodeDecode_Double(value);
        }
    }
    return status;
}

StatusCode ByteString_Write(const UA_ByteString* str, UA_MsgBuffer* msgBuffer)
{
    StatusCode status = STATUS_NOK;
    if(str == NULL){
        status = STATUS_INVALID_PARAMETERS;
    }else{
        int32_t length;
        if(str->length > 0){
            length = str->length;
        }else{
            length = -1;
        }
        status = Int32_Write(&length, msgBuffer);
        if(status == STATUS_OK &&
           str->length > 0)
        {
            status = TCP_UA_WriteMsgBuffer(msgBuffer, str->characters, str->length);
        }
    }
    return status;
}

StatusCode ByteString_Read(UA_ByteString* str, UA_MsgBuffer* msgBuffer)
{
    StatusCode status = STATUS_NOK;
    int32_t length;
    if(str == NULL || (str != NULL && str->characters != NULL)){
        status = STATUS_INVALID_PARAMETERS;
    }else{
        status = TCP_UA_ReadMsgBuffer((UA_Byte*)&length, 4, msgBuffer, 4);
        if(status == STATUS_OK){
            EncodeDecode_Int32(&length);
            if(length > 0){
                str->length = length;
                str->characters = malloc(sizeof(UA_Byte) * length);
                if(str->characters != NULL){
                    status = TCP_UA_ReadMsgBuffer(str->characters, length, msgBuffer, length);
                    if(status != STATUS_OK){
                        status = STATUS_INVALID_STATE;
                        free(str->characters);
                        str->characters = NULL;
                        str->length = -1;
                    }
                }
            }else{
                str->length = -1;
            }
        }

    }
    return status;
}

StatusCode String_Write(const UA_String* str, UA_MsgBuffer* msgBuffer)
{
    return ByteString_Write((UA_ByteString*) str, msgBuffer);
}

StatusCode String_Read(UA_String* str, UA_MsgBuffer* msgBuffer)
{
    return ByteString_Read((UA_ByteString*) str, msgBuffer);
}

StatusCode XmlElement_Write(const UA_XmlElement* xml, UA_MsgBuffer* msgBuffer)
{
    // TODO: check XML validity ?
    return ByteString_Write((UA_ByteString*) xml, msgBuffer);
}

StatusCode XmlElement_Read(UA_XmlElement* xml, UA_MsgBuffer* msgBuffer)
{
    // TODO: parse XML ?
    return ByteString_Read((UA_ByteString*) xml, msgBuffer);
}

StatusCode DateTime_Write(const UA_DateTime* date, UA_MsgBuffer* msgBuffer)
{
    return Int64_Write(date, msgBuffer);
}

StatusCode DateTime_Read(UA_DateTime* date, UA_MsgBuffer* msgBuffer)
{
    return Int64_Read(date, msgBuffer);
}

StatusCode Guid_Write(const UA_Guid* guid, UA_MsgBuffer* msgBuffer)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(guid != NULL){
        status = UInt32_Write(&guid->data1, msgBuffer);
    }
    if(status == STATUS_OK){
        status = UInt16_Write(&guid->data2, msgBuffer);
    }
    if(status == STATUS_OK){
        status = UInt16_Write(&guid->data3, msgBuffer);
    }
    if(status == STATUS_OK){
        status = TCP_UA_WriteMsgBuffer(msgBuffer, &(guid->data4[0]), 8);
    }
    return status;
}

StatusCode Guid_Read(UA_Guid* guid, UA_MsgBuffer* msgBuffer)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(guid != NULL){
        status = UInt32_Read(&guid->data1, msgBuffer);
    }
    if(status == STATUS_OK){
        status = UInt16_Read(&guid->data2, msgBuffer);
    }
    if(status == STATUS_OK){
        status = UInt16_Read(&guid->data3, msgBuffer);
    }
    if(status == STATUS_OK){
        status = TCP_UA_ReadMsgBuffer(&(guid->data4[0]), 8, msgBuffer, 8);
    }
    return status;
}

UA_NodeId_DataEncoding GetNodeIdDataEncoding(const UA_NodeId* nodeId){
    UA_NodeId_DataEncoding encodingEnum = NodeIdEncoding_Invalid;
    switch(nodeId->identifierType){
        case IdentifierType_Numeric:
            if(nodeId->numeric <= UINT8_MAX){
                encodingEnum = NodeIdEncoding_TwoByte;
            }else if(nodeId->numeric <= UINT16_MAX){
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

StatusCode Internal_NodeId_Write(UA_MsgBuffer* msgBuffer,
                                 UA_Byte encodingByte,
                                 const UA_NodeId* nodeId)
{
    assert(nodeId != NULL);
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_NodeId_DataEncoding encodingType = 0x0F & encodingByte; // Eliminate flags

    UA_Byte byte = 0;
    uint16_t twoBytes = 0;

    status = Byte_Write(&encodingByte, msgBuffer);
    if(status == STATUS_OK){
        switch(encodingType){
            case NodeIdEncoding_Invalid:
                status = STATUS_INVALID_PARAMETERS;
                break;
            case NodeIdEncoding_TwoByte:
                byte = (UA_Byte) nodeId->numeric;
                status = Byte_Write(&byte, msgBuffer);
                break;
            case  NodeIdEncoding_FourByte:
                twoBytes = (uint16_t) nodeId->numeric;
                if(nodeId->namespace <= UINT8_MAX){
                    const UA_Byte namespace = nodeId->namespace;
                    status = Byte_Write(&namespace, msgBuffer);
                }else{
                    status = STATUS_INVALID_PARAMETERS;
                }
                if(status == STATUS_OK){
                    status = UInt16_Write(&twoBytes, msgBuffer);
                }
                break;
            case  NodeIdEncoding_Numeric:
                status = UInt16_Write(&nodeId->namespace, msgBuffer);
                if(status == STATUS_OK){
                    status = UInt32_Write(&nodeId->numeric, msgBuffer);
                }
                break;
            case  NodeIdEncoding_String:
                status = UInt16_Write(&nodeId->namespace, msgBuffer);
                if(status == STATUS_OK){
                    status = String_Write(&nodeId->string, msgBuffer);
                }
                break;
            case  NodeIdEncoding_Guid:
                status = UInt16_Write(&nodeId->namespace, msgBuffer);
                if(status == STATUS_OK){
                    status = Guid_Write(&nodeId->guid, msgBuffer);
                }
                break;
            case  NodeIdEncoding_ByteString:
                status = UInt16_Write(&nodeId->namespace, msgBuffer);
                if(status == STATUS_OK){
                    status = ByteString_Write(&nodeId->bstring, msgBuffer);
                }
                break;
            default:
                status = STATUS_INVALID_PARAMETERS;
        }
    }

    return status;
}

StatusCode NodeId_Write(const UA_NodeId* nodeId, UA_MsgBuffer* msgBuffer)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(nodeId != NULL){
        status = Internal_NodeId_Write(msgBuffer, GetNodeIdDataEncoding(nodeId), nodeId);
    }
    return status;
}

StatusCode Internal_NodeId_Read(UA_MsgBuffer* msgBuffer,
                                UA_NodeId* nodeId,
                                UA_Byte* encodingByte)
{
    assert(nodeId != NULL);
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_NodeId_DataEncoding encodingType = 0x0F;

    UA_Byte byte = 0;
    uint16_t twoBytes = 0;

    status = Byte_Read(encodingByte, msgBuffer);

    if(status == STATUS_OK){
        encodingType = 0x0F & *encodingByte; // Eliminate flags
        switch(encodingType){
            case NodeIdEncoding_Invalid:
                status = STATUS_INVALID_RCV_PARAMETER;
                break;
            case NodeIdEncoding_TwoByte:
                nodeId->identifierType = IdentifierType_Numeric;
                nodeId->namespace = 0;
                status = Byte_Read(&byte, msgBuffer);
                nodeId->numeric = (uint32_t) byte;
                break;
            case  NodeIdEncoding_FourByte:
                nodeId->identifierType = IdentifierType_Numeric;
                status = Byte_Read(&byte, msgBuffer);
                nodeId->namespace = byte;
                if(status == STATUS_OK){
                    status = UInt16_Read(&twoBytes, msgBuffer);
                    nodeId->numeric = twoBytes;
                }
                break;
            case  NodeIdEncoding_Numeric:
                nodeId->identifierType = IdentifierType_Numeric;
                status = UInt16_Read(&nodeId->namespace, msgBuffer);
                if(status == STATUS_OK){
                    status = UInt32_Read(&nodeId->numeric, msgBuffer);
                }
                break;
            case  NodeIdEncoding_String:
                nodeId->identifierType = IdentifierType_String;
                status = UInt16_Read(&nodeId->namespace, msgBuffer);
                if(status == STATUS_OK){
                    status = String_Read(&nodeId->string, msgBuffer);
                }
                break;
            case  NodeIdEncoding_Guid:
                nodeId->identifierType = IdentifierType_Guid;
                status = UInt16_Read(&nodeId->namespace, msgBuffer);
                if(status == STATUS_OK){
                    status = Guid_Read(&nodeId->guid, msgBuffer);
                }
                break;
            case  NodeIdEncoding_ByteString:
                nodeId->identifierType = IdentifierType_ByteString;
                status = UInt16_Read(&nodeId->namespace, msgBuffer);
                if(status == STATUS_OK){
                    status = ByteString_Read(&nodeId->bstring, msgBuffer);
                }
                break;
            default:
                status = STATUS_INVALID_PARAMETERS;
        }
    }

    return status;
}

StatusCode NodeId_Read(UA_NodeId* nodeId, UA_MsgBuffer* msgBuffer)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_Byte encodingByte = 0;
    if(nodeId != NULL){
        status = Internal_NodeId_Read(msgBuffer, nodeId, &encodingByte);
    }
    return status;
}

StatusCode ExpandedNodeId_Write(const UA_ExpandedNodeId* expNodeId, UA_MsgBuffer* msgBuffer){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_Byte encodingByte = 0xFF;
    if(expNodeId != NULL){
        encodingByte = GetNodeIdDataEncoding(&expNodeId->nodeId);
        if(expNodeId->namespaceUri.length > 0){
            encodingByte |= NodeIdEncoding_NamespaceUriFlag;
        }
        if(expNodeId->serverIndex > 0){
            encodingByte |= NodeIdEncoding_ServerIndexFlag;
        }
        status = Internal_NodeId_Write(msgBuffer, encodingByte, &expNodeId->nodeId);
    }
    if(status == STATUS_OK && expNodeId->namespaceUri.length > 0)
    {
        status = String_Write(&expNodeId->namespaceUri, msgBuffer);
    }
    if(status == STATUS_OK && expNodeId->serverIndex > 0)
    {
        status = UInt32_Write(&expNodeId->serverIndex, msgBuffer);
    }

    return status;
}

StatusCode ExpandedNodeId_Read(UA_ExpandedNodeId* expNodeId, UA_MsgBuffer* msgBuffer){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_Byte encodingByte = 0;
    if(expNodeId != NULL){
        status = Internal_NodeId_Read(msgBuffer, &expNodeId->nodeId, &encodingByte);
    }
    if(status == STATUS_OK &&
       (encodingByte & NodeIdEncoding_NamespaceUriFlag) != 0x00)
    {
        status = String_Read(&expNodeId->namespaceUri, msgBuffer);
    }else{
        String_Clear(&expNodeId->namespaceUri);
    }
    if(status == STATUS_OK &&
        (encodingByte & NodeIdEncoding_ServerIndexFlag) != 0)
    {
        status = UInt32_Read(&expNodeId->serverIndex, msgBuffer);
    }else{
        UInt32_Clear(&expNodeId->serverIndex);
    }
    return status;
}

StatusCode StatusCode_Write(const StatusCode* status, UA_MsgBuffer* msgBuffer){
    return UInt32_Write(status, msgBuffer);
}

StatusCode StatusCode_Read(StatusCode* status, UA_MsgBuffer* msgBuffer){
    return UInt32_Read(status, msgBuffer);
}

UA_Byte GetDiagInfoEncodingByte(const UA_DiagnosticInfo* diagInfo){
    assert(diagInfo != NULL);
    UA_Byte encodingByte = 0x00;
    if(diagInfo->symbolicId > -1){
        encodingByte |= DiagInfoEncoding_SymbolicId;
    }
    if(diagInfo->namespaceUri > -1){
        encodingByte |= DiagInfoEncoding_Namespace;
    }
    if(diagInfo->locale > -1){
        encodingByte |= DiagInfoEncoding_Locale;
    }
    if(diagInfo->localizedText > -1){
        encodingByte |= DiagInfoEncoding_LocalizedTest;
    }
    if(diagInfo->additionalInfo.length > 0){
        encodingByte |= DiagInfoEncoding_AdditionalInfo;
    }
    if(diagInfo->innerStatusCode > 0){ // OK status code does not provide information
        encodingByte |= DiagInfoEncoding_InnerStatusCode;
    }
    if(diagInfo->innerDiagnosticInfo != NULL){
        encodingByte |= DiagInfoEncoding_InnerDianosticInfo;
    }
    return encodingByte;
}

StatusCode DiagnosticInfo_Write(const UA_DiagnosticInfo* diagInfo, UA_MsgBuffer* msgBuffer){
    UA_Byte encodingByte = 0x00;
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(diagInfo != NULL){
        status = STATUS_OK;
        encodingByte = GetDiagInfoEncodingByte(diagInfo);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_SymbolicId) != 0x00){
        status = Int32_Write(&diagInfo->symbolicId, msgBuffer);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_Namespace) != 0x00){
        status = Int32_Write(&diagInfo->namespaceUri, msgBuffer);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_Locale) != 0x00){
        status = Int32_Write(&diagInfo->locale, msgBuffer);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_LocalizedTest) != 0x00){
        status = Int32_Write(&diagInfo->localizedText, msgBuffer);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_AdditionalInfo) != 0x00){
        status = String_Write(&diagInfo->additionalInfo, msgBuffer);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_InnerStatusCode) != 0x00){
        status = StatusCode_Write(&diagInfo->innerStatusCode, msgBuffer);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_InnerDianosticInfo) != 0x00){
        status = DiagnosticInfo_Write(diagInfo->innerDiagnosticInfo, msgBuffer);
    }
    return status;
}

StatusCode DiagnosticInfo_Read(UA_DiagnosticInfo* diagInfo, UA_MsgBuffer* msgBuffer){
    UA_Byte encodingByte = 0x00;
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(diagInfo != NULL){
        status  = Byte_Read(&encodingByte, msgBuffer);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_SymbolicId) != 0x00){
        status = Int32_Read(&diagInfo->symbolicId, msgBuffer);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_Namespace) != 0x00){
        status = Int32_Read(&diagInfo->namespaceUri, msgBuffer);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_Locale) != 0x00){
        status = Int32_Read(&diagInfo->locale, msgBuffer);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_LocalizedTest) != 0x00){
        status = Int32_Read(&diagInfo->localizedText, msgBuffer);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_AdditionalInfo) != 0x00){
        status = String_Read(&diagInfo->additionalInfo, msgBuffer);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_InnerStatusCode) != 0x00){
        status = StatusCode_Read(&diagInfo->innerStatusCode, msgBuffer);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_InnerDianosticInfo) != 0x00){
        status = DiagnosticInfo_Read(diagInfo->innerDiagnosticInfo, msgBuffer);
    }
    return status;
}

StatusCode QualifiedName_Write(const UA_QualifiedName* qname, UA_MsgBuffer* msgBuffer){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(qname != NULL){
        status = UInt16_Write(&qname->namespaceIndex, msgBuffer);
    }
    if(status == STATUS_OK){
        status = String_Write(&qname->name, msgBuffer);
    }
    return status;
}

StatusCode QualifiedName_Read(UA_QualifiedName* qname, UA_MsgBuffer* msgBuffer){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(qname != NULL){
        status = UInt16_Read(&qname->namespaceIndex, msgBuffer);
    }
    if(status == STATUS_OK){
        status = String_Read(&qname->name, msgBuffer);
    }
    return status;
}


UA_Byte GetLocalizedTextEncodingByte(const UA_LocalizedText* ltext){
    assert(ltext != NULL);
    UA_Byte encodingByte = 0;
    if(ltext->locale.length > 0){
        encodingByte |= LocalizedText_Locale;
    }
    if(ltext->text.length > 0){
        encodingByte |= LocalizedText_Text;
    }
    return encodingByte;
}

StatusCode LocalizedText_Write(const UA_LocalizedText* localizedText, UA_MsgBuffer* msgBuffer){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_Byte encodingByte = 0;
    if(localizedText != NULL){
        encodingByte = GetLocalizedTextEncodingByte(localizedText);
    }
    if(status == STATUS_OK && (encodingByte & LocalizedText_Locale) != 0){
        status = String_Write(&localizedText->locale, msgBuffer);
    }
    if(status == STATUS_OK && (encodingByte & LocalizedText_Text) != 0){
        status = String_Write(&localizedText->text, msgBuffer);
    }
    return status;
}

StatusCode LocalizedText_Read(UA_LocalizedText* localizedText, UA_MsgBuffer* msgBuffer){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_Byte encodingByte = 0;
    if(localizedText != NULL){
        status = Byte_Read(&encodingByte, msgBuffer);
    }
    if(status == STATUS_OK && (encodingByte & LocalizedText_Locale) != 0){
        status = String_Read(&localizedText->locale, msgBuffer);
    }
    if(status == STATUS_OK && (encodingByte & LocalizedText_Text) != 0){
        status = String_Read(&localizedText->text, msgBuffer);
    }
    return status;
}

StatusCode ExtensionObject_Write(const UA_ExtensionObject* extObj, UA_MsgBuffer* msgBuffer){
    const int32_t tmpLength = -1;
    UA_NodeId objNodeId = extObj->typeId;
    uint32_t lengthPos;
    uint32_t curPos;
    int32_t length;
    uint16_t nsIndex = OPCUA_NAMESPACE_INDEX;
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_Byte encodingByte = 0;
    if(extObj != NULL){
        encodingByte = extObj->encoding;
        status = STATUS_OK;
    }

    if(status == STATUS_OK &&
       encodingByte == UA_ExtObjBodyEncoding_Object)
    {
        encodingByte = UA_ExtObjBodyEncoding_ByteString;
        if(extObj->body.object.objType == NULL){
            status = STATUS_INVALID_PARAMETERS;
        }else{
            if(strncmp(extObj->body.object.objType->namespace,
                       OPCUA_NAMESPACE_NAME,
                       strlen(OPCUA_NAMESPACE_NAME))
               !=  0)
            {
                status = Namespace_GetIndex(&msgBuffer->nsTable, extObj->body.object.objType->namespace, &nsIndex);
            }

            objNodeId.identifierType = IdentifierType_Numeric;
            objNodeId.namespace = nsIndex;
            objNodeId.numeric = extObj->body.object.objType->binaryTypeId;
        }
    }

    if(status == STATUS_OK){
        status = NodeId_Write(&objNodeId, msgBuffer);
    }

    if(status == STATUS_OK){
        status = Byte_Write(&encodingByte, msgBuffer);
    }

    if(status == STATUS_OK){
        switch(extObj->encoding){
            case UA_ExtObjBodyEncoding_None:
                break;
            case UA_ExtObjBodyEncoding_ByteString:
                status = ByteString_Write(&extObj->body.bstring, msgBuffer);
                break;
            case UA_ExtObjBodyEncoding_XMLElement:
                status = XmlElement_Write(&extObj->body.xml, msgBuffer);
                break;
            case UA_ExtObjBodyEncoding_Object:
                lengthPos = msgBuffer->buffers->position;
                status = Int32_Write(&tmpLength, msgBuffer);
                if(status == STATUS_OK){
                    status = extObj->body.object.objType->encodeFunction(extObj->body.object.value, msgBuffer);
                }
                if(status == STATUS_OK){
                    // Go backward to write correct length value
                    curPos = msgBuffer->buffers->position;
                    length = curPos - (lengthPos + 4);
                    Buffer_SetPosition(msgBuffer->buffers, lengthPos);
                    Int32_Write(&length, msgBuffer);
                    Buffer_SetPosition(msgBuffer->buffers, curPos);
                }
                break;
        }
    }

    return status;
}

StatusCode ExtensionObject_Read(UA_ExtensionObject* extObj, UA_MsgBuffer* msgBuffer){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_EncodeableType* encType;
    const char* nsName;
    UA_Byte encodingByte = 0;
    if(extObj != NULL){
        status = NodeId_Read(&extObj->typeId, msgBuffer);
    }
    if(status == STATUS_OK){
        status = Byte_Read(&encodingByte, msgBuffer);
    }

    if(status == STATUS_OK &&
       encodingByte == UA_ExtObjBodyEncoding_ByteString){
        if(extObj->typeId.identifierType == IdentifierType_Numeric){
            if(extObj->typeId.namespace != OPCUA_NAMESPACE_INDEX){
                nsName = Namespace_GetName(&msgBuffer->nsTable, extObj->typeId.namespace);
            }else{
                nsName = OPCUA_NAMESPACE_NAME;
            }
            if(nsName != NULL){
                encType = EncodeableType_GetEncodeableType(msgBuffer->encTypesTable,
                                                           nsName,
                                                           extObj->typeId.numeric);
            }
            if(nsName == NULL || encType == NULL){
                status = STATUS_NOK;
            }else{
                encodingByte = UA_ExtObjBodyEncoding_Object;
                extObj->body.object.objType = encType;
            }
        }else{
            status = STATUS_INVALID_RCV_PARAMETER;
        }

    }

    if(status == STATUS_OK){
        switch(encodingByte){
            case UA_ExtObjBodyEncoding_None:
                extObj->length = -1;
                break;
            case UA_ExtObjBodyEncoding_ByteString:
                status = ByteString_Read(&extObj->body.bstring, msgBuffer);
                break;
            case UA_ExtObjBodyEncoding_XMLElement:
                status = XmlElement_Read(&extObj->body.xml, msgBuffer);
                break;
            case UA_ExtObjBodyEncoding_Object:
                status = Int32_Read(&extObj->length, msgBuffer);
                if(status == STATUS_OK){
                    extObj->body.object.value = malloc(extObj->body.object.objType->allocSize);
                    status = extObj->body.object.objType->decodeFunction(&extObj->body.object.value, msgBuffer);
                }
                break;
            default:
                status = STATUS_INVALID_RCV_PARAMETER;
        }
        if(status == STATUS_OK){
            extObj->encoding = encodingByte;
        }
    }

    return status;
}

UA_Byte GetVariantEncodingMask(const UA_Variant* variant){
    assert(variant != NULL);
    UA_Byte encodingByte = variant->builtInTypeMask;
    if((variant->arrayTypeMask & UA_VariantArrayMatrixFlag) != 0){
        encodingByte |= UA_VariantArrayMatrixFlag;
    }
    if((variant->arrayTypeMask & UA_VariantArrayValueFlag) != 0){
        encodingByte |= UA_VariantArrayValueFlag;
    }
    return encodingByte;
}

StatusCode WriteVariantNonArrayBuiltInType(UA_MsgBuffer* msgBuffer,
                                           UA_BuiltinId builtInTypeId,
                                           const UA_VariantValue *val){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    switch(builtInTypeId){
        case UA_Boolean_Id:
            status = Boolean_Write(&val->boolean, msgBuffer);
            break;
        case UA_SByte_Id:
            status = SByte_Write(&val->sbyte, msgBuffer);
            break;
        case UA_Byte_Id:
            status = Byte_Write(&val->byte, msgBuffer);
            break;
        case UA_Int16_Id:
            status = Int16_Write(&val->int16, msgBuffer);
            break;
        case UA_UInt16_Id:
            status = UInt16_Write(&val->uint16, msgBuffer);
            break;
        case UA_Int32_Id:
            status = Int32_Write(&val->int32, msgBuffer);
            break;
        case UA_UInt32_Id:
            status = UInt32_Write(&val->uint32, msgBuffer);
            break;
        case UA_Int64_Id:
            status = Int64_Write(&val->int64, msgBuffer);
            break;
        case UA_UInt64_Id:
            status = UInt64_Write(&val->uint64, msgBuffer);
            break;
        case UA_Float_Id:
            status = Float_Write(&val->floatv, msgBuffer);
            break;
        case UA_Double_Id:
            status = Double_Write(&val->doublev, msgBuffer);
            break;
        case UA_String_Id:
            status = String_Write(&val->string, msgBuffer);
            break;
        case UA_DateTime_Id:
            status = DateTime_Write(&val->date, msgBuffer);
            break;
        case UA_Guid_Id:
            status = Guid_Write(val->guid, msgBuffer);
            break;
        case UA_ByteString_Id:
            status = ByteString_Write(&val->bstring, msgBuffer);
            break;
        case UA_XmlElement_Id:
            status = XmlElement_Write(&val->xmlElt, msgBuffer);
            break;
        case UA_NodeId_Id:
            status = NodeId_Write(val->nodeId, msgBuffer);
            break;
        case UA_ExpandedNodeId_Id:
            status = ExpandedNodeId_Write(val->expNodeId, msgBuffer);
            break;
        case UA_StatusCode_Id:
            status = StatusCode_Write(&val->status, msgBuffer);
            break;
        case UA_QualifiedName_Id:
            status = QualifiedName_Write(val->qname, msgBuffer);
            break;
        case UA_LocalizedText_Id:
            status = LocalizedText_Write(val->localizedText, msgBuffer);
            break;
        case UA_ExtensionObject_Id:
            status = ExtensionObject_Write(val->extObject, msgBuffer);
            break;
        case UA_DataValue_Id:
            status = DataValue_Write(val->dataValue, msgBuffer);
            break;
        case UA_Variant_Id:
            assert(UA_FALSE);
            break;
        case UA_DiagnosticInfo_Id:
            status = DiagnosticInfo_Write(val->diagInfo, msgBuffer);
            break;
        default:
            break;
    }
    return status;
}

StatusCode WriteVariantArrayBuiltInType(UA_MsgBuffer* msgBuffer,
                                        UA_BuiltinId builtInTypeId,
                                        const UA_VariantArrayValue* array,
                                        int32_t length)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    int32_t idx = 0;
    switch(builtInTypeId){
        case UA_Boolean_Id:
            for(idx = 0; idx < length; idx++){
                status |= Boolean_Write(&array->booleanArr[idx], msgBuffer);
            }
            break;
        case UA_SByte_Id:
            for(idx = 0; idx < length; idx++){
                status |= SByte_Write(&array->sbyteArr[idx], msgBuffer);
            }
            break;
        case UA_Byte_Id:
            for(idx = 0; idx < length; idx++){
                status |= Byte_Write(&array->byteArr[idx], msgBuffer);
            }
            break;
        case UA_Int16_Id:
            for(idx = 0; idx < length; idx++){
                status |= Int16_Write(&array->int16Arr[idx], msgBuffer);
            }
            break;
        case UA_UInt16_Id:
            for(idx = 0; idx < length; idx++){
                status |= UInt16_Write(&array->uint16Arr[idx], msgBuffer);
            }
            break;
        case UA_Int32_Id:
            for(idx = 0; idx < length; idx++){
                status |= Int32_Write(&array->int32Arr[idx], msgBuffer);
            }
            break;
        case UA_UInt32_Id:
            for(idx = 0; idx < length; idx++){
                status |= UInt32_Write(&array->uint32Arr[idx], msgBuffer);
            }
            break;
        case UA_Int64_Id:
            for(idx = 0; idx < length; idx++){
                status |= Int64_Write(&array->int64Arr[idx], msgBuffer);
            }
            break;
        case UA_UInt64_Id:
            for(idx = 0; idx < length; idx++){
                status |= UInt64_Write(&array->uint64Arr[idx], msgBuffer);
            }
            break;
        case UA_Float_Id:
            for(idx = 0; idx < length; idx++){
                status |= Float_Write(&array->floatvArr[idx], msgBuffer);
            }
            break;
        case UA_Double_Id:
            for(idx = 0; idx < length; idx++){
                status |= Double_Write(&array->doublevArr[idx], msgBuffer);
            }
            break;
        case UA_String_Id:
            for(idx = 0; idx < length; idx++){
                status |= String_Write(&array->stringArr[idx], msgBuffer);
            }
            break;
        case UA_DateTime_Id:
            for(idx = 0; idx < length; idx++){
                status |= DateTime_Write(&array->dateArr[idx], msgBuffer);
            }
            break;
        case UA_Guid_Id:
            for(idx = 0; idx < length; idx++){
                status |= Guid_Write(&array->guidArr[idx], msgBuffer);
            }
            break;
        case UA_ByteString_Id:
            for(idx = 0; idx < length; idx++){
                status |= ByteString_Write(&array->bstringArr[idx], msgBuffer);
            }
            break;
        case UA_XmlElement_Id:
            for(idx = 0; idx < length; idx++){
                status |= XmlElement_Write(&array->xmlEltArr[idx], msgBuffer);
            }
            break;
        case UA_NodeId_Id:
            for(idx = 0; idx < length; idx++){
                status |= NodeId_Write(&array->nodeIdArr[idx], msgBuffer);
            }
            break;
        case UA_ExpandedNodeId_Id:
            for(idx = 0; idx < length; idx++){
                status |= ExpandedNodeId_Write(&array->expNodeIdArr[idx], msgBuffer);
            }
            break;
        case UA_StatusCode_Id:
            for(idx = 0; idx < length; idx++){
                status |= StatusCode_Write(&array->statusArr[idx], msgBuffer);
            }
            break;
        case UA_QualifiedName_Id:
            for(idx = 0; idx < length; idx++){
                status |= QualifiedName_Write(&array->qnameArr[idx], msgBuffer);
            }
            break;
        case UA_LocalizedText_Id:
            for(idx = 0; idx < length; idx++){
                status |= LocalizedText_Write(&array->localizedTextArr[idx], msgBuffer);
            }
            break;
        case UA_ExtensionObject_Id:
            for(idx = 0; idx < length; idx++){
                status |= ExtensionObject_Write(&array->extObjectArr[idx], msgBuffer);
            }
            break;
        case UA_DataValue_Id:
            for(idx = 0; idx < length; idx++){
                status |= DataValue_Write(&array->dataValueArr[idx], msgBuffer);
            }
            break;
        case UA_Variant_Id:
            for(idx = 0; idx < length; idx++){
                status |= Variant_Write(&array->variantArr[idx], msgBuffer);
            }
            break;
        case UA_DiagnosticInfo_Id:
            for(idx = 0; idx < length; idx++){
                status |= DiagnosticInfo_Write(&array->diagInfoArr[idx], msgBuffer);
            }
            break;
        default:
            break;
    }
    return status;
}

StatusCode Variant_Write(const UA_Variant* variant, UA_MsgBuffer* msgBuffer){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_Byte encodingByte = 0;
    int32_t arrayLength = 0;
    if(variant != NULL){
        encodingByte = GetVariantEncodingMask(variant);
        status = Byte_Write(&encodingByte, msgBuffer);
    }
    if(status == STATUS_OK){
        if((variant->arrayTypeMask & UA_VariantArrayValueFlag) != 0){
            if((variant->arrayTypeMask & UA_VariantArrayMatrixFlag) != 0){
                int32_t idx = 0;
                for(idx = 0; idx < variant->value.matrix.dimensions; idx ++){
                    arrayLength *= variant->value.matrix.arrayDimensions[idx];
                }
            }else{
                arrayLength = variant->value.array.length;
            }
            status = Int32_Write(&arrayLength, msgBuffer);
        }else if((variant->arrayTypeMask & UA_VariantArrayMatrixFlag) != 0){
            status = STATUS_INVALID_PARAMETERS;
        }
    }
    if(status == STATUS_OK){
        if((variant->arrayTypeMask & UA_VariantArrayValueFlag) != 0){
            if((variant->arrayTypeMask & UA_VariantArrayMatrixFlag) != 0){
                status = WriteVariantArrayBuiltInType(msgBuffer,
                                                      variant->builtInTypeMask,
                                                      &variant->value.matrix.content,
                                                      arrayLength);
                // Encode dimension array
                if(status == STATUS_OK){
                    // length
                    status = Int32_Write(&variant->value.matrix.dimensions, msgBuffer);
                }
                if(status == STATUS_OK){
                    // array
                    int32_t idx = 0;
                    for(idx = 0; idx < variant->value.matrix.dimensions; idx ++){
                        status = Int32_Write(&variant->value.matrix.arrayDimensions[idx], msgBuffer);
                    }
                }
            }else{
                status = WriteVariantArrayBuiltInType(msgBuffer,
                                                      variant->builtInTypeMask,
                                                      &variant->value.array.content,
                                                      arrayLength);
            }
        }else{
            // We already checked that matrix flag => array flag, here it's a single builtin type
            status = WriteVariantNonArrayBuiltInType(msgBuffer,
                                                     variant->builtInTypeMask,
                                                     &variant->value);
        }
    }
    return status;
}

StatusCode ReadVariantNonArrayBuiltInType(UA_MsgBuffer* msgBuffer,
                                           UA_BuiltinId builtInTypeId,
                                           UA_VariantValue *val){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    switch(builtInTypeId){
        case UA_Boolean_Id:
            status = Boolean_Read(&val->boolean, msgBuffer);
            break;
        case UA_SByte_Id:
            status = SByte_Read(&val->sbyte, msgBuffer);
            break;
        case UA_Byte_Id:
            status = Byte_Read(&val->byte, msgBuffer);
            break;
        case UA_Int16_Id:
            status = Int16_Read(&val->int16, msgBuffer);
            break;
        case UA_UInt16_Id:
            status = UInt16_Read(&val->uint16, msgBuffer);
            break;
        case UA_Int32_Id:
            status = Int32_Read(&val->int32, msgBuffer);
            break;
        case UA_UInt32_Id:
            status = UInt32_Read(&val->uint32, msgBuffer);
            break;
        case UA_Int64_Id:
            status = Int64_Read(&val->int64, msgBuffer);
            break;
        case UA_UInt64_Id:
            status = UInt64_Read(&val->uint64, msgBuffer);
            break;
        case UA_Float_Id:
            status = Float_Read(&val->floatv, msgBuffer);
            break;
        case UA_Double_Id:
            status = Double_Read(&val->doublev, msgBuffer);
            break;
        case UA_String_Id:
            status = String_Read(&val->string, msgBuffer);
            break;
        case UA_DateTime_Id:
            status = DateTime_Read(&val->date, msgBuffer);
            break;
        case UA_Guid_Id:
            val->guid = malloc(sizeof(UA_Guid));
            if(val->guid != NULL){
                status = Guid_Read(val->guid, msgBuffer);
            }else{
                status = STATUS_NOK;
            }
            break;
        case UA_ByteString_Id:
            status = ByteString_Read(&val->bstring, msgBuffer);
            break;
        case UA_XmlElement_Id:
            status = XmlElement_Read(&val->xmlElt, msgBuffer);
            break;
        case UA_NodeId_Id:
            val->nodeId = malloc(sizeof(UA_NodeId));
            if(val->nodeId != NULL){
                status = NodeId_Read(val->nodeId, msgBuffer);
            }else{
                status = STATUS_NOK;
            }
            break;
        case UA_ExpandedNodeId_Id:
            val->expNodeId = malloc(sizeof(UA_ExpandedNodeId));
            if(val->expNodeId != NULL){
                status = ExpandedNodeId_Read(val->expNodeId, msgBuffer);
            }else{
                status = STATUS_NOK;
            }
            break;
        case UA_StatusCode_Id:
            status = StatusCode_Read(&val->status, msgBuffer);
            break;
        case UA_QualifiedName_Id:
            val->qname = malloc(sizeof(UA_QualifiedName));
            if(val->qname != NULL){
                status = QualifiedName_Read(val->qname, msgBuffer);
            }else{
                status = STATUS_NOK;
            }
            break;
        case UA_LocalizedText_Id:
            val->localizedText = malloc(sizeof(UA_LocalizedText));
            if(val->localizedText != NULL){
                status = LocalizedText_Read(val->localizedText, msgBuffer);
            }else{
                status = STATUS_NOK;
            }
            break;
        case UA_ExtensionObject_Id:
            val->extObject = malloc(sizeof(UA_ExtensionObject));
            if(val->extObject != NULL){
                status = ExtensionObject_Read(val->extObject, msgBuffer);
            }else{
                status = STATUS_NOK;
            }
            break;
        case UA_DataValue_Id:
            val->dataValue = malloc(sizeof(UA_DataValue));
            if(val->dataValue != NULL){
                status = DataValue_Read(val->dataValue, msgBuffer);
            }else{
                status = STATUS_NOK;
            }
            break;
        case UA_Variant_Id:
            assert(UA_FALSE);
            break;
        case UA_DiagnosticInfo_Id:
            val->diagInfo = malloc(sizeof(UA_DiagnosticInfo));
            if(val->diagInfo != NULL){
                status = DiagnosticInfo_Read(val->diagInfo, msgBuffer);
            }else{
                status = STATUS_NOK;
            }
            break;
        default:
            break;
    }
    return status;
}

StatusCode ReadVariantArrayBuiltInType(UA_MsgBuffer* msgBuffer,
                                        UA_BuiltinId builtInTypeId,
                                        UA_VariantArrayValue* array,
                                        int32_t length)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    int32_t idx = 0;
    switch(builtInTypeId){
        case UA_Boolean_Id:
            array->booleanArr = malloc(sizeof(UA_Boolean) * length);
            if(array->booleanArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= Boolean_Read(&array->booleanArr[idx], msgBuffer);
                }
            }
            break;
        case UA_SByte_Id:
            array->sbyteArr = malloc(sizeof(UA_SByte) * length);
            if(array->sbyteArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= SByte_Read(&array->sbyteArr[idx], msgBuffer);
                }
            }
            break;
        case UA_Byte_Id:
            array->byteArr = malloc(sizeof(UA_Byte) * length);
            if(array->byteArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= Byte_Read(&array->byteArr[idx], msgBuffer);
                }
            }
            break;
        case UA_Int16_Id:
            array->int16Arr = malloc(sizeof(int16_t) * length);
            if(array->int16Arr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= Int16_Read(&array->int16Arr[idx], msgBuffer);
                }
            }
            break;
        case UA_UInt16_Id:
            array->sbyteArr = malloc(sizeof(uint16_t) * length);
            if(array->sbyteArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= UInt16_Read(&array->uint16Arr[idx], msgBuffer);
                }
            }
            break;
        case UA_Int32_Id:
            array->int32Arr = malloc(sizeof(int32_t) * length);
            if(array->int32Arr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= Int32_Read(&array->int32Arr[idx], msgBuffer);
                }
            }
            break;
        case UA_UInt32_Id:
            array->uint32Arr = malloc(sizeof(uint32_t) * length);
            if(array->uint32Arr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= UInt32_Read(&array->uint32Arr[idx], msgBuffer);
                }
            }
            break;
        case UA_Int64_Id:
            array->int64Arr = malloc(sizeof(int64_t) * length);
            if(array->int64Arr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= Int64_Read(&array->int64Arr[idx], msgBuffer);
                }
            }
            break;
        case UA_UInt64_Id:
            array->uint64Arr = malloc(sizeof(uint64_t) * length);
            if(array->uint64Arr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= UInt64_Read(&array->uint64Arr[idx], msgBuffer);
                }
            }
            break;
        case UA_Float_Id:
            array->floatvArr = malloc(sizeof(float) * length);
            if(array->floatvArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= Float_Read(&array->floatvArr[idx], msgBuffer);
                }
            }
            break;
        case UA_Double_Id:
            array->doublevArr = malloc(sizeof(double) * length);
            if(array->doublevArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= Double_Read(&array->doublevArr[idx], msgBuffer);
                }
            }
            break;
        case UA_String_Id:
            array->stringArr = malloc(sizeof(UA_String) * length);
            if(array->stringArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= String_Read(&array->stringArr[idx], msgBuffer);
                }
            }
            break;
        case UA_DateTime_Id:
            array->dateArr = malloc(sizeof(UA_DateTime) * length);
            if(array->dateArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= DateTime_Read(&array->dateArr[idx], msgBuffer);
                }
            }
            break;
        case UA_Guid_Id:
            array->guidArr = malloc(sizeof(UA_Guid) * length);
            if(array->guidArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= Guid_Read(&array->guidArr[idx], msgBuffer);
                }
            }
            break;
        case UA_ByteString_Id:
            array->bstringArr = malloc(sizeof(UA_ByteString) * length);
            if(array->bstringArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= ByteString_Read(&array->bstringArr[idx], msgBuffer);
                }
            }
            break;
        case UA_XmlElement_Id:
            array->xmlEltArr = malloc(sizeof(UA_XmlElement) * length);
            if(array->xmlEltArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= XmlElement_Read(&array->xmlEltArr[idx], msgBuffer);
                }
            }
            break;
        case UA_NodeId_Id:
            array->nodeIdArr = malloc(sizeof(UA_NodeId) * length);
            if(array->nodeIdArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= NodeId_Read(&array->nodeIdArr[idx], msgBuffer);
                }
            }
            break;
        case UA_ExpandedNodeId_Id:
            array->expNodeIdArr = malloc(sizeof(UA_ExpandedNodeId) * length);
            if(array->expNodeIdArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= ExpandedNodeId_Read(&array->expNodeIdArr[idx], msgBuffer);
                }
            }
            break;
        case UA_StatusCode_Id:
            array->statusArr = malloc(sizeof(StatusCode) * length);
            if(array->statusArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= StatusCode_Read(&array->statusArr[idx], msgBuffer);
                }
            }
            break;
        case UA_QualifiedName_Id:
            array->qnameArr = malloc(sizeof(UA_QualifiedName) * length);
            if(array->qnameArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= QualifiedName_Read(&array->qnameArr[idx], msgBuffer);
                }
            }
            break;
        case UA_LocalizedText_Id:
            array->localizedTextArr = malloc(sizeof(UA_LocalizedText) * length);
            if(array->localizedTextArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= LocalizedText_Read(&array->localizedTextArr[idx], msgBuffer);
                }
            }
            break;
        case UA_ExtensionObject_Id:
            array->extObjectArr = malloc(sizeof(UA_ExtensionObject) * length);
            if(array->extObjectArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= ExtensionObject_Read(&array->extObjectArr[idx], msgBuffer);
                }
            }
            break;
        case UA_DataValue_Id:
            array->dataValueArr = malloc(sizeof(UA_DataValue) * length);
            if(array->dataValueArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= DataValue_Read(&array->dataValueArr[idx], msgBuffer);
                }
            }
            break;
        case UA_Variant_Id:
            array->variantArr = malloc(sizeof(UA_Variant) * length);
            if(array->variantArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= Variant_Read(&array->variantArr[idx], msgBuffer);
                }
            }
            break;
        case UA_DiagnosticInfo_Id:
            array->diagInfoArr = malloc(sizeof(UA_DiagnosticInfo) * length);
            if(array->diagInfoArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= DiagnosticInfo_Read(&array->diagInfoArr[idx], msgBuffer);
                }
            }
            break;
        default:
            break;
    }
    return status;
}

StatusCode Variant_Read(UA_Variant* variant, UA_MsgBuffer* msgBuffer){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_Byte encodingByte = 0;
    int32_t arrayLength = 0;
    if(variant != NULL){
        status = Byte_Read(&encodingByte, msgBuffer);
    }
    if(status == STATUS_OK){
        // Retrieve array flags
        if((encodingByte & UA_VariantArrayValueFlag) != 0){
            variant->arrayTypeMask = UA_VariantArrayValueFlag;
            if((encodingByte & UA_VariantArrayMatrixFlag) != 0){
                variant->arrayTypeMask |= UA_VariantArrayMatrixFlag;
            }
            // Read array length
            status = Int32_Read(&arrayLength, msgBuffer);
        }else if((encodingByte & UA_VariantArrayMatrixFlag) != 0){
            status = STATUS_INVALID_PARAMETERS;
        }
        // Retrieve builtin type id: avoid 2^7 and 2^6 which are array flags
        variant->builtInTypeMask = 0x3F & encodingByte;
    }

    if(status == STATUS_OK){
        if((variant->arrayTypeMask & UA_VariantArrayValueFlag) != 0){
            status = ReadVariantArrayBuiltInType(msgBuffer,
                                                 variant->builtInTypeMask,
                                                 &variant->value.matrix.content,
                                                 arrayLength);
            if(status == STATUS_OK && (variant->arrayTypeMask & UA_VariantArrayMatrixFlag) != 0){
                // Decode dimension array
                if(status == STATUS_OK){
                    // length
                    status = Int32_Read(&variant->value.matrix.dimensions, msgBuffer);
                }
                if(status == STATUS_OK){
                    // array
                    variant->value.matrix.arrayDimensions = malloc(sizeof(int32_t) * variant->value.matrix.dimensions);
                    if(variant->value.matrix.arrayDimensions != NULL){
                        int32_t idx = 0;
                        for(idx = 0; idx < variant->value.matrix.dimensions; idx ++){
                            status = Int32_Read(&variant->value.matrix.arrayDimensions[idx], msgBuffer);
                        }
                    }else{
                        status = STATUS_NOK;
                    }
                }
            }else{
                status = ReadVariantArrayBuiltInType(msgBuffer,
                                                     variant->builtInTypeMask,
                                                     &variant->value.array.content,
                                                     arrayLength);
            }
        }else{
            // We already checked that matrix flag => array flag, here it's a single builtin type
            status = ReadVariantNonArrayBuiltInType(msgBuffer,
                                                    variant->builtInTypeMask,
                                                    &variant->value);
        }
    }
    return status;
}

UA_Byte GetDataValueEncodingMask(const UA_DataValue* dataValue){
    assert(dataValue != NULL);
    UA_Byte mask = 0;
    if(dataValue->value.builtInTypeMask != UA_Null_Id && dataValue->value.builtInTypeMask <= UA_BUILTINID_MAX){
        mask |= DataValue_NotNullValue;
    }
    if(dataValue->status != STATUS_OK){
        mask |= DataValue_NotGoodStatusCode;
    }
    if(dataValue->sourceTimestamp > 0){
        mask |= DataValue_NotMinSourceDate;
    }
    if(dataValue->sourcePicoSeconds > 0){
        mask |= DataValue_NotZeroSourcePico;
    }
    if(dataValue->serverTimestamp > 0){
            mask |= DataValue_NotMinServerDate;
    }
    if(dataValue->serverPicoSeconds > 0){
        mask |= DataValue_NotZeroServerPico;
    }
    return mask;
}

StatusCode DataValue_Write(const UA_DataValue* dataValue, UA_MsgBuffer* msgBuffer)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_Byte encodingMask = 0;
    if(dataValue != NULL){
        encodingMask = GetDataValueEncodingMask(dataValue);
        status = Byte_Write(&encodingMask, msgBuffer);
    }
    if(status == STATUS_OK && (encodingMask & DataValue_NotNullValue) != 0){
        status = Variant_Write(&dataValue->value, msgBuffer);
    }
    if(status == STATUS_OK && (encodingMask & DataValue_NotGoodStatusCode) != 0){
        status = StatusCode_Write(&dataValue->status, msgBuffer);
    }
    if(status == STATUS_OK && (encodingMask & DataValue_NotMinSourceDate) != 0){
        status = DateTime_Write(&dataValue->sourceTimestamp, msgBuffer);
    }
    if(status == STATUS_OK && (encodingMask & DataValue_NotZeroSourcePico) != 0){
        status = UInt16_Write(&dataValue->sourcePicoSeconds, msgBuffer);
    }
    if(status == STATUS_OK && (encodingMask & DataValue_NotMinServerDate) != 0){
            status = DateTime_Write(&dataValue->serverTimestamp, msgBuffer);
    }
    if(status == STATUS_OK && (encodingMask & DataValue_NotZeroServerPico) != 0){
        status = UInt16_Write(&dataValue->serverPicoSeconds, msgBuffer);
    }
    return status;
}

StatusCode DataValue_Read(UA_DataValue* dataValue, UA_MsgBuffer* msgBuffer){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_Byte encodingMask = 0;
    if(dataValue != NULL){
        status = Byte_Read(&encodingMask, msgBuffer);
    }
    if(status == STATUS_OK && (encodingMask & DataValue_NotNullValue) != 0){
        status = Variant_Read(&dataValue->value, msgBuffer);
    }else{
        dataValue->value.builtInTypeMask = UA_Null_Id;
    }
    if(status == STATUS_OK && (encodingMask & DataValue_NotGoodStatusCode) != 0){
        status = StatusCode_Read(&dataValue->status, msgBuffer);
    }else{
        dataValue->status = STATUS_OK;
    }
    if(status == STATUS_OK && (encodingMask & DataValue_NotMinSourceDate) != 0){
        status = DateTime_Read(&dataValue->sourceTimestamp, msgBuffer);
    }else{
        dataValue->sourceTimestamp = 0;
    }
    if(status == STATUS_OK && (encodingMask & DataValue_NotZeroSourcePico) != 0){
        status = UInt16_Read(&dataValue->sourcePicoSeconds, msgBuffer);
    }else{
        dataValue->sourcePicoSeconds = 0;
    }
    if(status == STATUS_OK && (encodingMask & DataValue_NotMinServerDate) != 0){
            status = DateTime_Read(&dataValue->serverTimestamp, msgBuffer);
    }else{
        dataValue->serverTimestamp = 0;
    }
    if(status == STATUS_OK && (encodingMask & DataValue_NotZeroServerPico) != 0){
        status = UInt16_Read(&dataValue->serverPicoSeconds, msgBuffer);
    }else{
        dataValue->serverPicoSeconds = 0;
    }
    return status;
}

