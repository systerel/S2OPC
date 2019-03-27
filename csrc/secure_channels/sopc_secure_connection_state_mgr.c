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

#include "sopc_secure_connection_state_mgr.h"
#include "sopc_secure_connection_state_mgr_internal.h"
#include "sopc_secure_listener_state_mgr.h"

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "opcua_statuscodes.h"
#include "sopc_crypto_provider.h"

#include "sopc_encodeable.h"
#include "sopc_encoder.h"
#include "sopc_event_timer_manager.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_protocol_constants.h"
#include "sopc_secure_channels_api.h"
#include "sopc_secure_channels_api_internal.h"
#include "sopc_secure_channels_internal_ctx.h"
#include "sopc_singly_linked_list.h"
#include "sopc_sockets_api.h"
#include "sopc_time.h"
#include "sopc_toolkit_config_internal.h"

bool SC_InitNewConnection(uint32_t* newConnectionIdx)
{
    bool result = false;
    SOPC_SecureConnection* scConnection = NULL;

    uint32_t connectionIdx = lastSecureConnectionArrayIdx;
    do
    {
        if (connectionIdx < SOPC_MAX_SECURE_CONNECTIONS)
        {
            connectionIdx++; // Minimum used == 1 && Maximum used == MAX + 1
            if (secureConnectionsArray[connectionIdx].state == SECURE_CONNECTION_STATE_SC_CLOSED)
            {
                result = true;
            }
        }
        else
        {
            connectionIdx = 0; // 0 is reserved for indet => connectionIdx++ == 1 will be tested next time
        }
    } while (connectionIdx != lastSecureConnectionArrayIdx && false == result);

    if (result != false)
    {
        scConnection = &(secureConnectionsArray[connectionIdx]);

        // Initialize TCP message properties
        scConnection->tcpMsgProperties.protocolVersion = SOPC_PROTOCOL_VERSION;
        assert(SOPC_MAX_MESSAGE_LENGTH > SOPC_TCP_UA_MIN_BUFFER_SIZE);
        scConnection->tcpMsgProperties.receiveBufferSize = SOPC_MAX_MESSAGE_LENGTH;
        scConnection->tcpMsgProperties.sendBufferSize = SOPC_MAX_MESSAGE_LENGTH;
        // TODO: reduce size since it includes only the body and not the headers/signature ?
        scConnection->tcpMsgProperties.receiveMaxMessageSize = SOPC_MAX_MESSAGE_LENGTH;
        scConnection->tcpMsgProperties.sendMaxMessageSize = SOPC_MAX_MESSAGE_LENGTH;
        // Note: we do not manage multiple chunks in this version of the toolkit
        scConnection->tcpMsgProperties.receiveMaxChunkCount = 1;
        scConnection->tcpMsgProperties.sendMaxChunkCount = 1;

        // Initialize TCP sequence properties
        scConnection->tcpSeqProperties.sentRequestIds = SOPC_SLinkedList_Create(0);
        if (NULL == scConnection->tcpSeqProperties.sentRequestIds)
        {
            result = false;
        }

        // Initialize state
        scConnection->state = SECURE_CONNECTION_STATE_TCP_INIT;
    }

    if (result != false)
    {
        lastSecureConnectionArrayIdx = connectionIdx;
        *newConnectionIdx = connectionIdx;
    }

    return result;
}

static void SC_Client_ClearPendingRequest(uint32_t id, void* val)
{
    (void) (id);
    SOPC_SentRequestMsg_Context* msgCtx = val;
    if (NULL != msgCtx)
    {
        SOPC_EventTimer_Cancel(msgCtx->timerId);

        switch (msgCtx->msgType)
        {
        case SOPC_MSG_TYPE_SC_MSG:
            // Notifies the upper layer
            SOPC_EventHandler_Post(secureChannelsEventHandler, SC_REQUEST_TIMEOUT, msgCtx->scConnectionIdx, NULL,
                                   msgCtx->requestHandle);
            break;
        default:
            // Other cases are SC level requests pending: nothing to do
            break;
        }
        free(msgCtx);
    }
}

bool SC_CloseConnection(uint32_t connectionIdx, bool socketFailure)
{
    SOPC_SecureConnection* scConnection = NULL;
    bool result = false;
    bool configRes = false;
    if (connectionIdx > 0 && connectionIdx <= SOPC_MAX_SECURE_CONNECTIONS)
    {
        scConnection = &(secureConnectionsArray[connectionIdx]);
        if (scConnection->state != SECURE_CONNECTION_STATE_SC_CLOSED)
        {
            result = true;
            // Clear chunk manager context
            if (scConnection->chunksCtx.chunkInputBuffer != NULL)
            {
                SOPC_Buffer_Delete(scConnection->chunksCtx.chunkInputBuffer);
            }

            // Clear TCP sequence properties
            assert(scConnection->tcpSeqProperties.sentRequestIds != NULL);

            SOPC_SLinkedList_Apply(scConnection->tcpSeqProperties.sentRequestIds, SC_Client_ClearPendingRequest);
            SOPC_SLinkedList_Delete(scConnection->tcpSeqProperties.sentRequestIds);
            scConnection->tcpSeqProperties.sentRequestIds = NULL;

            // Clear TCP asymmetric security properties
            if (scConnection->serverAsymmSecuInfo.clientCertificate != NULL)
            {
                SOPC_KeyManager_Certificate_Free(scConnection->serverAsymmSecuInfo.clientCertificate);
                scConnection->serverAsymmSecuInfo.clientCertificate = NULL;
            }
            // scConnection->serverAsymmSecuInfo.securityPolicyUri => do not free (pointer to config data)
            scConnection->serverAsymmSecuInfo.securityPolicyUri = NULL;

            if (scConnection->cryptoProvider != NULL)
            {
                SOPC_CryptoProvider_Free(scConnection->cryptoProvider);
                scConnection->cryptoProvider = NULL;
            }

            // Clear security sets
            if (scConnection->precedentSecuKeySets.receiverKeySet != NULL)
            {
                SOPC_KeySet_Delete(scConnection->precedentSecuKeySets.receiverKeySet);
                scConnection->precedentSecuKeySets.receiverKeySet = NULL;
            }

            if (scConnection->precedentSecuKeySets.senderKeySet != NULL)
            {
                SOPC_KeySet_Delete(scConnection->precedentSecuKeySets.senderKeySet);
                scConnection->precedentSecuKeySets.senderKeySet = NULL;
            }

            if (scConnection->currentSecuKeySets.receiverKeySet != NULL)
            {
                SOPC_KeySet_Delete(scConnection->currentSecuKeySets.receiverKeySet);
                scConnection->currentSecuKeySets.receiverKeySet = NULL;
            }

            if (scConnection->currentSecuKeySets.senderKeySet != NULL)
            {
                SOPC_KeySet_Delete(scConnection->currentSecuKeySets.senderKeySet);
                scConnection->currentSecuKeySets.senderKeySet = NULL;
            }

            // Clear nonce
            if (scConnection->clientNonce != NULL)
            {
                SOPC_SecretBuffer_DeleteClear(scConnection->clientNonce);
                scConnection->clientNonce = NULL;
            }

            if (socketFailure == false)
            {
                // Close the underlying socket if it is not already closed
                SOPC_Sockets_EnqueueEvent(SOCKET_CLOSE, scConnection->socketIndex, NULL, (uintptr_t) connectionIdx);
            }

            if (scConnection->isServerConnection != false)
            {
                // Remove the connection configuration created on connection establishment
                configRes = SOPC_ToolkitServer_RemoveSecureChannelConfig(scConnection->endpointConnectionConfigIdx);
                if (configRes == false && scConnection->state != SECURE_CONNECTION_STATE_TCP_INIT &&
                    scConnection->state != SECURE_CONNECTION_STATE_TCP_NEGOTIATE &&
                    scConnection->state != SECURE_CONNECTION_STATE_SC_INIT)
                {
                    // Note: configuration is only added after treatment of OPN request: do not consider previous states
                    SOPC_Logger_TraceError("ScStateMgr: SC_CloseConnection: scCfgIdx=%" PRIu32 " not found",
                                           scConnection->endpointConnectionConfigIdx);
                }
            }

            SOPC_KeyManager_AsymmetricKey_Free(scConnection->privateKey);
            SOPC_KeyManager_Certificate_Free(scConnection->serverCertificate);
            SOPC_KeyManager_Certificate_Free(scConnection->clientCertificate);

            // Clear the rest (state=0 <=> SC_CLOSED)
            memset(scConnection, 0, sizeof(SOPC_SecureConnection));
        }
    }
    return result;
}

static uint32_t SC_StartConnectionEstablishTimer(uint32_t connectionIdx)
{
    assert(connectionIdx > 0 && connectionIdx <= SOPC_MAX_SECURE_CONNECTIONS);
    SOPC_Event event;
    event.eltId = connectionIdx;
    event.event = TIMER_SC_CONNECTION_TIMEOUT;
    event.params = NULL;
    event.auxParam = 0;
    return SOPC_EventTimer_Create(secureChannelsTimerEventHandler, event, SOPC_SC_CONNECTION_TIMEOUT_MS);
}

static uint32_t SC_Client_StartOPNrenewTimer(uint32_t connectionIdx, uint32_t timeoutMs)
{
    assert(connectionIdx > 0 && connectionIdx <= SOPC_MAX_SECURE_CONNECTIONS);
    SOPC_Event event;
    event.eltId = connectionIdx;
    event.event = TIMER_SC_CLIENT_OPN_RENEW;
    event.params = NULL;
    event.auxParam = 0;
    return SOPC_EventTimer_Create(secureChannelsTimerEventHandler, event, timeoutMs);
}

static char* SC_ClientTransition_ReceivedErrorMsg(SOPC_Buffer* errBuffer, SOPC_StatusCode* errorStatus)
{
    assert(errBuffer != NULL);
    assert(errorStatus != NULL);
    char* result = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_String reason;
    SOPC_String_Initialize(&reason);

    // Read error code
    status = SOPC_UInt32_Read(errorStatus, errBuffer);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_String_Read(&reason, errBuffer);
    }
    if (SOPC_STATUS_OK == status)
    {
        result = SOPC_String_GetCString(&reason);
    }
    SOPC_String_Clear(&reason);
    return result;
}

static bool SC_Server_SendErrorMsgAndClose(uint32_t scConnectionIdx, SOPC_StatusCode errorStatus, char* reason)
{
    bool result = false;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    SOPC_String tempString;
    SOPC_String_Initialize(&tempString);

    SOPC_Buffer* buffer = SOPC_Buffer_Create(SOPC_TCP_UA_ERR_MIN_MSG_LENGTH + SOPC_TCP_UA_MAX_URL_LENGTH);

    if (buffer != NULL)
    {
        // Let size of the headers for the chunk manager
        status = SOPC_Buffer_SetDataLength(buffer, SOPC_TCP_UA_HEADER_LENGTH);

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_Buffer_SetPosition(buffer, SOPC_TCP_UA_HEADER_LENGTH);
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_UInt32_Write(&errorStatus, buffer);
    }
    if (SOPC_STATUS_OK == status)
    {
        SOPC_String_AttachFromCstring(&tempString, reason);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_String_Write(&tempString, buffer);
    }

    if (SOPC_STATUS_OK == status)
    {
        result = true;

        // Delay SC closure after ERR message treatment will be done by chunks manager
        // IMPORTANT NOTE: will be 2nd event to be treated (see ERR below)
        SOPC_SecureChannels_EnqueueInternalEventAsNext(INT_SC_CLOSE, scConnectionIdx, reason, errorStatus);

        // ERR will be treated before INT_SC_CLOSE since both added as next event
        // IMPORTANT NOTE: will be 1st event to be treated regarding INT_SC_CLOSE
        SOPC_SecureChannels_EnqueueInternalEventAsNext(INT_SC_SND_ERR, scConnectionIdx, (void*) buffer, 0);
    }
    else if (buffer != NULL)
    {
        // Buffer will not be used anymore
        SOPC_Buffer_Delete(buffer);
    }

    SOPC_String_Clear(&tempString);

    return result;
}

static void SC_Client_SendCloseSecureChannelRequestAndClose(SOPC_SecureConnection* scConnection,
                                                            uint32_t scConnectionIdx,
                                                            SOPC_StatusCode errorStatus,
                                                            char* reason)
{
    assert(scConnection != NULL);
    SOPC_Buffer* msgBuffer;
    bool result = false;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    OpcUa_RequestHeader reqHeader;
    OpcUa_RequestHeader_Initialize(&reqHeader);
    OpcUa_CloseSecureChannelRequest cloReq;
    OpcUa_CloseSecureChannelRequest_Initialize(&cloReq);

    msgBuffer = SOPC_Buffer_Create(scConnection->tcpMsgProperties.sendBufferSize);

    if (msgBuffer != NULL)
    {
        // Let size of the headers for the chunk manager
        status = SOPC_Buffer_SetDataLength(msgBuffer, SOPC_UA_SECURE_MESSAGE_HEADER_LENGTH +
                                                          SOPC_UA_SYMMETRIC_SECURITY_HEADER_LENGTH +
                                                          SOPC_UA_SECURE_MESSAGE_SEQUENCE_LENGTH);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_Buffer_SetPosition(msgBuffer, SOPC_UA_SECURE_MESSAGE_HEADER_LENGTH +
                                                            SOPC_UA_SYMMETRIC_SECURITY_HEADER_LENGTH +
                                                            SOPC_UA_SECURE_MESSAGE_SEQUENCE_LENGTH);
        }

        // Fill header
        reqHeader.RequestHandle = scConnectionIdx;
        // TODO: reqHeader.AuditEntryId ?
        reqHeader.Timestamp = SOPC_Time_GetCurrentTimeUTC();
        reqHeader.TimeoutHint = 0; // TODO: define same timeout as the one set to set SC disconnected without response

        // Fill close request
        // No content to fill: it is an empty body !

        // Encode message in buffer
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_EncodeMsg_Type_Header_Body(msgBuffer, &OpcUa_CloseSecureChannelRequest_EncodeableType,
                                                     &OpcUa_RequestHeader_EncodeableType, (void*) &reqHeader,
                                                     (void*) &cloReq);
        }

        if (SOPC_STATUS_OK == status)
        {
            result = true;
            // Delay SC closure after CLO message treatment will be done by chunks manager
            // IMPORTANT NOTE: will be 2nd event to be treated (see CLO below)
            SOPC_SecureChannels_EnqueueInternalEventAsNext(INT_SC_CLOSE, scConnectionIdx, reason, errorStatus);

            // CLO will be treated before INT_SC_CLOSE since both added as next event
            // IMPORTANT NOTE: will be 1st event to be treated regarding INT_SC_CLOSE
            SOPC_SecureChannels_EnqueueInternalEventAsNext(INT_SC_SND_CLO, scConnectionIdx, (void*) msgBuffer,
                                                           0); // no request Id context in request
        }
        else if (msgBuffer != NULL)
        {
            // Buffer will not be used anymore
            SOPC_Buffer_Delete(msgBuffer);
        }
    }

    OpcUa_RequestHeader_Clear(&reqHeader);
    OpcUa_CloseSecureChannelRequest_Clear(&cloReq);

    if (false == result)
    {
        // Immediatly close the connection if failed
        if (SC_CloseConnection(scConnectionIdx, false) != false)
        {
            // Notify services in case of successful closure
            SOPC_EventHandler_Post(secureChannelsEventHandler, SC_DISCONNECTED, scConnectionIdx, NULL,
                                   OpcUa_BadSecureChannelClosed);
        }
    }
}

