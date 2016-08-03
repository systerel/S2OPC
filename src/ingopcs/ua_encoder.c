/*
 * ua_encoder.c
 *
 *  Created on: Aug 3, 2016
 *      Author: vincent
 */

#include <assert.h>
#include <tcp_ua_low_level.h>
#include <ua_encoder.h>

uint32_t Encode_UInt32(uint32_t from){
    assert(pendianess != Endianess_Undefined);
    if(pendianess == Endianess_BigEndian){
        return SWAP_4_BYTES(from);
    }else{
        return from;
    }
}

uint32_t Encode_Int32(int32_t from){
    assert(pendianess != Endianess_Undefined);
    if(pendianess == Endianess_BigEndian){
        return SWAP_4_BYTES(from);
    }else{
        return from;
    }
}

StatusCode Write_UInt32(UA_Msg_Buffer* msgBuffer, uint32_t value){
    StatusCode status = STATUS_NOK;
    uint32_t encodedValue = Encode_UInt32(value);
    status = Write_Msg_Buffer(msgBuffer, (UA_Byte*) &encodedValue, 4);
    return status;
}

StatusCode Write_Int32(UA_Msg_Buffer* msgBuffer, int32_t value){
    StatusCode status = STATUS_NOK;
    int32_t encodedValue = Encode_Int32(value);
    status = Write_Msg_Buffer(msgBuffer, (UA_Byte*) &encodedValue, 4);
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
