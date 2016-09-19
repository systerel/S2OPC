/*
 * ua_encoder.c
 *
 *  Created on: Aug 3, 2016
 *      Author: vincent
 */

#include <stdlib.h>
#include <assert.h>
#include <ua_encoder.h>
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

StatusCode Byte_Write(UA_MsgBuffer* msgBuffer, const UA_Byte* value)
{
    if(value == UA_NULL){
        return STATUS_INVALID_PARAMETERS;
    }
    return TCP_UA_WriteMsgBuffer(msgBuffer, value, 1);
}

StatusCode Byte_Read(UA_MsgBuffer* msgBuffer, UA_Byte* value)
{
    if(value == UA_NULL){
        return STATUS_INVALID_PARAMETERS;
    }

    return TCP_UA_ReadMsgBuffer(value, 1, msgBuffer, 1);
}

StatusCode Boolean_Write(UA_MsgBuffer* msgBuffer, const UA_Boolean* value)
{
    UA_Byte encodedValue;
    if(value == UA_NULL){
        return STATUS_INVALID_PARAMETERS;
    }

    if(*value == UA_FALSE){
        encodedValue = *value;
    }else{
        // Encoder should use 1 as True value
        encodedValue = 1;
    }

    return Byte_Write(msgBuffer, &encodedValue);
}

StatusCode Boolean_Read(UA_MsgBuffer* msgBuffer, UA_Boolean* value)
{
    StatusCode status = STATUS_NOK;
    if(value == UA_NULL){
        status = STATUS_INVALID_PARAMETERS;
    }else{
        status = Byte_Read(msgBuffer, (UA_Byte*) value);
        if(status == STATUS_OK){
            if(*value != UA_FALSE){
                // Decoder should use 1 as True value
                *value = 1;
            }
        }
    }
    return status;
}

StatusCode SByte_Write(UA_MsgBuffer* msgBuffer, const UA_SByte* value)
{
    if(value == UA_NULL){
        return STATUS_INVALID_PARAMETERS;
    }
    return TCP_UA_WriteMsgBuffer(msgBuffer, (UA_Byte*) value, 1);
}

StatusCode SByte_Read(UA_MsgBuffer* msgBuffer, UA_SByte* value)
{
    if(value == UA_NULL){
        return STATUS_INVALID_PARAMETERS;
    }

    return TCP_UA_ReadMsgBuffer((UA_Byte*) value, 1, msgBuffer, 1);
}

StatusCode Int16_Write(UA_MsgBuffer* msgBuffer, const int16_t* value)
{
    int16_t encodedValue = *value;
    StatusCode status = STATUS_NOK;
    EncodeDecode_Int16(&encodedValue);
    status = TCP_UA_WriteMsgBuffer(msgBuffer, (UA_Byte*) &encodedValue, 2);
    return status;
}

