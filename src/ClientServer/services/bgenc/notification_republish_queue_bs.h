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

 File Name            : notification_republish_queue_bs.h

 Date                 : 04/08/2022 14:53:43

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _notification_republish_queue_bs_h
#define _notification_republish_queue_bs_h

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
extern void notification_republish_queue_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void notification_republish_queue_bs__add_republish_notif_to_queue(
   const constants__t_notifRepublishQueue_i notification_republish_queue_bs__p_queue,
   const constants__t_sub_seq_num_i notification_republish_queue_bs__p_seq_num,
   const constants__t_notif_msg_i notification_republish_queue_bs__p_notif_msg,
   t_bool * const notification_republish_queue_bs__bres);
extern void notification_republish_queue_bs__allocate_new_republish_queue(
   t_bool * const notification_republish_queue_bs__bres,
   constants__t_notifRepublishQueue_i * const notification_republish_queue_bs__queue);
extern void notification_republish_queue_bs__clear_and_deallocate_republish_queue(
   const constants__t_notifRepublishQueue_i notification_republish_queue_bs__p_queue);
extern void notification_republish_queue_bs__clear_republish_queue(
   const constants__t_notifRepublishQueue_i notification_republish_queue_bs__p_queue);
extern void notification_republish_queue_bs__discard_oldest_republish_notif(
   const constants__t_notifRepublishQueue_i notification_republish_queue_bs__p_queue);
extern void notification_republish_queue_bs__get_nb_republish_notifs(
   const constants__t_notifRepublishQueue_i notification_republish_queue_bs__p_queue,
   t_entier4 * const notification_republish_queue_bs__nb_notifs);
extern void notification_republish_queue_bs__get_republish_notif_from_queue(
   const constants__t_notifRepublishQueue_i notification_republish_queue_bs__p_queue,
   const constants__t_sub_seq_num_i notification_republish_queue_bs__p_seq_num,
   t_bool * const notification_republish_queue_bs__bres,
   constants__t_notif_msg_i * const notification_republish_queue_bs__p_notif_msg);
extern void notification_republish_queue_bs__remove_republish_notif_from_queue(
   const constants__t_notifRepublishQueue_i notification_republish_queue_bs__p_queue,
   const constants__t_sub_seq_num_i notification_republish_queue_bs__p_seq_num,
   t_bool * const notification_republish_queue_bs__bres);

#endif
