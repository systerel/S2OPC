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
typedef uint32_t (UA_EncodeableObject_PfnEncode) (struct UA_MsgBuffer* msgBuffer, void* value);
typedef uint32_t (UA_EncodeableObject_PfnDecode) (struct UA_MsgBuffer* msgBuffer, void* value);

typedef struct UA_EncodeableType {
    char*                              name;
    uint32_t                           typeId;
    uint32_t                           binaryTypeId;
    uint32_t                           xmlTypeId;
    char*                              namespace;
    uint32_t                           allocSize;
    UA_EncodeableObject_PfnInitialize* initFunction;
    UA_EncodeableObject_PfnClear*      clearFunction;
    UA_EncodeableObject_PfnGetSize*    getSizeFunction;
    UA_EncodeableObject_PfnEncode*     encodeFunction;
    UA_EncodeableObject_PfnDecode*     decodeFunction;
} UA_EncodeableType;


UA_EncodeableType* EncodeableType_GetEncodeableType(UA_EncodeableType** encTypesTable, // null terminated enctype* list
                                                    const char*         namespace,
                                                    uint32_t            typeId);

#endif /* INGOPCS_UA_ENCODEABLE_H_ */