StatusCode Int16_Read(UA_MsgBuffer* msgBuffer, int16_t* value)
{
    StatusCode status = STATUS_NOK;
    int16_t readValue;
    if(value == UA_NULL){
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

StatusCode UInt16_Write(UA_MsgBuffer* msgBuffer, const uint16_t* value)
{
    StatusCode status = STATUS_NOK;
    uint16_t encodedValue = *value;
    EncodeDecode_UInt16(&encodedValue);
    status = TCP_UA_WriteMsgBuffer(msgBuffer, (UA_Byte*) &encodedValue, 2);
    return status;
}

StatusCode UInt16_Read(UA_MsgBuffer* msgBuffer, uint16_t* value)
{
    StatusCode status = STATUS_NOK;
    if(value == UA_NULL){
        status = STATUS_INVALID_PARAMETERS;
    }else{
        status = TCP_UA_ReadMsgBuffer((UA_Byte*)value, 2, msgBuffer, 2);
        if(status == STATUS_OK){
            EncodeDecode_UInt16(value);
        }
    }
    return status;
}

StatusCode Int32_Write(UA_MsgBuffer* msgBuffer, const int32_t* value)
{
    StatusCode status = STATUS_NOK;
    int32_t encodedValue = *value;
    if(value == UA_NULL){
            status = STATUS_INVALID_PARAMETERS;
    }else{
        EncodeDecode_Int32(&encodedValue);
        status = TCP_UA_WriteMsgBuffer(msgBuffer, (UA_Byte*) &encodedValue, 4);
    }
    return status;
}

StatusCode Int32_Read(UA_MsgBuffer* msgBuffer, int32_t* value)
{
    StatusCode status = STATUS_NOK;
    if(value == UA_NULL){
        status = STATUS_INVALID_PARAMETERS;
    }else{
        status = TCP_UA_ReadMsgBuffer((UA_Byte*)value, 4, msgBuffer, 4);
        if(status == STATUS_OK){
            EncodeDecode_Int32(value);
        }
    }
    return status;
}

StatusCode UInt32_Write(UA_MsgBuffer* msgBuffer, const uint32_t* value)
{
    StatusCode status = STATUS_NOK;
    uint32_t encodedValue = *value;
    EncodeDecode_UInt32(&encodedValue);
    status = TCP_UA_WriteMsgBuffer(msgBuffer, (UA_Byte*) &encodedValue, 4);
    return status;
}

StatusCode UInt32_Read(UA_MsgBuffer* msgBuffer, uint32_t* value)
{
    StatusCode status = STATUS_NOK;
    if(value == UA_NULL){
        status = STATUS_INVALID_PARAMETERS;
    }else{
        status = TCP_UA_ReadMsgBuffer((UA_Byte*)value, 4, msgBuffer, 4);
        if(status == STATUS_OK){
            EncodeDecode_UInt32(value);
        }
    }
    return status;
}

StatusCode Int64_Write(UA_MsgBuffer* msgBuffer, const int64_t* value)
{
    StatusCode status = STATUS_NOK;
    int64_t encodedValue = *value;
    if(value == UA_NULL){
            status = STATUS_INVALID_PARAMETERS;
    }else{
        EncodeDecode_Int64(&encodedValue);
        status = TCP_UA_WriteMsgBuffer(msgBuffer, (UA_Byte*) &encodedValue, 8);
    }
    return status;
}

StatusCode Int64_Read(UA_MsgBuffer* msgBuffer, int64_t* value)
{
    StatusCode status = STATUS_NOK;
    if(value == UA_NULL){
        status = STATUS_INVALID_PARAMETERS;
    }else{
        status = TCP_UA_ReadMsgBuffer((UA_Byte*) value, 8, msgBuffer, 8);
        if(status == STATUS_OK){
            EncodeDecode_Int64(value);
        }
    }
    return status;
}

StatusCode UInt64_Write(UA_MsgBuffer* msgBuffer, const uint64_t* value)
{
    StatusCode status = STATUS_NOK;
    uint64_t encodedValue = *value;
    if(value == UA_NULL){
            status = STATUS_INVALID_PARAMETERS;
    }else{
        EncodeDecode_UInt64(&encodedValue);
        status = TCP_UA_WriteMsgBuffer(msgBuffer, (UA_Byte*) &encodedValue, 8);
    }
    return status;
}

StatusCode UInt64_Read(UA_MsgBuffer* msgBuffer, uint64_t* value)
{
    StatusCode status = STATUS_NOK;
    if(value == UA_NULL){
        status = STATUS_INVALID_PARAMETERS;
    }else{
        status = TCP_UA_ReadMsgBuffer((UA_Byte*) value, 8, msgBuffer, 8);
        if(status == STATUS_OK){
            EncodeDecode_UInt64(value);
        }
    }
    return status;
}

StatusCode Float_Write(UA_MsgBuffer* msgBuffer, const float* value)
{
    StatusCode status = STATUS_NOK;
    float encodedValue = *value;
    EncodeDecode_Float(&encodedValue);
    status = TCP_UA_WriteMsgBuffer(msgBuffer, (UA_Byte*) &encodedValue, 4);
    return status;
}

StatusCode Float_Read(UA_MsgBuffer* msgBuffer, float* value)
{
    StatusCode status = STATUS_NOK;
    if(value == UA_NULL){
        status = STATUS_INVALID_PARAMETERS;
    }else{
        status = TCP_UA_ReadMsgBuffer((UA_Byte*)value, 4, msgBuffer, 4);
        if(status == STATUS_OK){
            EncodeDecode_Float(value);
        }
    }
    return status;
}

StatusCode Double_Write(UA_MsgBuffer* msgBuffer, const double* value)
{
    StatusCode status = STATUS_NOK;
    double encodedValue = *value;
    EncodeDecode_Double(&encodedValue);
    status = TCP_UA_WriteMsgBuffer(msgBuffer, (UA_Byte*) &encodedValue, 4);
    return status;
}

StatusCode Double_Read(UA_MsgBuffer* msgBuffer, double* value)
{
    StatusCode status = STATUS_NOK;
    if(value == UA_NULL){
        status = STATUS_INVALID_PARAMETERS;
    }else{
        status = TCP_UA_ReadMsgBuffer((UA_Byte*)value, 4, msgBuffer, 4);
        if(status == STATUS_OK){
            EncodeDecode_Double(value);
        }
    }
    return status;
}

StatusCode ByteString_Write(UA_MsgBuffer* msgBuffer, const UA_ByteString* str)
{
    StatusCode status = STATUS_NOK;
    if(str == UA_NULL){
        status = STATUS_INVALID_PARAMETERS;
    }else{
        int32_t length;
        if(str->length > 0){
            length = str->length;
        }else{
            length = -1;
        }
        status = Int32_Write(msgBuffer, &length);
        if(status == STATUS_OK &&
           str->length > 0)
        {
            status = TCP_UA_WriteMsgBuffer(msgBuffer, str->characters, str->length);
        }
    }
    return status;
}

StatusCode ByteString_Read(UA_MsgBuffer* msgBuffer, UA_ByteString* str)
{
    StatusCode status = STATUS_NOK;
    int32_t length;
    if(str == UA_NULL || (str != UA_NULL && str->characters != UA_NULL)){
        status = STATUS_INVALID_PARAMETERS;
    }else{
        status = TCP_UA_ReadMsgBuffer((UA_Byte*)&length, 4, msgBuffer, 4);
        if(status == STATUS_OK){
            EncodeDecode_Int32(&length);
            if(length > 0){
                str->length = length;
                str->characters = malloc(sizeof(UA_Byte) * length);
                if(str->characters != UA_NULL){
                    status = TCP_UA_ReadMsgBuffer(str->characters, length, msgBuffer, length);
                    if(status != STATUS_OK){
                        status = STATUS_INVALID_STATE;
                        free(str->characters);
                        str->characters = UA_NULL;
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

StatusCode String_Write(UA_MsgBuffer* msgBuffer, const UA_String* str)
{
    return ByteString_Write(msgBuffer, (UA_ByteString*) str);
}

StatusCode String_Read(UA_MsgBuffer* msgBuffer, UA_String* str)
{
    return ByteString_Read(msgBuffer, (UA_ByteString*) str);
}

StatusCode XmlElement_Write(UA_MsgBuffer* msgBuffer, const UA_XmlElement* xml)
{
    // TODO: check XML validity ?
    return ByteString_Write(msgBuffer, (UA_ByteString*) xml);
}

StatusCode XmlElement_Read(UA_MsgBuffer* msgBuffer, UA_XmlElement* xml)
{
    // TODO: parse XML ?
    return ByteString_Read(msgBuffer, (UA_ByteString*) xml);
}

StatusCode DateTime_Write(UA_MsgBuffer* msgBuffer, const UA_DateTime* date)
{
    return Int64_Write(msgBuffer, date);
}

StatusCode DateTime_Read(UA_MsgBuffer* msgBuffer, UA_DateTime* date)
{
    return Int64_Read(msgBuffer, date);
}

StatusCode Guid_Write(UA_MsgBuffer* msgBuffer, const UA_Guid* guid)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(guid != UA_NULL){
        status = UInt32_Write(msgBuffer, &guid->data1);
    }
    if(status == STATUS_OK){
        status = UInt16_Write(msgBuffer, &guid->data2);
    }
    if(status == STATUS_OK){
        status = UInt16_Write(msgBuffer, &guid->data3);
    }
    if(status == STATUS_OK){
        status = TCP_UA_WriteMsgBuffer(msgBuffer, &(guid->data4[0]), 8);
    }
    return status;
}

StatusCode Guid_Read(UA_MsgBuffer* msgBuffer, UA_Guid* guid)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(guid != UA_NULL){
        status = UInt32_Read(msgBuffer, &guid->data1);
    }
    if(status == STATUS_OK){
        status = UInt16_Read(msgBuffer, &guid->data2);
    }
    if(status == STATUS_OK){
        status = UInt16_Read(msgBuffer, &guid->data3);
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
        case IdentifierType_Undefined:
            encodingEnum = NodeIdEncoding_Invalid;
            break;
    }
    return encodingEnum;
}

StatusCode Internal_NodeId_Write(UA_MsgBuffer* msgBuffer,
                                 UA_Byte encodingByte,
                                 const UA_NodeId* nodeId)
{
    assert(nodeId != UA_NULL);
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_NodeId_DataEncoding encodingType = 0x0F & encodingByte; // Eliminate flags

    UA_Byte byte = 0;
    uint16_t twoBytes = 0;

    status = Byte_Write(msgBuffer, &encodingByte);
    if(status == STATUS_OK){
        switch(encodingType){
            case NodeIdEncoding_Invalid:
                status = STATUS_INVALID_PARAMETERS;
                break;
            case NodeIdEncoding_TwoByte:
                byte = (UA_Byte) nodeId->numeric;
                status = Byte_Write(msgBuffer, &byte);
                break;
            case  NodeIdEncoding_FourByte:
                twoBytes = (uint16_t) nodeId->numeric;
                if(nodeId->namespace <= UINT8_MAX){
                    const UA_Byte namespace = nodeId->namespace;
                    status = Byte_Write(msgBuffer, &namespace);
                }else{
                    status = STATUS_INVALID_PARAMETERS;
                }
                if(status == STATUS_OK){
                    status = UInt16_Write(msgBuffer, &twoBytes);
                }
                break;
            case  NodeIdEncoding_Numeric:
                status = UInt16_Write(msgBuffer, &nodeId->namespace);
                if(status == STATUS_OK){
                    status = UInt32_Write(msgBuffer, &nodeId->numeric);
                }
                break;
            case  NodeIdEncoding_String:
                status = UInt16_Write(msgBuffer, &nodeId->namespace);
                if(status == STATUS_OK){
                    status = String_Write(msgBuffer, &nodeId->string);
                }
                break;
            case  NodeIdEncoding_Guid:
                status = UInt16_Write(msgBuffer, &nodeId->namespace);
                if(status == STATUS_OK){
                    status = Guid_Write(msgBuffer, &nodeId->guid);
                }
                break;
            case  NodeIdEncoding_ByteString:
                status = UInt16_Write(msgBuffer, &nodeId->namespace);
                if(status == STATUS_OK){
                    status = ByteString_Write(msgBuffer, &nodeId->bstring);
                }
                break;
            default:
                status = STATUS_INVALID_PARAMETERS;
        }
    }

    return status;
}

StatusCode NodeId_Write(UA_MsgBuffer* msgBuffer, const UA_NodeId* nodeId)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(nodeId != UA_NULL){
        status = Internal_NodeId_Write(msgBuffer, GetNodeIdDataEncoding(nodeId), nodeId);
    }
    return status;
}

StatusCode Internal_NodeId_Read(UA_MsgBuffer* msgBuffer,
                                UA_NodeId* nodeId,
                                UA_Byte* encodingByte)
{
    assert(nodeId != UA_NULL);
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_NodeId_DataEncoding encodingType = 0x0F;

    UA_Byte byte = 0;
    uint16_t twoBytes = 0;

    status = Byte_Read(msgBuffer, encodingByte);

    if(status == STATUS_OK){
        encodingType = 0x0F & *encodingByte; // Eliminate flags
        switch(encodingType){
            case NodeIdEncoding_Invalid:
                status = STATUS_INVALID_RCV_PARAMETER;
                break;
            case NodeIdEncoding_TwoByte:
                nodeId->identifierType = IdentifierType_Numeric;
                nodeId->namespace = 0;
                status = Byte_Read(msgBuffer, &byte);
                nodeId->numeric = (uint32_t) byte;
                break;
            case  NodeIdEncoding_FourByte:
                nodeId->identifierType = IdentifierType_Numeric;
                status = Byte_Read(msgBuffer, &byte);
                nodeId->namespace = byte;
                if(status == STATUS_OK){
                    status = UInt16_Read(msgBuffer, &twoBytes);
                    nodeId->numeric = twoBytes;
                }
                break;
            case  NodeIdEncoding_Numeric:
                nodeId->identifierType = IdentifierType_Numeric;
                status = UInt16_Read(msgBuffer, &nodeId->namespace);
                if(status == STATUS_OK){
                    status = UInt32_Read(msgBuffer, &nodeId->numeric);
                }
                break;
            case  NodeIdEncoding_String:
                nodeId->identifierType = IdentifierType_String;
                status = UInt16_Read(msgBuffer, &nodeId->namespace);
                if(status == STATUS_OK){
                    status = String_Read(msgBuffer, &nodeId->string);
                }
                break;
            case  NodeIdEncoding_Guid:
                nodeId->identifierType = IdentifierType_Guid;
                status = UInt16_Read(msgBuffer, &nodeId->namespace);
                if(status == STATUS_OK){
                    status = Guid_Read(msgBuffer, &nodeId->guid);
                }
                break;
            case  NodeIdEncoding_ByteString:
                nodeId->identifierType = IdentifierType_ByteString;
                status = UInt16_Read(msgBuffer, &nodeId->namespace);
                if(status == STATUS_OK){
                    status = ByteString_Read(msgBuffer, &nodeId->bstring);
                }
                break;
            default:
                status = STATUS_INVALID_PARAMETERS;
        }
    }

    return status;
}

StatusCode NodeId_Read(UA_MsgBuffer* msgBuffer, UA_NodeId* nodeId)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_Byte encodingByte = 0;
    if(nodeId != UA_NULL){
        status = Internal_NodeId_Read(msgBuffer, nodeId, &encodingByte);
    }
    return status;
}

StatusCode ExpandedNodeId_Write(UA_MsgBuffer* msgBuffer, const UA_ExpandedNodeId* expNodeId){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_Byte encodingByte = 0xFF;
    if(expNodeId != UA_NULL){
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
        status = String_Write(msgBuffer, &expNodeId->namespaceUri);
    }
    if(status == STATUS_OK && expNodeId->serverIndex > 0)
    {
        status = UInt32_Write(msgBuffer, &expNodeId->serverIndex);
    }

    return status;
}

StatusCode ExpandedNodeId_Read(UA_MsgBuffer* msgBuffer, UA_ExpandedNodeId* expNodeId){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_Byte encodingByte = 0;
    if(expNodeId != UA_NULL){
        status = Internal_NodeId_Read(msgBuffer, &expNodeId->nodeId, &encodingByte);
    }
    if(status == STATUS_OK &&
       (encodingByte & NodeIdEncoding_NamespaceUriFlag) != 0x00)
    {
        status = String_Read(msgBuffer, &expNodeId->namespaceUri);
    }else{
        String_Clear(&expNodeId->namespaceUri);
    }
    if(status == STATUS_OK &&
        (encodingByte & NodeIdEncoding_ServerIndexFlag) != 0)
    {
        status = UInt32_Read(msgBuffer, &expNodeId->serverIndex);
    }else{
        UInt32_Clear(&expNodeId->serverIndex);
    }
    return status;
}

StatusCode StatusCode_Write(UA_MsgBuffer* msgBuffer, const StatusCode* status){
    return UInt32_Write(msgBuffer, status);
}

StatusCode StatusCode_Read(UA_MsgBuffer* msgBuffer, StatusCode* status){
    return UInt32_Read(msgBuffer, status);
}

UA_Byte GetDiagInfoEncodingByte(const UA_DiagnosticInfo* diagInfo){
    assert(diagInfo != UA_NULL);
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
    if(diagInfo->innerDiagnosticInfo != UA_NULL){
        encodingByte |= DiagInfoEncoding_InnerDianosticInfo;
    }
    return encodingByte;
}

StatusCode DiagnosticInfo_Write(UA_MsgBuffer* msgBuffer, const UA_DiagnosticInfo* diagInfo){
    UA_Byte encodingByte = 0x00;
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(diagInfo != UA_NULL){
        status = STATUS_OK;
        encodingByte = GetDiagInfoEncodingByte(diagInfo);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_SymbolicId) != 0x00){
        status = Int32_Write(msgBuffer, &diagInfo->symbolicId);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_Namespace) != 0x00){
        status = Int32_Write(msgBuffer, &diagInfo->namespaceUri);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_Locale) != 0x00){
        status = Int32_Write(msgBuffer, &diagInfo->locale);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_LocalizedTest) != 0x00){
        status = Int32_Write(msgBuffer, &diagInfo->localizedText);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_AdditionalInfo) != 0x00){
        status = String_Write(msgBuffer, &diagInfo->additionalInfo);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_InnerStatusCode) != 0x00){
        status = StatusCode_Write(msgBuffer, &diagInfo->innerStatusCode);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_InnerDianosticInfo) != 0x00){
        status = DiagnosticInfo_Write(msgBuffer, diagInfo->innerDiagnosticInfo);
    }
    return status;
}

