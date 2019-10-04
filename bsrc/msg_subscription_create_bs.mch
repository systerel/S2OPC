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
        a_msg_in_type = e_msg_subscription_modify_req
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
        a_msg_out_type = e_msg_subscription_modify_resp &
        p_revisedPublishInterval : t_opcua_duration_i &
        p_revisedLifetimeCount : t_counter &
        p_revisedMaxKeepAlive : t_counter &
        p_revisedLifetimeCount >= 3 * p_revisedMaxKeepAlive /* part 4 table 86 */
    THEN
        skip
    END

END
