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
            pkey = (Private_Key*) malloc(sizeof(Private_Key));
            if(pkey != NULL){
            	pkey->key = Create_Byte_String_Copy(key);
            }
        }
    }
    return pkey;
}

UA_Byte_String* Begin_Use_Private_Key (Private_Key* pkey){
    UA_Byte_String* key = NULL;
    if(pkey != NULL){
        if(pkey->key->length > 0){
            key = Create_Byte_String_Copy(pkey->key);
        }// In other cases an empty key is not a valid key
    }
    return key;
}

void End_Use_Private_Key (UA_Byte_String* key){
    if(key != NULL){
        Delete_Byte_String(key);
    }
}

void Delete_Private_Key (Private_Key* pkey){
    if(pkey != NULL){
    	if(pkey->key != NULL){
    		Delete_Byte_String(pkey->key);
    	}
        free(pkey);
    }
}


