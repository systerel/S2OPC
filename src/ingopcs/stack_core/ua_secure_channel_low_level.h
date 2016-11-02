/*
 * secure_channel_low_level.h
 *
 *  Created on: Jul 22, 2016
 *      Author: vincent
 */

#ifndef INGOPCS_SECURE_CHANNEL_LOW_LEVEL_H_
#define INGOPCS_SECURE_CHANNEL_LOW_LEVEL_H_

#include <wrappers.h>

#include <key_sets.h>
#include <ua_types.h>
#include <ua_tcp_ua_connection.h>

extern const uint32_t scProtocolVersion;

typedef struct {
    uint32_t channelId;
    uint32_t tokenId;
    int64_t  createdAt;
    int32_t  revisedLifetime;
} SC_SecurityToken;

typedef enum SC_ConnectionState
{
    SC_Connection_Connecting_Transport,
    SC_Connection_Connecting_Secure,
    SC_Connection_Connected,
    SC_Connection_Disconnecting,
    SC_Connection_Disconnected,
    SC_Connection_Error
} SC_ConnectionState;


typedef enum {
    UA_ConnectionEvent_Invalid = 0x00,
    UA_ConnectionEvent_Connected = 0x01,
//    UA_ConnectionEvent_Reconnecting, => not the case anymore ? mantis #3371
    UA_ConnectionEvent_Disconnected = 0x02,
    UA_ConnectionEvent_SecureMessageComplete = 0x03,
    UA_ConnectionEvent_SecureMessageChunk = 0x04,
    UA_ConnectionEvent_SecureMessageAbort = 0x05,
//    UA_ConnectionEvent_RefillSendQueue, => to be evaluated, socket behavior
    UA_ConnectionEvent_UnexpectedError = 0x06
} SC_ConnectionEvent;


typedef struct {
    TCP_UA_Connection*     transportConnection;
    SC_ConnectionState     state;
    uint32_t               startTime;
    UA_ByteString          runningAppCertificate;
    const Certificate*     runningAppPublicKeyCert;
    const AsymmetricKey*   runningAppPrivateKey; // Pointer on running app private key: do not manage allocation on it
    UA_ByteString          otherAppCertificate;
    const Certificate*     otherAppPublicKeyCert;
    UA_MsgBuffer*          sendingBuffer;
    uint32_t               sendingMaxBodySize;
    UA_MsgBuffers*         receptionBuffers;
    UA_MessageSecurityMode currentSecuMode;
    UA_String              currentSecuPolicy;
    SC_SecurityToken       currentSecuToken;
    SC_SecurityKeySets     currentSecuKeySets;
    CryptoProvider*        currentCryptoProvider;
    KeyManager*            currentKeyManager;
    UA_MessageSecurityMode precSecuMode;
    UA_String              precSecuPolicy;
    SC_SecurityToken       precSecuToken;
    SC_SecurityKeySets     precSecuKeySets;
    CryptoProvider*        precCryptoProvider;
    KeyManager*            precKeyManager;
    SecretBuffer*          currentNonce;
    uint32_t               lastSeqNumSent;
    uint32_t               lastSeqNumReceived;
    uint32_t               lastRequestIdSent;
    uint32_t               secureChannelId;

} SC_Connection;

SC_Connection* SC_Create (void);
void SC_Delete (SC_Connection* scConnection);

StatusCode SC_InitApplicationIdentities(SC_Connection*       scConnection,
                                        const Certificate*   runningAppCertificate,
                                        const AsymmetricKey* runningAppPrivateKey,
                                        const Certificate*   otherAppCertificate);

//Configure secure connection regarding the transport connection properties
StatusCode SC_InitReceiveSecureBuffers(SC_Connection* scConnection,
                                       UA_NamespaceTable*  namespaceTable,
                                       UA_EncodeableType** encodeableTypes);
StatusCode SC_InitSendSecureBuffer(SC_Connection* scConnection,
                                   UA_NamespaceTable*  namespaceTable,
                                   UA_EncodeableType** encodeableTypes);

StatusCode SC_EncodeSecureMsgHeader(UA_MsgBuffer*        msgBuffer,
                                    UA_SecureMessageType smType,
                                    uint32_t             secureChannelId);

StatusCode SC_EncodeAsymmSecurityHeader(SC_Connection* scConnection,
                                        UA_String*     securityPolicy);