// Note: 3 last params are for server side only and can be NULL for a client
static void SC_CloseSecureConnection(
    SOPC_SecureConnection* scConnection,
    uint32_t scConnectionIdx,
    bool immediateClose, /* Flag to indicate if we immediately close the socket connection or gently (error message)*/
    bool socketFailure,  /* Flag indicating if the socket connection is already closed */
    SOPC_StatusCode errorStatus,
    char* reason)
{
    assert((socketFailure && immediateClose) ||
           socketFailure == false); // socketFailure == true => immediateClose == true
    assert(scConnection != NULL);
    uint32_t serverEndpointConfigIdx = 0;
    uint32_t scConfigIdx = scConnection->endpointConnectionConfigIdx;
    const bool isScConnected = (scConnection->state == SECURE_CONNECTION_STATE_SC_CONNECTED ||
                                scConnection->state == SECURE_CONNECTION_STATE_SC_CONNECTED_RENEW);
    if (isScConnected == false)
    {
        // De-activate of SC connection timeout
        SOPC_EventTimer_Cancel(scConnection->connectionTimeoutTimerId);
    }
    if (false == scConnection->isServerConnection)
    {
        // CLIENT case
        if (isScConnected != false)
        {
            // De-activate of security token renew timeout
            SOPC_EventTimer_Cancel(scConnection->secuTokenRenewTimerId);

            // Client shall alway send a close secure channel message request before closing socket

            if (immediateClose == false)
            {
                SC_Client_SendCloseSecureChannelRequestAndClose(scConnection, scConnectionIdx, errorStatus, reason);
            }
            else
            {
                // Immediatly close the connection if failed or socket failure (no close message could be send)
                if (SC_CloseConnection(scConnectionIdx, socketFailure) != false)
                {
                    // Notify services in case of successful closure
                    SOPC_EventHandler_Post(secureChannelsEventHandler, SC_DISCONNECTED, scConnectionIdx, NULL,
                                           errorStatus);
                }
            }
        }
        else if (scConnection->state != SECURE_CONNECTION_STATE_SC_CLOSED &&
                 SC_CloseConnection(scConnectionIdx, socketFailure) != false)
        { // => Immediate close
            // Notify services in case of successful closure
            SOPC_EventHandler_Post(secureChannelsEventHandler, SC_CONNECTION_TIMEOUT,
                                   scConfigIdx, // SC config idx
                                   NULL, 0);
        }
    }
    else
    {
        // SERVER case
        if (scConnection->state != SECURE_CONNECTION_STATE_SC_CLOSED)
        {
            // Secure Channel is NOT closed nor in init state
            if (immediateClose == false)
            {
                bool result = false;
                // Server shall alway send a ERR message before closing socket
                if (OpcUa_BadSecurityChecksFailed == errorStatus)
                {
                    // Reason shall not provide more information in this case
                    reason = "";
                }
                result = SC_Server_SendErrorMsgAndClose(scConnectionIdx, errorStatus, reason);
                if (false == result)
                {
                    immediateClose = true;
                }
            }
            if (immediateClose != false)
            {
                serverEndpointConfigIdx = scConnection->serverEndpointConfigIdx;
                // Immediatly close the connection if failed
                if (SC_CloseConnection(scConnectionIdx, socketFailure) != false)
                {
                    scConnection = NULL; // Closed connection
                    if (isScConnected != false)
                    {
                        // Notify services in case of successful closure of a SC connected
                        SOPC_EventHandler_Post(secureChannelsEventHandler, SC_DISCONNECTED, scConnectionIdx, NULL,
                                               OpcUa_BadSecureChannelClosed);
                    }
                    // Server side: notify listener that connection closed
                    SOPC_SecureChannels_EnqueueInternalEvent(INT_EP_SC_DISCONNECTED, serverEndpointConfigIdx, NULL,
                                                             scConnectionIdx);
                }
            }
        }
    }
}

static bool SC_ReadAndCheckOpcUaMessageType(SOPC_EncodeableType* msgType, SOPC_Buffer* msgBuffer)
{
    assert(msgBuffer != NULL);
    SOPC_EncodeableType* msgEncType = NULL;
    SOPC_ReturnStatus status = SOPC_MsgBodyType_Read(msgBuffer, &msgEncType);
    return (status == SOPC_STATUS_OK) && (msgEncType == msgType);
}

// TODO: for each use case (client/server) the secret buffer is not adapted => avoid useless copy of nonces
static bool SC_DeriveSymmetricKeySets(bool isServer,
                                      SOPC_CryptoProvider* cryptoProvider,
                                      SOPC_SecretBuffer* clientNonce,
                                      SOPC_SecretBuffer* serverNonce,
                                      SOPC_SC_SecurityKeySets* keySets)
{
    assert(cryptoProvider != NULL);
    assert(clientNonce != NULL);
    assert(serverNonce != NULL);
    assert(keySets != NULL);

    bool result = false;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;

    uint32_t encryptKeyLength = 0;
    uint32_t signKeyLength = 0;
    uint32_t initVectorLength = 0;

    status = SOPC_CryptoProvider_DeriveGetLengths(cryptoProvider, &encryptKeyLength, &signKeyLength, &initVectorLength);

    if (SOPC_STATUS_OK == status)
    {
        result = true;
    }

    if (result != false)
    {
        SOPC_SC_SecurityKeySet* pks = NULL;
        keySets->receiverKeySet = SOPC_KeySet_Create();
        keySets->senderKeySet = SOPC_KeySet_Create();
        pks = keySets->receiverKeySet;
        if (NULL != pks)
        {
            pks->signKey = SOPC_SecretBuffer_New(signKeyLength);
            pks->encryptKey = SOPC_SecretBuffer_New(encryptKeyLength);
            pks->initVector = SOPC_SecretBuffer_New(initVectorLength);
        }
        else
        {
            result = false;
        }
        pks = keySets->senderKeySet;
        if (NULL != pks)
        {
            pks->signKey = SOPC_SecretBuffer_New(signKeyLength);
            pks->encryptKey = SOPC_SecretBuffer_New(encryptKeyLength);
            pks->initVector = SOPC_SecretBuffer_New(initVectorLength);
        }
        else
        {
            result = false;
        }
    }

    if (result != false)
    {
        // Generate the symmetric keys
        if (false == isServer)
        {
            status = SOPC_CryptoProvider_DeriveKeySetsClient(
                cryptoProvider, clientNonce, SOPC_SecretBuffer_Expose(serverNonce),
                SOPC_SecretBuffer_GetLength(serverNonce), keySets->senderKeySet, keySets->receiverKeySet);
        }
        else
        {
            status = SOPC_CryptoProvider_DeriveKeySetsServer(cryptoProvider, SOPC_SecretBuffer_Expose(clientNonce),
                                                             SOPC_SecretBuffer_GetLength(clientNonce), serverNonce,
                                                             keySets->receiverKeySet, keySets->senderKeySet);
        }
        if (SOPC_STATUS_OK != status)
        {
            result = false;
        }
    }

    return result;
}

static bool SC_ClientTransition_TcpInit_To_TcpNegotiate(SOPC_SecureConnection* scConnection,
                                                        uint32_t scConnectionIdx,
                                                        uint32_t socketIdx)
{
    assert(scConnection != NULL);
    SOPC_Buffer* msgBuffer;
    SOPC_SecureChannel_Config* scConfig =
        SOPC_ToolkitClient_GetSecureChannelConfig(scConnection->endpointConnectionConfigIdx);
    bool result = false;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    assert(scConnection != NULL);
    assert(scConnection->state == SECURE_CONNECTION_STATE_TCP_INIT);

    // Create OPC UA TCP Hello message
    // Max size of buffer is message minimum size + URL bytes length
    msgBuffer = SOPC_Buffer_Create(SOPC_TCP_UA_HEL_MIN_MSG_LENGTH + SOPC_TCP_UA_MAX_URL_LENGTH);

    if (msgBuffer != NULL && scConfig != NULL)
    {
        // Let size of the header for the chunk manager
        status = SOPC_Buffer_SetDataLength(msgBuffer, SOPC_TCP_UA_HEADER_LENGTH);
        if (SOPC_STATUS_OK == status)
        {
            SOPC_Buffer_SetPosition(msgBuffer, SOPC_TCP_UA_HEADER_LENGTH);
        }
        // Encode Hello message body
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_UInt32_Write(&scConnection->tcpMsgProperties.protocolVersion, msgBuffer);
        }
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_UInt32_Write(&scConnection->tcpMsgProperties.receiveBufferSize, msgBuffer);
        }
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_UInt32_Write(&scConnection->tcpMsgProperties.sendBufferSize, msgBuffer);
        }
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_UInt32_Write(&scConnection->tcpMsgProperties.receiveMaxMessageSize, msgBuffer);
        }
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_UInt32_Write(&scConnection->tcpMsgProperties.receiveMaxChunkCount, msgBuffer);
        }
        if (SOPC_STATUS_OK == status)
        {
            SOPC_String tmpString;
            SOPC_String_Initialize(&tmpString);
            status = SOPC_String_CopyFromCString(&tmpString, scConfig->url);
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_String_Write(&tmpString, msgBuffer);
            }
            SOPC_String_Clear(&tmpString);
        }

        if (SOPC_STATUS_OK == status)
        {
            result = true;
        }
    }

    if (result != false)
    {
        scConnection->socketIndex = socketIdx;
        scConnection->state = SECURE_CONNECTION_STATE_TCP_NEGOTIATE;
        SOPC_SecureChannels_EnqueueInternalEvent(INT_SC_SND_HEL, scConnectionIdx, (void*) msgBuffer, 0);
    }
    else if (msgBuffer != NULL)
    {
        // Buffer will not be used anymore
        SOPC_Buffer_Delete(msgBuffer);
    }

    return result;
}

static void SC_ClientTransition_Connected_To_Disconnected(SOPC_SecureConnection* scConnection, uint32_t scConnectionIdx)
{
    assert(scConnection != NULL);

    assert(scConnection->state == SECURE_CONNECTION_STATE_SC_CONNECTED ||
           scConnection->state == SECURE_CONNECTION_STATE_SC_CONNECTED_RENEW);

    SC_Client_SendCloseSecureChannelRequestAndClose(scConnection, scConnectionIdx, OpcUa_BadSecureChannelClosed,
                                                    "Secure channel requested to be closed by client");
}

static bool SC_ClientTransition_TcpNegotiate_To_ScInit(SOPC_SecureConnection* scConnection, SOPC_Buffer* ackMsgBuffer)
{
    assert(scConnection != NULL);
    assert(ackMsgBuffer != NULL);
    bool result = false;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    uint32_t tempValue = 0;

    assert(scConnection->state == SECURE_CONNECTION_STATE_TCP_NEGOTIATE);
    assert(false == scConnection->isServerConnection);

    result = true;

    // Read the Acknowledge message content

    // Read protocol version of server
    status = SOPC_UInt32_Read(&tempValue, ackMsgBuffer);
    if (SOPC_STATUS_OK == status)
    {
        // Check protocol version compatible
        if (scConnection->tcpMsgProperties.protocolVersion > tempValue)
        {
            // Use last version supported by the server
            scConnection->tcpMsgProperties.protocolVersion = tempValue;
        } // else => server will return the last version it supports
    }

    // ReceiveBufferSize
    if (SOPC_STATUS_OK == status)
    {
        // Read receive buffer size of SERVER
        status = SOPC_UInt32_Read(&tempValue, ackMsgBuffer);
        if (SOPC_STATUS_OK == status)
        {
            // Adapt send buffer size if needed
            if (scConnection->tcpMsgProperties.sendBufferSize > tempValue)
            {
                if (tempValue >= SOPC_TCP_UA_MIN_BUFFER_SIZE)
                { // see mantis #3447 for >= instead of >
                    scConnection->tcpMsgProperties.sendBufferSize = tempValue;
                }
                else
                {
                    status = SOPC_STATUS_INVALID_PARAMETERS;
                }
            }
            else if (scConnection->tcpMsgProperties.sendBufferSize < tempValue)
            {
                // shall not be larger than what requested in Hello message
                status = SOPC_STATUS_INVALID_PARAMETERS;
            }
        }
    }

    // SendBufferSize
    if (SOPC_STATUS_OK == status)
    {
        // Read sending buffer size of SERVER
        status = SOPC_UInt32_Read(&tempValue, ackMsgBuffer);
        if (SOPC_STATUS_OK == status)
        {
            // Check size and adapt receive buffer size if needed
            if (scConnection->tcpMsgProperties.receiveBufferSize > tempValue)
            {
                if (tempValue >= SOPC_TCP_UA_MIN_BUFFER_SIZE)
                { // see mantis #3447 for >= instead of >
                    scConnection->tcpMsgProperties.receiveBufferSize = tempValue;
                }
                else
                {
                    status = SOPC_STATUS_INVALID_PARAMETERS;
                }
            }
            else if (scConnection->tcpMsgProperties.receiveBufferSize < tempValue)
            {
                // shall not be larger than what requested in Hello message
                status = SOPC_STATUS_INVALID_PARAMETERS;
            }
        }
    }

    // MaxMessageSize of SERVER: request received by server => sent by client
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_UInt32_Read(&tempValue, ackMsgBuffer);
        if (SOPC_STATUS_OK == status)
        {
            if (scConnection->tcpMsgProperties.sendMaxMessageSize > tempValue ||
                scConnection->tcpMsgProperties.sendMaxMessageSize == 0)
            {
                scConnection->tcpMsgProperties.sendMaxMessageSize = tempValue;
            }
            // if "<" => OK since it is the maximum size requested by server will never be reached
        }
    }

    // MaxChunkCount of SERVER: request received by server => sent by client
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_UInt32_Read(&tempValue, ackMsgBuffer);
        if (SOPC_STATUS_OK == status)
        {
            if (scConnection->tcpMsgProperties.sendMaxChunkCount > tempValue ||
                scConnection->tcpMsgProperties.sendMaxChunkCount == 0)
            {
                scConnection->tcpMsgProperties.sendMaxChunkCount = tempValue;
            }
        }
    }

    if (SOPC_STATUS_OK != status)
    {
        result = false;
    }

    if (result != false)
    {
        scConnection->state = SECURE_CONNECTION_STATE_SC_INIT;
    }

    return result;
}

