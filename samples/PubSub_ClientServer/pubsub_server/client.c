/* Copyright (C) Systerel SAS 2019, all rights reserved. */

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"

#include "sopc_askpass.h"
#include "sopc_assert.h"
#include "sopc_atomic.h"
#include "sopc_crypto_profiles.h"
#include "sopc_encodeable.h"
#include "sopc_helper_string.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_pki_stack.h"
#include "sopc_time.h"
#include "sopc_toolkit_async_api.h"
#include "sopc_toolkit_config.h"
#include "sopc_toolkit_config_constants.h"
#include "sopc_types.h"

#include "client.h"
#include "config.h"

#ifdef WITH_STATIC_SECURITY_DATA
#include "static_security_data.h"
#endif

const char* SESSION_NAME = "S2OPC_SKS_client_session";

#define CLIENT_SKS_USER_POLICY_ID "username_Basic256Sha256"
#define CLIENT_SKS_USERNAME "user1"
#define CLIENT_SKS_PASSWORD "password"

#define CLIENT_SECURITY_GROUPID "sgid_1"
#define CLIENT_STARTING_TOKENID 0
#define CLIENT_REQUESTED_KEY_COUNT 5

#define CLIENT_TIMEOUT_ACTIVATE_SESSION 10000
#define CLIENT_TIMEOUT_CALL_REQUEST 10000

/* Length of token of keys (sign, encrypt, nonce) */
#define KEYS_TOKEN_LENGTH (32 + 32 + 4)

/* Should be true to use functionnality */
static int32_t g_Client_Started = 0;

uint32_t g_session = 0;
int32_t g_scState = (int32_t) SESSION_CONN_NEW;
// use to identify the active session response
uintptr_t g_Client_SessionContext = 1;

/* Secure Channel Configurations */
// Current and last secure channel registered
static int64_t g_Client_SecureChannel_Current = -1;
// channel configuration
static SOPC_SecureChannel_Config g_Client_SecureChannel_Config[SOPC_MAX_SECURE_CONNECTIONS];
// channel identifier
static SOPC_SecureChannelConfigIdx g_Client_SecureChannel_Id[SOPC_MAX_SECURE_CONNECTIONS];

int32_t g_sendFailures = 0;

static bool SOPC_TestHelper_AskPass_FromEnv(char** outPassword)
{
    SOPC_ASSERT(NULL != outPassword);
    /*
        We have to make a copy here because in any case, we will free the password and not distinguish if it come
        from environement or terminal after calling ::SOPC_KeyManager_SerializedAsymmetricKey_CreateFromFile_WithPwd
    */
    char* _outPassword = getenv(PASSWORD_ENV_NAME);
    *outPassword = SOPC_strdup(_outPassword); // Do a copy
    if (NULL == *outPassword)
    {
        printf("INFO: %s environment variable not set or empty, use terminal interactive input:\n", PASSWORD_ENV_NAME);
        return SOPC_AskPass_CustomPromptFromTerminal("Client private key password:\n", outPassword);
    }
    return true;
}