StatusCode SC_SetMaxBodySize(SC_Connection* scConnection,
                             uint32_t       isSymmetric);

StatusCode SC_EncodeSequenceHeader(UA_MsgBuffer* msgBuffer,
                                   uint32_t      requestId);

StatusCode SC_EncodeMsgBody(UA_MsgBuffer*      msgBuffer,
                            UA_EncodeableType* encType,
                            void*              msgBody);

StatusCode SC_WriteSecureMsgBuffer(UA_MsgBuffer*  msgBuffer,
                                   const UA_Byte* data_src,
                                   uint32_t       count);

StatusCode SC_FlushSecureMsgBuffer(UA_MsgBuffer*    msgBuffer,
                                   UA_MsgFinalChunk chunkType);

StatusCode SC_IsPrecedentCryptoData(SC_Connection* scConnection,
                                    uint32_t       receivedTokenId,
                                    uint32_t*      isPrecCryptoData);

StatusCode SC_DecodeSecureMsgSCid(SC_Connection* scConnection,
                                  UA_MsgBuffer*  transportBuffer);

StatusCode SC_DecodeAsymmSecurityHeader(SC_Connection* scConnection,
                                        const PKIProvider*   pkiProvider,
                                        UA_MsgBuffer*  transportBuffer,
                                        uint32_t       validateSenderCert,
                                        uint32_t*      sequenceNumberPosition);

StatusCode SC_DecryptMsg(SC_Connection* scConnection,
                         UA_MsgBuffer*  transportBuffer,
                         uint32_t       sequenceNumberPosition,
                         uint32_t       isSymmetric,
                         uint32_t       isPrecCryptoData);

StatusCode SC_DecodeMsgBody(UA_MsgBuffer*       receptionBuffer,
                            UA_NamespaceTable*  namespaceTable,
                            UA_EncodeableType** knownTypes, // only in case next 2 types not provided
                            UA_EncodeableType*  respEncType, // expected type
                            UA_EncodeableType*  errEncType,  // + expected error type (or both null if unknown)
                            UA_EncodeableType** receivedEncType, // actually received type
                            void**              encodeableObj);

StatusCode SC_VerifyMsgSignature(SC_Connection* scConnection,
                                 uint32_t       isSymmetric,
                                 uint32_t       isPrecCryptoData);

StatusCode SC_CheckSeqNumReceived(SC_Connection* scConnection);

StatusCode SC_CheckReceivedProtocolVersion(SC_Connection* scConnection,
                                           uint32_t       scProtocolVersion);

StatusCode SC_EncodeSecureMessage(SC_Connection*     scConnection,
                                  UA_EncodeableType* encType,
                                  void*              value,
                                  uint32_t           requestId);

StatusCode SC_DecodeSymmSecurityHeader(UA_MsgBuffer* transportBuffer,
                                       uint32_t*     tokenId,
                                       uint32_t*     snPosition);

// Only for symmetric encrypted buffer
StatusCode SC_RemovePaddingAndSig(SC_Connection* scConnection,
                                  uint32_t       isPrecCryptoData);

StatusCode SC_DecryptSecureMessage(SC_Connection* scConnection,
                                   UA_MsgBuffer*  transportMsgBuffer,
                                   uint32_t*      requestId);

StatusCode SC_CheckPrecChunk(UA_MsgBuffers* msgBuffer,
                             uint32_t       requestId,
                             uint8_t*       abortReqPresence,
                             uint32_t*      abortReqId);

StatusCode SC_CheckAbortChunk(UA_MsgBuffers* msgBuffer,
                              UA_String*     reason);

// SC_CheckPrecChunk and SC_CheckAbortChunk to be called before calling decode chunk
// HYP: msgBuffers->isFinal = Intermediate or Final
// (otherwise could fail on abort chunk or unexpected request id)
StatusCode SC_DecodeChunk(UA_MsgBuffers*      msgBuffers,
                          uint32_t            requestId,
                          UA_EncodeableType*  expEncType, // Should not be null for efficiency !
                          UA_EncodeableType*  errEncType,
                          UA_EncodeableType** recEncType,
                          void**              encObj);

#endif /* INGOPCS_SECURE_CHANNEL_LOW_LEVEL_H_ */