static bool SC_ClientTransitionHelper_SendOPN(SOPC_SecureConnection* scConnection,
                                              uint32_t scConnectionIdx,
                                              bool isRenewal)
{
    bool result = false;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    SOPC_SecureChannel_Config* config = NULL;
    SOPC_Buffer* opnMsgBuffer = NULL;
    OpcUa_RequestHeader reqHeader;
    OpcUa_RequestHeader_Initialize(&reqHeader);
    OpcUa_OpenSecureChannelRequest opnReq;
    OpcUa_OpenSecureChannelRequest_Initialize(&opnReq);
    const uint8_t* bytes = NULL;
    uint32_t length = 0;

    config = SOPC_ToolkitClient_GetSecureChannelConfig(scConnection->endpointConnectionConfigIdx);
    assert(config != NULL);

    result = true;

    // Create crypto provider if necessary
    if (false == isRenewal)
    {
        assert(NULL == scConnection->cryptoProvider);
        scConnection->cryptoProvider = SOPC_CryptoProvider_Create(config->reqSecuPolicyUri);
        if (NULL == scConnection->cryptoProvider)
        {
            result = false;
        }
    }

    if (result != false)
    {
        // Write the OPN request message
        opnMsgBuffer = SOPC_Buffer_Create(scConnection->tcpMsgProperties.sendBufferSize);
        if (NULL == opnMsgBuffer)
        {
            result = false;
        }
    }
    // Note: do not let size of headers for chunks manager since it is not a fixed size

    if (result != false)
    {
        // Fill request header
        reqHeader.RequestHandle = scConnectionIdx;
        reqHeader.Timestamp = SOPC_Time_GetCurrentTimeUTC();
        // TODO: reqHeader.AuditEntryId ?
        reqHeader.TimeoutHint = 0; // TODO: define same timeout as the one set to set SC disconnected without response

        // Fill the OPN body
        opnReq.ClientProtocolVersion = scConnection->tcpMsgProperties.protocolVersion;
        if (false == isRenewal)
        {
            opnReq.RequestType = OpcUa_SecurityTokenRequestType_Issue;
        }
        else
        {
            opnReq.RequestType = OpcUa_SecurityTokenRequestType_Renew;
        }

        opnReq.SecurityMode = config->msgSecurityMode;

        if (config->msgSecurityMode != OpcUa_MessageSecurityMode_None)
        {
            status = SOPC_CryptoProvider_GenerateSecureChannelNonce(scConnection->cryptoProvider,
                                                                    &scConnection->clientNonce);

            if (SOPC_STATUS_OK == status)
            {
                length = SOPC_SecretBuffer_GetLength(scConnection->clientNonce);
                if (length <= INT32_MAX)
                {
                    bytes = SOPC_SecretBuffer_Expose(scConnection->clientNonce);
                    status = SOPC_ByteString_CopyFromBytes(&opnReq.ClientNonce, bytes, (int32_t) length);
                }
                else
                {
                    status = SOPC_STATUS_NOK;
                }
            }

            if (status != SOPC_STATUS_OK)
            {
                result = false;
            }
        }

        if (config->requestedLifetime > SOPC_MINIMUM_SECURE_CONNECTION_LIFETIME)
        {
            opnReq.RequestedLifetime = config->requestedLifetime;
        }
        else
        {
            SOPC_Logger_TraceWarning("ScStateMgr: OPN requested lifetime set to minimum value %d instead of %" PRIu32,
                                     SOPC_MINIMUM_SECURE_CONNECTION_LIFETIME, config->requestedLifetime);
            opnReq.RequestedLifetime = SOPC_MINIMUM_SECURE_CONNECTION_LIFETIME;
        }

        // Encode the OPN message
        status =
            SOPC_EncodeMsg_Type_Header_Body(opnMsgBuffer, &OpcUa_OpenSecureChannelRequest_EncodeableType,
                                            &OpcUa_RequestHeader_EncodeableType, (void*) &reqHeader, (void*) &opnReq);

        if (SOPC_STATUS_OK != status)
        {
            result = false;
        }
    }

    if (result != false)
    {
        SOPC_SecureChannels_EnqueueInternalEvent(INT_SC_SND_OPN, scConnectionIdx, (void*) opnMsgBuffer, 0);
    }
    else
    {
        SOPC_Buffer_Delete(opnMsgBuffer);
    }

    OpcUa_RequestHeader_Clear(&reqHeader);
    OpcUa_OpenSecureChannelRequest_Clear(&opnReq);

    return result;
}

static bool SC_ClientTransition_ScInit_To_ScConnecting(SOPC_SecureConnection* scConnection, uint32_t scConnectionIdx)
{
    bool result = false;

    assert(scConnection != NULL);
    assert(scConnection->state == SECURE_CONNECTION_STATE_SC_INIT);
    result = SC_ClientTransitionHelper_SendOPN(scConnection, scConnectionIdx, false);

    if (result != false)
    {
        scConnection->state = SECURE_CONNECTION_STATE_SC_CONNECTING;
    }

    return result;
}

static bool SC_ClientTransition_ScConnected_To_ScConnectedRenew(SOPC_SecureConnection* scConnection,
                                                                uint32_t scConnectionIdx)
{
    bool result = false;

    assert(scConnection != NULL);
    assert(scConnection->state == SECURE_CONNECTION_STATE_SC_CONNECTED);
    result = SC_ClientTransitionHelper_SendOPN(scConnection, scConnectionIdx, true);

    if (result != false)
    {
        scConnection->state = SECURE_CONNECTION_STATE_SC_CONNECTED_RENEW;
    }

    return result;
}

static bool SC_ClientTransitionHelper_ReceiveOPN(SOPC_SecureConnection* scConnection,
                                                 SOPC_SecureChannel_Config* scConfig,
                                                 uint32_t scConnectionIdx,
                                                 SOPC_Buffer* opnRespBuffer,
                                                 bool isOPNrenew)
{
    bool result = false;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    OpcUa_ResponseHeader* respHeader = NULL;
    OpcUa_OpenSecureChannelResponse* opnResp = NULL;

    status = SOPC_DecodeMsg_HeaderOrBody(opnRespBuffer, &OpcUa_ResponseHeader_EncodeableType, (void**) &respHeader);
    if (SOPC_STATUS_OK == status)
    {
        assert(respHeader != NULL);
        result = true;
    }

    if (result != false)
    {
        // TODO: check timestamp ?
        if (respHeader->ServiceResult != SOPC_STATUS_OK)
        {
            result = false;
        }
        if (respHeader->RequestHandle != scConnectionIdx)
        {
            result = false;
        }
    }

    if (result != false)
    {
        // Decode message
        status = SOPC_DecodeMsg_HeaderOrBody(opnRespBuffer, &OpcUa_OpenSecureChannelResponse_EncodeableType,
                                             (void**) &opnResp);
        if (SOPC_STATUS_OK != status)
        {
            result = false;
        }
    }

    if (result != false)
    {
        // Check protocol version
        if (scConnection->tcpMsgProperties.protocolVersion != opnResp->ServerProtocolVersion)
        {
            // Note: since property was already adapted to server on ACK, it shall be the same
            result = false;
        }

        if (isOPNrenew == false)
        {
            // Check chanel Id and security token provided by the server
            if (0 == opnResp->SecurityToken.ChannelId ||
                scConnection->clientSecureChannelId != opnResp->SecurityToken.ChannelId || // same Id at TCP level
                0 == opnResp->SecurityToken.TokenId)
            {
                result = false;
            }
            // Clear temporary context
            scConnection->clientSecureChannelId = 0;
        }
        else
        {
            // Check channel Id provided by the server is the same
            // and security token is defined and different from the current one
            if (opnResp->SecurityToken.ChannelId != scConnection->currentSecurityToken.secureChannelId ||
                0 == opnResp->SecurityToken.TokenId ||
                opnResp->SecurityToken.TokenId == scConnection->currentSecurityToken.tokenId)
            {
                result = false;
            }
        }
    }

    if (result != false)
    {
        // Retrieve the server nonce and generate key sets if applicable
        assert(opnResp != NULL);

        if (OpcUa_MessageSecurityMode_None == scConfig->msgSecurityMode)
        {
            if (opnResp->ServerNonce.Length > 0)
            {
                // Unexpected token
                SOPC_Logger_TraceWarning("ScStateMgr: OPN resp decoding: unexpected token in None security mode");
            }
        }
        else
        {
            assert(scConnection->clientNonce != NULL);

            if (opnResp->ServerNonce.Length <= 0)
            {
                result = false;
            }

            if (result != false)
            {
                SOPC_SecretBuffer* secretServerNonce = SOPC_SecretBuffer_NewFromExposedBuffer(
                    opnResp->ServerNonce.Data, (uint32_t) opnResp->ServerNonce.Length);
                if (secretServerNonce != NULL)
                {
                    result = SC_DeriveSymmetricKeySets(false, scConnection->cryptoProvider, scConnection->clientNonce,
                                                       secretServerNonce, &scConnection->currentSecuKeySets);
                    SOPC_SecretBuffer_DeleteClear(secretServerNonce);
                }
                else
                {
                    result = false;
                }
            }

            SOPC_SecretBuffer_DeleteClear(scConnection->clientNonce);
            scConnection->clientNonce = NULL;
        }
    }

    if (result != false)
    {
        if (isOPNrenew == false)
        {
            // Define the current security token properties
            scConnection->currentSecurityToken.secureChannelId = opnResp->SecurityToken.ChannelId;
            scConnection->currentSecurityToken.tokenId = opnResp->SecurityToken.TokenId;
            scConnection->currentSecurityToken.createdAt = opnResp->SecurityToken.CreatedAt;
            scConnection->currentSecurityToken.revisedLifetime = opnResp->SecurityToken.RevisedLifetime;
            scConnection->currentSecurityToken.lifetimeEndTimeRef = SOPC_TimeReference_AddMilliseconds(
                SOPC_TimeReference_GetCurrent(), scConnection->currentSecurityToken.revisedLifetime);
        }
        else
        {
            // Set current security token properties as previous one
            scConnection->precedentSecurityToken = scConnection->currentSecurityToken;

            // Set new current security token
            scConnection->currentSecurityToken.secureChannelId = opnResp->SecurityToken.ChannelId;
            scConnection->currentSecurityToken.tokenId = opnResp->SecurityToken.TokenId;
            scConnection->currentSecurityToken.createdAt = opnResp->SecurityToken.CreatedAt;
            scConnection->currentSecurityToken.revisedLifetime = opnResp->SecurityToken.RevisedLifetime;
            scConnection->currentSecurityToken.lifetimeEndTimeRef = SOPC_TimeReference_AddMilliseconds(
                SOPC_TimeReference_GetCurrent(), scConnection->currentSecurityToken.revisedLifetime);
        }

        // Add timer to renew the secure channel security token before expiration
        // Part 4 1.03: "Clients should request a new SecurityToken after 75% of its lifetime elapsed"
        scConnection->secuTokenRenewTimerId =
            SC_Client_StartOPNrenewTimer(scConnectionIdx, scConnection->currentSecurityToken.revisedLifetime * 3 / 4);
    }

    SOPC_Encodeable_Delete(&OpcUa_ResponseHeader_EncodeableType, (void**) &respHeader);
    SOPC_Encodeable_Delete(&OpcUa_OpenSecureChannelResponse_EncodeableType, (void**) &opnResp);

    return result;
}

static bool SC_ClientTransition_ScConnecting_To_ScConnected(SOPC_SecureConnection* scConnection,
                                                            uint32_t scConnectionIdx,
                                                            SOPC_Buffer* opnRespBuffer)
{
    assert(scConnection != NULL);
    assert(scConnection->state == SECURE_CONNECTION_STATE_SC_CONNECTING);
    assert(false == scConnection->isServerConnection);
    assert(opnRespBuffer != NULL);
    SOPC_SecureChannel_Config* scConfig =
        SOPC_ToolkitClient_GetSecureChannelConfig(scConnection->endpointConnectionConfigIdx);
    assert(scConfig != NULL);
    bool result = false;

    result = SC_ClientTransitionHelper_ReceiveOPN(scConnection, scConfig, scConnectionIdx, opnRespBuffer, false);

    if (result != false)
    {
        scConnection->state = SECURE_CONNECTION_STATE_SC_CONNECTED;
    }

    return result;
}

static bool SC_ClientTransition_ScConnectedRenew_To_ScConnected(SOPC_SecureConnection* scConnection,
                                                                uint32_t scConnectionIdx,
                                                                SOPC_Buffer* opnRespBuffer)
{
    assert(scConnection != NULL);
    assert(scConnection->state == SECURE_CONNECTION_STATE_SC_CONNECTED_RENEW);
    assert(false == scConnection->isServerConnection);
    assert(opnRespBuffer != NULL);
    SOPC_SecureChannel_Config* scConfig =
        SOPC_ToolkitClient_GetSecureChannelConfig(scConnection->endpointConnectionConfigIdx);
    assert(scConfig != NULL);
    bool result = false;

    result = SC_ClientTransitionHelper_ReceiveOPN(scConnection, scConfig, scConnectionIdx, opnRespBuffer, true);

    if (result != false)
    {
        scConnection->state = SECURE_CONNECTION_STATE_SC_CONNECTED;
    }

    return result;
}

