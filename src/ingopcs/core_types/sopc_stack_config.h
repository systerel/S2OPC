/*
 *sopc_stack_config.h
 *
 *  Created on: Oct 3, 2016
 *      Author: vincent
 */

#ifndef SOPC_STACK_CONFIG_H_
#define SOPC_STACK_CONFIG_H_

#include "sopc_builtintypes.h"
#include "sopc_namespace_table.h"

typedef struct {
    uint8_t             traceLevel; // enum TBD
    // TBD: lengths configurable
    SOPC_NamespaceTable*  nsTable;
    SOPC_EncodeableType** encTypesTable;
    uint32_t            nbEncTypesTable;
} SOPC_StackConfiguration;

extern SOPC_StackConfiguration g_stackConfiguration;

SOPC_StatusCode StackConfiguration_Initialize(); // Init stack configuration and platform dependent code
void StackConfiguration_Locked();
void StackConfiguration_Unlocked();
void StackConfiguration_Clear();

SOPC_StatusCode StackConfiguration_SetNamespaceUris(SOPC_NamespaceTable* nsTable);
SOPC_StatusCode StackConfiguration_AddTypes(SOPC_EncodeableType** encTypesTable,
                                       uint32_t            nbTypes);
SOPC_EncodeableType** StackConfiguration_GetEncodeableTypes();
SOPC_NamespaceTable* StackConfiguration_GetNamespaces();

#endif /* INGOPCS_SOPC_STACK_CONFIG_H_ */
