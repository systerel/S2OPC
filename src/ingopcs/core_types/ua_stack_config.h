/*
 * ua_stack_config.h
 *
 *  Created on: Oct 3, 2016
 *      Author: vincent
 */

#ifndef INGOPCS_UA_STACK_CONFIG_H_
#define INGOPCS_UA_STACK_CONFIG_H_

#include <ua_builtintypes.h>
#include <ua_namespace_table.h>

typedef struct {
    uint8_t             traceLevel; // enum TBD
    // TBD: lengths configurable
    UA_NamespaceTable*  nsTable;
    UA_EncodeableType** encTypesTable;
    uint32_t            nbEncTypesTable;
} UA_StackConfiguration;

extern UA_StackConfiguration g_stackConfiguration;

StatusCode StackConfiguration_Initialize(); // Init stack configuration and platform dependent code
void StackConfiguration_Locked();
void StackConfiguration_Unlocked();
void StackConfiguration_Clear();

StatusCode StackConfiguration_SetNamespaceUris(UA_NamespaceTable* nsTable);
StatusCode StackConfiguration_AddTypes(UA_EncodeableType** encTypesTable,
                                       uint32_t            nbTypes);
UA_EncodeableType** StackConfiguration_GetEncodeableTypes();
UA_NamespaceTable* StackConfiguration_GetNamespaces();

#endif /* INGOPCS_UA_STACK_CONFIG_H_ */
