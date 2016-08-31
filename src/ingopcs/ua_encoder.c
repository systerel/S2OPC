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

StatusCode NodeId_Write(UA_MsgBuffer* msgBuffer, const UA_NodeId* nodeId);
StatusCode NodeId_Read(UA_MsgBuffer* msgBuffer, UA_NodeId* nodeId);
StatusCode ExpandedNodeId_Write(UA_MsgBuffer* msgBuffer, const UA_ExpandedNodeId* expNodeId);
StatusCode ExpandedNodeId_Read(UA_MsgBuffer* msgBuffer, UA_ExpandedNodeId* expNodeId);
StatusCode Guid_Write(UA_MsgBuffer* msgBuffer, const UA_Guid* guid);
StatusCode Guid_Read(UA_MsgBuffer* msgBuffer, UA_Guid* guid);
StatusCode StatusCode_Write(UA_MsgBuffer* msgBuffer, const StatusCode* status);
StatusCode StatusCode_Read(UA_MsgBuffer* msgBuffer, StatusCode* status);
StatusCode DiagnosticInfo_Write(UA_MsgBuffer* msgBuffer, const UA_DiagnosticInfo* diagInfo);
StatusCode DiagnosticInfo_Read(UA_MsgBuffer* msgBuffer, UA_DiagnosticInfo* diagInfo);
