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

#ifndef SOPC_SECURE_CHANNELS_INTERNAL_CTX_H_
#define SOPC_SECURE_CHANNELS_INTERNAL_CTX_H_

#include <stdbool.h>
#include <stdint.h>

#include "sopc_builtintypes.h"
#include "sopc_crypto_decl.h"
#include "sopc_event_handler.h"
#include "sopc_key_sets.h"
#include "sopc_secret_buffer.h"
#include "sopc_singly_linked_list.h"
#include "sopc_time.h"
#include "sopc_toolkit_config_constants.h"
#include "sopc_types.h"

typedef enum
{
    SECURE_LISTENER_STATE_CLOSED = 0,
    SECURE_LISTENER_STATE_OPENING,
    SECURE_LISTENER_STATE_OPENED,
    SECURE_LISTENER_STATE_INACTIVE /*<< This state is used when only reverse connections are managed but no listener is
                                      active. Listener is still used to keep SC associated to this EP. */
} SOPC_SecureListener_State;

typedef enum
{
    SECURE_CONNECTION_STATE_SC_CLOSED = 0,
    SECURE_CONNECTION_STATE_TCP_REVERSE_INIT,
    SECURE_CONNECTION_STATE_TCP_INIT,
    SECURE_CONNECTION_STATE_TCP_NEGOTIATE,
    SECURE_CONNECTION_STATE_SC_INIT,
    SECURE_CONNECTION_STATE_SC_CONNECTING,
    SECURE_CONNECTION_STATE_SC_CONNECTED,
    SECURE_CONNECTION_STATE_SC_CONNECTED_RENEW
} SOPC_SecureConnection_State;

/**
 *  \brief TCP UA Message types
 */
typedef enum
{
    SOPC_MSG_TYPE_INVALID = 0,
    SOPC_MSG_TYPE_HEL,
    SOPC_MSG_TYPE_ACK,
    SOPC_MSG_TYPE_ERR,
    SOPC_MSG_TYPE_RHE,
    SOPC_MSG_TYPE_SC_OPN,
    SOPC_MSG_TYPE_SC_CLO,
    SOPC_MSG_TYPE_SC_MSG
} SOPC_Msg_Type;

/**
 * \brief Structure containing the context of a sent request message at SC layer level
 */
typedef struct
{
    bool timeoutExpired; // set to true if the response shall now be ignored
    uint32_t scConnectionIdx;
    uint32_t requestHandle;
    SOPC_Msg_Type msgType;
    uint32_t timerId;
} SOPC_SentRequestMsg_Context;

/**
 *  \brief UA Message Chunk IsFinal type
 */
typedef enum
{
    SOPC_MSG_ISFINAL_INVALID = 0,
    SOPC_MSG_ISFINAL_INTERMEDIATE, /**< C type */
    SOPC_MSG_ISFINAL_FINAL,        /**< F type */
    SOPC_MSG_ISFINAL_ABORT         /**< A type */
} SOPC_Msg_IsFinal;

// Chunk manager context
typedef struct SOPC_SecureConnection_ChunkMgrCtx
{
    SOPC_Buffer* currentChunkInputBuffer; // The current chunk bytes accumulation
    uint32_t currentMsgSize;
    SOPC_Msg_Type currentMsgType;
    SOPC_Msg_IsFinal currentMsgIsFinal;
    SOPC_SLinkedList* intermediateChunksInputBuffers;
    bool hasCurrentMsgRequestId;            // Set to true if currentRequetId is set (set when first chunk received)
    uint32_t currentMsgRequestId;           // The valid requestId of the current message in reception
                                            // (shall be the same for all chunks)
    SOPC_Buffer* currentMessageInputBuffer; // The message (from one or several chunks) received:
                                            // only set when message is complete
} SOPC_SecureConnection_ChunkMgrCtx;

// Set on HEL/ACK exchange (see OPC UA specification Part 6 table 36/37)
typedef struct SOPC_SecureConnection_TcpProperties
{
    uint32_t protocolVersion;
    uint32_t receiveBufferSize; // Maximum size of connection for reception (static by configuration)
    uint32_t sendBufferSize;    // Maximum size of connection for sending (dynamic on HEL/ACK exchange)
    uint32_t
        receiveMaxMessageSize; // Max size of received OPC UA message BODY (see also part 4 §5.3 last § for more detail)
    uint32_t sendMaxMessageSize;   // Max size of OPC UA message BODY that could be used to send a message
    uint32_t receiveMaxChunkCount; // Maximum number of chunks accepted for a message received
    uint32_t sendMaxChunkCount;    // Maximum number of chunks that could be used to send a message
} SOPC_SecureConnection_TcpProperties;

