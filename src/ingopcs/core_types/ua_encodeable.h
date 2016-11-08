/*
 * ua_encodeable.h
 *
 *  Created on: Sep 5, 2016
 *      Author: vincent
 */

#ifndef INGOPCS_SOPC_ENCODEABLE_H_
#define INGOPCS_SOPC_ENCODEABLE_H_

#include <stdint.h>

#include "sopc_base_types.h"

BEGIN_EXTERN_C

struct SOPC_MsgBuffer;

typedef void (SOPC_EncodeableObject_PfnInitialize) (void* value);
typedef void (SOPC_EncodeableObject_PfnClear) (void* value);
typedef void (SOPC_EncodeableObject_PfnGetSize) (void); // Deactivated
// TODO: replace by status code => need to be separated from builtin types to avoid mutal dep
typedef uint32_t (SOPC_EncodeableObject_PfnEncode) (void* value, struct SOPC_MsgBuffer* msgBuffer);
typedef uint32_t (SOPC_EncodeableObject_PfnDecode) (void* value, struct SOPC_MsgBuffer* msgBuffer);

typedef struct SOPC_EncodeableType {
    char*                              TypeName;
    uint32_t                           TypeId;
    uint32_t                           BinaryEncodingTypeId;
    uint32_t                           XmlEncodingTypeId;
    char*                              NamespaceUri;
    uint32_t                           AllocationSize;
    SOPC_EncodeableObject_PfnInitialize* Initialize;
    SOPC_EncodeableObject_PfnClear*      Clear;
    SOPC_EncodeableObject_PfnGetSize*    GetSize;
    SOPC_EncodeableObject_PfnEncode*     Encode;
    SOPC_EncodeableObject_PfnDecode*     Decode;
} SOPC_EncodeableType;


SOPC_EncodeableType* EncodeableType_GetEncodeableType(SOPC_EncodeableType** encTypesTable, // null terminated enctype* list
                                                    const char*         namespac,
                                                    uint32_t            typeId);

END_EXTERN_C

#endif /* INGOPCS_SOPC_ENCODEABLE_H_ */
