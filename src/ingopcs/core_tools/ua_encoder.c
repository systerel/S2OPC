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
        *eightBytes = SWAP_8_BYTES(*eightBytes);
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

    if(*value == FALSE){
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
            if(*value != FALSE){
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
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(value != NULL){
        int16_t encodedValue = *value;
        EncodeDecode_Int16(&encodedValue);
        status = TCP_UA_WriteMsgBuffer(msgBuffer, (UA_Byte*) &encodedValue, 2);
    }
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
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(value != NULL){
        uint16_t encodedValue = *value;
        EncodeDecode_UInt16(&encodedValue);
        status = TCP_UA_WriteMsgBuffer(msgBuffer, (UA_Byte*) &encodedValue, 2);
    }
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
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(value != NULL){
        int32_t encodedValue = *value;
        EncodeDecode_Int32(&encodedValue);
        status = TCP_UA_WriteMsgBuffer(msgBuffer, (UA_Byte*) &encodedValue, 4);
    }
    return status;
}

StatusCode Int32_Read(int32_t* value, UA_MsgBuffer* msgBuffer)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(value != NULL){
        status = TCP_UA_ReadMsgBuffer((UA_Byte*)value, 4, msgBuffer, 4);
        if(status == STATUS_OK){
            EncodeDecode_Int32(value);
        }
    }
    return status;
}

StatusCode UInt32_Write(const uint32_t* value, UA_MsgBuffer* msgBuffer)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(value != NULL){
        uint32_t encodedValue = *value;
        EncodeDecode_UInt32(&encodedValue);
        status = TCP_UA_WriteMsgBuffer(msgBuffer, (UA_Byte*) &encodedValue, 4);
    }
    return status;
}

StatusCode UInt32_Read(uint32_t* value, UA_MsgBuffer* msgBuffer)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(value != NULL){
        status = TCP_UA_ReadMsgBuffer((UA_Byte*)value, 4, msgBuffer, 4);
        if(status == STATUS_OK){
            EncodeDecode_UInt32(value);
        }
    }
    return status;
}

StatusCode Int64_Write(const int64_t* value, UA_MsgBuffer* msgBuffer)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(value != NULL){
        int64_t encodedValue = *value;
        EncodeDecode_Int64(&encodedValue);
        status = TCP_UA_WriteMsgBuffer(msgBuffer, (UA_Byte*) &encodedValue, 8);
    }
    return status;
}

StatusCode Int64_Read(int64_t* value, UA_MsgBuffer* msgBuffer)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(value != NULL){
        status = TCP_UA_ReadMsgBuffer((UA_Byte*) value, 8, msgBuffer, 8);
        if(status == STATUS_OK){
            EncodeDecode_Int64(value);
        }
    }
    return status;
}

StatusCode UInt64_Write(const uint64_t* value, UA_MsgBuffer* msgBuffer)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(value != NULL){
        uint64_t encodedValue = *value;
        EncodeDecode_UInt64(&encodedValue);
        status = TCP_UA_WriteMsgBuffer(msgBuffer, (UA_Byte*) &encodedValue, 8);
    }
    return status;
}

StatusCode UInt64_Read(uint64_t* value, UA_MsgBuffer* msgBuffer)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(value != NULL){
        status = TCP_UA_ReadMsgBuffer((UA_Byte*) value, 8, msgBuffer, 8);
        if(status == STATUS_OK){
            EncodeDecode_UInt64(value);
        }
    }
    return status;
}

StatusCode Float_Write(const float* value, UA_MsgBuffer* msgBuffer)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(value != NULL){
        float encodedValue = *value;
        EncodeDecode_Float(&encodedValue);
        status = TCP_UA_WriteMsgBuffer(msgBuffer, (UA_Byte*) &encodedValue, 4);
    }
    return status;
}

StatusCode Float_Read(float* value, UA_MsgBuffer* msgBuffer)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(value != NULL){
        status = TCP_UA_ReadMsgBuffer((UA_Byte*)value, 4, msgBuffer, 4);
        if(status == STATUS_OK){
            EncodeDecode_Float(value);
        }
    }
    return status;
}

StatusCode Double_Write(const double* value, UA_MsgBuffer* msgBuffer)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(value != NULL){
        double encodedValue = *value;
        EncodeDecode_Double(&encodedValue);
        status = TCP_UA_WriteMsgBuffer(msgBuffer, (UA_Byte*) &encodedValue, 8);
    }
    return status;
}

StatusCode Double_Read(double* value, UA_MsgBuffer* msgBuffer)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(value != NULL){
        status = TCP_UA_ReadMsgBuffer((UA_Byte*)value, 8, msgBuffer, 8);
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
        if(str->Length > 0){
            length = str->Length;
        }else{
            length = -1;
        }
        status = Int32_Write(&length, msgBuffer);
        if(status == STATUS_OK &&
           str->Length > 0)
        {
            status = TCP_UA_WriteMsgBuffer(msgBuffer, str->Data, str->Length);
        }
    }
    return status;
}

