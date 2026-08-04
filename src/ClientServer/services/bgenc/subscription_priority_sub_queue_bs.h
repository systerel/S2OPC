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

 File Name            : subscription_priority_sub_queue_bs.h

 Date                 : 08/09/2026 15:25:26

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _subscription_priority_sub_queue_bs_h
#define _subscription_priority_sub_queue_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void subscription_priority_sub_queue_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void subscription_priority_sub_queue_bs__add_subscription_to_prio_queue(
   const constants__t_prioritySubQueue_i subscription_priority_sub_queue_bs__p_queue,
   const constants__t_subscription_i subscription_priority_sub_queue_bs__p_sub,
   t_bool * const subscription_priority_sub_queue_bs__bres);
extern void subscription_priority_sub_queue_bs__allocate_new_prio_queue(
   const constants__t_subscription_i subscription_priority_sub_queue_bs__p_first_sub,
   t_bool * const subscription_priority_sub_queue_bs__bres,
   constants__t_prioritySubQueue_i * const subscription_priority_sub_queue_bs__queue);
extern void subscription_priority_sub_queue_bs__clear_and_deallocate_prio_queue(
   const constants__t_prioritySubQueue_i subscription_priority_sub_queue_bs__p_queue);
extern void subscription_priority_sub_queue_bs__get_queue_nb_subs(
   const constants__t_prioritySubQueue_i subscription_priority_sub_queue_bs__p_queue,
   t_entier4 * const subscription_priority_sub_queue_bs__p_nb_subs);
extern void subscription_priority_sub_queue_bs__reconfigure_priority_subscriptions_queue_last_sub(
   const constants__t_prioritySubQueue_i subscription_priority_sub_queue_bs__p_queue,
   const constants__t_subscription_i subscription_priority_sub_queue_bs__p_sub);
extern void subscription_priority_sub_queue_bs__remove_subscription_from_queue(
   const constants__t_prioritySubQueue_i subscription_priority_sub_queue_bs__p_queue,
   const constants__t_subscription_i subscription_priority_sub_queue_bs__p_sub);

#endif
