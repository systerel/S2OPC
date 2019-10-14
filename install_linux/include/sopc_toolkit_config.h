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
 * \file sopc_toolkit_config.h
 *
 * \brief This module shall be used to initialize, configure and clear/terminate the toolkit execution.
 *
 * It is in charge to initialize each event based layer (sockets, secure channels, services) which will
 * start the associated threads.
 * It is also necessary to configure the endpoint description configuration and address space of a toolkit server
 * instance, or the endpoint connection configuration of a toolkit client instance.
 *
 */

#ifndef SOPC_TOOLKIT_CONFIG_H_
#define SOPC_TOOLKIT_CONFIG_H_

#include "sopc_address_space.h"
#include "sopc_user_app_itf.h"

#include "sopc_builtintypes.h"
#include "sopc_types.h"

/**
 *  \brief  Initialize the toolkit configuration, libraries and threads
 *
 *  \param pAppFct  Pointer to applicative code function in charge of toolkit communication events
 *
 *  \return SOPC_STATUS_OK if initialization succeeded,
 *  SOPC_STATUS_INVALID_PARAMETERS if \p pAppFct == NULL or
 *  SOPC_STATUS_INVALID_STATE if toolkit already initialized and
 *  SOPC_STATUS_NOK otherwise
 */
SOPC_ReturnStatus SOPC_Toolkit_Initialize(SOPC_ComEvent_Fct* pAppFct);

/**
 *  \brief  Define toolkit configuration as configured and lock its state until toolkit clear operation
 *
 *  \return SOPC_STATUS_OK if initialization succeeded,
 *  SOPC_STATUS_INVALID_STATE if toolkit is not initialized or already
 *  configured,
 *  SOPC_STATUS_INVALID_PARAMETERS if server configuration is defined but no address space is set
 */
SOPC_ReturnStatus SOPC_Toolkit_Configured(void);

/**
 *  \brief  Clear the stack configuration
 */
void SOPC_Toolkit_Clear(void);

/**
 *  \brief Set the given Address Space for the current toolkit server
 *  (SOPC_ToolkitServer_Initialize required, !SOPC_Toolkit_Configured required).
 *  Note: only one address space can be set, further call will be refused.
 *
 *  \param addressSpace  The address space definition
 *
 *  \return SOPC_STATUS_OK if configuration succeeded,
 *  SOPC_STATUS_INVALID_STATE if toolkit is not initialized, already
 *  configured or address space is already set, SOPC_STATUS_NOK otherwise
 */
SOPC_ReturnStatus SOPC_ToolkitServer_SetAddressSpaceConfig(SOPC_AddressSpace* addressSpace);

/**
 *  \brief Set the given Address Space modification notification callback
 *  for the current toolkit server (SOPC_ToolkitServer_Initialize required, !SOPC_Toolkit_Configured required).
 *  Note: only one callback can be set, further call will be refused.
 *
 *  \param pAddSpaceNotifFct  The address space notification event callback definition
 *
 *  \return SOPC_STATUS_OK if configuration succeeded,
 *  SOPC_STATUS_INVALID_STATE if toolkit is not initialized, already
 *  configured or address space is already set, SOPC_STATUS_NOK otherwise
 */
SOPC_ReturnStatus SOPC_ToolkitServer_SetAddressSpaceNotifCb(SOPC_AddressSpaceNotif_Fct* pAddSpaceNotifFct);

/**
 *  \brief Record the given secure channel configuration in returned index
 *  (SOPC_ToolkitClient_Initialize required, !SOPC_Toolkit_Configured required)
 *
 *  \return secure channel configuration index if configuration succeeded,
 *  0 if toolkit is not initialized, already
 *  configured or otherwise
 */
uint32_t SOPC_ToolkitClient_AddSecureChannelConfig(SOPC_SecureChannel_Config* scConfig);

/**
 *  \brief Record the given secure channel configuration in given index
 *  (SOPC_Toolkit_Initialize required, !SOPC_Toolkit_Configured required)
 *  Note: it is forbidden to have 2 configurations with same endpointURL
 *
 *  \return endpoint configuration index configuration succeeded,
 *  0 if toolkit is not initialized, already
 *  configured or otherwise
 */
uint32_t SOPC_ToolkitServer_AddEndpointConfig(SOPC_Endpoint_Config* config);

/**
 * \brief Configure the toolkit log generation properties (SOPC_Toolkit_Initialize required,
 *        !SOPC_Toolkit_Configured required)
 *
 * \param logDirPath       Absolute or relative path of the directory to be used for logs
 *                        (full path shall exists or createDirectory flag shall be set,
 *                         path shall terminate with directory separator).
 *                         Default value is execution directory (same * as "" value provided).
 *
 * \param createDirectory  Flag indicating if the directory (last item of path regarding directory separator) shall be
 *                         created
 *
 * \warning The value of the pointer \p logDirPath is used afterwards by the Toolkit. The string is not copied. Hence,
 *          it must not be modified nor freed by the caller before SOPC_Toolkit_Clear.
 */
SOPC_ReturnStatus SOPC_ToolkitConfig_SetCircularLogPath(const char* logDirPath, bool createDirectory);

/**
 * \brief Configure the toolkit log generation properties (SOPC_Toolkit_Initialize required,
 *        !SOPC_Toolkit_Configured required)
 *
 * \param maxBytes      A maximum amount of bytes (> 100) by log file before opening a new file incrementing the integer
 *                      suffix. It is a best effort value (amount verified after each print). Default value is 1048576.
 *
 * \param maxFiles      A maximum number of files (> 0) to be used, when reached the older log file is overwritten
 *                      (starting with *_00001.log). Default value is 50.
 */
SOPC_ReturnStatus SOPC_ToolkitConfig_SetCircularLogProperties(uint32_t maxBytes, uint16_t maxFiles);

/**
 * \brief Configure the toolkit log traces level SOPC_TOOLKIT_LOG_LEVEL_ERROR/WARNING/INFO/DEBUG
 * (SOPC_Toolkit_Initialize required)
 *
 * \param level  Minimum level of log traces to be printed in the log files (default ERROR)
 *
 * \return       SOPC_STATUS_INVALID state if toolkit not initialized, SOPC_STATUS_OK otherwise
 */
SOPC_ReturnStatus SOPC_ToolkitConfig_SetLogLevel(SOPC_Toolkit_Log_Level level);

/**
 * \brief Get Toolkit build information
 *
 *
 * \return          Toolkit build information
 */
SOPC_Build_Info SOPC_ToolkitConfig_GetBuildInfo(void);

#endif /* SOPC_TOOLKIT_CONFIG_H_ */