StatusCode ByteString_Read(UA_ByteString* str, UA_MsgBuffer* msgBuffer)
{
    StatusCode status = STATUS_NOK;
    int32_t length;
    if(str == NULL || (str != NULL && str->Data != NULL)){
        status = STATUS_INVALID_PARAMETERS;
    }else{
        status = TCP_UA_ReadMsgBuffer((UA_Byte*)&length, 4, msgBuffer, 4);
        if(status == STATUS_OK){
            EncodeDecode_Int32(&length);
            if(length > 0){
                str->Length = length;
                str->Data = malloc(sizeof(UA_Byte) * length);
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

StatusCode String_Write(const UA_String* str, UA_MsgBuffer* msgBuffer)
{
    return ByteString_Write((UA_ByteString*) str, msgBuffer);
}

StatusCode String_Read(UA_String* str, UA_MsgBuffer* msgBuffer)
{
    StatusCode status = STATUS_NOK;
    int32_t length;
    if(str == NULL || (str != NULL && str->Data != NULL)){
        status = STATUS_INVALID_PARAMETERS;
    }else{
        status = TCP_UA_ReadMsgBuffer((UA_Byte*)&length, 4, msgBuffer, 4);
        if(status == STATUS_OK){
            EncodeDecode_Int32(&length);
            if(length > 0){
                str->Length = length;
                // +1 to add '\0' character for CString compatibility
                str->Data = malloc(sizeof(UA_Byte) * (length + 1));
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
        status = UInt32_Write(&guid->Data1, msgBuffer);
    }
    if(status == STATUS_OK){
        status = UInt16_Write(&guid->Data2, msgBuffer);
    }
    if(status == STATUS_OK){
        status = UInt16_Write(&guid->Data3, msgBuffer);
    }
    if(status == STATUS_OK){
        status = TCP_UA_WriteMsgBuffer(msgBuffer, &(guid->Data4[0]), 8);
    }
    return status;
}

StatusCode Guid_Read(UA_Guid* guid, UA_MsgBuffer* msgBuffer)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(guid != NULL){
        status = UInt32_Read(&guid->Data1, msgBuffer);
    }
    if(status == STATUS_OK){
        status = UInt16_Read(&guid->Data2, msgBuffer);
    }
    if(status == STATUS_OK){
        status = UInt16_Read(&guid->Data3, msgBuffer);
    }
    if(status == STATUS_OK){
        status = TCP_UA_ReadMsgBuffer(&(guid->Data4[0]), 8, msgBuffer, 8);
    }
    return status;
}

UA_NodeId_DataEncoding GetNodeIdDataEncoding(const UA_NodeId* nodeId){
    UA_NodeId_DataEncoding encodingEnum = NodeIdEncoding_Invalid;
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
                byte = (UA_Byte) nodeId->Data.Numeric;
                status = Byte_Write(&byte, msgBuffer);
                break;
            case  NodeIdEncoding_FourByte:
                twoBytes = (uint16_t) nodeId->Data.Numeric;
                if(nodeId->Namespace <= UINT8_MAX){
                    const UA_Byte namespace = nodeId->Namespace;
                    status = Byte_Write(&namespace, msgBuffer);
                }else{
                    status = STATUS_INVALID_PARAMETERS;
                }
                if(status == STATUS_OK){
                    status = UInt16_Write(&twoBytes, msgBuffer);
                }
                break;
            case  NodeIdEncoding_Numeric:
                status = UInt16_Write(&nodeId->Namespace, msgBuffer);
                if(status == STATUS_OK){
                    status = UInt32_Write(&nodeId->Data.Numeric, msgBuffer);
                }
                break;
            case  NodeIdEncoding_String:
                status = UInt16_Write(&nodeId->Namespace, msgBuffer);
                if(status == STATUS_OK){
                    status = String_Write(&nodeId->Data.String, msgBuffer);
                }
                break;
            case  NodeIdEncoding_Guid:
                status = UInt16_Write(&nodeId->Namespace, msgBuffer);
                if(status == STATUS_OK){
                    status = Guid_Write(&nodeId->Data.Guid, msgBuffer);
                }
                break;
            case  NodeIdEncoding_ByteString:
                status = UInt16_Write(&nodeId->Namespace, msgBuffer);
                if(status == STATUS_OK){
                    status = ByteString_Write(&nodeId->Data.Bstring, msgBuffer);
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
                nodeId->IdentifierType = IdentifierType_Numeric;
                nodeId->Namespace = 0;
                status = Byte_Read(&byte, msgBuffer);
                nodeId->Data.Numeric = (uint32_t) byte;
                break;
            case  NodeIdEncoding_FourByte:
                nodeId->IdentifierType = IdentifierType_Numeric;
                status = Byte_Read(&byte, msgBuffer);
                nodeId->Namespace = byte;
                if(status == STATUS_OK){
                    status = UInt16_Read(&twoBytes, msgBuffer);
                    nodeId->Data.Numeric = twoBytes;
                }
                break;
            case  NodeIdEncoding_Numeric:
                nodeId->IdentifierType = IdentifierType_Numeric;
                status = UInt16_Read(&nodeId->Namespace, msgBuffer);
                if(status == STATUS_OK){
                    status = UInt32_Read(&nodeId->Data.Numeric, msgBuffer);
                }
                break;
            case  NodeIdEncoding_String:
                nodeId->IdentifierType = IdentifierType_String;
                status = UInt16_Read(&nodeId->Namespace, msgBuffer);
                if(status == STATUS_OK){
                    status = String_Read(&nodeId->Data.String, msgBuffer);
                }
                break;
            case  NodeIdEncoding_Guid:
                nodeId->IdentifierType = IdentifierType_Guid;
                status = UInt16_Read(&nodeId->Namespace, msgBuffer);
                if(status == STATUS_OK){
                    status = Guid_Read(&nodeId->Data.Guid, msgBuffer);
                }
                break;
            case  NodeIdEncoding_ByteString:
                nodeId->IdentifierType = IdentifierType_ByteString;
                status = UInt16_Read(&nodeId->Namespace, msgBuffer);
                if(status == STATUS_OK){
                    status = ByteString_Read(&nodeId->Data.Bstring, msgBuffer);
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
        status = String_Write(&expNodeId->NamespaceUri, msgBuffer);
    }
    if(status == STATUS_OK && expNodeId->ServerIndex > 0)
    {
        status = UInt32_Write(&expNodeId->ServerIndex, msgBuffer);
    }

    return status;
}

StatusCode ExpandedNodeId_Read(UA_ExpandedNodeId* expNodeId, UA_MsgBuffer* msgBuffer){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_Byte encodingByte = 0;
    if(expNodeId != NULL){
        status = Internal_NodeId_Read(msgBuffer, &expNodeId->NodeId, &encodingByte);
    }
    if(status == STATUS_OK &&
       (encodingByte & NodeIdEncoding_NamespaceUriFlag) != 0x00)
    {
        status = String_Read(&expNodeId->NamespaceUri, msgBuffer);
    }else{
        String_Clear(&expNodeId->NamespaceUri);
    }
    if(status == STATUS_OK &&
        (encodingByte & NodeIdEncoding_ServerIndexFlag) != 0)
    {
        status = UInt32_Read(&expNodeId->ServerIndex, msgBuffer);
    }else{
        UInt32_Clear(&expNodeId->ServerIndex);
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

StatusCode DiagnosticInfo_Write(const UA_DiagnosticInfo* diagInfo, UA_MsgBuffer* msgBuffer){
    UA_Byte encodingByte = 0x00;
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(diagInfo != NULL){
        status = STATUS_OK;
        encodingByte = GetDiagInfoEncodingByte(diagInfo);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_SymbolicId) != 0x00){
        status = Int32_Write(&diagInfo->SymbolicId, msgBuffer);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_Namespace) != 0x00){
        status = Int32_Write(&diagInfo->NamespaceUri, msgBuffer);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_Locale) != 0x00){
        status = Int32_Write(&diagInfo->Locale, msgBuffer);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_LocalizedTest) != 0x00){
        status = Int32_Write(&diagInfo->LocalizedText, msgBuffer);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_AdditionalInfo) != 0x00){
        status = String_Write(&diagInfo->AdditionalInfo, msgBuffer);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_InnerStatusCode) != 0x00){
        status = StatusCode_Write(&diagInfo->InnerStatusCode, msgBuffer);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_InnerDianosticInfo) != 0x00){
        status = DiagnosticInfo_Write(diagInfo->InnerDiagnosticInfo, msgBuffer);
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
        status = Int32_Read(&diagInfo->SymbolicId, msgBuffer);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_Namespace) != 0x00){
        status = Int32_Read(&diagInfo->NamespaceUri, msgBuffer);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_Locale) != 0x00){
        status = Int32_Read(&diagInfo->Locale, msgBuffer);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_LocalizedTest) != 0x00){
        status = Int32_Read(&diagInfo->LocalizedText, msgBuffer);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_AdditionalInfo) != 0x00){
        status = String_Read(&diagInfo->AdditionalInfo, msgBuffer);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_InnerStatusCode) != 0x00){
        status = StatusCode_Read(&diagInfo->InnerStatusCode, msgBuffer);
    }
    if(status == STATUS_OK &&
        (encodingByte & DiagInfoEncoding_InnerDianosticInfo) != 0x00){
        status = DiagnosticInfo_Read(diagInfo->InnerDiagnosticInfo, msgBuffer);
    }
    return status;
}

