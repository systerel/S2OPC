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
 * \brief Helpers for the Toolkit API.
 *
 */

#ifndef TOOLKIT_HELPERS_H_
#define TOOLKIT_HELPERS_H_

#define MAX_NOTIFICATIONS_PER_REQUEST 1000

#include "sopc_key_manager.h"
#include "sopc_toolkit_constants.h"
#include "sopc_types.h"
#include "sopc_user_app_itf.h"

/* The following includes are required to fetch the SOPC_LibSub_Value, SOPC_LibSub_DataTime and logger types */
#include "sopc_builtintypes.h"
#include "sopc_crypto_profiles.h"
#include "sopc_log_manager.h"
#include "sopc_toolkit_constants.h"
#include "sopc_types.h"
#define SKIP_S2OPC_DEFINITIONS
#include "libs2opc_client.h"

/**
 * \brief Creates a new Toolkit secure channel configuration from elements of the SOPC_LibSub_ConnectionCfg.
 */
SOPC_ReturnStatus Helpers_NewSCConfigFromLibSubCfg(const char* szServerUrl,
                                                   const char* szSecuPolicy,
                                                   OpcUa_MessageSecurityMode msgSecurityMode,
                                                   const char* szPathCertifAuth,
                                                   const char* szPathCertServer,
                                                   const char* szPathCertClient,
                                                   const char* szPathKeyClient,
                                                   const char* szPathCrl,
                                                   uint32_t iScRequestedLifetime,
                                                   SOPC_SecureChannel_Config** ppNewCfg);

/**
 * \brief Creates a new CreateSubscriptionRequest.
 */
SOPC_ReturnStatus Helpers_NewCreateSubscriptionRequest(double fPublishIntervalMs,
                                                       uint32_t iCntLifetime,
                                                       uint32_t iCntMaxKeepAlive,
                                                       void** ppRequest);

/**
 * \brief Creates a new PublishRequest.
 */
SOPC_ReturnStatus Helpers_NewPublishRequest(bool bAck, uint32_t iSubId, uint32_t iSeqNum, void** ppRequest);

/**
 * \brief Creates a new CreateMonitoredItemsRequest with a single ItemToCreate.
 */
SOPC_ReturnStatus Helpers_NewCreateMonitoredItemsRequest(SOPC_NodeId* pNid,
                                                         uint32_t iAttrId,
                                                         uint32_t iSubId,
                                                         OpcUa_TimestampsToReturn tsToReturn,
                                                         uint32_t iCliHndl,
                                                         uint32_t iQueueSize,
                                                         void** ppRequest);

/**
 * \brief Converts a SOPC_DataValue to a SOPC_LibSub_Value, returns NULL when the conversion is not possible
 *        (or on memory allocation failure, or when an uint64_t is greater than the INT64_MAX).
 *
 * Does not handle arrays.
 */
SOPC_ReturnStatus Helpers_NewValueFromDataValue(SOPC_DataValue* pVal, SOPC_LibSub_Value** pplsVal);

/**
 * \brief OPC-UA time (hundreds of nanosecs since 1601/01/01 00:00:00 UTC) to NTP time
 *        (2**-32 seconds since 1900/01/01 00:00:00 UTC).
 */
SOPC_LibSub_Timestamp Helpers_OPCTimeToNTP(SOPC_DateTime ts);

/**
 * \brief Buffers a log message, then calls the callback configured with the LibSub.
 *
 * See Helpers_SetLogger().
 *
 */
void Helpers_Log(const SOPC_Log_Level log_level, const char* format, ...);

/**
 * \brief Sets the \p cbk function as the called callback by Helpers_Log().
 *
 * The default log function outputs to the standard output.
 */
void Helpers_SetLogger(SOPC_LibSub_LogCbk cbk);

/**
 * \brief Prints a log message to stdout, with the following format "# log_level: text\n".
 */
void Helpers_LoggerStdout(const SOPC_Log_Level log_level, const SOPC_LibSub_CstString text);

#endif /* TOOLKIT_HELPERS_H_ */
