/*
 *  Copyright (C) 2018 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

#endif /* TOOLKIT_HELPERS_H_ */
