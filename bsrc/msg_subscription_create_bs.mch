/*
 *  Copyright (C) 2018 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

MACHINE
    msg_subscription_create_bs

SEES
    constants,
    message_in_bs,
    message_out_bs
    
OPERATIONS
   
    reqPublishInterval, reqLifetimeCount, reqMaxKeepAlive, maxNotificationsPerPublish, publishEnabled
      <-- get_msg_create_subscription_req_params (p_req_msg) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_subscription_create_req
    THEN
        reqPublishInterval,
        reqLifetimeCount,
        reqMaxKeepAlive,
        maxNotificationsPerPublish,
        publishEnabled
        :(reqPublishInterval : t_opcua_duration_i &
          reqLifetimeCount : t_counter &
          reqMaxKeepAlive : t_counter &
          maxNotificationsPerPublish : t_counter &
          publishEnabled : BOOL)
    END
    ;
  
    set_msg_create_subscription_resp_params (p_resp_msg,
                                             p_subscription, 
                                             p_revisedPublishInterval,
                                             p_revisedLifetimeCount,
                                             p_revisedMaxKeepAlive) =
    PRE
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_subscription_create_resp &
        p_subscription : t_subscription_i &
        p_subscription : t_subscription &
        p_revisedPublishInterval : t_opcua_duration_i &
        p_revisedLifetimeCount : t_counter &
        p_revisedMaxKeepAlive : t_counter &
        p_revisedLifetimeCount >= 3 * p_revisedMaxKeepAlive /* part 4 table 86 */ 
    THEN
        skip
    END
    ;
    
    subscription, reqPublishInterval, reqLifetimeCount, reqMaxKeepAlive, maxNotificationsPerPublish
      <-- get_msg_modify_subscription_req_params (p_req_msg) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_subscription_create_req
    THEN
        subscription,
        reqPublishInterval,
        reqLifetimeCount,
        reqMaxKeepAlive,
        maxNotificationsPerPublish
        :(subscription : t_subscription_i &
          reqPublishInterval : t_opcua_duration_i &
          reqLifetimeCount : t_counter &
          reqMaxKeepAlive : t_counter &
          maxNotificationsPerPublish : t_counter)
    END
    ;
  
    set_msg_modify_subscription_resp_params (p_resp_msg,
                                             p_revisedPublishInterval,
                                             p_revisedLifetimeCount,
                                             p_revisedMaxKeepAlive) =
    PRE
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_subscription_create_resp &
        p_revisedPublishInterval : t_opcua_duration_i &
        p_revisedLifetimeCount : t_counter &
        p_revisedMaxKeepAlive : t_counter &
        p_revisedLifetimeCount >= 3 * p_revisedMaxKeepAlive /* part 4 table 86 */ 
    THEN
        skip
    END

END
