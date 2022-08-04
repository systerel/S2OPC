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

 File Name            : channel_mgr_1.c

 Date                 : 04/08/2022 14:53:05

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "channel_mgr_1.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
constants__t_timeref_i channel_mgr_1__a_channel_connected_time_i[constants__t_channel_i_max+1];
constants__t_channel_config_idx_i channel_mgr_1__a_config_i[constants__t_channel_i_max+1];
constants__t_channel_i channel_mgr_1__a_config_inv_i[constants__t_channel_config_idx_i_max+1];
constants__t_endpoint_config_idx_i channel_mgr_1__a_endpoint_i[constants__t_channel_i_max+1];
t_entier4 channel_mgr_1__card_channel_connected_i;
t_entier4 channel_mgr_1__card_cli_channel_connecting_i;
t_bool channel_mgr_1__s_channel_connected_i[constants__t_channel_i_max+1];
t_bool channel_mgr_1__s_cli_channel_connecting_i[constants__t_channel_config_idx_i_max+1];
t_bool channel_mgr_1__s_cli_channel_disconnecting_i[constants__t_channel_config_idx_i_max+1];

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void channel_mgr_1__INITIALISATION(void) {
   {
      t_entier4 i;
      for (i = constants__t_channel_config_idx_i_max; 0 <= i; i = i - 1) {
         channel_mgr_1__s_cli_channel_connecting_i[i] = false;
      }
   }
   channel_mgr_1__card_cli_channel_connecting_i = 0;
   {
      t_entier4 i;
      for (i = constants__t_channel_config_idx_i_max; 0 <= i; i = i - 1) {
         channel_mgr_1__s_cli_channel_disconnecting_i[i] = false;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_channel_i_max; 0 <= i; i = i - 1) {
         channel_mgr_1__s_channel_connected_i[i] = false;
      }
   }
   channel_mgr_1__card_channel_connected_i = 0;
   {
      t_entier4 i;
      for (i = constants__t_channel_i_max; 0 <= i; i = i - 1) {
         channel_mgr_1__a_channel_connected_time_i[i] = constants__c_timeref_indet;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_channel_i_max; 0 <= i; i = i - 1) {
         channel_mgr_1__a_config_i[i] = constants__c_channel_config_idx_indet;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_channel_config_idx_i_max; 0 <= i; i = i - 1) {
         channel_mgr_1__a_config_inv_i[i] = constants__c_channel_indet;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_channel_i_max; 0 <= i; i = i - 1) {
         channel_mgr_1__a_endpoint_i[i] = constants__c_endpoint_config_idx_indet;
      }
   }
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void channel_mgr_1__is_connected_channel(
   const constants__t_channel_i channel_mgr_1__channel,
   t_bool * const channel_mgr_1__bres) {
   *channel_mgr_1__bres = channel_mgr_1__s_channel_connected_i[channel_mgr_1__channel];
}

void channel_mgr_1__is_disconnecting_channel(
   const constants__t_channel_config_idx_i channel_mgr_1__config_idx,
   t_bool * const channel_mgr_1__bres) {
   *channel_mgr_1__bres = channel_mgr_1__s_cli_channel_disconnecting_i[channel_mgr_1__config_idx];
}

void channel_mgr_1__get_connected_channel(
   const constants__t_channel_config_idx_i channel_mgr_1__config_idx,
   constants__t_channel_i * const channel_mgr_1__channel) {
   {
      t_bool channel_mgr_1__l_res;
      
      *channel_mgr_1__channel = channel_mgr_1__a_config_inv_i[channel_mgr_1__config_idx];
      constants__is_t_channel(*channel_mgr_1__channel,
         &channel_mgr_1__l_res);
      if (channel_mgr_1__l_res == false) {
         *channel_mgr_1__channel = constants__c_channel_indet;
      }
   }
}

void channel_mgr_1__get_channel_info(
   const constants__t_channel_i channel_mgr_1__channel,
   constants__t_channel_config_idx_i * const channel_mgr_1__config_idx) {
   {
      t_bool channel_mgr_1__l_res;
      
      *channel_mgr_1__config_idx = channel_mgr_1__a_config_i[channel_mgr_1__channel];
      constants__is_t_channel_config_idx(*channel_mgr_1__config_idx,
         &channel_mgr_1__l_res);
      if (channel_mgr_1__l_res == false) {
         *channel_mgr_1__config_idx = constants__c_channel_config_idx_indet;
      }
   }
}

void channel_mgr_1__is_client_channel(
   const constants__t_channel_i channel_mgr_1__channel,
   t_bool * const channel_mgr_1__bres) {
   {
      constants__t_endpoint_config_idx_i channel_mgr_1__l_endpoint;
      t_bool channel_mgr_1__l_bres;
      
      channel_mgr_1__l_endpoint = channel_mgr_1__a_endpoint_i[channel_mgr_1__channel];
      constants__is_t_endpoint_config_idx(channel_mgr_1__l_endpoint,
         &channel_mgr_1__l_bres);
      if (channel_mgr_1__l_bres == true) {
         *channel_mgr_1__bres = false;
      }
      else {
         *channel_mgr_1__bres = true;
      }
   }
}

void channel_mgr_1__server_get_endpoint_config(
   const constants__t_channel_i channel_mgr_1__channel,
   constants__t_endpoint_config_idx_i * const channel_mgr_1__endpoint_config_idx) {
   {
      t_bool channel_mgr_1__l_res;
      
      *channel_mgr_1__endpoint_config_idx = channel_mgr_1__a_endpoint_i[channel_mgr_1__channel];
      constants__is_t_endpoint_config_idx(*channel_mgr_1__endpoint_config_idx,
         &channel_mgr_1__l_res);
      if (channel_mgr_1__l_res == false) {
         *channel_mgr_1__endpoint_config_idx = constants__c_endpoint_config_idx_indet;
      }
   }
}

void channel_mgr_1__getall_config_inv(
   const constants__t_channel_config_idx_i channel_mgr_1__p_config_idx,
   t_bool * const channel_mgr_1__p_dom,
   constants__t_channel_i * const channel_mgr_1__p_channel) {
   *channel_mgr_1__p_channel = channel_mgr_1__a_config_inv_i[channel_mgr_1__p_config_idx];
   constants__is_t_channel(*channel_mgr_1__p_channel,
      channel_mgr_1__p_dom);
   if (*channel_mgr_1__p_dom == false) {
      *channel_mgr_1__p_channel = constants__c_channel_indet;
   }
}

void channel_mgr_1__get_card_cli_channel_connecting(
   t_entier4 * const channel_mgr_1__p_card_connecting) {
   *channel_mgr_1__p_card_connecting = channel_mgr_1__card_cli_channel_connecting_i;
}

void channel_mgr_1__get_card_channel_connected(
   t_entier4 * const channel_mgr_1__p_card_connected) {
   *channel_mgr_1__p_card_connected = channel_mgr_1__card_channel_connected_i;
}

void channel_mgr_1__get_card_channel_used(
   t_entier4 * const channel_mgr_1__p_card_used) {
   *channel_mgr_1__p_card_used = channel_mgr_1__card_cli_channel_connecting_i +
      channel_mgr_1__card_channel_connected_i;
}

void channel_mgr_1__add_cli_channel_connecting(
   const constants__t_channel_config_idx_i channel_mgr_1__p_config_idx) {
   {
      t_bool channel_mgr_1__l_res;
      
      channel_mgr_1__l_res = channel_mgr_1__s_cli_channel_connecting_i[channel_mgr_1__p_config_idx];
      if (channel_mgr_1__l_res == false) {
         channel_mgr_1__s_cli_channel_connecting_i[channel_mgr_1__p_config_idx] = true;
         channel_mgr_1__card_cli_channel_connecting_i = channel_mgr_1__card_cli_channel_connecting_i +
            1;
      }
   }
}

void channel_mgr_1__is_channel_connected(
   const constants__t_channel_i channel_mgr_1__p_channel,
   t_bool * const channel_mgr_1__p_con) {
   *channel_mgr_1__p_con = channel_mgr_1__s_channel_connected_i[channel_mgr_1__p_channel];
}

void channel_mgr_1__add_channel_connected(
   const constants__t_channel_i channel_mgr_1__p_channel,
   const constants__t_timeref_i channel_mgr_1__p_timeref) {
   channel_mgr_1__s_channel_connected_i[channel_mgr_1__p_channel] = true;
   channel_mgr_1__card_channel_connected_i = channel_mgr_1__card_channel_connected_i +
      1;
   channel_mgr_1__a_channel_connected_time_i[channel_mgr_1__p_channel] = channel_mgr_1__p_timeref;
}

void channel_mgr_1__set_config(
   const constants__t_channel_i channel_mgr_1__p_channel,
   const constants__t_channel_config_idx_i channel_mgr_1__p_channel_config_idx) {
   channel_mgr_1__a_config_i[channel_mgr_1__p_channel] = channel_mgr_1__p_channel_config_idx;
   channel_mgr_1__a_config_inv_i[channel_mgr_1__p_channel_config_idx] = channel_mgr_1__p_channel;
}

void channel_mgr_1__set_endpoint(
   const constants__t_channel_i channel_mgr_1__p_channel,
   const constants__t_endpoint_config_idx_i channel_mgr_1__p_endpoint_config_idx) {
   channel_mgr_1__a_endpoint_i[channel_mgr_1__p_channel] = channel_mgr_1__p_endpoint_config_idx;
}

void channel_mgr_1__getall_channel_connected(
   const constants__t_channel_i channel_mgr_1__p_channel,
   t_bool * const channel_mgr_1__p_dom,
   constants__t_channel_config_idx_i * const channel_mgr_1__p_config_idx) {
   *channel_mgr_1__p_config_idx = channel_mgr_1__a_config_i[channel_mgr_1__p_channel];
   constants__is_t_channel_config_idx(*channel_mgr_1__p_config_idx,
      channel_mgr_1__p_dom);
   if (*channel_mgr_1__p_dom == false) {
      *channel_mgr_1__p_config_idx = constants__c_channel_config_idx_indet;
   }
}

void channel_mgr_1__is_cli_channel_connecting(
   const constants__t_channel_config_idx_i channel_mgr_1__p_config_idx,
   t_bool * const channel_mgr_1__p_is_channel_connecting) {
   *channel_mgr_1__p_is_channel_connecting = channel_mgr_1__s_cli_channel_connecting_i[channel_mgr_1__p_config_idx];
}

void channel_mgr_1__add_cli_channel_disconnecting(
   const constants__t_channel_config_idx_i channel_mgr_1__p_config_idx) {
   channel_mgr_1__s_cli_channel_disconnecting_i[channel_mgr_1__p_config_idx] = true;
}

void channel_mgr_1__remove_channel_connected(
   const constants__t_channel_i channel_mgr_1__p_channel) {
   {
      t_bool channel_mgr_1__l_res;
      
      channel_mgr_1__l_res = channel_mgr_1__s_channel_connected_i[channel_mgr_1__p_channel];
      if (channel_mgr_1__l_res == true) {
         channel_mgr_1__s_channel_connected_i[channel_mgr_1__p_channel] = false;
         channel_mgr_1__card_channel_connected_i = channel_mgr_1__card_channel_connected_i -
            1;
         channel_mgr_1__a_channel_connected_time_i[channel_mgr_1__p_channel] = constants__c_timeref_indet;
      }
   }
}

void channel_mgr_1__remove_cli_channel_disconnecting(
   const constants__t_channel_config_idx_i channel_mgr_1__p_config_idx) {
   channel_mgr_1__s_cli_channel_disconnecting_i[channel_mgr_1__p_config_idx] = false;
}

void channel_mgr_1__reset_config(
   const constants__t_channel_i channel_mgr_1__p_channel) {
   {
      constants__t_channel_config_idx_i channel_mgr_1__l_config_idx;
      
      channel_mgr_1__l_config_idx = channel_mgr_1__a_config_i[channel_mgr_1__p_channel];
      channel_mgr_1__a_config_inv_i[channel_mgr_1__l_config_idx] = constants__c_channel_indet;
      channel_mgr_1__a_config_i[channel_mgr_1__p_channel] = constants__c_channel_config_idx_indet;
   }
}

void channel_mgr_1__reset_endpoint(
   const constants__t_channel_i channel_mgr_1__p_channel) {
   channel_mgr_1__a_endpoint_i[channel_mgr_1__p_channel] = constants__c_endpoint_config_idx_indet;
}

void channel_mgr_1__remove_cli_channel_connecting(
   const constants__t_channel_config_idx_i channel_mgr_1__p_config_idx) {
   {
      t_bool channel_mgr_1__l_res;
      
      channel_mgr_1__l_res = channel_mgr_1__s_cli_channel_connecting_i[channel_mgr_1__p_config_idx];
      if (channel_mgr_1__l_res == true) {
         channel_mgr_1__s_cli_channel_connecting_i[channel_mgr_1__p_config_idx] = false;
         channel_mgr_1__card_cli_channel_connecting_i = channel_mgr_1__card_cli_channel_connecting_i -
            1;
      }
   }
}

void channel_mgr_1__get_connection_time(
   const constants__t_channel_i channel_mgr_1__p_channel,
   constants__t_timeref_i * const channel_mgr_1__p_timeref) {
   *channel_mgr_1__p_timeref = channel_mgr_1__a_channel_connected_time_i[channel_mgr_1__p_channel];
}