StatusCode DiagnosticInfo_Read(UA_MsgBuffer* msgBuffer, UA_DiagnosticInfo* diagInfo){
    UA_Byte encodingByte = 0x00;
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(diagInfo != UA_NULL){
        status  = Byte_Read(msgBuffer, &encodingByte);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_SymbolicId) != 0x00){
        status = Int32_Read(msgBuffer, &diagInfo->symbolicId);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_Namespace) != 0x00){
        status = Int32_Read(msgBuffer, &diagInfo->namespaceUri);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_Locale) != 0x00){
        status = Int32_Read(msgBuffer, &diagInfo->locale);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_LocalizedTest) != 0x00){
        status = Int32_Read(msgBuffer, &diagInfo->localizedText);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_AdditionalInfo) != 0x00){
        status = String_Read(msgBuffer, &diagInfo->additionalInfo);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_InnerStatusCode) != 0x00){
        status = StatusCode_Read(msgBuffer, &diagInfo->innerStatusCode);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_InnerDianosticInfo) != 0x00){
        status = DiagnosticInfo_Read(msgBuffer, diagInfo->innerDiagnosticInfo);
    }
    return status;
}

StatusCode QualifiedName_Write(UA_MsgBuffer* msgBuffer, const UA_QualifiedName* qname){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(qname != UA_NULL){
        status = UInt16_Write(msgBuffer, &qname->namespaceIndex);
    }
    if(status == STATUS_OK){
        status = String_Write(msgBuffer, &qname->name);
    }
    return status;
}

