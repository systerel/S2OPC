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

/*------------------------
   Exported Declarations
  ------------------------*/
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "util_b2c.h"

#include "message_in_bs.h"
#include "message_out_bs.h"

#include "constants_bs.h"

#include "opcua_identifiers.h"
#include "sopc_encoder.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"

static SOPC_ExtensionObject nullAnonymousIdentityToken;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void message_in_bs__INITIALISATION(void)
{
    // Note: it cannot be declared statically due to use of OpcUa_AnonymousIdentityToken_EncodeableType
    //       which is imported global data when using a shared DLL for Windows.
    nullAnonymousIdentityToken = (SOPC_ExtensionObject){
        .Encoding = SOPC_ExtObjBodyEncoding_Object,
        .TypeId.NodeId.IdentifierType = SOPC_IdentifierType_Numeric,
        .TypeId.NodeId.Data.Numeric = OpcUaId_AnonymousIdentityToken_Encoding_DefaultBinary,
        .Body.Object.ObjType = &OpcUa_AnonymousIdentityToken_EncodeableType,
        /* When there is no default policyId for the AnonymousIdentityToken, it is unnecessary to even malloc it */
        .Body.Object.Value = NULL};
}

/*--------------------
   OPERATIONS Clause
  --------------------*/

void message_in_bs__bless_msg_in(const constants__t_msg_i message_in_bs__msg)
{
    /* NOTHING TO DO: in B model now message_in_bs__msg = c_msg_in now */
    SOPC_UNUSED_ARG(message_in_bs__msg);
}

void message_in_bs__copy_msg_resp_header_into_msg(const constants__t_msg_header_i message_in_bs__msg_header,
                                                  const constants__t_msg_i message_in_bs__msg)
{
    message_out_bs__copy_msg_resp_header_into_msg_out(message_in_bs__msg_header, message_in_bs__msg);
}

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
    SOPC_Buffer_Delete(message_in_bs__msg_buffer);
}

void message_in_bs__decode_msg_type(const constants__t_byte_buffer_i message_in_bs__msg_buffer,
                                    constants__t_msg_type_i* const message_in_bs__msg_type)
{
    *message_in_bs__msg_type = constants__c_msg_type_indet;
    SOPC_EncodeableType* encType = NULL;
    SOPC_ReturnStatus status = SOPC_MsgBodyType_Read(message_in_bs__msg_buffer, &encType);
    if (SOPC_STATUS_OK == status && encType != NULL)
    {
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER, "Services: decoded input message type = '%s'",
                               SOPC_EncodeableType_GetName(encType));

        util_message__get_message_type(encType, message_in_bs__msg_type);
    }
}

void message_in_bs__forget_resp_msg_in(const constants__t_msg_header_i message_in_bs__msg_header,
                                       const constants__t_msg_i message_in_bs__msg)
{
    SOPC_UNUSED_ARG(message_in_bs__msg);
    // In this case the message header shall have been copied into msg, we should free the header structure since then
    // Message structure dealloaction is now responsibility of the user application
    SOPC_Free(message_in_bs__msg_header);
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
        status = SOPC_DecodeMsg_HeaderOrBody(message_in_bs__msg_buffer, &OpcUa_ResponseHeader_EncodeableType, &header);

        if (SOPC_STATUS_OK == status)
        {
            SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "Services: decoded input message header statusCode= '%X'",
                                   ((OpcUa_ResponseHeader*) header)->ServiceResult);
        }
    }
    else
    {
        status = SOPC_DecodeMsg_HeaderOrBody(message_in_bs__msg_buffer, &OpcUa_RequestHeader_EncodeableType, &header);
    }
    if (SOPC_STATUS_OK == status)
    {
        *message_in_bs__msg_header = header;
    }
}

