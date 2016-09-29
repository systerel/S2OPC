/*
 * private_key.c
 *
 *  Created on: Jul 27, 2016
 *      Author: vincent
 */

#include <private_key.h>
#include <stdlib.h>
#include <string.h>


void PrivateKey_Initialize(PrivateKey* pkey){
    if(pkey != UA_NULL){
        ByteString_Initialize(&pkey->key);
    }
}

void PrivateKey_InitKey(PrivateKey* pkey, UA_ByteString* key){
    if(pkey != UA_NULL && key != UA_NULL){
        ByteString_Copy(&pkey->key, key);
    }
}

PrivateKey* PrivateKey_Create(UA_ByteString* key)
{
    PrivateKey* pkey = UA_NULL;
    if(key != UA_NULL){
        if(key->length > 0){
            pkey = (PrivateKey*) malloc(sizeof(PrivateKey));
            if(pkey != UA_NULL){
                PrivateKey_Initialize(pkey);
                PrivateKey_InitKey(pkey, key);
            }
        }
    }
    return pkey;
}

void PrivateKey_Clear(PrivateKey* pkey)
{
    ByteString_Clear(&pkey->key);
}

void PrivateKey_Delete(PrivateKey* pkey)
{
    if(pkey != UA_NULL){
        PrivateKey_Clear(pkey);
        free(pkey);
    }
}


UA_ByteString* PrivateKey_BeginUse(PrivateKey* pkey)
{
    UA_ByteString* key = ByteString_Create();
    if(pkey != UA_NULL){
        if(pkey->key.length > 0){
            ByteString_Copy(key, &pkey->key);
        }// In other cases an empty key is not a valid key
    }
    return key;
}

void PrivateKey_EndUse(UA_ByteString* key)
{
    if(key != UA_NULL){
        memset(key->characters, 0, key->length);
        ByteString_Delete(key);
    }
}

uint32_t PrivateKey_GetSize(PrivateKey* pkey)
{
    if(pkey->key.length <= 0){
        return 0;
    }
    return pkey->key.length;
}

