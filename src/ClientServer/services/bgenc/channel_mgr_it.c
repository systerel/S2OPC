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

 File Name            : channel_mgr_it.c

 Date                 : 04/08/2022 14:53:06

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "channel_mgr_it.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
t_entier4 channel_mgr_it__channel_i;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void channel_mgr_it__INITIALISATION(void) {
   channel_mgr_it__channel_i = 0;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void channel_mgr_it__init_iter_channel(
   t_bool * const channel_mgr_it__p_continue) {
   constants__get_card_t_channel(&channel_mgr_it__channel_i);
   *channel_mgr_it__p_continue = (1 <= channel_mgr_it__channel_i);
}

void channel_mgr_it__continue_iter_channel(
   t_bool * const channel_mgr_it__p_continue,
   constants__t_channel_i * const channel_mgr_it__p_channel) {
   constants__get_cast_t_channel(channel_mgr_it__channel_i,
      channel_mgr_it__p_channel);
   channel_mgr_it__channel_i = channel_mgr_it__channel_i -
      1;
   *channel_mgr_it__p_continue = (1 <= channel_mgr_it__channel_i);
}

