/* Copyright (C) Systerel SAS 2019, all rights reserved. */

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "opcua_statuscodes.h"

#include "sopc_atomic.h"
#include "sopc_crypto_profiles.h"
#include "sopc_encodeable.h"
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

#define CLIENT_SECURITY_GROUPID "sgid_1"
#define CLIENT_STARTING_TOKENID 0
#define CLIENT_REQUESTED_KEY_COUNT 5

#define CLIENT_TIMEOUT_ACTIVATE_SESSION 10000
#define CLIENT_TIMEOUT_CALL_REQUEST 10000

typedef struct s_Cerkey
{
    SOPC_SerializedCertificate* client_cert;
    SOPC_SerializedAsymmetricKey* client_key;
    SOPC_SerializedCertificate* server_cert;
    // SOPC_SerializedCertificate* crt_ca;
    SOPC_PKIProvider* pkiProvider;
} t_CerKey;

Client_Keys_Type Client_Keys = {.init = false};

/* Should be true to use functionnality */
int32_t Client_Started = 0;

uint32_t session = 0;
SessionConnectedState scState = SESSION_CONN_NEW;
// use to identify the active session response
uintptr_t Client_SessionContext = 1;

/* Secure Channel Configurations */
// Current and last secure channel registered
int64_t Client_SecureChannel_Current = -1;
// channel configuration
SOPC_SecureChannel_Config Client_SecureChannel_Config[SOPC_MAX_SECURE_CONNECTIONS];
// channel identifier
uint32_t Client_SecureChannel_Id[SOPC_MAX_SECURE_CONNECTIONS];

/* only one Secure Channel is used, keep all config to free memory when quit */
t_CerKey ck_cli[SOPC_MAX_SECURE_CONNECTIONS];

bool isScCreated = false;
int32_t sendFailures = 0;

/*
 * \brief Retrieve a the Secure Channel Configuration associated to the given endpoint url
 *        The Secure Channel Configuration should have been previously created using Client_AddSecureChannelconfig()
 *
 * \param endpoint_url     Endpoint Url of the SKS Server
 * \return                 A Secure Channel configuration ID or 0 if failed.
 */
static uint32_t Client_GetSecureChannelconfig(const char* endpoint_url);

static void Client_Copy_CallResponse_To_GetKeysResponse(Client_SKS_GetKeys_Response* response,
                                                        OpcUa_CallResponse* callResp);
static SOPC_ReturnStatus CerAndKeyLoader_client(const char* client_key_path,
                                                const char* client_cert_path,
                                                SOPC_SerializedCertificate* sks_server_cert);
static SOPC_ReturnStatus Wait_response_client(void);
static SOPC_ReturnStatus Client_SaveKeys(SOPC_ByteString* key, char* path);
SOPC_ReturnStatus Client_CloseSession(void);

static SOPC_ReturnStatus Client_SaveKeys(SOPC_ByteString* key, char* path)
{
    if (NULL == key || 0 >= key->Length || NULL == key->Data || NULL == path)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    FILE* fd = fopen(path, "w");
    if (NULL != fd)
    {
        size_t len = (size_t) key->Length;

        char* buf = SOPC_Calloc(len + 1, sizeof(char));
        if (NULL == buf)
        {
            printf("# Error: Out of memory when saving key in %s\n", path);
            result = SOPC_STATUS_OUT_OF_MEMORY;
        }
        else
        {
            memcpy(buf, key->Data, len);
            buf[len] = '\0';
            const int ret = fputs(buf, fd);
            if (EOF == ret)
            {
                printf("# Error: Cannot write key in %s\n", path);
                result = SOPC_STATUS_NOK;
            }
            SOPC_Free(buf);
            fclose(fd);
        }
    }
    else
    {
        printf("# Error: Cannot open \"%s\": %s\n", path, strerror(errno));
        result = SOPC_STATUS_NOK;
    }
    return result;
}

