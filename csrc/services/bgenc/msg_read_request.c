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

 File Name            : msg_read_request.c

 Date                 : 15/02/2018 15:01:12

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "msg_read_request.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
t_entier4 msg_read_request__nb_ReadValue;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void msg_read_request__INITIALISATION(void)
{
    msg_read_request__nb_ReadValue = 0;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void msg_read_request__check_ReadRequest(const constants__t_msg_i msg_read_request__p_msg,
                                         t_bool* const msg_read_request__p_read_ok,
                                         t_entier4* const msg_read_request__p_nb_ReadValue,
                                         constants__t_TimestampsToReturn_i* const msg_read_request__p_tsToReturn,
                                         constants__t_StatusCode_i* const msg_read_request__p_statusCode)
{
    msg_read_request_bs__read_req_nb_ReadValue(msg_read_request__p_msg, msg_read_request__p_nb_ReadValue);
    *msg_read_request__p_read_ok = ((0 < *msg_read_request__p_nb_ReadValue) &&
                                    (*msg_read_request__p_nb_ReadValue <= constants__k_n_read_resp_max));
    *msg_read_request__p_tsToReturn = constants__c_TimestampsToReturn_indet;
    *msg_read_request__p_statusCode = constants__c_StatusCode_indet;
    if (*msg_read_request__p_read_ok == true)
    {
        msg_read_request__nb_ReadValue = *msg_read_request__p_nb_ReadValue;
        msg_read_request_bs__read_req_TimestampsToReturn(msg_read_request__p_msg, msg_read_request__p_tsToReturn);
        if (*msg_read_request__p_tsToReturn == constants__c_TimestampsToReturn_indet)
        {
            *msg_read_request__p_read_ok = false;
            *msg_read_request__p_statusCode = constants__e_sc_bad_timestamps_to_return_invalid;
        }
        else
        {
            msg_read_request_bs__read_req_MaxAge(msg_read_request__p_msg, msg_read_request__p_read_ok);
            if (*msg_read_request__p_read_ok == false)
            {
                *msg_read_request__p_statusCode = constants__e_sc_bad_max_age_invalid;
            }
            else
            {
                *msg_read_request__p_statusCode = constants__e_sc_ok;
            }
        }
    }
    else
    {
        if (*msg_read_request__p_nb_ReadValue <= 0)
        {
            *msg_read_request__p_statusCode = constants__e_sc_bad_nothing_to_do;
        }
        else if (*msg_read_request__p_nb_ReadValue > constants__k_n_read_resp_max)
        {
            *msg_read_request__p_statusCode = constants__e_sc_bad_too_many_ops;
        }
    }
}

void msg_read_request__getall_ReadValue_NodeId_AttributeId(const constants__t_msg_i msg_read_request__p_msg,
                                                           const constants__t_ReadValue_i msg_read_request__p_rvi,
                                                           t_bool* const msg_read_request__p_isvalid,
                                                           constants__t_NodeId_i* const msg_read_request__p_nid,
                                                           constants__t_AttributeId_i* const msg_read_request__p_aid)
{
    msg_read_request_bs__getall_req_ReadValue_NodeId(msg_read_request__p_msg, msg_read_request__p_rvi,
                                                     msg_read_request__p_isvalid, msg_read_request__p_nid);
    if (*msg_read_request__p_isvalid == true)
    {
        msg_read_request_bs__getall_req_ReadValue_AttributeId(msg_read_request__p_msg, msg_read_request__p_rvi,
                                                              msg_read_request__p_isvalid, msg_read_request__p_aid);
    }
    else
    {
        *msg_read_request__p_aid = constants__c_AttributeId_indet;
    }
}

void msg_read_request__get_nb_ReadValue(t_entier4* const msg_read_request__p_nb_ReadValue)
{
    *msg_read_request__p_nb_ReadValue = msg_read_request__nb_ReadValue;
}
