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

#ifndef SOPC_DATASET_LAYER_H_
#define SOPC_DATASET_LAYER_H_

#include "sopc_network_layer.h"
#include "sopc_pubsub_conf.h"

typedef SOPC_Dataset_LL_NetworkMessage SOPC_Dataset_NetworkMessage;

/* Create a NetworkMessage from a given configuration */
SOPC_Dataset_NetworkMessage* SOPC_Create_NetworkMessage_From_WriterGroup(SOPC_WriterGroup* group);

void SOPC_Delete_NetworkMessage(SOPC_Dataset_NetworkMessage* nm);

void SOPC_NetworkMessage_Set_Variant_At(SOPC_Dataset_NetworkMessage* nm,
                                        uint8_t dsm_index,
                                        uint16_t dsf_index,
                                        SOPC_Variant* variant,
                                        SOPC_FieldMetaData* metadata);

#endif /* SOPC_DATASET_LAYER_H_ */