// Configure the 2 secure channel connections to use and retrieve channel configuration index
static SOPC_ReturnStatus CerAndKeyLoader_client(const char* client_key_path,
                                                const char* client_cert_path,
                                                SOPC_SerializedCertificate* sks_server_cert)
{
    SOPC_SerializedCertificate* client_cert = NULL;
    SOPC_SerializedAsymmetricKey* client_key = NULL;
    SOPC_SerializedCertificate* server_cert = NULL;
    SOPC_PKIProvider* pkiProvider = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    status = SOPC_KeyManager_SerializedCertificate_CreateFromFile(client_cert_path, &client_cert);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Client failed to load client certificate");
    }
    else
    {
        g_Client_SecureChannel_Config[g_Client_SecureChannel_Current].crt_cli = client_cert;
    }

    if (SOPC_STATUS_OK == status)
    {
        char* password = NULL;
        size_t lenPassword = 0;

        if (SOPC_STATUS_OK == status && ENCRYPTED_CLIENT_KEY)
        {
            bool res = SOPC_TestHelper_AskPass_FromEnv(&password);
            status = res ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
        }

        if (SOPC_STATUS_OK == status)
        {
            lenPassword = strlen(password);
            if (UINT32_MAX < lenPassword)
            {
                status = SOPC_STATUS_NOK;
            }
        }

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_KeyManager_SerializedAsymmetricKey_CreateFromFile_WithPwd(client_key_path, &client_key,
                                                                                    password, (uint32_t) lenPassword);
        }

        if (NULL != password)
        {
            SOPC_Free(password);
        }

        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Client failed to load private key");
        }
        else
        {
            g_Client_SecureChannel_Config[g_Client_SecureChannel_Current].key_priv_cli = client_key;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        server_cert = SOPC_Buffer_Create(sks_server_cert->length);
        if (NULL == server_cert)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
        else
        {
            status = SOPC_Buffer_Copy(server_cert, sks_server_cert);
        }
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Client failed to load server certificate");
        }
        else
        {
            g_Client_SecureChannel_Config[g_Client_SecureChannel_Current].crt_srv = server_cert;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        char* lPathsTrustedRoots[] = {CA_CERT_PATH, NULL};
        char* lPathsTrustedLinks[] = {NULL};
        char* lPathsUntrustedRoots[] = {NULL};
        char* lPathsUntrustedLinks[] = {NULL};
        char* lPathsIssuedCerts[] = {NULL};
        char* lPathsCRL[] = {CA_CRL_PATH, NULL};
        status =
            SOPC_PKIProviderStack_CreateFromPaths(lPathsTrustedRoots, lPathsTrustedLinks, lPathsUntrustedRoots,
                                                  lPathsUntrustedLinks, lPathsIssuedCerts, lPathsCRL, &pkiProvider);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Failed to create PKI");
        }
        else
        {
            g_Client_SecureChannel_Config[g_Client_SecureChannel_Current].pki = pkiProvider;
        }
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB,
                               "Client failed loading certificates and key (check paths are valid)");
    }
    return status;
}

void Client_Start(void)
{
    SOPC_Atomic_Int_Set(&g_Client_Started, true);
}

/*
 * \brief Retrieve a the Secure Channel Configuration associated to the given endpoint url
 *        The Secure Channel Configuration should have been previously created using Client_AddSecureChannelConfig()
 *
 * \param endpoint_url     Endpoint Url of the SKS Server
 * \return                 A Secure Channel configuration ID or 0 if failed.
 */
static SOPC_SecureChannelConfigIdx Client_GetSecureChannelConfig(const char* endpoint_url)
{
    for (uint32_t i = 0; i <= g_Client_SecureChannel_Current; i++)
    {
        if (0 == strcmp(g_Client_SecureChannel_Config[g_Client_SecureChannel_Current].url, endpoint_url))
        {
            return g_Client_SecureChannel_Id[g_Client_SecureChannel_Current];
        }
    }
    return 0;
}

SOPC_SecureChannelConfigIdx Client_AddSecureChannelConfig(const char* endpoint_url,
                                                          SOPC_SerializedCertificate* server_cert)
{
    SOPC_ASSERT(NULL != endpoint_url);
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    SOPC_SecureChannelConfigIdx scId = Client_GetSecureChannelConfig(endpoint_url);

    if (scId > 0)
    {
        /* We return same secure channel configuration if endpoint already used for an SC config */
        return scId;
    }

    if (SOPC_MAX_SECURE_CONNECTIONS <= g_Client_SecureChannel_Current + 1)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Too many secure channel created");
        return 0;
    }
    g_Client_SecureChannel_Current++;

    // A Secure channel connection configuration
    g_Client_SecureChannel_Config[g_Client_SecureChannel_Current].isClientSc = true;
    g_Client_SecureChannel_Config[g_Client_SecureChannel_Current].crt_cli = NULL;
    g_Client_SecureChannel_Config[g_Client_SecureChannel_Current].key_priv_cli = NULL;
    g_Client_SecureChannel_Config[g_Client_SecureChannel_Current].crt_srv = NULL;
    g_Client_SecureChannel_Config[g_Client_SecureChannel_Current].pki = NULL;
    g_Client_SecureChannel_Config[g_Client_SecureChannel_Current].reqSecuPolicyUri =
        SOPC_SecurityPolicy_Basic256Sha256_URI;
    g_Client_SecureChannel_Config[g_Client_SecureChannel_Current].requestedLifetime = 20000;
    g_Client_SecureChannel_Config[g_Client_SecureChannel_Current].msgSecurityMode =
        OpcUa_MessageSecurityMode_SignAndEncrypt;
    // Copy Endpoint URL
    char* endpoint_url_copy = SOPC_Calloc(strlen(endpoint_url) + 1, sizeof(char));
    g_Client_SecureChannel_Config[g_Client_SecureChannel_Current].url = endpoint_url_copy;
    if (NULL == endpoint_url_copy)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }
    else
    {
        strcpy(endpoint_url_copy, endpoint_url);
    }

    if (SOPC_STATUS_OK == status)
    {
        status = CerAndKeyLoader_client(CLIENT_KEY_PATH, CLIENT_CERT_PATH, server_cert);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "FAILED on configuring Certificate, key and SC");
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        g_Client_SecureChannel_Id[g_Client_SecureChannel_Current] =
            SOPC_ToolkitClient_AddSecureChannelConfig(&(g_Client_SecureChannel_Config[g_Client_SecureChannel_Current]));
        if (0 == g_Client_SecureChannel_Id[g_Client_SecureChannel_Current])
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Failed to configure the secure channel connections");
            status = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK != status)
    {
        return 0;
    }
    else
    {
        return g_Client_SecureChannel_Id[g_Client_SecureChannel_Current];
    }
}

