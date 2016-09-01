/*
 * private_key.h
 *
 *  Created on: Jul 22, 2016
 *      Author: vincent
 */

#ifndef INGOPCS_PRIVATE_KEY_H_
#define INGOPCS_PRIVATE_KEY_H_

#include "ua_builtintypes.h"

typedef struct {
    UA_ByteString* key; // to modify into non contiguous memory storage
} PrivateKey;

PrivateKey* PrivateKey_Create(UA_ByteString* key);
void PrivateKey_Delete(PrivateKey* pkey);

UA_ByteString* PrivateKey_BeginUse(PrivateKey* pkey);
void PrivateKey_EndUse(UA_ByteString* key);
uint32_t PrivateKey_GetSize(PrivateKey* pkey);

#endif /* INGOPCS_PRIVATE_KEY_H_ */
