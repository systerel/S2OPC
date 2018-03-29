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

 File Name            : address_space.h

 Date                 : 29/03/2018 14:46:07

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _address_space_h
#define _address_space_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "address_space_bs.h"
#include "address_space_it.h"
#include "response_write_bs.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "service_mgr_1.h"
#include "service_response_cb_bs.h"
#include "service_write_decode_bs.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
extern t_bool address_space__ResponseWrite_allocated;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void address_space__INITIALISATION(void);

/*-------------------------------
   PROMOTES and EXTENDS Clauses
  -------------------------------*/
#define address_space__get_BrowseName address_space_bs__get_BrowseName
#define address_space__get_DisplayName address_space_bs__get_DisplayName
#define address_space__get_NodeClass address_space_bs__get_NodeClass
#define address_space__get_Node_RefIndexBegin address_space_bs__get_Node_RefIndexBegin
#define address_space__get_Node_RefIndexEnd address_space_bs__get_Node_RefIndexEnd
#define address_space__get_RefIndex_Reference address_space_bs__get_RefIndex_Reference
#define address_space__get_Reference_IsForward address_space_bs__get_Reference_IsForward
#define address_space__get_Reference_ReferenceType address_space_bs__get_Reference_ReferenceType
#define address_space__get_Reference_TargetNode address_space_bs__get_Reference_TargetNode
#define address_space__get_TypeDefinition address_space_bs__get_TypeDefinition
#define address_space__get_Value_StatusCode address_space_bs__get_Value_StatusCode
#define address_space__read_AddressSpace_free_value address_space_bs__read_AddressSpace_free_value
#define address_space__readall_AddressSpace_Node address_space_bs__readall_AddressSpace_Node
#define address_space__write_WriteResponse_msg_out response_write_bs__write_WriteResponse_msg_out

/*--------------------------
   LOCAL_OPERATIONS Clause
  --------------------------*/
extern void address_space__treat_write_1(const t_bool address_space__isvalid,
                                         const constants__t_StatusCode_i address_space__status,
                                         const constants__t_NodeId_i address_space__nid,
                                         const constants__t_AttributeId_i address_space__aid,
                                         const constants__t_Variant_i address_space__value,
                                         constants__t_StatusCode_i* const address_space__sc);
extern void address_space__treat_write_2(const constants__t_NodeId_i address_space__nid,
                                         const constants__t_AttributeId_i address_space__aid,
                                         const constants__t_Variant_i address_space__value,
                                         constants__t_StatusCode_i* const address_space__sc);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void address_space__alloc_write_request_responses(const t_entier4 address_space__nb_req,
                                                         t_bool* const address_space__bret);
extern void address_space__dealloc_write_request_responses(void);
extern void address_space__read_NodeClass_Attribute(const constants__t_Node_i address_space__node,
                                                    const constants__t_AttributeId_i address_space__aid,
                                                    constants__t_StatusCode_i* const address_space__sc,
                                                    constants__t_NodeClass_i* const address_space__ncl,
                                                    constants__t_Variant_i* const address_space__val);
extern void address_space__treat_write_request_WriteValues(
    constants__t_StatusCode_i* const address_space__StatusCode_service);

#endif