static SOPC_ReturnStatus Wait_response_client(void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    // Sleep timeout in milliseconds
    const uint32_t sleepTimeout = 20;
    // Loop timeout in milliseconds
    const uint32_t loopTimeout = CLIENT_TIMEOUT_CALL_REQUEST;
    // Counter to stop waiting on timeout
    uint32_t loopCpt = 0;

    while (SOPC_STATUS_OK == status && SOPC_Atomic_Int_Get(&g_Client_Started) &&
           ((SessionConnectedState) SOPC_Atomic_Int_Get(&g_scState)) == SESSION_CONN_CONNECTED &&
           ((SessionConnectedState) SOPC_Atomic_Int_Get(&g_scState)) != SESSION_CONN_MSG_RECEIVED &&
           loopCpt * sleepTimeout <= loopTimeout)
    {
        loopCpt++;
        SOPC_Sleep(sleepTimeout);
    }

    if (loopCpt * sleepTimeout > loopTimeout)
    {
        status = SOPC_STATUS_TIMEOUT;
    }
    else if (SOPC_Atomic_Int_Get(&g_sendFailures) > 0)
    {
        status = SOPC_STATUS_NOK;
    }
    else if (!SOPC_Atomic_Int_Get(&g_Client_Started))
    {
        SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_PUBSUB, "Client is stopped while waiting response'\n");
        status = SOPC_STATUS_NOK;
    }

    return status;
}

static SOPC_ReturnStatus ActivateSessionWait_client(void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    // Sleep timeout in milliseconds
    const uint32_t sleepTimeout = 20;
    // Loop timeout in milliseconds
    const uint32_t loopTimeout = CLIENT_TIMEOUT_ACTIVATE_SESSION;
    // Counter to stop waiting on timeout
    uint32_t loopCpt = 0;

    /* Wait until session is activated or timeout */
    while (SOPC_Atomic_Int_Get(&g_Client_Started) &&
           ((SessionConnectedState) SOPC_Atomic_Int_Get(&g_scState)) != SESSION_CONN_CONNECTED &&
           ((SessionConnectedState) SOPC_Atomic_Int_Get(&g_scState)) != SESSION_CONN_FAILED &&
           loopCpt * sleepTimeout <= loopTimeout)
    {
        loopCpt++;
        // Retrieve received messages on socket
        SOPC_Sleep(sleepTimeout);
    }
    if (loopCpt * sleepTimeout >= loopTimeout)
    {
        status = SOPC_STATUS_TIMEOUT;
    }
    else if (false == SOPC_Atomic_Int_Get(&g_Client_Started))
    {
        SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_PUBSUB, "Client is stopped while connecting");
        status = SOPC_STATUS_NOK;
    }

    if (((SessionConnectedState) SOPC_Atomic_Int_Get(&g_scState)) == SESSION_CONN_CONNECTED &&
        SOPC_Atomic_Int_Get(&g_sendFailures) == 0)
    {
        SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_PUBSUB, "Client sessions activated: OK");
    }
    else
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Client sessions activated: NOK");
        status = SOPC_STATUS_NOK;
    }
    return status;
}

