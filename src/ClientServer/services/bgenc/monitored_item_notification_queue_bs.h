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

 File Name            : monitored_item_notification_queue_bs.h

 Date                 : 04/08/2022 14:53:33

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _monitored_item_notification_queue_bs_h
#define _monitored_item_notification_queue_bs_h

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
extern void monitored_item_notification_queue_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void monitored_item_notification_queue_bs__add_first_monitored_item_notification_to_queue(
   const constants__t_notificationQueue_i monitored_item_notification_queue_bs__p_queue,
   const constants__t_monitoredItemPointer_i monitored_item_notification_queue_bs__p_monitoredItem,
   const constants__t_NodeId_i monitored_item_notification_queue_bs__p_nid,
   const constants__t_AttributeId_i monitored_item_notification_queue_bs__p_aid,
   const constants__t_Variant_i monitored_item_notification_queue_bs__p_VariantValuePointer,
   const constants__t_RawStatusCode monitored_item_notification_queue_bs__p_ValueSc,
   const constants__t_Timestamp monitored_item_notification_queue_bs__p_val_ts_src,
   const constants__t_Timestamp monitored_item_notification_queue_bs__p_val_ts_srv,
   t_bool * const monitored_item_notification_queue_bs__bres);
extern void monitored_item_notification_queue_bs__add_monitored_item_notification_to_queue(
   const constants__t_notificationQueue_i monitored_item_notification_queue_bs__p_queue,
   const constants__t_monitoredItemPointer_i monitored_item_notification_queue_bs__p_monitoredItem,
   const constants__t_TimestampsToReturn_i monitored_item_notification_queue_bs__p_timestampToReturn,
   const constants__t_WriteValuePointer_i monitored_item_notification_queue_bs__p_writeValuePointer,
   t_bool * const monitored_item_notification_queue_bs__bres);
extern void monitored_item_notification_queue_bs__allocate_new_monitored_item_notification_queue(
   t_bool * const monitored_item_notification_queue_bs__bres,
   constants__t_notificationQueue_i * const monitored_item_notification_queue_bs__queue);
extern void monitored_item_notification_queue_bs__clear_and_deallocate_monitored_item_notification_queue(
   const constants__t_notificationQueue_i monitored_item_notification_queue_bs__p_queue);
extern void monitored_item_notification_queue_bs__continue_pop_iter_monitor_item_notification(
   const constants__t_notificationQueue_i monitored_item_notification_queue_bs__p_queue,
   t_bool * const monitored_item_notification_queue_bs__p_continue,
   constants__t_monitoredItemPointer_i * const monitored_item_notification_queue_bs__p_monitoredItem,
   constants__t_WriteValuePointer_i * const monitored_item_notification_queue_bs__p_writeValuePointer);
extern void monitored_item_notification_queue_bs__free_first_monitored_item_notification_value(
   const constants__t_Variant_i monitored_item_notification_queue_bs__p_VariantValuePointer);
extern void monitored_item_notification_queue_bs__init_iter_monitored_item_notification(
   const constants__t_notificationQueue_i monitored_item_notification_queue_bs__p_queue,
   t_entier4 * const monitored_item_notification_queue_bs__p_nb_notifications);

#endif
