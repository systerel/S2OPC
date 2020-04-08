/*
 * Licensed to Systerel under one or more contributor license
 * agreements. See the NOTICE file distributed with this work
 * for additional information regarding copyright ownership.
 * Systerel licenses this file to you under the Apache
 * License, Version 2.0 (the "License"); you may not use this
 * file except in compliance with the License. You may obtain
 * a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#ifndef CONFIG_CYCLIC_H_
#define CONFIG_CYCLIC_H_

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "sopc_builtintypes.h"
#include "sopc_types.h"

#include "config.h"

/* Configuration :
 * The configuration is composed of 3 elements :
 * - Lifetime of the execution
 * - Period to send write request
 * - Number of elements to write
 * - Array of Builtin Type Id of the elements to write
 * - Array of Node Id of the elements to write
 */

// time in MS
#define CONF_LIFETIME 1000000

// time in MS. 1s
#define CONF_SEND_MS_PERIOD 1000

#define CONF_NB_NODE_TO_WRITE 8

/* Data Values Type of the Node to read/write.
   Values in set in config_cyclic.c */
extern SOPC_BuiltinId CONF_DV_TYPE_ARRAY[CONF_NB_NODE_TO_WRITE];

/* Node ID the Node to read/write.
   Values in set in config_cyclic.c */
extern char* CONF_NODE_ID_ARRAY[CONF_NB_NODE_TO_WRITE];

/* Commom Variables
   Uses by Read and Write.
   Do not touch
 */

/* The start NodeId is global, so that it is accessible to the Print function in the other thread. */
extern SOPC_NodeId* g_nodeIdArray[CONF_NB_NODE_TO_WRITE];

SOPC_ReturnStatus conf_initialize_nodeid_array(void);

#endif /* CONFIG_CYCLIC_H_ */