StatusCode QualifiedName_Read(UA_MsgBuffer* msgBuffer, UA_QualifiedName* qname){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(qname != UA_NULL){
        status = UInt16_Read(msgBuffer, &qname->namespaceIndex);
    }
    if(status == STATUS_OK){
        status = String_Read(msgBuffer, &qname->name);
    }
    return status;
}


UA_Byte GetLocalizedTextEncodingByte(const UA_LocalizedText* ltext){
    assert(ltext != UA_NULL);
    UA_Byte encodingByte = 0;
    if(ltext->locale.length > 0){
        encodingByte |= LocalizedText_Locale;
    }
    if(ltext->text.length > 0){
        encodingByte |= LocalizedText_Text;
    }
    return encodingByte;
}

StatusCode LocalizedText_Write(UA_MsgBuffer* msgBuffer, const UA_LocalizedText* localizedText){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_Byte encodingByte = 0;
    if(localizedText != UA_NULL){
        encodingByte = GetLocalizedTextEncodingByte(localizedText);
    }
    if(status == STATUS_OK && (encodingByte & LocalizedText_Locale) != 0){
        status = String_Write(msgBuffer, &localizedText->locale);
    }
    if(status == STATUS_OK && (encodingByte & LocalizedText_Text) != 0){
        status = String_Write(msgBuffer, &localizedText->text);
    }
    return status;
}

StatusCode LocalizedText_Read(UA_MsgBuffer* msgBuffer, UA_LocalizedText* localizedText){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_Byte encodingByte = 0;
    if(localizedText != UA_NULL){
        status = Byte_Read(msgBuffer, &encodingByte);
    }
    if(status == STATUS_OK && (encodingByte & LocalizedText_Locale) != 0){
        status = String_Read(msgBuffer, &localizedText->locale);
    }
    if(status == STATUS_OK && (encodingByte & LocalizedText_Text) != 0){
        status = String_Read(msgBuffer, &localizedText->text);
    }
    return status;
}

