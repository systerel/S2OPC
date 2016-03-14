/*
 * private_key.h
 *
 *  Created on: Jul 22, 2016
 *      Author: vincent
 */

#ifndef INGOPCS_PRIVATE_KEY_H_
#define INGOPCS_PRIVATE_KEY_H_

#include <opcua_ingopcs_types.h>

typedef struct Private_Key {
    UA_Byte_String key; // to modify into non contiguous memory storage
} Private_Key;

Private_Key* Create_Private_Key (UA_Byte_String* key);
UA_Byte_String* Begin_Use_Private_Key (Private_Key* pkey);
void End_Use_Private_Key (UA_Byte_String* key);
void Delete_Private_Key (Private_Key* pkey);

#endif /* INGOPCS_PRIVATE_KEY_H_ */
