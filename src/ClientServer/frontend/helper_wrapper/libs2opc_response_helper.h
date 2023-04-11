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
 * \brief High level interface to help to extract data from OPC UA message response
 */

#ifndef LIBS2OPC_RESPONSE_HELPER_H_
#define LIBS2OPC_RESPONSE_HELPER_H_

#include "sopc_types.h"

/**
 * \brief Returns the DataChangeNotification if one is present in the PublishResponse message.
 *
 * \param pubResponse  The PublishResponse that may contain a DataChangeNotification message
 *
 * \return The DataChangeNotification present in the PublishResponse message or NULL if none is present.
 */
const OpcUa_DataChangeNotification* SOPC_PublishResponse_GetDataChangeNotification(
    const OpcUa_PublishResponse* pubResponse);

/**
 * \brief Returns the EventNotificationList if one is present in the PublishResponse message.
 *
 * \param pubResponse  The PublishResponse that may contain a EventNotificationList message
 *
 * \return The EventNotificationList present in the PublishResponse message or NULL if none is present.
 */
const OpcUa_EventNotificationList* SOPC_PublishResponse_GetEventNotificationList(
    const OpcUa_PublishResponse* pubResponse);

#endif /* LIBS2OPC_RESPONSE_HELPER_H_ */
