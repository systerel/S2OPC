/*
 * ua_encoder.c
 *
 *  Created on: Aug 3, 2016
 *      Author: vincent
 */

#include <stdlib.h>
#include <assert.h>
#include <ua_encoder.h>
#include <ua_tcp_ua_low_level.h>

uint16_t EncodeDecode_UInt16(uint16_t from)
{
    assert(endianess != Endianess_Undefined);
    if(endianess == Endianess_BigEndian){
        return SWAP_2_BYTES(from);
    }else{
        return from;
    }
}

uint32_t EncodeDecode_UInt32(uint32_t from)
{
    assert(endianess != Endianess_Undefined);
    if(endianess == Endianess_BigEndian){
        return SWAP_4_BYTES(from);
    }else{
        return from;
    }
}

uint32_t EncodeDecode_Int32(int32_t from)
{
    assert(endianess != Endianess_Undefined);
    if(endianess == Endianess_BigEndian){
        return SWAP_4_BYTES(from);
    }else{
        return from;
    }
}

uint64_t EncodeDecode_Int64(int64_t from)
{
    assert(endianess != Endianess_Undefined);
    if(endianess == Endianess_BigEndian){
        return SWAP_8_BYTES(from);
    }else{
        return from;
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

StatusCode UInt16_Write(UA_MsgBuffer* msgBuffer, const uint16_t* value)
{
    StatusCode status = STATUS_NOK;
    uint16_t encodedValue = EncodeDecode_UInt16(*value);
    status = TCP_UA_WriteMsgBuffer(msgBuffer, (UA_Byte*) &encodedValue, 2);
    return status;
}

StatusCode UInt16_Read(UA_MsgBuffer* msgBuffer, uint16_t* value)
{
    StatusCode status = STATUS_NOK;
    uint16_t readValue;
    if(value == UA_NULL){
        status = STATUS_INVALID_PARAMETERS;
    }else{
        status = TCP_UA_ReadMsgBuffer((UA_Byte*)&readValue, 2, msgBuffer, 2);
        if(status == STATUS_OK){
            *value = EncodeDecode_UInt16(readValue);
        }
    }
    return status;
}

StatusCode UInt32_Write(UA_MsgBuffer* msgBuffer, const uint32_t* value)
{
    StatusCode status = STATUS_NOK;
    uint32_t encodedValue = EncodeDecode_UInt32(*value);
    status = TCP_UA_WriteMsgBuffer(msgBuffer, (UA_Byte*) &encodedValue, 4);
    return status;
}

StatusCode UInt32_Read(UA_MsgBuffer* msgBuffer, uint32_t* value)
{
    StatusCode status = STATUS_NOK;
    uint32_t readValue;
    if(value == UA_NULL){
        status = STATUS_INVALID_PARAMETERS;
    }else{
        status = TCP_UA_ReadMsgBuffer((UA_Byte*)&readValue, 4, msgBuffer, 4);
        if(status == STATUS_OK){
            *value = EncodeDecode_UInt32(readValue);
        }
    }
    return status;
}

StatusCode Int32_Write(UA_MsgBuffer* msgBuffer, const int32_t* value)
{
    StatusCode status = STATUS_NOK;
    int32_t encodedValue;
    if(value == UA_NULL){
            status = STATUS_INVALID_PARAMETERS;
    }else{
        encodedValue = EncodeDecode_Int32(*value);
        status = TCP_UA_WriteMsgBuffer(msgBuffer, (UA_Byte*) &encodedValue, 4);
    }
    return status;
}

StatusCode Int32_Read(UA_MsgBuffer* msgBuffer, int32_t* value)
{
    StatusCode status = STATUS_NOK;
    int32_t readValue;
    if(value == UA_NULL){
        status = STATUS_INVALID_PARAMETERS;
    }else{
        status = TCP_UA_ReadMsgBuffer((UA_Byte*)&readValue, 4, msgBuffer, 4);
        if(status == STATUS_OK){
            *value = EncodeDecode_Int32(readValue);
        }
    }
    return status;
}

StatusCode Int64_Write(UA_MsgBuffer* msgBuffer, const int64_t* value)
{
    StatusCode status = STATUS_NOK;
    int64_t encodedValue;
    if(value == UA_NULL){
            status = STATUS_INVALID_PARAMETERS;
    }else{
        encodedValue = EncodeDecode_Int64(*value);
        status = TCP_UA_WriteMsgBuffer(msgBuffer, (UA_Byte*) &encodedValue, 8);
    }
    return status;
}

StatusCode Int64_Read(UA_MsgBuffer* msgBuffer, int64_t* value)
{
    StatusCode status = STATUS_NOK;
    int64_t readValue;
    if(value == UA_NULL){
        status = STATUS_INVALID_PARAMETERS;
    }else{
        status = TCP_UA_ReadMsgBuffer((UA_Byte*)&readValue, 8, msgBuffer, 8);
        if(status == STATUS_OK){
            *value = EncodeDecode_Int64(readValue);
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
    int32_t readValue, length;
    if(str == UA_NULL || (str != UA_NULL && str->characters != UA_NULL)){
        status = STATUS_INVALID_PARAMETERS;
    }else{
        status = TCP_UA_ReadMsgBuffer((UA_Byte*)&readValue, 4, msgBuffer, 4);
        if(status == STATUS_OK){
            length = EncodeDecode_Int32(readValue);
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
                nodeId->namespace = 0;
                status = Byte_Read(msgBuffer, &byte);
                nodeId->numeric = (uint32_t) byte;
                break;
            case  NodeIdEncoding_FourByte:
                status = Byte_Read(msgBuffer, &byte);
                nodeId->namespace = byte;
                if(status == STATUS_OK){
                    status = UInt16_Read(msgBuffer, &twoBytes);
                    nodeId->numeric = twoBytes;
                }
                break;
            case  NodeIdEncoding_Numeric:
                status = UInt16_Read(msgBuffer, &nodeId->namespace);
                if(status == STATUS_OK){
                    status = UInt32_Read(msgBuffer, &nodeId->numeric);
                }
                break;
            case  NodeIdEncoding_String:
                status = UInt16_Read(msgBuffer, &nodeId->namespace);
                if(status == STATUS_OK){
                    status = String_Read(msgBuffer, &nodeId->string);
                }
                break;
            case  NodeIdEncoding_Guid:
                status = UInt16_Read(msgBuffer, &nodeId->namespace);
                if(status == STATUS_OK){
                    status = Guid_Read(msgBuffer, &nodeId->guid);
                }
                break;
            case  NodeIdEncoding_ByteString:
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