static bool SC_ServerTransition_TcpInit_To_TcpNegotiate(SOPC_SecureConnection* scConnection,
                                                        SOPC_Buffer* helloMsgBuffer,
                                                        SOPC_StatusCode* errorStatus)
{
    // Note: errorStatus must be an error allowed in OPC UA TCP error message (part 6 Table 38)
    assert(scConnection != NULL);
    bool result = false;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_Endpoint_Config* epConfig = NULL;
    SOPC_String epUrl;
    SOPC_String_Initialize(&epUrl);
    SOPC_String url;
    SOPC_String_Initialize(&url);
    uint32_t tempValue = 0;

    assert(scConnection != NULL);
    assert(helloMsgBuffer != NULL);
    assert(scConnection->state == SECURE_CONNECTION_STATE_TCP_INIT);
    assert(scConnection->isServerConnection != false);

    epConfig = SOPC_ToolkitServer_GetEndpointConfig(scConnection->serverEndpointConfigIdx);
    result = true;

    // Read the Hello message content

    // Read protocol version of server
    if (epConfig != NULL)
    {
        status = SOPC_UInt32_Read(&tempValue, helloMsgBuffer);
    }
    else
    {
        result = false;
        *errorStatus = OpcUa_BadInternalError;
    }
    if (result != false && SOPC_STATUS_OK == status)
    {
        // Check protocol version compatible
        if (scConnection->tcpMsgProperties.protocolVersion > tempValue)
        {
            // Use last version supported by the client
            scConnection->tcpMsgProperties.protocolVersion = tempValue;
        } // else => server will return the last version it supports
    }
    else if (result != false)
    {
        result = false;
        *errorStatus = OpcUa_BadDecodingError;
    }

    // ReceiveBufferSize
    if (result != false)
    {
        // Read receive buffer size of CLIENT
        status = SOPC_UInt32_Read(&tempValue, helloMsgBuffer);
        if (SOPC_STATUS_OK == status)
        {
            // Adapt send buffer size if needed
            if (scConnection->tcpMsgProperties.sendBufferSize > tempValue)
            {
                if (tempValue >= SOPC_TCP_UA_MIN_BUFFER_SIZE)
                { // see mantis #3447 for >= instead of >
                    scConnection->tcpMsgProperties.sendBufferSize = tempValue;
                }
                else
                {
                    result = false;
                    *errorStatus = OpcUa_BadInvalidArgument;
                }
            }
        }
        else
        {
            result = false;
            *errorStatus = OpcUa_BadDecodingError;
        }
    }

    // SendBufferSize
    if (result != false)
    {
        // Read sending buffer size of CLIENT
        status = SOPC_UInt32_Read(&tempValue, helloMsgBuffer);
        if (SOPC_STATUS_OK == status)
        {
            // Check size and adapt receive buffer size if needed
            if (scConnection->tcpMsgProperties.receiveBufferSize > tempValue)
            {
                if (tempValue >= SOPC_TCP_UA_MIN_BUFFER_SIZE)
                { // see mantis #3447 for >= instead of >
                    scConnection->tcpMsgProperties.receiveBufferSize = tempValue;
                }
                else
                {
                    result = false;
                    *errorStatus = OpcUa_BadInvalidArgument;
                }
            }
            // In other case do not change size since it should be configured with max size by default
        }
        else
        {
            result = false;
            *errorStatus = OpcUa_BadDecodingError;
        }
    }

    // MaxMessageSize of CLIENT: response received by client => sent by server
    if (result != false)
    {
        status = SOPC_UInt32_Read(&tempValue, helloMsgBuffer);
        if (SOPC_STATUS_OK == status)
        {
            if (scConnection->tcpMsgProperties.sendMaxMessageSize > tempValue ||
                scConnection->tcpMsgProperties.sendMaxMessageSize == 0)
            {
                scConnection->tcpMsgProperties.sendMaxMessageSize = tempValue;
            }
        }
        else
        {
            result = false;
            *errorStatus = OpcUa_BadDecodingError;
        }
    }

    // MaxChunkCount of CLIENT: response received by client => sent by server
    if (result != false)
    {
        status = SOPC_UInt32_Read(&tempValue, helloMsgBuffer);
        if (SOPC_STATUS_OK == status)
        {
            if (scConnection->tcpMsgProperties.sendMaxChunkCount > tempValue ||
                scConnection->tcpMsgProperties.sendMaxChunkCount == 0)
            {
                scConnection->tcpMsgProperties.sendMaxChunkCount = tempValue;
            }
        }
        else
        {
            result = false;
            *errorStatus = OpcUa_BadDecodingError;
        }
    }

    // EndpointURL
    if (result != false)
    {
        status = SOPC_String_Read(&url, helloMsgBuffer);
        // Note: this parameter is normally used to forward to an endpoint sharing the same port
        //       but not in the same process.
        //       This is not supported by S2OPC secure channels layer, as consequence expected URL is only the one
        //       configured.
        if (SOPC_STATUS_OK == status)
        {
            if (url.Length > SOPC_TCP_UA_MAX_URL_LENGTH)
            {
                result = false;
                *errorStatus = OpcUa_BadTcpEndpointUrlInvalid;
            }
            else
            {
                int32_t compareValue;
                status = SOPC_String_AttachFromCstring(&epUrl, epConfig->endpointURL);
                if (SOPC_STATUS_OK == status)
                {
                    status = SOPC_String_Compare(&epUrl, &url, true, &compareValue);
                    if (SOPC_STATUS_OK == status)
                    {
                        if (compareValue != 0)
                        {
                            SOPC_Logger_TraceWarning(
                                "Endpoint URL is not identical to requested URL : %s instead of %s",
                                epConfig->endpointURL, (char*) url.Data);
                        }
                    }
                }
                if (SOPC_STATUS_OK != status)
                {
                    result = false;
                    *errorStatus = OpcUa_BadTcpInternalError;
                }
            }
        }
        else
        {
            result = false;
            *errorStatus = OpcUa_BadDecodingError;
        }
    }

    if (result != false)
    {
        scConnection->state = SECURE_CONNECTION_STATE_TCP_NEGOTIATE;
    }

    SOPC_String_Clear(&epUrl);
    SOPC_String_Clear(&url);

    return result;
}

static bool SC_ServerTransition_TcpNegotiate_To_ScInit(SOPC_SecureConnection* scConnection,
                                                       uint32_t scConnectionIdx,
                                                       SOPC_Buffer* helloMsgBuffer,
                                                       SOPC_StatusCode* errorStatus)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    assert(scConnection != NULL);
    assert(helloMsgBuffer != NULL);
    assert(scConnection->state == SECURE_CONNECTION_STATE_TCP_NEGOTIATE);
    assert(scConnection->isServerConnection != false);

    // Write the Acknowledge message content

    // Reuse the contents of the received message buffer

    SOPC_Buffer* ackMsgBuffer = SOPC_Buffer_Create(helloMsgBuffer->max_size);

    if (ackMsgBuffer == NULL || SOPC_Buffer_SetPosition(helloMsgBuffer, 0) != SOPC_STATUS_OK ||
        SOPC_Buffer_CopyWithLength(ackMsgBuffer, helloMsgBuffer, SOPC_TCP_UA_HEADER_LENGTH) != SOPC_STATUS_OK)
    {
        SOPC_Buffer_Delete(ackMsgBuffer);
        *errorStatus = OpcUa_BadEncodingError;
        return false;
    }

    status = SOPC_Buffer_SetPosition(ackMsgBuffer, SOPC_TCP_UA_HEADER_LENGTH);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Buffer_SetDataLength(ackMsgBuffer, SOPC_TCP_UA_HEADER_LENGTH);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_UInt32_Write(&scConnection->tcpMsgProperties.protocolVersion, ackMsgBuffer);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_UInt32_Write(&scConnection->tcpMsgProperties.receiveBufferSize, ackMsgBuffer);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_UInt32_Write(&scConnection->tcpMsgProperties.sendBufferSize, ackMsgBuffer);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_UInt32_Write(&scConnection->tcpMsgProperties.receiveMaxMessageSize, ackMsgBuffer);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_UInt32_Write(&scConnection->tcpMsgProperties.receiveMaxChunkCount, ackMsgBuffer);
    }

    if (SOPC_STATUS_OK == status)
    {
        *errorStatus = SOPC_GoodGenericStatus;
        scConnection->state = SECURE_CONNECTION_STATE_SC_INIT;
        SOPC_SecureChannels_EnqueueInternalEvent(INT_SC_SND_ACK, scConnectionIdx, ackMsgBuffer, 0);
    }
    else
    {
        *errorStatus = OpcUa_BadEncodingError;
        SOPC_Buffer_Delete(ackMsgBuffer);
    }

    return status == SOPC_STATUS_OK;
}

static bool get_certificate_der(SOPC_Certificate* cert, SOPC_Buffer** buffer)
{
    if (cert == NULL)
    {
        *buffer = NULL;
        return true;
    }

    uint8_t* cert_data = NULL;
    uint32_t cert_len = 0;
    SOPC_Buffer* cert_buffer = NULL;

    if (SOPC_KeyManager_Certificate_CopyDER(cert, &cert_data, &cert_len) != SOPC_STATUS_OK)
    {
        return false;
    }

    cert_buffer = SOPC_Buffer_Attach(cert_data, cert_len);

    if (cert_buffer == NULL)
    {
        free(cert_data);
        return false;
    }

    *buffer = cert_buffer;
    return true;
}

static bool SC_ServerTransition_ScInit_To_ScConnecting(SOPC_SecureConnection* scConnection,
                                                       SOPC_Buffer* opnReqMsgBuffer,
                                                       uint32_t* requestHandle,
                                                       SOPC_StatusCode* errorStatus)
{
    // Important note: errorStatus shall be one of the errors that can be sent with a OPC UA TCP error message (part4
    // Table 38)
    assert(scConnection != NULL);
    assert(opnReqMsgBuffer != NULL);
    assert(requestHandle != NULL);
    assert(scConnection->state == SECURE_CONNECTION_STATE_SC_INIT);
    assert(scConnection->isServerConnection != false);
    assert(scConnection->serverAsymmSecuInfo.securityPolicyUri != NULL); // set by chunks manager
    assert(scConnection->serverAsymmSecuInfo.validSecurityModes != 0);   // set by chunks manager

    bool result = false;
    uint32_t idx;
    bool validSecurityRequested = false;
    SOPC_Endpoint_Config* epConfig = NULL;
    OpcUa_RequestHeader* reqHeader = NULL;
    OpcUa_OpenSecureChannelRequest* opnReq = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    epConfig = SOPC_ToolkitServer_GetEndpointConfig(scConnection->serverEndpointConfigIdx);
    assert(epConfig != NULL);

    status = SOPC_DecodeMsg_HeaderOrBody(opnReqMsgBuffer, &OpcUa_RequestHeader_EncodeableType, (void**) &reqHeader);
    if (SOPC_STATUS_OK == status)
    {
        assert(reqHeader != NULL);
        result = true;
    }

    if (result != false)
    {
        // TODO: check timestamp + timeout ?
        *requestHandle = reqHeader->RequestHandle;
    }

    if (result != false)
    {
        status = SOPC_DecodeMsg_HeaderOrBody(opnReqMsgBuffer, &OpcUa_OpenSecureChannelRequest_EncodeableType,
                                             (void**) &opnReq);
        if (SOPC_STATUS_OK != status)
        {
            result = false;
        }
    }
    if (result != false)

    {
        assert(opnReq != NULL);
        if (scConnection->tcpMsgProperties.protocolVersion != opnReq->ClientProtocolVersion)
        {
            SOPC_Logger_TraceError("ScStateMgr OPN: different protocol version expected=%" PRIu32 " received=%" PRIu32
                                   " epCfgIdx=%" PRIu32 " scCfgIdx=%" PRIu32,
                                   scConnection->tcpMsgProperties.protocolVersion, opnReq->ClientProtocolVersion,
                                   scConnection->serverEndpointConfigIdx, scConnection->endpointConnectionConfigIdx);

            // Note: since property was already adapted to server on HEL, it shall be the same
            //*errorStatus = OpcUa_BadProtocolVersionUnsupported; => not a TCP error message authorized error
            *errorStatus = OpcUa_BadTcpInternalError;
            result = false;
        }
        if (result != false && opnReq->RequestType != OpcUa_SecurityTokenRequestType_Issue)
        {
            SOPC_Logger_TraceError("ScStateMgr OPN: invalid request type epCfgIdx=%" PRIu32 " scCfgIdx=%" PRIu32,
                                   scConnection->serverEndpointConfigIdx, scConnection->endpointConnectionConfigIdx);
            // Cannot renew in SC_Init state
            *errorStatus = OpcUa_BadTcpSecureChannelUnknown;
            result = false;
        }

        // Check it is a valid security policy in endpoint security policy
        switch (opnReq->SecurityMode)
        {
        case OpcUa_MessageSecurityMode_None:
            validSecurityRequested =
                (scConnection->serverAsymmSecuInfo.validSecurityModes & SOPC_SECURITY_MODE_NONE_MASK) != 0;
            break;
        case OpcUa_MessageSecurityMode_Sign:
            validSecurityRequested =
                (scConnection->serverAsymmSecuInfo.validSecurityModes & SOPC_SECURITY_MODE_SIGN_MASK) != 0;
            break;
        case OpcUa_MessageSecurityMode_SignAndEncrypt:
            validSecurityRequested =
                (scConnection->serverAsymmSecuInfo.validSecurityModes & SOPC_SECURITY_MODE_SIGNANDENCRYPT_MASK) != 0;
            break;
        case OpcUa_MessageSecurityMode_Invalid:
        default:
            validSecurityRequested = false;
        }

        // Check the valid security mode requested is the one guessed based on asymmetric security header content
        if (false == validSecurityRequested)
        {
            SOPC_Logger_TraceError(
                "ScStateMgr OPN: invalid security parameters requested=%d epCfgIdx=%" PRIu32 " scCfgIdx=%" PRIu32,
                opnReq->SecurityMode, scConnection->serverEndpointConfigIdx, scConnection->endpointConnectionConfigIdx);
            result = false;
            //*errorStatus = OpcUa_BadSecurityModeRejected; => not a TCP error message authorized error
            *errorStatus = OpcUa_BadSecurityChecksFailed;
        }
        else if (false == scConnection->serverAsymmSecuInfo.isSecureModeActive &&
                 opnReq->SecurityMode != OpcUa_MessageSecurityMode_None)
        {
            SOPC_Logger_TraceError("ScStateMgr OPN: certificates expected epCfgIdx=%" PRIu32 " scCfgIdx=%" PRIu32,
                                   scConnection->serverEndpointConfigIdx, scConnection->endpointConnectionConfigIdx);
            // Certificates were absent in asym. header and it is not compatible with the security mode requested
            *errorStatus = OpcUa_BadSecurityChecksFailed;
            result = false;
        }

        // Check RequestedLifetime parameter
        if (result != false)
        {
            if (opnReq->RequestedLifetime < SOPC_MINIMUM_SECURE_CONNECTION_LIFETIME)
            {
                opnReq->RequestedLifetime = SOPC_MINIMUM_SECURE_CONNECTION_LIFETIME;
            }
        }

        if (result != false)
        {
            if (opnReq->SecurityMode == OpcUa_MessageSecurityMode_None)
            {
                if (opnReq->ClientNonce.Length > 0)
                {
                    // Nonce unexpected
                    SOPC_Logger_TraceWarning(
                        "ScStateMgr: Nonce unexpected in OPN req for None security mode epCfgIdx=%" PRIu32
                        " scCfgIdx=%" PRIu32,
                        scConnection->serverEndpointConfigIdx, scConnection->endpointConnectionConfigIdx);
                }
            }
            else
            {
                assert(opnReq->SecurityMode == OpcUa_MessageSecurityMode_Sign ||
                       opnReq->SecurityMode == OpcUa_MessageSecurityMode_SignAndEncrypt);
                if (opnReq->ClientNonce.Length > 0)
                {
                    scConnection->clientNonce = SOPC_SecretBuffer_NewFromExposedBuffer(
                        opnReq->ClientNonce.Data, (uint32_t) opnReq->ClientNonce.Length);
                    if (NULL == scConnection->clientNonce)
                    {
                        *errorStatus = OpcUa_BadTcpInternalError;
                        result = false;
                    }
                }
                else
                {
                    // Nonce expected
                    SOPC_Logger_TraceError(
                        "ScStateMgr OPN: unexpected Nonce length=0 epCfgIdx=%" PRIu32 " scCfgIdx=%" PRIu32,
                        scConnection->serverEndpointConfigIdx, scConnection->endpointConnectionConfigIdx);
                    result = false;
                    *errorStatus = OpcUa_BadSecurityChecksFailed;
                }
            }
        }

        SOPC_SecureChannel_Config* nconfig = calloc(1, sizeof(SOPC_SecureChannel_Config));
        SOPC_Buffer* cert_buffer = NULL;

        if (nconfig == NULL || !get_certificate_der(scConnection->serverAsymmSecuInfo.clientCertificate, &cert_buffer))
        {
            result = false;
        }

        if (result)
        {
            nconfig->crt_cli = cert_buffer;
            nconfig->crt_srv = epConfig->serverCertificate;
            nconfig->isClientSc = false;
            nconfig->key_priv_cli = NULL;
            nconfig->msgSecurityMode = opnReq->SecurityMode;
            nconfig->pki = epConfig->pki;
            nconfig->reqSecuPolicyUri = scConnection->serverAsymmSecuInfo.securityPolicyUri;
            nconfig->requestedLifetime = opnReq->RequestedLifetime;
            nconfig->url = epConfig->endpointURL;
            idx = SOPC_ToolkitServer_AddSecureChannelConfig(nconfig);
            result = (idx > 0);
        }

        if (result)
        {
            scConnection->endpointConnectionConfigIdx = idx;
            scConnection->clientCertificate = scConnection->serverAsymmSecuInfo.clientCertificate;
        }

        if (result == false)
        {
            free(nconfig);
            free(cert_buffer);
            *errorStatus = OpcUa_BadTcpInternalError;
        }
    }

    if (result != false)
    {
        scConnection->state = SECURE_CONNECTION_STATE_SC_CONNECTING;
    }

    SOPC_Encodeable_Delete(&OpcUa_RequestHeader_EncodeableType, (void**) &reqHeader);
    SOPC_Encodeable_Delete(&OpcUa_OpenSecureChannelRequest_EncodeableType, (void**) &opnReq);
    // Clear the temporary security info recorded by chunks manager
    scConnection->serverAsymmSecuInfo.clientCertificate = NULL;
    scConnection->serverAsymmSecuInfo.securityPolicyUri = NULL;
    scConnection->serverAsymmSecuInfo.validSecurityModes = 0;
    scConnection->serverAsymmSecuInfo.isSecureModeActive = false;

    return result;
}

