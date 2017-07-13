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
 * Implements the structures behind the address space.
 */


#include "b2c.h"
#include "service_write_decode_bs.h"

#include "sopc_types.h"
#include "address_space_impl.h" /* e_aid_* */


/* Globals */
static OpcUa_WriteRequest *request;


/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void service_write_decode_bs__INITIALISATION(void)
{
    request = NULL;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void service_write_decode_bs__decode_write_request(
   const constants__t_ByteString_i service_write_decode_bs__req_payload,
   constants__t_StatusCode_i * const service_write_decode_bs__StatusCode_service)
{
    /* TODO: this is were you think you have a payload, because the variable is called "payload",
       but in fact you have a OpcUa_WriteRequest *, because you did not understand that
       the generic decoders were not that generic, and required the whole OPC stack to function properly. */
    OpcUa_WriteRequest *req = (OpcUa_WriteRequest *)service_write_decode_bs__req_payload;
    *service_write_decode_bs__StatusCode_service = constants__e_sc_nok;

    if(0 == req->NoOfNodesToWrite || constants__k_n_WriteResponse_max < req->NoOfNodesToWrite)
    {
        /* TODO: req shall not be freed before request is null... */
        request = req;
        *service_write_decode_bs__StatusCode_service = constants__e_sc_ok;
    }
}


extern void service_write_decode_bs__free_write_request(void)
{
    /* TODO: don't free the request that you did not initialize */
    request = NULL;
}


extern void service_write_decode_bs__get_nb_WriteValue(
   t_entier4 * const service_write_decode_bs__nb_req)
{
    /* TODO: does B prevent this operation from being called before decode ? */
    if(NULL != request)
        *service_write_decode_bs__nb_req = request->NoOfNodesToWrite;
    else
        *service_write_decode_bs__nb_req = 0;
}


/**
 * Note: When using request as a OpcUa_WriteRequest,
 * \p nid and \p value are borrowed from request,
 * you should not free them.
 */
extern void service_write_decode_bs__getall_WriteValue(
   const constants__t_WriteValue_i service_write_decode_bs__wvi,
   t_bool * const service_write_decode_bs__isvalid,
   constants__t_NodeId_i * const service_write_decode_bs__nid,
   constants__t_AttributeId_i * const service_write_decode_bs__aid,
   constants__t_Variant_i * const service_write_decode_bs__value)
{
    /* Failure reasons:
       - wvi is too high
       - invalid attribute id
       - TODO: does B prevent this operation from being called before decode ?
    */
    uint32_t aid;
    OpcUa_WriteValue *wv;

    *service_write_decode_bs__isvalid = false;

    if(NULL != request && service_write_decode_bs__wvi <= request->NoOfNodesToWrite)
    {
        wv = &request->NodesToWrite[service_write_decode_bs__wvi-1];
        *service_write_decode_bs__isvalid = true;
        *service_write_decode_bs__nid = (constants__t_NodeId_i)&wv->NodeId;
        aid = wv->AttributeId;
        switch(aid)
        {
        case e_aid_NodeId:
            *service_write_decode_bs__aid = constants__e_aid_NodeId;
            break;
        case e_aid_NodeClass:
            *service_write_decode_bs__aid = constants__e_aid_NodeClass;
            break;
        case e_aid_Value:
            *service_write_decode_bs__aid = constants__e_aid_Value;
            break;
        default:
            *service_write_decode_bs__isvalid = false;
            break;
        }
        *service_write_decode_bs__value = (constants__t_Variant_i)&wv->Value.Value;
    }
}

