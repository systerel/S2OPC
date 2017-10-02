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

#ifndef SOPC_SECURE_CHANNEL_LOW_LEVEL_H_
#define SOPC_SECURE_CHANNEL_LOW_LEVEL_H_

#include "key_manager.h"
#include "key_sets.h"
#include "pki.h"
#include "sopc_types.h"
#include "sopc_tcp_ua_connection.h"

extern const uint32_t scProtocolVersion;

typedef struct {
    uint32_t channelId;
    uint32_t tokenId;
    int64_t  createdAt;
    uint32_t revisedLifetime;
} SC_SecurityToken;

typedef enum SC_ConnectionState
{
    SC_Connection_Connecting_Transport,
    SC_Connection_Connecting_Secure,
    SC_Connection_Connected,
    SC_Connection_Disconnected,
    SC_Connection_Error
} SC_ConnectionState;


typedef enum {
    SOPC_ConnectionEvent_Invalid = 0x00,
    SOPC_ConnectionEvent_Connected = 0x01,
//    SOPC_ConnectionEvent_Reconnecting, => not the case anymore ? mantis #3371
    SOPC_ConnectionEvent_Disconnected = 0x02,
    SOPC_ConnectionEvent_SecureMessageComplete = 0x03,
    SOPC_ConnectionEvent_SecureMessageChunk = 0x04,
    SOPC_ConnectionEvent_SecureMessageAbort = 0x05,
//    SOPC_ConnectionEvent_RefillSendQueue, => to be evaluated, socket behavior
    SOPC_ConnectionEvent_UnexpectedError = 0x06,
    SOPC_ConnectionEvent_ConnectionFailed
} SC_ConnectionEvent;


typedef struct {
    TCP_UA_Connection*        transportConnection;
    SC_ConnectionState        state;
    uint32_t                  startTime;
    SOPC_ByteString           runningAppCertificate;
    const Certificate*        runningAppPublicKeyCert; // Pointer on upper level param: do not manage allocation on it
    const AsymmetricKey*      runningAppPrivateKey; // Pointer on upper level param: do not manage allocation on it
    SOPC_ByteString           otherAppCertificate;
    const Certificate*        otherAppPublicKeyCert; // Pointer on upper level param: manage allocation on it only on server side (check connection flag)
    //SOPC_MsgBuffer*           sendingBuffer;
    uint32_t                  sendingMaxBodySize;
    SOPC_MsgBuffers*          receptionBuffers;
    OpcUa_MessageSecurityMode currentSecuMode;
    SOPC_String               currentSecuPolicy;
    SC_SecurityToken          currentSecuToken;
    SC_SecurityKeySets        currentSecuKeySets;
    CryptoProvider*           currentCryptoProvider;
    OpcUa_MessageSecurityMode precSecuMode;
    SOPC_String               precSecuPolicy;
    SC_SecurityToken          precSecuToken;
    SC_SecurityKeySets        precSecuKeySets;
    CryptoProvider*           precCryptoProvider;
    SecretBuffer*             currentNonce;
    uint32_t                  lastSeqNumSent;
    uint32_t                  lastSeqNumReceived;
    uint32_t                  lastRequestIdSent;
    uint32_t                  secureChannelId;
    /* Ensuring message sending atomicity on a connection */
    SOPC_ActionQueue*         msgQueue;
    uint8_t                   msgQueueToken; // Must be accessed by an action only (no concurrency): FALSE => not available / !=FALSE => available
} SC_Connection;

SC_Connection* SC_Create (TCP_UA_Connection* connection);
void SC_Disconnect(SC_Connection* scConnection);
void SC_Delete (SC_Connection* scConnection);

SOPC_StatusCode SC_InitApplicationIdentities(SC_Connection*       scConnection,
                                             uint8_t              noneSecurityMode,
                                             const Certificate*   runningAppCertificate,
                                             const AsymmetricKey* runningAppPrivateKey,
                                             const Certificate*   otherAppCertificate);

//Configure secure connection regarding the transport connection properties
SOPC_StatusCode SC_InitReceiveSecureBuffers(SC_Connection*        scConnection,
                                            SOPC_NamespaceTable*  namespaceTable,
                                            SOPC_EncodeableType** encodeableTypes);
SOPC_StatusCode SC_InitSendSecureBuffer(SC_Connection*        scConnection,
                                        SOPC_NamespaceTable*  namespaceTable,
                                        SOPC_EncodeableType** encodeableTypes);

SOPC_MsgBuffers* SC_CreateSendSecureBuffers(uint32_t              maxChunksSendingCfg,
                                            uint32_t              maxMsgSizeSendingCfg,
                                            uint32_t              bufferSizeSendingCfg,
                                            SC_Connection*        flushData,
                                            SOPC_NamespaceTable*  nsTableCfg,
                                            SOPC_EncodeableType** encTypesTableCfg);

SOPC_StatusCode SC_EncodeSecureMsgHeader(SOPC_MsgBuffers*       msgBuffers,
                                         SOPC_SecureMessageType smType,
                                         uint32_t               secureChannelId);

SOPC_StatusCode SC_EncodeAsymmSecurityHeader(SC_Connection* scConnection,
                                             SOPC_MsgBuffer* msgBuffer,
                                             SOPC_String*   securityPolicy);

