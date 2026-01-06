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
#include "sopc_date_time.h"
#include "sopc_encodeabletype.h"
#include "sopc_helper_string.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_pki_stack.h"
#include "sopc_types.h"

#include "libs2opc_client.h"
#include "libs2opc_client_config_custom.h"

#include "client.h"
#include "config.h"

#ifdef WITH_STATIC_SECURITY_DATA
#include "common_static_security_data.h"
#endif

const char* SESSION_NAME = "S2OPC_SKS_client_session";

#define CLIENT_SKS_SECURITY_MODE OpcUa_MessageSecurityMode_SignAndEncrypt
#define CLIENT_SKS_REQ_SECURITY_POLICY SOPC_SecurityPolicy_Basic256Sha256

#define CLIENT_SKS_USER_POLICY_ID "username_Basic256Sha256"
#define CLIENT_SKS_USERNAME "secuAdmin"

#define CLIENT_STARTING_TOKENID 0
#define CLIENT_REQUESTED_KEY_COUNT 5

#define CLIENT_TIMEOUT_ACTIVATE_SESSION 10000
#define CLIENT_TIMEOUT_CALL_REQUEST 10000

/* Length of token of keys (sign, encrypt, nonce) */
#define KEYS_TOKEN_LENGTH (32 + 32 + 4)

/* Should be true to use functionality */
static int32_t g_Client_Started = 0;

uint32_t g_session = 0;
int32_t g_scState = (int32_t) SESSION_CONN_NEW;
// use to identify the active session response
uintptr_t g_Client_SessionContext = 1;

/* Secure Channel Configurations */
// Current and last secure connection configured
static int64_t g_Client_SecureConnection_Current = -1;
// secure connection configuration
static SOPC_SecureConnection_Config* g_Client_SecureConnection_Config[SOPC_MAX_CLIENT_SECURE_CONNECTIONS_CONFIG + 1] = {
    NULL};

int32_t g_sendFailures = 0;

#ifndef WITH_STATIC_SECURITY_DATA
static bool SOPC_TestHelper_AskPass_FromEnv(char** outPassword)
{
    SOPC_ASSERT(NULL != outPassword);
    /*
        We have to make a copy here because in any case, we will free the password and not distinguish if it come
        from environment or terminal after calling ::SOPC_KeyManager_SerializedAsymmetricKey_CreateFromFile_WithPwd
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
#endif

static bool SOPC_TestHelper_AskUserPass_FromEnv(char** outPassword)
{
    SOPC_ASSERT(NULL != outPassword);
    char* _outPassword = getenv(USER_PASSWORD_ENV_NAME);
    *outPassword = SOPC_strdup(_outPassword); // Do a copy
    if (NULL == *outPassword)
    {
        printf("INFO: %s environment variable not set or empty, use terminal interactive input:\n",
               USER_PASSWORD_ENV_NAME);
        return SOPC_AskPass_CustomPromptFromTerminal("User '" CLIENT_SKS_USERNAME "' password:\n", outPassword);
    }
    return true;
}

static bool SOPC_GetClientUserPassword(const SOPC_SecureConnection_Config* secConnConfig,
                                       char** outUserName,
                                       char** outPassword)
{
    SOPC_UNUSED_ARG(secConnConfig);

    char* userName = SOPC_Calloc(strlen(CLIENT_SKS_USERNAME) + 1, sizeof(*userName));
    if (NULL == userName)
    {
        return false;
    }
    memcpy(userName, CLIENT_SKS_USERNAME, strlen(CLIENT_SKS_USERNAME) + 1);

    bool res = SOPC_TestHelper_AskUserPass_FromEnv(outPassword);
    if (res)
    {
        *outUserName = userName;
    }
    else
    {
        SOPC_Free(userName);
    }
    return res;
}

SOPC_ReturnStatus Client_Initialize(void)
{
    SOPC_ReturnStatus status = SOPC_ClientConfigHelper_Initialize();
    if (SOPC_STATUS_OK != status)
    {
        return status;
    }

    status = SOPC_ClientConfigHelper_SetPreferredLocaleIds(1, (const char*[]){"en-US"});

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_SetApplicationDescription(APPLICATION_URI, PRODUCT_URI, CLIENT_DESCRIPTION,
                                                                   "en-US", OpcUa_ApplicationType_Client);
    }

    SOPC_PKIProvider* pkiProvider = NULL;

#ifdef WITH_STATIC_SECURITY_DATA
    SOPC_CertificateList* static_cacert = NULL;
    SOPC_CRLList* static_cacrl = NULL;

    /* Load client certificates and key from C source files (no filesystem needed) */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_SetKeyCertPairFromBytes(sizeof(client_2k_cert), client_2k_cert,
                                                                 sizeof(client_2k_key), client_2k_key);
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_Certificate_CreateOrAddFromDER(cacert, sizeof(cacert), &static_cacert);
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_CRL_CreateOrAddFromDER(cacrl, sizeof(cacrl), &static_cacrl);
    }

    /* Create the PKI (Public Key Infrastructure) provider */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_PKIProvider_CreateFromList(static_cacert, static_cacrl, NULL, NULL, &pkiProvider);
    }
    /* Clean in all cases */
    SOPC_KeyManager_Certificate_Free(static_cacert);
    SOPC_KeyManager_CRL_Free(static_cacrl);

