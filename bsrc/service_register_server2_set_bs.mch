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
    service_register_server2_set_bs

SEES
    constants

DEFINITIONS
    d_variables_set ==
    RegisteredServer2_CounterResetTime,
    s_RegisteredServer2,
    a_RegisteredServer2_RecordId,
    a_RegisteredServer2_RegisteredServer,
    a_RegisteredServer2_mDNSconfig
    ;

    d_inv_set ==
    RegisteredServer2_CounterResetTime : t_Timestamp & /* It is a simple DateTime but use Timestamp to avoid adding a new type */
    s_RegisteredServer2 <: t_RegisteredServer2Info &
    a_RegisteredServer2_RecordId : s_RegisteredServer2 >->> NAT &
    a_RegisteredServer2_RegisteredServer : s_RegisteredServer2 --> t_RegisteredServer &
    a_RegisteredServer2_mDNSconfig : s_RegisteredServer2 --> t_MdnsDiscoveryConfig
    ;

    d_init_set ==
    d_variables_set
    :(
        RegisteredServer2_CounterResetTime : t_Timestamp & /* INITIALISATION timestamp */
        s_RegisteredServer2 = {} &
        a_RegisteredServer2_RecordId = {} &
        a_RegisteredServer2_RegisteredServer = {} &
        a_RegisteredServer2_mDNSconfig = {}
    )
    ;

    d_variables_set_it ==
    s_registered_server2_to_iterate_it,
    s_registered_server2_iterated_it
    ;

    d_inv_set_it ==
    s_registered_server2_to_iterate_it <: s_RegisteredServer2 &
    s_registered_server2_iterated_it <: s_RegisteredServer2 &
    s_registered_server2_to_iterate_it /\ s_registered_server2_iterated_it = {} &
    s_registered_server2_to_iterate_it \/ s_registered_server2_iterated_it = s_RegisteredServer2 &
    /* RecordId is always strictly increasing during iteration: */
    (s_registered_server2_iterated_it /= {} & s_registered_server2_to_iterate_it /= {} =>
        max(a_RegisteredServer2_RecordId[s_registered_server2_iterated_it])
        < min(a_RegisteredServer2_RecordId[s_registered_server2_to_iterate_it]))
    ;

    d_init_set_it ==
    s_registered_server2_to_iterate_it :(s_registered_server2_to_iterate_it = {}) ||
    s_registered_server2_iterated_it :(s_registered_server2_iterated_it = {})

ABSTRACT_VARIABLES
    d_variables_set,

    d_variables_set_it

INVARIANT
    d_inv_set &

    d_inv_set_it

INITIALISATION
    d_init_set ||

    d_init_set_it