static bool SC_Server_GenerateFreshSecureChannelAndTokenId(SOPC_SecureConnection* scConnection,
                                                           uint32_t* secureChannelId,
                                                           uint32_t* tokenId)
{
    assert(scConnection->isServerConnection != false);
    assert(secureChannelId != NULL);
    assert(tokenId != NULL);

    bool result = false;
    uint32_t resultTokenId = 0;
    uint32_t resultSecureChannelId = 0;
    SOPC_SecureListener* scListener = &secureListenersArray[scConnection->serverEndpointConfigIdx];

    if (scListener->state == SECURE_LISTENER_STATE_OPENED)
    {
        // Randomize secure channel ids (table 26 part 6)
        uint32_t newSecureChannelId = 0;
        uint32_t newTokenId = 0;
        uint32_t idx = 0;
        uint32_t connectionIdx = 0;
        bool occupiedScId = false;
        bool occupiedTokenId = false;
        uint8_t attempts = 5; // attempts to find a non conflicting secure Id
        while ((resultSecureChannelId == 0 || resultTokenId == 0) && attempts > 0)
        {
            attempts--;
            if (resultSecureChannelId == 0 && SOPC_CryptoProvider_GenerateRandomID(
                                                  scConnection->cryptoProvider, &newSecureChannelId) != SOPC_STATUS_OK)
            {
                continue;
            }
            if (resultTokenId == 0 &&
                SOPC_CryptoProvider_GenerateRandomID(scConnection->cryptoProvider, &newTokenId) != SOPC_STATUS_OK)
            {
                continue;
            }
            occupiedScId = false;
            occupiedTokenId = false;
            // A server cannot attribute 0 as secure channel id:
            //  not so clear but implied by 6.7.6 part 6: "may be 0 if the Message is an OPN"
            if (newSecureChannelId != 0 && newTokenId != 0)
            {
                // Check if other channels already use the random id in existing connections
                for (idx = 0; idx < SOPC_MAX_SOCKETS_CONNECTIONS && (false == occupiedScId || false == occupiedTokenId);
                     idx++)
                {
                    if (scListener->isUsedConnectionIdxArray[idx] != false)
                    {
                        connectionIdx = scListener->connectionIdxArray[idx];
                        if (secureConnectionsArray[connectionIdx].state != SECURE_CONNECTION_STATE_SC_CLOSED)
                        {
                            if (newSecureChannelId ==
                                secureConnectionsArray[connectionIdx].currentSecurityToken.secureChannelId)
                            {
                                // If same SC id we will have to generate a new one
                                occupiedScId = true;
                            }
                            if (newTokenId == secureConnectionsArray[connectionIdx].currentSecurityToken.tokenId)
                            {
                                // If same SC id we will have to generate a new one
                                occupiedTokenId = true;
                            }
                        }
                    }
                }
                if (false == occupiedScId)
                {
                    // Id is not used by another channel in the endpoint:
                    resultSecureChannelId = newSecureChannelId;
                }
                if (false == occupiedTokenId)
                {
                    // Id is not used by another channel in the endpoint:
                    resultTokenId = newTokenId;
                }
            }
        }
        if (resultSecureChannelId != 0 && resultTokenId != 0)
        {
            *secureChannelId = resultSecureChannelId;
            *tokenId = resultTokenId;
            result = true;
        }
    }
    return result;
}

// TODO: to be used for renew
static uint32_t SC_Server_GenerateFreshTokenId(SOPC_SecureConnection* scConnection)
{
    assert(scConnection->isServerConnection != false);

    uint32_t resultTokenId = 0;
    SOPC_SecureListener* scListener = &secureListenersArray[scConnection->serverEndpointConfigIdx];

    if (scListener->state == SECURE_LISTENER_STATE_OPENED)
    {
        // Randomize secure channel ids (table 26 part 6)
        uint32_t newTokenId = 0;
        uint32_t idx = 0;
        uint32_t connectionIdx = 0;
        bool occupiedId = false;
        uint8_t attempts = 5; // attempts to find a non conflicting secure Id
        while (resultTokenId == 0 && attempts > 0)
        {
            attempts--;

            if (SOPC_CryptoProvider_GenerateRandomID(scConnection->cryptoProvider, &newTokenId) != SOPC_STATUS_OK)
            {
                continue;
            }

            occupiedId = false;
            if (newTokenId != 0)
            {
                // Check if other channels already use the random id in existing connections
                for (idx = 0; idx < SOPC_MAX_SOCKETS_CONNECTIONS && false == occupiedId; idx++)
                {
                    if (scListener->isUsedConnectionIdxArray[idx] != false)
                    {
                        connectionIdx = scListener->connectionIdxArray[idx];
                        if (secureConnectionsArray[connectionIdx].state != SECURE_CONNECTION_STATE_SC_CLOSED &&
                            newTokenId == secureConnectionsArray[connectionIdx].currentSecurityToken.tokenId)
                        {
                            // If same SC id we will have to generate a new one
                            occupiedId = true;
                        }
                    }
                }
                if (false == occupiedId)
                {
                    // Id is not used by another channel in the endpoint:
                    resultTokenId = newTokenId;
                }
            }
        }
    }
    return resultTokenId;
}

static bool SC_ServerTransition_ScConnecting_To_ScConnected(SOPC_SecureConnection* scConnection,
                                                            uint32_t scConnectionIdx,
                                                            uint32_t requestId,
                                                            uint32_t requestHandle)
{
    assert(scConnection != NULL);
    assert(scConnection->state == SECURE_CONNECTION_STATE_SC_CONNECTING);
    assert(scConnection->isServerConnection != false);
    assert(scConnection->cryptoProvider != NULL);

    bool result = false;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    OpcUa_ResponseHeader respHeader;
    OpcUa_ResponseHeader_Initialize(&respHeader);
    OpcUa_OpenSecureChannelResponse opnResp;
    OpcUa_OpenSecureChannelResponse_Initialize(&opnResp);
    SOPC_Buffer* opnRespBuffer = NULL;
    SOPC_SecureChannel_Config* scConfig = NULL;
    const uint8_t* bytes = NULL;
    uint32_t length = 0;

    scConfig = SOPC_ToolkitServer_GetSecureChannelConfig(scConnection->endpointConnectionConfigIdx);
    assert(scConfig != NULL);

    // Write the OPN response message
    opnRespBuffer = SOPC_Buffer_Create(scConnection->tcpMsgProperties.sendBufferSize);
    if (opnRespBuffer != NULL)
    {
        result = true;
    }
    // Note: do not let size of headers for chunks manager since it is not a fixed size

    // Generate security token parameters
    if (result != false)
    {
        // Define the security token
        result = SC_Server_GenerateFreshSecureChannelAndTokenId(scConnection,
                                                                &scConnection->currentSecurityToken.secureChannelId,
                                                                &scConnection->currentSecurityToken.tokenId);
        scConnection->currentSecurityToken.revisedLifetime = scConfig->requestedLifetime;
        scConnection->currentSecurityToken.createdAt = SOPC_Time_GetCurrentTimeUTC();
        scConnection->currentSecurityToken.lifetimeEndTimeRef = SOPC_TimeReference_AddMilliseconds(
            SOPC_TimeReference_GetCurrent(), scConnection->currentSecurityToken.revisedLifetime);
        scConnection->serverNewSecuTokenActive = true; // There is no precedent security token on establishment
    }

    // Fill response header

    if (result != false)
    {
        // Fill the server nonce and generate key sets if applicable
        SOPC_SecretBuffer* serverNonce = NULL;
        if (scConfig->msgSecurityMode != OpcUa_MessageSecurityMode_None)
        {
            assert(scConnection->clientNonce != NULL);

            status = SOPC_CryptoProvider_GenerateSecureChannelNonce(scConnection->cryptoProvider, &serverNonce);

            if (SOPC_STATUS_OK == status)
            {
                result = SC_DeriveSymmetricKeySets(true, scConnection->cryptoProvider, scConnection->clientNonce,
                                                   serverNonce, &scConnection->currentSecuKeySets);
            }
            else
            {
                result = false;
            }

            if (result != false)
            {
                if (SOPC_STATUS_OK == status)
                {
                    length = SOPC_SecretBuffer_GetLength(serverNonce);
                    if (length <= INT32_MAX)
                    {
                        bytes = SOPC_SecretBuffer_Expose(serverNonce);
                        status = SOPC_ByteString_CopyFromBytes(&opnResp.ServerNonce, bytes, (int32_t) length);
                    }
                    else
                    {
                        status = SOPC_STATUS_NOK;
                    }
                }

                if (status != SOPC_STATUS_OK)
                {
                    result = false;
                }
            }

            SOPC_SecretBuffer_DeleteClear(serverNonce);
            SOPC_SecretBuffer_DeleteClear(scConnection->clientNonce);
            scConnection->clientNonce = NULL;
        }
    }

    if (result != false)
    {
        // Fill the rest

        respHeader.Timestamp = SOPC_Time_GetCurrentTimeUTC();
        respHeader.RequestHandle = requestHandle;

        opnResp.ServerProtocolVersion = scConnection->tcpMsgProperties.protocolVersion;
        opnResp.SecurityToken.ChannelId = scConnection->currentSecurityToken.secureChannelId;
        opnResp.SecurityToken.TokenId = scConnection->currentSecurityToken.tokenId;
        opnResp.SecurityToken.RevisedLifetime = scConnection->currentSecurityToken.revisedLifetime;
        opnResp.SecurityToken.CreatedAt = scConnection->currentSecurityToken.createdAt;

        // Encode the OPN message
        status = SOPC_EncodeMsg_Type_Header_Body(opnRespBuffer, &OpcUa_OpenSecureChannelResponse_EncodeableType,
                                                 &OpcUa_ResponseHeader_EncodeableType, (void*) &respHeader,
                                                 (void*) &opnResp);

        if (SOPC_STATUS_OK != status)
        {
            result = false;
        }
    }

    if (result != false)
    {
        scConnection->state = SECURE_CONNECTION_STATE_SC_CONNECTED;
        SOPC_SecureChannels_EnqueueInternalEvent(INT_SC_SND_OPN, scConnectionIdx, (void*) opnRespBuffer, requestId);
    }
    else if (opnRespBuffer != NULL)
    {
        // Buffer will not be used anymore
        SOPC_Buffer_Delete(opnRespBuffer);
    }

    OpcUa_ResponseHeader_Clear(&respHeader);
    OpcUa_OpenSecureChannelResponse_Clear(&opnResp);

    return result;
}

