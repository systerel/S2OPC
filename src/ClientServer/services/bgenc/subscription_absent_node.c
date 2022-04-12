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

 File Name            : subscription_absent_node.c

 Date                 : 05/08/2022 08:40:49

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "subscription_absent_node.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void subscription_absent_node__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void subscription_absent_node__if_not_present_is_Node_known(
   const t_bool subscription_absent_node__p_node_is_present,
   const constants__t_endpoint_config_idx_i subscription_absent_node__p_endpoint_config_idx,
   const constants__t_NodeId_i subscription_absent_node__p_nid,
   t_bool * const subscription_absent_node__bres_absent_knownNode,
   constants__t_NodeClass_i * const subscription_absent_node__knownNodeClass,
   constants__t_RawStatusCode * const subscription_absent_node__valueSc) {
   if (subscription_absent_node__p_node_is_present == true) {
      *subscription_absent_node__bres_absent_knownNode = false;
      *subscription_absent_node__knownNodeClass = constants__c_NodeClass_indet;
      constants_statuscodes_bs__get_const_RawStatusCode_Good(subscription_absent_node__valueSc);
   }
   else {
      subscription_absent_node_bs__absent_Node_is_known(subscription_absent_node__p_endpoint_config_idx,
         subscription_absent_node__p_nid,
         subscription_absent_node__bres_absent_knownNode,
         subscription_absent_node__knownNodeClass,
         subscription_absent_node__valueSc);
   }
}

void subscription_absent_node__eval_knownNode_requested_properties(
   const constants__t_NodeId_i subscription_absent_node__p_nid,
   const constants__t_NodeClass_i subscription_absent_node__p_knownNodeClass,
   const constants__t_AttributeId_i subscription_absent_node__p_aid,
   const constants__t_IndexRange_i subscription_absent_node__p_indexRange,
   constants_statuscodes_bs__t_StatusCode_i * const subscription_absent_node__sc,
   constants__t_Variant_i * const subscription_absent_node__value,
   constants__t_Timestamp * const subscription_absent_node__val_ts_src,
   constants__t_Timestamp * const subscription_absent_node__val_ts_srv) {
   {
      t_bool subscription_absent_node__l_is_supported_attribute;
      t_bool subscription_absent_node__l_is_index_range_defined;
      
      subscription_absent_node_bs__nodeId_do_nothing(subscription_absent_node__p_nid);
      *subscription_absent_node__sc = constants_statuscodes_bs__e_sc_bad_attribute_id_invalid;
      *subscription_absent_node__value = constants__c_Variant_indet;
      *subscription_absent_node__val_ts_src = constants__c_Timestamp_null;
      *subscription_absent_node__val_ts_srv = constants__c_Timestamp_null;
      address_space_itf__is_mandatory_attribute(subscription_absent_node__p_knownNodeClass,
         subscription_absent_node__p_aid,
         &subscription_absent_node__l_is_supported_attribute);
      if (subscription_absent_node__l_is_supported_attribute == true) {
         address_space_itf__is_IndexRangeDefined(subscription_absent_node__p_indexRange,
            &subscription_absent_node__l_is_index_range_defined);
         if ((subscription_absent_node__p_aid != constants__e_aid_Value) &&
            (subscription_absent_node__l_is_index_range_defined == true)) {
            *subscription_absent_node__sc = constants_statuscodes_bs__e_sc_bad_index_range_no_data;
         }
         else {
            *subscription_absent_node__sc = constants_statuscodes_bs__e_sc_ok;
         }
      }
   }
}