StatusCode ExtensionObject_Write(UA_MsgBuffer* msgBuffer, const UA_ExtensionObject* extObj){
    const int32_t tmpLength = -1;
    UA_NodeId objNodeId = extObj->typeId;
    uint32_t lengthPos;
    uint32_t curPos;
    int32_t length;
    uint16_t nsIndex = OPCUA_NAMESPACE_INDEX;
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_Byte encodingByte = 0;
    if(extObj != UA_NULL){
        encodingByte = extObj->encoding;
    }

    if(encodingByte == UA_ExtObjBodyEncoding_Object){
        encodingByte = UA_ExtObjBodyEncoding_ByteString;
        if(extObj->body.object.objType == UA_NULL){
            status = STATUS_INVALID_PARAMETERS;
        }else{
            if(strncmp(extObj->body.object.objType->namespace,
                       OPCUA_NAMESPACE_NAME,
                       strlen(OPCUA_NAMESPACE_NAME))
               !=  0)
            {
                status = Namespace_GetIndex(msgBuffer->nsTable, extObj->body.object.objType->namespace, &nsIndex);
            }

            objNodeId.identifierType = IdentifierType_Numeric;
            objNodeId.namespace = nsIndex;
            objNodeId.numeric = extObj->body.object.objType->binaryTypeId;
        }
    }

    if(status == STATUS_OK){
        status = Byte_Write(msgBuffer, &encodingByte);
    }

    if(status == STATUS_OK){
        status = NodeId_Write(msgBuffer, &objNodeId);
    }

    if(status == STATUS_OK){
        switch(extObj->encoding){
            case UA_ExtObjBodyEncoding_None:
                break;
            case UA_ExtObjBodyEncoding_ByteString:
                status = ByteString_Write(msgBuffer, &extObj->body.bstring);
                break;
            case UA_ExtObjBodyEncoding_XMLElement:
                status = XmlElement_Write(msgBuffer, &extObj->body.xml);
                break;
            case UA_ExtObjBodyEncoding_Object:
                lengthPos = msgBuffer->buffers->position;
                status = Int32_Write(msgBuffer, &tmpLength);
                if(status == STATUS_OK){
                    status = extObj->body.object.objType->encodeFunction(msgBuffer, extObj->body.object.value);
                }
                if(status == STATUS_OK){
                    // Go backward to write correct length value
                    curPos = msgBuffer->buffers->position;
                    length = curPos - (lengthPos + 4);
                    Buffer_SetPosition(msgBuffer->buffers, lengthPos);
                    Int32_Write(msgBuffer, &length);
                    Buffer_SetPosition(msgBuffer->buffers, curPos);
                }
                break;
        }
    }

    return status;
}