static OpcUa_CallRequest* newCallRequest_client(const char* securityGroupId,
                                                uint32_t startingTokenId,
                                                uint32_t requestedKeyCount)
{
    SOPC_Variant* arguments = SOPC_Calloc(3, sizeof(SOPC_Variant));
    if (NULL == arguments)
    {
        return NULL;
    }

    /* Build Method Call */

    SOPC_Variant* variant;

    // Security Group id
    variant = &arguments[0];
    SOPC_Variant_Initialize(variant);
    variant->BuiltInTypeId = SOPC_String_Id;
    variant->ArrayType = SOPC_VariantArrayType_SingleValue;
    SOPC_String_Initialize(&(variant->Value.String));
    SOPC_String_CopyFromCString(&(variant->Value.String), securityGroupId);

    // Starting Token Id
    variant = &arguments[1];
    SOPC_Variant_Initialize(variant);
    variant->BuiltInTypeId = SOPC_UInt32_Id;
    variant->ArrayType = SOPC_VariantArrayType_SingleValue;
    variant->Value.Uint32 = startingTokenId;

    // Starting Token Id
    variant = &arguments[2];
    SOPC_Variant_Initialize(variant);
    variant->BuiltInTypeId = SOPC_UInt32_Id;
    variant->ArrayType = SOPC_VariantArrayType_SingleValue;
    variant->Value.Uint32 = requestedKeyCount;

    OpcUa_CallMethodRequest* call = SOPC_Calloc(1, sizeof(OpcUa_CallMethodRequest));
    if (NULL == call)
    {
        for (int i = 0; i < 3; i++)
        {
            SOPC_Variant_Clear(&arguments[i]);
        }
        SOPC_Free(arguments);
        return NULL;
    }

    call->encodeableType = &OpcUa_CallMethodRequest_EncodeableType;

    call->ObjectId = (SOPC_NodeId){
        .IdentifierType = SOPC_IdentifierType_Numeric, .Namespace = 0, .Data = {.Numeric = OpcUaId_PublishSubscribe}};
    call->MethodId = (SOPC_NodeId){.IdentifierType = SOPC_IdentifierType_Numeric,
                                   .Namespace = 0,
                                   .Data = {.Numeric = OpcUaId_PublishSubscribe_GetSecurityKeys}};

    call->NoOfInputArguments = (int32_t) 3;
    call->InputArguments = arguments;

    /* Build Message */
    OpcUa_CallRequest* req = SOPC_Calloc(1, sizeof(OpcUa_CallRequest));
    if (NULL == req)
    {
        OpcUa_CallMethodRequest_Clear(call);
        SOPC_Free(arguments);
        return NULL;
    }
    req->encodeableType = &OpcUa_CallRequest_EncodeableType;
    req->NoOfMethodsToCall = 1;
    req->MethodsToCall = call;

    return req;
}

static SOPC_ReturnStatus Client_CloseSession(void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    uint32_t session1_idx = (uint32_t) SOPC_Atomic_Int_Get((int32_t*) &g_session);

    g_sendFailures = 0;

    /* Close the session */
    SessionConnectedState scStateCompare = (SessionConnectedState) SOPC_Atomic_Int_Get(&g_scState);
    if (0 != session1_idx && (scStateCompare == SESSION_CONN_MSG_RECEIVED || scStateCompare == SESSION_CONN_CONNECTED))
    {
        SOPC_ToolkitClient_AsyncCloseSession(session1_idx);
    }
    SOPC_Atomic_Int_Set(&g_scState, (int32_t) SESSION_CONN_NEW);
    return status;
}

SOPC_ReturnStatus Client_GetSecurityKeys(uint32_t SecureChannel_Id,
                                         uint32_t StartingTokenId,
                                         uint32_t requestedKeys,
                                         Client_SKS_GetKeys_Response* response)
{
    if (0 == requestedKeys || NULL == response)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (!SOPC_Atomic_Int_Get(&g_Client_Started))
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_PUBSUB, "Client_GetSecurityKeys %" PRIu32, SecureChannel_Id);

    SOPC_EndpointConnectionCfg endpointConnectionCfg = SOPC_EndpointConnectionCfg_CreateClassic(SecureChannel_Id);

    // For next step, with multiple client, use a object with is own state.
    status = SOPC_ToolkitClient_AsyncActivateSession_UsernamePassword(
        endpointConnectionCfg, SESSION_NAME, g_Client_SessionContext, CLIENT_SKS_USER_POLICY_ID, CLIENT_SKS_USERNAME,
        (const uint8_t*) CLIENT_SKS_PASSWORD, (int32_t) strlen(CLIENT_SKS_PASSWORD));
    if (SOPC_STATUS_OK == status)
    {
        status = ActivateSessionWait_client();
    }

    if (SOPC_STATUS_OK == status)
    {
        // Create CallRequest to be sent (deallocated by toolkit)
        OpcUa_CallRequest* callReq = newCallRequest_client(CLIENT_SECURITY_GROUPID, StartingTokenId, requestedKeys);
        SOPC_ToolkitClient_AsyncSendRequestOnSession((uint32_t) SOPC_Atomic_Int_Get((int32_t*) &g_session), callReq,
                                                     (uintptr_t) response);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = Wait_response_client();
    }

    Client_CloseSession();

    return status;
}

