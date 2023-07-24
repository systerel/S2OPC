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

 File Name            : translate_browse_path_source.c

 Date                 : 24/07/2023 14:29:25

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "translate_browse_path_source.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void translate_browse_path_source__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void translate_browse_path_source__update_one_browse_path_source(
   const constants__t_NodeId_i translate_browse_path_source__source,
   constants_statuscodes_bs__t_StatusCode_i * const translate_browse_path_source__statusCode_operation) {
   {
      t_bool translate_browse_path_source__l_alloc;
      constants__t_NodeId_i translate_browse_path_source__l_source_copy;
      
      *translate_browse_path_source__statusCode_operation = constants_statuscodes_bs__c_StatusCode_indet;
      node_id_pointer_bs__copy_node_id_pointer_content(translate_browse_path_source__source,
         &translate_browse_path_source__l_alloc,
         &translate_browse_path_source__l_source_copy);
      if (translate_browse_path_source__l_alloc == true) {
         translate_browse_path_source_1__add_BrowsePathSource(translate_browse_path_source__l_source_copy);
         *translate_browse_path_source__statusCode_operation = constants_statuscodes_bs__e_sc_ok;
      }
      else {
         *translate_browse_path_source__statusCode_operation = constants_statuscodes_bs__e_sc_bad_out_of_memory;
      }
   }
}

void translate_browse_path_source__free_BrowsePathSource(void) {
   {
      t_entier4 translate_browse_path_source__l_size;
      t_bool translate_browse_path_source__l_continue;
      t_entier4 translate_browse_path_source__l_index;
      constants__t_NodeId_i translate_browse_path_source__l_nodeId;
      
      translate_browse_path_source_1__get_BrowsePathSourceSize(&translate_browse_path_source__l_size);
      translate_browse_path_source_1_it__init_iter_browsePathSourceIdx(translate_browse_path_source__l_size,
         &translate_browse_path_source__l_continue);
      while (translate_browse_path_source__l_continue == true) {
         translate_browse_path_source_1_it__continue_iter_browsePathSourceIdx(&translate_browse_path_source__l_continue,
            &translate_browse_path_source__l_index);
         translate_browse_path_source_1__get_BrowsePathSource(translate_browse_path_source__l_index,
            &translate_browse_path_source__l_nodeId);
         node_id_pointer_bs__free_node_id_pointer(translate_browse_path_source__l_nodeId);
      }
      translate_browse_path_source_1__init_BrowsePathSource();
   }
}

