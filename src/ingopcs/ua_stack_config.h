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
} UA_StackConfiguration;

extern UA_StackConfiguration g_stackConfiguration;

void StackConfiguration_Initialize();
void StackConfiguration_Clear();

StatusCode StackConfiguration_SetNamespaceUris(UA_NamespaceTable* nsTable);
StatusCode StackConfiguration_AddTypes(UA_EncodeableType** encTypesTable);

#endif /* INGOPCS_UA_STACK_CONFIG_H_ */
