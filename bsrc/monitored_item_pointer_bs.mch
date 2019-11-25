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
    monitored_item_pointer_bs

SEES
    constants,
    constants_statuscodes_bs

ABSTRACT_VARIABLES
    s_monitoredItemPointer,
    s_monitoredItemId,
    a_monitoredItemId,
    a_monitoredItemSubs,
    a_monitoredItemNodeId

INVARIANT

    s_monitoredItemPointer <: t_monitoredItemPointer &
    s_monitoredItemId <: t_monitoredItemId &
    a_monitoredItemId : s_monitoredItemPointer >->> s_monitoredItemId &
    a_monitoredItemSubs : s_monitoredItemPointer --> t_subscription &
    a_monitoredItemNodeId : s_monitoredItemPointer --> t_NodeId

INITIALISATION

    s_monitoredItemPointer :(s_monitoredItemPointer <: t_monitoredItemPointer & s_monitoredItemPointer = {}) ||
    s_monitoredItemId :(s_monitoredItemId <: t_monitoredItemId & s_monitoredItemId = {}) ||
    a_monitoredItemId :(a_monitoredItemId : s_monitoredItemPointer <-> s_monitoredItemId & a_monitoredItemId = {}) ||
    a_monitoredItemSubs :(a_monitoredItemSubs : s_monitoredItemPointer <-> t_subscription & a_monitoredItemSubs = {}) ||
    a_monitoredItemNodeId :(a_monitoredItemNodeId : s_monitoredItemPointer <-> t_NodeId & a_monitoredItemNodeId = {})

OPERATIONS

    /* Needs UNINIT to deallocate dict and list */
    monitored_item_pointer_bs_UNINITIALISATION =
    BEGIN
        skip
    END
    ;

    StatusCode, monitoredItemPointer, monitoredItemId <-- create_monitored_item_pointer (p_subscription,
                                                                                         p_nid,
                                                                                         p_aid,
                                                                                         p_indexRange,
                                                                                         p_timestampToReturn,
                                                                                         p_monitoringMode,
                                                                                         p_clientHandle) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : t_subscription &
        p_nid : t_NodeId_i &
        p_nid : t_NodeId &
        p_aid : t_AttributeId_i &
        p_aid : t_AttributeId &
        p_indexRange : t_IndexRange_i &
        p_timestampToReturn : t_TimestampsToReturn_i &
        p_timestampToReturn : t_TimestampsToReturn &
        p_monitoringMode : t_monitoringMode_i &
        p_monitoringMode : t_monitoringMode &
        p_clientHandle : t_client_handle_i
    THEN
        IF t_monitoredItemPointer = s_monitoredItemPointer or
            t_monitoredItemId = s_monitoredItemId
        THEN
            StatusCode := e_sc_bad_out_of_memory ||
            monitoredItemPointer := c_monitoredItemPointer_indet ||
            monitoredItemId := c_monitoredItemId_indet
        ELSIF p_indexRange : t_IndexRange /* It is null or it can be parsed */
        THEN
            ANY l_monitoredItem, l_monitoredItemId WHERE
                l_monitoredItem : t_monitoredItemPointer - s_monitoredItemPointer &
                l_monitoredItemId : t_monitoredItemId - s_monitoredItemId
            THEN
                StatusCode := e_sc_ok ||
                s_monitoredItemPointer := s_monitoredItemPointer \/ {l_monitoredItem} ||
                s_monitoredItemId := s_monitoredItemId \/ {l_monitoredItemId} ||
                a_monitoredItemId(l_monitoredItem) := l_monitoredItemId ||
                a_monitoredItemSubs (l_monitoredItem) := p_subscription ||
                a_monitoredItemNodeId (l_monitoredItem) := p_nid ||
                monitoredItemPointer := l_monitoredItem ||
                monitoredItemId := l_monitoredItemId
            END
        ELSE
            StatusCode := e_sc_bad_index_range_invalid ||
            monitoredItemPointer := c_monitoredItemPointer_indet ||
            monitoredItemId := c_monitoredItemId_indet
        END
    END
    ;

    delete_monitored_item_pointer (p_monitoredItemPointer) =
    PRE
        p_monitoredItemPointer : t_monitoredItemPointer_i &
        p_monitoredItemPointer : s_monitoredItemPointer
    THEN
        s_monitoredItemPointer := s_monitoredItemPointer - {p_monitoredItemPointer} ||
        s_monitoredItemId := s_monitoredItemId - {a_monitoredItemId(p_monitoredItemPointer)} ||
        a_monitoredItemId := a_monitoredItemId - {p_monitoredItemPointer |-> a_monitoredItemId(p_monitoredItemPointer)} ||
        a_monitoredItemSubs := {p_monitoredItemPointer} <<| a_monitoredItemSubs ||
        a_monitoredItemNodeId := {p_monitoredItemPointer} <<| a_monitoredItemNodeId
    END
    ;

    p_monitoredItemId,
    p_subscription,
    p_nid,
    p_aid,
    p_timestampToReturn,
    p_monitoringMode,
    p_clientHandle
      <--getall_monitoredItemPointer (p_monitoredItemPointer) =
    PRE
        p_monitoredItemPointer : t_monitoredItemPointer_i &
        p_monitoredItemPointer : s_monitoredItemPointer
    THEN
        p_monitoredItemId,
        p_subscription,
        p_nid,
        p_aid,
        p_timestampToReturn,
        p_monitoringMode,
        p_clientHandle
        :(p_monitoredItemId : t_monitoredItemId_i &
          p_monitoredItemId = a_monitoredItemId(p_monitoredItemPointer) &
          p_subscription : t_subscription_i &
          p_subscription = a_monitoredItemSubs(p_monitoredItemPointer) &
          p_nid : t_NodeId_i &
          p_nid = a_monitoredItemNodeId(p_monitoredItemPointer) &
          p_aid : t_AttributeId_i &
          p_aid : t_AttributeId &
          p_timestampToReturn : t_TimestampsToReturn_i &
          p_timestampToReturn : t_TimestampsToReturn &
          p_monitoringMode : t_monitoringMode_i &
          p_monitoringMode : t_monitoringMode &
          p_clientHandle : t_client_handle_i
        )
    END
    ;

    bres <-- is_notification_triggered (p_monitoredItemPointer, p_old_wv_pointer, p_new_wv_pointer) =
    PRE
        p_monitoredItemPointer : t_monitoredItemPointer_i &
        p_monitoredItemPointer : t_monitoredItemPointer &
        p_old_wv_pointer : t_WriteValuePointer_i &
        p_old_wv_pointer : t_WriteValuePointer &
        p_new_wv_pointer : t_WriteValuePointer_i &
        p_new_wv_pointer : t_WriteValuePointer
    THEN
        bres :: BOOL
    END

END