StatusCode QualifiedName_Write(const UA_QualifiedName* qname, UA_MsgBuffer* msgBuffer){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(qname != NULL){
        status = UInt16_Write(&qname->NamespaceIndex, msgBuffer);
    }
    if(status == STATUS_OK){
        status = String_Write(&qname->Name, msgBuffer);
    }
    return status;
}

StatusCode QualifiedName_Read(UA_QualifiedName* qname, UA_MsgBuffer* msgBuffer){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(qname != NULL){
        status = UInt16_Read(&qname->NamespaceIndex, msgBuffer);
    }
    if(status == STATUS_OK){
        status = String_Read(&qname->Name, msgBuffer);
    }
    return status;
}


UA_Byte GetLocalizedTextEncodingByte(const UA_LocalizedText* ltext){
    assert(ltext != NULL);
    UA_Byte encodingByte = 0;
    if(ltext->Locale.Length > 0){
        encodingByte |= LocalizedText_Locale;
    }
    if(ltext->Text.Length > 0){
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
        status = String_Write(&localizedText->Locale, msgBuffer);
    }
    if(status == STATUS_OK && (encodingByte & LocalizedText_Text) != 0){
        status = String_Write(&localizedText->Text, msgBuffer);
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
        status = String_Read(&localizedText->Locale, msgBuffer);
    }
    if(status == STATUS_OK && (encodingByte & LocalizedText_Text) != 0){
        status = String_Read(&localizedText->Text, msgBuffer);
    }
    return status;
}

