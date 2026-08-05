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

/**
 * \file util_fast_init_clear.h
 * \brief Provides some customized Initialization and ClearContent functions designed for fast treatment:
 *        - Initialize: do not use the generic initializer to optimize CPU usage
 *        - ClearContent: only deallocate memory without clearing content (memory not set to zero)
 */

#ifndef UTIL_FAST_INIT_CLEAR_H_
#define UTIL_FAST_INIT_CLEAR_H_

#include <stddef.h>

#include "sopc_types.h"

/**
 * \brief Fast-path equivalent of ::OpcUa_WriteValue_Initialize: calls the same
 *        underlying typed Initialize functions (NodeId, String, DataValue...)
 *        directly instead of through the generic reflective
 *        SOPC_EncodeableObject_InternalInitialize field-descriptor dispatch.
 *        Behaviorally identical to the generic path; only avoids the per-field
 *        descriptor walk and indirect function-pointer calls it performs.
 * \param wv  WriteValue to initialize; must not be NULL.
 */
void SOPC_WriteValue_FastInitialize(OpcUa_WriteValue* wv);

/**
 * \brief Internal use only or for content never accessed after call.
 *        This function free the allocated content in the write value
 *        without clearing anything else. The WriteValue itself is not freed:
 *        the caller shall release it with the allocator that provided it
 *        (e.g. ::SOPC_Free for heap, ::SOPC_WriteValuePool_ClearAndFree already calls this function for the pool).
 *        It is used internally by services layer.
 * \param wv  WriteValue to clear; must not be NULL.
 */
void SOPC_WriteValue_FastFreeContent(OpcUa_WriteValue* wv);

/**
 * \brief Internal use only or for content never accessed after call,
 *        ::SOPC_Free(notifMsg) shall be done after call and pointer set to NULL.
 *        This function free the allocated content in the notification message
 *        without clearing anything else (only implemented for DataChangeNotif, otherwise classic Clear used).
 *        It is used internally by services layer.
 */
void SOPC_NotificationMessage_FastFreeContent(OpcUa_NotificationMessage* notifMsg);

#endif /* UTIL_FAST_INIT_CLEAR_H_ */