void Client_Clear(void)
{
    for (int32_t i = 0; i <= g_Client_SecureChannel_Current; i++)
    {
        SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
        SOPC_KeyManager_SerializedCertificate_Delete(
            (SOPC_SerializedCertificate*) g_Client_SecureChannel_Config[i].crt_cli);
        SOPC_KeyManager_SerializedAsymmetricKey_Delete(
            (SOPC_SerializedAsymmetricKey*) g_Client_SecureChannel_Config[i].key_priv_cli);
        SOPC_KeyManager_SerializedCertificate_Delete(
            (SOPC_SerializedCertificate*) g_Client_SecureChannel_Config[i].crt_srv);
        SOPC_PKIProvider_Free((SOPC_PKIProvider**) &(g_Client_SecureChannel_Config[i].pki));
        // SOPC_KeyManager_SerializedCertificate_Delete((SOPC_SerializedCertificate*) ck_cli[i].crt_ca);
        SOPC_Free((char*) g_Client_SecureChannel_Config[i].url);
        SOPC_GCC_DIAGNOSTIC_RESTORE

        g_Client_SecureChannel_Config[i].crt_cli = NULL;
        g_Client_SecureChannel_Config[i].key_priv_cli = NULL;
        g_Client_SecureChannel_Config[i].crt_srv = NULL;
        g_Client_SecureChannel_Config[i].pki = NULL;
        g_Client_SecureChannel_Config[i].url = NULL;
    }
}

void Client_Stop(void)
{
    SOPC_Atomic_Int_Set(&g_Client_Started, false);
}

