/*
 * wrapper_channel.c
 *
 *  Created on: Nov 17, 2016
 *      Author: vincent
 */

#include "wrapper_channel.h"
#include "sopc_secure_channel_client_connection.h"

#include <string.h>

#include "sopc_stack_csts.h"
#include "pki_stack.h"

#ifdef OPCUA_HAVE_CLIENTAPI

typedef enum
{
    TMP_Invalid_PKI   = 0,
    TMP_NO_PKI        = 1,
    TMP_Override      = 2,
    TMP_DefaultPKI    = 3
} PKI_Types;

typedef struct
{
    PKI_Types       type;
    char*           trustListLocation;
    char*           revocationListLocation;
    char*           untrustedListLocation;
    uint32_t        flags;
    void*           unused;
} PKIConfig;

SOPC_StatusCode OpcUa_Channel_Create(SOPC_Channel*               channel,
                                     SOPC_Channel_SerializerType serializerType)
{
    if(serializerType != SOPC_ChannelSerializer_Binary){
        return STATUS_NOK;
    }
    return SOPC_Channel_Create(channel,
                               SOPC_ChannelSerializer_Binary);
}

void OpcUa_Channel_Clear(SOPC_Channel channel)
{
    OpcUa_Channel_Disconnect(channel);
}

void OpcUa_Channel_Delete(SOPC_Channel* channel)
{
    if(channel != NULL){
        SC_ClientConnection* cConnection = (SC_ClientConnection*) *channel;
        if(cConnection->pkiProvider != NULL){
            KeyManager_Certificate_Free(cConnection->pkiProvider->pUserCertAuthList);
            PKIProviderStack_Free((PKIProvider*) cConnection->pkiProvider);
        }
        cConnection->pkiProvider = NULL;
        KeyManager_Certificate_Free((Certificate*) cConnection->clientCertificate);
        cConnection->clientCertificate = NULL;
        KeyManager_Certificate_Free((Certificate*) cConnection->serverCertificate);
        cConnection->serverCertificate = NULL;
        KeyManager_AsymmetricKey_Free((AsymmetricKey*) cConnection->clientKey);
        cConnection->clientKey = NULL;
        SOPC_Channel_Delete(channel);
    }
}

SOPC_StatusCode OpcUa_Channel_BeginInvokeService(SOPC_Channel                     channel,
                                                 char*                            name,
                                                 void*                            request,
                                                 SOPC_EncodeableType*             requestType,
                                                 SOPC_Channel_PfnRequestComplete* callback,
                                                 void*                            callbackData)
{
    return SOPC_Channel_BeginInvokeService(channel,
                                           name,
                                           request,
                                           requestType,
                                           NULL,
                                           callback,
                                           callbackData);
}

SOPC_StatusCode OpcUa_Channel_InvokeService(SOPC_Channel          channel,
                                            char*                 name,
                                            void*                 request,
                                            SOPC_EncodeableType*  requestType,
                                            void**                response,
                                            SOPC_EncodeableType** responseType)
{
    return SOPC_Channel_InvokeService(channel,
                                      name,
                                      request,
                                      requestType,
                                      NULL,
                                      response,
                                      responseType);
}

SOPC_StatusCode OpcUa_Channel_BeginDisconnect(SOPC_Channel                            channel,
                                              SOPC_Channel_PfnConnectionStateChanged* callback,
                                              void*                                   callbackData)
{
    (void) channel;
    (void) callback;
    (void) callbackData;
    return STATUS_NOK;
}

SOPC_StatusCode OpcUa_Channel_Disconnect(SOPC_Channel channel)
{
    SOPC_StatusCode status = STATUS_OK;
    SC_ClientConnection* cConnection = (SC_ClientConnection*) channel;
    if(cConnection->pkiProvider != NULL){
        KeyManager_Certificate_Free(cConnection->pkiProvider->pUserCertAuthList);
        PKIProviderStack_Free((PKIProvider*) cConnection->pkiProvider);
    }
    cConnection->pkiProvider = NULL;
    KeyManager_Certificate_Free((Certificate*) cConnection->clientCertificate);
    cConnection->clientCertificate = NULL;
    KeyManager_Certificate_Free((Certificate*) cConnection->serverCertificate);
    cConnection->serverCertificate = NULL;
    KeyManager_AsymmetricKey_Free((AsymmetricKey*) cConnection->clientKey);
    cConnection->clientKey = NULL;
    status = SOPC_Channel_Disconnect(channel);
    return status;
}

