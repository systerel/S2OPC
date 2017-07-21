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
 * Implements the base machine that "sends" the ReadResponse.
 */


#include <string.h>
#include <stdlib.h>

#include "msg_read_response_bs.h"

#include "internal_msg.h"

#include "sopc_types.h"


/*------------------------
   INITIALISATION Clause
  ------------------------*/
void msg_read_response_bs__INITIALISATION(void)
{
}


/*--------------------
   OPERATIONS Clause
  --------------------*/
void msg_read_response_bs__write_read_response_init(
   const t_entier4 msg_read_response_bs__p_nb_resps,
   const constants__t_msg_i msg_read_response_bs__p_resp_msg,
   t_bool * const msg_read_response_bs__p_isvalid)
{
    OpcUa_ReadResponse *msg_read_resp = (OpcUa_ReadResponse *)(((message__message *)msg_read_response_bs__p_resp_msg)->msg);

    msg_read_resp->NoOfResults = msg_read_response_bs__p_nb_resps;
    msg_read_resp->Results = (SOPC_DataValue *)malloc(sizeof(SOPC_DataValue)*msg_read_response_bs__p_nb_resps);
    if(NULL == msg_read_resp->Results)
    {
        *msg_read_response_bs__p_isvalid = false;
        /* TODO: implement abstract variables */
    }
    else
    {
        *msg_read_response_bs__p_isvalid = true;
    }

    for(int32_t i = 0; i < msg_read_resp->NoOfResults; i++)
        SOPC_DataValue_Initialize(&msg_read_resp->Results[i]);

    msg_read_resp->NoOfDiagnosticInfos = 0;
    msg_read_resp->DiagnosticInfos = NULL;
}


void msg_read_response_bs__set_read_response(
   const constants__t_msg_i msg_read_response_bs__resp_msg,
   const constants__t_ReadValue_i msg_read_response_bs__rvi,
   const constants__t_Variant_i msg_read_response_bs__val,
   const constants__t_StatusCode_i msg_read_response_bs__sc)
{
    OpcUa_ReadResponse *msg_read_resp = (OpcUa_ReadResponse *)(((message__message *)msg_read_response_bs__resp_msg)->msg);

    /* rvi is castable, it's one of its properties, but it starts at 1 */
    if(constants__c_Variant_indet != msg_read_response_bs__val)
        /* Note: the following only copies the context of the the Variant, not the entire Variant */
        memcpy((void *)&msg_read_resp->Results[msg_read_response_bs__rvi-1].Value,
               msg_read_response_bs__val,
               sizeof(SOPC_Variant));
    else
        SOPC_Variant_InitializeAux((void *)&msg_read_resp->Results[msg_read_response_bs__rvi-1].Value);

    /* TODO: Make a util__ that converts the status */
    msg_read_resp->Results[msg_read_response_bs__rvi-1].Status = msg_read_response_bs__sc;
}