StatusCode ExtensionObject_Write(const UA_ExtensionObject* extObj, UA_MsgBuffer* msgBuffer){
    const int32_t tmpLength = -1;
    UA_NodeId objNodeId = extObj->TypeId;
    uint32_t lengthPos;
    uint32_t curPos;
    int32_t length;
    uint16_t nsIndex = OPCUA_NAMESPACE_INDEX;
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_Byte encodingByte = 0;
    if(extObj != NULL){
        encodingByte = extObj->Encoding;
        status = STATUS_OK;
    }

    if(status == STATUS_OK &&
       encodingByte == UA_ExtObjBodyEncoding_Object)
    {
        encodingByte = UA_ExtObjBodyEncoding_ByteString;
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
        status = NodeId_Write(&objNodeId, msgBuffer);
    }

    if(status == STATUS_OK){
        status = Byte_Write(&encodingByte, msgBuffer);
    }

    if(status == STATUS_OK){
        switch(extObj->Encoding){
            case UA_ExtObjBodyEncoding_None:
                break;
            case UA_ExtObjBodyEncoding_ByteString:
                status = ByteString_Write(&extObj->Body.Bstring, msgBuffer);
                break;
            case UA_ExtObjBodyEncoding_XMLElement:
                status = XmlElement_Write(&extObj->Body.Xml, msgBuffer);
                break;
            case UA_ExtObjBodyEncoding_Object:
                lengthPos = msgBuffer->buffers->position;
                status = Int32_Write(&tmpLength, msgBuffer);
                if(status == STATUS_OK){
                    status = extObj->Body.Object.ObjType->Encode(extObj->Body.Object.Value, msgBuffer);
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
        status = NodeId_Read(&extObj->TypeId, msgBuffer);
    }
    if(status == STATUS_OK){
        status = Byte_Read(&encodingByte, msgBuffer);
    }

    if(status == STATUS_OK &&
       encodingByte == UA_ExtObjBodyEncoding_ByteString){
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
                encodingByte = UA_ExtObjBodyEncoding_Object;
                extObj->Body.Object.ObjType = encType;
            }
        }else{
            status = STATUS_INVALID_RCV_PARAMETER;
        }

    }

    if(status == STATUS_OK){
        switch(encodingByte){
            case UA_ExtObjBodyEncoding_None:
                extObj->Length = -1;
                break;
            case UA_ExtObjBodyEncoding_ByteString:
                status = ByteString_Read(&extObj->Body.Bstring, msgBuffer);
                break;
            case UA_ExtObjBodyEncoding_XMLElement:
                status = XmlElement_Read(&extObj->Body.Xml, msgBuffer);
                break;
            case UA_ExtObjBodyEncoding_Object:
                status = Int32_Read(&extObj->Length, msgBuffer);
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

UA_Byte GetVariantEncodingMask(const UA_Variant* variant){
    assert(variant != NULL);
    UA_Byte encodingByte = variant->BuiltInTypeMask;
    if((variant->ArrayTypeMask & UA_VariantArrayMatrixFlag) != 0){
        encodingByte |= UA_VariantArrayMatrixFlag;
    }
    if((variant->ArrayTypeMask & UA_VariantArrayValueFlag) != 0){
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
            status = Boolean_Write(&val->Boolean, msgBuffer);
            break;
        case UA_SByte_Id:
            status = SByte_Write(&val->Sbyte, msgBuffer);
            break;
        case UA_Byte_Id:
            status = Byte_Write(&val->Byte, msgBuffer);
            break;
        case UA_Int16_Id:
            status = Int16_Write(&val->Int16, msgBuffer);
            break;
        case UA_UInt16_Id:
            status = UInt16_Write(&val->Uint16, msgBuffer);
            break;
        case UA_Int32_Id:
            status = Int32_Write(&val->Int32, msgBuffer);
            break;
        case UA_UInt32_Id:
            status = UInt32_Write(&val->Uint32, msgBuffer);
            break;
        case UA_Int64_Id:
            status = Int64_Write(&val->Int64, msgBuffer);
            break;
        case UA_UInt64_Id:
            status = UInt64_Write(&val->Uint64, msgBuffer);
            break;
        case UA_Float_Id:
            status = Float_Write(&val->Floatv, msgBuffer);
            break;
        case UA_Double_Id:
            status = Double_Write(&val->Doublev, msgBuffer);
            break;
        case UA_String_Id:
            status = String_Write(&val->String, msgBuffer);
            break;
        case UA_DateTime_Id:
            status = DateTime_Write(&val->Date, msgBuffer);
            break;
        case UA_Guid_Id:
            status = Guid_Write(val->Guid, msgBuffer);
            break;
        case UA_ByteString_Id:
            status = ByteString_Write(&val->Bstring, msgBuffer);
            break;
        case UA_XmlElement_Id:
            status = XmlElement_Write(&val->XmlElt, msgBuffer);
            break;
        case UA_NodeId_Id:
            status = NodeId_Write(val->NodeId, msgBuffer);
            break;
        case UA_ExpandedNodeId_Id:
            status = ExpandedNodeId_Write(val->ExpNodeId, msgBuffer);
            break;
        case UA_StatusCode_Id:
            status = StatusCode_Write(&val->Status, msgBuffer);
            break;
        case UA_QualifiedName_Id:
            status = QualifiedName_Write(val->Qname, msgBuffer);
            break;
        case UA_LocalizedText_Id:
            status = LocalizedText_Write(val->LocalizedText, msgBuffer);
            break;
        case UA_ExtensionObject_Id:
            status = ExtensionObject_Write(val->ExtObject, msgBuffer);
            break;
        case UA_DataValue_Id:
            status = DataValue_Write(val->DataValue, msgBuffer);
            break;
        case UA_Variant_Id:
            assert(FALSE);
            break;
        case UA_DiagnosticInfo_Id:
            status = DiagnosticInfo_Write(val->DiagInfo, msgBuffer);
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
                status |= Boolean_Write(&array->BooleanArr[idx], msgBuffer);
            }
            break;
        case UA_SByte_Id:
            for(idx = 0; idx < length; idx++){
                status |= SByte_Write(&array->SbyteArr[idx], msgBuffer);
            }
            break;
        case UA_Byte_Id:
            for(idx = 0; idx < length; idx++){
                status |= Byte_Write(&array->ByteArr[idx], msgBuffer);
            }
            break;
        case UA_Int16_Id:
            for(idx = 0; idx < length; idx++){
                status |= Int16_Write(&array->Int16Arr[idx], msgBuffer);
            }
            break;
        case UA_UInt16_Id:
            for(idx = 0; idx < length; idx++){
                status |= UInt16_Write(&array->Uint16Arr[idx], msgBuffer);
            }
            break;
        case UA_Int32_Id:
            for(idx = 0; idx < length; idx++){
                status |= Int32_Write(&array->Int32Arr[idx], msgBuffer);
            }
            break;
        case UA_UInt32_Id:
            for(idx = 0; idx < length; idx++){
                status |= UInt32_Write(&array->Uint32Arr[idx], msgBuffer);
            }
            break;
        case UA_Int64_Id:
            for(idx = 0; idx < length; idx++){
                status |= Int64_Write(&array->Int64Arr[idx], msgBuffer);
            }
            break;
        case UA_UInt64_Id:
            for(idx = 0; idx < length; idx++){
                status |= UInt64_Write(&array->Uint64Arr[idx], msgBuffer);
            }
            break;
        case UA_Float_Id:
            for(idx = 0; idx < length; idx++){
                status |= Float_Write(&array->FloatvArr[idx], msgBuffer);
            }
            break;
        case UA_Double_Id:
            for(idx = 0; idx < length; idx++){
                status |= Double_Write(&array->DoublevArr[idx], msgBuffer);
            }
            break;
        case UA_String_Id:
            for(idx = 0; idx < length; idx++){
                status |= String_Write(&array->StringArr[idx], msgBuffer);
            }
            break;
        case UA_DateTime_Id:
            for(idx = 0; idx < length; idx++){
                status |= DateTime_Write(&array->DateArr[idx], msgBuffer);
            }
            break;
        case UA_Guid_Id:
            for(idx = 0; idx < length; idx++){
                status |= Guid_Write(&array->GuidArr[idx], msgBuffer);
            }
            break;
        case UA_ByteString_Id:
            for(idx = 0; idx < length; idx++){
                status |= ByteString_Write(&array->BstringArr[idx], msgBuffer);
            }
            break;
        case UA_XmlElement_Id:
            for(idx = 0; idx < length; idx++){
                status |= XmlElement_Write(&array->XmlEltArr[idx], msgBuffer);
            }
            break;
        case UA_NodeId_Id:
            for(idx = 0; idx < length; idx++){
                status |= NodeId_Write(&array->NodeIdArr[idx], msgBuffer);
            }
            break;
        case UA_ExpandedNodeId_Id:
            for(idx = 0; idx < length; idx++){
                status |= ExpandedNodeId_Write(&array->ExpNodeIdArr[idx], msgBuffer);
            }
            break;
        case UA_StatusCode_Id:
            for(idx = 0; idx < length; idx++){
                status |= StatusCode_Write(&array->StatusArr[idx], msgBuffer);
            }
            break;
        case UA_QualifiedName_Id:
            for(idx = 0; idx < length; idx++){
                status |= QualifiedName_Write(&array->QnameArr[idx], msgBuffer);
            }
            break;
        case UA_LocalizedText_Id:
            for(idx = 0; idx < length; idx++){
                status |= LocalizedText_Write(&array->LocalizedTextArr[idx], msgBuffer);
            }
            break;
        case UA_ExtensionObject_Id:
            for(idx = 0; idx < length; idx++){
                status |= ExtensionObject_Write(&array->ExtObjectArr[idx], msgBuffer);
            }
            break;
        case UA_DataValue_Id:
            for(idx = 0; idx < length; idx++){
                status |= DataValue_Write(&array->DataValueArr[idx], msgBuffer);
            }
            break;
        case UA_Variant_Id:
            for(idx = 0; idx < length; idx++){
                status |= Variant_Write(&array->VariantArr[idx], msgBuffer);
            }
            break;
        case UA_DiagnosticInfo_Id:
            for(idx = 0; idx < length; idx++){
                status |= DiagnosticInfo_Write(&array->DiagInfoArr[idx], msgBuffer);
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
        if((variant->ArrayTypeMask & UA_VariantArrayValueFlag) != 0){
            if((variant->ArrayTypeMask & UA_VariantArrayMatrixFlag) != 0){
                int32_t idx = 0;
                for(idx = 0; idx < variant->Value.Matrix.Dimensions; idx ++){
                    arrayLength *= variant->Value.Matrix.ArrayDimensions[idx];
                }
            }else{
                arrayLength = variant->Value.Array.Length;
            }
            status = Int32_Write(&arrayLength, msgBuffer);
        }else if((variant->ArrayTypeMask & UA_VariantArrayMatrixFlag) != 0){
            status = STATUS_INVALID_PARAMETERS;
        }
    }
    if(status == STATUS_OK){
        if((variant->ArrayTypeMask & UA_VariantArrayValueFlag) != 0){
            if((variant->ArrayTypeMask & UA_VariantArrayMatrixFlag) != 0){
                status = WriteVariantArrayBuiltInType(msgBuffer,
                                                      variant->BuiltInTypeMask,
                                                      &variant->Value.Matrix.Content,
                                                      arrayLength);
                // Encode dimension array
                if(status == STATUS_OK){
                    // length
                    status = Int32_Write(&variant->Value.Matrix.Dimensions, msgBuffer);
                }
                if(status == STATUS_OK){
                    // array
                    int32_t idx = 0;
                    for(idx = 0; idx < variant->Value.Matrix.Dimensions; idx ++){
                        status = Int32_Write(&variant->Value.Matrix.ArrayDimensions[idx], msgBuffer);
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

StatusCode ReadVariantNonArrayBuiltInType(UA_MsgBuffer* msgBuffer,
                                           UA_BuiltinId builtInTypeId,
                                           UA_VariantValue *val){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    switch(builtInTypeId){
        case UA_Boolean_Id:
            status = Boolean_Read(&val->Boolean, msgBuffer);
            break;
        case UA_SByte_Id:
            status = SByte_Read(&val->Sbyte, msgBuffer);
            break;
        case UA_Byte_Id:
            status = Byte_Read(&val->Byte, msgBuffer);
            break;
        case UA_Int16_Id:
            status = Int16_Read(&val->Int16, msgBuffer);
            break;
        case UA_UInt16_Id:
            status = UInt16_Read(&val->Uint16, msgBuffer);
            break;
        case UA_Int32_Id:
            status = Int32_Read(&val->Int32, msgBuffer);
            break;
        case UA_UInt32_Id:
            status = UInt32_Read(&val->Uint32, msgBuffer);
            break;
        case UA_Int64_Id:
            status = Int64_Read(&val->Int64, msgBuffer);
            break;
        case UA_UInt64_Id:
            status = UInt64_Read(&val->Uint64, msgBuffer);
            break;
        case UA_Float_Id:
            status = Float_Read(&val->Floatv, msgBuffer);
            break;
        case UA_Double_Id:
            status = Double_Read(&val->Doublev, msgBuffer);
            break;
        case UA_String_Id:
            status = String_Read(&val->String, msgBuffer);
            break;
        case UA_DateTime_Id:
            status = DateTime_Read(&val->Date, msgBuffer);
            break;
        case UA_Guid_Id:
            val->Guid = malloc(sizeof(UA_Guid));
            if(val->Guid != NULL){
                status = Guid_Read(val->Guid, msgBuffer);
            }else{
                status = STATUS_NOK;
            }
            break;
        case UA_ByteString_Id:
            status = ByteString_Read(&val->Bstring, msgBuffer);
            break;
        case UA_XmlElement_Id:
            status = XmlElement_Read(&val->XmlElt, msgBuffer);
            break;
        case UA_NodeId_Id:
            val->NodeId = malloc(sizeof(UA_NodeId));
            if(val->NodeId != NULL){
                status = NodeId_Read(val->NodeId, msgBuffer);
            }else{
                status = STATUS_NOK;
            }
            break;
        case UA_ExpandedNodeId_Id:
            val->ExpNodeId = malloc(sizeof(UA_ExpandedNodeId));
            if(val->ExpNodeId != NULL){
                status = ExpandedNodeId_Read(val->ExpNodeId, msgBuffer);
            }else{
                status = STATUS_NOK;
            }
            break;
        case UA_StatusCode_Id:
            status = StatusCode_Read(&val->Status, msgBuffer);
            break;
        case UA_QualifiedName_Id:
            val->Qname = malloc(sizeof(UA_QualifiedName));
            if(val->Qname != NULL){
                status = QualifiedName_Read(val->Qname, msgBuffer);
            }else{
                status = STATUS_NOK;
            }
            break;
        case UA_LocalizedText_Id:
            val->LocalizedText = malloc(sizeof(UA_LocalizedText));
            if(val->LocalizedText != NULL){
                status = LocalizedText_Read(val->LocalizedText, msgBuffer);
            }else{
                status = STATUS_NOK;
            }
            break;
        case UA_ExtensionObject_Id:
            val->ExtObject = malloc(sizeof(UA_ExtensionObject));
            if(val->ExtObject != NULL){
                status = ExtensionObject_Read(val->ExtObject, msgBuffer);
            }else{
                status = STATUS_NOK;
            }
            break;
        case UA_DataValue_Id:
            val->DataValue = malloc(sizeof(UA_DataValue));
            if(val->DataValue != NULL){
                status = DataValue_Read(val->DataValue, msgBuffer);
            }else{
                status = STATUS_NOK;
            }
            break;
        case UA_Variant_Id:
            assert(FALSE);
            break;
        case UA_DiagnosticInfo_Id:
            val->DiagInfo = malloc(sizeof(UA_DiagnosticInfo));
            if(val->DiagInfo != NULL){
                status = DiagnosticInfo_Read(val->DiagInfo, msgBuffer);
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
            array->BooleanArr = malloc(sizeof(UA_Boolean) * length);
            if(array->BooleanArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= Boolean_Read(&array->BooleanArr[idx], msgBuffer);
                }
            }
            break;
        case UA_SByte_Id:
            array->SbyteArr = malloc(sizeof(UA_SByte) * length);
            if(array->SbyteArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= SByte_Read(&array->SbyteArr[idx], msgBuffer);
                }
            }
            break;
        case UA_Byte_Id:
            array->ByteArr = malloc(sizeof(UA_Byte) * length);
            if(array->ByteArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= Byte_Read(&array->ByteArr[idx], msgBuffer);
                }
            }
            break;
        case UA_Int16_Id:
            array->Int16Arr = malloc(sizeof(int16_t) * length);
            if(array->Int16Arr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= Int16_Read(&array->Int16Arr[idx], msgBuffer);
                }
            }
            break;
        case UA_UInt16_Id:
            array->SbyteArr = malloc(sizeof(uint16_t) * length);
            if(array->SbyteArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= UInt16_Read(&array->Uint16Arr[idx], msgBuffer);
                }
            }
            break;
        case UA_Int32_Id:
            array->Int32Arr = malloc(sizeof(int32_t) * length);
            if(array->Int32Arr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= Int32_Read(&array->Int32Arr[idx], msgBuffer);
                }
            }
            break;
        case UA_UInt32_Id:
            array->Uint32Arr = malloc(sizeof(uint32_t) * length);
            if(array->Uint32Arr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= UInt32_Read(&array->Uint32Arr[idx], msgBuffer);
                }
            }
            break;
        case UA_Int64_Id:
            array->Int64Arr = malloc(sizeof(int64_t) * length);
            if(array->Int64Arr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= Int64_Read(&array->Int64Arr[idx], msgBuffer);
                }
            }
            break;
        case UA_UInt64_Id:
            array->Uint64Arr = malloc(sizeof(uint64_t) * length);
            if(array->Uint64Arr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= UInt64_Read(&array->Uint64Arr[idx], msgBuffer);
                }
            }
            break;
        case UA_Float_Id:
            array->FloatvArr = malloc(sizeof(float) * length);
            if(array->FloatvArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= Float_Read(&array->FloatvArr[idx], msgBuffer);
                }
            }
            break;
        case UA_Double_Id:
            array->DoublevArr = malloc(sizeof(double) * length);
            if(array->DoublevArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= Double_Read(&array->DoublevArr[idx], msgBuffer);
                }
            }
            break;
        case UA_String_Id:
            array->StringArr = malloc(sizeof(UA_String) * length);
            if(array->StringArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= String_Read(&array->StringArr[idx], msgBuffer);
                }
            }
            break;
        case UA_DateTime_Id:
            array->DateArr = malloc(sizeof(UA_DateTime) * length);
            if(array->DateArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= DateTime_Read(&array->DateArr[idx], msgBuffer);
                }
            }
            break;
        case UA_Guid_Id:
            array->GuidArr = malloc(sizeof(UA_Guid) * length);
            if(array->GuidArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= Guid_Read(&array->GuidArr[idx], msgBuffer);
                }
            }
            break;
        case UA_ByteString_Id:
            array->BstringArr = malloc(sizeof(UA_ByteString) * length);
            if(array->BstringArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= ByteString_Read(&array->BstringArr[idx], msgBuffer);
                }
            }
            break;
        case UA_XmlElement_Id:
            array->XmlEltArr = malloc(sizeof(UA_XmlElement) * length);
            if(array->XmlEltArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= XmlElement_Read(&array->XmlEltArr[idx], msgBuffer);
                }
            }
            break;
        case UA_NodeId_Id:
            array->NodeIdArr = malloc(sizeof(UA_NodeId) * length);
            if(array->NodeIdArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= NodeId_Read(&array->NodeIdArr[idx], msgBuffer);
                }
            }
            break;
        case UA_ExpandedNodeId_Id:
            array->ExpNodeIdArr = malloc(sizeof(UA_ExpandedNodeId) * length);
            if(array->ExpNodeIdArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= ExpandedNodeId_Read(&array->ExpNodeIdArr[idx], msgBuffer);
                }
            }
            break;
        case UA_StatusCode_Id:
            array->StatusArr = malloc(sizeof(StatusCode) * length);
            if(array->StatusArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= StatusCode_Read(&array->StatusArr[idx], msgBuffer);
                }
            }
            break;
        case UA_QualifiedName_Id:
            array->QnameArr = malloc(sizeof(UA_QualifiedName) * length);
            if(array->QnameArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= QualifiedName_Read(&array->QnameArr[idx], msgBuffer);
                }
            }
            break;
        case UA_LocalizedText_Id:
            array->LocalizedTextArr = malloc(sizeof(UA_LocalizedText) * length);
            if(array->LocalizedTextArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= LocalizedText_Read(&array->LocalizedTextArr[idx], msgBuffer);
                }
            }
            break;
        case UA_ExtensionObject_Id:
            array->ExtObjectArr = malloc(sizeof(UA_ExtensionObject) * length);
            if(array->ExtObjectArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= ExtensionObject_Read(&array->ExtObjectArr[idx], msgBuffer);
                }
            }
            break;
        case UA_DataValue_Id:
            array->DataValueArr = malloc(sizeof(UA_DataValue) * length);
            if(array->DataValueArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= DataValue_Read(&array->DataValueArr[idx], msgBuffer);
                }
            }
            break;
        case UA_Variant_Id:
            array->VariantArr = malloc(sizeof(UA_Variant) * length);
            if(array->VariantArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= Variant_Read(&array->VariantArr[idx], msgBuffer);
                }
            }
            break;
        case UA_DiagnosticInfo_Id:
            array->DiagInfoArr = malloc(sizeof(UA_DiagnosticInfo) * length);
            if(array->DiagInfoArr == NULL){
                status = STATUS_NOK;
            }else{
                for(idx = 0; idx < length; idx++){
                    status |= DiagnosticInfo_Read(&array->DiagInfoArr[idx], msgBuffer);
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
            variant->ArrayTypeMask = UA_VariantArrayValueFlag;
            if((encodingByte & UA_VariantArrayMatrixFlag) != 0){
                variant->ArrayTypeMask |= UA_VariantArrayMatrixFlag;
            }
            // Read array length
            status = Int32_Read(&arrayLength, msgBuffer);
        }else if((encodingByte & UA_VariantArrayMatrixFlag) != 0){
            status = STATUS_INVALID_PARAMETERS;
        }
        // Retrieve builtin type id: avoid 2^7 and 2^6 which are array flags
        variant->BuiltInTypeMask = 0x3F & encodingByte;
    }

    if(status == STATUS_OK){
        if((variant->ArrayTypeMask & UA_VariantArrayValueFlag) != 0){
            status = ReadVariantArrayBuiltInType(msgBuffer,
                                                 variant->BuiltInTypeMask,
                                                 &variant->Value.Matrix.Content,
                                                 arrayLength);
            if(status == STATUS_OK && (variant->ArrayTypeMask & UA_VariantArrayMatrixFlag) != 0){
                // Decode dimension array
                if(status == STATUS_OK){
                    // length
                    status = Int32_Read(&variant->Value.Matrix.Dimensions, msgBuffer);
                }
                if(status == STATUS_OK){
                    // array
                    variant->Value.Matrix.ArrayDimensions = malloc(sizeof(int32_t) * variant->Value.Matrix.Dimensions);
                    if(variant->Value.Matrix.ArrayDimensions != NULL){
                        int32_t idx = 0;
                        for(idx = 0; idx < variant->Value.Matrix.Dimensions; idx ++){
                            status = Int32_Read(&variant->Value.Matrix.ArrayDimensions[idx], msgBuffer);
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

UA_Byte GetDataValueEncodingMask(const UA_DataValue* dataValue){
    assert(dataValue != NULL);
    UA_Byte mask = 0;
    if(dataValue->Value.BuiltInTypeMask != UA_Null_Id && dataValue->Value.BuiltInTypeMask <= UA_BUILTINID_MAX){
        mask |= DataValue_NotNullValue;
    }
    if(dataValue->Status != STATUS_OK){
        mask |= DataValue_NotGoodStatusCode;
    }
    if(dataValue->SourceTimestamp > 0){
        mask |= DataValue_NotMinSourceDate;
    }
    if(dataValue->SourcePicoSeconds > 0){
        mask |= DataValue_NotZeroSourcePico;
    }
    if(dataValue->ServerTimestamp > 0){
            mask |= DataValue_NotMinServerDate;
    }
    if(dataValue->ServerPicoSeconds > 0){
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
        status = Variant_Write(&dataValue->Value, msgBuffer);
    }
    if(status == STATUS_OK && (encodingMask & DataValue_NotGoodStatusCode) != 0){
        status = StatusCode_Write(&dataValue->Status, msgBuffer);
    }
    if(status == STATUS_OK && (encodingMask & DataValue_NotMinSourceDate) != 0){
        status = DateTime_Write(&dataValue->SourceTimestamp, msgBuffer);
    }
    if(status == STATUS_OK && (encodingMask & DataValue_NotZeroSourcePico) != 0){
        status = UInt16_Write(&dataValue->SourcePicoSeconds, msgBuffer);
    }
    if(status == STATUS_OK && (encodingMask & DataValue_NotMinServerDate) != 0){
            status = DateTime_Write(&dataValue->ServerTimestamp, msgBuffer);
    }
    if(status == STATUS_OK && (encodingMask & DataValue_NotZeroServerPico) != 0){
        status = UInt16_Write(&dataValue->ServerPicoSeconds, msgBuffer);
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
        status = Variant_Read(&dataValue->Value, msgBuffer);
    }else{
        dataValue->Value.BuiltInTypeMask = UA_Null_Id;
    }
    if(status == STATUS_OK && (encodingMask & DataValue_NotGoodStatusCode) != 0){
        status = StatusCode_Read(&dataValue->Status, msgBuffer);
    }else{
        dataValue->Status = STATUS_OK;
    }
    if(status == STATUS_OK && (encodingMask & DataValue_NotMinSourceDate) != 0){
        status = DateTime_Read(&dataValue->SourceTimestamp, msgBuffer);
    }else{
        dataValue->SourceTimestamp = 0;
    }
    if(status == STATUS_OK && (encodingMask & DataValue_NotZeroSourcePico) != 0){
        status = UInt16_Read(&dataValue->SourcePicoSeconds, msgBuffer);
    }else{
        dataValue->SourcePicoSeconds = 0;
    }
    if(status == STATUS_OK && (encodingMask & DataValue_NotMinServerDate) != 0){
            status = DateTime_Read(&dataValue->ServerTimestamp, msgBuffer);
    }else{
        dataValue->ServerTimestamp = 0;
    }
    if(status == STATUS_OK && (encodingMask & DataValue_NotZeroServerPico) != 0){
        status = UInt16_Read(&dataValue->ServerPicoSeconds, msgBuffer);
    }else{
        dataValue->ServerPicoSeconds = 0;
    }
    return status;
}

