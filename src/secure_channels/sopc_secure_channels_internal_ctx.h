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

#ifndef SOPC_SECURE_CHANNELS_INTERNAL_CTX_H_
#define SOPC_SECURE_CHANNELS_INTERNAL_CTX_H_

#include <stdbool.h>
#include <stdint.h>

#include "../helpers_crypto/sopc_key_sets.h"
#include "sopc_toolkit_constants.h"
#include "sopc_builtintypes.h"
#include "sopc_singly_linked_list.h"
#include "sopc_secret_buffer.h"
#include "sopc_crypto_decl.h"
#include "sopc_types.h"

typedef enum {
    SECURE_LISTENER_STATE_CLOSED = 0,
    SECURE_LISTENER_STATE_OPENING,
    SECURE_LISTENER_STATE_OPENED
} SOPC_SecureListener_State;

typedef enum {
    SECURE_CONNECTION_STATE_SC_CLOSED = 0,
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
typedef enum {
    SOPC_MSG_TYPE_INVALID = 0,
    SOPC_MSG_TYPE_HEL,
    SOPC_MSG_TYPE_ACK,
    SOPC_MSG_TYPE_ERR,
    SOPC_MSG_TYPE_SC_OPN,
    SOPC_MSG_TYPE_SC_CLO,
    SOPC_MSG_TYPE_SC_MSG
} SOPC_Msg_Type;

/**
 *  \brief UA Message Chunk IsFinal type
 */
typedef enum {
    SOPC_MSG_ISFINAL_INVALID = 0,
    SOPC_MSG_ISFINAL_INTERMEDIATE, /**< C type */
    SOPC_MSG_ISFINAL_FINAL,        /**< F type */
    SOPC_MSG_ISFINAL_ABORT         /**< A type */
} SOPC_Msg_IsFinal;

// Chunk manager context
typedef struct SOPC_SecureConnection_ChunkMgrCtx {
    SOPC_Buffer*     chunkInputBuffer;
    uint32_t         currentMsgSize;
    SOPC_Msg_Type    currentMsgType;
    SOPC_Msg_IsFinal currentMsgIsFinal;
} SOPC_SecureConnection_ChunkMgrCtx;

// Set on HEL/ACK exchange (see OPC UA specification Part 6 table 36/37)
typedef struct SOPC_SecureConnection_TcpProperties {
    uint32_t protocolVersion;
    uint32_t receiveBufferSize; // Maximum size of connection for reception (static by configuration)
    uint32_t sendBufferSize;    // Maximum size of connection for sending (dynamic on HEL/ACK exchange)
    uint32_t maxMessageSize;    // Maximum size of OPC UA message BODY (see also part 4 §5.3 last § for more detail)
    uint32_t maxChunkCount;     // Maximum number of chunks accepted
} SOPC_SecureConnection_TcpProperties;

// Set on OPN request reception (see OPC UA specification Part 6 table 27): necessary to check coherence with body OPN message content
typedef struct SOPC_SecureConnection_TcpOpnReqAsymmSecu {
    const char*               securityPolicyUri;
    uint16_t                  validSecurityModes;  // accepted security mode for the valid security policy requested
    bool                      isSecureModeActive; // a secure mode is active (sign or signAndEncrypt) choice based on certificates presence in OPN
    SOPC_Certificate*              clientCertificate; /* temporary record of the client certificate */
} SOPC_SecureConnection_TcpAsymmSecu;


// See Part 6 table 29
typedef struct SOPC_SecureConnection_TcpSequenceProperties {
    uint32_t          lastSNsent;     // Last sequence number sent on connection
    uint32_t          lastSNreceived; // Last sequence number received on connection
    SOPC_SLinkedList* sentRequestIds; // Request ids sent for which a response can be received
} SOPC_SecureConnection_TcpSequenceProperties;

typedef struct SOPC_SecureConnection_SecurityToken
{
    uint32_t      secureChannelId; // TODO: move secure channel Id outside (it shall not be changed with the token)
    uint32_t      tokenId;
    SOPC_DateTime createdAt;
    uint32_t      revisedLifetime;
}
SOPC_SecureConnection_SecurityToken;

typedef struct SOPC_SecureConnection {
    /* Set and accessed only by Chunks manager */
    SOPC_SecureConnection_ChunkMgrCtx           chunksCtx;
    /* Set by Chunks manager */
    SOPC_SecureConnection_TcpSequenceProperties tcpSeqProperties;
    uint32_t                                    asymmSecuMaxBodySize;
    uint32_t                                    symmSecuMaxBodySize;

    // (Client side specific)
    uint32_t                                    clientSecureChannelId;  // Temporary recorded information from the OPN response TCP message
    // (Client side specific)
    uint32_t                                    clientLastReqId; // client last request Id used
    // (Server side specific)
    SOPC_SecureConnection_TcpAsymmSecu          serverAsymmSecuInfo; // Temporary recorded information form the OPN request asymmetric security header
    

    /* Set by SC connection state manager */
    SOPC_SecureConnection_State                 state;
    uint32_t                                    endpointConnectionConfigIdx;
    uint32_t                                    socketIndex; // associated TCP socket index (defined when state != TCP_INIT or SC_CLOSED)

    // Message body content dependent properties
    SOPC_SecureConnection_TcpProperties         tcpMsgProperties;
    SOPC_CryptoProvider*                             cryptoProvider; // defined once security policy id define (OPN req)
    SOPC_SecureConnection_SecurityToken         precedentSecurityToken;
    SOPC_SC_SecurityKeySets                          precedentSecuKeySets;
    SOPC_SecureConnection_SecurityToken         currentSecurityToken;
    SOPC_SC_SecurityKeySets                          currentSecuKeySets;
    // (Server side specific)
    SOPC_SecretBuffer*                               clientNonce; // client nonce used to create symmetric key  
    // (Server side specific)
    bool                                        serverNewSecuTokenActive;

    /* Server or Client side connection */
    bool                                        isServerConnection;
    // (Server side specific)
    uint32_t                                    serverEndpointConfigIdx; // endpoint description configuration association

} SOPC_SecureConnection;

typedef struct SOPC_SecureListener {
    SOPC_SecureListener_State state;
    uint32_t                  serverEndpointConfigIdx;
    uint32_t                  socketIndex; // associated TCP socket index (in OPENED state only)
    // Management of the active connections on the listener
    uint32_t                  connectionIdxArray[SOPC_MAX_SOCKETS_CONNECTIONS]; // index of connected connections on the listener
    bool                      isUsedConnectionIdxArray[SOPC_MAX_SOCKETS_CONNECTIONS]; //
    uint32_t                  lastConnectionIdxArrayIdx;
} SOPC_SecureListener;

/** @brief Array containing all listeners that can be used */
extern SOPC_SecureListener secureListenersArray[SOPC_MAX_ENDPOINT_DESCRIPTION_CONFIGURATIONS];

/** @brief Array containing all connections that can be used */
extern SOPC_SecureConnection secureConnectionsArray[SOPC_MAX_SECURE_CONNECTIONS];
extern uint32_t              lastSecureConnectionArrayIdx; // last secure connection index used for a new secure connection


/** @brief Initialize the array of secure listeners/connections */
void SOPC_SecureChannelsInternalContext_Initialize(void);

/** @brief Clear the array of secure listeners/connections */
void SOPC_SecureChannelsInternalContext_Clear(void);

#endif /* SOPC_SECURE_CHANNELS_INTERNAL_CTX_H_ */