SOPC_StatusCode OpcUa_Channel_BeginConnect(SOPC_Channel                            channel,
                                           char*                                   url,
                                           SOPC_ByteString*                        clientCertificate,
                                           SOPC_ByteString*                        clientPrivateKey,
                                           SOPC_ByteString*                        serverCertificate,
                                           void*                                   pkiConfig,
                                           SOPC_String*                            requestedSecurityPolicyUri,
                                           int32_t                                 requestedLifetime,
                                           OpcUa_MessageSecurityMode               messageSecurityMode,
                                           uint32_t                                networkTimeout,
                                           SOPC_Channel_PfnConnectionStateChanged* callback,
                                           void*                                   callbackData)
{
    SOPC_StatusCode status = STATUS_OK;
    Certificate *cli = NULL, *srv = NULL, *crt_ca = NULL;
    AsymmetricKey *pKeyCli = NULL;
    PKIProvider *pki;
    PKIConfig *pPKIConfig = pkiConfig;
    if(clientCertificate != NULL && clientPrivateKey != NULL &&
       serverCertificate != NULL && pkiConfig != NULL){
        status = KeyManager_Certificate_CreateFromDER(clientCertificate->Data,
                                                      clientCertificate->Length,
                                                      &cli);
        if(STATUS_OK == status){
            status = KeyManager_AsymmetricKey_CreateFromBuffer(clientPrivateKey->Data, clientPrivateKey->Length, &pKeyCli);
        }
        if(STATUS_OK == status){
            status = KeyManager_Certificate_CreateFromDER(serverCertificate->Data,
                                                          serverCertificate->Length,
                                                          &srv);
        }
        //TODO: CA folder != CA cert: how to deal with that ?
        if(STATUS_OK == status){
            const char* cacertname = "/cacert.der";
            char cacert[strlen(pPKIConfig->trustListLocation) + strlen(cacertname) + 1];
            if(cacert != memcpy(cacert, pPKIConfig->trustListLocation, strlen(pPKIConfig->trustListLocation)))
                status = STATUS_NOK;
            if(&cacert[strlen(pPKIConfig->trustListLocation)] !=
                memcpy(&cacert[strlen(pPKIConfig->trustListLocation)], cacertname, strlen(cacertname) + 1))
                status = STATUS_NOK;
            if(STATUS_OK == status){
                status = KeyManager_Certificate_CreateFromFile(cacert, &crt_ca);
            }
        }
        if(STATUS_OK == status){
            status = PKIProviderStack_Create(crt_ca, NULL, &pki);
        }
    }

    if(STATUS_OK == status){
        status = SOPC_Channel_BeginConnect(channel,
                                           url,
                                           cli,
                                           pKeyCli,
                                           srv,
                                           pki,
                                           SOPC_String_GetRawCString(requestedSecurityPolicyUri),
                                           requestedLifetime,
                                           messageSecurityMode,
                                           networkTimeout,
                                           callback,
                                           callbackData);
    }
    if(STATUS_OK != status){
        if(pki != NULL){
            KeyManager_Certificate_Free(pki->pUserCertAuthList);
            PKIProviderStack_Free(pki);
        }
        KeyManager_Certificate_Free(cli);
        KeyManager_Certificate_Free(srv);
        KeyManager_AsymmetricKey_Free(pKeyCli);
    }
    return status;
}

SOPC_StatusCode OpcUa_Channel_Connect(SOPC_Channel                            channel,
                                      char*                                   url,
                                      SOPC_Channel_PfnConnectionStateChanged* callback,
                                      void*                                   callbackData,
                                      SOPC_ByteString*                        clientCertificate,
                                      SOPC_ByteString*                        clientPrivateKey,
                                      SOPC_ByteString*                        serverCertificate,
                                      void*                                   pkiConfig,
                                      SOPC_String*                            requestedSecurityPolicyUri,
                                      int32_t                                 requestedLifetime,
                                      OpcUa_MessageSecurityMode               messageSecurityMode,
                                      void*                                   securityToken,
                                      uint32_t                                networkTimeout)
{
    SOPC_StatusCode status = STATUS_OK;
    Certificate *cli = NULL, *srv = NULL, *crt_ca = NULL;
    AsymmetricKey *pKeyCli = NULL;
    PKIProvider *pki = NULL;
    PKIConfig *pPKIConfig = pkiConfig;
    if(clientCertificate != NULL && clientCertificate->Length > 0 &&
       clientPrivateKey != NULL && clientPrivateKey->Length > 0 &&
       serverCertificate != NULL && serverCertificate->Length > 0 &&
       pkiConfig != NULL){
        status = KeyManager_Certificate_CreateFromDER(clientCertificate->Data,
                                                      clientCertificate->Length,
                                                      &cli);
        if(STATUS_OK == status){
            status = KeyManager_AsymmetricKey_CreateFromBuffer(clientPrivateKey->Data, clientPrivateKey->Length, &pKeyCli);
        }
        if(STATUS_OK == status){
            status = KeyManager_Certificate_CreateFromDER(serverCertificate->Data,
                                                          serverCertificate->Length,
                                                          &srv);
        }
        //TODO: CA folder != CA cert: how to deal with that ?
        if(STATUS_OK == status){
            const char* cacertname = "/cacert.der";
            char cacert[strlen(pPKIConfig->trustListLocation) + strlen(cacertname) + 1];
            if(cacert != memcpy(cacert, pPKIConfig->trustListLocation, strlen(pPKIConfig->trustListLocation)))
                status = STATUS_NOK;
            if(&cacert[strlen(pPKIConfig->trustListLocation)] !=
                memcpy(&cacert[strlen(pPKIConfig->trustListLocation)], cacertname, strlen(cacertname) + 1))
                status = STATUS_NOK;
            if(STATUS_OK == status){
                status = KeyManager_Certificate_CreateFromFile(cacert, &crt_ca);
            }
        }
        if(STATUS_OK == status){
            status = PKIProviderStack_Create(crt_ca, NULL, &pki);
        }
    }

    if(STATUS_OK == status){
        status = SOPC_Channel_Connect(channel,
                                      url,
                                      cli,
                                      pKeyCli,
                                      srv,
                                      pki,
                                      SOPC_String_GetRawCString(requestedSecurityPolicyUri),
                                      requestedLifetime,
                                      messageSecurityMode,
                                      networkTimeout,
                                      callback,
                                      callbackData);
    }
    if(STATUS_OK != status){
        if(pki != NULL){
            KeyManager_Certificate_Free(pki->pUserCertAuthList);
            PKIProviderStack_Free(pki);
        }
        KeyManager_Certificate_Free(cli);
        KeyManager_Certificate_Free(srv);
        KeyManager_AsymmetricKey_Free(pKeyCli);
    }
    return status;
}

#endif /* OPCUA_HAVE_CLIENTAPI */
