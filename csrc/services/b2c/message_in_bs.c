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

/*------------------------
   Exported Declarations
  ------------------------*/
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util_b2c.h"

#include "internal_msg.h"
#include "message_in_bs.h"
#include "message_out_bs.h"

#include "constants_bs.h"

#include "sopc_encoder.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void message_in_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/

void message_in_bs__dealloc_msg_in_header(const constants__t_msg_header_i message_in_bs__msg_header)
{
    // Generated header, parameter not really a const. TODO: Check if message should not be a / in a global variable
    SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
    if ((*(SOPC_EncodeableType**) message_in_bs__msg_header) == &OpcUa_ResponseHeader_EncodeableType)
    {
        SOPC_Encodeable_Delete(&OpcUa_ResponseHeader_EncodeableType, (void**) &message_in_bs__msg_header);
    }
    else if ((*(SOPC_EncodeableType**) message_in_bs__msg_header) == &OpcUa_RequestHeader_EncodeableType)
    {
        SOPC_Encodeable_Delete(&OpcUa_RequestHeader_EncodeableType, (void**) &message_in_bs__msg_header);
    }
    else
    {
        assert(false);
    }
    SOPC_GCC_DIAGNOSTIC_RESTORE
}

void message_in_bs__dealloc_msg_in(const constants__t_msg_i message_in_bs__msg)
{
    // Generated header, parameter not really a const. TODO: Check if message should not be a / in a global variable
    SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
    SOPC_Encodeable_Delete(*(SOPC_EncodeableType**) message_in_bs__msg, (void**) &message_in_bs__msg);
    SOPC_GCC_DIAGNOSTIC_RESTORE
}

void message_in_bs__dealloc_msg_in_buffer(const constants__t_byte_buffer_i message_in_bs__msg_buffer)
{
    // TODO: const modification
    SOPC_Buffer_Delete((SOPC_Buffer*) message_in_bs__msg_buffer);
}

void message_in_bs__decode_msg_type(const constants__t_byte_buffer_i message_in_bs__msg_buffer,
                                    constants__t_msg_type_i* const message_in_bs__msg_type)
{
    *message_in_bs__msg_type = constants__c_msg_type_indet;
    SOPC_EncodeableType* encType = NULL;
    SOPC_ReturnStatus status = SOPC_MsgBodyType_Read((SOPC_Buffer*) message_in_bs__msg_buffer, &encType);
    if (SOPC_STATUS_OK == status && encType != NULL)
    {
        util_message__get_message_type(encType, message_in_bs__msg_type);
    }
}

void message_in_bs__decode_msg_header(const t_bool message_in_bs__is_request,
                                      const constants__t_byte_buffer_i message_in_bs__msg_buffer,
                                      constants__t_msg_header_i* const message_in_bs__msg_header)
{
    *message_in_bs__msg_header = constants__c_msg_header_indet;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    void* header = NULL;
    if (false == message_in_bs__is_request)
    {
        status = SOPC_DecodeMsg_HeaderOrBody((SOPC_Buffer*) message_in_bs__msg_buffer,
                                             &OpcUa_ResponseHeader_EncodeableType, &header);
    }
    else
    {
        status = SOPC_DecodeMsg_HeaderOrBody((SOPC_Buffer*) message_in_bs__msg_buffer,
                                             &OpcUa_RequestHeader_EncodeableType, &header);
    }
    if (SOPC_STATUS_OK == status)
    {
        *message_in_bs__msg_header = header;
    }
}

void message_in_bs__decode_msg(const constants__t_msg_type_i message_in_bs__msg_type,
                               const constants__t_byte_buffer_i message_in_bs__msg_buffer,
                               constants__t_msg_i* const message_in_bs__msg)
{
    *message_in_bs__msg = constants__c_msg_indet;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    SOPC_EncodeableType* reqEncType = NULL;
    SOPC_EncodeableType* respEncType = NULL;
    SOPC_EncodeableType* encType = NULL;
    t_bool isReq = false;
    void* msg = NULL;
    util_message__get_encodeable_type(message_in_bs__msg_type, &reqEncType, &respEncType, &isReq);
    if (false == isReq)
    {
        encType = respEncType;
    }
    else
    {
        encType = reqEncType;
    }

    status = SOPC_DecodeMsg_HeaderOrBody((SOPC_Buffer*) message_in_bs__msg_buffer, encType, &msg);
    if (SOPC_STATUS_OK == status)
    {
        *message_in_bs__msg = (constants__t_msg_i) msg;
    }
}

