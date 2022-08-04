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

 File Name            : notification_republish_queue_it_bs.h

 Date                 : 04/08/2022 14:53:43

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _notification_republish_queue_it_bs_h
#define _notification_republish_queue_it_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "notification_republish_queue_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void notification_republish_queue_it_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void notification_republish_queue_it_bs__clear_notif_republish_iterator(
   const constants__t_notifRepublishQueue_i notification_republish_queue_it_bs__p_queue,
   const constants__t_notifRepublishQueueIterator_i notification_republish_queue_it_bs__p_iterator);
extern void notification_republish_queue_it_bs__continue_iter_notif_republish(
   const constants__t_notifRepublishQueue_i notification_republish_queue_it_bs__p_queue,
   const constants__t_notifRepublishQueueIterator_i notification_republish_queue_it_bs__p_iterator,
   t_bool * const notification_republish_queue_it_bs__continue,
   constants__t_sub_seq_num_i * const notification_republish_queue_it_bs__seq_num);
extern void notification_republish_queue_it_bs__get_available_republish(
   const constants__t_notifRepublishQueue_i notification_republish_queue_it_bs__p_queue,
   t_entier4 * const notification_republish_queue_it_bs__nb_seq_nums);
extern void notification_republish_queue_it_bs__init_iter_notif_republish(
   const constants__t_notifRepublishQueue_i notification_republish_queue_it_bs__p_queue,
   t_bool * const notification_republish_queue_it_bs__continue,
   constants__t_notifRepublishQueueIterator_i * const notification_republish_queue_it_bs__iterator);

#endif
