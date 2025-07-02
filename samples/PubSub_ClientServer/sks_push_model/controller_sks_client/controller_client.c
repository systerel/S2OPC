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

#include "controller_client.h"
#include "controller_config.h"

const char* SESSION_NAME = "S2OPC_SKS_client_session";

#define CLIENT_SKS_SECURITY_MODE OpcUa_MessageSecurityMode_SignAndEncrypt
#define CLIENT_SKS_REQ_SECURITY_POLICY SOPC_SecurityPolicy_Basic256Sha256

#define CLIENT_SKS_USER_POLICY_ID "user"
#define CLIENT_SKS_USERNAME "user1"

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
    else
    {
        status = SOPC_ClientConfigHelper_SetClientKeyPasswordCallback(&SOPC_TestHelper_AskPass_FromEnv);
    }

    if (SOPC_STATUS_OK == status)
    {
        // Set callback necessary to retrieve user password (from environment variable)
        status = SOPC_ClientConfigHelper_SetUserNamePasswordCallback(&SOPC_GetClientUserPassword);
    }

    if (SOPC_STATUS_OK == status)
    {
        size_t nbScConfigs = 0;
        SOPC_SecureConnection_Config** scConfigArray = NULL;
        // TODO: make and use proper copy with cmake
        status = SOPC_ClientConfigHelper_ConfigureFromXML(CLIENT_CONFIG_XML, NULL, &nbScConfigs, &scConfigArray);
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Failed to configure the client from XML");
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
        // TODO: we shall restore use of username but it is not configured on server side
        // status = SOPC_SecureConnectionConfig_SetUserName(secureConnConfig, CLIENT_SKS_USER_POLICY_ID, NULL, NULL);
        status = SOPC_STATUS_OK;
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
                                                SOPC_String* SecurityPolicyUri,
                                                uint32_t CurrentTokenId,
                                                SOPC_ByteString* CurrentKey,
                                                uint32_t NbKeys,
                                                SOPC_ByteString* FutureKeys,
                                                uint32_t TimeToNextKey,
                                                uint32_t KeyLifetime)
{
    int32_t nbElements = 7; // Number of arguments for the SetSecurityKeys method
    SOPC_Variant* arguments = SOPC_Calloc((size_t) nbElements, sizeof(SOPC_Variant));
    if (NULL == arguments)
    {
        return NULL;
    }

    /* Build Method Call */

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    SOPC_Variant* variant;

    // Security Group id
    variant = &arguments[0];
    SOPC_Variant_Initialize(variant);
    variant->BuiltInTypeId = SOPC_String_Id;
    variant->ArrayType = SOPC_VariantArrayType_SingleValue;
    SOPC_String_Initialize(&(variant->Value.String));
    status = SOPC_String_CopyFromCString(&(variant->Value.String), securityGroupId);

    // Security Policy URI
    if (SOPC_STATUS_OK == status)
    {
        variant = &arguments[1];
        SOPC_Variant_Initialize(variant);
        variant->BuiltInTypeId = SOPC_String_Id;
        variant->ArrayType = SOPC_VariantArrayType_SingleValue;
        SOPC_String_Initialize(&(variant->Value.String));
        status = SOPC_String_Copy(&variant->Value.String, SecurityPolicyUri);
    }

    // Current Token Id
    if (SOPC_STATUS_OK == status)
    {
        variant = &arguments[2];
        SOPC_Variant_Initialize(variant);
        variant->BuiltInTypeId = SOPC_UInt32_Id;
        variant->ArrayType = SOPC_VariantArrayType_SingleValue;
        variant->Value.Uint32 = CurrentTokenId;
    }

    // Current Key
    if (SOPC_STATUS_OK == status)
    {
        variant = &arguments[3];
        SOPC_Variant_Initialize(variant);
        variant->BuiltInTypeId = SOPC_ByteString_Id;
        variant->ArrayType = SOPC_VariantArrayType_SingleValue;
        SOPC_ByteString_Initialize(&(variant->Value.Bstring));
        status = SOPC_ByteString_Copy(&(variant->Value.Bstring), CurrentKey);
    }

    // Future Keys
    if (SOPC_STATUS_OK == status)
    {
        status = NbKeys < INT32_MAX ? SOPC_STATUS_OK : SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (SOPC_STATUS_OK == status)
    {
        variant = &arguments[4];
        SOPC_Variant_Initialize(variant);
        variant->BuiltInTypeId = SOPC_ByteString_Id;
        variant->ArrayType = SOPC_VariantArrayType_Array;
        variant->Value.Array.Length = (int32_t) NbKeys;
        if (NbKeys > 0)
        {
            variant->Value.Array.Content.BstringArr = SOPC_Calloc(NbKeys, sizeof(SOPC_ByteString));
            if (NULL != variant->Value.Array.Content.BstringArr)
            {
                for (uint32_t i = 0; i < NbKeys; i++)
                {
                    SOPC_ByteString_Initialize(&(variant->Value.Array.Content.BstringArr[i]));
                    if (SOPC_STATUS_OK == status)
                    {
                        status = SOPC_ByteString_Copy(&(variant->Value.Array.Content.BstringArr[i]), &(FutureKeys[i]));
                    }
                }
            }
            else
            {
                status = SOPC_STATUS_OUT_OF_MEMORY;
            }
        }
    }

    // Time to Next Key
    if (SOPC_STATUS_OK == status)
    {
        variant = &arguments[5];
        SOPC_Variant_Initialize(variant);
        variant->BuiltInTypeId = SOPC_Double_Id;
        variant->ArrayType = SOPC_VariantArrayType_SingleValue;
        variant->Value.Doublev = (double) TimeToNextKey;
    }

    // Key Lifetime
    if (SOPC_STATUS_OK == status)
    {
        variant = &arguments[6];
        SOPC_Variant_Initialize(variant);
        variant->BuiltInTypeId = SOPC_Double_Id;
        variant->ArrayType = SOPC_VariantArrayType_SingleValue;
        variant->Value.Doublev = (double) KeyLifetime;
    }

    OpcUa_CallMethodRequest* call = NULL;
    if (SOPC_STATUS_OK == status)
    {
        call = SOPC_Calloc(1, sizeof(OpcUa_CallMethodRequest));
        status = (NULL == call) ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK;
    }

    if (SOPC_STATUS_OK == status)
    {
        call->encodeableType = &OpcUa_CallMethodRequest_EncodeableType;

        call->ObjectId = (SOPC_NodeId) SOPC_NODEID_NS0_NUMERIC(OpcUaId_PublishSubscribe);
        // We reused the same numeric id but in NS 1 for our SetSecurityKeys method
        call->MethodId = (SOPC_NodeId) SOPC_NODEID_NUMERIC(1, OpcUaId_PublishSubscribeType_SetSecurityKeys);

        call->NoOfInputArguments = (int32_t) nbElements;
        call->InputArguments = arguments;
    }
    else
    {
        // Clear the arguments array if it cannot be configured in a call
        SOPC_Clear_Array(&nbElements, (void**) &arguments, sizeof(*arguments), SOPC_Variant_ClearAux);
        SOPC_Free(arguments);
    }

    /* Build Message */
    OpcUa_CallRequest* req = NULL;
    if (SOPC_STATUS_OK == status)
    {
        req = SOPC_Calloc(1, sizeof(OpcUa_CallRequest));
        status = (NULL == req) ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK;
    }
    if (SOPC_STATUS_OK == status)
    {
        req->encodeableType = &OpcUa_CallRequest_EncodeableType;
        req->NoOfMethodsToCall = 1;
        req->MethodsToCall = call;
    }
    if (SOPC_STATUS_OK != status)
    {
        OpcUa_CallMethodRequest_Clear(call);
        return NULL;
    }

    return req;
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

SOPC_ReturnStatus Client_SetSecurityKeys(SOPC_SecureConnection_Config* config,
                                         const char* securityGroupId,
                                         SOPC_String* SecurityPolicyUri,
                                         uint32_t CurrentTokenId,
                                         SOPC_ByteString* CurrentKey,
                                         uint32_t NbKeys,
                                         SOPC_ByteString* FutureKeys,
                                         uint32_t TimeToNextKey,
                                         uint32_t KeyLifetime)
{
    if (!SOPC_Atomic_Int_Get(&g_Client_Started))
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    if (NULL == config || NULL == securityGroupId || NULL == SecurityPolicyUri || NULL == CurrentKey ||
        NULL == FutureKeys || 0 == TimeToNextKey || 0 == KeyLifetime)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_PUBSUB, "Client_SetSecurityKeys %s for security group id %s",
                          config->scConfig.url, securityGroupId);

    SOPC_ClientConnection* secureConnection = NULL;
    SOPC_ReturnStatus status = SOPC_ClientHelper_Connect(config, &ClientConnectionEvent, &secureConnection);

    if (SOPC_STATUS_OK == status)
    {
        OpcUa_CallResponse* callResp = NULL;
        // Create CallRequest to be sent (deallocated by toolkit)
        OpcUa_CallRequest* callReq = newCallRequest_client(securityGroupId, SecurityPolicyUri, CurrentTokenId,
                                                           CurrentKey, NbKeys, FutureKeys, TimeToNextKey, KeyLifetime);

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
            SOPC_ASSERT(callResp->NoOfResults == 1);
            SOPC_ASSERT(NULL != callResp->Results);
            if (SOPC_IsGoodStatus(callResp->Results[0].StatusCode))
            {
                SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_PUBSUB,
                                      "Client_SetSecurityKeys on %s for security group id %s succeeded",
                                      config->scConfig.url, securityGroupId);
            }
            else
            {
                SOPC_Logger_TraceError(
                    SOPC_LOG_MODULE_PUBSUB,
                    "Client_SetSecurityKeys on %s for security group id %s failed with status 0x%08" PRIX32,
                    config->scConfig.url, securityGroupId, callResp->Results[0].StatusCode);
                status = SOPC_STATUS_NOK;
            }
        }
        else
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Service Call for Client_SetSecurityKeys %s failed",
                                   config->scConfig.url);
        }
        if (NULL != callResp)
        {
            SOPC_EncodeableObject_Delete(callResp->encodeableType, (void**) &callResp);
        }

        SOPC_ClientHelper_Disconnect(&secureConnection);
    }

    return status;
}
