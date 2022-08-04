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

 File Name            : translate_browse_path_1.c

 Date                 : 04/08/2022 14:53:23

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "translate_browse_path_1.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
t_entier4 translate_browse_path_1__BrowsePathRemainingIndex_tab_i[constants__t_BrowsePathResPerElt_i_max+1];
t_entier4 translate_browse_path_1__BrowsePathRemainingNodeId_size_i;
constants__t_ExpandedNodeId_i translate_browse_path_1__BrowsePathRemainingNodeId_tab_i[constants__t_BrowsePathResPerElt_i_max+1];
t_entier4 translate_browse_path_1__BrowsePathResult_size_i;
constants__t_ExpandedNodeId_i translate_browse_path_1__BrowsePathResult_tab_i[constants__t_BrowsePathResPerElt_i_max+1];
t_entier4 translate_browse_path_1__BrowsePathSource_size_i;
constants__t_NodeId_i translate_browse_path_1__BrowsePathSource_tab_i[constants__t_BrowsePathResPerElt_i_max+1];

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void translate_browse_path_1__INITIALISATION(void) {
   {
      t_entier4 i;
      for (i = constants__t_BrowsePathResPerElt_i_max; 0 <= i; i = i - 1) {
         translate_browse_path_1__BrowsePathSource_tab_i[i] = constants__c_NodeId_indet;
      }
   }
   translate_browse_path_1__BrowsePathSource_size_i = 0;
   {
      t_entier4 i;
      for (i = constants__t_BrowsePathResPerElt_i_max; 0 <= i; i = i - 1) {
         translate_browse_path_1__BrowsePathResult_tab_i[i] = constants__c_ExpandedNodeId_indet;
      }
   }
   translate_browse_path_1__BrowsePathResult_size_i = 0;
   translate_browse_path_1__BrowsePathRemainingNodeId_size_i = 0;
   {
      t_entier4 i;
      for (i = constants__t_BrowsePathResPerElt_i_max; 0 <= i; i = i - 1) {
         translate_browse_path_1__BrowsePathRemainingNodeId_tab_i[i] = constants__c_ExpandedNodeId_indet;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_BrowsePathResPerElt_i_max; 0 <= i; i = i - 1) {
         translate_browse_path_1__BrowsePathRemainingIndex_tab_i[i] = 0;
      }
   }
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void translate_browse_path_1__init_BrowsePathSource(void) {
   {
      t_entier4 i;
      for (i = constants__t_BrowsePathResPerElt_i_max; 0 <= i; i = i - 1) {
         translate_browse_path_1__BrowsePathSource_tab_i[i] = constants__c_NodeId_indet;
      }
   }
   translate_browse_path_1__BrowsePathSource_size_i = 0;
}

void translate_browse_path_1__get_BrowsePathSourceSize(
   t_entier4 * const translate_browse_path_1__res) {
   *translate_browse_path_1__res = translate_browse_path_1__BrowsePathSource_size_i;
}

void translate_browse_path_1__get_BrowsePathSource(
   const t_entier4 translate_browse_path_1__index,
   constants__t_NodeId_i * const translate_browse_path_1__nodeId) {
   *translate_browse_path_1__nodeId = translate_browse_path_1__BrowsePathSource_tab_i[translate_browse_path_1__index];
}

void translate_browse_path_1__add_BrowsePathSource(
   const constants__t_NodeId_i translate_browse_path_1__nodeId) {
   translate_browse_path_1__BrowsePathSource_size_i = translate_browse_path_1__BrowsePathSource_size_i +
      1;
   translate_browse_path_1__BrowsePathSource_tab_i[translate_browse_path_1__BrowsePathSource_size_i] = translate_browse_path_1__nodeId;
}

void translate_browse_path_1__init_BrowsePathResult(void) {
   {
      t_entier4 i;
      for (i = constants__t_BrowsePathResPerElt_i_max; 0 <= i; i = i - 1) {
         translate_browse_path_1__BrowsePathResult_tab_i[i] = constants__c_ExpandedNodeId_indet;
      }
   }
   translate_browse_path_1__BrowsePathResult_size_i = 0;
}

void translate_browse_path_1__get_BrowsePathResultSize(
   t_entier4 * const translate_browse_path_1__res) {
   *translate_browse_path_1__res = translate_browse_path_1__BrowsePathResult_size_i;
}

void translate_browse_path_1__get_BrowsePathResult_IsFull(
   t_bool * const translate_browse_path_1__res) {
   *translate_browse_path_1__res = (translate_browse_path_1__BrowsePathResult_size_i == constants__k_n_BrowsePathResPerElt_max);
}

void translate_browse_path_1__get_BrowsePathResult(
   const t_entier4 translate_browse_path_1__index,
   constants__t_ExpandedNodeId_i * const translate_browse_path_1__nodeId) {
   *translate_browse_path_1__nodeId = translate_browse_path_1__BrowsePathResult_tab_i[translate_browse_path_1__index];
}

void translate_browse_path_1__add_BrowsePathResult(
   const constants__t_ExpandedNodeId_i translate_browse_path_1__nodeId) {
   translate_browse_path_1__BrowsePathResult_size_i = translate_browse_path_1__BrowsePathResult_size_i +
      1;
   translate_browse_path_1__BrowsePathResult_tab_i[translate_browse_path_1__BrowsePathResult_size_i] = translate_browse_path_1__nodeId;
}

void translate_browse_path_1__init_BrowsePathRemaining(void) {
   translate_browse_path_1__BrowsePathRemainingNodeId_size_i = 0;
   {
      t_entier4 i;
      for (i = constants__t_BrowsePathResPerElt_i_max; 0 <= i; i = i - 1) {
         translate_browse_path_1__BrowsePathRemainingNodeId_tab_i[i] = constants__c_ExpandedNodeId_indet;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_BrowsePathResPerElt_i_max; 0 <= i; i = i - 1) {
         translate_browse_path_1__BrowsePathRemainingIndex_tab_i[i] = 0;
      }
   }
}

void translate_browse_path_1__get_BrowsePathRemainingSize(
   t_entier4 * const translate_browse_path_1__res) {
   *translate_browse_path_1__res = translate_browse_path_1__BrowsePathRemainingNodeId_size_i;
}

void translate_browse_path_1__get_BrowsePathRemaining_IsFull(
   t_bool * const translate_browse_path_1__res) {
   *translate_browse_path_1__res = (translate_browse_path_1__BrowsePathRemainingNodeId_size_i == constants__k_n_BrowsePathResPerElt_max);
}

void translate_browse_path_1__get_BrowsePathRemaining(
   const t_entier4 translate_browse_path_1__index,
   constants__t_ExpandedNodeId_i * const translate_browse_path_1__nodeId,
   t_entier4 * const translate_browse_path_1__remainingIndex) {
   *translate_browse_path_1__nodeId = translate_browse_path_1__BrowsePathRemainingNodeId_tab_i[translate_browse_path_1__index];
   *translate_browse_path_1__remainingIndex = translate_browse_path_1__BrowsePathRemainingIndex_tab_i[translate_browse_path_1__index];
}

void translate_browse_path_1__add_BrowsePathResultRemaining(
   const constants__t_ExpandedNodeId_i translate_browse_path_1__nodeId,
   const t_entier4 translate_browse_path_1__index) {
   translate_browse_path_1__BrowsePathRemainingNodeId_size_i = translate_browse_path_1__BrowsePathRemainingNodeId_size_i +
      1;
   translate_browse_path_1__BrowsePathRemainingNodeId_tab_i[translate_browse_path_1__BrowsePathRemainingNodeId_size_i] = translate_browse_path_1__nodeId;
   translate_browse_path_1__BrowsePathRemainingIndex_tab_i[translate_browse_path_1__BrowsePathRemainingNodeId_size_i] = translate_browse_path_1__index;
}