StatusCode ExtensionObject_Read(UA_MsgBuffer* msgBuffer, UA_ExtensionObject* extObj){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_EncodeableType* encType;
    const char* nsName;
    UA_Byte encodingByte = 0;
    if(extObj != UA_NULL){
        status = NodeId_Read(msgBuffer, &extObj->typeId);
    }
    if(status == STATUS_OK){
        status = Byte_Read(msgBuffer, &encodingByte);
    }

    if(status == STATUS_OK &&
       encodingByte == UA_ExtObjBodyEncoding_ByteString){
        if(extObj->typeId.identifierType == IdentifierType_Numeric){
            if(extObj->typeId.namespace != OPCUA_NAMESPACE_INDEX){
                nsName = Namespace_GetName(msgBuffer->nsTable, extObj->typeId.namespace);
            }else{
                nsName = OPCUA_NAMESPACE_NAME;
            }
            if(nsName != UA_NULL){
                encType = EncodeableType_GetEncodeableType(msgBuffer->encTypesTable,
                                                           nsName,
                                                           extObj->typeId.numeric);
            }
            if(nsName == UA_NULL || encType == UA_NULL){
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
                status = ByteString_Read(msgBuffer, &extObj->body.bstring);
                break;
            case UA_ExtObjBodyEncoding_XMLElement:
                status = XmlElement_Read(msgBuffer, &extObj->body.xml);
                break;
            case UA_ExtObjBodyEncoding_Object:
                status = Int32_Read(msgBuffer, &extObj->length);
                if(status == STATUS_OK){
                    extObj->body.object.value = malloc(extObj->body.object.objType->allocSize);
                    status = extObj->body.object.objType->decodeFunction(msgBuffer, &extObj->body.object.value);
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
    assert(variant != UA_NULL);
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
            status = Boolean_Write(msgBuffer, &val->boolean);
            break;
        case UA_SByte_Id:
            status = SByte_Write(msgBuffer, &val->sbyte);
            break;
        case UA_Byte_Id:
            status = Byte_Write(msgBuffer, &val->byte);
            break;
        case UA_Int16_Id:
            status = Int16_Write(msgBuffer, &val->int16);
            break;
        case UA_UInt16_Id:
            status = UInt16_Write(msgBuffer, &val->uint16);
            break;
        case UA_Int32_Id:
            status = Int32_Write(msgBuffer, &val->int32);
            break;
        case UA_UInt32_Id:
            status = UInt32_Write(msgBuffer, &val->uint32);
            break;
        case UA_Int64_Id:
            status = Int64_Write(msgBuffer, &val->int64);
            break;
        case UA_UInt64_Id:
            status = UInt64_Write(msgBuffer, &val->uint64);
            break;
        case UA_Float_Id:
            status = Float_Write(msgBuffer, &val->floatv);
            break;
        case UA_Double_Id:
            status = Double_Write(msgBuffer, &val->doublev);
            break;
        case UA_String_Id:
            status = String_Write(msgBuffer, &val->string);
            break;
        case UA_DateTime_Id:
            status = DateTime_Write(msgBuffer, &val->date);
            break;
        case UA_Guid_Id:
            status = Guid_Write(msgBuffer, val->guid);
            break;
        case UA_ByteString_Id:
            status = ByteString_Write(msgBuffer, &val->bstring);
            break;
        case UA_XmlElement_Id:
            status = XmlElement_Write(msgBuffer, &val->xmlElt);
            break;
        case UA_NodeId_Id:
            status = NodeId_Write(msgBuffer, val->nodeId);
            break;
        case UA_ExpandedNodeId_Id:
            status = ExpandedNodeId_Write(msgBuffer, val->expNodeId);
            break;
        case UA_StatusCode_Id:
            status = StatusCode_Write(msgBuffer, &val->status);
            break;
        case UA_QualifiedName_Id:
            status = QualifiedName_Write(msgBuffer, val->qname);
            break;
        case UA_LocalizedText_Id:
            status = LocalizedText_Write(msgBuffer, val->localizedText);
            break;
        case UA_ExtensionObject_Id:
            status = ExtensionObject_Write(msgBuffer, val->extObject);
            break;
        case UA_DataValue_Id:
            status = DataValue_Write(msgBuffer, val->dataValue);
            break;
        case UA_Variant_Id:
            assert(UA_FALSE);
            break;
        case UA_DiagnosticInfo_Id:
            status = DiagnosticInfo_Write(msgBuffer, val->diagInfo);
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
                status |= Boolean_Write(msgBuffer, &array->booleanArr[idx]);
            }
            break;
        case UA_SByte_Id:
            for(idx = 0; idx < length; idx++){
                status |= SByte_Write(msgBuffer, &array->sbyteArr[idx]);
            }
            break;
        case UA_Byte_Id:
            for(idx = 0; idx < length; idx++){
                status |= Byte_Write(msgBuffer, &array->byteArr[idx]);
            }
            break;
        case UA_Int16_Id:
            for(idx = 0; idx < length; idx++){
                status |= Int16_Write(msgBuffer, &array->int16Arr[idx]);
            }
            break;
        case UA_UInt16_Id:
            for(idx = 0; idx < length; idx++){
                status |= UInt16_Write(msgBuffer, &array->uint16Arr[idx]);
            }
            break;
        case UA_Int32_Id:
            for(idx = 0; idx < length; idx++){
                status |= Int32_Write(msgBuffer, &array->int32Arr[idx]);
            }
            break;
        case UA_UInt32_Id:
            for(idx = 0; idx < length; idx++){
                status |= UInt32_Write(msgBuffer, &array->uint32Arr[idx]);
            }
            break;
        case UA_Int64_Id:
            for(idx = 0; idx < length; idx++){
                status |= Int64_Write(msgBuffer, &array->int64Arr[idx]);
            }
            break;
        case UA_UInt64_Id:
            for(idx = 0; idx < length; idx++){
                status |= UInt64_Write(msgBuffer, &array->uint64Arr[idx]);
            }
            break;
        case UA_Float_Id:
            for(idx = 0; idx < length; idx++){
                status |= Float_Write(msgBuffer, &array->floatvArr[idx]);
            }
            break;
        case UA_Double_Id:
            for(idx = 0; idx < length; idx++){
                status |= Double_Write(msgBuffer, &array->doublevArr[idx]);
            }
            break;
        case UA_String_Id:
            for(idx = 0; idx < length; idx++){
                status |= String_Write(msgBuffer, &array->stringArr[idx]);
            }
            break;
        case UA_DateTime_Id:
            for(idx = 0; idx < length; idx++){
                status |= DateTime_Write(msgBuffer, &array->dateArr[idx]);
            }
            break;
        case UA_Guid_Id:
            for(idx = 0; idx < length; idx++){
                status |= Guid_Write(msgBuffer, &array->guidArr[idx]);
            }
            break;
        case UA_ByteString_Id:
            for(idx = 0; idx < length; idx++){
                status |= ByteString_Write(msgBuffer, &array->bstringArr[idx]);
            }
            break;
        case UA_XmlElement_Id:
            for(idx = 0; idx < length; idx++){
                status |= XmlElement_Write(msgBuffer, &array->xmlEltArr[idx]);
            }
            break;
        case UA_NodeId_Id:
            for(idx = 0; idx < length; idx++){
                status |= NodeId_Write(msgBuffer, &array->nodeIdArr[idx]);
            }
            break;
        case UA_ExpandedNodeId_Id:
            for(idx = 0; idx < length; idx++){
                status |= ExpandedNodeId_Write(msgBuffer, &array->expNodeIdArr[idx]);
            }
            break;
        case UA_StatusCode_Id:
            for(idx = 0; idx < length; idx++){
                status |= StatusCode_Write(msgBuffer, &array->statusArr[idx]);
            }
            break;
        case UA_QualifiedName_Id:
            for(idx = 0; idx < length; idx++){
                status |= QualifiedName_Write(msgBuffer, &array->qnameArr[idx]);
            }
            break;
        case UA_LocalizedText_Id:
            for(idx = 0; idx < length; idx++){
                status |= LocalizedText_Write(msgBuffer, &array->localizedTextArr[idx]);
            }
            break;
        case UA_ExtensionObject_Id:
            for(idx = 0; idx < length; idx++){
                status |= ExtensionObject_Write(msgBuffer, &array->extObjectArr[idx]);
            }
            break;
        case UA_DataValue_Id:
            for(idx = 0; idx < length; idx++){
                status |= DataValue_Write(msgBuffer, &array->dataValueArr[idx]);
            }
            break;
        case UA_Variant_Id:
            for(idx = 0; idx < length; idx++){
                status |= Variant_Write(msgBuffer, &array->variantArr[idx]);
            }
            break;
        case UA_DiagnosticInfo_Id:
            for(idx = 0; idx < length; idx++){
                status |= DiagnosticInfo_Write(msgBuffer, &array->diagInfoArr[idx]);
            }
            break;
        default:
            break;
    }
    return status;
}

StatusCode Variant_Write(UA_MsgBuffer* msgBuffer, const UA_Variant* variant){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_Byte encodingByte = 0;
    int32_t arrayLength = 0;
    if(variant != UA_NULL){
        encodingByte = GetVariantEncodingMask(variant);
        status = Byte_Write(msgBuffer, &encodingByte);
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
            status = Int32_Write(msgBuffer, &arrayLength);
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
                    status = Int32_Write(msgBuffer, &variant->value.matrix.dimensions);
                }
                if(status == STATUS_OK){
                    // array
                    int32_t idx = 0;
                    for(idx = 0; idx < variant->value.matrix.dimensions; idx ++){
                        status = Int32_Write(msgBuffer, &variant->value.matrix.arrayDimensions[idx]);
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
            status = Boolean_Read(msgBuffer, &val->boolean);
            break;
        case UA_SByte_Id:
            status = SByte_Read(msgBuffer, &val->sbyte);
            break;
        case UA_Byte_Id:
            status = Byte_Read(msgBuffer, &val->byte);
            break;
        case UA_Int16_Id:
            status = Int16_Read(msgBuffer, &val->int16);
            break;
        case UA_UInt16_Id:
            status = UInt16_Read(msgBuffer, &val->uint16);
            break;
        case UA_Int32_Id:
            status = Int32_Read(msgBuffer, &val->int32);
            break;
        case UA_UInt32_Id:
            status = UInt32_Read(msgBuffer, &val->uint32);
            break;
        case UA_Int64_Id:
            status = Int64_Read(msgBuffer, &val->int64);
            break;
        case UA_UInt64_Id:
            status = UInt64_Read(msgBuffer, &val->uint64);
            break;
        case UA_Float_Id:
            status = Float_Read(msgBuffer, &val->floatv);
            break;
        case UA_Double_Id:
            status = Double_Read(msgBuffer, &val->doublev);
            break;
        case UA_String_Id:
            status = String_Read(msgBuffer, &val->string);
            break;
        case UA_DateTime_Id:
            status = DateTime_Read(msgBuffer, &val->date);
            break;
        case UA_Guid_Id:
            val->guid = malloc(sizeof(UA_Guid));
            if(val->guid != UA_NULL){
                status = Guid_Read(msgBuffer, val->guid);
            }else{
                status = STATUS_NOK;
            }
            break;
        case UA_ByteString_Id:
            status = ByteString_Read(msgBuffer, &val->bstring);
            break;
        case UA_XmlElement_Id:
            status = XmlElement_Read(msgBuffer, &val->xmlElt);
            break;
        case UA_NodeId_Id:
            val->nodeId = malloc(sizeof(UA_NodeId));
            if(val->nodeId != UA_NULL){
                status = NodeId_Read(msgBuffer, val->nodeId);
            }else{
                status = STATUS_NOK;
            }
            break;
        case UA_ExpandedNodeId_Id:
            val->expNodeId = malloc(sizeof(UA_ExpandedNodeId));
            if(val->expNodeId != UA_NULL){
                status = ExpandedNodeId_Read(msgBuffer, val->expNodeId);
            }else{
                status = STATUS_NOK;
            }
            break;
        case UA_StatusCode_Id:
            status = StatusCode_Read(msgBuffer, &val->status);
            break;
        case UA_QualifiedName_Id:
            val->qname = malloc(sizeof(UA_QualifiedName));
            if(val->qname != UA_NULL){
                status = QualifiedName_Read(msgBuffer, val->qname);
            }else{
                status = STATUS_NOK;
            }
            break;
        case UA_LocalizedText_Id:
            val->localizedText = malloc(sizeof(UA_LocalizedText));
            if(val->localizedText != UA_NULL){
                status = LocalizedText_Read(msgBuffer, val->localizedText);
            }else{
                status = STATUS_NOK;
            }
            break;
        case UA_ExtensionObject_Id:
            val->extObject = malloc(sizeof(UA_ExtensionObject));
            if(val->extObject != UA_NULL){
                status = ExtensionObject_Read(msgBuffer, val->extObject);
            }else{
                status = STATUS_NOK;
            }
            break;
        case UA_DataValue_Id:
            val->dataValue = malloc(sizeof(UA_DataValue));
            if(val->dataValue != UA_NULL){
                status = DataValue_Read(msgBuffer, val->dataValue);
            }else{
                status = STATUS_NOK;
            }
            break;
        case UA_Variant_Id:
            assert(UA_FALSE);
            break;
        case UA_DiagnosticInfo_Id:
            val->diagInfo = malloc(sizeof(UA_DiagnosticInfo));
            if(val->diagInfo != UA_NULL){
                status = DiagnosticInfo_Read(msgBuffer, val->diagInfo);
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
            if(array->booleanArr == UA_NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= Boolean_Read(msgBuffer, &array->booleanArr[idx]);
                }
            }
            break;
        case UA_SByte_Id:
            array->sbyteArr = malloc(sizeof(UA_SByte) * length);
            if(array->sbyteArr == UA_NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= SByte_Read(msgBuffer, &array->sbyteArr[idx]);
                }
            }
            break;
        case UA_Byte_Id:
            array->byteArr = malloc(sizeof(UA_Byte) * length);
            if(array->byteArr == UA_NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= Byte_Read(msgBuffer, &array->byteArr[idx]);
                }
            }
            break;
        case UA_Int16_Id:
            array->int16Arr = malloc(sizeof(int16_t) * length);
            if(array->int16Arr == UA_NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= Int16_Read(msgBuffer, &array->int16Arr[idx]);
                }
            }
            break;
        case UA_UInt16_Id:
            array->sbyteArr = malloc(sizeof(uint16_t) * length);
            if(array->sbyteArr == UA_NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= UInt16_Read(msgBuffer, &array->uint16Arr[idx]);
                }
            }
            break;
        case UA_Int32_Id:
            array->int32Arr = malloc(sizeof(int32_t) * length);
            if(array->int32Arr == UA_NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= Int32_Read(msgBuffer, &array->int32Arr[idx]);
                }
            }
            break;
        case UA_UInt32_Id:
            array->uint32Arr = malloc(sizeof(uint32_t) * length);
            if(array->uint32Arr == UA_NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= UInt32_Read(msgBuffer, &array->uint32Arr[idx]);
                }
            }
            break;
        case UA_Int64_Id:
            array->int64Arr = malloc(sizeof(int64_t) * length);
            if(array->int64Arr == UA_NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= Int64_Read(msgBuffer, &array->int64Arr[idx]);
                }
            }
            break;
        case UA_UInt64_Id:
            array->uint64Arr = malloc(sizeof(uint64_t) * length);
            if(array->uint64Arr == UA_NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= UInt64_Read(msgBuffer, &array->uint64Arr[idx]);
                }
            }
            break;
        case UA_Float_Id:
            array->floatvArr = malloc(sizeof(float) * length);
            if(array->floatvArr == UA_NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= Float_Read(msgBuffer, &array->floatvArr[idx]);
                }
            }
            break;
        case UA_Double_Id:
            array->doublevArr = malloc(sizeof(double) * length);
            if(array->doublevArr == UA_NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= Double_Read(msgBuffer, &array->doublevArr[idx]);
                }
            }
            break;
        case UA_String_Id:
            array->stringArr = malloc(sizeof(UA_String) * length);
            if(array->stringArr == UA_NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= String_Read(msgBuffer, &array->stringArr[idx]);
                }
            }
            break;
        case UA_DateTime_Id:
            array->dateArr = malloc(sizeof(UA_DateTime) * length);
            if(array->dateArr == UA_NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= DateTime_Read(msgBuffer, &array->dateArr[idx]);
                }
            }
            break;
        case UA_Guid_Id:
            array->guidArr = malloc(sizeof(UA_Guid) * length);
            if(array->guidArr == UA_NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= Guid_Read(msgBuffer, &array->guidArr[idx]);
                }
            }
            break;
        case UA_ByteString_Id:
            array->bstringArr = malloc(sizeof(UA_ByteString) * length);
            if(array->bstringArr == UA_NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= ByteString_Read(msgBuffer, &array->bstringArr[idx]);
                }
            }
            break;
        case UA_XmlElement_Id:
            array->xmlEltArr = malloc(sizeof(UA_XmlElement) * length);
            if(array->xmlEltArr == UA_NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= XmlElement_Read(msgBuffer, &array->xmlEltArr[idx]);
                }
            }
            break;
        case UA_NodeId_Id:
            array->nodeIdArr = malloc(sizeof(UA_NodeId) * length);
            if(array->nodeIdArr == UA_NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= NodeId_Read(msgBuffer, &array->nodeIdArr[idx]);
                }
            }
            break;
        case UA_ExpandedNodeId_Id:
            array->expNodeIdArr = malloc(sizeof(UA_ExpandedNodeId) * length);
            if(array->expNodeIdArr == UA_NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= ExpandedNodeId_Read(msgBuffer, &array->expNodeIdArr[idx]);
                }
            }
            break;
        case UA_StatusCode_Id:
            array->statusArr = malloc(sizeof(StatusCode) * length);
            if(array->statusArr == UA_NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= StatusCode_Read(msgBuffer, &array->statusArr[idx]);
                }
            }
            break;
        case UA_QualifiedName_Id:
            array->qnameArr = malloc(sizeof(UA_QualifiedName) * length);
            if(array->qnameArr == UA_NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= QualifiedName_Read(msgBuffer, &array->qnameArr[idx]);
                }
            }
            break;
        case UA_LocalizedText_Id:
            array->localizedTextArr = malloc(sizeof(UA_LocalizedText) * length);
            if(array->localizedTextArr == UA_NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= LocalizedText_Read(msgBuffer, &array->localizedTextArr[idx]);
                }
            }
            break;
        case UA_ExtensionObject_Id:
            array->extObjectArr = malloc(sizeof(UA_ExtensionObject) * length);
            if(array->extObjectArr == UA_NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= ExtensionObject_Read(msgBuffer, &array->extObjectArr[idx]);
                }
            }
            break;
        case UA_DataValue_Id:
            array->dataValueArr = malloc(sizeof(UA_DataValue) * length);
            if(array->dataValueArr == UA_NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= DataValue_Read(msgBuffer, &array->dataValueArr[idx]);
                }
            }
            break;
        case UA_Variant_Id:
            array->variantArr = malloc(sizeof(UA_Variant) * length);
            if(array->variantArr == UA_NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= Variant_Read(msgBuffer, &array->variantArr[idx]);
                }
            }
            break;
        case UA_DiagnosticInfo_Id:
            array->diagInfoArr = malloc(sizeof(UA_DiagnosticInfo) * length);
            if(array->diagInfoArr == UA_NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= DiagnosticInfo_Read(msgBuffer, &array->diagInfoArr[idx]);
                }
            }
            break;
        default:
            break;
    }
    return status;
}

