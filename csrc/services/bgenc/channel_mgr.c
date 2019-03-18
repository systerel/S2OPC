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

 File Name            : channel_mgr.c

 Date                 : 29/01/2019 09:56:37

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "channel_mgr.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
t_bool channel_mgr__all_channel_closing;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void channel_mgr__INITIALISATION(void) {
   channel_mgr__all_channel_closing = false;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void channel_mgr__l_close_secure_channel(
   const constants__t_channel_i channel_mgr__p_channel) {
   {
      t_bool channel_mgr__l_res;
      constants__t_channel_config_idx_i channel_mgr__l_channel_conf;
      
      channel_mgr_1__getall_channel_connected(channel_mgr__p_channel,
         &channel_mgr__l_res,
         &channel_mgr__l_channel_conf);
      if (channel_mgr__l_res == true) {
         channel_mgr_1__add_cli_channel_disconnecting(channel_mgr__l_channel_conf);
         channel_mgr_bs__finalize_close_secure_channel(channel_mgr__p_channel);
      }
   }
}

void channel_mgr__l_check_all_channel_lost(void) {
   {
      t_bool channel_mgr__l_con;
      t_bool channel_mgr__l_continue;
      constants__t_channel_i channel_mgr__l_channel;
      
      if (channel_mgr__all_channel_closing == true) {
         channel_mgr__l_con = false;
         channel_mgr_it__init_iter_channel(&channel_mgr__l_continue);
         while ((channel_mgr__l_continue == true) &&
            (channel_mgr__l_con == false)) {
            channel_mgr_it__continue_iter_channel(&channel_mgr__l_continue,
               &channel_mgr__l_channel);
            channel_mgr_1__is_channel_connected(channel_mgr__l_channel,
               &channel_mgr__l_con);
         }
         if (channel_mgr__l_con == false) {
            channel_mgr_bs__last_connected_channel_lost();
         }
      }
   }
}

void channel_mgr__cli_open_secure_channel(
   const constants__t_channel_config_idx_i channel_mgr__config_idx,
   t_bool * const channel_mgr__bres) {
   {
      t_bool channel_mgr__l_already_connecting;
      t_bool channel_mgr__l_dom;
      constants__t_channel_i channel_mgr__l_channel;
      t_entier4 channel_mgr__l_card_connecting;
      t_entier4 channel_mgr__l_card_connected;
      t_entier4 channel_mgr__l_sum;
      t_entier4 channel_mgr__l_card_channel;
      
      channel_mgr_1__is_cli_channel_connecting(channel_mgr__config_idx,
         &channel_mgr__l_already_connecting);
      if (channel_mgr__l_already_connecting == true) {
         *channel_mgr__bres = true;
      }
      else {
         channel_mgr_1__getall_config_inv(channel_mgr__config_idx,
            &channel_mgr__l_dom,
            &channel_mgr__l_channel);
         if (channel_mgr__l_dom == false) {
            channel_mgr_1__get_card_cli_channel_connecting(&channel_mgr__l_card_connecting);
            channel_mgr_1__get_card_channel_connected(&channel_mgr__l_card_connected);
            channel_mgr__l_sum = channel_mgr__l_card_connecting +
               channel_mgr__l_card_connected;
            constants__get_card_t_channel(&channel_mgr__l_card_channel);
            if (channel_mgr__l_sum != channel_mgr__l_card_channel) {
               channel_mgr_1__add_cli_channel_connecting(channel_mgr__config_idx);
               channel_mgr_bs__prepare_cli_open_secure_channel(channel_mgr__config_idx);
               *channel_mgr__bres = true;
            }
            else {
               *channel_mgr__bres = false;
            }
         }
         else {
            *channel_mgr__bres = false;
         }
      }
   }
}

void channel_mgr__srv_new_secure_channel(
   const constants__t_endpoint_config_idx_i channel_mgr__endpoint_config_idx,
   const constants__t_channel_config_idx_i channel_mgr__channel_config_idx,
   const constants__t_channel_i channel_mgr__channel,
   t_bool * const channel_mgr__bres) {
   {
      t_bool channel_mgr__l_con;
      t_bool channel_mgr__l_dom;
      constants__t_channel_i channel_mgr__l_channel;
      
      channel_mgr_1__is_channel_connected(channel_mgr__channel,
         &channel_mgr__l_con);
      channel_mgr_1__getall_config_inv(channel_mgr__channel_config_idx,
         &channel_mgr__l_dom,
         &channel_mgr__l_channel);
      if ((channel_mgr__l_con == false) &&
         (channel_mgr__l_dom == false)) {
         channel_mgr_1__add_channel_connected(channel_mgr__channel);
         channel_mgr_1__set_config(channel_mgr__channel,
            channel_mgr__channel_config_idx);
         channel_mgr_1__set_endpoint(channel_mgr__channel,
            channel_mgr__endpoint_config_idx);
         channel_mgr_bs__define_SecurityPolicy(channel_mgr__channel);
         *channel_mgr__bres = true;
      }
      else {
         *channel_mgr__bres = false;
      }
   }
}

void channel_mgr__close_secure_channel(
   const constants__t_channel_i channel_mgr__channel) {
   channel_mgr__l_close_secure_channel(channel_mgr__channel);
}

void channel_mgr__close_all_channel(
   t_bool * const channel_mgr__bres) {
   {
      t_bool channel_mgr__l_continue;
      constants__t_channel_i channel_mgr__l_channel;
      t_bool channel_mgr__l_con;
      
      channel_mgr__all_channel_closing = false;
      channel_mgr_it__init_iter_channel(&channel_mgr__l_continue);
      while (channel_mgr__l_continue == true) {
         channel_mgr_it__continue_iter_channel(&channel_mgr__l_continue,
            &channel_mgr__l_channel);
         channel_mgr_1__is_channel_connected(channel_mgr__l_channel,
            &channel_mgr__l_con);
         if (channel_mgr__l_con == true) {
            channel_mgr__all_channel_closing = true;
            channel_mgr__l_close_secure_channel(channel_mgr__l_channel);
         }
      }
      *channel_mgr__bres = channel_mgr__all_channel_closing;
   }
}

void channel_mgr__channel_lost(
   const constants__t_channel_i channel_mgr__channel) {
   {
      t_bool channel_mgr__l_res;
      constants__t_channel_config_idx_i channel_mgr__l_channel_conf;
      
      channel_mgr_1__getall_channel_connected(channel_mgr__channel,
         &channel_mgr__l_res,
         &channel_mgr__l_channel_conf);
      if (channel_mgr__l_res == true) {
         channel_mgr_1__remove_channel_connected(channel_mgr__channel);
         channel_mgr_1__remove_cli_channel_disconnecting(channel_mgr__l_channel_conf);
         channel_mgr_1__reset_config(channel_mgr__channel);
         channel_mgr_1__reset_endpoint(channel_mgr__channel);
         channel_mgr_bs__reset_SecurityPolicy(channel_mgr__channel);
         channel_mgr__l_check_all_channel_lost();
      }
   }
}

void channel_mgr__cli_set_connected_channel(
   const constants__t_channel_config_idx_i channel_mgr__config_idx,
   const constants__t_channel_i channel_mgr__channel,
   t_bool * const channel_mgr__bres) {
   {
      t_bool channel_mgr__l_is_channel_connecting;
      t_bool channel_mgr__l_is_channel_connected;
      
      channel_mgr_1__is_cli_channel_connecting(channel_mgr__config_idx,
         &channel_mgr__l_is_channel_connecting);
      channel_mgr_1__is_channel_connected(channel_mgr__channel,
         &channel_mgr__l_is_channel_connected);
      if ((channel_mgr__l_is_channel_connecting == true) &&
         (channel_mgr__l_is_channel_connected == false)) {
         channel_mgr_1__remove_cli_channel_connecting(channel_mgr__config_idx);
         channel_mgr_1__add_channel_connected(channel_mgr__channel);
         channel_mgr_1__set_config(channel_mgr__channel,
            channel_mgr__config_idx);
         channel_mgr_bs__define_SecurityPolicy(channel_mgr__channel);
         *channel_mgr__bres = true;
      }
      else {
         *channel_mgr__bres = false;
      }
   }
}

void channel_mgr__cli_set_connection_timeout_channel(
   const constants__t_channel_config_idx_i channel_mgr__config_idx,
   t_bool * const channel_mgr__bres) {
   {
      t_bool channel_mgr__l_res;
      
      channel_mgr_1__is_cli_channel_connecting(channel_mgr__config_idx,
         &channel_mgr__l_res);
      if (channel_mgr__l_res == true) {
         channel_mgr_1__remove_cli_channel_connecting(channel_mgr__config_idx);
         *channel_mgr__bres = true;
      }
      else {
         *channel_mgr__bres = false;
      }
   }
}