#else // WITH_STATIC_SECURITY_DATA == false

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_SetClientKeyPasswordCallback(&SOPC_TestHelper_AskPass_FromEnv);
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Failed to configure the client key password callback");
    }

    /* Load client certificate and key from files */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_SetKeyCertPairFromPath(CLIENT_CERT_PATH, CLIENT_KEY_PATH, true);
    }
    /* Create the PKI (Public Key Infrastructure) provider */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_PKIProvider_CreateFromStore(PKI_PATH, &pkiProvider);
    }
#endif

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_SetPKIprovider(pkiProvider);
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Failed to create PKI");
    }

    if (SOPC_STATUS_OK == status)
    {
        // Set callback necessary to retrieve user password (from environment variable)
        status = SOPC_ClientConfigHelper_SetUserNamePasswordCallback(&SOPC_GetClientUserPassword);
        SOPC_ASSERT(SOPC_STATUS_OK == status);
    }

    return status;
}

void Client_Start(void)
{
    SOPC_Atomic_Int_Set(&g_Client_Started, true);
}

/*
 * \brief Retrieve a the Secure Connection Configuration associated to the given endpoint url and server certificate
 *        The Secure Connection Configuration should have been previously created using
 * Client_AddSecureConnectionConfig()
 *
 * \param endpoint_url     Endpoint Url of the SKS Server
 * \param server_cert      SKS server serialized certificate
 * \return                 A Secure Connection configuration or NULL if failed.
 */
static SOPC_SecureConnection_Config* Client_GetSecureConnectionConfig(const char* endpoint_url,
                                                                      SOPC_SerializedCertificate* server_cert)
{
    for (uint32_t i = 0; NULL != g_Client_SecureConnection_Config[i] && i <= g_Client_SecureConnection_Current; i++)
    {
        SOPC_SecureChannel_Config* scCfg = &g_Client_SecureConnection_Config[i]->scConfig;
        if (0 == strcmp(scCfg->url, endpoint_url))
        {
            SOPC_SerializedCertificate* peerSrvCrt = NULL;
            SOPC_ReturnStatus status = SOPC_KeyCertPair_GetSerializedCertCopy(scCfg->peerAppCert, &peerSrvCrt);
            if (SOPC_STATUS_OK == status && peerSrvCrt->length == server_cert->length &&
                0 == memcmp(peerSrvCrt->data, server_cert->data, server_cert->length))
            {
                SOPC_KeyManager_SerializedCertificate_Delete(peerSrvCrt);
                return g_Client_SecureConnection_Config[i];
            }
            SOPC_KeyManager_SerializedCertificate_Delete(peerSrvCrt);
        }
    }
    return NULL;
}

SOPC_SecureConnection_Config* Client_AddSecureConnectionConfig(const char* endpoint_url,
                                                               SOPC_SerializedCertificate* server_cert)
{
    SOPC_ASSERT(NULL != endpoint_url);

    SOPC_SecureConnection_Config* scConnConfig = Client_GetSecureConnectionConfig(endpoint_url, server_cert);

    if (NULL != scConnConfig)
    {
        /* We return same secure connection configuration if endpoint already used for an SC config */
        return scConnConfig;
    }

    SOPC_SecureConnection_Config* secureConnConfig = SOPC_ClientConfigHelper_CreateSecureConnection(
        "", endpoint_url, CLIENT_SKS_SECURITY_MODE, CLIENT_SKS_REQ_SECURITY_POLICY);

    if (NULL == secureConnConfig)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Too many secure channel created");
        return NULL;
    }

    SOPC_ReturnStatus status = SOPC_SecureConnectionConfig_SetServerCertificateFromBytes(
        secureConnConfig, server_cert->length, server_cert->data);

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_SecureConnectionConfig_SetUserName(secureConnConfig, CLIENT_SKS_USER_POLICY_ID, NULL, NULL);
    }

    if (SOPC_STATUS_OK == status)
    {
        g_Client_SecureConnection_Current++;
        g_Client_SecureConnection_Config[g_Client_SecureConnection_Current] = scConnConfig;
    }

    if (SOPC_STATUS_OK != status)
    {
        secureConnConfig = NULL;
    }
    return secureConnConfig;
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

    // Requested Key Count
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

    call->ObjectId = (SOPC_NodeId) SOPC_NODEID_NS0_NUMERIC(OpcUaId_PublishSubscribe);
    call->MethodId = (SOPC_NodeId) SOPC_NODEID_NS0_NUMERIC(OpcUaId_PublishSubscribe_GetSecurityKeys);

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
                        response->TimeToNextKey = (uint64_t) variant->Value.Doublev;
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
                        response->KeyLifetime = (uint64_t) variant->Value.Doublev;
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

