/*
 * secure_channel_low_level.h
 *
 *  Created on: Jul 22, 2016
 *      Author: vincent
 */

#ifndef INGOPCS_SECURE_CHANNEL_LOW_LEVEL_H_
#define INGOPCS_SECURE_CHANNEL_LOW_LEVEL_H_

#include <wrappers.h>

#include <tcp_ua_connection.h>
#include <private_key.h>

typedef struct SC_Security_Token {
    uint32_t channelId;
    uint32_t tokenId;
    int64_t  createdAt;
    int32_t  revisedLifetime;
} SC_Security_Token;

typedef struct SC_Security_Key_Set{
    Private_Key* signKey;
    Private_Key* encryptKey;
    Private_Key* initVector;
} SC_Security_Key_Set;

typedef struct SC_Security_Key_Sets {
    SC_Security_Key_Set* senderKeySet;
    SC_Security_Key_Set* receiverKeySet;
} SC_Security_Key_Sets;

typedef enum SC_Connection_State
{
    SC_Connection_Connecting_Transport,
    SC_Connection_Connecting_Secure,
    SC_Connection_Connected,
    SC_Connection_Disconnecting,
    SC_Connection_Disconnected,
    SC_Connection_Error
} SC_Connection_State;

typedef enum Msg_Security_Mode
{
    Msg_Security_Mode_Invalid        = 0,
    Msg_Security_Mode_None           = 1,
    Msg_Security_Mode_Sign           = 2,
    Msg_Security_Mode_SignAndEncrypt = 3,
} Msg_Security_Mode;

typedef struct SecureChannel_Connection {
    TCP_UA_Connection*    transportConnection;
    SC_Connection_State   state;
    uint32_t              startTime;
    UA_Byte_String*       runningAppCertificate;
    UA_Byte_String*       runningAppPublicKey;
    Private_Key*          runningAppPrivateKey;
    UA_Byte_String*       otherAppCertificate;
    UA_Byte_String*       otherAppPublicKey;
    UA_Msg_Buffer*        sendingBuffer;
    uint32_t              sendingMaxBodySize;
    UA_Msg_Buffers*       receptionBuffers;
    Msg_Security_Mode     currentSecuMode;
    UA_String*            currentSecuPolicy;
    SC_Security_Token     currentSecuToken;
    SC_Security_Key_Sets  currentSecuKeySets;
    Crypto_Provider*      currentCryptoProvider;
    Msg_Security_Mode     precSecuMode;
    UA_String*            precSecuPolicy;
    SC_Security_Token     precSecuToken;
    SC_Security_Key_Sets  precSecuKeySets;
    Crypto_Provider*      precCryptoProvider;
    Private_Key*          currentNonce;
    uint32_t              lastSeqNumSent;
    uint32_t              lastSeqNumReceived;
    uint32_t              lastRequestIdSent;
    uint32_t              secureChannelId;

} SecureChannel_Connection;

SecureChannel_Connection* Create_Secure_Connection (void);
void Delete_Secure_Connection (SecureChannel_Connection* scConnection);

StatusCode Initiate_Applications_Identities(SecureChannel_Connection* scConnection,
                                            UA_Byte_String*           runningAppCertificate,
                                            Private_Key*              runningAppPrivateKey,
                                            UA_Byte_String*           otherAppCertificate);
//Configure secure connection regarding the transport connection properties
StatusCode Initiate_Receive_Secure_Buffers(SecureChannel_Connection* scConnection);
StatusCode Initiate_Send_Secure_Buffer(SecureChannel_Connection* scConnection);

StatusCode Encode_Secure_Message_Header(UA_Msg_Buffer*         msgBuffer,
                                        UA_Secure_Message_Type smType,
                                        uint32_t               secureChannelId);

StatusCode Encode_Asymmetric_Security_Header(SecureChannel_Connection* scConnection,
                                             UA_String*                securityPolicy,
                                             UA_Byte_String*           senderCertificate,
                                             UA_Byte_String*           receiverCertificate);

StatusCode Set_MaxBodySize(SecureChannel_Connection* scConnection,
                           uint32_t                  isAsymmetric);

StatusCode Encode_Sequence_Header(SecureChannel_Connection* scConnection,
                                  uint32_t*                 requestId);

StatusCode Write_Secure_Msg_Buffer(UA_Msg_Buffer* msgBuffer,
                                   const UA_Byte* data_src,
                                   uint32_t       count);

StatusCode Flush_Secure_Msg_Buffer(UA_Msg_Buffer*     msgBuffer,
                                   UA_Msg_Final_Chunk chunkType);

StatusCode Decode_Secure_Message_SecureChannelId(SecureChannel_Connection* scConnection,
                                                 UA_Msg_Buffer*            transportBuffer);

StatusCode Decode_Asymmetric_Security_Header(SecureChannel_Connection* scConnection,
                                             PKIProvider*              pkiProvider,
                                             UA_Msg_Buffer*            transportBuffer,
                                             uint32_t                  validateSenderCert);

StatusCode Decrypt_Message_Content(SecureChannel_Connection* scConnection,
                                   uint32_t                  useTokenId,
                                   uint32_t                  receivedTokenId,
                                   UA_Msg_Buffer*            transportBuffer,
                                   uint32_t                  isAsymmetric);

StatusCode Verify_Message_Signature(SecureChannel_Connection* scConnection,
                                    uint32_t                  isAsymmetric);

StatusCode Check_Sequence_Number_Received(SecureChannel_Connection* scConnection);



#endif /* INGOPCS_SECURE_CHANNEL_LOW_LEVEL_H_ */
