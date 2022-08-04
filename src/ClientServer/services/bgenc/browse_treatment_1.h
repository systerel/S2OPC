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

 File Name            : browse_treatment_1.h

 Date                 : 04/08/2022 14:53:00

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _browse_treatment_1_h
#define _browse_treatment_1_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "address_space_itf.h"
#include "constants.h"
#include "constants_statuscodes_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void browse_treatment_1__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void browse_treatment_1__Is_RefTypes_Compatible(
   const t_bool browse_treatment_1__p_is_ref_type1,
   const constants__t_NodeId_i browse_treatment_1__p_ref_type1,
   const t_bool browse_treatment_1__p_inc_subtypes,
   const constants__t_NodeId_i browse_treatment_1__p_ref_type2,
   t_bool * const browse_treatment_1__p_ref_types_compat);
extern void browse_treatment_1__get_optional_fields_ReferenceDescription(
   const constants__t_ExpandedNodeId_i browse_treatment_1__p_TargetNodeId,
   constants__t_QualifiedName_i * const browse_treatment_1__p_BrowseName,
   constants__t_LocalizedText_i * const browse_treatment_1__p_DisplayName,
   constants__t_NodeClass_i * const browse_treatment_1__p_NodeClass,
   constants__t_ExpandedNodeId_i * const browse_treatment_1__p_TypeDefinition);
extern void browse_treatment_1__getall_SourceNode_NbRef(
   const constants__t_NodeId_i browse_treatment_1__p_src_nodeid,
   t_bool * const browse_treatment_1__p_isvalid,
   t_entier4 * const browse_treatment_1__p_nb_ref,
   constants__t_Node_i * const browse_treatment_1__p_src_node);

#endif