OPERATIONS

    /* Iterator on set */

    continue <-- init_iter_registered_server2_set =
    BEGIN
        s_registered_server2_to_iterate_it,
        s_registered_server2_iterated_it,
        continue
        :(s_registered_server2_to_iterate_it <: s_RegisteredServer2 &
          s_registered_server2_iterated_it <: s_RegisteredServer2 &
          continue : BOOL &
          s_registered_server2_to_iterate_it = s_RegisteredServer2 &
          s_registered_server2_iterated_it = {} &
          continue = bool(s_registered_server2_to_iterate_it /= {})
        )
    END
    ;

    clear_iter_registered_server2_set =
    BEGIN
        s_registered_server2_to_iterate_it := {} ||
        s_registered_server2_iterated_it := {}
    END
    ;

    continue, p_registeredServerInfo <-- continue_iter_registered_server2_set =
    PRE
        s_registered_server2_to_iterate_it /= {}
    THEN
        ANY l_registeredServerInfo WHERE
            l_registeredServerInfo : t_RegisteredServer2Info_i &
            l_registeredServerInfo : s_registered_server2_to_iterate_it &
            a_RegisteredServer2_RecordId(l_registeredServerInfo) =
              min(a_RegisteredServer2_RecordId[s_registered_server2_to_iterate_it])
        THEN
            continue := bool(s_registered_server2_to_iterate_it /= {l_registeredServerInfo}) ||
            p_registeredServerInfo := l_registeredServerInfo ||
            s_registered_server2_to_iterate_it := s_registered_server2_to_iterate_it - {l_registeredServerInfo} ||
            s_registered_server2_iterated_it := s_registered_server2_iterated_it \/ {l_registeredServerInfo}
        END
    END
    ;

    /* Operations on set */
    reset_registered_server2_set =
    BEGIN
        d_init_set
    END
    ;

    remove_from_registered_server2_set (p_recordId) =
    PRE
        p_recordId : NAT &
        p_recordId : ran(a_RegisteredServer2_RecordId)
    THEN
        s_RegisteredServer2,
        a_RegisteredServer2_RecordId,
        a_RegisteredServer2_RegisteredServer,
        a_RegisteredServer2_mDNSconfig
        :(d_inv_set  &
            s_RegisteredServer2 = s_RegisteredServer2$0 - {a_RegisteredServer2_RecordId~(p_recordId)} &
            a_RegisteredServer2_RecordId = s_RegisteredServer2 <| a_RegisteredServer2_RecordId$0 &
            a_RegisteredServer2_RegisteredServer = s_RegisteredServer2 <| a_RegisteredServer2_RegisteredServer$0 &
            a_RegisteredServer2_mDNSconfig = s_RegisteredServer2 <| a_RegisteredServer2_mDNSconfig$0
        )
    END
    ;

    alloc_success <-- add_to_registered_server2_set (p_registered_server, p_mdns_config, p_recordId) =
    PRE
        p_registered_server : t_RegisteredServer_i &
        p_registered_server : t_RegisteredServer &
        p_mdns_config : t_MdnsDiscoveryConfig_i &
        p_mdns_config : t_MdnsDiscoveryConfig &
        p_recordId : NAT
    THEN
        IF t_RegisteredServer2Info = s_RegisteredServer2
        THEN
            alloc_success := FALSE
        ELSE
            alloc_success := TRUE ||
            ANY l_registerInfo WHERE
                l_registerInfo : t_RegisteredServer2Info - s_RegisteredServer2
            THEN
                d_variables_set
                :(d_inv_set &
                    RegisteredServer2_CounterResetTime = RegisteredServer2_CounterResetTime$0 &
                    s_RegisteredServer2 = s_RegisteredServer2$0 \/ {l_registerInfo} &
                    a_RegisteredServer2_RecordId =
                      a_RegisteredServer2_RecordId$0 <+ {l_registerInfo |-> p_recordId} &
                    a_RegisteredServer2_RegisteredServer =
                      a_RegisteredServer2_RegisteredServer$0  <+ {l_registerInfo |-> p_registered_server} &
                    a_RegisteredServer2_mDNSconfig =
                      a_RegisteredServer2_mDNSconfig$0 <+ {l_registerInfo |-> p_mdns_config}
                )
            END
        END
    END
    ;

    card_set <-- get_card_register2_set =
    BEGIN
        card_set := card(s_RegisteredServer2)
    END
    ;

    /* Last reset timestamp */
    p_timestamp <-- get_registered_server2_counter_reset_time =
    BEGIN
        p_timestamp := RegisteredServer2_CounterResetTime
    END
    ;

    /* Operations on set elements */
    p_registered_server <-- get_registered_server2_registered_server (p_registeredServerInfo) =
    PRE
        p_registeredServerInfo : t_RegisteredServer2Info_i &
        p_registeredServerInfo : s_RegisteredServer2
    THEN
        p_registered_server
        :(p_registered_server : t_RegisteredServer_i &
          p_registered_server = a_RegisteredServer2_RegisteredServer (p_registeredServerInfo))
    END
    ;

    p_recordId <-- get_registered_server2_recordId (p_registeredServerInfo) =
    PRE
        p_registeredServerInfo : t_RegisteredServer2Info_i &
        p_registeredServerInfo : s_RegisteredServer2
    THEN
        p_recordId := a_RegisteredServer2_RecordId (p_registeredServerInfo)
    END
    ;

    p_mdns_config <-- get_registered_server2_mdns_config (p_registeredServerInfo) =
    PRE
        p_registeredServerInfo : t_RegisteredServer2Info_i &
        p_registeredServerInfo : s_RegisteredServer2
    THEN
        p_mdns_config
        :(p_mdns_config : t_MdnsDiscoveryConfig_i &
          p_mdns_config = a_RegisteredServer2_mDNSconfig (p_registeredServerInfo))
    END
    ;

    service_register_server2_set_bs_UNINITIALISATION =
    BEGIN
        skip
    END

END
