/*
 * secure_channel_low_level.h
 *
 *  Created on: Jul 22, 2016
 *      Author: vincent
 */

#ifndef INGOPCS_SECURE_CHANNEL_LOW_LEVEL_H_
#define INGOPCS_SECURE_CHANNEL_LOW_LEVEL_H_

#include <tcp_ua_connection.h>
#include <private_key.h>

typedef OpcUa_CryptoProvider Crypto_Provider;

typedef struct SC_Security_Token {
    uint32_t channelId;
    uint32_t tokenId;
    int64_t  createdAt;
    int32_t  revisedLifetime;
} SC_Security_Token;

typedef struct SC_Security_Key_Sets {
    Private_Key* senderKeySet;
    Private_Key* receiverKeySet;
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
    Msg_Security_Mode     currentSecuMode;
    UA_String*            currentSecuPolicy;
    SC_Security_Token     currentSecuToken;
    SC_Security_Key_Sets  currentSecuKeySets;
    Crypto_Provider*      currentCryptoProvider;
    Msg_Security_Mode     precSecuMode;
    UA_String             precSecuPolicy;
    SC_Security_Token     precSecuToken;
    SC_Security_Key_Sets  precSecuKeySets;
    Crypto_Provider*      precCryptoProvider;
    Private_Key*          currentNonce;
    uint32_t              lastSeqNumSent;
    uint32_t              lastSeqNumReceived;

} SecureChannel_Connection;

SecureChannel_Connection* Create_Secure_Connection (void);
void Delete_Secure_Connection (SecureChannel_Connection* scConnection);

#endif /* INGOPCS_SECURE_CHANNEL_LOW_LEVEL_H_ */
