/* Copyright (C) Systerel SAS 2019, all rights reserved. */

/** \file
 *
 * Client's API, split client functionalities in smaller chunks.
 */

#ifndef CLIENT_H_
#define CLIENT_H_

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
SOPC_ReturnStatus Client_AddSecureChannelconfig(const char* endpoint_url);
SOPC_ReturnStatus Client_GetSecurityKeys(void);
void Client_Teardown(void);
void Client_KeysClear(void);
void Client_Treat_Session_Response(void* param);

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
