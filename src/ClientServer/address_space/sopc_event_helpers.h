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
 * \brief Provides various helpers for events fields filling.
 */

#ifndef SOPC_EVENT_HELPERS_H_
#define SOPC_EVENT_HELPERS_H_

#include "sopc_builtintypes.h"
#include "sopc_enums.h"
#include "sopc_event.h"

// The following helpers set a single value in a variant (various types)
void SOPC_EventHelpers_SetNull(SOPC_Event* const event, const char* qnPath);
void SOPC_EventHelpers_SetDate(SOPC_Event* const event, const char* qnPath, const SOPC_DateTime value);
void SOPC_EventHelpers_SetDouble(SOPC_Event* const event, const char* qnPath, const double value);
void SOPC_EventHelpers_SetBool(SOPC_Event* const event, const char* qnPath, const bool value);
void SOPC_EventHelpers_SetUInt32(SOPC_Event* const event, const char* qnPath, const uint32_t value);
void SOPC_EventHelpers_SetInt32(SOPC_Event* const event, const char* qnPath, const int32_t value);
void SOPC_EventHelpers_SetStatusCode(SOPC_Event* const event, const char* qnPath, const SOPC_StatusCode value);
/** Set a String attribute using a C-String as Value. The value is copied and can be released/modified after call. */
void SOPC_EventHelpers_SetCString(SOPC_Event* const event, const char* qnPath, const char* value);
/** Set a String attribute using another String as Value. The value is copied and can be released/modified after call.
 */
void SOPC_EventHelpers_SetString(SOPC_Event* const event, const char* qnPath, const SOPC_String* value);
/** Set a ByteString attribute using a memory buffer as Value. The value is copied and can be released/modified after
 * call. */
void SOPC_EventHelpers_SetByteString(SOPC_Event* const event,
                                     const char* qnPath,
                                     const uint8_t* value,
                                     const uint32_t len);
/** Set a SOPC_NodeId attribute using another NodeId _buias Value. The value is copied and can be released/modified
 * after call. */
void SOPC_EventHelpers_SetNodeId(SOPC_Event* const event, const char* qnPath, const SOPC_NodeId* value);

/** Set the attributes of an AuditEventType event. This function does not set inherited attributes
 * Note : \p auditEntryId can be NULL if no context connection is available*/
void SOPC_EventsHelpers_SetAuditEventType(SOPC_Event* const event,
                                          const char* auditEntryId,
                                          const SOPC_DateTime actionTimestamp,
                                          const char* serverUri,
                                          const char* clientUserId,
                                          const bool bStatus);

/** Set the attributes of a BaseEventType event. This function does not set inherited attributes */
void SOPC_EventsHelpers_SetBaseEventType(SOPC_Event* const event,
                                         const char* sourceNameC,
                                         const char* message,
                                         uint16_t severity);

/** Set the attributes of an AuditSecurityEventType event. This function does not set inherited attributes */
void SOPC_EventsHelpers_SetAuditSecurityEventType(SOPC_Event* const event, const SOPC_StatusCode statusCode);

/** Set the attributes of an AuditChannelEventType event. This function does not set inherited attributes */
void SOPC_EventsHelpers_SetAuditChannelEventType(SOPC_Event* const event, uint32_t secureChannelId);

#endif // SOPC_EVENT_HELPERS_H_