// Set on OPN request reception (see OPC UA specification Part 6 table 27): necessary to check coherence with body OPN
// message content
typedef struct SOPC_SecureConnection_TcpOpnReqAsymmSecu
{
    const char* securityPolicyUri;
    uint16_t validSecurityModes; // accepted security mode for the valid security policy requested
    bool isSecureModeActive; // a secure mode is active (sign or signAndEncrypt) choice based on certificates presence
                             // in OPN
    SOPC_CertificateList* clientCertificate; /* temporary record of the client certificate */
} SOPC_SecureConnection_TcpAsymmSecu;

// See Part 6 table 29
typedef struct SOPC_SecureConnection_TcpSequenceProperties
{
    uint32_t lastSNsent;              // Last sequence number sent on connection
    uint32_t lastSNreceived;          // Last sequence number received on connection
    SOPC_SLinkedList* sentRequestIds; // Request ids sent for which a response can be received
} SOPC_SecureConnection_TcpSequenceProperties;

typedef struct SOPC_SecureConnection_SecurityToken
{
    uint32_t secureChannelId; // TODO: move secure channel Id outside (it shall not be changed with the token)
    uint32_t tokenId;
    SOPC_DateTime createdAt;               // OpcUa date format
    SOPC_TimeReference lifetimeEndTimeRef; // target time reference (monotonic)
    uint32_t revisedLifetime;
} SOPC_SecureConnection_SecurityToken;

typedef struct SOPC_SecureConnection
{
    /* Set and accessed only by Chunks manager */
    SOPC_SecureConnection_ChunkMgrCtx chunksCtx;
    /* Set by Chunks manager */
    SOPC_SecureConnection_TcpSequenceProperties tcpSeqProperties;
    uint32_t asymmSecuMaxBodySize;
    uint32_t symmSecuMaxBodySize;
    bool hasExtraPaddingSize;

    // (Client side specific)
    uint32_t clientSecureChannelId; // Temporary recorded information from the OPN response TCP message
    // (Client side specific)
    uint32_t clientNextReqId; // client last request Id used
    // (Server side specific)
    SOPC_SecureConnection_TcpAsymmSecu
        serverAsymmSecuInfo; // Temporary recorded information form the OPN request asymmetric security header

    /* Set by SC connection state manager */
    SOPC_SecureConnection_State state;
    uint32_t endpointConnectionConfigIdx;
    // SC reverse connection info
    bool isReverseConnection; // use ReverseHello mechanism for this connection
    uint16_t
        reverseConnIdx; // reverse connection index in endpoint configuration (set only if isReverseConnection is true)

    uint32_t socketIndex; // associated TCP socket index (defined when state != TCP_INIT or SC_CLOSED)

    // SC connection timeout management
    uint32_t connectionTimeoutTimerId;

    // Message body content dependent properties
    SOPC_SecureConnection_TcpProperties tcpMsgProperties;
    SOPC_CryptoProvider* cryptoProvider; // defined once security policy id define (OPN req)
    SOPC_SecureConnection_SecurityToken precedentSecurityToken;
    SOPC_SC_SecurityKeySets precedentSecuKeySets;
    SOPC_SecureConnection_SecurityToken currentSecurityToken;
    SOPC_SC_SecurityKeySets currentSecuKeySets;
    // (Server side specific)
    SOPC_SecretBuffer* clientNonce; // client nonce used to create symmetric key

    // (Client side specific)
    uint32_t secuTokenRenewTimerId;

    // (Server side specific)
    // flag indicating if the new (current) security token shall be used to send MSG otherwise use precedent until
    // new one activated by client (reception of MSG with new token)
    bool serverNewSecuTokenActive;

    /* Server or Client side connection */
    bool isServerConnection;
    // (Server side specific)
    uint32_t serverEndpointConfigIdx; // endpoint description configuration association

    SOPC_AsymmetricKey* privateKey;
    SOPC_CertificateList* serverCertificate;
    SOPC_CertificateList* clientCertificate;
} SOPC_SecureConnection;