static void Client_Copy_CallResponse_To_GetKeysResponse(Client_SKS_GetKeys_Response* response,
                                                        OpcUa_CallResponse* callResp)
{
    SOPC_ASSERT(NULL != response);
    SOPC_ASSERT(NULL != callResp);
    if (1 == callResp->NoOfResults && NULL != callResp->Results)
    {
        OpcUa_CallMethodResult* callResult = &callResp->Results[0];
        if (SOPC_STATUS_OK == callResult->StatusCode)
        {
            if (5 == callResult->NoOfOutputArguments && NULL != callResult->OutputArguments)
            {
                SOPC_Variant* variant;
                bool isValid = true;
                /* SecurityPolicyUri */
                variant = &(callResult->OutputArguments[0]);
                if (SOPC_String_Id == variant->BuiltInTypeId && variant->ArrayType == SOPC_VariantArrayType_SingleValue)
                {
                    if (0 != strcmp(SOPC_String_GetRawCString(&variant->Value.String),
                                    SOPC_SecurityPolicy_PubSub_Aes256_URI))
                    {
                        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_PUBSUB,
                                                 "GetSecurityKeys SecurityPolicyUri is not valid : %s",
                                                 SOPC_String_GetRawCString(&variant->Value.String));
                        isValid = false;
                    }
                    else
                    {
                        response->SecurityPolicyUri = SOPC_Calloc(1, sizeof(SOPC_String));
                        isValid = (NULL != response->SecurityPolicyUri);
                        if (isValid)
                        {
                            SOPC_String_Initialize(response->SecurityPolicyUri);

                            SOPC_String_Copy(response->SecurityPolicyUri, &variant->Value.String);
                        }
                    }
                }
                else
                {
                    SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_PUBSUB, "GetSecurityKeys output argument 1 is not valid");
                    isValid = false;
                }

                if (isValid)
                {
                    /* FirstTokenId */
                    variant = &(callResult->OutputArguments[1]);
                    if (SOPC_UInt32_Id == variant->BuiltInTypeId &&
                        variant->ArrayType == SOPC_VariantArrayType_SingleValue)
                    {
                        response->FirstTokenId = variant->Value.Uint32;
                    }
                    else
                    {
                        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_PUBSUB,
                                                 "GetSecurityKeys output argument 2 is not valid.");
                        isValid = false;
                    }
                }
                if (isValid)
                {
                    /* Keys */
                    variant = &(callResult->OutputArguments[2]);

                    /* ByteString Array of 1 element */
                    if (SOPC_ByteString_Id == variant->BuiltInTypeId &&
                        variant->ArrayType == SOPC_VariantArrayType_Array && 0 < variant->Value.Array.Length &&
                        NULL != variant->Value.Array.Content.BstringArr)
                    {
                        response->NbKeys = (uint32_t) variant->Value.Array.Length;
                        response->Keys = SOPC_Calloc(response->NbKeys, sizeof(SOPC_ByteString));
                        isValid = (NULL != response->Keys);
                        // Copy the Keys
                        for (uint32_t index = 0; index < response->NbKeys && isValid; index++)
                        {
                            SOPC_ByteString_Initialize(&response->Keys[index]);
                            SOPC_ByteString* byteString = &(variant->Value.Array.Content.BstringArr[index]);
                            if (KEYS_TOKEN_LENGTH == byteString->Length) // check size define by security policy
                            {
                                SOPC_ReturnStatus status;
                                status = SOPC_ByteString_Copy(&response->Keys[index], byteString);
                                isValid = (SOPC_STATUS_OK == status);
                            }
                            else
                            {
                                isValid = false;
                            }
                        }
                    }
                    else
                    {
                        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_PUBSUB,
                                                 "GetSecurityKeys output argument 3 is not valid");
                        isValid = false;
                    }
                }

                if (isValid)
                {
                    /* TimeToNextKey */
                    variant = &(callResult->OutputArguments[3]);
                    if (SOPC_Double_Id == variant->BuiltInTypeId &&
                        variant->ArrayType == SOPC_VariantArrayType_SingleValue && 0 <= variant->Value.Doublev &&
                        UINT32_MAX >= variant->Value.Doublev)
                    {
                        response->TimeToNextKey = (uint32_t) variant->Value.Doublev;
                        if (0 == response->TimeToNextKey)
                        {
                            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_PUBSUB,
                                                     "GetSecurityKeys TimeToNextKey is not valid. Should be not 0");
                            isValid = false;
                        }
                    }
                    else
                    {
                        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_PUBSUB,
                                                 "GetSecurityKeys output argument 3 is not valid");
                        isValid = false;
                    }
                }
                if (isValid)
                {
                    /* KeyLifetime */
                    variant = &(callResult->OutputArguments[4]);
                    if (SOPC_Double_Id == variant->BuiltInTypeId &&
                        variant->ArrayType == SOPC_VariantArrayType_SingleValue && 0 <= variant->Value.Doublev &&
                        UINT32_MAX >= variant->Value.Doublev)
                    {
                        response->KeyLifetime = (uint32_t) variant->Value.Doublev;
                        if (0 == response->KeyLifetime)
                        {
                            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_PUBSUB,
                                                     "GetSecurityKeys KeyLifetime is not valid. Should be not 0");
                            isValid = false;
                        }
                    }
                    else
                    {
                        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_PUBSUB,
                                                 "GetSecurityKeys output argument 4 is not valid");
                        isValid = false;
                    }
                }

                if (!isValid)
                {
                    // Clear response if something failed
                    SOPC_String_Clear(response->SecurityPolicyUri);
                    response->SecurityPolicyUri = NULL;
                    response->FirstTokenId = 0;
                    if (NULL != response->Keys)
                    {
                        for (uint32_t index = 0; index < response->NbKeys; index++)
                        {
                            SOPC_ByteString_Clear(&response->Keys[index]);
                        }
                        SOPC_Free(response->Keys);
                    }
                    response->NbKeys = 0;
                    response->TimeToNextKey = 0;
                    response->KeyLifetime = 0;
                }
            }
            else
            {
                SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_PUBSUB, "GetSecurityKeys does not return expected arguments");
            }
        }
        else
        {
            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_PUBSUB, "GetSecurityKeys failed. Returned status is %d",
                                     callResult->StatusCode);
        }
    }
    else
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_PUBSUB,
                                 "Call request should have returned one result but returned %" PRIi32 " instead",
                                 callResp->NoOfResults);
    }
}

