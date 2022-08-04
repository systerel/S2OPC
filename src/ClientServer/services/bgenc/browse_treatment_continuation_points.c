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

 File Name            : browse_treatment_continuation_points.c

 Date                 : 04/08/2022 14:53:01

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "browse_treatment_continuation_points.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
constants__t_ContinuationPoint_i browse_treatment_continuation_points__session_ContinuationPoint_i[constants__t_session_i_max+1];
t_bool browse_treatment_continuation_points__session_hasContinuationPoint_i[constants__t_session_i_max+1];

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void browse_treatment_continuation_points__INITIALISATION(void) {
   {
      t_entier4 i;
      for (i = constants__t_session_i_max; 0 <= i; i = i - 1) {
         browse_treatment_continuation_points__session_hasContinuationPoint_i[i] = false;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_session_i_max; 0 <= i; i = i - 1) {
         browse_treatment_continuation_points__session_ContinuationPoint_i[i] = constants__c_ContinuationPoint_indet;
      }
   }
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void browse_treatment_continuation_points__has_continuation_point(
   const constants__t_session_i browse_treatment_continuation_points__p_session,
   t_bool * const browse_treatment_continuation_points__bres,
   constants__t_ContinuationPointId_i * const browse_treatment_continuation_points__p_ContinuationPointId) {
   {
      t_bool browse_treatment_continuation_points__l_valid_session;
      t_bool browse_treatment_continuation_points__l_has_cp;
      
      session_mgr__is_valid_session(browse_treatment_continuation_points__p_session,
         &browse_treatment_continuation_points__l_valid_session);
      browse_treatment_continuation_points__l_has_cp = browse_treatment_continuation_points__session_hasContinuationPoint_i[browse_treatment_continuation_points__p_session];
      if ((browse_treatment_continuation_points__l_valid_session == true) &&
         (browse_treatment_continuation_points__l_has_cp == true)) {
         *browse_treatment_continuation_points__bres = true;
         browse_treatment_continuation_points_bs__get_continuation_point_id(browse_treatment_continuation_points__session_ContinuationPoint_i[browse_treatment_continuation_points__p_session],
            browse_treatment_continuation_points__p_ContinuationPointId);
      }
      else {
         *browse_treatment_continuation_points__bres = false;
         *browse_treatment_continuation_points__p_ContinuationPointId = constants__c_ContinuationPointId_indet;
      }
   }
}

void browse_treatment_continuation_points__create_continuation_point(
   const constants__t_session_i browse_treatment_continuation_points__p_session,
   const t_entier4 browse_treatment_continuation_points__p_nextIndex,
   const t_entier4 browse_treatment_continuation_points__p_maxTargetRef,
   const constants__t_NodeId_i browse_treatment_continuation_points__p_browseView,
   const constants__t_NodeId_i browse_treatment_continuation_points__p_nodeId,
   const constants__t_BrowseDirection_i browse_treatment_continuation_points__p_browseDirection,
   const constants__t_NodeId_i browse_treatment_continuation_points__p_referenceType,
   const t_bool browse_treatment_continuation_points__p_includeSubtypes,
   const constants__t_BrowseNodeClassMask_i browse_treatment_continuation_points__p_nodeClassMask,
   const constants__t_BrowseResultMask_i browse_treatment_continuation_points__p_resultMask,
   t_bool * const browse_treatment_continuation_points__bres,
   constants__t_ContinuationPointId_i * const browse_treatment_continuation_points__p_ContinuationPointId) {
   {
      t_bool browse_treatment_continuation_points__l_valid_session;
      t_bool browse_treatment_continuation_points__l_has_cp;
      constants__t_ContinuationPoint_i browse_treatment_continuation_points__l_new_cp;
      
      *browse_treatment_continuation_points__p_ContinuationPointId = constants__c_ContinuationPointId_indet;
      session_mgr__is_valid_session(browse_treatment_continuation_points__p_session,
         &browse_treatment_continuation_points__l_valid_session);
      browse_treatment_continuation_points__l_has_cp = browse_treatment_continuation_points__session_hasContinuationPoint_i[browse_treatment_continuation_points__p_session];
      *browse_treatment_continuation_points__bres = ((browse_treatment_continuation_points__l_valid_session == true) &&
         (browse_treatment_continuation_points__l_has_cp == false));
      if (*browse_treatment_continuation_points__bres == true) {
         browse_treatment_continuation_points_bs__create_continuation_point_bs(browse_treatment_continuation_points__p_nextIndex,
            browse_treatment_continuation_points__p_maxTargetRef,
            browse_treatment_continuation_points__p_browseView,
            browse_treatment_continuation_points__p_nodeId,
            browse_treatment_continuation_points__p_browseDirection,
            browse_treatment_continuation_points__p_referenceType,
            browse_treatment_continuation_points__p_includeSubtypes,
            browse_treatment_continuation_points__p_nodeClassMask,
            browse_treatment_continuation_points__p_resultMask,
            browse_treatment_continuation_points__bres,
            &browse_treatment_continuation_points__l_new_cp);
         if (*browse_treatment_continuation_points__bres == true) {
            browse_treatment_continuation_points__session_hasContinuationPoint_i[browse_treatment_continuation_points__p_session] = true;
            browse_treatment_continuation_points__session_ContinuationPoint_i[browse_treatment_continuation_points__p_session] = browse_treatment_continuation_points__l_new_cp;
            browse_treatment_continuation_points_bs__get_continuation_point_id(browse_treatment_continuation_points__l_new_cp,
               browse_treatment_continuation_points__p_ContinuationPointId);
         }
      }
   }
}

void browse_treatment_continuation_points__getall_continuation_point(
   const constants__t_session_i browse_treatment_continuation_points__p_session,
   const constants__t_ContinuationPointId_i browse_treatment_continuation_points__p_continuationPointId,
   t_bool * const browse_treatment_continuation_points__bres,
   t_entier4 * const browse_treatment_continuation_points__p_nextIndex,
   t_entier4 * const browse_treatment_continuation_points__p_maxTargetRef,
   constants__t_NodeId_i * const browse_treatment_continuation_points__p_browseView,
   constants__t_NodeId_i * const browse_treatment_continuation_points__p_nodeId,
   constants__t_BrowseDirection_i * const browse_treatment_continuation_points__p_browseDirection,
   constants__t_NodeId_i * const browse_treatment_continuation_points__p_referenceType,
   t_bool * const browse_treatment_continuation_points__p_includeSubtypes,
   constants__t_BrowseNodeClassMask_i * const browse_treatment_continuation_points__p_nodeClassMask,
   constants__t_BrowseResultMask_i * const browse_treatment_continuation_points__p_resultMask) {
   {
      t_bool browse_treatment_continuation_points__l_valid_session;
      t_bool browse_treatment_continuation_points__l_session_has_cp;
      constants__t_ContinuationPoint_i browse_treatment_continuation_points__l_session_cp;
      constants__t_ContinuationPointId_i browse_treatment_continuation_points__l_session_cp_id;
      
      *browse_treatment_continuation_points__bres = false;
      *browse_treatment_continuation_points__p_nextIndex = 0;
      *browse_treatment_continuation_points__p_maxTargetRef = 0;
      *browse_treatment_continuation_points__p_browseView = constants__c_NodeId_indet;
      *browse_treatment_continuation_points__p_nodeId = constants__c_NodeId_indet;
      *browse_treatment_continuation_points__p_browseDirection = constants__e_bd_indet;
      *browse_treatment_continuation_points__p_referenceType = constants__c_NodeId_indet;
      *browse_treatment_continuation_points__p_includeSubtypes = false;
      *browse_treatment_continuation_points__p_nodeClassMask = constants__c_BrowseNodeClassMask_indet;
      *browse_treatment_continuation_points__p_resultMask = constants__c_BrowseResultMask_indet;
      session_mgr__is_valid_session(browse_treatment_continuation_points__p_session,
         &browse_treatment_continuation_points__l_valid_session);
      browse_treatment_continuation_points__l_session_has_cp = browse_treatment_continuation_points__session_hasContinuationPoint_i[browse_treatment_continuation_points__p_session];
      if ((browse_treatment_continuation_points__l_valid_session == true) &&
         (browse_treatment_continuation_points__l_session_has_cp == true)) {
         browse_treatment_continuation_points__l_session_cp = browse_treatment_continuation_points__session_ContinuationPoint_i[browse_treatment_continuation_points__p_session];
         browse_treatment_continuation_points_bs__get_continuation_point_id(browse_treatment_continuation_points__l_session_cp,
            &browse_treatment_continuation_points__l_session_cp_id);
         if (browse_treatment_continuation_points__l_session_cp_id == browse_treatment_continuation_points__p_continuationPointId) {
            *browse_treatment_continuation_points__bres = true;
            browse_treatment_continuation_points_bs__getall_continuation_point_bs(browse_treatment_continuation_points__l_session_cp,
               browse_treatment_continuation_points__p_nextIndex,
               browse_treatment_continuation_points__p_maxTargetRef,
               browse_treatment_continuation_points__p_browseView,
               browse_treatment_continuation_points__p_nodeId,
               browse_treatment_continuation_points__p_browseDirection,
               browse_treatment_continuation_points__p_referenceType,
               browse_treatment_continuation_points__p_includeSubtypes,
               browse_treatment_continuation_points__p_nodeClassMask,
               browse_treatment_continuation_points__p_resultMask);
         }
      }
   }
}

void browse_treatment_continuation_points__release_continuation_point(
   const constants__t_session_i browse_treatment_continuation_points__p_session,
   const constants__t_ContinuationPointId_i browse_treatment_continuation_points__p_continuationPointId,
   t_bool * const browse_treatment_continuation_points__bres) {
   {
      t_bool browse_treatment_continuation_points__l_valid_session;
      t_bool browse_treatment_continuation_points__l_session_has_cp;
      constants__t_ContinuationPoint_i browse_treatment_continuation_points__l_session_cp;
      constants__t_ContinuationPointId_i browse_treatment_continuation_points__l_session_cp_id;
      
      *browse_treatment_continuation_points__bres = false;
      session_mgr__is_valid_session(browse_treatment_continuation_points__p_session,
         &browse_treatment_continuation_points__l_valid_session);
      browse_treatment_continuation_points__l_session_has_cp = browse_treatment_continuation_points__session_hasContinuationPoint_i[browse_treatment_continuation_points__p_session];
      if ((browse_treatment_continuation_points__l_valid_session == true) &&
         (browse_treatment_continuation_points__l_session_has_cp == true)) {
         browse_treatment_continuation_points__l_session_cp = browse_treatment_continuation_points__session_ContinuationPoint_i[browse_treatment_continuation_points__p_session];
         browse_treatment_continuation_points_bs__get_continuation_point_id(browse_treatment_continuation_points__l_session_cp,
            &browse_treatment_continuation_points__l_session_cp_id);
         if (browse_treatment_continuation_points__l_session_cp_id == browse_treatment_continuation_points__p_continuationPointId) {
            *browse_treatment_continuation_points__bres = true;
            browse_treatment_continuation_points__session_hasContinuationPoint_i[browse_treatment_continuation_points__p_session] = false;
            browse_treatment_continuation_points__session_ContinuationPoint_i[browse_treatment_continuation_points__p_session] = constants__c_ContinuationPoint_indet;
            browse_treatment_continuation_points_bs__clear_continuation_point(browse_treatment_continuation_points__l_session_cp);
         }
      }
   }
}

void browse_treatment_continuation_points__set_session_closed(
   const constants__t_session_i browse_treatment_continuation_points__p_session) {
   browse_treatment_continuation_points_bs__clear_continuation_point(browse_treatment_continuation_points__session_ContinuationPoint_i[browse_treatment_continuation_points__p_session]);
   browse_treatment_continuation_points__session_hasContinuationPoint_i[browse_treatment_continuation_points__p_session] = false;
   browse_treatment_continuation_points__session_ContinuationPoint_i[browse_treatment_continuation_points__p_session] = constants__c_ContinuationPoint_indet;
}

void browse_treatment_continuation_points__continuation_points_UNINITIALISATION(void) {
   {
      t_bool browse_treatment_continuation_points__l_continue;
      constants__t_session_i browse_treatment_continuation_points__l_session;
      
      browse_treatment_continuation_points_session_it__init_iter_session(&browse_treatment_continuation_points__l_continue);
      while (browse_treatment_continuation_points__l_continue == true) {
         browse_treatment_continuation_points_session_it__continue_iter_session(&browse_treatment_continuation_points__l_continue,
            &browse_treatment_continuation_points__l_session);
         browse_treatment_continuation_points_bs__clear_continuation_point(browse_treatment_continuation_points__session_ContinuationPoint_i[browse_treatment_continuation_points__l_session]);
         browse_treatment_continuation_points__session_hasContinuationPoint_i[browse_treatment_continuation_points__l_session] = false;
         browse_treatment_continuation_points__session_ContinuationPoint_i[browse_treatment_continuation_points__l_session] = constants__c_ContinuationPoint_indet;
      }
   }
}

