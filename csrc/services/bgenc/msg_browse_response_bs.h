/*
 *  Copyright (C) 2018 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/******************************************************************************

 File Name            : msg_browse_response_bs.h

 Date                 : 05/02/2018 16:15:25

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _msg_browse_response_bs_h
#define _msg_browse_response_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "message_out_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void msg_browse_response_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void msg_browse_response_bs__free_browse_result(void);
extern void msg_browse_response_bs__malloc_browse_response(const t_entier4 msg_browse_response_bs__p_nb_bvi,
                                                           t_bool* const msg_browse_response_bs__p_isallocated);
extern void msg_browse_response_bs__malloc_browse_result(const constants__t_BrowseValue_i msg_browse_response_bs__p_bvi,
                                                         const t_entier4 msg_browse_response_bs__p_nb_bri,
                                                         t_bool* const msg_browse_response_bs__p_isallocated);
extern void msg_browse_response_bs__reset_ResponseBrowse_ContinuationPoint(
    const constants__t_BrowseValue_i msg_browse_response_bs__p_bvi);
extern void msg_browse_response_bs__reset_ResponseBrowse_Res_BrowseName(
    const constants__t_BrowseValue_i msg_browse_response_bs__p_bvi,
    const constants__t_BrowseResult_i msg_browse_response_bs__p_bri);
extern void msg_browse_response_bs__reset_ResponseBrowse_Res_DisplayName(
    const constants__t_BrowseValue_i msg_browse_response_bs__p_bvi,
    const constants__t_BrowseResult_i msg_browse_response_bs__p_bri);
extern void msg_browse_response_bs__reset_ResponseBrowse_Res_NodeClass(
    const constants__t_BrowseValue_i msg_browse_response_bs__p_bvi,
    const constants__t_BrowseResult_i msg_browse_response_bs__p_bri);
extern void msg_browse_response_bs__reset_ResponseBrowse_Res_TypeDefinition(
    const constants__t_BrowseValue_i msg_browse_response_bs__p_bvi,
    const constants__t_BrowseResult_i msg_browse_response_bs__p_bri);
extern void msg_browse_response_bs__set_ResponseBrowse_BrowseStatus(
    const constants__t_BrowseValue_i msg_browse_response_bs__p_bvi,
    const constants__t_StatusCode_i msg_browse_response_bs__p_sc);
extern void msg_browse_response_bs__set_ResponseBrowse_ContinuationPoint(
    const constants__t_BrowseValue_i msg_browse_response_bs__p_bvi,
    const constants__t_Reference_i msg_browse_response_bs__p_ref);
extern void msg_browse_response_bs__set_ResponseBrowse_Res_BrowseName(
    const constants__t_BrowseValue_i msg_browse_response_bs__p_bvi,
    const constants__t_BrowseResult_i msg_browse_response_bs__p_bri,
    const constants__t_QualifiedName_i msg_browse_response_bs__p_BrowseName);
extern void msg_browse_response_bs__set_ResponseBrowse_Res_DisplayName(
    const constants__t_BrowseValue_i msg_browse_response_bs__p_bvi,
    const constants__t_BrowseResult_i msg_browse_response_bs__p_bri,
    const constants__t_LocalizedText_i msg_browse_response_bs__p_DisplayName);
extern void msg_browse_response_bs__set_ResponseBrowse_Res_Forwards(
    const constants__t_BrowseValue_i msg_browse_response_bs__p_bvi,
    const constants__t_BrowseResult_i msg_browse_response_bs__p_bri,
    const t_bool msg_browse_response_bs__p_bool);
extern void msg_browse_response_bs__set_ResponseBrowse_Res_NodeClass(
    const constants__t_BrowseValue_i msg_browse_response_bs__p_bvi,
    const constants__t_BrowseResult_i msg_browse_response_bs__p_bri,
    const constants__t_NodeClass_i msg_browse_response_bs__p_NodeClass);
extern void msg_browse_response_bs__set_ResponseBrowse_Res_NodeId(
    const constants__t_BrowseValue_i msg_browse_response_bs__p_bvi,
    const constants__t_BrowseResult_i msg_browse_response_bs__p_bri,
    const constants__t_ExpandedNodeId_i msg_browse_response_bs__p_ExpandedNodeId);
extern void msg_browse_response_bs__set_ResponseBrowse_Res_ReferenceTypeId(
    const constants__t_BrowseValue_i msg_browse_response_bs__p_bvi,
    const constants__t_BrowseResult_i msg_browse_response_bs__p_bri,
    const constants__t_NodeId_i msg_browse_response_bs__p_NodeId);
extern void msg_browse_response_bs__set_ResponseBrowse_Res_TypeDefinition(
    const constants__t_BrowseValue_i msg_browse_response_bs__p_bvi,
    const constants__t_BrowseResult_i msg_browse_response_bs__p_bri,
    const constants__t_ExpandedNodeId_i msg_browse_response_bs__p_TypeDefinition);
extern void msg_browse_response_bs__write_BrowseResponse_msg_out(
    const constants__t_msg_i msg_browse_response_bs__p_msg_out,
    t_bool* const msg_browse_response_bs__p_isvalid);

#endif
