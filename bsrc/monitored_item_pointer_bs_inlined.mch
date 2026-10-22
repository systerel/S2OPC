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
    monitored_item_pointer_bs_inlined

SEES
    constants,
    monitored_item_pointer_bs

OPERATIONS

    p_monitoredItemId,
    p_subscription,
    p_nid,
    p_aid,
    p_indexRange,
    p_timestampToReturn,
    p_monitoringMode,
    p_clientHandle
      <-- getall_monitoredItemPointer (p_monitoredItemPointer) =
    PRE
        p_monitoredItemPointer : t_monitoredItemPointer_i &
        p_monitoredItemPointer : s_monitoredItemPointer
    THEN
        p_monitoredItemId,
        p_subscription,
        p_nid,
        p_aid,
        p_indexRange,
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
          p_aid = a_monitoredItemAttributeId(p_monitoredItemPointer) &
          p_indexRange : t_IndexRange_i &
          p_timestampToReturn : t_TimestampsToReturn_i &
          p_timestampToReturn : t_TimestampsToReturn &
          p_monitoringMode : t_monitoringMode_i &
          p_monitoringMode = a_monitoredItemMonitMode(p_monitoredItemPointer) &
          p_clientHandle : t_client_handle_i
        )
    END

END