void message_in_bs__get_msg_in_type(const constants__t_msg_i message_in_bs__msg,
                                    constants__t_msg_type_i* const message_in_bs__msgtype)
{
    message_out_bs__get_msg_out_type(message_in_bs__msg, message_in_bs__msgtype);
}

void message_in_bs__is_valid_msg_in(const constants__t_msg_i message_in_bs__msg, t_bool* const message_in_bs__bres)
{
    message_out_bs__is_valid_msg_out(message_in_bs__msg, message_in_bs__bres);
}

void message_in_bs__is_valid_msg_in_header(const constants__t_msg_header_i message_in_bs__msg_header,
                                           t_bool* const message_in_bs__bres)
{
    message_in_bs__is_valid_msg_in((constants__t_msg_i) message_in_bs__msg_header, message_in_bs__bres);
}

void message_in_bs__is_valid_msg_in_type(const constants__t_msg_type_i message_in_bs__msg_typ,
                                         t_bool* const message_in_bs__bres)
{
    *message_in_bs__bres = message_in_bs__msg_typ != constants__c_msg_type_indet;
}

void message_in_bs__is_valid_request_context(const constants__t_request_context_i message_in_bs__req_context,
                                             t_bool* const message_in_bs__bres)
{
    *message_in_bs__bres = (message_in_bs__req_context != constants__c_request_context_indet);
}

void message_in_bs__msg_in_memory_changed(void)
{
    printf("message_in_bs__msgs_memory_changed\n");
    exit(1);
    ;
}

void message_in_bs__read_activate_msg_user(const constants__t_msg_i message_in_bs__msg,
                                           constants__t_user_i* const message_in_bs__user)
{
    (void) message_in_bs__msg;
    (void) message_in_bs__user;
    // TODO: define anonymous user in B ? Still 1 in C implem for anym
    *message_in_bs__user = 1;
}

void message_in_bs__read_create_session_msg_session_token(
    const constants__t_msg_i message_in_bs__msg,
    constants__t_session_token_i* const message_in_bs__session_token)
{
    OpcUa_CreateSessionResponse* createSessionResp = (OpcUa_CreateSessionResponse*) message_in_bs__msg;
    *message_in_bs__session_token = &createSessionResp->AuthenticationToken;
}

void message_in_bs__read_msg_header_req_handle(const constants__t_msg_header_i message_in_bs__msg_header,
                                               constants__t_request_handle_i* const message_in_bs__handle)
{
    if ((*(SOPC_EncodeableType**) message_in_bs__msg_header) == &OpcUa_ResponseHeader_EncodeableType)
    {
        *message_in_bs__handle = ((OpcUa_ResponseHeader*) message_in_bs__msg_header)->RequestHandle;
    }
    else if ((*(SOPC_EncodeableType**) message_in_bs__msg_header) == &OpcUa_RequestHeader_EncodeableType)
    {
        *message_in_bs__handle = ((OpcUa_RequestHeader*) message_in_bs__msg_header)->RequestHandle;
    }
    else
    {
        assert(false);
    }
}

void message_in_bs__read_msg_req_header_session_token(const constants__t_msg_header_i message_in_bs__msg_header,
                                                      constants__t_session_token_i* const message_in_bs__session_token)
{
    *message_in_bs__session_token = constants__c_session_token_indet;
    // TODO: IMPORTANT: if NULL token  => shall return an indet token !
    *message_in_bs__session_token = &((OpcUa_RequestHeader*) message_in_bs__msg_header)->AuthenticationToken;
}

void message_in_bs__read_msg_resp_header_service_status(const constants__t_msg_header_i message_in_bs__msg_header,
                                                        constants__t_StatusCode_i* const message_in_bs__status)
{
    util_status_code__C_to_B(((OpcUa_ResponseHeader*) message_in_bs__msg_header)->ServiceResult, message_in_bs__status);
}