void message_in_bs__decode_service_fault_msg_req_handle(
    const constants__t_byte_buffer_i message_in_bs__msg_buffer,
    constants__t_client_request_handle_i* const message_in_bs__handle)
{
    *message_in_bs__handle = constants__c_client_request_handle_indet;
    constants__t_msg_header_i message_in_bs__msg_header;
    // Backup buffer position
    uint32_t positionBackup = message_in_bs__msg_buffer->position;
    message_in_bs__decode_msg_header(false, message_in_bs__msg_buffer, &message_in_bs__msg_header);
    if (constants__c_msg_header_indet != message_in_bs__msg_header)
    {
        message_in_bs__client_read_msg_header_req_handle(message_in_bs__msg_header, message_in_bs__handle);
        message_in_bs__dealloc_msg_in_header(message_in_bs__msg_header);
        // Restore initial position
        SOPC_ReturnStatus retStatus = SOPC_Buffer_SetPosition(message_in_bs__msg_buffer, positionBackup);
        assert(SOPC_STATUS_OK == retStatus);
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

    status = SOPC_DecodeMsg_HeaderOrBody(message_in_bs__msg_buffer, encType, &msg);
    if (SOPC_STATUS_OK == status)
    {
        *message_in_bs__msg = (constants__t_msg_i) msg;
    }
    else
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "Services: Failed to decode input message type = '%s'",
                               SOPC_EncodeableType_GetName(encType));
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
    message_out_bs__is_valid_msg_out_header((constants__t_msg_i) message_in_bs__msg_header, message_in_bs__bres);
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

void message_in_bs__is_valid_app_msg_in(const constants__t_msg_i message_in_bs__msg,
                                        t_bool* const message_in_bs__bres,
                                        constants__t_msg_type_i* const message_in_bs__msg_typ)
{
    // Since message is provided from application, we have to check it is non NULL and the message type is known
    *message_in_bs__msg_typ = constants__c_msg_type_indet;
    *message_in_bs__bres = false;
    if (message_in_bs__msg != constants__c_msg_indet)
    {
        message_in_bs__get_msg_in_type(message_in_bs__msg, message_in_bs__msg_typ);
        if (*message_in_bs__msg_typ != constants__c_msg_type_indet)
        {
            *message_in_bs__bres = true;
        }
    }
}

void message_in_bs__read_activate_req_msg_identity_token(const constants__t_msg_i message_in_bs__p_msg,
                                                         t_bool* const message_in_bs__p_valid_user_token,
                                                         constants__t_user_token_i* const message_in_bs__p_user_token)
{
    *message_in_bs__p_valid_user_token = false;
    OpcUa_ActivateSessionRequest* activateSessionReq = (OpcUa_ActivateSessionRequest*) message_in_bs__p_msg;

    if (activateSessionReq->UserIdentityToken.Length > 0)
    {
        if (activateSessionReq->UserIdentityToken.TypeId.NodeId.IdentifierType == SOPC_IdentifierType_Numeric &&
            activateSessionReq->UserIdentityToken.TypeId.NodeId.Namespace == OPCUA_NAMESPACE_INDEX)
        {
            bool isAnonymous =
                OpcUaId_AnonymousIdentityToken == activateSessionReq->UserIdentityToken.TypeId.NodeId.Data.Numeric ||
                OpcUaId_AnonymousIdentityToken_Encoding_DefaultBinary ==
                    activateSessionReq->UserIdentityToken.TypeId.NodeId.Data.Numeric ||
                OpcUaId_AnonymousIdentityToken_Encoding_DefaultXml ==
                    activateSessionReq->UserIdentityToken.TypeId.NodeId.Data.Numeric;
            bool isUserName =
                OpcUaId_UserNameIdentityToken == activateSessionReq->UserIdentityToken.TypeId.NodeId.Data.Numeric ||
                OpcUaId_UserNameIdentityToken_Encoding_DefaultBinary ==
                    activateSessionReq->UserIdentityToken.TypeId.NodeId.Data.Numeric ||
                OpcUaId_UserNameIdentityToken_Encoding_DefaultXml ==
                    activateSessionReq->UserIdentityToken.TypeId.NodeId.Data.Numeric;
            /* TODO: Add support for X509 user identity token */
            if (isAnonymous || isUserName)
            {
                *message_in_bs__p_valid_user_token = true;
                *message_in_bs__p_user_token = &activateSessionReq->UserIdentityToken;
            }
        }
    }
    else
    {
        /* NULL value is also the anonymous user identity token */
        *message_in_bs__p_valid_user_token = true;
        *message_in_bs__p_user_token = &nullAnonymousIdentityToken;
    }
}

void message_in_bs__read_activate_req_msg_locales(const constants__t_msg_i message_in_bs__p_msg,
                                                  constants__t_LocaleIds_i* const message_in_bs__p_localeIds)
{
    OpcUa_ActivateSessionRequest* activateSessionReq = (OpcUa_ActivateSessionRequest*) message_in_bs__p_msg;
    if (activateSessionReq->NoOfLocaleIds > 0)
    {
        *message_in_bs__p_localeIds =
            SOPC_String_GetCStringArray(activateSessionReq->NoOfLocaleIds, activateSessionReq->LocaleIds);
    }
    else
    {
        *message_in_bs__p_localeIds = constants__c_LocaleIds_indet;
    }
}

void message_in_bs__read_create_session_msg_session_token(
    const constants__t_msg_i message_in_bs__msg,
    constants__t_session_token_i* const message_in_bs__session_token)
{
    OpcUa_CreateSessionResponse* createSessionResp = (OpcUa_CreateSessionResponse*) message_in_bs__msg;
    *message_in_bs__session_token = &createSessionResp->AuthenticationToken;
}

void message_in_bs__server_read_msg_header_req_handle(const constants__t_msg_header_i message_in_bs__msg_header,
                                                      constants__t_server_request_handle_i* const message_in_bs__handle)
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

void message_in_bs__client_read_msg_header_req_handle(const constants__t_msg_header_i message_in_bs__msg_header,
                                                      constants__t_client_request_handle_i* const message_in_bs__handle)
{
    message_in_bs__server_read_msg_header_req_handle(message_in_bs__msg_header, message_in_bs__handle);
}

void message_in_bs__read_msg_req_header_session_token(const constants__t_msg_header_i message_in_bs__msg_header,
                                                      constants__t_session_token_i* const message_in_bs__session_token)
{
    *message_in_bs__session_token = constants__c_session_token_indet;
    // TODO: IMPORTANT: if NULL token  => shall return an indet token !
    *message_in_bs__session_token = &((OpcUa_RequestHeader*) message_in_bs__msg_header)->AuthenticationToken;
}

void message_in_bs__read_msg_resp_header_service_status(
    const constants__t_msg_header_i message_in_bs__msg_header,
    constants_statuscodes_bs__t_StatusCode_i* const message_in_bs__status)
{
    util_status_code__C_to_B(((OpcUa_ResponseHeader*) message_in_bs__msg_header)->ServiceResult, message_in_bs__status);
}
