/*
 * wrapper_encodeableobject.c
 *
 *  Created on: Nov 25, 2016
 *      Author: vincent
 */

#include "wrapper_encodeableobject.h"
#include "sopc_encodeable.h"

SOPC_StatusCode OpcUa_EncodeableObject_Create(SOPC_EncodeableType* encTyp, void** encObject){
    return SOPC_Encodeable_Create(encTyp, encObject);
}

SOPC_StatusCode OpcUa_EncodeableObject_Delete(SOPC_EncodeableType* encTyp, void** encObject){
    return SOPC_Encodeable_Delete(encTyp, encObject);
}

SOPC_StatusCode OpcUa_EncodeableObject_CreateExtension(SOPC_EncodeableType*  encTyp,
                                                       SOPC_ExtensionObject* extObj,
                                                       void**                encObject){
    return SOPC_Encodeable_CreateExtension(extObj, encTyp, encObject);
}