// Configure the 2 secure channel connections to use and retrieve channel configuration index
static SOPC_ReturnStatus CerAndKeyLoader_client(const char* client_key_path,
                                                const char* client_cert_path,
                                                SOPC_SerializedCertificate* sks_server_cert)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    status = SOPC_KeyManager_SerializedCertificate_CreateFromFile(client_cert_path,
                                                                  &ck_cli[Client_SecureChannel_Current].client_cert);
    if (SOPC_STATUS_OK != status)
    {
        printf("# Error: Client failed to load client certificate\n");
    }
    else
    {
        Client_SecureChannel_Config[Client_SecureChannel_Current].crt_cli =
            ck_cli[Client_SecureChannel_Current].client_cert;
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_SerializedAsymmetricKey_CreateFromFile(
            client_key_path, &ck_cli[Client_SecureChannel_Current].client_key);
        if (SOPC_STATUS_OK != status)
        {
            printf("# Error: Client failed to load private key\n");
        }
        else
        {
            Client_SecureChannel_Config[Client_SecureChannel_Current].key_priv_cli =
                ck_cli[Client_SecureChannel_Current].client_key;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        ck_cli[Client_SecureChannel_Current].server_cert = SOPC_Buffer_Create(sks_server_cert->length);
        if (NULL == ck_cli[Client_SecureChannel_Current].server_cert)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
        else
        {
            status = SOPC_Buffer_Copy(ck_cli[Client_SecureChannel_Current].server_cert, sks_server_cert);
        }
        if (SOPC_STATUS_OK != status)
        {
            printf("# Error: Client failed to load server certificate\n");
        }
        else
        {
            Client_SecureChannel_Config[Client_SecureChannel_Current].crt_srv =
                ck_cli[Client_SecureChannel_Current].server_cert;
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
        status = SOPC_PKIProviderStack_CreateFromPaths(lPathsTrustedRoots, lPathsTrustedLinks, lPathsUntrustedRoots,
                                                       lPathsUntrustedLinks, lPathsIssuedCerts, lPathsCRL,
                                                       &ck_cli[Client_SecureChannel_Current].pkiProvider);
        if (SOPC_STATUS_OK != status)
        {
            printf("# Error: Failed to create PKI\n");
        }
        else
        {
            Client_SecureChannel_Config[Client_SecureChannel_Current].pki =
                ck_cli[Client_SecureChannel_Current].pkiProvider;
        }
    }

    if (SOPC_STATUS_OK != status)
    {
        printf("# Error: Client failed loading certificates and key (check paths are valid)\n");
    }
    return (status);
}

SOPC_ReturnStatus Client_Setup()
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    return (status);
}

void Client_Start(void)
{
    SOPC_Atomic_Int_Set(&Client_Started, 1);
}

void Client_Stop(void)
{
    SOPC_Atomic_Int_Set(&Client_Started, 0);
}

uint32_t Client_AddSecureChannelconfig(const char* endpoint_url, SOPC_SerializedCertificate* server_cert)
{
    assert(NULL != endpoint_url);
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    uint32_t scId = Client_GetSecureChannelconfig(endpoint_url);

    if (scId > 0)
    {
        return scId;
    }

    if (SOPC_MAX_SECURE_CONNECTIONS <= Client_SecureChannel_Current + 1)
    {
        printf("# Error : Too many secure channel created\n");
        return SOPC_STATUS_NOK;
    }
    Client_SecureChannel_Current++;

    // A Secure channel connection configuration
    Client_SecureChannel_Config[Client_SecureChannel_Current].isClientSc = true;
    Client_SecureChannel_Config[Client_SecureChannel_Current].crt_cli = NULL;
    Client_SecureChannel_Config[Client_SecureChannel_Current].key_priv_cli = NULL;
    Client_SecureChannel_Config[Client_SecureChannel_Current].crt_srv = NULL;
    Client_SecureChannel_Config[Client_SecureChannel_Current].pki = NULL;
    Client_SecureChannel_Config[Client_SecureChannel_Current].reqSecuPolicyUri = SOPC_SecurityPolicy_Basic256Sha256_URI;
    Client_SecureChannel_Config[Client_SecureChannel_Current].requestedLifetime = 20000;
    Client_SecureChannel_Config[Client_SecureChannel_Current].msgSecurityMode =
        OpcUa_MessageSecurityMode_SignAndEncrypt;
    // Copy Endpoint URL
    char* endpoint_url_copy = SOPC_Calloc(strlen(endpoint_url) + 1, sizeof(char));
    Client_SecureChannel_Config[Client_SecureChannel_Current].url = endpoint_url_copy;
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
            printf("# Error: FAILED on configuring Certificate, key and Sc\n");
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        Client_SecureChannel_Id[Client_SecureChannel_Current] =
            SOPC_ToolkitClient_AddSecureChannelConfig(&(Client_SecureChannel_Config[Client_SecureChannel_Current]));
        if (0 == Client_SecureChannel_Id[Client_SecureChannel_Current])
        {
            printf("# Error: Failed to configure the secure channel connections\n");
            status = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK != status)
    {
        return 0;
    }
    else
    {
        return Client_SecureChannel_Id[Client_SecureChannel_Current];
    }
}

