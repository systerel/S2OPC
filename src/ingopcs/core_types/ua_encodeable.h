/*
 * ua_encodeable.h
 *
 *  Created on: Sep 5, 2016
 *      Author: vincent
 */

#ifndef INGOPCS_UA_ENCODEABLE_H_
#define INGOPCS_UA_ENCODEABLE_H_

#include <stdint.h>

struct UA_MsgBuffer;

typedef void (UA_EncodeableObject_PfnInitialize) (void* value);
typedef void (UA_EncodeableObject_PfnClear) (void* value);
typedef void (UA_EncodeableObject_PfnGetSize) (void); // Deactivated
// TODO: replace by status code => need to be separated from builtin types to avoid mutal dep
typedef uint32_t (UA_EncodeableObject_PfnEncode) (void* value, struct UA_MsgBuffer* msgBuffer);
typedef uint32_t (UA_EncodeableObject_PfnDecode) (void* value, struct UA_MsgBuffer* msgBuffer);

typedef struct UA_EncodeableType {
    char*                              TypeName;
    uint32_t                           TypeId;
    uint32_t                           BinaryEncodingTypeId;
    uint32_t                           XmlEncodingTypeId;
    char*                              NamespaceUri;
    uint32_t                           AllocationSize;
    UA_EncodeableObject_PfnInitialize* Initialize;
    UA_EncodeableObject_PfnClear*      Clear;
    UA_EncodeableObject_PfnGetSize*    GetSize;
    UA_EncodeableObject_PfnEncode*     Encode;
    UA_EncodeableObject_PfnDecode*     Decode;
} UA_EncodeableType;


UA_EncodeableType* EncodeableType_GetEncodeableType(UA_EncodeableType** encTypesTable, // null terminated enctype* list
                                                    const char*         namespace,
                                                    uint32_t            typeId);

#endif /* INGOPCS_UA_ENCODEABLE_H_ */
