/*
 * wrapper_encodeableobject.h
 *
 *  Created on: Nov 25, 2016
 *      Author: vincent
 */

#ifndef SOPC_WRAPPER_ENCODEABLEOBJECT_H_
#define SOPC_WRAPPER_ENCODEABLEOBJECT_H_

#include "sopc_encodeabletype.h"
#include "sopc_builtintypes.h"

SOPC_StatusCode OpcUa_EncodeableObject_Create(SOPC_EncodeableType* encTyp, void** encObject);
SOPC_StatusCode OpcUa_EncodeableObject_Delete(SOPC_EncodeableType* encTyp, void** encObject);
SOPC_StatusCode OpcUa_EncodeableObject_CreateExtension(SOPC_EncodeableType*  encTyp,
                                                       SOPC_ExtensionObject* extObj,
                                                       void**                encObject);

#endif /* SOPC_WRAPPER_ENCODEABLEOBJECT_H_ */
