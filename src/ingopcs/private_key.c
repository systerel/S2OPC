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
    Private_Key* pkey = NULL;
    if(key != NULL){
        if(key->length > 0){
            pkey = (Private_Key*) malloc(sizeof(Private_Key) + sizeof(UA_Byte) * key->length);
        }
    }
    return pkey;
}

UA_Byte_String* Begin_Use_Private_Key (Private_Key* pkey){
    UA_Byte_String* key = NULL;
    if(pkey != NULL){
        if(pkey->key.length > 0){
            key = (UA_Byte_String*) malloc(sizeof(UA_Byte_String) + sizeof (UA_Byte) * pkey->key.length);
            memcpy (key, &pkey->key, sizeof(UA_Byte_String) + sizeof (UA_Byte) * pkey->key.length);
        }
    }
    return key;
}

void End_Use_Private_Key (UA_Byte_String* key){
    if(key != NULL){
        free(key);
    }
}

void Delete_Private_Key (Private_Key* pkey){
    if(pkey != NULL){
        free(pkey);
    }
}


