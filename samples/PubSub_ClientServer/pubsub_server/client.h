/* Copyright (C) Systerel SAS 2019, all rights reserved. */

/** \file
 *
 * Client's API, split client functionalities in smaller chunks.
 */

#ifndef CLIENT_H_
#define CLIENT_H_

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
SOPC_ReturnStatus Client_AddSecureChannelconfig(const char* endpoint_url, const char* server_cert_path);
SOPC_ReturnStatus Client_GetSecurityKeys(uint32_t StartingTokenId,
                                         uint32_t requestedKeys,
                                         Client_SKS_GetKeys_Response* response);
void Client_Teardown(void);
void Client_KeysClear(void);
void Client_Treat_Session_Response(void* param, uintptr_t appContext);

/*
 * \brief Create a Security Keys Provider to get Keys using SKS Get Security Keys request
 * TODO SKS : For multi SKS : give a SCConfig as parameter
 */
SOPC_SKProvider* Client_Provider_BySKS_Create(void);

/*
 * \brief Create a Security Keys Builder to set Keys of a SKManager after a get security keys request
 *  Update function calls SOPC_SKManager_SetKeys() on the given SKManager.
 * TODO SKS : For multi SKS : give a SCConfig as parameter
 */
SOPC_SKBuilder* Client_Create_Builder(void);

// channel identifier
extern uint32_t channel_config_idx;
// session identifier
extern uint32_t session;
// use to identify the active session response
extern uintptr_t Client_SessionContext;
// Session state
extern SessionConnectedState scState;
// indicate that request send failed
extern int32_t sendFailures;

#endif // CLIENT_H_
