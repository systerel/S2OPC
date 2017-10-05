/*
 *  Copyright (C) 2017 Systerel and others.
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

/** \file
 *
 * Implements the base machine that reads a ReadRequest.
 */


#include <stdint.h>
#include <stdio.h>

#include "msg_read_request_bs.h"

#include "sopc_types.h"
#include "address_space_impl.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void msg_read_request_bs__INITIALISATION(void)
{
}


/*--------------------
   OPERATIONS Clause
  --------------------*/
void msg_read_request_bs__getall_req_ReadValue_AttributeId(
   const constants__t_msg_i msg_read_request_bs__msg,
   const constants__t_ReadValue_i msg_read_request_bs__rvi,
   t_bool * const msg_read_request_bs__isvalid,
   constants__t_AttributeId_i * const msg_read_request_bs__aid)
{
    *msg_read_request_bs__aid = constants__c_AttributeId_indet;
    /* TODO: is message type checked at this point? */
    OpcUa_ReadRequest *msg_read_req = (OpcUa_ReadRequest *)msg_read_request_bs__msg;

    *msg_read_request_bs__isvalid = false;
    if(! msg_read_req)
        return;
    if(msg_read_request_bs__rvi > msg_read_req->NoOfNodesToRead)
        return;
    if(! msg_read_req->NodesToRead)
        return;

    *msg_read_request_bs__isvalid = true;
    switch(msg_read_req->NodesToRead[msg_read_request_bs__rvi-1].AttributeId)
    {
    case e_aid_NodeId:
        *msg_read_request_bs__aid = constants__e_aid_NodeId;
        break;
    case e_aid_NodeClass:
        *msg_read_request_bs__aid = constants__e_aid_NodeClass;
        break;
    case e_aid_BrowseName:
        *msg_read_request_bs__aid = constants__e_aid_BrowseName;
        break;
    case e_aid_DisplayName:
        *msg_read_request_bs__aid = constants__e_aid_DisplayName;
        break;
    case e_aid_Value:
        *msg_read_request_bs__aid = constants__e_aid_Value;
        break;
    default:
        printf("msg_read_request_bs__getall_req_ReadValue_AttributeId: unsupported attribute id\n");
        *msg_read_request_bs__isvalid = false;
        break;
    }
}


void msg_read_request_bs__getall_req_ReadValue_NodeId(
   const constants__t_msg_i msg_read_request_bs__msg,
   const constants__t_ReadValue_i msg_read_request_bs__rvi,
   t_bool * const msg_read_request_bs__isvalid,
   constants__t_NodeId_i * const msg_read_request_bs__nid)
{
    *msg_read_request_bs__nid = constants__c_NodeId_indet;
    /* TODO: is message type checked at this point? */
    OpcUa_ReadRequest *msg_read_req = (OpcUa_ReadRequest *) msg_read_request_bs__msg;

    *msg_read_request_bs__isvalid = false;
    if(! msg_read_req)
        return;
    if(msg_read_request_bs__rvi > msg_read_req->NoOfNodesToRead)
        return;
    if(! msg_read_req->NodesToRead)
        return;

    /* TODO: this should raise a warning, constants__t_NodeId_i IS the void *... No need to cast to a (void **) */
    *msg_read_request_bs__isvalid = true;
    *msg_read_request_bs__nid = (constants__t_NodeId_i *)&msg_read_req->NodesToRead[msg_read_request_bs__rvi-1].NodeId;
}


void msg_read_request_bs__read_req_nb_ReadValue(
   const constants__t_msg_i msg_read_request_bs__msg,
   t_entier4 * const msg_read_request_bs__nb)
{
    *msg_read_request_bs__nb = 0;
    /* TODO: is message type checked at this point? */
    OpcUa_ReadRequest *msg_read_req = (OpcUa_ReadRequest *) msg_read_request_bs__msg;

    if(! msg_read_req)
        return;

    *msg_read_request_bs__nb = msg_read_req->NoOfNodesToRead;
}

