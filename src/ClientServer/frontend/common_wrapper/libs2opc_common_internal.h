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
 * \privatesection
 *
 * \brief Internal module used to manage the wrapper for client and/or server config.
 * It should not be used outside of the client/server wrapper implementation.
 *
 */

#ifndef LIBS2OPC_COMMON_INTERNAL_H_
#define LIBS2OPC_COMMON_INTERNAL_H_

#include <inttypes.h>
#include <stdbool.h>

#include "sopc_address_space.h"
#include "sopc_mutexes.h"
#include "sopc_toolkit_config_constants.h"
#include "sopc_types.h"
#include "sopc_user_app_itf.h"

/**
 * \brief Define a function to be called for client side communication events.
 *        It should be called by a client wrapper to be record/unrecord its events callback
 *        (automatically done by ::SOPC_ClientHelper_Initialize)
 *
 * \note The callback definition and use is protected by a mutex
 *
 * \param clientComEvtCb  The function callback for all client related events
 *
 * \return SOPC_STATUS_OK in case of success, otherwise SOPC_STATUS_INVALID_STATE
 *         if the configuration is not possible (common wrapper not initialized).
 *
 * \warning The callback function shall not do anything blocking or long treatment since it will block any other
 *          callback call (other instance of write notification, local service sync/async response, etc.).
 */
SOPC_ReturnStatus SOPC_CommonHelper_SetClientComEvent(SOPC_ComEvent_Fct* clientComEvtCb);

/**
 * \brief Define a function to be called for server side communication events.
 *        It should be called by a server wrapper to be record/unrecord its events callback
 *        (automatically done by ::SOPC_HelperConfigServer_Initialize)
 *
 * \note The callback definition and use is protected by a mutex
 *
 * \param serverComEvtCb  The function callback for all server related events
 *
 * \return SOPC_STATUS_OK in case of success, otherwise SOPC_STATUS_INVALID_STATE
 *         if the configuration is not possible (common wrapper not initialized).
 *
 * \warning The callback function shall not do anything blocking or long treatment since it will block any other
 *          callback call (other instance of write notification, local service sync/async response, etc.).
 */
SOPC_ReturnStatus SOPC_CommonHelper_SetServerComEvent(SOPC_ComEvent_Fct* serverComEvtCb);

/**
 * \brief Copy the provided C string array of known size into a new NULL terminated C string array
 *
 *  \param nbElts Number of element in the provided C string array
 *  \param src    Array of C string to copy
 *
 *  \return A new NULL terminated C string array containing a copy of C strings from the source array.
 */
char** SOPC_CommonHelper_Copy_Char_Array(size_t nbElts, char** src);

#endif
