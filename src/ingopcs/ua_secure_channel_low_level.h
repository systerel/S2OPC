/*
 * secure_channel_low_level.h
 *
 *  Created on: Jul 22, 2016
 *      Author: vincent
 */

#ifndef INGOPCS_SECURE_CHANNEL_LOW_LEVEL_H_
#define INGOPCS_SECURE_CHANNEL_LOW_LEVEL_H_

#include <wrappers.h>

#include <private_key.h>
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

typedef struct {
    TCP_UA_Connection*     transportConnection;
    SC_ConnectionState     state;
    uint32_t               startTime;
    UA_ByteString*         runningAppCertificate;
    UA_ByteString*         runningAppPublicKey;
    PrivateKey*            runningAppPrivateKey;
    UA_ByteString*         otherAppCertificate;
    UA_ByteString*         otherAppPublicKey;
    UA_MsgBuffer*          sendingBuffer;
    uint32_t               sendingMaxBodySize;
    UA_MsgBuffers*         receptionBuffers;
    UA_MessageSecurityMode currentSecuMode;
    UA_String*             currentSecuPolicy;
    SC_SecurityToken       currentSecuToken;
    SC_SecurityKeySets     currentSecuKeySets;
    CryptoProvider*        currentCryptoProvider;
    UA_MessageSecurityMode precSecuMode;
    UA_String*             precSecuPolicy;
    SC_SecurityToken       precSecuToken;
    SC_SecurityKeySets     precSecuKeySets;
    CryptoProvider*        precCryptoProvider;
    PrivateKey*            currentNonce;
    uint32_t               lastSeqNumSent;
    uint32_t               lastSeqNumReceived;
    uint32_t               lastRequestIdSent;
    uint32_t               secureChannelId;

} SC_Connection;

SC_Connection* SC_Create (void);
void SC_Delete (SC_Connection* scConnection);

StatusCode SC_InitApplicationIdentities(SC_Connection* scConnection,
                                        UA_ByteString* runningAppCertificate,
                                        PrivateKey*    runningAppPrivateKey,
                                        UA_ByteString* otherAppCertificate);

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
                                        UA_String*     securityPolicy,
                                        UA_ByteString* senderCertificate,
                                        UA_ByteString* receiverCertificate);

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
                                        PKIProvider*   pkiProvider,
                                        UA_MsgBuffer*  transportBuffer,
                                        uint32_t       validateSenderCert,
                                        uint32_t*      sequenceNumberPosition);

StatusCode SC_DecryptMsg(SC_Connection* scConnection,
                         UA_MsgBuffer*  transportBuffer,
                         uint32_t       sequenceNumberPosition,
                         uint32_t       isSymmetric,
                         uint32_t       isPrecCryptoData);

StatusCode SC_DecodeMsgBody(SC_Connection*      scConnection,
                            UA_EncodeableType*  respEncType, // expected response type
                            UA_EncodeableType*  errEncType,  // expected response error type
                            UA_EncodeableType** receivedEncType, // actual received type (in those provided)
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

#endif /* INGOPCS_SECURE_CHANNEL_LOW_LEVEL_H_ */