static bool SC_ServerTransition_ScConnected_To_ScConnectedRenew(SOPC_SecureConnection* scConnection,
                                                                SOPC_Buffer* opnReqMsgBuffer,
                                                                uint32_t* requestHandle,
                                                                uint32_t* requestedLifetime,
                                                                SOPC_StatusCode* errorStatus)
{
    // Important note: errorStatus shall be one of the errors that can be sent with a OPC UA TCP error message (part4
    // Table 38)
    assert(scConnection != NULL);
    assert(opnReqMsgBuffer != NULL);
    assert(requestHandle != NULL);
    assert(requestedLifetime != NULL);
    assert(scConnection->state == SECURE_CONNECTION_STATE_SC_CONNECTED);
    assert(scConnection->isServerConnection != false);
    // Note: Chunks manager shall have checked it is the same secu policy and mode

    bool result = false;
    SOPC_SecureChannel_Config* scConfig = NULL;
    OpcUa_RequestHeader* reqHeader = NULL;
    OpcUa_OpenSecureChannelRequest* opnReq = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    scConfig = SOPC_ToolkitServer_GetSecureChannelConfig(scConnection->endpointConnectionConfigIdx);
    assert(scConfig != NULL);

    status = SOPC_DecodeMsg_HeaderOrBody(opnReqMsgBuffer, &OpcUa_RequestHeader_EncodeableType, (void**) &reqHeader);
    if (SOPC_STATUS_OK == status)
    {
        assert(reqHeader != NULL);
        result = true;
    }

    if (result != false)
    {
        // TODO: check timestamp + timeout ?
        *requestHandle = reqHeader->RequestHandle;
    }

    if (result != false)
    {
        status = SOPC_DecodeMsg_HeaderOrBody(opnReqMsgBuffer, &OpcUa_OpenSecureChannelRequest_EncodeableType,
                                             (void**) &opnReq);
        if (SOPC_STATUS_OK != status)
        {
            result = false;
        }
    }
    if (result != false)
    {
        assert(opnReq != NULL);
        if (scConnection->tcpMsgProperties.protocolVersion != opnReq->ClientProtocolVersion)
        {
            // Different protocol version provided on RENEW
            *errorStatus = OpcUa_BadProtocolVersionUnsupported;
            result = false;
        }
        if (result != false && opnReq->RequestType != OpcUa_SecurityTokenRequestType_Renew)
        {
            // Cannot Issue in CONNECTED state
            *errorStatus = OpcUa_BadSecurityChecksFailed;
            result = false;
        }

        if (result != false && opnReq->SecurityMode != scConfig->msgSecurityMode)
        {
            // Different security mode provided on RENEW
            *errorStatus = OpcUa_BadSecurityModeRejected;
            result = false;
        }

        if (result != false)
        {
            // Note: the revised lifetime is not updated in SC configuration for RENEW
            if (opnReq->RequestedLifetime < SOPC_MINIMUM_SECURE_CONNECTION_LIFETIME)
            {
                *requestedLifetime = SOPC_MINIMUM_SECURE_CONNECTION_LIFETIME;
            }
            else
            {
                *requestedLifetime = opnReq->RequestedLifetime;
            }
        }

        if (result != false)
        {
            // Check security mode is the same as the original one
            if (scConfig->msgSecurityMode == OpcUa_MessageSecurityMode_None)
            {
                if (opnReq->ClientNonce.Length > 0)
                {
                    // Nonce unexpected
                    SOPC_Logger_TraceWarning(
                        "ScStateMgr: Nonce unexpected in OPN req for None security mode epCfgIdx=%" PRIu32
                        " scCfgIdx=%" PRIu32,
                        scConnection->serverEndpointConfigIdx, scConnection->endpointConnectionConfigIdx);
                }
            }
            else
            {
                assert(scConfig->msgSecurityMode == OpcUa_MessageSecurityMode_Sign ||
                       scConfig->msgSecurityMode == OpcUa_MessageSecurityMode_SignAndEncrypt);
                if (opnReq->ClientNonce.Length > 0)
                {
                    scConnection->clientNonce = SOPC_SecretBuffer_NewFromExposedBuffer(
                        opnReq->ClientNonce.Data, (uint32_t) opnReq->ClientNonce.Length);
                    if (NULL == scConnection->clientNonce)
                    {
                        *errorStatus = OpcUa_BadTcpInternalError;
                        result = false;
                    }
                }
                else
                {
                    // Nonce expected
                    result = false;
                    *errorStatus = OpcUa_BadSecurityChecksFailed;
                }
            }
        }
    }

    if (result != false)
    {
        scConnection->state = SECURE_CONNECTION_STATE_SC_CONNECTED_RENEW;
    }

    SOPC_Encodeable_Delete(&OpcUa_RequestHeader_EncodeableType, (void**) &reqHeader);
    SOPC_Encodeable_Delete(&OpcUa_OpenSecureChannelRequest_EncodeableType, (void**) &opnReq);

    return result;
}

static bool SC_ServerTransition_ScConnectedRenew_To_ScConnected(SOPC_SecureConnection* scConnection,
                                                                uint32_t scConnectionIdx,
                                                                uint32_t requestId,
                                                                uint32_t requestHandle,
                                                                uint32_t requestedLifetime)
{
    assert(scConnection != NULL);
    assert(scConnection->state == SECURE_CONNECTION_STATE_SC_CONNECTED_RENEW);
    assert(scConnection->isServerConnection != false);

    bool result = false;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    OpcUa_ResponseHeader respHeader;
    OpcUa_ResponseHeader_Initialize(&respHeader);
    OpcUa_OpenSecureChannelResponse opnResp;
    OpcUa_OpenSecureChannelResponse_Initialize(&opnResp);
    SOPC_Buffer* opnRespBuffer = NULL;
    SOPC_SecureChannel_Config* scConfig = NULL;
    SOPC_SecureConnection_SecurityToken newSecuToken;
    SOPC_SC_SecurityKeySets newSecuKeySets;
    const uint8_t* bytes = NULL;
    uint32_t length = 0;

    memset(&newSecuKeySets, 0, sizeof(SOPC_SC_SecurityKeySets));

    scConfig = SOPC_ToolkitServer_GetSecureChannelConfig(scConnection->endpointConnectionConfigIdx);
    assert(scConfig != NULL);

    // Write the OPN request message
    opnRespBuffer = SOPC_Buffer_Create(scConnection->tcpMsgProperties.sendBufferSize);
    if (opnRespBuffer != NULL)
    {
        result = true;
    }
    // Note: do not let size of headers for chunks manager since it is not a fixed size

    // Generate security token parameters
    if (result != false)
    {
        newSecuToken.tokenId = SC_Server_GenerateFreshTokenId(scConnection);
        if (newSecuToken.tokenId == 0)
        {
            result = false;
        }
        else
        {
            newSecuToken.secureChannelId = scConnection->currentSecurityToken.secureChannelId;
            newSecuToken.revisedLifetime = requestedLifetime;
            newSecuToken.createdAt = SOPC_Time_GetCurrentTimeUTC();
            newSecuToken.lifetimeEndTimeRef =
                SOPC_TimeReference_AddMilliseconds(SOPC_TimeReference_GetCurrent(), newSecuToken.revisedLifetime);
        }
    }

    // Fill response header

    if (result != false)
    {
        // Fill the server nonce and generate key sets if applicable
        if (scConfig->msgSecurityMode != OpcUa_MessageSecurityMode_None)
        {
            SOPC_SecretBuffer* serverNonce = NULL;
            assert(scConnection->clientNonce != NULL);

            status = SOPC_CryptoProvider_GenerateSecureChannelNonce(scConnection->cryptoProvider, &serverNonce);

            if (SOPC_STATUS_OK == status)
            {
                result = SC_DeriveSymmetricKeySets(true, scConnection->cryptoProvider, scConnection->clientNonce,
                                                   serverNonce, &newSecuKeySets);
            }
            else
            {
                result = false;
            }

            if (result != false)
            {
                if (SOPC_STATUS_OK == status)
                {
                    length = SOPC_SecretBuffer_GetLength(serverNonce);
                    if (length <= INT32_MAX)
                    {
                        bytes = SOPC_SecretBuffer_Expose(serverNonce);
                        status = SOPC_ByteString_CopyFromBytes(&opnResp.ServerNonce, bytes, (int32_t) length);
                    }
                    else
                    {
                        status = SOPC_STATUS_NOK;
                    }
                }

                if (status != SOPC_STATUS_OK)
                {
                    result = false;
                }
            }

            SOPC_SecretBuffer_DeleteClear(serverNonce);
            SOPC_SecretBuffer_DeleteClear(scConnection->clientNonce);
            scConnection->clientNonce = NULL;
        }
    }

    if (result != false)
    {
        respHeader.Timestamp = SOPC_Time_GetCurrentTimeUTC();
        respHeader.RequestHandle = requestHandle;

        opnResp.ServerProtocolVersion = scConnection->tcpMsgProperties.protocolVersion;
        opnResp.SecurityToken.ChannelId = newSecuToken.secureChannelId;
        opnResp.SecurityToken.TokenId = newSecuToken.tokenId;
        opnResp.SecurityToken.RevisedLifetime = newSecuToken.revisedLifetime;
        opnResp.SecurityToken.CreatedAt = newSecuToken.createdAt;

        // Encode the OPN message
        status = SOPC_EncodeMsg_Type_Header_Body(opnRespBuffer, &OpcUa_OpenSecureChannelResponse_EncodeableType,
                                                 &OpcUa_ResponseHeader_EncodeableType, (void*) &respHeader,
                                                 (void*) &opnResp);

        if (SOPC_STATUS_OK != status)
        {
            result = false;
        }
    }

    if (result != false)
    {
        scConnection->state = SECURE_CONNECTION_STATE_SC_CONNECTED;
        // copy current security token in precedent and new in current
        scConnection->precedentSecurityToken = scConnection->currentSecurityToken;
        scConnection->precedentSecuKeySets = scConnection->currentSecuKeySets;
        scConnection->currentSecurityToken = newSecuToken;
        scConnection->currentSecuKeySets = newSecuKeySets;
        // Precedent security token will remain active until expiration or recetion of client message with new token
        scConnection->serverNewSecuTokenActive = false;
        SOPC_SecureChannels_EnqueueInternalEvent(INT_SC_SND_OPN, scConnectionIdx, (void*) opnRespBuffer, requestId);
    }
    else
    {
        if (opnRespBuffer != NULL)
        {
            // Buffer will not be used anymore
            SOPC_Buffer_Delete(opnRespBuffer);
        }
        if (newSecuKeySets.receiverKeySet != NULL)
        {
            SOPC_KeySet_Delete(newSecuKeySets.receiverKeySet);
        }
        if (newSecuKeySets.senderKeySet != NULL)
        {
            SOPC_KeySet_Delete(newSecuKeySets.senderKeySet);
        }
    }

    OpcUa_ResponseHeader_Clear(&respHeader);
    OpcUa_OpenSecureChannelResponse_Clear(&opnResp);

    return result;
}

static bool sc_init_key_and_certs(SOPC_SecureConnection* sc)
{
    const SOPC_SerializedAsymmetricKey* serialized_private_key = NULL;
    const SOPC_Buffer* cert_data = NULL;
    const SOPC_Buffer* peer_cert_data = NULL;

    if (sc->isServerConnection)
    {
        SOPC_Endpoint_Config* epConfig = SOPC_ToolkitServer_GetEndpointConfig(sc->serverEndpointConfigIdx);
        assert(epConfig != NULL);
        serialized_private_key = epConfig->serverKey;
        cert_data = epConfig->serverCertificate;
    }
    else
    {
        SOPC_SecureChannel_Config* scConfig =
            SOPC_ToolkitClient_GetSecureChannelConfig(sc->endpointConnectionConfigIdx);
        assert(scConfig != NULL);
        serialized_private_key = scConfig->key_priv_cli;
        cert_data = scConfig->crt_cli;
        peer_cert_data = scConfig->crt_srv;
    }

    if (serialized_private_key == NULL || cert_data == NULL)
    {
        return true;
    }

    SOPC_Certificate** cert = sc->isServerConnection ? &sc->serverCertificate : &sc->clientCertificate;

    if (SOPC_KeyManager_SerializedAsymmetricKey_Deserialize(serialized_private_key, false, &sc->privateKey) !=
            SOPC_STATUS_OK ||
        SOPC_KeyManager_Certificate_CreateFromDER(cert_data->data, cert_data->length, cert) != SOPC_STATUS_OK ||
        (peer_cert_data != NULL &&
         SOPC_KeyManager_Certificate_CreateFromDER(peer_cert_data->data, peer_cert_data->length,
                                                   &sc->serverCertificate) != SOPC_STATUS_OK))
    {
        SOPC_KeyManager_AsymmetricKey_Free(sc->privateKey);
        sc->privateKey = NULL;

        SOPC_KeyManager_Certificate_Free(*cert);
        *cert = NULL;

        if (peer_cert_data != NULL)
        {
            SOPC_KeyManager_Certificate_Free(sc->serverCertificate);
            sc->serverCertificate = NULL;
        }

        return false;
    }

    return true;
}

static bool initServerSC(uint32_t socketIndex, uint32_t serverEndpointConfigIdx, uint32_t* conn_idx)
{
    if (!SC_InitNewConnection(conn_idx))
    {
        return false;
    }

    SOPC_SecureConnection* scConnection = SC_GetConnection(*conn_idx);
    assert(scConnection != NULL);

    // set the socket index associated
    scConnection->socketIndex = socketIndex;
    // record the endpoint description configuration
    scConnection->serverEndpointConfigIdx = serverEndpointConfigIdx;
    // set connection as a server side connection
    scConnection->isServerConnection = true;

    if (!sc_init_key_and_certs(scConnection))
    {
        return false;
    }

    // Activate SC connection timeout (server side)
    scConnection->connectionTimeoutTimerId = SC_StartConnectionEstablishTimer(*conn_idx);

    return true;
}