static uint32_t Client_GetSecureChannelconfig(const char* endpoint_url)
{
    for (uint32_t i = 0; i <= Client_SecureChannel_Current; i++)
    {
        if (0 == strcmp(Client_SecureChannel_Config[Client_SecureChannel_Current].url, endpoint_url))
        {
            return Client_SecureChannel_Id[Client_SecureChannel_Current];
        }
    }
    return 0;
}

SOPC_ReturnStatus Wait_response_client()
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    // Sleep timeout in milliseconds
    const uint32_t sleepTimeout = 20;
    // Loop timeout in milliseconds
    const uint32_t loopTimeout = CLIENT_TIMEOUT_CALL_REQUEST;
    // Counter to stop waiting on timeout
    uint32_t loopCpt = 0;

    loopCpt = 0;
    while (SOPC_STATUS_OK == status && 1 == SOPC_Atomic_Int_Get(&Client_Started) &&
           SOPC_Atomic_Int_Get(&scState) != SESSION_CONN_MSG_RECEIVED && loopCpt * sleepTimeout <= loopTimeout)
    {
        loopCpt++;
        SOPC_Sleep(sleepTimeout);
    }

    if (loopCpt * sleepTimeout > loopTimeout)
    {
        status = SOPC_STATUS_TIMEOUT;
    }
    else if (SOPC_Atomic_Int_Get(&sendFailures) > 0)
    {
        status = SOPC_STATUS_NOK;
    }
    else if (0 == SOPC_Atomic_Int_Get(&Client_Started))
    {
        printf("# Info: Client is stopped while waiting response'\n");
        status = SOPC_STATUS_NOK;
    }

    return (status);
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
    while (1 == SOPC_Atomic_Int_Get(&Client_Started) && SOPC_Atomic_Int_Get(&scState) != SESSION_CONN_CONNECTED &&
           loopCpt * sleepTimeout <= loopTimeout)
    {
        loopCpt++;
        // Retrieve received messages on socket
        SOPC_Sleep(sleepTimeout);
    }
    if (loopCpt * sleepTimeout > loopTimeout)
    {
        status = SOPC_STATUS_TIMEOUT;
    }
    else if (0 == SOPC_Atomic_Int_Get(&Client_Started))
    {
        printf("# Info: Client is stopped while connecting'\n");
        status = SOPC_STATUS_NOK;
    }

    if (SOPC_Atomic_Int_Get(&scState) == SESSION_CONN_CONNECTED && SOPC_Atomic_Int_Get(&sendFailures) == 0)
    {
        printf("# Info: Client sessions activated: OK'\n");
    }
    else
    {
        printf("# Error: Client sessions activated: NOK'\n");
        status = SOPC_STATUS_NOK;
    }
    return (status);
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

    call->ObjectId =
        (SOPC_NodeId){.IdentifierType = SOPC_IdentifierType_Numeric, .Namespace = 0, .Data = {.Numeric = 15000}};
    call->MethodId =
        (SOPC_NodeId){.IdentifierType = SOPC_IdentifierType_Numeric, .Namespace = 0, .Data = {.Numeric = 15001}};

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

