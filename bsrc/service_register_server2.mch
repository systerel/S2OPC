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
    service_register_server2

SEES
    constants,
    constants_statuscodes_bs,
    service_set_discovery_server_data_bs

DEFINITIONS

    d_variables ==
    RegisteredServer2_CounterResetTime,
    RegisteredServer2_Counter,
    s_RegisteredServer2,
    a_RegisteredServer2_RecordId,
    a_RegisteredServer2_RegisteredServer,
    a_RegisteredServer2_mDNSconfig
    ;

    d_inv ==
    RegisteredServer2_CounterResetTime : t_Timestamp & /* It is a simple DateTime but use Timestamp to avoid adding a new type */
    RegisteredServer2_Counter : NAT &
    s_RegisteredServer2 <: t_RegisteredServer2Info &
    a_RegisteredServer2_RecordId : s_RegisteredServer2 >->> NAT &
    a_RegisteredServer2_RegisteredServer : s_RegisteredServer2 --> t_RegisteredServer &
    a_RegisteredServer2_mDNSconfig : s_RegisteredServer2 --> t_MdnsDiscoveryConfig
    ;

    d_init ==
        d_variables
        :(
        RegisteredServer2_CounterResetTime : t_Timestamp & /* INITIALISATION timestamp */
        RegisteredServer2_Counter = 0 &
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
    d_variables,
    d_variables_set_it

INVARIANT
    d_inv &
    d_inv_set_it

INITIALISATION
    d_init ||
    d_init_set_it

OPERATIONS

    p_sc <-- register_server2_create (p_registered_server, p_mdns_config) =
    PRE
        p_registered_server : t_RegisteredServer_i &
        p_registered_server : t_RegisteredServer &
        p_mdns_config : t_MdnsDiscoveryConfig_i &
        p_mdns_config : t_MdnsDiscoveryConfig &
        /* If same server URI already recorded, it shall have been removed previously */
        RegisteredServer_ServerUri(p_registered_server) /:
         RegisteredServer_ServerUri[ran(a_RegisteredServer2_RegisteredServer)]
    THEN
        IF t_RegisteredServer2Info = s_RegisteredServer2
        THEN
            p_sc := e_sc_bad_out_of_memory
        ELSE
            ANY l_registerInfo WHERE
                l_registerInfo : t_RegisteredServer2Info - s_RegisteredServer2
            THEN
                p_sc := e_sc_ok ||
                s_RegisteredServer2,
                RegisteredServer2_Counter,
                a_RegisteredServer2_RecordId,
                a_RegisteredServer2_RegisteredServer,
                a_RegisteredServer2_mDNSconfig
                :(d_inv &
                    (RegisteredServer2_Counter = MAXINT => /* Reset records and create new record */
                        RegisteredServer2_CounterResetTime : t_Timestamp &
                        RegisteredServer2_Counter = 1 &
                        /* Create new server info from empty variables */
                        s_RegisteredServer2 = {l_registerInfo} &
                        a_RegisteredServer2_RecordId = {l_registerInfo |-> RegisteredServer2_Counter} &
                        a_RegisteredServer2_RegisteredServer = {l_registerInfo |-> p_registered_server} &
                        a_RegisteredServer2_mDNSconfig = {l_registerInfo |-> p_mdns_config}
                    ) &
                    (RegisteredServer2_Counter /= MAXINT =>  /* Create or update record */
                        RegisteredServer2_Counter = RegisteredServer2_Counter$0 + 1 &
                        /* Create new server info: */
                        s_RegisteredServer2 = s_RegisteredServer2$0 \/ {l_registerInfo} &
                        a_RegisteredServer2_RecordId =
                          a_RegisteredServer2_RecordId$0 <+ {l_registerInfo |-> RegisteredServer2_Counter} &
                        a_RegisteredServer2_RegisteredServer =
                          a_RegisteredServer2_RegisteredServer$0 <+ {l_registerInfo |-> p_registered_server} &
                        a_RegisteredServer2_mDNSconfig =
                          a_RegisteredServer2_mDNSconfig$0 <+ {l_registerInfo |-> p_mdns_config}
                    )
                )
            END
        END
    END
    ;

    register_server2_remove (p_registered_server) =
    PRE
        p_registered_server : t_RegisteredServer_i &
        p_registered_server : t_RegisteredServer
    THEN
        s_RegisteredServer2,
        a_RegisteredServer2_RecordId,
        a_RegisteredServer2_RegisteredServer,
        a_RegisteredServer2_mDNSconfig
        :(d_inv &
            /* ServerUri was not recorded yet: do not change anything */
            (RegisteredServer_ServerUri(p_registered_server) /:
                RegisteredServer_ServerUri[ran(a_RegisteredServer2_RegisteredServer$0)] =>
                s_RegisteredServer2 = s_RegisteredServer2$0 &
                a_RegisteredServer2_RecordId = a_RegisteredServer2_RecordId$0 &
                a_RegisteredServer2_RegisteredServer = a_RegisteredServer2_RegisteredServer$0 &
                a_RegisteredServer2_mDNSconfig = a_RegisteredServer2_mDNSconfig$0
            ) &
            /* Update existing server info */
            (RegisteredServer_ServerUri(p_registered_server) :
                RegisteredServer_ServerUri[ran(a_RegisteredServer2_RegisteredServer$0)] =>
                s_RegisteredServer2 = s_RegisteredServer2$0
                /* Remove from set the record with same server Uri as new new record */
                - a_RegisteredServer2_RegisteredServer$0~
                [RegisteredServer_ServerUri~[{RegisteredServer_ServerUri(p_registered_server)}]] &
                /* Restrict previous relations to the new set of records (removing old record with same serverUri)
                and add the new record in each relation */
                a_RegisteredServer2_RecordId =
                s_RegisteredServer2 <| a_RegisteredServer2_RecordId$0 &
                a_RegisteredServer2_RegisteredServer =
                s_RegisteredServer2 <| a_RegisteredServer2_RegisteredServer$0 &
                a_RegisteredServer2_mDNSconfig =
                s_RegisteredServer2 <| a_RegisteredServer2_mDNSconfig$0
            )
        )
    END
    ;

    service_register_server2_UNINITIALISATION =
    BEGIN
        skip
    END
    ;

    /* PROMOTED */
    /* iterator on set */

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

END