StatusCode Variant_Read(UA_MsgBuffer* msgBuffer, UA_Variant* variant){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_Byte encodingByte = 0;
    int32_t arrayLength = 0;
    if(variant != UA_NULL){
        status = Byte_Read(msgBuffer, &encodingByte);
    }
    if(status == STATUS_OK){
        // Retrieve array flags
        if((encodingByte & UA_VariantArrayValueFlag) != 0){
            variant->arrayTypeMask = UA_VariantArrayValueFlag;
            if((encodingByte & UA_VariantArrayMatrixFlag) != 0){
                variant->arrayTypeMask |= UA_VariantArrayMatrixFlag;
            }
            // Read array length
            status = Int32_Read(msgBuffer, &arrayLength);
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
                    status = Int32_Read(msgBuffer, &variant->value.matrix.dimensions);
                }
                if(status == STATUS_OK){
                    // array
                    variant->value.matrix.arrayDimensions = malloc(sizeof(int32_t) * variant->value.matrix.dimensions);
                    if(variant->value.matrix.arrayDimensions != UA_NULL){
                        int32_t idx = 0;
                        for(idx = 0; idx < variant->value.matrix.dimensions; idx ++){
                            status = Int32_Read(msgBuffer, &variant->value.matrix.arrayDimensions[idx]);
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
    assert(dataValue != UA_NULL);
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

StatusCode DataValue_Write(UA_MsgBuffer* msgBuffer, const UA_DataValue* dataValue)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_Byte encodingMask = 0;
    if(dataValue != UA_NULL){
        encodingMask = GetDataValueEncodingMask(dataValue);
        status = Byte_Write(msgBuffer, &encodingMask);
    }
    if(status == STATUS_OK && (encodingMask & DataValue_NotNullValue) != 0){
        status = Variant_Write(msgBuffer, &dataValue->value);
    }
    if(status == STATUS_OK && (encodingMask & DataValue_NotGoodStatusCode) != 0){
        status = StatusCode_Write(msgBuffer, &dataValue->status);
    }
    if(status == STATUS_OK && (encodingMask & DataValue_NotMinSourceDate) != 0){
        status = DateTime_Write(msgBuffer, &dataValue->sourceTimestamp);
    }
    if(status == STATUS_OK && (encodingMask & DataValue_NotZeroSourcePico) != 0){
        status = UInt16_Write(msgBuffer, &dataValue->sourcePicoSeconds);
    }
    if(status == STATUS_OK && (encodingMask & DataValue_NotMinServerDate) != 0){
            status = DateTime_Write(msgBuffer, &dataValue->serverTimestamp);
    }
    if(status == STATUS_OK && (encodingMask & DataValue_NotZeroServerPico) != 0){
        status = UInt16_Write(msgBuffer, &dataValue->serverPicoSeconds);
    }
    return status;
}

StatusCode DataValue_Read(UA_MsgBuffer* msgBuffer, UA_DataValue* dataValue){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_Byte encodingMask = 0;
    if(dataValue != UA_NULL){
        status = Byte_Read(msgBuffer, &encodingMask);
    }
    if(status == STATUS_OK && (encodingMask & DataValue_NotNullValue) != 0){
        status = Variant_Read(msgBuffer, &dataValue->value);
    }else{
        dataValue->value.builtInTypeMask = UA_Null_Id;
    }
    if(status == STATUS_OK && (encodingMask & DataValue_NotGoodStatusCode) != 0){
        status = StatusCode_Read(msgBuffer, &dataValue->status);
    }else{
        dataValue->status = STATUS_OK;
    }
    if(status == STATUS_OK && (encodingMask & DataValue_NotMinSourceDate) != 0){
        status = DateTime_Read(msgBuffer, &dataValue->sourceTimestamp);
    }else{
        dataValue->sourceTimestamp = 0;
    }
    if(status == STATUS_OK && (encodingMask & DataValue_NotZeroSourcePico) != 0){
        status = UInt16_Read(msgBuffer, &dataValue->sourcePicoSeconds);
    }else{
        dataValue->sourcePicoSeconds = 0;
    }
    if(status == STATUS_OK && (encodingMask & DataValue_NotMinServerDate) != 0){
            status = DateTime_Read(msgBuffer, &dataValue->serverTimestamp);
    }else{
        dataValue->serverTimestamp = 0;
    }
    if(status == STATUS_OK && (encodingMask & DataValue_NotZeroServerPico) != 0){
        status = UInt16_Read(msgBuffer, &dataValue->serverPicoSeconds);
    }else{
        dataValue->serverPicoSeconds = 0;
    }
    return status;
}

