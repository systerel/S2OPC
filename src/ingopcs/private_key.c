/*
 * private_key.c
 *
 *  Created on: Jul 27, 2016
 *      Author: vincent
 */

#include <private_key.h>
#include <stdlib.h>
#include <string.h>

Private_Key* Create_Private_Key (UA_Byte_String* key){
    Private_Key* pkey = UA_NULL;
    if(key != UA_NULL){
        if(key->length > 0){
            pkey = (Private_Key*) malloc(sizeof(Private_Key));
            if(pkey != UA_NULL){
                pkey->key = Create_Byte_String_Copy(key);
            }
        }
    }
    return pkey;
}

void Delete_Private_Key (Private_Key* pkey){
    if(pkey != UA_NULL){
        if(pkey->key != UA_NULL){
            Delete_Byte_String(pkey->key);
        }
        free(pkey);
    }
}

UA_Byte_String* Begin_Use_Private_Key (Private_Key* pkey){
    UA_Byte_String* key = UA_NULL;
    if(pkey != UA_NULL){
        if(pkey->key->length > 0){
            key = Create_Byte_String_Copy(pkey->key);
        }// In other cases an empty key is not a valid key
    }
    return key;
}

void End_Use_Private_Key (UA_Byte_String* key){
    if(key != UA_NULL){
        Delete_Byte_String(key);
    }
}

uint32_t Get_Private_Key_Size(Private_Key* pkey){
    return pkey->key->length;
}

