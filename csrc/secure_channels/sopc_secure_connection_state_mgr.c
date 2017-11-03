/*
 *  Copyright (C) 2016 Systerel and others.
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

#include "sopc_secure_listener_state_mgr.h"

#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>

#include "sopc_crypto_provider.h"
#include "opcua_statuscodes.h"

#include "sopc_toolkit_constants.h"
#include "sopc_toolkit_config.h"
#include "sopc_toolkit_config_internal.h"
#include "sopc_singly_linked_list.h"
#include "sopc_secure_channels_api.h"
#include "sopc_secure_channels_api_internal.h"
#include "sopc_secure_channels_internal_ctx.h"
#include "sopc_sockets_api.h"
#include "sopc_encoder.h"
#include "sopc_encodeable.h"
#include "sopc_services_api.h"


static SOPC_SecureConnection* SC_GetConnection(uint32_t connectionIdx){
    SOPC_SecureConnection* scConnection = NULL;
    if(connectionIdx < SOPC_MAX_SECURE_CONNECTIONS){
        scConnection = &(secureConnectionsArray[connectionIdx]);
    }
    return scConnection;
}

static bool SC_InitNewConnection(uint32_t* newConnectionIdx){
    bool result = false;
    SOPC_SecureConnection* scConnection = NULL;
    uint32_t lastConnectionIdx = 0;

    uint32_t connectionIdx = (lastSecureConnectionArrayIdx + 1) % SOPC_MAX_SECURE_CONNECTIONS;
    do{
        if(connectionIdx == 0){
            // forbidden index => reserved as invalid index
            connectionIdx = 1;
        }
        lastConnectionIdx = connectionIdx;
        if(secureConnectionsArray[connectionIdx].state == SECURE_CONNECTION_STATE_SC_CLOSED){
            result = true;
        }
        connectionIdx = (connectionIdx + 1) % SOPC_MAX_SECURE_CONNECTIONS;
    }while(connectionIdx != lastSecureConnectionArrayIdx && result == false);

    if(result != false){
        scConnection= &(secureConnectionsArray[lastConnectionIdx]);

        // Initialize TCP message properties
        scConnection->tcpMsgProperties.protocolVersion = SOPC_PROTOCOL_VERSION;
        assert(SOPC_MAX_MESSAGE_LENGTH > SOPC_TCP_UA_MIN_BUFFER_SIZE);
        scConnection->tcpMsgProperties.receiveBufferSize = SOPC_MAX_MESSAGE_LENGTH;
        scConnection->tcpMsgProperties.sendBufferSize = SOPC_MAX_MESSAGE_LENGTH;
        scConnection->tcpMsgProperties.maxMessageSize = SOPC_MAX_MESSAGE_LENGTH; // TODO: reduce size since it includes only the body and not the headers/signature ?
        // Note: we do not manage multiple chunks in this version of the toolkit
        scConnection->tcpMsgProperties.maxChunkCount = 1;

        // Initialize TCP sequence properties
        scConnection->tcpSeqProperties.sentRequestIds = SOPC_SLinkedList_Create(0);
        if(scConnection->tcpSeqProperties.sentRequestIds == NULL){
            result = false;
        }

        // Initialize state
        scConnection->state = SECURE_CONNECTION_STATE_TCP_INIT;
    }

    if(result != false){
        lastSecureConnectionArrayIdx = lastConnectionIdx;
        *newConnectionIdx = lastConnectionIdx;
    }

    return result;
}

static bool SC_CloseConnection(uint32_t connectionIdx){
    SOPC_SecureConnection* scConnection = NULL;
    bool result = false;
    if(connectionIdx < SOPC_MAX_SECURE_CONNECTIONS){
        scConnection= &(secureConnectionsArray[connectionIdx]);
        if(scConnection->state != SECURE_CONNECTION_STATE_SC_CLOSED){
            result = true;
            // Clear chunk manager context
            if(scConnection->chunksCtx.chunkInputBuffer != NULL){
                SOPC_Buffer_Delete(scConnection->chunksCtx.chunkInputBuffer);
            }

            // Clear TCP sequence properties
            assert(scConnection->tcpSeqProperties.sentRequestIds != NULL);
            SOPC_SLinkedList_Delete(scConnection->tcpSeqProperties.sentRequestIds);
            scConnection->tcpSeqProperties.sentRequestIds = NULL;

            // Clear TCP asymmetric security properties
            if(scConnection->serverAsymmSecuInfo.clientCertificate != NULL){
                SOPC_KeyManager_Certificate_Free(scConnection->serverAsymmSecuInfo.clientCertificate);
                scConnection->serverAsymmSecuInfo.clientCertificate = NULL;
            }
            // scConnection->serverAsymmSecuInfo.securityPolicyUri => do not free (pointer to config data)
            scConnection->serverAsymmSecuInfo.securityPolicyUri = NULL;

            if(scConnection->cryptoProvider != NULL){
                SOPC_CryptoProvider_Free(scConnection->cryptoProvider);
                scConnection->cryptoProvider = NULL;
            }

            // Clear security sets
            if(scConnection->precedentSecuKeySets.receiverKeySet != NULL){
                SOPC_KeySet_Delete(scConnection->precedentSecuKeySets.receiverKeySet);
                scConnection->precedentSecuKeySets.receiverKeySet = NULL;
            }

            if(scConnection->precedentSecuKeySets.senderKeySet != NULL){
                SOPC_KeySet_Delete(scConnection->precedentSecuKeySets.senderKeySet);
                scConnection->precedentSecuKeySets.senderKeySet = NULL;
            }

            if(scConnection->currentSecuKeySets.receiverKeySet != NULL){
                SOPC_KeySet_Delete(scConnection->currentSecuKeySets.receiverKeySet);
                scConnection->currentSecuKeySets.receiverKeySet = NULL;
            }

            if(scConnection->currentSecuKeySets.senderKeySet != NULL){
                SOPC_KeySet_Delete(scConnection->currentSecuKeySets.senderKeySet);
                scConnection->currentSecuKeySets.senderKeySet = NULL;
            }

            // Clear nonce
            if(scConnection->clientNonce != NULL){
                SOPC_SecretBuffer_DeleteClear(scConnection->clientNonce);
                scConnection->clientNonce = NULL;
            }

            if(scConnection->state != SECURE_CONNECTION_STATE_TCP_INIT){
                // Close the underlying socket
                SOPC_Sockets_EnqueueEvent(SOCKET_CLOSE,
                                          scConnection->socketIndex,
                                          NULL,
                                          0);
            }

            // Clear the rest (state=0 <=> SC_CLOSED)
            memset(scConnection, 0, sizeof(SOPC_SecureConnection));
        }
    }
    return result;
}

static bool SC_Server_SendErrorMsgAndClose(uint32_t        scConnectionIdx,
                                           SOPC_StatusCode errorStatus,
                                           char*           reason)
{
    bool result = false;
    SOPC_StatusCode status = STATUS_NOK;
    SOPC_String tempString;
    SOPC_String_Initialize(&tempString);

    SOPC_Buffer* buffer = SOPC_Buffer_Create(SOPC_TCP_UA_ERR_MIN_MSG_LENGTH + SOPC_TCP_UA_MAX_URL_LENGTH);

    if(buffer != NULL){
        status = SOPC_UInt32_Write(&errorStatus,
                                   buffer);
    }
    if(STATUS_OK == status){
        SOPC_String_AttachFromCstring(&tempString, reason);
    }
    if(STATUS_OK == status){
        status = SOPC_String_Write(&tempString,
                                   buffer);
    }

    if(STATUS_OK == status){
        result = true;

        // Delay SC closure after ERR message treatment will be done by chunks manager
        // IMPORTANT NOTE: will be 2nd event to be treated (see ERR below)
        SOPC_SecureChannels_EnqueueInternalEventAsNext(INT_SC_CLOSE,
                                                       scConnectionIdx,
                                                       reason,
                                                       errorStatus);

        // ERR will be treated before INT_SC_CLOSE since both added as next event
        // IMPORTANT NOTE: will be 1st event to be treated regarding INT_SC_CLOSE
        SOPC_SecureChannels_EnqueueInternalEventAsNext(INT_SC_SND_ERR,
                                                       scConnectionIdx,
                                                       (void*) buffer,
                                                       0);
    }

    SOPC_String_Clear(&tempString);

    return result;
}

static void SC_Client_SendCloseSecureChannelRequestAndClose(SOPC_SecureConnection* scConnection,
                                                            uint32_t               scConnectionIdx,
                                                            SOPC_StatusCode        errorStatus,
                                                            char*                  reason){
    assert(scConnection != NULL);
    SOPC_Buffer* msgBuffer;
    bool result = false;
    SOPC_StatusCode status = STATUS_OK;

    OpcUa_RequestHeader reqHeader;
    OpcUa_RequestHeader_Initialize(&reqHeader);
    OpcUa_CloseSecureChannelRequest cloReq;
    OpcUa_CloseSecureChannelRequest_Initialize(&cloReq);

    msgBuffer = SOPC_Buffer_Create(scConnection->tcpMsgProperties.sendBufferSize);

    if(msgBuffer != NULL){
        // Let size of the headers for the chunk manager
        status = SOPC_Buffer_SetDataLength(msgBuffer, SOPC_UA_SECURE_MESSAGE_HEADER_LENGTH +
                                                      SOPC_UA_SYMMETRIC_SECURITY_HEADER_LENGTH +
                                                      SOPC_UA_SECURE_MESSAGE_SEQUENCE_LENGTH);
        if(STATUS_OK == status){
            status = SOPC_Buffer_SetPosition(msgBuffer, SOPC_UA_SECURE_MESSAGE_HEADER_LENGTH +
                                                        SOPC_UA_SYMMETRIC_SECURITY_HEADER_LENGTH +
                                                        SOPC_UA_SECURE_MESSAGE_SEQUENCE_LENGTH);
        }

        // Fill header
        reqHeader.RequestHandle = scConnectionIdx;
        // TODO: reqHeader.AuditEntryId ?
        reqHeader.TimeoutHint = 0; // TODO: define same timeout as the one set to set SC disconnected without response

        // Fill close request
        // No content to fill: it is an empty body !

        // Encode message in buffer
        if(STATUS_OK == status){
            status = SOPC_EncodeMsg_Type_Header_Body(msgBuffer,
                                                     &OpcUa_CloseSecureChannelRequest_EncodeableType,
                                                     &OpcUa_RequestHeader_EncodeableType,
                                                     (void*) &reqHeader,
                                                     (void*) &cloReq);
        }

        if(STATUS_OK == status){
            result = true;
            // Delay SC closure after CLO message treatment will be done by chunks manager
            // IMPORTANT NOTE: will be 2nd event to be treated (see CLO below)
            SOPC_SecureChannels_EnqueueInternalEventAsNext(INT_SC_CLOSE,
                                                           scConnectionIdx,
                                                           reason,
                                                           errorStatus);

            // CLO will be treated before INT_SC_CLOSE since both added as next event
            // IMPORTANT NOTE: will be 1st event to be treated regarding INT_SC_CLOSE
            SOPC_SecureChannels_EnqueueInternalEventAsNext(INT_SC_SND_CLO,
                                                           scConnectionIdx,
                                                           (void*) msgBuffer,
                                                           -1); // no request Id context in request
        }
    }

    OpcUa_RequestHeader_Clear(&reqHeader);
    OpcUa_CloseSecureChannelRequest_Clear(&cloReq);

    if(result == false){
        // Immediatly close the connection if failed
        if(SC_CloseConnection(scConnectionIdx) != false){
            // Notify services in case of successful closure
            SOPC_Services_EnqueueEvent(SC_TO_SE_SC_DISCONNECTED,
                                       scConnectionIdx,
                                       NULL,
                                       OpcUa_BadSecureChannelClosed);
        }
    }
}

// Note: 3 last params are for server side only and can be NULL for a client
static void SC_CloseSecureConnection(SOPC_SecureConnection* scConnection,
                                     uint32_t               scConnectionIdx,
                                     bool                   isSocketClosed,
                                     SOPC_StatusCode        errorStatus,
                                     char*                  reason){
    SOPC_Buffer* errBuffer = NULL;
    assert(scConnection != NULL);
    bool immediateClose = false;
    const bool isScConnected = (scConnection->state == SECURE_CONNECTION_STATE_SC_CONNECTED ||
                                scConnection->state == SECURE_CONNECTION_STATE_SC_CONNECTED_RENEW);
    if(scConnection->isServerConnection == false){
        // CLIENT case
        if(isScConnected != false){
            // Server shall alway send a close secure channel message request before closing socket

            if(isSocketClosed == false){
                errBuffer = SOPC_Buffer_Create(scConnection->tcpMsgProperties.sendBufferSize);
                if(errBuffer != NULL){
                    SC_Client_SendCloseSecureChannelRequestAndClose(scConnection,
                                                                    scConnectionIdx,
                                                                    errorStatus,
                                                                    reason);
                }else{
                    immediateClose = true;
                }
            }else{
                // Socket failure or closed => do not send any error message
                immediateClose = true;
            }
            if(immediateClose != false){
                // Immediatly close the connection if failed or socket failure (no close message could be send)
                if(SC_CloseConnection(scConnectionIdx) != false){
                    // Notify services in case of successful closure
                    SOPC_Services_EnqueueEvent(SC_TO_SE_SC_DISCONNECTED,
                                               scConnectionIdx,
                                               NULL,
                                               errorStatus);
                }
            }


        }else if(scConnection->state != SECURE_CONNECTION_STATE_SC_CLOSED &&
           SC_CloseConnection(scConnectionIdx) != false){ // => Immediate close
            // Notify services in case of successful closure
            SOPC_Services_EnqueueEvent(SC_TO_SE_SC_CONNECTION_TIMEOUT,
                                       scConnection->endpointConnectionConfigIdx, // SC config idx
                                       NULL,
                                       0);
        }
    }else{
        // SERVER case
        if(scConnection->state == SECURE_CONNECTION_STATE_SC_INIT){
            // No correct hello message received: just close without error message
            if(SC_CloseConnection(scConnectionIdx) != false){ // => Immediate close
                // Do not notify services since SC was not connected yet (and services not aware of SC then)
                // Notify secure listener:
                SOPC_SecureChannels_EnqueueInternalEvent(INT_EP_SC_DISCONNECTED,
                                                         scConnection->serverEndpointConfigIdx,
                                                         NULL,
                                                         scConnectionIdx);
            }
        }else if(scConnection->state != SECURE_CONNECTION_STATE_SC_CLOSED){
            if(isSocketClosed == false){
                bool result = false;
                // Server shall alway send a ERR message before closing socket
                if(OpcUa_BadSecurityChecksFailed == errorStatus){
                    // Reason shall not provide more information in this case
                    reason = "";
                }
                result = SC_Server_SendErrorMsgAndClose(scConnectionIdx,
                                                        errorStatus,
                                                        reason);
                if(result == false){
                    immediateClose = true;
                }
            }else{
                // Socket failure or closed case => just close without error message sending
                immediateClose = true;
            }
            if(immediateClose != false){
                // Immediatly close the connection if failed
                if(SC_CloseConnection(scConnectionIdx) != false){
                    if(isScConnected != false){
                        // Notify services in case of successful closure of a SC connected
                        SOPC_Services_EnqueueEvent(SC_TO_SE_SC_DISCONNECTED,
                                                   scConnectionIdx,
                                                   NULL,
                                                   OpcUa_BadSecureChannelClosed);
                    }
                    // Server side: notify listener that connection closed
                    SOPC_SecureChannels_EnqueueInternalEvent(INT_EP_SC_DISCONNECTED,
                                                             scConnection->serverEndpointConfigIdx,
                                                             NULL,
                                                             scConnectionIdx);
                }
            }
        }
    }
}

static bool SC_ReadAndCheckOpcUaMessageType(SOPC_EncodeableType* msgType,
                                            SOPC_Buffer*         msgBuffer){
    assert(msgBuffer != NULL);
    SOPC_EncodeableType* msgEncType = NULL;
    bool result = false;
    if(STATUS_OK == SOPC_MsgBodyType_Read(msgBuffer, &msgEncType)){
        if(msgEncType == msgType){
            result = true;
        }else if(msgEncType == msgType){
            result = true;
        }
    }
    return result;
}

// TODO: for each use case (client/server) the secret buffer is not adapted => avoid useless copy of nonces
static bool SC_DeriveSymmetricKeySets(bool                isServer,
                                      SOPC_CryptoProvider*     cryptoProvider,
                                      SOPC_SecretBuffer*       clientNonce,
                                      SOPC_SecretBuffer*       serverNonce,
                                      SOPC_SC_SecurityKeySets* keySets){
    assert(cryptoProvider != NULL);
    assert(clientNonce != NULL);
    assert(serverNonce != NULL);
    assert(keySets != NULL);

    bool result = false;
    SOPC_StatusCode status = STATUS_NOK;

    uint32_t encryptKeyLength = 0;
    uint32_t signKeyLength = 0;
    uint32_t initVectorLength = 0;

    status = SOPC_CryptoProvider_DeriveGetLengths(cryptoProvider,
                                             &encryptKeyLength,
                                             &signKeyLength,
                                             &initVectorLength);

    if(STATUS_OK == status){
        result = true;
    }

    if(result != false){
        SOPC_SC_SecurityKeySet* pks = NULL;
        keySets->receiverKeySet = SOPC_KeySet_Create();
        keySets->senderKeySet = SOPC_KeySet_Create();
        pks = keySets->receiverKeySet;
        if(NULL != pks) {
            pks->signKey = SOPC_SecretBuffer_New(signKeyLength);
            pks->encryptKey = SOPC_SecretBuffer_New(encryptKeyLength);
            pks->initVector = SOPC_SecretBuffer_New(initVectorLength);
        }else{
            result = false;
        }
        pks = keySets->senderKeySet;
        if(NULL != pks) {
            pks->signKey = SOPC_SecretBuffer_New(signKeyLength);
            pks->encryptKey = SOPC_SecretBuffer_New(encryptKeyLength);
            pks->initVector = SOPC_SecretBuffer_New(initVectorLength);
        }else{
            result = false;
        }
    }

    if(result != false){
        // Generate the symmetric keys
        if(isServer == false){
            status = SOPC_CryptoProvider_DeriveKeySetsClient(cryptoProvider,
                                                        clientNonce,
                                                        SOPC_SecretBuffer_Expose(serverNonce),
                                                        SOPC_SecretBuffer_GetLength(serverNonce),
                                                        keySets->senderKeySet,
                                                        keySets->receiverKeySet);
        }else{
            status = SOPC_CryptoProvider_DeriveKeySetsServer(cryptoProvider,
                                                        SOPC_SecretBuffer_Expose(clientNonce),
                                                        SOPC_SecretBuffer_GetLength(clientNonce),
                                                        serverNonce,
                                                        keySets->receiverKeySet,
                                                        keySets->senderKeySet);
        }
        if(STATUS_OK != status){
            result = false;
        }
    }

    return result;
}

static bool SC_ClientTransition_TcpInit_To_TcpNegotiate(SOPC_SecureConnection* scConnection,
                                                        uint32_t               scConnectionIdx,
                                                        uint32_t               socketIdx){
    assert(scConnection != NULL);
    SOPC_Buffer* msgBuffer;
    SOPC_SecureChannel_Config* scConfig = SOPC_Toolkit_GetSecureChannelConfig(scConnection->endpointConnectionConfigIdx);
    bool result = false;
    SOPC_StatusCode status = STATUS_OK;
    assert(scConnection != NULL);
    assert(scConnection->state == SECURE_CONNECTION_STATE_TCP_INIT);

    // Create OPC UA TCP Hello message
    // Max size of buffer is message minimum size + URL bytes length
    msgBuffer = SOPC_Buffer_Create(SOPC_TCP_UA_HEL_MIN_MSG_LENGTH + SOPC_TCP_UA_MAX_URL_LENGTH);

    if(msgBuffer != NULL){
        // Let size of the header for the chunk manager
        status = SOPC_Buffer_SetDataLength(msgBuffer, SOPC_TCP_UA_HEADER_LENGTH);
        if(STATUS_OK == status){
            SOPC_Buffer_SetPosition(msgBuffer, SOPC_TCP_UA_HEADER_LENGTH);
        }
        // Encode Hello message body
        if(STATUS_OK == status){
            status = SOPC_UInt32_Write(&scConnection->tcpMsgProperties.protocolVersion,
                                       msgBuffer);
        }
        if(STATUS_OK == status){
            status = SOPC_UInt32_Write(&scConnection->tcpMsgProperties.receiveBufferSize,
                                       msgBuffer);
        }
        if(STATUS_OK == status){
            status = SOPC_UInt32_Write(&scConnection->tcpMsgProperties.sendBufferSize,
                                       msgBuffer);
        }
        if(STATUS_OK == status){
            status = SOPC_UInt32_Write(&scConnection->tcpMsgProperties.maxMessageSize,
                                       msgBuffer);
        }
        if(STATUS_OK == status){
            status = SOPC_UInt32_Write(&scConnection->tcpMsgProperties.maxChunkCount,
                                       msgBuffer);
        }
        if(STATUS_OK == status){
            SOPC_String tmpString;
            SOPC_String_Initialize(&tmpString);
            status = SOPC_String_AttachFromCstring(&tmpString, (char*) scConfig->url);
            if(STATUS_OK == status){
                status = SOPC_String_Write(&tmpString,
                                           msgBuffer);
            }
            SOPC_String_Clear(&tmpString);
        }

        if(STATUS_OK == status){
            result = true;
        }
    }

    if(result != false){
        scConnection->socketIndex = socketIdx;
        scConnection->state = SECURE_CONNECTION_STATE_TCP_NEGOTIATE;
        SOPC_SecureChannels_EnqueueInternalEvent(INT_SC_SND_HEL,
                                                 scConnectionIdx,
                                                 (void*) msgBuffer,
                                                 0);
    }

    return result;
}


static void SC_ClientTransition_Connected_To_Disconnected(SOPC_SecureConnection* scConnection,
                                                          uint32_t               scConnectionIdx){
    assert(scConnection != NULL);

    assert(scConnection->state == SECURE_CONNECTION_STATE_SC_CONNECTED ||
           scConnection->state == SECURE_CONNECTION_STATE_SC_CONNECTED_RENEW);

    SC_Client_SendCloseSecureChannelRequestAndClose(scConnection,
                                                    scConnectionIdx,
                                                    OpcUa_BadSecureChannelClosed,
                                                    "Secure channel requested to be closed by client");
}

static bool SC_ClientTransition_TcpNegotiate_To_ScInit(SOPC_SecureConnection* scConnection,
                                                       SOPC_Buffer*           ackMsgBuffer){
    assert(scConnection != NULL);
    assert(ackMsgBuffer != NULL);
    bool result = false;
    SOPC_StatusCode status = STATUS_OK;
    uint32_t tempValue = 0;

    assert(scConnection->state == SECURE_CONNECTION_STATE_TCP_NEGOTIATE);
    assert(scConnection->isServerConnection == false);

    result = true;

    // Read the Acknowledge message content

    // Read protocol version of server
    status = SOPC_UInt32_Read(&tempValue, ackMsgBuffer);
    if(STATUS_OK == status){
        // Check protocol version compatible
        if(scConnection->tcpMsgProperties.protocolVersion > tempValue){
            // Use last version supported by the server
            scConnection->tcpMsgProperties.protocolVersion = tempValue;
        }// else => server will return the last version it supports
    }

    // ReceiveBufferSize
    if(STATUS_OK == status){
        // Read receive buffer size of SERVER
        status = SOPC_UInt32_Read(&tempValue, ackMsgBuffer);
        if(STATUS_OK == status){
            // Adapt send buffer size if needed
            if(scConnection->tcpMsgProperties.sendBufferSize > tempValue){
                if(tempValue >= SOPC_TCP_UA_MIN_BUFFER_SIZE){ // see mantis #3447 for >= instead of >
                    scConnection->tcpMsgProperties.sendBufferSize = tempValue;
                }else{
                    status = STATUS_INVALID_RCV_PARAMETER;
                }
            }else if(scConnection->tcpMsgProperties.sendBufferSize < tempValue){
                // shall not be larger than what requested in Hello message
                status = STATUS_INVALID_RCV_PARAMETER;
            }
        }
    }

    // SendBufferSize
    if(STATUS_OK == status){
        // Read sending buffer size of SERVER
        status = SOPC_UInt32_Read(&tempValue, ackMsgBuffer);
        if(STATUS_OK == status){
            // Check size and adapt receive buffer size if needed
            if(scConnection->tcpMsgProperties.receiveBufferSize > tempValue){
                if(tempValue >= SOPC_TCP_UA_MIN_BUFFER_SIZE){ // see mantis #3447 for >= instead of >
                    scConnection->tcpMsgProperties.receiveBufferSize = tempValue;
                }else{
                    status = STATUS_INVALID_RCV_PARAMETER;
                }
            }else if(scConnection->tcpMsgProperties.receiveBufferSize < tempValue){
                // shall not be larger than what requested in Hello message
                status = STATUS_INVALID_RCV_PARAMETER;
            }
        }
    }


    //MaxMessageSize of SERVER
    if(STATUS_OK == status){
        status = SOPC_UInt32_Read(&tempValue, ackMsgBuffer);
        if(STATUS_OK == status){
            if(scConnection->tcpMsgProperties.maxMessageSize > tempValue ||
               scConnection->tcpMsgProperties.maxMessageSize == 0){
                scConnection->tcpMsgProperties.maxMessageSize = tempValue;
            }
            // if "<" => OK since it is the maximum size requested by server will never be reached
        }
    }

    //MaxChunkCount of SERVER
    if(STATUS_OK == status){
        status = SOPC_UInt32_Read(&tempValue, ackMsgBuffer);
        if(STATUS_OK == status){
            if(scConnection->tcpMsgProperties.maxChunkCount > tempValue ||
               scConnection->tcpMsgProperties.maxChunkCount == 0){
                scConnection->tcpMsgProperties.maxChunkCount = tempValue;
            }
        }
    }

    if(STATUS_OK != status){
        result = false;
    }

    if(result != false){
        scConnection->state = SECURE_CONNECTION_STATE_SC_INIT;
    }

    return result;
}

static bool SC_ClientTransitionHelper_SendOPN(SOPC_SecureConnection* scConnection,
                                              uint32_t               scConnectionIdx,
                                              bool                   isRenewal){
    bool result = false;
    SOPC_StatusCode status = STATUS_OK;

    SOPC_SecureChannel_Config* config = NULL;
    SOPC_Buffer* opnMsgBuffer;
    OpcUa_RequestHeader reqHeader;
    OpcUa_RequestHeader_Initialize(&reqHeader);
    OpcUa_OpenSecureChannelRequest opnReq;
    OpcUa_OpenSecureChannelRequest_Initialize(&opnReq);

    config = SOPC_Toolkit_GetSecureChannelConfig(scConnection->endpointConnectionConfigIdx);
    assert(config != NULL);

    result = true;

    // Create crypto provider if necessary
    if(isRenewal == false){
        assert(scConnection->cryptoProvider == NULL);
        scConnection->cryptoProvider = SOPC_CryptoProvider_Create(config->reqSecuPolicyUri);
        if(scConnection->cryptoProvider == NULL){
            result = false;
        }
    }

    if(result != false){
        // Write the OPN request message
        opnMsgBuffer = SOPC_Buffer_Create(scConnection->tcpMsgProperties.sendBufferSize);
        if(opnMsgBuffer == NULL){
            result = false;
        }
    }
    // Note: do not let size of headers for chunks manager since it is not a fixed size

    if(result != false){
        // Fill request header
        reqHeader.RequestHandle = scConnectionIdx;
        // TODO: reqHeader.AuditEntryId ?
        reqHeader.TimeoutHint = 0; // TODO: define same timeout as the one set to set SC disconnected without response


        // Fill the OPN body
        opnReq.ClientProtocolVersion = scConnection->tcpMsgProperties.protocolVersion;
        if(isRenewal == false){
            opnReq.RequestType = OpcUa_SecurityTokenRequestType_Issue;
        }else{
            opnReq.RequestType = OpcUa_SecurityTokenRequestType_Renew;
        }

        opnReq.SecurityMode = config->msgSecurityMode;

        if(config->msgSecurityMode != OpcUa_MessageSecurityMode_None){
            status = SOPC_CryptoProvider_GenerateSecureChannelNonce(scConnection->cryptoProvider,
                    &scConnection->clientNonce);

            if(status == STATUS_OK){
                uint8_t* bytes = NULL;
                bytes = SOPC_SecretBuffer_Expose(scConnection->clientNonce);
                status = SOPC_ByteString_CopyFromBytes(&opnReq.ClientNonce,
                        bytes,
                        SOPC_SecretBuffer_GetLength(scConnection->clientNonce));
            }

            if(status != STATUS_OK){
                result = false;
            }
        }

        opnReq.RequestedLifetime = config->requestedLifetime;


        // Encode the OPN message
        status = SOPC_EncodeMsg_Type_Header_Body(opnMsgBuffer,
                &OpcUa_OpenSecureChannelRequest_EncodeableType,
                &OpcUa_RequestHeader_EncodeableType,
                (void*) &reqHeader,
                (void*) &opnReq);

        if(STATUS_OK != status){
            result = false;
        }

    }

    if(result != false){
        SOPC_SecureChannels_EnqueueInternalEvent(INT_SC_SND_OPN,
                                                 scConnectionIdx,
                                                 (void*) opnMsgBuffer,
                                                 0);
    }

    OpcUa_RequestHeader_Clear(&reqHeader);
    OpcUa_OpenSecureChannelRequest_Clear(&opnReq);

    return result;
}

static bool SC_ClientTransition_ScInit_To_ScConnecting(SOPC_SecureConnection* scConnection,
                                                       uint32_t               scConnectionIdx){
    bool result = false;

    assert(scConnection != NULL);
    assert(scConnection->state == SECURE_CONNECTION_STATE_SC_INIT);
    result = SC_ClientTransitionHelper_SendOPN(scConnection,
                                               scConnectionIdx,
                                               false);

    if(result != false){
        scConnection->state = SECURE_CONNECTION_STATE_SC_CONNECTING;
    }

    return result;
}

static bool SC_ClientTransition_ScConnected_To_ScConnectedRenew(SOPC_SecureConnection* scConnection,
                                                                uint32_t               scConnectionIdx){
    bool result = false;

    assert(scConnection != NULL);
    assert(scConnection->state == SECURE_CONNECTION_STATE_SC_CONNECTED);
    result = SC_ClientTransitionHelper_SendOPN(scConnection,
                                               scConnectionIdx,
                                               true);

    if(result != false){
        scConnection->state = SECURE_CONNECTION_STATE_SC_CONNECTED_RENEW;
    }

    return result;
}

static bool SC_ClientTransition_ScConnecting_To_ScConnected(SOPC_SecureConnection* scConnection,
                                                            uint32_t               scConnectionIdx,
                                                            SOPC_Buffer*           opnRespBuffer){
    assert(scConnection != NULL);
    assert(scConnection->state == SECURE_CONNECTION_STATE_SC_CONNECTING);
    assert(scConnection->isServerConnection == false);
    assert(opnRespBuffer != NULL);
    SOPC_SecureChannel_Config* scConfig = SOPC_Toolkit_GetSecureChannelConfig(scConnectionIdx);
    assert(scConfig != NULL);

    bool result = false;
    SOPC_StatusCode status = STATUS_NOK;
    OpcUa_ResponseHeader* respHeader = NULL;
    OpcUa_OpenSecureChannelResponse* opnResp = NULL;

    if(STATUS_OK == SOPC_DecodeMsg_HeaderOrBody(opnRespBuffer,
                                                &OpcUa_ResponseHeader_EncodeableType,
                                                (void**) &respHeader)){
        assert(respHeader != NULL);
        result = true;
    }

    if(result != false){
        // TODO: check timestamp ?
        if(respHeader->ServiceResult != STATUS_OK){
            result = false;
        }
        if(respHeader->RequestHandle != scConnectionIdx){
            result = false;
        }
    }

    if(result != false){
        // Decode message
        status = SOPC_DecodeMsg_HeaderOrBody(opnRespBuffer,
                                             &OpcUa_OpenSecureChannelResponse_EncodeableType,
                                             (void**) &opnResp);
       if(STATUS_OK != status){
           result = false;
       }
    }

    if(result != false){
        // Check protocol version
        if(scConnection->tcpMsgProperties.protocolVersion != opnResp->ServerProtocolVersion){
            // Note: since property was already adapted to server on ACK, it shall be the same
            result = false;
        }
        // Check chanel Id and security token provided by the server
        if(opnResp->SecurityToken.ChannelId == 0 ||
           scConnection->clientSecureChannelId != opnResp->SecurityToken.ChannelId || // same Id at TCP level
           opnResp->SecurityToken.TokenId == 0){
            result = false;
        }
        // Clear temporary context
        scConnection->clientSecureChannelId = 0;
    }

    if(result != false){
        // Retrieve the server nonce and generate key sets if applicable
        assert(opnResp != NULL);

        if(OpcUa_MessageSecurityMode_None == scConfig->msgSecurityMode){

            if(opnResp->ServerNonce.Length > 0){
                // Unexpected token
                // TODO: log
            }
        }else{
            assert(scConnection->clientNonce != NULL);

            if(opnResp->ServerNonce.Length <= 0){
                result = false;
            }

            if(result != false){
                SOPC_SecretBuffer* secretServerNonce = SOPC_SecretBuffer_NewFromExposedBuffer(opnResp->ServerNonce.Data, opnResp->ServerNonce.Length);
                if(secretServerNonce != NULL){
                    result = SC_DeriveSymmetricKeySets(false,
                                                       scConnection->cryptoProvider,
                                                       scConnection->clientNonce,
                                                       secretServerNonce,
                                                       &scConnection->currentSecuKeySets);
                    SOPC_SecretBuffer_DeleteClear(secretServerNonce);
                }else{
                    result = false;
                }
            }

            SOPC_SecretBuffer_DeleteClear(scConnection->clientNonce);
            scConnection->clientNonce = NULL;
        }

    }

    if(result != false){
        // Define the current security token properties
        scConnection->currentSecurityToken.secureChannelId = opnResp->SecurityToken.ChannelId;
        scConnection->currentSecurityToken.tokenId = opnResp->SecurityToken.TokenId;
        scConnection->currentSecurityToken.createdAt = opnResp->SecurityToken.CreatedAt;
        scConnection->currentSecurityToken.revisedLifetime = opnResp->SecurityToken.RevisedLifetime;
        scConnection->serverNewSecuTokenActive = true; // There is no precedent security token on establishement
    }

    if(result != false){
        scConnection->state = SECURE_CONNECTION_STATE_SC_CONNECTED;
    }

    SOPC_Encodeable_Delete(&OpcUa_ResponseHeader_EncodeableType,
                           (void**) &respHeader);
    SOPC_Encodeable_Delete(&OpcUa_OpenSecureChannelResponse_EncodeableType,
                           (void**) &opnResp);

    return result;
}

static bool SC_ClientTransition_ScConnectedRenew_To_ScConnected(SOPC_SecureConnection* scConnection,
                                                                uint32_t               scConnectionIdx,
                                                                SOPC_Buffer*           opnRespBuffer){
    (void) scConnection;
    (void) scConnectionIdx;
    (void) opnRespBuffer;
    // TODO: to be managed when timers will be added to renew the security token (current => prec, current = with new OPN resp)
    return false;
}

static bool SC_ServerTransition_TcpInit_To_TcpNegotiate(SOPC_SecureConnection* scConnection,
                                                        SOPC_Buffer*           helloMsgBuffer,
                                                        SOPC_StatusCode*       errorStatus){
    // Note: errorStatus must be an error allowed in OPC UA TCP error message (part 6 Table 38)
    assert(scConnection != NULL);
    bool result = false;
    SOPC_StatusCode status = STATUS_OK;
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
    status = SOPC_UInt32_Read(&tempValue, helloMsgBuffer);
    if(STATUS_OK == status){
        // Check protocol version compatible
        if(scConnection->tcpMsgProperties.protocolVersion > tempValue){
            // Use last version supported by the client
            scConnection->tcpMsgProperties.protocolVersion = tempValue;
        }// else => server will return the last version it supports
    }

    // ReceiveBufferSize
    if(STATUS_OK == status){
        // Read receive buffer size of CLIENT
        status = SOPC_UInt32_Read(&tempValue, helloMsgBuffer);
        if(STATUS_OK == status){
            // Adapt send buffer size if needed
            if(scConnection->tcpMsgProperties.sendBufferSize > tempValue){
                if(tempValue >= SOPC_TCP_UA_MIN_BUFFER_SIZE){ // see mantis #3447 for >= instead of >
                    scConnection->tcpMsgProperties.sendBufferSize = tempValue;
                }else{
                    status = STATUS_INVALID_RCV_PARAMETER;
                }
            }
        }
    }

    // SendBufferSize
    if(STATUS_OK == status){
        // Read sending buffer size of CLIENT
        status = SOPC_UInt32_Read(&tempValue, helloMsgBuffer);
        if(STATUS_OK == status){
            // Check size and adapt receive buffer size if needed
            if(scConnection->tcpMsgProperties.receiveBufferSize > tempValue){
                if(tempValue >= SOPC_TCP_UA_MIN_BUFFER_SIZE){ // see mantis #3447 for >= instead of >
                    scConnection->tcpMsgProperties.receiveBufferSize = tempValue;
                }else{
                    status = STATUS_INVALID_RCV_PARAMETER;
                }
            }
            // In other case do not change size since it should be configured with max size by default
        }
    }


    //MaxMessageSize of CLIENT
    if(STATUS_OK == status){
        status = SOPC_UInt32_Read(&tempValue, helloMsgBuffer);
        if(STATUS_OK == status){
            if(scConnection->tcpMsgProperties.maxMessageSize > tempValue ||
               scConnection->tcpMsgProperties.maxMessageSize == 0){
                scConnection->tcpMsgProperties.maxMessageSize = tempValue;
            }
        }
    }

    //MaxChunkCount of CLIENT
    if(STATUS_OK == status){
        status = SOPC_UInt32_Read(&tempValue, helloMsgBuffer);
        if(STATUS_OK == status){
            if(scConnection->tcpMsgProperties.maxChunkCount > tempValue ||
               scConnection->tcpMsgProperties.maxChunkCount == 0){
                scConnection->tcpMsgProperties.maxChunkCount = tempValue;
            }
        }
    }

    // EndpointURL
    if(STATUS_OK == status){
        status = SOPC_String_Read(&url, helloMsgBuffer);
        // Note: this parameter is normally used to could forward to an endpoint sharing the same port
        //       but not in the same process.
        //       This is not supported by INGOPCS secure channels layer, as consequence expected URL is only the one configured.
        if(STATUS_OK == status){
            if(url.Length > SOPC_TCP_UA_MAX_URL_LENGTH)
            {
                status = OpcUa_BadTcpEndpointUrlInvalid;
            }else{
                int32_t compareValue;
                status = SOPC_String_AttachFromCstring(&epUrl, epConfig->endpointURL);
                if(STATUS_OK == status){
                    status = SOPC_String_Compare(&epUrl, &url,true, &compareValue);
                }
                if(STATUS_OK == status){
                    if(compareValue != 0){
                        status = OpcUa_BadTcpEndpointUrlInvalid;
                    }
                }
            }
        }

        if(STATUS_OK != status && OpcUa_BadTcpEndpointUrlInvalid != status){
            status = OpcUa_BadTcpInternalError;
        }
    }else{
        // Note: any precedent status error shall be reported as an internal error
        status = OpcUa_BadTcpInternalError;
    }

    if(STATUS_OK != status){
        result = false;
        *errorStatus = status;
    }

    if(result != false){
        scConnection->state = SECURE_CONNECTION_STATE_TCP_NEGOTIATE;
    }

    SOPC_String_Clear(&epUrl);
    SOPC_String_Clear(&url);

    return result;
}

static bool SC_ServerTransition_TcpNegotiate_To_ScInit(SOPC_SecureConnection* scConnection,
                                                       uint32_t               scConnectionIdx,
                                                       SOPC_Buffer*           helloMsgBuffer,
                                                       SOPC_StatusCode*       errorStatus){
    bool result = false;
    SOPC_StatusCode status = STATUS_OK;
    SOPC_Buffer* ackMsgBuffer = NULL;

    assert(scConnection != NULL);
    assert(helloMsgBuffer != NULL);
    assert(scConnection->state == SECURE_CONNECTION_STATE_TCP_NEGOTIATE);
    assert(scConnection->isServerConnection != false);

    // Write the Acknowledge message content

    // Reuse the received message buffer: set length and position after TCP UA header
    ackMsgBuffer = helloMsgBuffer;
    status = SOPC_Buffer_SetPosition(ackMsgBuffer, SOPC_TCP_UA_HEADER_LENGTH);
    if(STATUS_OK == status){
        status = SOPC_Buffer_SetDataLength(ackMsgBuffer, SOPC_TCP_UA_HEADER_LENGTH);
    }
    if(STATUS_OK == status){
        status = SOPC_UInt32_Write(&scConnection->tcpMsgProperties.protocolVersion,
                ackMsgBuffer);
    }
    if(STATUS_OK == status){
        status = SOPC_UInt32_Write(&scConnection->tcpMsgProperties.receiveBufferSize,
                ackMsgBuffer);
    }
    if(STATUS_OK == status){
        status = SOPC_UInt32_Write(&scConnection->tcpMsgProperties.sendBufferSize,
                ackMsgBuffer);
    }
    if(STATUS_OK == status){
        status = SOPC_UInt32_Write(&scConnection->tcpMsgProperties.maxMessageSize,
                ackMsgBuffer);
    }
    if(STATUS_OK == status){
        status = SOPC_UInt32_Write(&scConnection->tcpMsgProperties.maxChunkCount,
                ackMsgBuffer);
    }

    if(STATUS_OK == status){
        result = true;
    }else{
        *errorStatus = OpcUa_BadEncodingError;
    }

    if(result != false){
        scConnection->state = SECURE_CONNECTION_STATE_SC_INIT;
        SOPC_SecureChannels_EnqueueInternalEvent(INT_SC_SND_ACK,
                                                 scConnectionIdx,
                                                 ackMsgBuffer,
                                                 0);
    }

    return result;
}

static bool SC_ServerTransition_ScInit_To_ScConnecting(SOPC_SecureConnection* scConnection,
                                                       SOPC_Buffer*           opnReqMsgBuffer,
                                                       uint32_t*              requestHandle,
                                                       SOPC_StatusCode*       errorStatus){
    // Important note: errorStatus shall be one of the errors that can be sent with a OPC UA TCP error message (part4 Table 38)
    assert(scConnection != NULL);
    assert(opnReqMsgBuffer != NULL);
    assert(requestHandle != NULL);
    assert(scConnection->state == SECURE_CONNECTION_STATE_SC_INIT);
    assert(scConnection->isServerConnection != false);
    assert(scConnection->serverAsymmSecuInfo.securityPolicyUri != NULL); // set by chunks manager
    assert(scConnection->serverAsymmSecuInfo.validSecurityModes != 0); // set by chunks manager

    bool result = false;
    uint32_t idx;
    bool validSecurityRequested = false;
    SOPC_Endpoint_Config* epConfig = NULL;
    OpcUa_RequestHeader* reqHeader = NULL;
    OpcUa_OpenSecureChannelRequest* opnReq = NULL;

    epConfig = SOPC_ToolkitServer_GetEndpointConfig(scConnection->serverEndpointConfigIdx);
    assert(epConfig != NULL);


    if(STATUS_OK == SOPC_DecodeMsg_HeaderOrBody(opnReqMsgBuffer,
                                                &OpcUa_RequestHeader_EncodeableType,
                                                (void**) &reqHeader)){
        assert(reqHeader != NULL);
        result = true;
    }

    if(result != false){
        // TODO: check timestamp + timeout ?
        *requestHandle = reqHeader->RequestHandle;
    }

    if(result != false &&
       STATUS_OK == SOPC_DecodeMsg_HeaderOrBody(opnReqMsgBuffer,
                                                &OpcUa_OpenSecureChannelRequest_EncodeableType,
                                                (void**) &opnReq)){
        assert(opnReq != NULL);
        if(scConnection->tcpMsgProperties.protocolVersion != opnReq->ClientProtocolVersion){
            // Note: since property was already adapted to server on HEL, it shall be the same
            //*errorStatus = OpcUa_BadProtocolVersionUnsupported; => not a TCP error message authorized error
            *errorStatus = OpcUa_BadTcpInternalError;
            result = false;
        }
        if(result != false &&
           opnReq->RequestType != OpcUa_SecurityTokenRequestType_Issue){
            // Cannot renew in SC_Init state
            *errorStatus = OpcUa_BadTcpSecureChannelUnknown;
            result = false;
        }

        // Check it is a valid security policy in endpoint security policy
        switch(opnReq->SecurityMode){
        case OpcUa_MessageSecurityMode_None:
            validSecurityRequested = (scConnection->serverAsymmSecuInfo.validSecurityModes & SOPC_SECURITY_MODE_NONE_MASK) != 0;
            break;
        case OpcUa_MessageSecurityMode_Sign:
            validSecurityRequested = (scConnection->serverAsymmSecuInfo.validSecurityModes & SOPC_SECURITY_MODE_SIGN_MASK) != 0;
            break;
        case OpcUa_MessageSecurityMode_SignAndEncrypt:
            validSecurityRequested = (scConnection->serverAsymmSecuInfo.validSecurityModes & SOPC_SECURITY_MODE_SIGNANDENCRYPT_MASK) != 0;
            break;
        case OpcUa_MessageSecurityMode_Invalid:
        default:
            validSecurityRequested = false;
        }

        // Check the valid security mode requested is the one guessed based on asymmetric security header content
        if(validSecurityRequested == false){
            result = false;
            //*errorStatus = OpcUa_BadSecurityModeRejected; => not a TCP error message authorized error
            *errorStatus = OpcUa_BadSecurityChecksFailed;
        }else if(scConnection->serverAsymmSecuInfo.isSecureModeActive == false &&
                 opnReq->SecurityMode != OpcUa_MessageSecurityMode_None){
            // Certificates were absent in asym. header and it is not compatible with the security mode requested
            *errorStatus = OpcUa_BadSecurityChecksFailed;
            result = false;
        }

        if(result != false){
            // TODO: use a coherent minimum value (in milliseconds !)
            if(opnReq->RequestedLifetime == 0){
                //*errorStatus = OpcUa_BadInvalidArgument; => not a TCP error message authorized error
                *errorStatus = OpcUa_BadTcpInternalError;
                result = false;
            }
        }

        if(result != false){
            if(opnReq->SecurityMode == OpcUa_MessageSecurityMode_None){
                if(opnReq->ClientNonce.Length > 0){
                    // Nonce unexpected
                    // TODO: log
                }
            }else{
                assert(opnReq->SecurityMode == OpcUa_MessageSecurityMode_Sign ||
                       opnReq->SecurityMode == OpcUa_MessageSecurityMode_SignAndEncrypt);
                if(opnReq->ClientNonce.Length > 0){
                    scConnection->clientNonce = SOPC_SecretBuffer_NewFromExposedBuffer(opnReq->ClientNonce.Data, opnReq->ClientNonce.Length);
                    if(scConnection->clientNonce == NULL){
                        *errorStatus = OpcUa_BadTcpInternalError;
                        result = false;
                    }
                }else{
                    // Nonce expected
                    result = false;
                    *errorStatus = OpcUa_BadSecurityChecksFailed;
                }
            }
        }

        if(result != false){
            SOPC_SecureChannel_Config* nconfig = calloc(1, sizeof(SOPC_SecureChannel_Config));
            if(nconfig == NULL){
                result = false;
                //*errorStatus = OpcUa_BadOutOfMemory; => not a TCP error message authorized error
                *errorStatus = OpcUa_BadTcpInternalError;
            }else{
                nconfig->crt_cli = scConnection->serverAsymmSecuInfo.clientCertificate;
                nconfig->crt_srv = epConfig->serverCertificate;
                nconfig->isClientSc = false;
                nconfig->key_priv_cli = NULL;
                nconfig->msgSecurityMode = opnReq->SecurityMode;
                nconfig->pki = epConfig->pki;
                nconfig->reqSecuPolicyUri = scConnection->serverAsymmSecuInfo.securityPolicyUri;
                nconfig->requestedLifetime = opnReq->RequestedLifetime;
                nconfig->url = epConfig->endpointURL;
                idx = SOPC_ToolkitClient_AddSecureChannelConfig(nconfig);
                if(idx == 0){
                    result = false;
                    //*errorStatus = OpcUa_BadOutOfMemory; => not a TCP error message authorized error
                    *errorStatus = OpcUa_BadTcpInternalError;
                }else{
                    scConnection->endpointConnectionConfigIdx = idx;
                }
            }
        }

    }

    if(result != false){
        scConnection->state = SECURE_CONNECTION_STATE_SC_CONNECTING;
    }

    SOPC_Encodeable_Delete(&OpcUa_RequestHeader_EncodeableType,
                           (void**) &reqHeader);
    SOPC_Encodeable_Delete(&OpcUa_OpenSecureChannelRequest_EncodeableType,
                           (void**) &opnReq);
    // Clear the temporary security info recorded by chunks manager
    scConnection->serverAsymmSecuInfo.clientCertificate = NULL;
    scConnection->serverAsymmSecuInfo.securityPolicyUri = NULL;
    scConnection->serverAsymmSecuInfo.validSecurityModes = 0;
    scConnection->serverAsymmSecuInfo.isSecureModeActive = false;

    return result;
}

static bool SC_Server_GenerateFreshSecureChannelAndTokenId(SOPC_SecureConnection* scConnection,
                                                           uint32_t*              secureChannelId,
                                                           uint32_t*              tokenId){
    assert(scConnection->isServerConnection != false);
    assert(secureChannelId != NULL);
    assert(tokenId != NULL);

    bool result = false;
    uint32_t resultTokenId = 0;
    uint32_t resultSecureChannelId = 0;
    SOPC_SecureListener* scListener = &secureListenersArray[scConnection->serverEndpointConfigIdx];

    if(scListener->state == SECURE_LISTENER_STATE_OPENED){
        // Randomize secure channel ids (table 26 part 6)
        uint32_t newSecureChannelId = 0;
        uint32_t newTokenId = 0;
        uint32_t idx = 0;
        uint32_t connectionIdx = 0;
        bool occupiedScId = false;
        bool occupiedTokenId = false;
        uint8_t attempts = 5; // attempts to find a non conflicting secure Id
        while((resultSecureChannelId == 0 || resultTokenId == 0) && attempts > 0){
            attempts--;
            if(resultSecureChannelId == 0){
                SOPC_CryptoProvider_GenerateRandomID(scConnection->cryptoProvider, &newSecureChannelId);
            }
            if(resultTokenId == 0){
                SOPC_CryptoProvider_GenerateRandomID(scConnection->cryptoProvider, &newTokenId);
            }
            occupiedScId = false;
            occupiedTokenId = false;
            // A server cannot attribute 0 as secure channel id:
            //  not so clear but implied by 6.7.6 part 6: "may be 0 if the Message is an OPN"
            if(newSecureChannelId != 0 && newTokenId != 0){
                // Check if other channels already use the random id in existing connections
                for(idx = 0; idx < SOPC_MAX_SOCKETS_CONNECTIONS && (occupiedScId == false || occupiedTokenId == false); idx++){
                    if(scListener->isUsedConnectionIdxArray[idx] != false){
                        connectionIdx = scListener->connectionIdxArray[idx];
                        if(secureConnectionsArray[connectionIdx].state != SECURE_CONNECTION_STATE_SC_CLOSED){
                            if(newSecureChannelId == secureConnectionsArray[connectionIdx].currentSecurityToken.secureChannelId){
                                // If same SC id we will have to generate a new one
                                occupiedScId = true;
                            }
                            if(newTokenId == secureConnectionsArray[connectionIdx].currentSecurityToken.tokenId){
                                // If same SC id we will have to generate a new one
                                occupiedTokenId = true;
                            }
                        }
                    }
                }
                if(occupiedScId == false){
                    // Id is not used by another channel in the endpoint:
                    resultSecureChannelId = newSecureChannelId;
                }
                if(occupiedTokenId == false){
                    // Id is not used by another channel in the endpoint:
                    resultTokenId = newTokenId;
                }
            }
        }
        if(resultSecureChannelId != 0 && resultTokenId != 0){
            *secureChannelId = resultSecureChannelId;
            *tokenId = resultTokenId;
            result = true;
        }
    }
    return result;
}


// TODO: to be used for renew
static uint32_t SC_Server_GenerateFreshTokenId(SOPC_SecureConnection* scConnection){
    assert(scConnection->isServerConnection != false);

    uint32_t resultTokenId = 0;
    SOPC_SecureListener* scListener = &secureListenersArray[scConnection->serverEndpointConfigIdx];

    if(scListener->state == SECURE_LISTENER_STATE_OPENED){
        // Randomize secure channel ids (table 26 part 6)
        uint32_t newTokenId = 0;
        uint32_t idx = 0;
        uint32_t connectionIdx = 0;
        bool occupiedId = false;
        uint8_t attempts = 5; // attempts to find a non conflicting secure Id
        while(resultTokenId == 0 && attempts > 0){
            attempts--;
            SOPC_CryptoProvider_GenerateRandomID(scConnection->cryptoProvider, &newTokenId);
            occupiedId = false;
            if(newTokenId != 0){
                // Check if other channels already use the random id in existing connections
                for(idx = 0; idx < SOPC_MAX_SOCKETS_CONNECTIONS && occupiedId == false; idx++){
                    if(scListener->isUsedConnectionIdxArray[idx] != false){
                        connectionIdx = scListener->connectionIdxArray[idx];
                        if(secureConnectionsArray[connectionIdx].state != SECURE_CONNECTION_STATE_SC_CLOSED &&
                           newTokenId == secureConnectionsArray[connectionIdx].currentSecurityToken.tokenId){
                            // If same SC id we will have to generate a new one
                            occupiedId = true;
                        }
                    }
                }
                if(occupiedId == false){
                    // Id is not used by another channel in the endpoint:
                    resultTokenId = newTokenId;
                }
            }
        }
    }
    return resultTokenId;
}

static bool SC_ServerTransition_ScConnecting_To_ScConnected(SOPC_SecureConnection* scConnection,
                                                            uint32_t               scConnectionIdx,
                                                            uint32_t               requestId,
                                                            uint32_t               requestHandle){

    assert(scConnection != NULL);
    assert(scConnection->state == SECURE_CONNECTION_STATE_SC_CONNECTING);
    assert(scConnection->isServerConnection != false);
    assert(scConnection->cryptoProvider != NULL);

    bool result = false;
    SOPC_StatusCode status = STATUS_NOK;
    OpcUa_ResponseHeader respHeader;
    OpcUa_ResponseHeader_Initialize(&respHeader);
    OpcUa_OpenSecureChannelResponse opnResp;
    OpcUa_OpenSecureChannelResponse_Initialize(&opnResp);
    SOPC_Buffer* opnRespBuffer = NULL;
    SOPC_SecureChannel_Config* scConfig = NULL;

    scConfig = SOPC_Toolkit_GetSecureChannelConfig(scConnection->endpointConnectionConfigIdx);
    assert(scConfig != NULL);

    // Write the OPN response message
    opnRespBuffer = SOPC_Buffer_Create(scConnection->tcpMsgProperties.sendBufferSize);
    if(opnRespBuffer != NULL){
        result = true;
    }
    // Note: do not let size of headers for chunks manager since it is not a fixed size

    // Generate security token parameters
    if(result != false){
        // Define the security token
        result = SC_Server_GenerateFreshSecureChannelAndTokenId(scConnection,
                                                                &scConnection->currentSecurityToken.secureChannelId,
                                                                &scConnection->currentSecurityToken.tokenId);
        scConnection->currentSecurityToken.revisedLifetime = scConfig->requestedLifetime;
        //TODO: scConnection->currentSecurityToken.createdAt
    }

    // Fill response header

    if(result != false){
        // Fill the server nonce and generate key sets if applicable
        SOPC_SecretBuffer* serverNonce = NULL;
        if(scConfig->msgSecurityMode != OpcUa_MessageSecurityMode_None){
            assert(scConnection->clientNonce != NULL);

            status = SOPC_CryptoProvider_GenerateSecureChannelNonce(scConnection->cryptoProvider,
                                                               &serverNonce);

            if(status == STATUS_OK){
                result = SC_DeriveSymmetricKeySets(true,
                                                   scConnection->cryptoProvider,
                                                   scConnection->clientNonce,
                                                   serverNonce,
                                                   &scConnection->currentSecuKeySets);
            }else{
                result = false;
            }

            if(result != false){
                if(status == STATUS_OK){
                    uint8_t* bytes = NULL;
                    bytes = SOPC_SecretBuffer_Expose(serverNonce);
                    status = SOPC_ByteString_CopyFromBytes(&opnResp.ServerNonce,
                                                           bytes,
                                                           SOPC_SecretBuffer_GetLength(scConnection->clientNonce));
                }

                if(status != STATUS_OK){
                    result = false;
                }
            }

            SOPC_SecretBuffer_DeleteClear(serverNonce);
            SOPC_SecretBuffer_DeleteClear(scConnection->clientNonce);
            scConnection->clientNonce = NULL;
        }

    }

    if(result != false){
        // Fill the rest

        // TODO: timestamp
        respHeader.RequestHandle = requestHandle;

        opnResp.ServerProtocolVersion = scConnection->tcpMsgProperties.protocolVersion;
        opnResp.SecurityToken.ChannelId = scConnection->currentSecurityToken.secureChannelId;
        opnResp.SecurityToken.TokenId = scConnection->currentSecurityToken.tokenId;
        opnResp.SecurityToken.RevisedLifetime = scConnection->currentSecurityToken.revisedLifetime;
        opnResp.SecurityToken.CreatedAt = scConnection->currentSecurityToken.createdAt;

        // TODO: opnResp.ServerNonce

        // Encode the OPN message
        status = SOPC_EncodeMsg_Type_Header_Body(opnRespBuffer,
                                                 &OpcUa_OpenSecureChannelResponse_EncodeableType,
                                                 &OpcUa_ResponseHeader_EncodeableType,
                                                 (void*) &respHeader,
                                                 (void*) &opnResp);

        if(STATUS_OK != status){
            result = false;
        }
    }

    if(result != false){
        scConnection->state = SECURE_CONNECTION_STATE_SC_CONNECTED;
        SOPC_SecureChannels_EnqueueInternalEvent(INT_SC_SND_OPN,
                                                 scConnectionIdx,
                                                 (void*) opnRespBuffer,
                                                 requestId);
    }

    OpcUa_ResponseHeader_Clear(&respHeader);
    OpcUa_OpenSecureChannelResponse_Clear(&opnResp);

    return result;
}

static bool SC_ServerTransition_ScConnected_To_ScConnectedRenew(SOPC_SecureConnection* scConnection,
                                                                SOPC_Buffer*           opnReqMsgBuffer,
                                                                uint32_t*              requestHandle,
                                                                uint32_t*              requestedLifetime,
                                                                SOPC_StatusCode*       errorStatus){
    // Important note: errorStatus shall be one of the errors that can be sent with a OPC UA TCP error message (part4 Table 38)
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

    scConfig = SOPC_Toolkit_GetSecureChannelConfig(scConnection->endpointConnectionConfigIdx);
    assert(scConfig != NULL);


    if(STATUS_OK == SOPC_DecodeMsg_HeaderOrBody(opnReqMsgBuffer,
                                                &OpcUa_RequestHeader_EncodeableType,
                                                (void**) &reqHeader)){
        assert(reqHeader != NULL);
        result = true;
    }

    if(result != false){
        // TODO: check timestamp + timeout ?
        *requestHandle = reqHeader->RequestHandle;
    }

    if(result != false &&
       STATUS_OK == SOPC_DecodeMsg_HeaderOrBody(opnReqMsgBuffer,
                                                &OpcUa_OpenSecureChannelRequest_EncodeableType,
                                                (void**) &opnReq)){
        assert(opnReq != NULL);
        if(scConnection->tcpMsgProperties.protocolVersion != opnReq->ClientProtocolVersion){
            // Different protocol version provided on RENEW
            *errorStatus = OpcUa_BadProtocolVersionUnsupported;
            result = false;
        }
        if(result != false &&
           opnReq->RequestType != OpcUa_SecurityTokenRequestType_Renew){
            // Cannot Issue in CONNECTED state
            *errorStatus = OpcUa_BadSecurityChecksFailed;
            result = false;
        }

        if(result != false &&
           opnReq->SecurityMode != scConfig->msgSecurityMode){
            // Different security mode provided on RENEW
            *errorStatus = OpcUa_BadSecurityModeRejected;
            result = false;
        }

        if(result != false){
            // TODO: use a coherent minimum value (in milliseconds !)
            if(opnReq->RequestedLifetime > 0){
                *requestedLifetime = opnReq->RequestedLifetime;
            }else{
                *errorStatus = OpcUa_BadInvalidTimestampArgument;
                result = false;
            }
        }

        if(result != false){
            if(scConfig->msgSecurityMode == OpcUa_MessageSecurityMode_None){
                if(opnReq->ClientNonce.Length > 0){
                    // Nonce unexpected
                    // TODO: log
                }
            }else{
                assert(scConfig->msgSecurityMode == OpcUa_MessageSecurityMode_Sign ||
                       scConfig->msgSecurityMode == OpcUa_MessageSecurityMode_SignAndEncrypt);
                if(opnReq->ClientNonce.Length > 0){
                    scConnection->clientNonce = SOPC_SecretBuffer_NewFromExposedBuffer(opnReq->ClientNonce.Data, opnReq->ClientNonce.Length);
                    if(scConnection->clientNonce == NULL){
                        *errorStatus = OpcUa_BadTcpInternalError;
                        result = false;
                    }
                }else{
                    // Nonce expected
                    result = false;
                    *errorStatus = OpcUa_BadSecurityChecksFailed;
                }
            }
        }
    }

    if(result != false){
        scConnection->state = SECURE_CONNECTION_STATE_SC_CONNECTED_RENEW;
    }

    SOPC_Encodeable_Delete(&OpcUa_RequestHeader_EncodeableType,
                           (void**) &reqHeader);
    SOPC_Encodeable_Delete(&OpcUa_OpenSecureChannelRequest_EncodeableType,
                           (void**) &opnReq);

    return result;
}

static bool SC_ServerTransition_ScConnectedRenew_To_ScConnected(SOPC_SecureConnection* scConnection,
                                                                uint32_t               scConnectionIdx,
                                                                uint32_t               requestId,
                                                                uint32_t               requestHandle,
                                                                uint32_t               requestedLifetime){
    assert(scConnection != NULL);
    assert(scConnection->state == SECURE_CONNECTION_STATE_SC_CONNECTED_RENEW);
    assert(scConnection->isServerConnection != false);

    bool result = false;
    SOPC_StatusCode status = STATUS_NOK;
    OpcUa_ResponseHeader respHeader;
    OpcUa_ResponseHeader_Initialize(&respHeader);
    OpcUa_OpenSecureChannelResponse opnResp;
    OpcUa_OpenSecureChannelResponse_Initialize(&opnResp);
    SOPC_Buffer* opnRespBuffer = NULL;
    SOPC_SecureChannel_Config* scConfig = NULL;
    SOPC_SecureConnection_SecurityToken newSecuToken;
    SOPC_SC_SecurityKeySets                  newSecuKeySets;
    memset(&newSecuKeySets, 0, sizeof(SOPC_SC_SecurityKeySets));

    scConfig = SOPC_Toolkit_GetSecureChannelConfig(scConnection->endpointConnectionConfigIdx);
    assert(scConfig != NULL);

    // Write the OPN request message
    opnRespBuffer = SOPC_Buffer_Create(scConnection->tcpMsgProperties.sendBufferSize);
    if(opnRespBuffer != NULL){
        result = true;
    }
    // Note: do not let size of headers for chunks manager since it is not a fixed size

    // Generate security token parameters
    if(result != false){
        newSecuToken.tokenId = SC_Server_GenerateFreshTokenId(scConnection);
        if(newSecuToken.tokenId == 0){
            result = false;
        }else{
            newSecuToken.secureChannelId = scConnection->currentSecurityToken.secureChannelId;
            newSecuToken.revisedLifetime = requestedLifetime;
            //TODO: newSecuToken.createdAt
            SOPC_DateTime_Initialize(&newSecuToken.createdAt);
        }
    }

    // Fill response header

    if(result != false){
        // Fill the server nonce and generate key sets if applicable
        if(scConfig->msgSecurityMode != OpcUa_MessageSecurityMode_None){
            SOPC_SecretBuffer* serverNonce = NULL;
            assert(scConnection->clientNonce != NULL);

            status = SOPC_CryptoProvider_GenerateSecureChannelNonce(scConnection->cryptoProvider,
                                                               &serverNonce);

            if(status == STATUS_OK){
                result = SC_DeriveSymmetricKeySets(true,
                                                   scConnection->cryptoProvider,
                                                   scConnection->clientNonce,
                                                   serverNonce,
                                                   &newSecuKeySets);
            }else{
                result = false;
            }

            if(result != false){
                if(status == STATUS_OK){
                    uint8_t* bytes = NULL;
                    bytes = SOPC_SecretBuffer_Expose(serverNonce);
                    status = SOPC_ByteString_CopyFromBytes(&opnResp.ServerNonce,
                                                           bytes,
                                                           SOPC_SecretBuffer_GetLength(scConnection->clientNonce));
                }

                if(status != STATUS_OK){
                    result = false;
                }
            }

            SOPC_SecretBuffer_DeleteClear(serverNonce);
            SOPC_SecretBuffer_DeleteClear(scConnection->clientNonce);
            scConnection->clientNonce = NULL;
        }

    }

    if(result != false){
        // TODO: timestamp
        respHeader.RequestHandle = requestHandle;

        opnResp.ServerProtocolVersion = scConnection->tcpMsgProperties.protocolVersion;
        opnResp.SecurityToken.ChannelId = newSecuToken.secureChannelId;
        opnResp.SecurityToken.TokenId = newSecuToken.tokenId;
        opnResp.SecurityToken.RevisedLifetime = newSecuToken.revisedLifetime;
        opnResp.SecurityToken.CreatedAt = newSecuToken.createdAt;

        // TODO: opnResp.ServerNonce

        // Encode the OPN message
        status = SOPC_EncodeMsg_Type_Header_Body(opnRespBuffer,
                &OpcUa_OpenSecureChannelResponse_EncodeableType,
                &OpcUa_ResponseHeader_EncodeableType,
                (void*) &respHeader,
                (void*) &opnResp);

        if(STATUS_OK != status){
            result = false;
        }
    }

    if(result != false){
        scConnection->state = SECURE_CONNECTION_STATE_SC_CONNECTED;
        // copy current security token in precedent and new in current
        scConnection->precedentSecurityToken = scConnection->currentSecurityToken;
        scConnection->precedentSecuKeySets = scConnection->currentSecuKeySets;
        scConnection->currentSecurityToken = newSecuToken;
        scConnection->currentSecuKeySets = newSecuKeySets;
        // Precedent security token will remain active until expiration or client sent message with new token
        scConnection->serverNewSecuTokenActive = false;
        SOPC_SecureChannels_EnqueueInternalEvent(INT_SC_SND_OPN,
                                                 scConnectionIdx,
                                                 (void*) opnRespBuffer,
                                                 requestId);
    }else{
        if(newSecuKeySets.receiverKeySet != NULL){
            SOPC_KeySet_Delete(newSecuKeySets.receiverKeySet);
        }
        if(newSecuKeySets.senderKeySet != NULL){
            SOPC_KeySet_Delete(newSecuKeySets.senderKeySet);
        }
    }

    OpcUa_ResponseHeader_Clear(&respHeader);
    OpcUa_OpenSecureChannelResponse_Clear(&opnResp);

    return result;
}

void SOPC_SecureConnectionStateMgr_Dispatcher(SOPC_SecureChannels_InputEvent event,
                                              uint32_t                       eltId,
                                              void*                          params,
                                              int32_t                        auxParam){
    bool result = false;
    bool isExpectedType = false;
    uint32_t idx;
    SOPC_SecureChannel_Config* scConfig = NULL;
    SOPC_SecureConnection* scConnection = NULL;
    uint32_t requestHandle = 0;
    uint32_t requestedLifetime = 0;
    SOPC_StatusCode errorStatus = STATUS_OK;
    switch(event){
    /* Sockets events: */
    /* Sockets manager -> SC connection state manager */
    case SOCKET_CONNECTION:
        if(SOPC_DEBUG_PRINTING != false){
            printf("ScStateMgr: SOCKET_CONNECTION\n");
        }
        // CLIENT side only
        /* id = secure channel connection index,
           auxParam = socket index */
        scConnection = SC_GetConnection(eltId);
        if(scConnection != NULL){
            if(scConnection->state == SECURE_CONNECTION_STATE_TCP_INIT){
                result = SC_ClientTransition_TcpInit_To_TcpNegotiate(scConnection,
                                                                     eltId,
                                                                     auxParam);
                if(result == false){
                    // Error case: close the secure connection if invalid state or unexpected error.
                    //  (client case only on SOCKET_CONNECTION event)
                    SC_CloseSecureConnection(scConnection,
                                             eltId,
                                             false,
                                             0,
                                             "SecureConnection: closed on SOCKET_CONNECTION");
                }
            }else{
                // No socket connection expected just close the socket
                SOPC_Sockets_EnqueueEvent(SOCKET_CLOSE,
                                          auxParam,
                                          NULL,
                                          0);
                result = true;
            }
        }else{
            // In case of unidentified secure connection problem, close the socket just connected
            SOPC_Sockets_EnqueueEvent(SOCKET_CLOSE,
                                      auxParam,
                                      NULL,
                                      0);
        }
        break;
    case SOCKET_FAILURE:
        if(SOPC_DEBUG_PRINTING != false){
            printf("ScStateMgr: SOCKET_FAILURE\n");
        }
        /* id = secure channel connection index,
           auxParam = socket index */
        scConnection = SC_GetConnection(eltId);
        if(scConnection != NULL){
            SC_CloseSecureConnection(scConnection,
                                     eltId,
                                     false,
                                     0,
                                     "SecureConnection: disconnected (SOCKET_FAILURE event)");
        }
        break;

    /* Services events: */
    /* Services manager -> SC connection state manager */
    case SC_CONNECT:
        if(SOPC_DEBUG_PRINTING != false){
            printf("ScStateMgr: SC_CONNECT\n");
        }
        /* id = secure channel connection index */

        /* Define INIT state of a client */
        scConfig = SOPC_Toolkit_GetSecureChannelConfig(eltId);
        if(scConfig != NULL){
            result = SC_InitNewConnection(&idx);
            if(result != false){
                scConnection = SC_GetConnection(idx);
                assert(scConnection != NULL);
                // record the secure channel connection configuration
                scConnection->endpointConnectionConfigIdx = eltId;
            }
        }
        if(result == false){
            // Error case: notify services that it failed
            // TODO: add a connection failure ? (with config idx + (optional) connection id)
            SOPC_Services_EnqueueEvent(SC_TO_SE_SC_CONNECTION_TIMEOUT,
                                       eltId,
                                       NULL,
                                       0);
        }else{
            // Require a socket connection for this secure connection
            SOPC_Sockets_EnqueueEvent(SOCKET_CREATE_CLIENT,
                                      idx,
                                      (void*) scConfig->url,
                                      0);
        }
        break;
    case SC_DISCONNECT:
        if(SOPC_DEBUG_PRINTING != false){
            printf("ScStateMgr: SC_DISCONNECT\n");
        }
        /* id = secure channel connection index */
        scConnection = SC_GetConnection(eltId);
        if(scConnection != NULL){
           if(scConnection->isServerConnection == false &&
              (scConnection->state == SECURE_CONNECTION_STATE_SC_CONNECTED ||
                scConnection->state == SECURE_CONNECTION_STATE_SC_CONNECTED_RENEW)){
                SC_ClientTransition_Connected_To_Disconnected(scConnection,
                                                              eltId);
                result = true;
           }
           if(result == false){
               SC_CloseSecureConnection(scConnection,
                                        eltId,
                                        false,
                                        OpcUa_BadTcpInternalError,
                                        "Invalid secure connection state or error when sending a close secure channel request");
           }
        }
        // else => nothing to do, services should be notified that SC was already closed before calling disconnect if SC connection was valid
        break;

    case SC_SERVICE_SND_MSG:
        if(SOPC_DEBUG_PRINTING != false){
            printf("ScStateMgr: SC_SERVICE_SND_MSG\n");
        }
        /* id = secure channel connection index,
           params = (SOPC_Buffer*) received buffer,
           auxParam = request Id context (optional: defined if  >= 0) */
        scConnection = SC_GetConnection(eltId);
        if(scConnection == NULL || (scConnection->state != SECURE_CONNECTION_STATE_SC_CONNECTED &&
                                    scConnection->state != SECURE_CONNECTION_STATE_SC_CONNECTED_RENEW)){
            // Error case:
            // TODO: add event to services to notify it
        }else{
            SOPC_SecureChannels_EnqueueInternalEvent(INT_SC_SND_MSG_CHUNKS,
                                                     eltId,
                                                     params,
                                                     auxParam);
        }
        break;
    /* Test events: */
    case DEBUG_SC_FORCE_OPN_RENEW:
        if(SOPC_DEBUG_PRINTING != false){
            printf("ScStateMgr: DEBUG_SC_FORCE_OPN_RENEW\n");
        }
        scConnection = SC_GetConnection(eltId);
        if(scConnection != NULL){
            if(scConnection->state == SECURE_CONNECTION_STATE_SC_CONNECTED && scConnection->isServerConnection == false){
                result = SC_ClientTransition_ScConnected_To_ScConnectedRenew(scConnection,
                                                                             eltId);
            }

            if(result == false){
                // Error case: close the connection
                SC_CloseSecureConnection(scConnection,
                                         eltId,
                                         false,
                                         OpcUa_BadTcpInternalError,
                                         "Open secure channel forced renew failed");
            }
        }
        break;

    /* Internal events: */
    /* SC listener manager -> SC connection manager */
    case INT_EP_SC_CREATE:
        if(SOPC_DEBUG_PRINTING != false){
            printf("ScStateMgr: INT_EP_SC_CREATE\n");
        }
        /* id = endpoint description configuration index,
           auxParam = socket index */
        result = SC_InitNewConnection(&idx);
        if(result != false){
            scConnection = SC_GetConnection(idx);
            assert(scConnection != NULL);

            // set the socket index associated
            scConnection->socketIndex = auxParam;
            // record the endpoint description configuration
            scConnection->serverEndpointConfigIdx = eltId;
            // set connection as a server side connection
            scConnection->isServerConnection = true;

            // notify socket that connection is accepted
            SOPC_Sockets_EnqueueEvent(SOCKET_ACCEPTED_CONNECTION,
                                      auxParam,
                                      NULL,
                                      idx);
            // notify secure listener that connection is accepted
            SOPC_SecureChannels_EnqueueInternalEvent(INT_EP_SC_CREATED,
                                                     eltId,
                                                     NULL,
                                                     idx);

        }else{
            // Error case: request to close the socket newly created
            //             / nothing to send to SC listener state manager (no record of new connection in it for now)
            SOPC_Sockets_EnqueueEvent(SOCKET_CLOSE,
                                      auxParam,
                                      NULL,
                                      0);
        }
        break;

    /* OPC UA chunks message manager -> SC connection manager */
    case INT_SC_RCV_HEL:
        if(SOPC_DEBUG_PRINTING != false){
            printf("ScStateMgr: INT_SC_RCV_HEL\n");
        }
        scConnection = SC_GetConnection(eltId);
        if(scConnection != NULL){
            if(scConnection->state == SECURE_CONNECTION_STATE_TCP_INIT && scConnection->isServerConnection != false){
                result = SC_ServerTransition_TcpInit_To_TcpNegotiate(scConnection,
                                                                     (SOPC_Buffer*) params,
                                                                     &errorStatus);
                if(result != false){
                    result = SC_ServerTransition_TcpNegotiate_To_ScInit(scConnection,
                                                                        eltId,
                                                                        (SOPC_Buffer*) params,
                                                                        &errorStatus);
                }

                if(result == false){
                    SC_CloseSecureConnection(scConnection,
                                                    eltId,
                                                    false,
                                                    errorStatus,
                                                    "Error on HELLO message treatment");
                }
            }else{
                SC_CloseSecureConnection(scConnection,
                                                eltId,
                                                false,
                                                OpcUa_BadTcpInternalError,
                                                "Hello message received not expected");
            }
        }
        // else: nothing to do (=> socket should already be required to close)
        if(result == false){
            // In other case buffer has been resent as response and shall not be freed
            SOPC_Buffer_Delete((SOPC_Buffer*) params);
        }
        break;
    case INT_SC_RCV_ACK:
        if(SOPC_DEBUG_PRINTING != false){
            printf("ScStateMgr: INT_SC_RCV_ACK\n");
        }
        scConnection = SC_GetConnection(eltId);
        if(scConnection != NULL){
            if(scConnection->state == SECURE_CONNECTION_STATE_TCP_NEGOTIATE && scConnection->isServerConnection == false){
                result = SC_ClientTransition_TcpNegotiate_To_ScInit(scConnection,
                                                                    (SOPC_Buffer*) params);
                if(result != false){
                    result = SC_ClientTransition_ScInit_To_ScConnecting(scConnection,
                                                                        eltId);
                }
            }

            if(result == false){
                // Error case: close the connection
                SC_CloseSecureConnection(scConnection,
                                         eltId,
                                         false,
                                         OpcUa_BadTcpInternalError,
                                         "Invalid or unexpected Hello message received");
            }
        }
        // else: nothing to do (=> socket should already be required to close)
        SOPC_Buffer_Delete((SOPC_Buffer*) params);
        break;
    case INT_SC_RCV_OPN:
        if(SOPC_DEBUG_PRINTING != false){
            printf("ScStateMgr: INT_SC_RCV_OPN\n");
        }
        scConnection = SC_GetConnection(eltId);
        if(scConnection != NULL){
            if((scConnection->state == SECURE_CONNECTION_STATE_SC_CONNECTING ||
                scConnection->state == SECURE_CONNECTION_STATE_SC_CONNECTED_RENEW)
               && scConnection->isServerConnection == false)
            { // CLIENT SIDE: OPN response for new SC or for renew SC (symmetric keys)
                // Check the OPC UA msg is an OPN resp
                isExpectedType = SC_ReadAndCheckOpcUaMessageType(&OpcUa_OpenSecureChannelResponse_EncodeableType,
                                                                 (SOPC_Buffer*) params);

                if(isExpectedType != false){
                    if(scConnection->state == SECURE_CONNECTION_STATE_SC_CONNECTING){
                        // transition: Connecting => Connected
                        result = SC_ClientTransition_ScConnecting_To_ScConnected(scConnection,
                                                                                 eltId,
                                                                                 (SOPC_Buffer*) params);
                    }else if(scConnection->state == SECURE_CONNECTION_STATE_SC_CONNECTED_RENEW){
                        // transition: Connected_Renew => Connected
                        result = SC_ClientTransition_ScConnectedRenew_To_ScConnected(scConnection,
                                                                                     eltId,
                                                                                     (SOPC_Buffer*) params);
                    }else{
                        assert(false);
                    }

                    if(result == false){
                        SC_CloseSecureConnection(scConnection,
                                                 eltId,
                                                 false,
                                                 OpcUa_BadInvalidArgument,
                                                 "Failure during OpenSecureChannel response treatment");
                    }else{
                        SOPC_Services_EnqueueEvent(SC_TO_SE_SC_CONNECTED,
                                                   eltId,
                                                   NULL,
                                                   scConnection->endpointConnectionConfigIdx);
                    }
                }else{
                    SC_CloseSecureConnection(scConnection,
                                             eltId,
                                             false,
                                             OpcUa_BadRequestNotAllowed,
                                             "Unexpected OpenSecureChannel request received by client");
                }

            }else if((scConnection->state == SECURE_CONNECTION_STATE_SC_INIT ||
                    scConnection->state == SECURE_CONNECTION_STATE_SC_CONNECTED)
                    && scConnection->isServerConnection != false)
            {// SERVER SIDE: OPN request for new SC or for renew SC (symmetric keys)
                // Check the OPC UA msg is an OPN req
                isExpectedType = SC_ReadAndCheckOpcUaMessageType(&OpcUa_OpenSecureChannelRequest_EncodeableType,
                                                                 (SOPC_Buffer*) params);
                if(isExpectedType != false){
                    if(scConnection->state == SECURE_CONNECTION_STATE_SC_INIT){
                        // transition: Init => Connecting
                        result = SC_ServerTransition_ScInit_To_ScConnecting(scConnection,
                                                                            (SOPC_Buffer*) params,
                                                                            &requestHandle,
                                                                            &errorStatus);
                        if(result != false){
                            // transition: Connecting => Connected
                            result = SC_ServerTransition_ScConnecting_To_ScConnected(scConnection,
                                                                                     eltId,
                                                                                     auxParam,
                                                                                     requestHandle);
                            if(result == false){
                                // since security verifications were done before, it should be an internal error
                                // (moreover we cannot detail error statuses before SC established)
                                errorStatus = OpcUa_BadTcpInternalError;
                            }
                        }

                        if(result == false){
                            SC_CloseSecureConnection(scConnection,
                                                     eltId,
                                                     false,
                                                     errorStatus,
                                                     "Failure during new OpenSecureChannel request treatment");
                        }else{
                            SOPC_Services_EnqueueEvent(SC_TO_SE_EP_SC_CONNECTED,
                                                       scConnection->serverEndpointConfigIdx,
                                                       (void*) &scConnection->endpointConnectionConfigIdx,
                                                       eltId);
                        }
                    }else if(scConnection->state == SECURE_CONNECTION_STATE_SC_CONNECTED){
                        // transition: Connected => ConnectedRenew
                        result = SC_ServerTransition_ScConnected_To_ScConnectedRenew(scConnection,
                                                                                     (SOPC_Buffer*) params,
                                                                                     &requestHandle,
                                                                                     &requestedLifetime,
                                                                                     &errorStatus);
                        if(result != false){
                            // transition: ConnectedRenew => Connected
                            result = SC_ServerTransition_ScConnectedRenew_To_ScConnected(scConnection,
                                                                                         eltId,
                                                                                         auxParam,
                                                                                         requestHandle,
                                                                                         requestedLifetime);
                            // used only if result == false
                            errorStatus = OpcUa_BadTcpInternalError;
                        }

                        if(result == false){
                            // TODO: send a service fault ?
                            SC_CloseSecureConnection(scConnection,
                                                     eltId,
                                                     false,
                                                     errorStatus,
                                                     "Failure during renew OpenSecureChannel request treatment");
                        }
                    }
                }else{
                    // OPC UA message type is unexpected: close the connection
                    // Note: it is not really a security check that failed but it is the generic error to send in the current state
                    SC_CloseSecureConnection(scConnection,
                                             eltId,
                                             false,
                                             OpcUa_BadSecurityChecksFailed,
                                             "TCP UA OPN message content is not the one expected (wrong type: request/response or message type)");
                }
            }else{
                // Error case: close the connection
                // Note: it is not really a security check that failed but in case the SC connection is not established yet,
                //       it is the error to send. It can be changed to more precise errors by checking again the state (CONNECTED* allows precise error).
                SC_CloseSecureConnection(scConnection,
                                         eltId,
                                         false,
                                         OpcUa_BadSecurityChecksFailed,
                                         "Invalid state to receive an OPN request");
            }
        }
        // else: nothing to do (=> socket should already be required to close)
        SOPC_Buffer_Delete((SOPC_Buffer*) params);
        break;
    case INT_SC_RCV_CLO:
        if(SOPC_DEBUG_PRINTING != false){
            printf("ScStateMgr: INT_SC_RCV_CLO\n");
        }
        scConnection = SC_GetConnection(eltId);
        if(scConnection != NULL){
            if((scConnection->state == SECURE_CONNECTION_STATE_SC_CONNECTED ||
               scConnection->state == SECURE_CONNECTION_STATE_SC_CONNECTED_RENEW)
               && scConnection->isServerConnection != false){
                // SERVER side
                isExpectedType = SC_ReadAndCheckOpcUaMessageType(&OpcUa_CloseSecureChannelRequest_EncodeableType,
                                                                 (SOPC_Buffer*) params);
                if(isExpectedType != false){
                    result = true;
                    // Just close the socket without any error (Part 6 §7.1.4)
                    SC_CloseSecureConnection(scConnection,
                                             eltId,
                                             true, // consider socket is closed since client should have closed now
                                             OpcUa_BadSecureChannelClosed,
                                             "CLO request message received");
                }else{
                    // Close the socket after reporting an error (Part 6 §7.1.4)
                    // Note: use a security check failure error since it is an incorrect use of protocol
                    SC_CloseSecureConnection(scConnection,
                                             eltId,
                                             false,
                                             OpcUa_BadSecurityChecksFailed,
                                             "Invalid CLO request message");
                }
            }else{
                // Close the socket after reporting an error (Part 6 §7.1.4)
                // Note: use a security check failure error since either SC is not established or unexpected type of message was sent
                SC_CloseSecureConnection(scConnection,
                                         eltId,
                                         false,
                                         OpcUa_BadSecurityChecksFailed,
                                         "Failure when encoding a message to send on the secure connection");
            }
        }
        // else: nothing to do (=> socket should already be required to close)
        SOPC_Buffer_Delete((SOPC_Buffer*) params);
        break;
    case INT_SC_RCV_MSG_CHUNKS:
        if(SOPC_DEBUG_PRINTING != false){
            printf("ScStateMgr: INT_SC_RCV_MSG_CHUNKS\n");
        }
        scConnection = SC_GetConnection(eltId);
        if(scConnection != NULL){
            if(scConnection->state == SECURE_CONNECTION_STATE_SC_CONNECTED ||
               scConnection->state == SECURE_CONNECTION_STATE_SC_CONNECTED_RENEW){
                assert(params != NULL);
                // Do not delete buffer since it is provided to services:
                result = true;
                if(scConnection->isServerConnection == false){
                    // Do not provide request Id since it is client side (no response to send)
                    auxParam = 0;
                }
                // No server / client differentiation at this level
                SOPC_Services_EnqueueEvent(SC_TO_SE_SC_SERVICE_RCV_MSG,
                                           eltId, // secure connection id
                                           params, // buffer
                                           auxParam); // request Id
            }else{
                // Error case: close the socket with security check failure since SC is not established
                SC_CloseSecureConnection(scConnection,
                                         eltId,
                                         false,
                                         OpcUa_BadSecurityChecksFailed,
                                         "SecureConnection: received an OpcUa message on not established secure connection");
            }
        }
        // else: nothing to do (=> socket should already be required to close)
        if(result == false){
            // In other case, buffer has been transmitted to services layer
            SOPC_Buffer_Delete((SOPC_Buffer*) params);
        }
        break;
    case INT_SC_RCV_FAILURE:
        if(SOPC_DEBUG_PRINTING != false){
            printf("ScStateMgr: INT_SC_RCV_FAILURE\n");
        }
        scConnection = SC_GetConnection(eltId);
        if(scConnection != NULL){
             SC_CloseSecureConnection(scConnection,
                                      eltId,
                                      false,
                                      auxParam,
                                      "Failure when receiving a message on the secure connection");
        } // else: nothing to do (=> socket should already be required to close)
        break;
    case INT_SC_SND_FAILURE:
        if(SOPC_DEBUG_PRINTING != false){
            printf("ScStateMgr: INT_SC_SND_FAILURE\n");
        }
        scConnection = SC_GetConnection(eltId);
        if(scConnection != NULL){
            SC_CloseSecureConnection(scConnection,
                                     eltId,
                                     false,
                                     auxParam,
                                     "Failure when encoding a message to send on the secure connection");
        } // else: nothing to do (=> socket should already be required to close)
        break;
    case INT_SC_RCV_ERR:
        if(SOPC_DEBUG_PRINTING != false){
            printf("ScStateMgr: INT_SC_RCV_ERR\n");
        }
        /* id = secure channel connection index,
           auxParam = params = (SOPC_Buffer*) buffer */
        scConnection = SC_GetConnection(eltId);

        // TODO: Decode ERR message and use reason/error code (received on client side only ! => guaranteed by chunks manager filtering)
        SC_CloseSecureConnection(scConnection,
                                 eltId,
                                 true, // consider socket is closed since server should have closed now
                                 OpcUa_BadSecureChannelClosed,
                                 "ERR message received");
        break;
    case INT_EP_SC_CLOSE:
        if(SOPC_DEBUG_PRINTING != false){
            printf("ScStateMgr: INT_EP_SC_CLOSE\n");
        }
        /* id = secure channel connection index,
           auxParam = endpoint description configuration index */
        scConnection = SC_GetConnection(eltId);

        if(scConnection != NULL){
             SC_CloseSecureConnection(scConnection,
                                      eltId,
                                      false,
                                      OpcUa_BadSecureChannelClosed,
                                      "Endpoint secure connection listener closed");
        }
        break;
    case INT_SC_CLOSE:
        if(SOPC_DEBUG_PRINTING != false){
            printf("ScStateMgr: INT_SC_CLOSE\n");
        }
        /* id = secure channel connection index */
        scConnection = SC_GetConnection(eltId);

        if(scConnection != NULL){
            SC_CloseSecureConnection(scConnection,
                                     eltId,
                                     true, // consider socket is closed since it shall be closed immediatly now
                                     auxParam,
                                     (char*) params);
        }
        break;
    default:
        // Already filtered by secure channels API module
        assert(false);
    }
}

