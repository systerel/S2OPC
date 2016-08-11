/*
 * ua_encoder.c
 *
 *  Created on: Aug 3, 2016
 *      Author: vincent
 */

#include <stdlib.h>
#include <assert.h>
#include <tcp_ua_low_level.h>
#include <ua_encoder.h>

uint16_t EncodeDecode_UInt16(uint16_t from){
    assert(endianess != Endianess_Undefined);
    if(endianess == Endianess_BigEndian){
        return SWAP_2_BYTES(from);
    }else{
        return from;
    }
}

uint32_t EncodeDecode_UInt32(uint32_t from){
    assert(endianess != Endianess_Undefined);
    if(endianess == Endianess_BigEndian){
        return SWAP_4_BYTES(from);
    }else{
        return from;
    }
}

uint32_t EncodeDecode_Int32(int32_t from){
    assert(endianess != Endianess_Undefined);
    if(endianess == Endianess_BigEndian){
        return SWAP_4_BYTES(from);
    }else{
        return from;
    }
}

StatusCode Write_UA_Boolean(UA_Msg_Buffer* msgBuffer, UA_Boolean value){
    StatusCode status = STATUS_NOK;
    uint32_t encodedValue;
    if(value == UA_FALSE){
        encodedValue = EncodeDecode_UInt32(value);
    }else{
        // Encoder should use 1 as True value
        encodedValue = EncodeDecode_UInt32(1);
    }
    status = Write_Msg_Buffer(msgBuffer, (UA_Byte*) &encodedValue, 4);
    return status;
}

StatusCode Read_UA_Boolean(UA_Msg_Buffer* msgBuffer, UA_Boolean* value){
    StatusCode status = STATUS_NOK;
    uint32_t readValue;
    if(value == UA_NULL){
        status = STATUS_INVALID_PARAMETERS;
    }else{
        status = Read_Msg_Buffer((UA_Byte*)&readValue, 4, msgBuffer, 4);
        if(status == STATUS_OK){
            *value = EncodeDecode_UInt32(readValue);
            if(*value != UA_FALSE){
                *value = 1;
            }
        }
    }
    return status;
}

StatusCode Write_UInt32(UA_Msg_Buffer* msgBuffer, uint32_t value){
    StatusCode status = STATUS_NOK;
    uint32_t encodedValue = EncodeDecode_UInt32(value);
    status = Write_Msg_Buffer(msgBuffer, (UA_Byte*) &encodedValue, 4);
    return status;
}

StatusCode Read_UInt32(UA_Msg_Buffer* msgBuffer, uint32_t* value){
    StatusCode status = STATUS_NOK;
    uint32_t readValue;
    if(value == UA_NULL){
        status = STATUS_INVALID_PARAMETERS;
    }else{
        status = Read_Msg_Buffer((UA_Byte*)&readValue, 4, msgBuffer, 4);
        if(status == STATUS_OK){
            *value = EncodeDecode_UInt32(readValue);
        }
    }
    return status;
}

StatusCode Write_Int32(UA_Msg_Buffer* msgBuffer, int32_t value){
    StatusCode status = STATUS_NOK;
    int32_t encodedValue = EncodeDecode_Int32(value);
    status = Write_Msg_Buffer(msgBuffer, (UA_Byte*) &encodedValue, 4);
    return status;
}

StatusCode Read_Int32(UA_Msg_Buffer* msgBuffer, int32_t* value){
    StatusCode status = STATUS_NOK;
    int32_t readValue;
    if(value == UA_NULL){
        status = STATUS_INVALID_PARAMETERS;
    }else{
        status = Read_Msg_Buffer((UA_Byte*)&readValue, 4, msgBuffer, 4);
        if(status == STATUS_OK){
            *value = EncodeDecode_Int32(readValue);
        }
    }
    return status;
}

StatusCode Write_UA_String(UA_Msg_Buffer* msgBuffer, UA_String* str){
    StatusCode status = STATUS_NOK;
    if(str == UA_NULL){
        status = STATUS_INVALID_PARAMETERS;
    }else{
        if(str->length > 0){
            status = Write_Int32(msgBuffer, str->length);
        }else{
            status = Write_Int32(msgBuffer, -1);
        }
        if(status == STATUS_OK &&
           str->length > 0)
        {
            status = Write_Msg_Buffer(msgBuffer, str->characters, str->length);
        }
    }
    return status;
}

StatusCode Read_UA_String(UA_Msg_Buffer* msgBuffer, UA_String* str){
    StatusCode status = STATUS_NOK;
    int32_t readValue, length;
    if(str == UA_NULL || (str != UA_NULL && str->characters != UA_NULL)){
        status = STATUS_INVALID_PARAMETERS;
    }else{
        status = Read_Msg_Buffer((UA_Byte*)&readValue, 4, msgBuffer, 4);
        if(status == STATUS_OK){
            length = EncodeDecode_Int32(readValue);
            if(length > 0){
                str->length = length;
                str->characters = malloc(sizeof(length));
                if(str->characters != UA_NULL){
                    status = Read_Msg_Buffer(str->characters, length, msgBuffer, length);
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
