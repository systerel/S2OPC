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

/** \file
 *
 * Utils to manage events in the services layer
 */

#ifndef UTIL_EVENT_H_
#define UTIL_EVENT_H_

#include "constants.h"
#include "monitored_item_pointer_impl.h"
#include "sopc_types.h"

extern const SOPC_Server_Event_Types* initEventTypes;

bool init_event_types(SOPC_EndpointConfigIdx epIdx);

bool util_event__alloc_event_field_list(uint32_t clientHandle,
                                        int32_t nbSelectClauses,
                                        OpcUa_EventFieldList** ppEventFieldList);

void util_event__set_event_field_list_elt(char** preferredLocalesIds,
                                          const constants__t_TimestampsToReturn_i timestampToReturn,
                                          bool userAccessGranted,
                                          int32_t selectClauseIdx,
                                          const SOPC_InternalMonitoredItemFilterCtx* pFilterCtx,
                                          const SOPC_Event* pEvent,
                                          OpcUa_EventFieldList* pEventFieldList);

bool util_event__alloc_and_fill_event_field_list(const SOPC_InternalMonitoredItemFilterCtx* pFilterCtx,
                                                 uint32_t clientHandle,
                                                 char** preferredLocalesIds,
                                                 const constants__t_TimestampsToReturn_i timestampToReturn,
                                                 bool userAccessGranted,
                                                 const SOPC_Event* pEvent,
                                                 OpcUa_EventFieldList** ppEventFieldList);

bool util_event__gen_event_queue_overflow_notification(const SOPC_InternalMonitoredItemFilterCtx* pFilterCtx,
                                                       uint32_t clientHandle,
                                                       OpcUa_EventFieldList** ppEventFieldList);

#endif /* UTIL_EVENT_H_ */
