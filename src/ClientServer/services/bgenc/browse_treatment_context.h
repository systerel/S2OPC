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

 File Name            : browse_treatment_context.h

 Date                 : 04/08/2022 14:53:00

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _browse_treatment_context_h
#define _browse_treatment_context_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "browse_treatment_context_bs.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "constants_statuscodes_bs.h"
#include "node_id_pointer_bs.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
extern constants__t_BrowseDirection_i browse_treatment_context__in_BrowseValue_BrowseDirection_i;
extern t_bool browse_treatment_context__in_BrowseValue_IncludeSubtypes_i;
extern constants__t_BrowseNodeClassMask_i browse_treatment_context__in_BrowseValue_NodeClassMask_i;
extern constants__t_NodeId_i browse_treatment_context__in_BrowseValue_NodeId_i;
extern constants__t_NodeId_i browse_treatment_context__in_BrowseValue_ReferenceTypeId_i;
extern constants__t_BrowseResultMask_i browse_treatment_context__in_BrowseValue_ResultMask_i;
extern constants__t_NodeId_i browse_treatment_context__in_BrowseView_i;
extern t_bool browse_treatment_context__in_ReleasePrevContinuationPoint_i;
extern t_entier4 browse_treatment_context__in_maxReferencesPerNode_i;
extern constants__t_session_i browse_treatment_context__in_session_i;
extern t_entier4 browse_treatment_context__in_startIndex_i;
extern t_bool browse_treatment_context__isBrowseValueContextDefined_i;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void browse_treatment_context__INITIALISATION(void);

/*--------------------------
   LOCAL_OPERATIONS Clause
  --------------------------*/
extern void browse_treatment_context__local_clear_browse_value_context(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void browse_treatment_context__clear_browse_value_context(void);
extern void browse_treatment_context__getall_browse_value_context(
   t_entier4 * const browse_treatment_context__p_startIndex,
   constants__t_session_i * const browse_treatment_context__p_session,
   t_entier4 * const browse_treatment_context__p_maxTargetRef,
   constants__t_NodeId_i * const browse_treatment_context__p_browseView,
   constants__t_NodeId_i * const browse_treatment_context__p_nodeId,
   constants__t_BrowseDirection_i * const browse_treatment_context__p_browseDirection,
   t_bool * const browse_treatment_context__p_refType_defined,
   constants__t_NodeId_i * const browse_treatment_context__p_referenceType,
   t_bool * const browse_treatment_context__p_includeSubtypes,
   constants__t_BrowseNodeClassMask_i * const browse_treatment_context__p_nodeClassMask,
   constants__t_BrowseResultMask_i * const browse_treatment_context__p_resultMask,
   t_bool * const browse_treatment_context__p_autoReleaseCP);
extern void browse_treatment_context__is_NodeClass_in_NodeClassMask(
   const constants__t_NodeClass_i browse_treatment_context__p_nodeClass,
   t_bool * const browse_treatment_context__bres);
extern void browse_treatment_context__setall_browse_value_context(
   const t_entier4 browse_treatment_context__p_startIndex,
   const constants__t_session_i browse_treatment_context__p_session,
   const t_entier4 browse_treatment_context__p_maxTargetRef,
   const constants__t_NodeId_i browse_treatment_context__p_browseView,
   const constants__t_NodeId_i browse_treatment_context__p_nodeId,
   const constants__t_BrowseDirection_i browse_treatment_context__p_browseDirection,
   const constants__t_NodeId_i browse_treatment_context__p_referenceType,
   const t_bool browse_treatment_context__p_includeSubtypes,
   const constants__t_BrowseNodeClassMask_i browse_treatment_context__p_nodeClassMask,
   const constants__t_BrowseResultMask_i browse_treatment_context__p_resultMask,
   const t_bool browse_treatment_context__p_autoReleaseCP,
   constants_statuscodes_bs__t_StatusCode_i * const browse_treatment_context__p_service_StatusCode);

#endif