SOPC_ReturnStatus Client_GetSecurityKeys(uint32_t SecureChannel_Id,
                                         uint32_t StartingTokenId,
                                         uint32_t requestedKeys,
                                         Client_SKS_GetKeys_Response* response)
{
    if (0 == requestedKeys || NULL == response)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (0 == SOPC_Atomic_Int_Get(&Client_Started))
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    printf("Client_GetSecurityKeys %u\n", SecureChannel_Id);

    const char* pwd = "password";
    // status = SOPC_ToolkitClient_AsyncActivateSession_Anonymous(channel_config_idx, Client_SessionContext,
    // "anonymous");
    // TODO use Client_SessionContext instead of 1 as 2e parameter. It will be use to identify the current session
    // For next step, with multiple client, use a object with is own state.
    status = SOPC_ToolkitClient_AsyncActivateSession_UsernamePassword(SecureChannel_Id, 1, "username", "user1",
                                                                      (const uint8_t*) pwd, (int32_t) strlen(pwd));
    if (SOPC_STATUS_OK == status)
    {
        status = ActivateSessionWait_client();
    }

    if (SOPC_STATUS_OK == status)
    {
        // Create CallRequest to be sent (deallocated by toolkit)
        OpcUa_CallRequest* callReq = newCallRequest_client(CLIENT_SECURITY_GROUPID, StartingTokenId, requestedKeys);
        SOPC_ToolkitClient_AsyncSendRequestOnSession((uint32_t) SOPC_Atomic_Int_Get((int32_t*) &session), callReq,
                                                     (uintptr_t) response);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = Wait_response_client();
    }

    /* Save received keys */
    /* TODO Remove ? */
    if (SOPC_STATUS_OK == status && Client_Keys.init)
    {
        status = Client_SaveKeys(&(Client_Keys.SigningKey), PUBSUB_SKS_SIGNING_KEY);
    }
    if (SOPC_STATUS_OK == status && Client_Keys.init)
    {
        status = Client_SaveKeys(&(Client_Keys.EncryptingKey), PUBSUB_SKS_ENCRYPT_KEY);
    }
    if (SOPC_STATUS_OK == status && Client_Keys.init)
    {
        status = Client_SaveKeys(&(Client_Keys.KeyNonce), PUBSUB_SKS_KEY_NONCE);
    }

    Client_CloseSession();

    return (status);
}

SOPC_ReturnStatus Client_CloseSession(void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    uint32_t session1_idx = (uint32_t) SOPC_Atomic_Int_Get((int32_t*) &session);

    sendFailures = 0;

    /* Close the session */
    SessionConnectedState scStateCompare = SOPC_Atomic_Int_Get(&scState);
    if (0 != session1_idx && (scStateCompare == SESSION_CONN_MSG_RECEIVED || scStateCompare == SESSION_CONN_CONNECTED))
    {
        SOPC_ToolkitClient_AsyncCloseSession(session1_idx);
    }
    SOPC_Atomic_Int_Set((SessionConnectedState*) &scState, (SessionConnectedState) SESSION_CONN_NEW);
    return (status);
}

void Client_Teardown()
{
    for (int32_t i = 0; i <= Client_SecureChannel_Current; i++)
    {
        SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
        SOPC_KeyManager_SerializedCertificate_Delete(
            (SOPC_SerializedCertificate*) Client_SecureChannel_Config[i].crt_cli);
        SOPC_KeyManager_SerializedAsymmetricKey_Delete(
            (SOPC_SerializedAsymmetricKey*) Client_SecureChannel_Config[i].key_priv_cli);
        SOPC_KeyManager_SerializedCertificate_Delete(
            (SOPC_SerializedCertificate*) Client_SecureChannel_Config[i].crt_srv);
        SOPC_PKIProvider_Free((SOPC_PKIProvider**) &(Client_SecureChannel_Config[i].pki));
        // SOPC_KeyManager_SerializedCertificate_Delete((SOPC_SerializedCertificate*) ck_cli[i].crt_ca);
        SOPC_Free((char*) Client_SecureChannel_Config[i].url);
        SOPC_GCC_DIAGNOSTIC_RESTORE

        Client_SecureChannel_Config[i].crt_cli = NULL;
        Client_SecureChannel_Config[i].key_priv_cli = NULL;
        Client_SecureChannel_Config[i].crt_srv = NULL;
        Client_SecureChannel_Config[i].pki = NULL;
        Client_SecureChannel_Config[i].url = NULL;
        ck_cli[i].client_cert = NULL;
        ck_cli[i].client_key = NULL;
        ck_cli[i].server_cert = NULL;
        // ck_cli[i].crt_ca = NULL;
        ck_cli[i].pkiProvider = NULL;
    }

    Client_KeysClear();
}

