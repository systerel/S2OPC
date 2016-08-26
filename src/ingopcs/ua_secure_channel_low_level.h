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
#include "ua_tcp_ua_connection.h"

extern const uint32_t scProtocolVersion;

typedef struct SC_SecurityToken {
    uint32_t channelId;
    uint32_t tokenId;
    int64_t  createdAt;
    int32_t  revisedLifetime;
} SC_SecurityToken;

typedef struct SC_SecurityKeySet{
    PrivateKey* signKey;
    PrivateKey* encryptKey;
    PrivateKey* initVector;
} SC_SecurityKeySet;

typedef struct SC_SecurityKeySets {
    SC_SecurityKeySet* senderKeySet;
    SC_SecurityKeySet* receiverKeySet;
} SC_SecurityKeySets;

typedef enum SC_ConnectionState
{
    SC_Connection_Connecting_Transport,
    SC_Connection_Connecting_Secure,
    SC_Connection_Connected,
    SC_Connection_Disconnecting,
    SC_Connection_Disconnected,
    SC_Connection_Error
} SC_ConnectionState;

typedef enum MsgSecurityMode
{
    Msg_Security_Mode_Invalid        = 0,
    Msg_Security_Mode_None           = 1,
    Msg_Security_Mode_Sign           = 2,
    Msg_Security_Mode_SignAndEncrypt = 3,
} MsgSecurityMode;

typedef struct SC_Connection {
    TCP_UA_Connection* transportConnection;
    SC_ConnectionState state;
    uint32_t           startTime;
    UA_ByteString*     runningAppCertificate;
    UA_ByteString*     runningAppPublicKey;
    PrivateKey*        runningAppPrivateKey;
    UA_ByteString*     otherAppCertificate;
    UA_ByteString*     otherAppPublicKey;
    UA_MsgBuffer*      sendingBuffer;
    uint32_t           sendingMaxBodySize;
    UA_MsgBuffers*     receptionBuffers;
    MsgSecurityMode    currentSecuMode;
    UA_String*         currentSecuPolicy;
    SC_SecurityToken   currentSecuToken;
    SC_SecurityKeySets currentSecuKeySets;
    CryptoProvider*    currentCryptoProvider;
    MsgSecurityMode    precSecuMode;
    UA_String*         precSecuPolicy;
    SC_SecurityToken   precSecuToken;
    SC_SecurityKeySets precSecuKeySets;
    CryptoProvider*    precCryptoProvider;
    PrivateKey*        currentNonce;
    uint32_t           lastSeqNumSent;
    uint32_t           lastSeqNumReceived;
    uint32_t           lastRequestIdSent;
    uint32_t           secureChannelId;

} SC_Connection;

SC_Connection* SC_Create (void);
void SC_Delete (SC_Connection* scConnection);

StatusCode SC_InitApplicationIdentities(SC_Connection* scConnection,
                                        UA_ByteString* runningAppCertificate,
                                        PrivateKey*    runningAppPrivateKey,
                                        UA_ByteString* otherAppCertificate);

//Configure secure connection regarding the transport connection properties
StatusCode SC_InitReceiveSecureBuffers(SC_Connection* scConnection);
StatusCode SC_InitSendSecureBuffer(SC_Connection* scConnection);

StatusCode SC_EncodeSecureMsgHeader(UA_MsgBuffer*        msgBuffer,
                                    UA_SecureMessageType smType,
                                    uint32_t             secureChannelId);

StatusCode SC_EncodeAsymmSecurityHeader(SC_Connection* scConnection,
                                        UA_String*     securityPolicy,
                                        UA_ByteString* senderCertificate,
                                        UA_ByteString* receiverCertificate);

StatusCode SC_SetMaxBodySize(SC_Connection* scConnection,
                             uint32_t       isSymmetric);

StatusCode SC_EncodeSequenceHeader(SC_Connection* scConnection,
                                   uint32_t*      requestId);

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

StatusCode SC_VerifyMsgSignature(SC_Connection* scConnection,
                                 uint32_t       isSymmetric,
                                 uint32_t       isPrecCryptoData);

StatusCode SC_CheckSeqNumReceived(SC_Connection* scConnection);



#endif /* INGOPCS_SECURE_CHANNEL_LOW_LEVEL_H_ */
