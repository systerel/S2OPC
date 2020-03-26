/* Copyright (C) Systerel SAS 2019, all rights reserved. */

/** \file
 *
 * Client's API, split client functionalities in smaller chunks.
 */

#ifndef CLIENT_H_
#define CLIENT_H_

#include "sopc_key_manager.h"
#include "sopc_sk_builder.h"
#include "sopc_sk_manager.h"

/* TODO multi key : ajouter une structure ( peut etre caché )
 qui contient le configuration et le numéro de session (utilisation interne) */

typedef struct Client_SKS_GetKeys_Response
{
    SOPC_String* SecurityPolicyUri;
    uint32_t FirstTokenId;
    SOPC_ByteString* Keys;
    uint32_t NbKeys;
    uint32_t TimeToNextKey;
    uint32_t KeyLifetime;
} Client_SKS_GetKeys_Response;

typedef struct Client_Keys_Type
{
    bool init;
    SOPC_ByteString SigningKey;
    SOPC_ByteString EncryptingKey;
    SOPC_ByteString KeyNonce;
} Client_Keys_Type;

extern Client_Keys_Type Client_Keys;

typedef enum
{
    SESSION_CONN_FAILED = -1,
    SESSION_CONN_CLOSED,
    SESSION_CONN_NEW,
    SESSION_CONN_CONNECTED,
    SESSION_CONN_MSG_RECEIVED,
} SessionConnectedState;

SOPC_ReturnStatus Client_Setup(void);

/*
 * \brief Add a new Secure Channel Configration to connect to a SKS server.
 *        Security Policy is Basic256Sha256
 * \param endpoint_url     Endpoint Url of the SKS Server
 * \param server_cert      A server certificate
 * \return                 A Secure Channel configuration ID or 0 if failed.
 */
uint32_t Client_AddSecureChannelconfig(const char* endpoint_url, SOPC_SerializedCertificate* server_cert);

/*
 * \brief Get Security Keys using a Security Keys Service.
 *  This function create a Session and send a Call Method request.
 *  It returns when the response is received or on timeout.
 *
 * \param SecureChannel_Id    Endpoint connection configuration index provided by Client_AddSecureChannelconfig()
 * \param StartingTokenId     Requested Staring Token ID
 * \param RequestedKeys       Number of requested Keys
 * \param response            A valid pointer of a Structure where received data will be copied
 */
SOPC_ReturnStatus Client_GetSecurityKeys(uint32_t SecureChannel_Id,
                                         uint32_t StartingTokenId,
                                         uint32_t requestedKeys,
                                         Client_SKS_GetKeys_Response* response);
void Client_Teardown(void);
void Client_KeysClear(void);

/*
 * To be call only by Toolkit callback when received response of Client_GetSecurityKeys() request
 */
void Client_Treat_Session_Response(void* param, uintptr_t appContext);

/*
 * \brief  Create an instance of SOPC_SKProvider to get Keys using SKS Get Security Keys request
 *
 * \param SecureChannel_Id    Endpoint connection configuration index provided by Client_AddSecureChannelconfig()
 * \return a SOPC_SKProvider object or NULL if not enough memory
 */
SOPC_SKProvider* Client_Provider_BySKS_Create(uint32_t SecureChannel_Id);

// session identifier
extern uint32_t session;
// use to identify the active session response
extern uintptr_t Client_SessionContext;
// Session state
extern SessionConnectedState scState;
// indicate that request send failed
extern int32_t sendFailures;

#endif // CLIENT_H_
