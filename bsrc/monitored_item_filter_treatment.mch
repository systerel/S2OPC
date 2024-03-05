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
    monitored_item_filter_treatment

SEES
    constants,
    constants_statuscodes_bs,
    address_space_itf

ABSTRACT_VARIABLES
    /* TODO: remove this variable when create/modify MI specification is simplified */
    a_monitoredItemNodeFilterValid

INVARIANT
    a_monitoredItemNodeFilterValid : t_NodeId --> t_monitoringFilter_i

INITIALISATION
    a_monitoredItemNodeFilterValid :(a_monitoredItemNodeFilterValid : t_NodeId --> t_monitoringFilter_i)

OPERATIONS

    statusCode, filterCtx, filterResult, isEvent <-- check_monitored_item_filter_valid_and_fill_result (p_endpoint_idx, p_nid, p_aid, p_filter, p_value) =
    PRE
        p_endpoint_idx : t_endpoint_config_idx_i &
        p_endpoint_idx : t_endpoint_config_idx &
        p_nid : t_NodeId_i &
        p_nid : t_NodeId &
        p_aid : t_AttributeId_i &
        p_aid : t_AttributeId &
        p_filter : t_monitoringFilter_i &
        (p_filter = c_monitoringFilter_indet or p_filter : t_monitoringFilter) &
        p_value : t_Variant_i
    THEN
        statusCode, filterCtx, filterResult, isEvent
        :(statusCode : t_StatusCode_i & statusCode : t_StatusCode &
          filterCtx : t_monitoringFilterCtx_i &
          filterResult : t_filterResult_i &
          isEvent : BOOL &
          (statusCode = e_sc_ok => (p_nid |-> p_filter) : a_monitoredItemNodeFilterValid) &
          (statusCode /= e_sc_ok => not((p_nid |-> p_filter) : a_monitoredItemNodeFilterValid) &
                                    statusCode : {e_sc_bad_out_of_memory, e_sc_bad_filter_not_allowed,
                                                  e_sc_bad_monitored_item_filter_invalid,
                                                  e_sc_bad_monitored_item_filter_unsupported}
          )
        )
    END
    ;

    /* PROMOTED */

    notifTriggered, eventNotif
      <-- server_subscription_get_notification_on_event (p_clientHandle, p_localeIds,
                                                         p_timestampToReturn, p_userAccessGranted,
                                                         p_filterCtx, p_event) =
    PRE
        p_clientHandle : t_client_handle_i &
        p_localeIds : t_LocaleIds_i &
        p_localeIds : t_LocaleIds &
        p_timestampToReturn : t_TimestampsToReturn_i &
        p_timestampToReturn : t_TimestampsToReturn &
        p_userAccessGranted : BOOL &
        p_filterCtx : t_monitoringFilterCtx_i &
        p_event : t_Event_i &
        p_event : t_Event
    THEN
        notifTriggered,
        eventNotif
        :(notifTriggered : BOOL &
          eventNotif : t_eventFieldList_i &
          (notifTriggered = TRUE =>
              eventNotif : t_eventFieldList)
        )
    END
    ;

    delete_event_filter_context (p_filterCtx) =
    PRE
        p_filterCtx : t_monitoringFilterCtx_i
    THEN
        skip
    END

END
