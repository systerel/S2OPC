/*
 * key_sets.c
 *
 *  Created on: Sep 15, 2016
 *      Author: vincent
 */

#include <key_sets.h>

SC_SecurityKeySet* KeySet_Create(){
    SC_SecurityKeySet* keySet = malloc(sizeof(SC_SecurityKeySet));
    return keySet;
}

void KeySet_Delete(SC_SecurityKeySet* keySet){
    if(keySet != 0){
        PrivateKey_Delete(keySet->encryptKey);
        PrivateKey_Delete(keySet->initVector);
        PrivateKey_Delete(keySet->signKey);
        free(keySet);
    }
}
