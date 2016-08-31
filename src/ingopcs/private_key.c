/*
 * private_key.c
 *
 *  Created on: Jul 27, 2016
 *      Author: vincent
 */

#include <private_key.h>
#include <stdlib.h>
#include <string.h>

PrivateKey* PrivateKey_Create(UA_ByteString* key)
{
    PrivateKey* pkey = UA_NULL;
    if(key != UA_NULL){
        if(key->length > 0){
            pkey = (PrivateKey*) malloc(sizeof(PrivateKey));
            if(pkey != UA_NULL){
                pkey->key = ByteString_Copy(key);
            }
        }
    }
    return pkey;
}

void PrivateKey_Delete(PrivateKey* pkey)
{
    if(pkey != UA_NULL){
        if(pkey->key != UA_NULL){
            ByteString_Clear(pkey->key);
        }
        free(pkey);
    }
}

UA_ByteString* PrivateKey_BeginUse(PrivateKey* pkey)
{
    UA_ByteString* key = UA_NULL;
    if(pkey != UA_NULL){
        if(pkey->key->length > 0){
            key = ByteString_Copy(pkey->key);
        }// In other cases an empty key is not a valid key
    }
    return key;
}

void PrivateKey_EndUse(UA_ByteString* key)
{
    if(key != UA_NULL){
        memset(key->characters, 0, key->length);
        ByteString_Clear(key);
    }
}

uint32_t PrivateKey_GetSize(PrivateKey* pkey)
{
    return pkey->key->length;
}