SOPC_StatusCode SC_SetMaxBodySize(SC_Connection* scConnection,
                                  uint32_t       nonEncryptedHeadersSize,
                                  uint32_t       chunkSize,
                                  uint32_t       isSymmetric);

SOPC_StatusCode SC_EncodeSequenceHeader(SOPC_MsgBuffers* msgBuffers,
                                        uint32_t         requestId);

SOPC_StatusCode SC_WriteSecureMsgBuffer(SOPC_MsgBuffers* msgBuffers,
                                        const SOPC_Byte* data_src,
                                        uint32_t         count);

SOPC_StatusCode SC_FlushSecureMsgBuffers(SOPC_MsgBuffers*             msgBuffers,
                                         uint32_t                     flushChunkIdx,
                                         uint32_t                     tokenId,
                                         uint32_t                     requestId,
                                         SOPC_Socket_EndOperation_CB* fctPointer,
                                         void*                        fctArgument);

SOPC_StatusCode SC_IsPrecedentCryptoData(SC_Connection* scConnection,
                                         uint32_t       receivedTokenId,
                                         uint32_t*      isPrecCryptoData);

SOPC_StatusCode SC_DecodeSecureMsgSCid(SC_Connection* scConnection,
                                       SOPC_MsgBuffer*  transportBuffer);

SOPC_StatusCode SC_DecodeAsymSecurityHeader_SecurityPolicy(SC_Connection*  scConnection,
                                                           SOPC_MsgBuffer* transportBuffer,
                                                           SOPC_String*    securityPolicy);


SOPC_StatusCode SC_DecodeAsymSecurityHeader_Certificates(SC_Connection*     scConnection,
                                                         SOPC_MsgBuffer*    transportBuffer,
                                                         const PKIProvider* pkiProvider,
                                                         uint32_t           validateSenderCert,
                                                         uint8_t            enforceSecuMode,
                                                         uint32_t*          sequenceNumberPosition,
                                                         uint8_t*           senderCertificatePresence,
                                                         uint8_t*           receiverCertificatePresense);

SOPC_StatusCode SC_DecodeAsymmSecurityHeader(SC_Connection*     scConnection,
                                             const PKIProvider* pkiProvider,
                                             SOPC_MsgBuffer*    transportBuffer,
                                             uint32_t           validateSenderCert,
                                             uint32_t*          sequenceNumberPosition);

SOPC_StatusCode SC_DecryptMsg(SC_Connection*  scConnection,
                              SOPC_MsgBuffer* transportBuffer,
                              uint32_t        sequenceNumberPosition,
                              uint32_t        isSymmetric,
                              uint32_t        isPrecCryptoData);

SOPC_StatusCode SC_VerifyMsgSignature(SC_Connection* scConnection,
                                      uint32_t       isSymmetric,
                                      uint32_t       isPrecCryptoData);

SOPC_StatusCode SC_CheckSeqNumReceived(SC_Connection* scConnection);

SOPC_StatusCode SC_CheckReceivedProtocolVersion(SC_Connection* scConnection,
                                                uint32_t       scProtocolVersion);

// Caller must call SC_AbortMsg in case of encode secure message failure
// in order to at least reset buffer and send an abort message if necessary (chunks already sent)
SOPC_StatusCode SC_EncodeSecureMessage(SOPC_MsgBuffers*     msgBuffers,
                                       SOPC_EncodeableType* encType,
                                       void*                value,
                                       uint32_t             secureChannelId,
                                       uint32_t             tokenId,
                                       uint32_t             requestId);

SOPC_StatusCode SC_DecodeSymmSecurityHeader(SOPC_MsgBuffer* transportBuffer,
                                            uint32_t*       tokenId,
                                            uint32_t*       snPosition);

// Only for symmetric encrypted buffer
SOPC_StatusCode SC_RemovePaddingAndSig(SC_Connection* scConnection,
                                       uint32_t       isPrecCryptoData);

SOPC_StatusCode SC_DecryptSecureMessage(SC_Connection*  scConnection,
                                        SOPC_MsgBuffer* transportMsgBuffer,
                                        uint32_t*       requestId);

SOPC_StatusCode SC_CheckPrecChunk(SOPC_MsgBuffers* msgBuffer,
                                  uint32_t         requestId,
                                  uint8_t*         abortReqPresence,
                                  uint32_t*        abortReqId);

SOPC_StatusCode SC_CheckAbortChunk(SOPC_MsgBuffers* msgBuffer,
                                   SOPC_String*     reason);

// SC_CheckPrecChunk and SC_CheckAbortChunk to be called before calling decode chunk
// HYP: msgBuffers->isFinal = Intermediate or Final
// (otherwise could fail on abort chunk or unexpected request id)
SOPC_StatusCode SC_DecodeChunk(SOPC_MsgBuffers*      msgBuffers,
                               uint32_t              requestId,
                               SOPC_EncodeableType*  expEncType, // Should not be null for efficiency !
                               SOPC_EncodeableType*  errEncType,
                               SOPC_EncodeableType** recEncType,
                               void**                encObj);

// Added to treat messages to send through actions
void SC_Action_TreateMsgQueue(void* arg);

void SC_CreateAction_ReleaseToken(SC_Connection* scConnection);

#endif /* SOPC_SECURE_CHANNEL_LOW_LEVEL_H_ */
