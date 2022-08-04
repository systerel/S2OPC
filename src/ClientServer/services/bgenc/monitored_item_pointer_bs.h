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

/******************************************************************************

 File Name            : monitored_item_pointer_bs.h

 Date                 : 04/08/2022 14:53:33

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _monitored_item_pointer_bs_h
#define _monitored_item_pointer_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "constants_statuscodes_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void monitored_item_pointer_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void monitored_item_pointer_bs__create_monitored_item_pointer(
   const constants__t_subscription_i monitored_item_pointer_bs__p_subscription,
   const constants__t_NodeId_i monitored_item_pointer_bs__p_nid,
   const constants__t_AttributeId_i monitored_item_pointer_bs__p_aid,
   const constants__t_IndexRange_i monitored_item_pointer_bs__p_indexRange,
   const constants__t_TimestampsToReturn_i monitored_item_pointer_bs__p_timestampToReturn,
   const constants__t_monitoringMode_i monitored_item_pointer_bs__p_monitoringMode,
   const constants__t_client_handle_i monitored_item_pointer_bs__p_clientHandle,
   constants_statuscodes_bs__t_StatusCode_i * const monitored_item_pointer_bs__StatusCode,
   constants__t_monitoredItemPointer_i * const monitored_item_pointer_bs__monitoredItemPointer,
   constants__t_monitoredItemId_i * const monitored_item_pointer_bs__monitoredItemId);
extern void monitored_item_pointer_bs__delete_monitored_item_pointer(
   const constants__t_monitoredItemPointer_i monitored_item_pointer_bs__p_monitoredItemPointer);
extern void monitored_item_pointer_bs__getall_monitoredItemPointer(
   const constants__t_monitoredItemPointer_i monitored_item_pointer_bs__p_monitoredItemPointer,
   constants__t_monitoredItemId_i * const monitored_item_pointer_bs__p_monitoredItemId,
   constants__t_subscription_i * const monitored_item_pointer_bs__p_subscription,
   constants__t_NodeId_i * const monitored_item_pointer_bs__p_nid,
   constants__t_AttributeId_i * const monitored_item_pointer_bs__p_aid,
   constants__t_TimestampsToReturn_i * const monitored_item_pointer_bs__p_timestampToReturn,
   constants__t_monitoringMode_i * const monitored_item_pointer_bs__p_monitoringMode,
   constants__t_client_handle_i * const monitored_item_pointer_bs__p_clientHandle);
extern void monitored_item_pointer_bs__is_notification_triggered(
   const constants__t_monitoredItemPointer_i monitored_item_pointer_bs__p_monitoredItemPointer,
   const constants__t_WriteValuePointer_i monitored_item_pointer_bs__p_old_wv_pointer,
   const constants__t_WriteValuePointer_i monitored_item_pointer_bs__p_new_wv_pointer,
   t_bool * const monitored_item_pointer_bs__bres);
extern void monitored_item_pointer_bs__monitored_item_pointer_bs_UNINITIALISATION(void);

#endif
