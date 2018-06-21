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

#ifndef SOPC_SECURE_CHANNELS_API_INTERNAL_H_
#define SOPC_SECURE_CHANNELS_API_INTERNAL_H_

#include <stdint.h>
#include "sopc_event_dispatcher_manager.h"
#include "sopc_secure_channels_api.h"

/* Secure channel internal event enqueue function
 * IMPORTANT NOTE: non-internal events use will cause an assertion error */
void SOPC_SecureChannels_EnqueueInternalEvent(SOPC_SecureChannels_InputEvent scEvent,
                                              uint32_t id,
                                              void* params,
                                              uintptr_t auxParam);

/* Secure channel internal event enqueue function: event will be enqueued as next to be treated (only for close SC
 * situation) Note: it is important to close the SC as soon as possible in order to avoid any treatment of new messages
 * on a SC to be closed. IMPORTANT NOTE: non-internal events will be refused */
void SOPC_SecureChannels_EnqueueInternalEventAsNext(SOPC_SecureChannels_InputEvent scEvent,
                                                    uint32_t id,
                                                    void* params,
                                                    uintptr_t auxParam);

// Internal use only (timers)
SOPC_EventDispatcherManager* SOPC_SecureChannels_GetEventDispatcher(void);

#endif /* SOPC_SECURE_CHANNELS_API_INTERNAL_H_ */
