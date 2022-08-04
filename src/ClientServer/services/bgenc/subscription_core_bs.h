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

 File Name            : subscription_core_bs.h

 Date                 : 04/08/2022 14:53:48

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _subscription_core_bs_h
#define _subscription_core_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "monitored_item_queue_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void subscription_core_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void subscription_core_bs__compute_create_subscription_revised_params(
   const constants__t_opcua_duration_i subscription_core_bs__p_reqPublishInterval,
   const t_entier4 subscription_core_bs__p_reqLifetimeCount,
   const t_entier4 subscription_core_bs__p_reqMaxKeepAlive,
   const t_entier4 subscription_core_bs__p_maxNotificationsPerPublish,
   constants__t_opcua_duration_i * const subscription_core_bs__revisedPublishInterval,
   t_entier4 * const subscription_core_bs__revisedLifetimeCount,
   t_entier4 * const subscription_core_bs__revisedMaxKeepAlive,
   t_entier4 * const subscription_core_bs__revisedMaxNotificationsPerPublish);
extern void subscription_core_bs__create_periodic_publish_timer(
   const constants__t_subscription_i subscription_core_bs__p_subscription,
   const constants__t_opcua_duration_i subscription_core_bs__p_publishInterval,
   t_bool * const subscription_core_bs__bres,
   constants__t_timer_id_i * const subscription_core_bs__timerId);
extern void subscription_core_bs__delete_publish_timer(
   const constants__t_timer_id_i subscription_core_bs__p_timer_id);
extern void subscription_core_bs__get_next_subscription_sequence_number(
   const constants__t_sub_seq_num_i subscription_core_bs__p_prev_seq_num,
   constants__t_sub_seq_num_i * const subscription_core_bs__p_next_seq_num);
extern void subscription_core_bs__get_nodeToMonitoredItemQueue(
   const constants__t_NodeId_i subscription_core_bs__p_nid,
   t_bool * const subscription_core_bs__p_bres,
   constants__t_monitoredItemQueue_i * const subscription_core_bs__p_monitoredItemQueue);
extern void subscription_core_bs__modify_publish_timer_period(
   const constants__t_timer_id_i subscription_core_bs__p_timerId,
   const constants__t_opcua_duration_i subscription_core_bs__p_revPublishInterval);
extern void subscription_core_bs__subscription_core_bs_UNINITIALISATION(void);

#endif