void Client_KeysClear(void)
{
    if (Client_Keys.init)
    {
        SOPC_ByteString_Clear(&(Client_Keys.SigningKey));
        SOPC_ByteString_Clear(&(Client_Keys.EncryptingKey));
        SOPC_ByteString_Clear(&(Client_Keys.KeyNonce));
    }
    Client_Keys.init = false;
}

void Client_Treat_Session_Response(void* param, uintptr_t appContext)
{
    if (NULL != param)
    {
        SOPC_EncodeableType* encType = *(SOPC_EncodeableType**) param;

        if (encType == &OpcUa_CallResponse_EncodeableType)
        {
            SOPC_Atomic_Int_Set((SessionConnectedState*) &scState, (SessionConnectedState) SESSION_CONN_MSG_RECEIVED);
            OpcUa_CallResponse* callResp = (OpcUa_CallResponse*) param;
            Client_SKS_GetKeys_Response* response = (Client_SKS_GetKeys_Response*) appContext;
            Client_Copy_CallResponse_To_GetKeysResponse(response, callResp);
        }
        else
        {
            printf("# Warning: Type of receive message not managed in client side.\n");
        }
    }
}

void Client_Copy_CallResponse_To_GetKeysResponse(Client_SKS_GetKeys_Response* response, OpcUa_CallResponse* callResp)
{
    assert(NULL != response);
    assert(NULL != callResp);
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
                        printf("# Warning: GetSecurityKeys SecurityPolicyUri is not valid : %s .\n",
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
                    printf("# Warning: GetSecurityKeys output argument 1 is not valid.\n");
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
                        printf("# Warning: GetSecurityKeys output argument 2 is not valid.\n");
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
                        // Initialize all Keys first (to be delete properly if next allocation failed)
                        for (uint32_t index = 0; index < response->NbKeys && isValid; index++)
                        {
                            SOPC_ByteString_Initialize(&response->Keys[index]);
                        }
                        // Then copy the Keys
                        for (uint32_t index = 0; index < response->NbKeys && isValid; index++)
                        {
                            SOPC_ByteString* byteString = &(variant->Value.Array.Content.BstringArr[index]);
                            if (32 + 32 + 4 == byteString->Length) // check size define by security policy
                            {
                                SOPC_ReturnStatus status;
                                SOPC_ByteString_Initialize(&response->Keys[index]);
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
                        printf("# Warning: GetSecurityKeys output argument 3 is not valid.\n");
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
                            printf("# Warning: GetSecurityKeys TimeToNextKey is not valid. Should be not 0.\n");
                            isValid = false;
                        }
                    }
                    else
                    {
                        printf("# Warning: GetSecurityKeys output argument 3 is not valid.\n");
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
                            printf("# Warning: GetSecurityKeys KeyLifetime is not valid. Should be not 0.\n");
                            isValid = false;
                        }
                    }
                    else
                    {
                        printf("# Warning: GetSecurityKeys output argument 4 is not valid.\n");
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
                printf("# Warning: GetSecurityKeys does not return expected arguments.\n");
            }
        }
        else
        {
            printf("# Warning: GetSecurityKeys failed. Returned status is %d.\n", callResult->StatusCode);
        }
    }
    else
    {
        printf("# Warning: Call request should have returned one result but returned %d instead of.\n",
               callResp->NoOfResults);
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

    uint32_t SecureChannel_Id = *((uint32_t*) skp->data);
    assert(0 < SecureChannel_Id);

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
        printf("# Error: PubSub Security Keys cannot be retrieved from SKS\n");
    }
    return status;
}

SOPC_SKProvider* Client_Provider_BySKS_Create(uint32_t SecureChannel_Id)
{
    SOPC_SKProvider* skp = SOPC_Calloc(1, sizeof(SOPC_SKProvider));
    if (NULL == skp)
    {
        return NULL;
    }

    skp->data = SOPC_Calloc(1, sizeof(uint32_t));
    if (NULL == skp->data)
    {
        SOPC_Free(skp);
        return NULL;
    }

    *(uint32_t*) skp->data = SecureChannel_Id; // index of the SC Channel in Client module context
    skp->ptrGetKeys = Client_Provider_GetKeys_BySKS;
    skp->ptrClear = NULL; // Data point on an integer

    return skp;
}