static void onClientSideOpen(SOPC_SecureConnection* scConnection, uint32_t scIdx, SOPC_Buffer* msg)
{
    assert(!scConnection->isServerConnection);

    // CLIENT SIDE: OPN response for new SC or for renew
    // SC (symmetric keys)
    // Check the OPC UA msg is an OPN resp

    if (scConnection->state != SECURE_CONNECTION_STATE_SC_CONNECTING &&
        scConnection->state != SECURE_CONNECTION_STATE_SC_CONNECTED_RENEW)
    {
        // Error case: close the connection
        // Note: it is not really a security check that failed but in case the SC connection is not
        // established yet,
        //       it is the error to send. It can be changed to more precise errors by checking again the
        //       state (CONNECTED* allows precise error).
        SC_CloseSecureConnection(scConnection, scIdx, false, false, OpcUa_BadSecurityChecksFailed,
                                 "Invalid state to receive an OPN request");
        return;
    }

    if (!SC_ReadAndCheckOpcUaMessageType(&OpcUa_OpenSecureChannelResponse_EncodeableType, (SOPC_Buffer*) msg))
    {
        SC_CloseSecureConnection(scConnection, scIdx, false, false, OpcUa_BadRequestNotAllowed,
                                 "Unexpected OpenSecureChannel request received by client");
        return;
    }

    if (scConnection->state == SECURE_CONNECTION_STATE_SC_CONNECTING)
    {
        // transition: Connecting => Connected
        if (!SC_ClientTransition_ScConnecting_To_ScConnected(scConnection, scIdx, (SOPC_Buffer*) msg))
        {
            SC_CloseSecureConnection(scConnection, scIdx, false, false, OpcUa_BadInvalidArgument,
                                     "Failure during OpenSecureChannel response treatment");
            return;
        }

        // De-activate SC connection timeout (client side)
        SOPC_EventTimer_Cancel(scConnection->connectionTimeoutTimerId);

        SOPC_EventHandler_Post(secureChannelsEventHandler, SC_CONNECTED, scIdx, NULL,
                               scConnection->endpointConnectionConfigIdx);
        return;
    }
    else if (scConnection->state == SECURE_CONNECTION_STATE_SC_CONNECTED_RENEW)
    {
        // transition: Connected_Renew => Connected
        if (!SC_ClientTransition_ScConnectedRenew_To_ScConnected(scConnection, scIdx, (SOPC_Buffer*) msg))
        {
            // Manage renew failure ? => nothing can be done since secu token will be expire
            SC_CloseSecureConnection(scConnection, scIdx, false, false, OpcUa_BadInvalidArgument,
                                     "Failure during OpenSecureChannel RENEW response treatment");
            return;
        }

        return;
    }

    assert(false);
}

static void onServerSideOpen(SOPC_SecureConnection* scConnection, uint32_t scIdx, SOPC_Buffer* msg, uint32_t requestId)
{
    assert(scConnection->isServerConnection);

    // SERVER SIDE: OPN request for new SC or for renew SC (symmetric keys)
    // Check the OPC UA msg is an OPN req

    if (scConnection->state != SECURE_CONNECTION_STATE_SC_INIT &&
        scConnection->state != SECURE_CONNECTION_STATE_SC_CONNECTED)
    {
        // Error case: close the connection
        // Note: it is not really a security check that failed but in case the SC connection is not
        // established yet,
        //       it is the error to send. It can be changed to more precise errors by checking again the
        //       state (CONNECTED* allows precise error).
        SC_CloseSecureConnection(scConnection, scIdx, false, false, OpcUa_BadSecurityChecksFailed,
                                 "Invalid state to receive an OPN request");
        return;
    }

    if (!SC_ReadAndCheckOpcUaMessageType(&OpcUa_OpenSecureChannelRequest_EncodeableType, (SOPC_Buffer*) msg))
    {
        // OPC UA message type is unexpected: close the connection
        // Note: it is not really a security check that failed but it is the generic error to send in
        // the current state
        SC_CloseSecureConnection(scConnection, scIdx, false, false, OpcUa_BadSecurityChecksFailed,
                                 "TCP UA OPN message content is not the one expected (wrong type: "
                                 "request/response or message type)");
        return;
    }

    if (scConnection->state == SECURE_CONNECTION_STATE_SC_INIT)
    {
        SOPC_StatusCode status = SOPC_GoodGenericStatus;
        uint32_t requestHandle;

        if (!SC_ServerTransition_ScInit_To_ScConnecting(scConnection, msg, &requestHandle, &status) ||
            !SC_ServerTransition_ScConnecting_To_ScConnected(scConnection, scIdx, requestId, requestHandle))
        {
            // Ensure we set an error status if the transition to Connected state fails
            status = (status == SOPC_GoodGenericStatus ? OpcUa_BadTcpInternalError : status);

            SC_CloseSecureConnection(scConnection, scIdx, false, false, status,
                                     "Failure during new OpenSecureChannel request treatment");
            return;
        }

        // De-activate SC connection timeout (server side)
        SOPC_EventTimer_Cancel(scConnection->connectionTimeoutTimerId);

        SOPC_EventHandler_Post(secureChannelsEventHandler, EP_CONNECTED, scConnection->serverEndpointConfigIdx,
                               (void*) &scConnection->endpointConnectionConfigIdx, scIdx);

        return;
    }
    else if (scConnection->state == SECURE_CONNECTION_STATE_SC_CONNECTED)
    {
        SOPC_StatusCode status = SOPC_GoodGenericStatus;
        uint32_t requestHandle;
        uint32_t requestedLifetime;

        // transition: Connected => ConnectedRenew
        if (!SC_ServerTransition_ScConnected_To_ScConnectedRenew(scConnection, msg, &requestHandle, &requestedLifetime,
                                                                 &status) ||
            !SC_ServerTransition_ScConnectedRenew_To_ScConnected(scConnection, scIdx, requestId, requestHandle,
                                                                 requestedLifetime))
        {
            // Ensure we set an error status if the transition to Connected state fails
            status = (status == SOPC_GoodGenericStatus ? OpcUa_BadTcpInternalError : status);

            // TODO: send a service fault ?
            SC_CloseSecureConnection(scConnection, scIdx, false, false, status,
                                     "Failure during renew OpenSecureChannel request treatment");
        }

        return;
    }

    assert(false);
}

void SOPC_SecureConnectionStateMgr_OnInternalEvent(SOPC_SecureChannels_InternalEvent event,
                                                   uint32_t eltId,
                                                   void* params,
                                                   uintptr_t auxParam)
{
    switch (event)
    {
    /* SC listener manager -> SC connection manager */
    case INT_EP_SC_CREATE:
    {
        /* id = endpoint description configuration index,
           auxParam = socket index */

        assert(auxParam <= UINT32_MAX);
        SOPC_Logger_TraceDebug("ScStateMgr: INT_EP_SC_CREATE epCfgIdx=%" PRIu32 " socketIdx=%" PRIuPTR, eltId,
                               auxParam);

        uint32_t conn_idx = 0;

        if (initServerSC((uint32_t) auxParam, eltId, &conn_idx))
        {
            // notify socket that connection is accepted
            SOPC_Sockets_EnqueueEvent(SOCKET_ACCEPTED_CONNECTION, (uint32_t) auxParam, NULL, conn_idx);
            // notify secure listener that connection is accepted
            SOPC_SecureChannels_EnqueueInternalEvent(INT_EP_SC_CREATED, eltId, NULL, conn_idx);
        }
        else
        {
            // Error case: request to close the socket newly created
            //             / nothing to send to SC listener state manager (no record of new connection in it for
            //             now)
            SOPC_Sockets_EnqueueEvent(SOCKET_CLOSE, (uint32_t) auxParam, NULL, 0);
        }
        break;
    }
    /* OPC UA chunks message manager -> SC connection manager */
    case INT_SC_RCV_HEL:
    {
        SOPC_Logger_TraceDebug("ScStateMgr: INT_SC_RCV_HEL scIdx=%" PRIu32, eltId);

        SOPC_Buffer* buffer = params;
        SOPC_SecureConnection* scConnection = SC_GetConnection(eltId);
        SOPC_StatusCode errorStatus = SOPC_GoodGenericStatus;

        if (scConnection == NULL)
        {
            // Jump to the end of the if
        }
        else if (scConnection->state != SECURE_CONNECTION_STATE_TCP_INIT || !scConnection->isServerConnection)
        {
            SC_CloseSecureConnection(scConnection, eltId, false, false, OpcUa_BadTcpMessageTypeInvalid,
                                     "Hello message received not expected");
        }
        else if (!SC_ServerTransition_TcpInit_To_TcpNegotiate(scConnection, (SOPC_Buffer*) params, &errorStatus) ||
                 !SC_ServerTransition_TcpNegotiate_To_ScInit(scConnection, eltId, (SOPC_Buffer*) params, &errorStatus))
        {
            SC_CloseSecureConnection(scConnection, eltId, false, false, errorStatus,
                                     "Error on HELLO message treatment");
        }

        SOPC_Buffer_Delete(buffer);

        break;
    }
    case INT_SC_RCV_ACK:
    {
        SOPC_Logger_TraceDebug("ScStateMgr: INT_SC_RCV_ACK scIdx=%" PRIu32, eltId);

        SOPC_Buffer* buffer = params;
        SOPC_SecureConnection* scConnection = SC_GetConnection(eltId);

        if (scConnection == NULL)
        {
            // Jump to the end of the if
        }
        else if (scConnection->state != SECURE_CONNECTION_STATE_TCP_NEGOTIATE || scConnection->isServerConnection)
        {
            SC_CloseSecureConnection(scConnection, eltId, false, false, OpcUa_BadTcpMessageTypeInvalid,
                                     "Unexpected Hello message received");
        }
        else if (!SC_ClientTransition_TcpNegotiate_To_ScInit(scConnection, (SOPC_Buffer*) params) ||
                 !SC_ClientTransition_ScInit_To_ScConnecting(scConnection, eltId))
        {
            SC_CloseSecureConnection(scConnection, eltId, false, false, OpcUa_BadInvalidArgument,
                                     "Invalid Hello message received");
        }

        SOPC_Buffer_Delete(buffer);

        break;
    }
    case INT_SC_RCV_OPN:
    {
        SOPC_Logger_TraceDebug("ScStateMgr: INT_SC_RCV_OPN scIdx=%" PRIu32 " reqId=%" PRIuPTR, eltId, auxParam);

        SOPC_SecureConnection* scConnection = SC_GetConnection(eltId);

        if (scConnection == NULL)
        {
            SOPC_Buffer_Delete(params);
            return;
        }

        if (scConnection->isServerConnection)
        {
            assert(auxParam <= UINT32_MAX);
            onServerSideOpen(scConnection, eltId, params, (uint32_t) auxParam);
        }
        else
        {
            onClientSideOpen(scConnection, eltId, params);
        }

        SOPC_Buffer_Delete(params);

        break;
    }
    case INT_SC_RCV_CLO:
    {
        SOPC_Logger_TraceDebug("ScStateMgr: INT_SC_RCV_CLO scIdx=%" PRIu32 " reqId=%" PRIuPTR, eltId, auxParam);

        SOPC_SecureConnection* scConnection = SC_GetConnection(eltId);

        if (scConnection != NULL)
        {
            if ((scConnection->state == SECURE_CONNECTION_STATE_SC_CONNECTED ||
                 scConnection->state == SECURE_CONNECTION_STATE_SC_CONNECTED_RENEW) &&
                scConnection->isServerConnection != false)
            {
                // SERVER side
                if (SC_ReadAndCheckOpcUaMessageType(&OpcUa_CloseSecureChannelRequest_EncodeableType,
                                                    (SOPC_Buffer*) params))
                {
                    // Just close the socket without any error (Part 6 §7.1.4)
                    SC_CloseSecureConnection(scConnection, eltId,
                                             true,  // consider immediate close since client should have closed now
                                             false, // still request to close socket just in case client did not
                                             OpcUa_BadSecureChannelClosed, "CLO request message received");
                }
                else
                {
                    // Close the socket after reporting an error (Part 6 §7.1.4)
                    // Note: use a security check failure error since it is an incorrect use of protocol
                    SC_CloseSecureConnection(scConnection, eltId, false, false, OpcUa_BadSecurityChecksFailed,
                                             "Invalid CLO request message");
                }
            }
            else
            {
                // Close the socket after reporting an error (Part 6 §7.1.4)
                // Note: use a security check failure error since either SC is not established or unexpected type of
                // message was sent
                SC_CloseSecureConnection(scConnection, eltId, false, false, OpcUa_BadSecurityChecksFailed,
                                         "Failure when encoding a message to send on the secure connection");
            }
        }
        // else: nothing to do (=> socket should already be required to close)
        SOPC_Buffer_Delete((SOPC_Buffer*) params);
        break;
    }
    case INT_SC_RCV_MSG_CHUNKS:
    {
        SOPC_Logger_TraceDebug("ScStateMgr: INT_SC_RCV_MSG_CHUNKS scIdx=%" PRIu32 " reqId=%" PRIuPTR, eltId, auxParam);

        SOPC_SecureConnection* scConnection = SC_GetConnection(eltId);

        if (scConnection != NULL)
        {
            if (scConnection->state == SECURE_CONNECTION_STATE_SC_CONNECTED ||
                scConnection->state == SECURE_CONNECTION_STATE_SC_CONNECTED_RENEW)
            {
                assert(params != NULL);
                // Do not delete buffer since it is provided to services:
                if (false == scConnection->isServerConnection)
                {
                    // Do not provide request Id since it is client side (no response to send)
                    auxParam = 0;
                }
                // No server / client differentiation at this level
                SOPC_EventHandler_Post(secureChannelsEventHandler, SC_SERVICE_RCV_MSG,
                                       eltId,     // secure connection id
                                       params,    // buffer
                                       auxParam); // request Id
            }
            else
            {
                // Error case: close the socket with security check failure since SC is not established
                SC_CloseSecureConnection(
                    scConnection, eltId, false, false, OpcUa_BadSecurityChecksFailed,
                    "SecureConnection: received an OpcUa message on not established secure connection");

                // In other case, buffer has been transmitted to services layer
                SOPC_Buffer_Delete((SOPC_Buffer*) params);
            }
        }
        break;
    }
    case INT_SC_RCV_FAILURE:
    {
        SOPC_Logger_TraceDebug("ScStateMgr: INT_SC_RCV_FAILURE scIdx=%" PRIu32 " statusCode=%" PRIXPTR, eltId,
                               auxParam);

        SOPC_SecureConnection* scConnection = SC_GetConnection(eltId);

        if (scConnection != NULL)
        {
            SC_CloseSecureConnection(scConnection, eltId, false, false, (SOPC_StatusCode) auxParam,
                                     "Failure when receiving a message on the secure connection");
        } // else: nothing to do (=> socket should already be required to close)
        break;
    }
    case INT_SC_SND_FAILURE:
    {
        SOPC_Logger_TraceDebug("ScStateMgr: INT_SC_SND_FAILURE scIdx=%" PRIu32 " reqId/Handle=%" PRIu32
                               " statusCode=%" PRIXPTR,
                               eltId, params == NULL ? 0 : *(uint32_t*) params, auxParam);

        if (params != NULL)
        {
            SOPC_EventHandler_Post(secureChannelsEventHandler, SC_SND_FAILURE,
                                   eltId,     // secure connection id
                                   params,    // request Id
                                   auxParam); // error status
        }
        // else: without request Id, nothing can be treated for the failure
        SOPC_SecureConnection* scConnection = SC_GetConnection(eltId);

        if (scConnection != NULL)
        {
            SC_CloseSecureConnection(scConnection, eltId, false, false, (SOPC_StatusCode) auxParam,
                                     "Failure when encoding a message to send on the secure connection");
        } // else: nothing to do (=> socket should already be required to close)
        break;
    }
    case INT_SC_RCV_ERR:
    {
        SOPC_Logger_TraceDebug("ScStateMgr: INT_SC_RCV_ERR scIdx=%" PRIu32, eltId);

        /* id = secure channel connection index,
           auxParam = params = (SOPC_Buffer*) buffer */
        SOPC_SecureConnection* scConnection = SC_GetConnection(eltId);
        SOPC_StatusCode errorStatus = SOPC_GoodGenericStatus;
        char* errorReason = NULL;

        if (params != NULL)
        {
            errorReason = SC_ClientTransition_ReceivedErrorMsg((SOPC_Buffer*) params, &errorStatus);
        }

        if (errorReason != NULL)
        {
            SOPC_Logger_TraceError("ScStateMgr: ERR message received with status=%" PRIX32
                                   " and reason=%s (scIdx=%" PRIu32 ")",
                                   errorStatus, errorReason, eltId);
            SC_CloseSecureConnection(scConnection, eltId,
                                     true,  // consider immediate close since server should have closed now
                                     false, // still request to close socket just in case server did not
                                     errorStatus, errorReason);
            free(errorReason);
        }
        else
        {
            // Uncertain errorStatus value printed here:
            SOPC_Logger_TraceError("ScStateMgr: ERR message received: error decoding status (%" PRIX32
                                   ") / reason (scIdx=%" PRIu32 ")",
                                   errorStatus, eltId);
            SC_CloseSecureConnection(scConnection, eltId,
                                     true,  // consider immediate close since server should have closed now
                                     false, // still request to close socket just in case server did not
                                     OpcUa_BadSecureChannelClosed, "ERR message received");
        }

        SOPC_Buffer_Delete((SOPC_Buffer*) params);
        break;
    }
    case INT_EP_SC_CLOSE:
    {
        SOPC_Logger_TraceDebug("ScStateMgr: INT_EP_SC_CLOSE scIdx=%" PRIu32 " epCfgIdx=%" PRIuPTR, eltId, auxParam);

        /* id = secure channel connection index,
           auxParam = endpoint description configuration index */
        SOPC_SecureConnection* scConnection = SC_GetConnection(eltId);

        if (scConnection != NULL)
        {
            SC_CloseSecureConnection(scConnection, eltId, false, false, OpcUa_BadSecureChannelClosed,
                                     "Endpoint secure connection listener closed");
        }
        break;
    }
    case INT_SC_CLOSE:
    {
        SOPC_Logger_TraceDebug("ScStateMgr: INT_SC_CLOSE scIdx=%" PRIu32 " reason=%s statusCode=%" PRIXPTR, eltId,
                               params == NULL ? "NULL" : (char*) params, auxParam);

        /* id = secure channel connection index */
        SOPC_SecureConnection* scConnection = SC_GetConnection(eltId);

        if (scConnection != NULL)
        {
            SC_CloseSecureConnection(scConnection, eltId,
                                     true, // it shall be closed immediately now
                                     false, (SOPC_StatusCode) auxParam, (char*) params);
        }
        break;
    }
    default:
        assert(false);
    }
}

