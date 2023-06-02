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
 * \brief High level interface to configure an OPC UA client and/or server
 *
 */

#ifndef LIBS2OPC_COMMON_CONFIG_H_
#define LIBS2OPC_COMMON_CONFIG_H_

#include <stdbool.h>

#include "sopc_common.h"
#include "sopc_encodeabletype.h"
#include "sopc_types.h"
#include "sopc_user_app_itf.h"

/**
 * \brief Initialize the S2OPC Client/Server frontend library (start threads, initialize configuration, etc.)
 *        and define a custom log configuration.
 *        Call to ::SOPC_CommonHelper_Initialize is required before any other operation.
 *
 * \param optLogConfig the custom log configuration or NULL to keep default configuration
 *
 * \result SOPC_STATUS_OK in case of success, otherwise SOPC_STATUS_INVALID_STATE in case of double initialization.
 */
SOPC_ReturnStatus SOPC_CommonHelper_Initialize(SOPC_Log_Configuration* optLogConfig);

/**
 * \brief Clear the S2OPC Client/Server frontend library (stop threads, clear common configuration, etc.)
 *        Call to ::SOPC_CommonHelper_Clear shall be done after any Client/Server wrapper Clear operations.
 */
void SOPC_CommonHelper_Clear(void);

/**
 * \brief Retrieve the S2OPC Client/Server frontend library build info (version, date, etc.).
 *        Shortcut to ::SOPC_ToolkitConfig_GetBuildInfo.
 *
 * \return Toolkit build information
 *
 */
SOPC_Toolkit_Build_Info SOPC_CommonHelper_GetBuildInfo(void);

/**
 * \brief Retrieve the S2OPC Client/Server configuration.
 *        It should be called after call to ::SOPC_CommonHelper_Initialize and before call to ::SOPC_CommonHelper_Clear
 *
 * \return The returned value is ensured to be a non-NULL pointer to Helper configuration
 *
 */
SOPC_S2OPC_Config* SOPC_CommonHelper_GetConfiguration(void);

/**
 * \brief Retrieve the S2OPC Client/Server frontend "initialized" state
 *
 * \return True if the Helper is initialized (when ::SOPC_CommonHelper_Initialize is called)
 *
 */
bool SOPC_CommonHelper_GetInitialized(void);

#endif