void Client_Treat_Session_Response(void* param, uintptr_t appContext)
{
    if (NULL != param)
    {
        SOPC_EncodeableType* encType = *(SOPC_EncodeableType**) param;

        if (encType == &OpcUa_CallResponse_EncodeableType)
        {
            SOPC_Atomic_Int_Set(&g_scState, (int32_t) SESSION_CONN_MSG_RECEIVED);
            OpcUa_CallResponse* callResp = (OpcUa_CallResponse*) param;
            Client_SKS_GetKeys_Response* response = (Client_SKS_GetKeys_Response*) appContext;
            Client_Copy_CallResponse_To_GetKeysResponse(response, callResp);
        }
        else
        {
            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_PUBSUB, "Type of receive message not managed in client side.");
        }
    }
}

/* SKS part */

static SOPC_ReturnStatus Client_Provider_GetKeys_BySKS(SOPC_SKProvider* skp,
                                                       uint32_t StartingTokenId,
                                                       uint32_t NbRequestedToken,
                                                       SOPC_String** SecurityPolicyUri,
                                                       uint32_t* FirstTokenId,
                                                       SOPC_ByteString** Keys,
                                                       uint32_t* NbToken,
                                                       uint32_t* TimeToNextKey,
                                                       uint32_t* KeyLifetime)
{
    /* Not used*/
    (void) StartingTokenId;
    if (NULL == skp || NULL == SecurityPolicyUri || NULL == FirstTokenId || NULL == Keys || NULL == NbToken ||
        NULL == TimeToNextKey || NULL == KeyLifetime)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_SecureChannelConfigIdx SecureChannel_Id = (SOPC_SecureChannelConfigIdx) skp->data;
    SOPC_ASSERT(0 < SecureChannel_Id);

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    Client_SKS_GetKeys_Response* response = SOPC_Calloc(1, sizeof(Client_SKS_GetKeys_Response));
    if (NULL == response)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }

    if (SOPC_STATUS_OK == status)
    {
        /* Retrieve Security Keys from SKS.
           This function wait SKS server response or timeout */
        status = Client_GetSecurityKeys(SecureChannel_Id, 0, NbRequestedToken, response);
        if (0 == response->NbKeys || NULL == response->Keys)
        {
            status = SOPC_STATUS_NOK;
            SOPC_String_Clear(response->SecurityPolicyUri);
            SOPC_Free(response->SecurityPolicyUri);
        }
        else
        {
            *SecurityPolicyUri = response->SecurityPolicyUri;
            *FirstTokenId = response->FirstTokenId;
            *Keys = response->Keys;
            *NbToken = response->NbKeys;
            *TimeToNextKey = response->TimeToNextKey;
            *KeyLifetime = response->KeyLifetime;
        }
    }

    /* Free response: response data are moved in output parameters */
    SOPC_Free(response);

    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "PubSub Security Keys cannot be retrieved from SKS");
    }
    return status;
}

SOPC_SKProvider* Client_Provider_BySKS_Create(SOPC_SecureChannelConfigIdx SecureChannel_Id)
{
    SOPC_SKProvider* skp = SOPC_Calloc(1, sizeof(SOPC_SKProvider));
    if (NULL == skp)
    {
        return NULL;
    }

    skp->data = SecureChannel_Id; // index of the SC Channel in Client module context
    skp->ptrGetKeys = Client_Provider_GetKeys_BySKS;
    skp->ptrClear = NULL; // Data point on an integer

    return skp;
}

