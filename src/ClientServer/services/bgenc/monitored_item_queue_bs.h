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

 File Name            : monitored_item_queue_bs.h

 Date                 : 04/08/2022 14:53:34

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _monitored_item_queue_bs_h
#define _monitored_item_queue_bs_h

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
extern void monitored_item_queue_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void monitored_item_queue_bs__add_monitored_item_to_queue(
   const constants__t_monitoredItemQueue_i monitored_item_queue_bs__p_queue,
   const constants__t_monitoredItemPointer_i monitored_item_queue_bs__p_monitoredItem,
   t_bool * const monitored_item_queue_bs__bres);
extern void monitored_item_queue_bs__allocate_new_monitored_item_queue(
   t_bool * const monitored_item_queue_bs__bres,
   constants__t_monitoredItemQueue_i * const monitored_item_queue_bs__queue);
extern void monitored_item_queue_bs__clear_and_deallocate_monitored_item_queue(
   const constants__t_monitoredItemQueue_i monitored_item_queue_bs__p_queue);
extern void monitored_item_queue_bs__remove_monitored_item(
   const constants__t_monitoredItemQueue_i monitored_item_queue_bs__p_queue,
   const constants__t_monitoredItemPointer_i monitored_item_queue_bs__p_monitoredItem,
   t_bool * const monitored_item_queue_bs__bres);

#endif
