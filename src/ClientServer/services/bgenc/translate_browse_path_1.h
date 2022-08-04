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

 File Name            : translate_browse_path_1.h

 Date                 : 04/08/2022 14:53:23

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _translate_browse_path_1_h
#define _translate_browse_path_1_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
extern t_entier4 translate_browse_path_1__BrowsePathRemainingIndex_tab_i[constants__t_BrowsePathResPerElt_i_max+1];
extern t_entier4 translate_browse_path_1__BrowsePathRemainingNodeId_size_i;
extern constants__t_ExpandedNodeId_i translate_browse_path_1__BrowsePathRemainingNodeId_tab_i[constants__t_BrowsePathResPerElt_i_max+1];
extern t_entier4 translate_browse_path_1__BrowsePathResult_size_i;
extern constants__t_ExpandedNodeId_i translate_browse_path_1__BrowsePathResult_tab_i[constants__t_BrowsePathResPerElt_i_max+1];
extern t_entier4 translate_browse_path_1__BrowsePathSource_size_i;
extern constants__t_NodeId_i translate_browse_path_1__BrowsePathSource_tab_i[constants__t_BrowsePathResPerElt_i_max+1];

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void translate_browse_path_1__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void translate_browse_path_1__add_BrowsePathResult(
   const constants__t_ExpandedNodeId_i translate_browse_path_1__nodeId);
extern void translate_browse_path_1__add_BrowsePathResultRemaining(
   const constants__t_ExpandedNodeId_i translate_browse_path_1__nodeId,
   const t_entier4 translate_browse_path_1__index);
extern void translate_browse_path_1__add_BrowsePathSource(
   const constants__t_NodeId_i translate_browse_path_1__nodeId);
extern void translate_browse_path_1__get_BrowsePathRemaining(
   const t_entier4 translate_browse_path_1__index,
   constants__t_ExpandedNodeId_i * const translate_browse_path_1__nodeId,
   t_entier4 * const translate_browse_path_1__remainingIndex);
extern void translate_browse_path_1__get_BrowsePathRemainingSize(
   t_entier4 * const translate_browse_path_1__res);
extern void translate_browse_path_1__get_BrowsePathRemaining_IsFull(
   t_bool * const translate_browse_path_1__res);
extern void translate_browse_path_1__get_BrowsePathResult(
   const t_entier4 translate_browse_path_1__index,
   constants__t_ExpandedNodeId_i * const translate_browse_path_1__nodeId);
extern void translate_browse_path_1__get_BrowsePathResultSize(
   t_entier4 * const translate_browse_path_1__res);
extern void translate_browse_path_1__get_BrowsePathResult_IsFull(
   t_bool * const translate_browse_path_1__res);
extern void translate_browse_path_1__get_BrowsePathSource(
   const t_entier4 translate_browse_path_1__index,
   constants__t_NodeId_i * const translate_browse_path_1__nodeId);
extern void translate_browse_path_1__get_BrowsePathSourceSize(
   t_entier4 * const translate_browse_path_1__res);
extern void translate_browse_path_1__init_BrowsePathRemaining(void);
extern void translate_browse_path_1__init_BrowsePathResult(void);
extern void translate_browse_path_1__init_BrowsePathSource(void);

#endif