static SOPC_ReturnStatus Fallback_Provider_GetKeys_BySKS(SOPC_SKProvider* skp,
                                                         uint32_t StartingTokenId,
                                                         uint32_t NbRequestedToken,
                                                         SOPC_String** SecurityPolicyUri,
                                                         uint32_t* FirstTokenId,
                                                         SOPC_ByteString** Keys,
                                                         uint32_t* NbKeys,
                                                         uint32_t* TimeToNextKey,
                                                         uint32_t* KeyLifetime)
{
    /* Not used*/
    (void) StartingTokenId;
    (void) NbRequestedToken;

    if (NULL == skp || NULL == SecurityPolicyUri || NULL == FirstTokenId || NULL == Keys || NULL == NbKeys ||
        NULL == TimeToNextKey || NULL == KeyLifetime)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    *SecurityPolicyUri = SOPC_Malloc(sizeof(SOPC_String));
    SOPC_ReturnStatus status = (NULL == *SecurityPolicyUri) ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK;
    if (SOPC_STATUS_OK == status)
    {
        SOPC_String_Initialize(*SecurityPolicyUri);
        status = SOPC_String_AttachFromCstring(*SecurityPolicyUri, SOPC_SecurityPolicy_PubSub_Aes256_URI);
    }
    if (SOPC_STATUS_OK != status)
    {
        return status;
    }

    *FirstTokenId = SOPC_SK_MANAGER_CURRENT_TOKEN_ID + 1;
    *KeyLifetime = SOPC_SK_MANAGER_DEFAULT_KEYLIFETIME;
    *TimeToNextKey = SOPC_SK_MANAGER_DEFAULT_KEYLIFETIME;

    SOPC_Buffer* keys = NULL;
    uint32_t nbKeys = 1;

#ifdef WITH_STATIC_SECURITY_DATA
    keys = SOPC_Buffer_Create(sizeof(pubSub_keySign) + sizeof(pubSub_keyEncrypt) + sizeof(pubSub_keyNonce));

    if (SOPC_STATUS_OK == status && NULL != keys)
    {
        status = SOPC_Buffer_Write(keys, pubSub_keySign, (uint32_t) sizeof(pubSub_keySign));
    }
    else
    {
        status = SOPC_STATUS_NOK;
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Buffer_Write(keys, pubSub_keyEncrypt, (uint32_t) sizeof(pubSub_keyEncrypt));
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Buffer_Write(keys, pubSub_keyNonce, (uint32_t) sizeof(pubSub_keyNonce));
    }

    if (SOPC_STATUS_OK == status)
    {
        nbKeys = 1;
    }
#else
    SOPC_Buffer* signingKey = NULL;
    SOPC_Buffer* encryptKey = NULL;
    SOPC_Buffer* keyNonce = NULL;

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Buffer_ReadFile(PUBSUB_SKS_SIGNING_KEY, &signingKey);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Buffer_ReadFile(PUBSUB_SKS_ENCRYPT_KEY, &encryptKey);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Buffer_ReadFile(PUBSUB_SKS_KEY_NONCE, &keyNonce);
    }

    if (SOPC_STATUS_OK == status)
    {
        keys = SOPC_Buffer_Create(signingKey->length + encryptKey->length + keyNonce->length);
        if (NULL != keys)
        {
            status = SOPC_Buffer_Copy(keys, signingKey);
        }
        else
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
        if (SOPC_STATUS_OK == status)
        {
            SOPC_Buffer_SetPosition(keys, signingKey->length);
        }
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_Buffer_Write(keys, encryptKey->data, encryptKey->length);
        }
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_Buffer_Write(keys, keyNonce->data, keyNonce->length);
        }
        SOPC_Buffer_Clear(signingKey);
        SOPC_Free(signingKey);
        SOPC_Buffer_Clear(encryptKey);
        SOPC_Free(encryptKey);
        SOPC_Buffer_Clear(keyNonce);
        SOPC_Free(keyNonce);
    }
#endif

    if (SOPC_STATUS_OK == status)
    {
        *Keys = SOPC_Calloc(nbKeys, sizeof(SOPC_ByteString));
        status = (NULL == *Keys) ? SOPC_STATUS_OUT_OF_MEMORY : status;
    }
    if (SOPC_STATUS_OK == status)
    {
        SOPC_ASSERT(KEYS_TOKEN_LENGTH * nbKeys == keys->length);
        for (uint32_t i = 0; i < nbKeys; i++)
        {
            SOPC_ByteString_Initialize(&((*Keys)[i]));
            status =
                SOPC_ByteString_CopyFromBytes(&((*Keys)[i]), keys->data + i * KEYS_TOKEN_LENGTH, KEYS_TOKEN_LENGTH);
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        *NbKeys = nbKeys;
    }
    else
    {
        *NbKeys = 0;
        for (uint32_t i = 0; i < nbKeys; i++)
        {
            SOPC_ByteString_Clear(&((*Keys)[i]));
            SOPC_Free(*Keys);
        }
    }

    SOPC_Buffer_Clear(keys);
    SOPC_Free(keys);

    return status;
}

SOPC_SKProvider* Fallback_Provider_Create(void)
{
    SOPC_SKProvider* skp = SOPC_Calloc(1, sizeof(SOPC_SKProvider));
    if (NULL == skp)
    {
        return NULL;
    }

    skp->ptrGetKeys = Fallback_Provider_GetKeys_BySKS;
    skp->ptrClear = NULL;

    return skp;
}
