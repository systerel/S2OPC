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

#ifndef SOPC_PUB_SCHEDULER_H_
#define SOPC_PUB_SCHEDULER_H_

#include "sopc_event_handler.h"
#include "sopc_pub_source_variable.h"
#include "sopc_pubsub_conf.h"

bool SOPC_PubScheduler_Start(SOPC_PubSubConfiguration* config, SOPC_PubSourceVariableConfig* sourceConfig);

void SOPC_PubScheduler_Stop(void);

// To be called to close Event Timer Service
void SOPC_PubScheduler_Finalize(void);

// To be called from application to notify response
SOPC_ReturnStatus SOPC_PubScheduler_EnqueueComEvent(SOPC_PubScheduler_Event event,
                                                    uint32_t id,
                                                    uintptr_t params,
                                                    uintptr_t auxParam);

#endif /* SOPC_PUB_SCHEDULER_H_ */
