/*
 * ua_namespace_table.h
 *
 *  Created on: Sep 15, 2016
 *      Author: vincent
 */

#ifndef INGOPCS_UA_NAMESPACE_TABLE_H_
#define INGOPCS_UA_NAMESPACE_TABLE_H_

#include <ua_builtintypes.h>

#define OPCUA_NAMESPACE_INDEX 0
extern const char* OPCUA_NAMESPACE_NAME;

extern const int32_t OPCUA_NAMESPACE_NAME_MAXLENGTH;

typedef struct {
    uint16_t namespaceIndex;
    char*    namespaceName;
} UA_Namespace;

typedef struct {
    uint16_t      lastIdx;
    UA_Namespace* namespaceArray;
    uint8_t       clearTable;
} UA_NamespaceTable;

void Namespace_Initialize(UA_NamespaceTable* nsTable);

SOPC_StatusCode Namespace_AllocateTable(UA_NamespaceTable* nsTable, uint32_t length);

UA_NamespaceTable* Namespace_CreateTable(uint32_t length); // length + 1 <= UINT16_MAX

SOPC_StatusCode Namespace_AttachTable(UA_NamespaceTable* dst, UA_NamespaceTable* src);

SOPC_StatusCode Namespace_GetIndex(UA_NamespaceTable* namespaceTable,
                              const char*        namespaceName,
                              uint16_t*          index);
const char* Namespace_GetName(UA_NamespaceTable* namespaceTable,
                              uint16_t           index);

void Namespace_Clear(UA_NamespaceTable* namespaceTable);

void Namespace_Delete(UA_NamespaceTable* namespaceTable);

#endif /* INGOPCS_UA_NAMESPACE_TABLE_H_ */
