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

 File Name            : msg_translate_browse_path_bs.h

 Date                 : 04/08/2022 14:53:41

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _msg_translate_browse_path_bs_h
#define _msg_translate_browse_path_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "constants_statuscodes_bs.h"
#include "message_in_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void msg_translate_browse_path_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void msg_translate_browse_path_bs__add_BrowsePath_Res_Target(
   const constants__t_BrowsePath_i msg_translate_browse_path_bs__browsePath,
   const constants__t_ExpandedNodeId_i msg_translate_browse_path_bs__node,
   constants_statuscodes_bs__t_StatusCode_i * const msg_translate_browse_path_bs__StatusCode);
extern void msg_translate_browse_path_bs__add_BrowsePath_Res_Target_withRemainingPath(
   const constants__t_BrowsePath_i msg_translate_browse_path_bs__browsePath,
   const constants__t_ExpandedNodeId_i msg_translate_browse_path_bs__node,
   const t_entier4 msg_translate_browse_path_bs__remainingIndex,
   constants_statuscodes_bs__t_StatusCode_i * const msg_translate_browse_path_bs__StatusCode);
extern void msg_translate_browse_path_bs__alloc_BrowsePath_Res_Target(
   const constants__t_BrowsePath_i msg_translate_browse_path_bs__browsePath,
   const t_entier4 msg_translate_browse_path_bs__nbTargetMax,
   constants_statuscodes_bs__t_StatusCode_i * const msg_translate_browse_path_bs__statusCode);
extern void msg_translate_browse_path_bs__alloc_browse_path_result(
   constants_statuscodes_bs__t_StatusCode_i * const msg_translate_browse_path_bs__statusCode);
extern void msg_translate_browse_path_bs__decode_translate_browse_paths_request(
   const constants__t_msg_i msg_translate_browse_path_bs__req_msg,
   constants_statuscodes_bs__t_StatusCode_i * const msg_translate_browse_path_bs__StatusCode_service);
extern void msg_translate_browse_path_bs__free_translate_browse_paths_response(void);
extern void msg_translate_browse_path_bs__read_BrowsePath_RelativePath(
   const constants__t_BrowsePath_i msg_translate_browse_path_bs__browsePath,
   constants__t_RelativePath_i * const msg_translate_browse_path_bs__relativePath);
extern void msg_translate_browse_path_bs__read_BrowsePath_StartingNode(
   const constants__t_BrowsePath_i msg_translate_browse_path_bs__browsePath,
   constants__t_NodeId_i * const msg_translate_browse_path_bs__nodeId);
extern void msg_translate_browse_path_bs__read_RelativePathElt_IncludedSubtypes(
   const constants__t_RelativePathElt_i msg_translate_browse_path_bs__relativePathElt,
   t_bool * const msg_translate_browse_path_bs__includedSubtypes);
extern void msg_translate_browse_path_bs__read_RelativePathElt_IsInverse(
   const constants__t_RelativePathElt_i msg_translate_browse_path_bs__relativePathElt,
   t_bool * const msg_translate_browse_path_bs__isInverse);
extern void msg_translate_browse_path_bs__read_RelativePathElt_ReferenceTypeId(
   const constants__t_RelativePathElt_i msg_translate_browse_path_bs__relativePathElt,
   constants__t_NodeId_i * const msg_translate_browse_path_bs__referenceTypeId);
extern void msg_translate_browse_path_bs__read_RelativePathElt_TargetName(
   const constants__t_RelativePathElt_i msg_translate_browse_path_bs__relativePathElt,
   constants__t_QualifiedName_i * const msg_translate_browse_path_bs__targetName);
extern void msg_translate_browse_path_bs__read_RelativePath_Nb_RelativePathElt(
   const constants__t_RelativePath_i msg_translate_browse_path_bs__relativePath,
   t_entier4 * const msg_translate_browse_path_bs__nb_relativePathElt);
extern void msg_translate_browse_path_bs__read_RelativePath_RelativePathElt(
   const constants__t_RelativePath_i msg_translate_browse_path_bs__relativePath,
   const t_entier4 msg_translate_browse_path_bs__index,
   constants__t_RelativePathElt_i * const msg_translate_browse_path_bs__relativePathElt);
extern void msg_translate_browse_path_bs__read_nb_BrowsePaths(
   t_entier4 * const msg_translate_browse_path_bs__p_nb_BrowsePaths);
extern void msg_translate_browse_path_bs__write_BrowsePath_Res_StatusCode(
   const constants__t_BrowsePath_i msg_translate_browse_path_bs__browsePath,
   const constants_statuscodes_bs__t_StatusCode_i msg_translate_browse_path_bs__statusCode);
extern void msg_translate_browse_path_bs__write_translate_browse_paths_response(
   const constants__t_msg_i msg_translate_browse_path_bs__p_msg_out);

#endif