static void ClientConnectionEvent(SOPC_ClientConnection* config,
                                  SOPC_ClientConnectionEvent event,
                                  SOPC_StatusCode status)
{
    SOPC_UNUSED_ARG(config);

    // We do not expect events since we use synchronous connection / disconnection, only for degraded case
    SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB,
                           "ClientConnectionEvent: Unexpected connection event %d with status 0x%08" PRIX32 "\n", event,
                           status);
}

SOPC_ReturnStatus Client_GetSecurityKeys(SOPC_SecureConnection_Config* config,
                                         const char* securityGroupId,
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

    SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_PUBSUB, "Client_GetSecurityKeys %s", config->scConfig.url);

    SOPC_ClientConnection* secureConnection = NULL;
    SOPC_ReturnStatus status = SOPC_ClientHelper_Connect(config, &ClientConnectionEvent, &secureConnection);

    if (SOPC_STATUS_OK == status)
    {
        OpcUa_CallResponse* callResp = NULL;
        // Create CallRequest to be sent (deallocated by toolkit)
        OpcUa_CallRequest* callReq = newCallRequest_client(securityGroupId, StartingTokenId, requestedKeys);

        if (NULL != callReq)
        {
            status = SOPC_ClientHelper_ServiceSync(secureConnection, (void*) callReq, (void**) &callResp);
        }
        else
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
        if (SOPC_STATUS_OK == status && SOPC_IsGoodStatus(callResp->ResponseHeader.ServiceResult))
        {
            SOPC_ASSERT(&OpcUa_CallResponse_EncodeableType == callResp->encodeableType);
            Client_Copy_CallResponse_To_GetKeysResponse(response, callResp);
        }
        else
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Service Call for Client_GetSecurityKeys %s failed",
                                   config->scConfig.url);
        }
        if (NULL != callResp)
        {
            SOPC_EncodeableObject_Delete(callResp->encodeableType, (void**) &callResp);
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelper_Disconnect(&secureConnection);
    }

    return status;
}

void Client_Clear(void)
{
    SOPC_ClientConfigHelper_Clear();
    g_Client_SecureConnection_Current = -1;
}

void Client_Stop(void)
{
    SOPC_Atomic_Int_Set(&g_Client_Started, false);
}

/* SKS part */

static SOPC_ReturnStatus Client_Provider_GetKeys_BySKS(SOPC_SKProvider* skp,
                                                       const char* securityGroupId,
                                                       uint32_t StartingTokenId,
                                                       uint32_t NbRequestedToken,
                                                       SOPC_String** SecurityPolicyUri,
                                                       uint32_t* FirstTokenId,
                                                       SOPC_ByteString** Keys,
                                                       uint32_t* NbToken,
                                                       uint64_t* TimeToNextKey,
                                                       uint64_t* KeyLifetime)
{
    /* Not used*/
    (void) StartingTokenId;
    if (NULL == skp || NULL == SecurityPolicyUri || NULL == FirstTokenId || NULL == Keys || NULL == NbToken ||
        NULL == TimeToNextKey || NULL == KeyLifetime)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_SecureConnection_Config* secureConnCfg = (SOPC_SecureConnection_Config*) skp->data;
    SOPC_ASSERT(NULL != secureConnCfg);

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

        status = Client_GetSecurityKeys(secureConnCfg, securityGroupId, 0, NbRequestedToken, response);
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

SOPC_SKProvider* Client_Provider_BySKS_Create(SOPC_SecureConnection_Config* secureConnCfg)
{
    SOPC_SKProvider* skp = SOPC_Calloc(1, sizeof(SOPC_SKProvider));
    if (NULL == skp)
    {
        return NULL;
    }

    skp->data = (uintptr_t) secureConnCfg; // secure connection configuration in Client module context
    skp->ptrGetKeys = Client_Provider_GetKeys_BySKS;
    skp->ptrClear = NULL; // Data point on an integer

    return skp;
}

static SOPC_ReturnStatus Fallback_Provider_GetKeys_BySKS(SOPC_SKProvider* skp,
                                                         const char* securityGroupId,
                                                         uint32_t StartingTokenId,
                                                         uint32_t NbRequestedToken,
                                                         SOPC_String** SecurityPolicyUri,
                                                         uint32_t* FirstTokenId,
                                                         SOPC_ByteString** Keys,
                                                         uint32_t* NbKeys,
                                                         uint64_t* TimeToNextKey,
                                                         uint64_t* KeyLifetime)
{
    /* Not used*/
    SOPC_UNUSED_ARG(securityGroupId); // not managed for fallback provider
    SOPC_UNUSED_ARG(StartingTokenId);
    SOPC_UNUSED_ARG(NbRequestedToken);

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
    }
    SOPC_Buffer_Clear(signingKey);
    SOPC_Free(signingKey);
    SOPC_Buffer_Clear(encryptKey);
    SOPC_Free(encryptKey);
    SOPC_Buffer_Clear(keyNonce);
    SOPC_Free(keyNonce);

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
        for (uint32_t i = 0; NULL != *Keys && i < nbKeys; i++)
        {
            SOPC_ByteString_Clear(&((*Keys)[i]));
        }
        SOPC_Free(*Keys);
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
