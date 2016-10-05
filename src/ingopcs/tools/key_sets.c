/*
 * key_sets.c
 *
 *  Created on: Sep 15, 2016
 *      Author: vincent
 */

#include <key_sets.h>
#include <stdlib.h>
#include <stddef.h>

SC_SecurityKeySet* KeySet_Create(){
    SC_SecurityKeySet* keySet = malloc(sizeof(SC_SecurityKeySet));
    return keySet;
}

void KeySet_Delete(SC_SecurityKeySet* keySet){
    if(keySet != NULL){
        SecretBuffer_DeleteClear(keySet->encryptKey);
        SecretBuffer_DeleteClear(keySet->initVector);
        SecretBuffer_DeleteClear(keySet->signKey);
        free(keySet);
    }
}