typedef struct SOPC_SecureListener
{
    SOPC_SecureListener_State state;
    uint32_t serverEndpointConfigIdx;
    uint32_t socketIndex; // associated TCP socket index (in OPENED state only)
    // Management of the active connections on the listener
    uint32_t connectionIdxArray[SOPC_MAX_SOCKETS_CONNECTIONS];   // index of connected connections on the listener
    bool isUsedConnectionIdxArray[SOPC_MAX_SOCKETS_CONNECTIONS]; //
    uint32_t lastConnectionIdxArrayIdx;
    // Management of retry timeouts for reverse connection to clients
    uint32_t reverseConnRetryTimerIds[SOPC_MAX_REVERSE_CLIENT_CONNECTIONS];
} SOPC_SecureListener;

/** @brief Array containing all listeners that can be used */
extern SOPC_SecureListener secureListenersArray[SOPC_MAX_ENDPOINT_DESCRIPTION_CONFIGURATIONS + 1];

/** @brief Array containing all connections that can be used */
extern SOPC_SecureConnection secureConnectionsArray[SOPC_MAX_SECURE_CONNECTIONS_PLUS_BUFFERED + 1];
extern uint32_t lastSecureConnectionArrayIdx; // last secure connection index used for a new secure connection

extern SOPC_Looper* secureChannelsLooper;
extern SOPC_EventHandler* secureChannelsInputEventHandler;
extern SOPC_EventHandler* secureChannelsInternalEventHandler;
extern SOPC_EventHandler* secureChannelsTimerEventHandler;
extern SOPC_EventHandler* secureChannelsEventHandler;

/** @brief Initialize the array of secure listeners/connections */
void SOPC_SecureChannelsInternalContext_Initialize(SOPC_SetListenerFunc* setSocketsListener);

/** @brief Clear the array of secure listeners/connections */
void SOPC_SecureChannelsInternalContext_Clear(void);

SOPC_SecureConnection* SC_GetConnection(uint32_t connectionIdx);

const SOPC_CertificateList* SC_OwnCertificate(SOPC_SecureConnection* conn);
const SOPC_CertificateList* SC_PeerCertificate(SOPC_SecureConnection* conn);

/** @brief Retrieve the number of intermediate chunks already received */
uint32_t SOPC_ScInternalContext_GetNbIntermediateInputChunks(SOPC_SecureConnection_ChunkMgrCtx* chunkCtx);
/** @brief Add an intermediate chunk received into message chunks */
bool SOPC_ScInternalContext_AddIntermediateInputChunk(SOPC_SecureConnection_TcpProperties* tcpProperties,
                                                      SOPC_SecureConnection_ChunkMgrCtx* chunkCtx,
                                                      SOPC_Buffer* intermediateChunk);
/** @brief Clear the list of intermediate chunks received
 *  (should be called directly only in case of abort chunk received) */
void SOPC_ScInternalContext_ClearIntermediateInputChunks(SOPC_SecureConnection_ChunkMgrCtx* chunkCtx);

/** @brief Clear the current chunk context but not the intermediate chunks context */
void SOPC_ScInternalContext_ClearCurrentInputChunkContext(SOPC_SecureConnection_ChunkMgrCtx* chunkCtx);

/** @brief Clear the current chunk and intermediate chunks context */
void SOPC_ScInternalContext_ClearInputChunksContext(SOPC_SecureConnection_ChunkMgrCtx* chunkCtx);

void SOPC_SecureChannels_OnInternalEvent(SOPC_EventHandler* handler,
                                         int32_t event,
                                         uint32_t id,
                                         uintptr_t params,
                                         uintptr_t auxParam);
void SOPC_SecureChannels_OnSocketsEvent(SOPC_EventHandler* handler,
                                        int32_t event,
                                        uint32_t id,
                                        uintptr_t params,
                                        uintptr_t auxParam);
void SOPC_SecureChannels_OnTimerEvent(SOPC_EventHandler* handler,
                                      int32_t event,
                                      uint32_t id,
                                      uintptr_t params,
                                      uintptr_t auxParam);
void SOPC_SecureChannels_OnInputEvent(SOPC_EventHandler* handler,
                                      int32_t event,
                                      uint32_t id,
                                      uintptr_t params,
                                      uintptr_t auxParam);

#endif /* SOPC_SECURE_CHANNELS_INTERNAL_CTX_H_ */