void SOPC_SecureConnectionStateMgr_OnSocketEvent(SOPC_Sockets_OutputEvent event,
                                                 uint32_t eltId,
                                                 void* params,
                                                 uintptr_t auxParam)
{
    (void) params;

    switch (event)
    {
    case SOCKET_CONNECTION:
        // CLIENT side only
        /* id = secure channel connection index,
           auxParam = socket index */

        SOPC_Logger_TraceDebug("ScStateMgr: SOCKET_CONNECTION scIdx=%" PRIu32 " socketIdx=%" PRIuPTR, eltId, auxParam);
        assert(auxParam <= UINT32_MAX);

        SOPC_SecureConnection* scConnection = SC_GetConnection(eltId);

        if (scConnection == NULL || scConnection->state != SECURE_CONNECTION_STATE_TCP_INIT)
        {
            // In case of unidentified secure connection problem or wrong state,
            // close the socket
            SOPC_Sockets_EnqueueEvent(SOCKET_CLOSE, (uint32_t) auxParam, NULL, (uintptr_t) eltId);
            return;
        }

        if (!SC_ClientTransition_TcpInit_To_TcpNegotiate(scConnection, eltId, (uint32_t) auxParam))
        {
            // Error case: close the secure connection if invalid state or unexpected error.
            //  (client case only on SOCKET_CONNECTION event)
            SC_CloseSecureConnection(scConnection, eltId, false, false, 0,
                                     "SecureConnection: closed on SOCKET_CONNECTION");
        }

        break;
    case SOCKET_FAILURE:
        SOPC_Logger_TraceDebug("ScStateMgr: SOCKET_FAILURE scIdx=%" PRIu32 " socketIdx=%" PRIuPTR, eltId, auxParam);

        /* id = secure channel connection index,
           auxParam = socket index */
        scConnection = SC_GetConnection(eltId);
        if (scConnection != NULL)
        {
            // Since there was a socket failure, consider the socket close now
            SC_CloseSecureConnection(scConnection, eltId, true, true, 0,
                                     "SecureConnection: disconnected (SOCKET_FAILURE event)");
        }
        break;

    default:
        assert(false);
    }
}

void SOPC_SecureConnectionStateMgr_OnTimerEvent(SOPC_SecureChannels_TimerEvent event,
                                                uint32_t eltId,
                                                void* params,
                                                uintptr_t auxParam)
{
    (void) params;

    switch (event)
    {
    /* Timer events */
    case TIMER_SC_CONNECTION_TIMEOUT:
    {
        SOPC_Logger_TraceDebug("ScStateMgr: TIMER_SC_CONNECTION_TIMEOUT scIdx=%" PRIu32, eltId);

        /* id = secure channel connection index*/
        SOPC_SecureConnection* scConnection = SC_GetConnection(eltId);

        if (scConnection == NULL)
        {
            return;
        }

        scConnection->connectionTimeoutTimerId = 0; // Timer is expired, do not keep reference on it

        // Check SC valid + avoid to close a secure channel established just after timeout
        if (scConnection->state != SECURE_CONNECTION_STATE_SC_CONNECTED &&
            scConnection->state != SECURE_CONNECTION_STATE_SC_CONNECTED_RENEW)
        {
            // SC_SC_CONNECTION_TIMEOUT will be generated in close SC function fpr client side
            SC_CloseSecureConnection(scConnection, eltId, false, false, OpcUa_BadTimeout,
                                     "SecureConnection: disconnected (TIMER_SC_CONNECTION_TIMEOUT event)");
        }
        break;
    }
    case TIMER_SC_CLIENT_OPN_RENEW:
    {
        SOPC_Logger_TraceDebug("ScStateMgr: TIMER_SC_CLIENT_OPN_RENEW scIdx=%" PRIu32, eltId);

        SOPC_SecureConnection* scConnection = SC_GetConnection(eltId);

        if (scConnection == NULL)
        {
            return;
        }

        scConnection->secuTokenRenewTimerId = 0; // Timer is expired, do not keep reference on it

        if (scConnection->state == SECURE_CONNECTION_STATE_SC_CONNECTED && !scConnection->isServerConnection &&
            !SC_ClientTransition_ScConnected_To_ScConnectedRenew(scConnection, eltId))
        {
            // Error case: close the connection
            SC_CloseSecureConnection(scConnection, eltId, false, false, OpcUa_BadTcpInternalError,
                                     "Open secure channel forced renew failed");
        }
        break;
    }
    case TIMER_SC_REQUEST_TIMEOUT:
    {
        assert(auxParam <= UINT32_MAX);

        SOPC_Logger_TraceDebug("ScStateMgr: TIMER_SC_REQUEST_TIMEOUT scIdx=%" PRIu32 " reqId=%" PRIuPTR, eltId,
                               auxParam);

        SOPC_SecureConnection* scConnection = SC_GetConnection(eltId);

        if (scConnection == NULL)
        {
            return;
        }

        assert(scConnection->state != SECURE_CONNECTION_STATE_SC_CLOSED);

        SOPC_SentRequestMsg_Context* msgCtx =
            SOPC_SLinkedList_RemoveFromId(scConnection->tcpSeqProperties.sentRequestIds, (uint32_t) auxParam);

        if (msgCtx == NULL)
        {
            return;
        }

        switch (msgCtx->msgType)
        {
        case SOPC_MSG_TYPE_SC_MSG:
            // Notifies the upper layer
            SOPC_EventHandler_Post(secureChannelsEventHandler, SC_REQUEST_TIMEOUT, eltId, NULL, msgCtx->requestHandle);
            break;
        case SOPC_MSG_TYPE_SC_OPN:
            SOPC_Logger_TraceError("ScStateMgr: OPN request timeout for response on scId=%" PRIu32, eltId);
            // The secure channel will then stay in RENEW if no response received
            break;
        default:
            // Other cases are also covered by the SC establishment timeout: no other action to do
            break;
        }

        free(msgCtx);

        break;
    }
    default:
        assert(false);
    }
}

void SOPC_SecureConnectionStateMgr_Dispatcher(SOPC_SecureChannels_InputEvent event,
                                              uint32_t eltId,
                                              void* params,
                                              uintptr_t auxParam)
{
    bool result = false;
    uint32_t idx = 0;
    SOPC_SecureChannel_Config* scConfig = NULL;
    SOPC_SecureConnection* scConnection = NULL;
    uint32_t* requestIdForSndFailure = NULL;
    SOPC_StatusCode errorStatus = SOPC_GoodGenericStatus; // Good
    switch (event)
    {
    /* Sockets events: */
    /* Sockets manager -> SC connection state manager */

    /* Services events: */
    /* Services manager -> SC connection state manager */
    case SC_CONNECT:
        SOPC_Logger_TraceDebug("ScStateMgr: SC_CONNECT scCfgIdx=%" PRIu32, eltId);

        /* id = secure channel connection configuration index */

        /* Define INIT state of a client */
        scConfig = SOPC_ToolkitClient_GetSecureChannelConfig(eltId);
        if (scConfig != NULL)
        {
            result = SC_InitNewConnection(&idx);
            if (result != false)
            {
                SOPC_Logger_TraceDebug("ScStateMgr: SC_CONNECT scCfgIdx=%" PRIu32 " => new scIdx=%" PRIu32, eltId, idx);

                scConnection = SC_GetConnection(idx);
                assert(scConnection != NULL);
                // record the secure channel connection configuration
                scConnection->endpointConnectionConfigIdx = eltId;

                result = sc_init_key_and_certs(scConnection);
            }
        }
        if (false == result)
        {
            // Error case: notify services that it failed
            // TODO: add a connection failure ? (with config idx + (optional) connection id)
            SOPC_Logger_TraceError("ScStateMgr: SC_CONNECT scCfgIdx=%" PRIu32 " failed to create new connection",
                                   eltId);
            SOPC_EventHandler_Post(secureChannelsEventHandler, SC_CONNECTION_TIMEOUT, eltId, NULL, 0);
        }
        else
        {
            // Require a socket connection for this secure connection

            // URL is not modified but API cannot allow to keep const qualifier: cast to const on treatment
            SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
            SOPC_Sockets_EnqueueEvent(SOCKET_CREATE_CLIENT, idx, (void*) scConfig->url, 0);
            SOPC_GCC_DIAGNOSTIC_RESTORE

            // Activate SC connection timeout (client side)
            scConnection->connectionTimeoutTimerId = SC_StartConnectionEstablishTimer(idx);
        }
        break;
    case SC_DISCONNECT:
        SOPC_Logger_TraceDebug("ScStateMgr: SC_DISCONNECT scIdx=%" PRIu32, eltId);

        /* id = secure channel connection index */
        scConnection = SC_GetConnection(eltId);
        if (scConnection != NULL)
        {
            if (false == scConnection->isServerConnection &&
                (scConnection->state == SECURE_CONNECTION_STATE_SC_CONNECTED ||
                 scConnection->state == SECURE_CONNECTION_STATE_SC_CONNECTED_RENEW))
            {
                SC_ClientTransition_Connected_To_Disconnected(scConnection, eltId);
                result = true;
            }
            if (false == result)
            {
                SC_CloseSecureConnection(
                    scConnection, eltId, false, false, OpcUa_BadTcpInternalError,
                    "Invalid secure connection state or error when sending a close secure channel request");
            }
        }
        // else => nothing to do, services should be notified that SC was already closed before calling disconnect
        // if SC connection was valid
        break;

    case SC_SERVICE_SND_MSG:
        SOPC_Logger_TraceDebug("ScStateMgr: SC_SERVICE_SND_MSG scIdx=%" PRIu32 " reqId/Handle=%" PRIuPTR, eltId,
                               auxParam);

        /* id = secure channel connection index,
           params = (SOPC_Buffer*) received buffer,
           auxParam = request Id context if response (server) / request Handle context if request (client) */
        scConnection = SC_GetConnection(eltId);
        if (NULL == scConnection || (scConnection->state != SECURE_CONNECTION_STATE_SC_CONNECTED &&
                                     scConnection->state != SECURE_CONNECTION_STATE_SC_CONNECTED_RENEW))
        {
            if (NULL == scConnection)
            {
                errorStatus = OpcUa_BadSecureChannelIdInvalid;
            }
            else
            {
                errorStatus = OpcUa_BadSecureChannelClosed;
            }
            // Error case:
            requestIdForSndFailure = malloc(sizeof(uint32_t));
            if (requestIdForSndFailure != NULL)
            {
                *requestIdForSndFailure = (uint32_t) auxParam;
                SOPC_EventHandler_Post(secureChannelsEventHandler, SC_SND_FAILURE,
                                       eltId,                  // secure connection id
                                       requestIdForSndFailure, // request Id
                                       errorStatus);           // error status
            }
            // else: without request Id, nothing can be treated for the failure

            SOPC_Buffer_Delete((SOPC_Buffer*) params);
        }
        else
        {
            SOPC_SecureChannels_EnqueueInternalEvent(INT_SC_SND_MSG_CHUNKS, eltId, params, auxParam);
        }
        break;
    default:
        // Already filtered by secure channels API module
        assert(false);
    }
}
