/*
 * key_sets.h
 *
 *  Created on: Sep 15, 2016
 *      Author: vincent
 */

#ifndef INGOPCS_KEY_SETS_H_
#define INGOPCS_KEY_SETS_H_

#include <secret_buffer.h>

typedef struct SC_SecurityKeySet{
    SecretBuffer* signKey;
    SecretBuffer* encryptKey;
    SecretBuffer* initVector;
} SC_SecurityKeySet;

typedef struct {
    SC_SecurityKeySet* senderKeySet;
    SC_SecurityKeySet* receiverKeySet;
} SC_SecurityKeySets;

SC_SecurityKeySet* KeySet_Create();
void KeySet_Delete(SC_SecurityKeySet* keySet);

#endif /* INGOPCS_KEY_SETS_H_ */
