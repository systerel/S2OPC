/*
 * Licensed to Systerel under one or more contributor license
 * agreements. See the NOTICE file distributed with this work
 * for additional information regarding copyright ownership.
 * Systerel licenses this file to you under the Apache
 * License, Version 2.0 (the "License"); you may not use this
 * file except in compliance with the License. You may obtain
 * a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <assert.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h> /* getenv, exit */
#include <string.h>

#include "client.h"

#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"
#include "sopc_atomic.h"
#include "sopc_common_constants.h"
#include "sopc_helper_string.h"
#include "sopc_logger.h"
#include "sopc_mem_alloc.h"
#include "sopc_pki_stack.h"
#include "sopc_sk_builder.h"
#include "sopc_sk_manager.h"
#include "sopc_sk_provider.h"
#include "sopc_sk_scheduler.h"
#include "sopc_time.h"

#include "libs2opc_server.h"
#include "libs2opc_server_config.h"
#include "libs2opc_server_config_custom.h"

#include "embedded/sopc_addspace_loader.h"

static int32_t endpointClosed = 0;

static volatile sig_atomic_t stopServer = 0;

#ifdef WITH_STATIC_SECURITY_DATA
#include "static_security_data.h"
#else
// Default certificate paths

static char* default_server_cert = "./server_public/server_2k_cert.der";
static char* default_key_cert = "./server_private/server_2k_key.pem";

static char* default_trusted_root_issuers[] = {"trusted/cacert.der", NULL};
static char* default_trusted_intermediate_issuers[] = {NULL};

static char* default_issued_certs[] = {NULL};
static char* default_untrusted_root_issuers[] = {NULL};
static char* default_untrusted_intermediate_issuers[] = {NULL};
static char* default_revoked_certs[] = {"revoked/cacrl.der", NULL};

#endif // WITH_STATIC_SECURITY_DATA

#define DEFAULT_APPLICATION_URI "urn:S2OPC:localhost"
#define DEFAULT_PRODUCT_URI "urn:S2OPC:localhost"

/* Define application namespaces: ns=1 and ns=2 */
static char* default_app_namespace_uris[] = {DEFAULT_PRODUCT_URI};
static char* default_locale_ids[] = {"en-US", "fr-FR"};

/* SKS Constants */
// Period to init the ordonnancer. Then the Keys are generated when half-time
#define SKS_ORDONANCER_INIT_MSPERIOD 1000
// Key Lifetime is 10s
#define SKS_KEYLIFETIME 100000
// Number of keys generated randomly
#define SKS_NB_GENERATED_KEYS 5
// Maximum number of Security Keys managed. When the number of keys exceed this limit, only the valid Keys are keept
#define SKS_NB_MAX_KEYS 20

#define SKS_SECURITY_GROUPID "sgid_1"
#define SKS_ARG_MODE_MASTER "master"
#define SKS_ARG_MODE_SLAVE "slave"
#define SKS_ARG_RESTART "--restart"

// Number of SKS Server ( Master + Slaves )
#define nb_sks_server 3
/* SKS server endpoint: 1st is uri of master, others are uris of slave (depending of the main parameters)
   It is use by all Servers to get their own uris.
   Master get the uris of Slaves.
   Slave get the one of Master.
   Slave don't need uris of the others Slaves
*/
static char* sks_server_endpoint_uris[] = {"opc.tcp://localhost:4841", "opc.tcp://localhost:4842",
                                           "opc.tcp://localhost:4843"};
/* SKS server certificate: 1st is cert of master, others are certs of slave */
static char* sks_server_cert_pathes[] = {"./server_public/server_2k_cert.der", "./server_public/server_2k_cert.der",
                                         "./server_public/server_2k_cert.der"};
typedef enum SKS_ServerModeType
{
    SKS_ServerMode_Indet = 0,
    SKS_ServerMode_Master = 1,
    SKS_ServerMode_Slave = 2
} SKS_ServerModeType;

/* SKS Data  */
SOPC_SKManager* skManager = NULL;
SOPC_SKscheduler* skScheduler = NULL;
SKS_ServerModeType sksServerMode = SKS_ServerMode_Indet;
char* sksStrIndex = NULL;
uint8_t sksServerIndex = 0;
bool sksRestart = false;

/* NodeIds of method for Call Method Service */
SOPC_NodeId* methodIds[1] = {NULL};
uint32_t nbMethodIds = 0;

/*
 * Management of Ctrl-C to stop the server (callback on stop signal)
 */
static void Test_StopSignal(int sig)
{
    /* avoid unused parameter compiler warning */
    (void) sig;

    /*
     * Signal steps:
     * - 1st signal: activate server shutdown phase of OPC UA server (will stop after <SHUTDOWN_PHASE_IN_SECONDS>s)
     * - 2nd signal: activate ASAP server shutdown gracefully closing all connections and clearing context
     * - 3rd signal: abrupt exit with error code '1'
     */
    if (stopServer > 1)
    {
        exit(1);
    }
    else
    {
        stopServer++;
    }
}

static void Test_ServerStopped_Fct(SOPC_ReturnStatus status)
{
    printf("<Test_SKS_Server: server stopped with status %d\n", status);
    SOPC_Atomic_Int_Set(&endpointClosed, 1);
}

/*---------------------------------------------------------------------------
 *             PubSub Security Key Service specific configuration
 *---------------------------------------------------------------------------*/
static SOPC_StatusCode SOPC_Method_Func_PublishSubscribe_GetSecurityKeys(const SOPC_CallContext* callContextPtr,
                                                                         const SOPC_NodeId* objectId,
                                                                         uint32_t nbInputArgs,
                                                                         const SOPC_Variant* inputArgs,
                                                                         uint32_t* nbOutputArgs,
                                                                         SOPC_Variant** outputArgs,
                                                                         void* param);

static void SKS_ComEvent_FctClient(SOPC_App_Com_Event event, uint32_t idOrStatus, void* param, uintptr_t appContext)
{
    /* avoid unused parameter compiler warning */
    (void) idOrStatus;
    (void) appContext;
    bool debug = false;

    switch (event)
    {
        /* Client application events */
    case SE_SESSION_ACTIVATION_FAILURE:
        if (debug)
        {
            printf(">>Client debug : SE_SESSION_ACTIVATION_FAILURE RECEIVED\n");
            printf(">>Client debug : appContext: %" PRIuPTR "\n", appContext);
        }
        if (0 != appContext && appContext == Client_SessionContext)
        {
            SOPC_Atomic_Int_Set(&scState, (int32_t) SESSION_CONN_FAILED);
        }
        else
        {
            assert(false && ">>Client : bad app context");
        }
        break;
    case SE_ACTIVATED_SESSION:
        SOPC_Atomic_Int_Set((int32_t*) &session, (int32_t) idOrStatus);
        if (debug)
        {
            printf(">>Client debug : SE_ACTIVATED_SESSION RECEIVED\n");
        }
        SOPC_Atomic_Int_Set(&scState, (int32_t) SESSION_CONN_CONNECTED);
        break;
    case SE_SESSION_REACTIVATING:
        if (debug)
        {
            printf(">>Client debug : SE_SESSION_REACTIVATING RECEIVED\n");
        }
        break;
    case SE_RCV_SESSION_RESPONSE:
        if (debug)
        {
            printf(">>Client debug : SE_RCV_SESSION_RESPONSE RECEIVED\n");
        }
        Client_Treat_Session_Response(param, appContext);
        break;
    case SE_CLOSED_SESSION:
        if (debug == true)
        {
            printf(">>Client debug : SE_CLOSED_SESSION RECEIVED\n");
        }
        break;

    case SE_RCV_DISCOVERY_RESPONSE:
        if (debug == true)
        {
            printf(">>Client debug : SE_RCV_DISCOVERY_RESPONSE RECEIVED\n");
        }
        break;

    case SE_SND_REQUEST_FAILED:
        if (debug == true)
        {
            printf(">>Client debug : SE_SND_REQUEST_FAILED RECEIVED\n");
        }
        SOPC_Atomic_Int_Add(&sendFailures, 1);
        break;

    /* SERVER EVENT */
    default:
        printf("<Test_SKS_Server: unexpected endpoint event %d : NOK\n", event);
    }
}

static SOPC_StatusCode Server_SKS_CreateMasterBuilder(SOPC_SKBuilder** builder, SOPC_SKProvider** provider)

{
    if (NULL == builder || NULL == provider)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    *builder = NULL;
    *provider = NULL;
    SOPC_StatusCode status = SOPC_STATUS_OK;

    /* Init SK Provider : Create Random Keys */
    *provider = SOPC_SKProvider_RandomPubSub_Create(SKS_NB_GENERATED_KEYS);
    if (NULL == *provider)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }

    /* Init SK Builder : adds Keys to Manager and removes obsolete Keys when maximum size is reached */
    SOPC_SKBuilder* skbAppend = NULL;
    if (SOPC_STATUS_OK == status)
    {
        skbAppend = SOPC_SKBuilder_Append_Create();
        if (NULL == skbAppend)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        *builder = SOPC_SKBuilder_Truncate_Create(skbAppend, SKS_NB_MAX_KEYS);
        if (NULL == *builder)
        {
            SOPC_SKBuilder_Clear(skbAppend);
            SOPC_Free(skbAppend);
            skbAppend = NULL;
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    if (SOPC_STATUS_OK != status)
    {
        if (NULL != *provider)
        {
            SOPC_SKProvider_Clear(*provider);
            SOPC_Free(*provider);
            *provider = NULL;
        }

        if (NULL != *builder)
        {
            SOPC_SKBuilder_Clear(*builder);
            SOPC_Free(*builder);
            *builder = NULL;
        }
    }

    return status;
}

static SOPC_StatusCode Server_SKS_CreateSlaveBuilder(uint32_t SecureChannel_Id,
                                                     SOPC_SKBuilder** builder,
                                                     SOPC_SKProvider** provider)
{
    if (NULL == builder || NULL == provider)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    *builder = NULL;
    *provider = NULL;
    SOPC_StatusCode status = SOPC_STATUS_OK;

    /* Create a SK Builder which replace all Keys of a SK Manager */

    // Create a SK Provider which get Keys from a GetSecurityKeys request
    *provider = Client_Provider_BySKS_Create(SecureChannel_Id);
    if (NULL == *provider)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }

    if (SOPC_STATUS_OK == status)
    {
        // Create Builder to replace all Keys
        *builder = SOPC_SKBuilder_Setter_Create();
        if (NULL == *builder)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    if (SOPC_STATUS_OK != status)
    {
        if (NULL != *provider)
        {
            SOPC_SKProvider_Clear(*provider);
            SOPC_Free(*provider);
            *provider = NULL;
        }

        if (NULL != *builder)
        {
            SOPC_SKBuilder_Clear(*builder);
            SOPC_Free(*builder);
            *builder = NULL;
        }
    }

    return status;
}

static SOPC_StatusCode Server_SKManager_Init(SOPC_SKManager* manager)
{
    if (SKS_ServerMode_Master != sksServerMode || !sksRestart)
    {
        return SOPC_STATUS_OK;
    }

    if (NULL == manager)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_StatusCode status = SOPC_STATUS_OK;
    SOPC_SKBuilder* builder = NULL;
    SOPC_SKProvider* provider = NULL;
    bool isInit = false;
    uint32_t SecureChannel_Id = 0;

    uint64_t i = 0;
    // Stop if add SC failed or Keys are found
    for (i = 1; i < nb_sks_server && SOPC_STATUS_OK == status && !isInit; i++)
    {
        SOPC_SerializedCertificate* server_cert = NULL;
        status = SOPC_KeyManager_SerializedCertificate_CreateFromFile(sks_server_cert_pathes[i], &server_cert);
        if (SOPC_STATUS_OK == status)
        {
            SecureChannel_Id = Client_AddSecureChannelconfig(sks_server_endpoint_uris[i], server_cert);
            if (0 == SecureChannel_Id)
            {
                status = SOPC_STATUS_NOK;
            }
            SOPC_KeyManager_SerializedCertificate_Delete(server_cert);
        }
        else
        {
            printf("<Security Key Service Error: Cannot create Server Certificate\n");
        }

        if (SOPC_STATUS_OK == status)
        {
            status = Server_SKS_CreateSlaveBuilder(SecureChannel_Id, &builder, &provider);
        }

        if (SOPC_STATUS_OK == status)
        {
            SOPC_SKBuilder_Update(builder, provider, manager);
            uint32_t size = SOPC_SKManager_Size(manager);
            isInit = size > 0;
        }

        SOPC_SKBuilder_Clear(builder);
        SOPC_Free(builder);
        SOPC_SKProvider_Clear(provider);
        SOPC_Free(provider);
    }
    if (isInit)
    {
        assert(0 < i);
        assert(i <= nb_sks_server);
        printf("<Security Keys Service : Master retrieve Keys from Slave %s\n", sks_server_endpoint_uris[i - i]);
    }

    return status;
}

static SOPC_StatusCode Server_SKS_Start(void)
{
    SOPC_StatusCode status = SOPC_STATUS_OK;

    Client_Start();

    /* Init SK Manager */
    skManager = SOPC_SKManager_Create();
    if (NULL == skManager)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }

    if (SOPC_STATUS_OK == status)
    {
        status = Server_SKManager_Init(skManager);
    }

    // Set KeyLifeTime only if SK Manager didn't get Keys from Slave
    if (SOPC_STATUS_OK == status && 0 == SOPC_SKManager_Size(skManager))
    {
        status = SOPC_SKManager_SetKeyLifetime(skManager, SKS_KEYLIFETIME);
        if (SOPC_STATUS_OK == status)
        {
            SOPC_String policy;
            SOPC_String_Initialize(&policy);
            SOPC_String_CopyFromCString(&policy, SOPC_SecurityPolicy_PubSub_Aes256_URI);
            status = SOPC_SKManager_SetSecurityPolicyUri(skManager, &policy);
            SOPC_String_Clear(&policy);
        }
    }

    /* Init SK Scheduler */
    if (SOPC_STATUS_OK == status)
    {
        skScheduler = SOPC_SKscheduler_Create();
        if (NULL == skScheduler)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    /* Init SK Builder and SK Provider */
    SOPC_SKBuilder* skBuilder = NULL;
    SOPC_SKProvider* skProvider = NULL;
    uint32_t SecureChannel_Id = 0;
    /* Init SK Builder : adds Keys to Manager and removes obsolete Keys when maximum size is reached */
    if (SOPC_STATUS_OK == status)
    {
        SOPC_SerializedCertificate* server_cert = NULL;
        switch (sksServerMode)
        {
        case SKS_ServerMode_Master:
            status = Server_SKS_CreateMasterBuilder(&skBuilder, &skProvider);
            break;
        case SKS_ServerMode_Slave:
            // Configure Client module with Master uri

            status = SOPC_KeyManager_SerializedCertificate_CreateFromFile(sks_server_cert_pathes[0], &server_cert);
            if (SOPC_STATUS_OK == status)
            {
                SecureChannel_Id = Client_AddSecureChannelconfig(sks_server_endpoint_uris[0], server_cert);
                if (0 == SecureChannel_Id)
                {
                    status = SOPC_STATUS_NOK;
                }
                SOPC_KeyManager_SerializedCertificate_Delete(server_cert);
            }
            else
            {
                printf("<Security Key Service Error: Cannot create Server Certificate\n");
            }

            if (SOPC_STATUS_OK == status)
            {
                status = Server_SKS_CreateSlaveBuilder(SecureChannel_Id, &skBuilder, &skProvider);
            }
            else
            {
                printf("<Security Key Service Error: Slave Server cannot configure channel to Master Server\n");
            }
            break;
        default:
            // should not happen
            status = SOPC_STATUS_NOK;
            break;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        /* Init the task with 1s */
        status = SOPC_SKscheduler_AddTask(skScheduler, skBuilder, skProvider, skManager, SKS_ORDONANCER_INIT_MSPERIOD);
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_SKscheduler_Start(skScheduler);
    }

    if (SOPC_STATUS_OK == status)
    {
        printf("<Security Keys Service: Started\n");
    }
    else
    {
        printf("<Security Keys Service Error: Start failed\n");
    }

    return status;
}

/*---------------------------------------------------------------------------
 *                          Callbacks definition
 *---------------------------------------------------------------------------*/

/*
 * Implementation of GetSecurityKeys method for PubSub Security Key Service
 */
static SOPC_StatusCode SOPC_Method_Func_PublishSubscribe_GetSecurityKeys(const SOPC_CallContext* callContextPtr,
                                                                         const SOPC_NodeId* objectId,
                                                                         uint32_t nbInputArgs,
                                                                         const SOPC_Variant* inputArgs,
                                                                         uint32_t* nbOutputArgs,
                                                                         SOPC_Variant** outputArgs,
                                                                         void* param)
{
    (void) objectId; /* Should be "i=14443"*/
    (void) param;    /* Should be NULL */

    /* Check Call Context */

    // SecureChannel shall use encryption to keep the provided keys secret
    OpcUa_MessageSecurityMode msm = SOPC_CallContext_GetSecurityMode(callContextPtr);
    if (OpcUa_MessageSecurityMode_SignAndEncrypt != msm)
    {
        return OpcUa_BadSecurityModeInsufficient;
    }

    // User shall be authorized to call the GetSecurityKeys method
    const SOPC_User* user = SOPC_CallContext_GetUser(callContextPtr);
    /* Check if the user is authorized to call the method for this Security Group */
    if (SOPC_User_IsUsername(user))
    { /* Type of user should be username */
        const SOPC_String* username = SOPC_User_GetUsername(user);
        if (0 != strcmp("user1", SOPC_String_GetRawCString(username)))
        {
            /* Only user1 is allowed to call getSecurityKeys() */
            return OpcUa_BadUserAccessDenied;
        }
    }
    else
    {
        return OpcUa_BadUserAccessDenied;
    }

    /* Check Input Object */

    if (3 != nbInputArgs || NULL == inputArgs)
    {
        /* Should not happen if method is well defined in address space */
        return OpcUa_BadInternalError;
    }

    /* Check Security Group */
    if (SOPC_VariantArrayType_SingleValue != inputArgs[0].ArrayType)
    {
        /* Should not happen if method is well defined in address space */
        return OpcUa_BadInternalError;
    }

    if (0 != strcmp(SKS_SECURITY_GROUPID, SOPC_String_GetRawCString(&inputArgs[0].Value.String)))
    {
        return OpcUa_BadNotFound;
    }

    if (SOPC_UInt32_Id != inputArgs[1].BuiltInTypeId || SOPC_VariantArrayType_SingleValue != inputArgs[1].ArrayType)
    {
        return OpcUa_BadInternalError;
    }

    uint32_t requestedStartingTokenId = inputArgs[1].Value.Uint32;

    if (SOPC_UInt32_Id != inputArgs[2].BuiltInTypeId || SOPC_VariantArrayType_SingleValue != inputArgs[2].ArrayType)
    {
        return OpcUa_BadInternalError;
    }

    uint32_t requestedNbKeys = inputArgs[2].Value.Uint32;

    *nbOutputArgs = 5;
    *outputArgs = SOPC_Calloc(5, sizeof(SOPC_Variant));
    SOPC_StatusCode status = SOPC_GoodGenericStatus;
    if (NULL == *outputArgs)
    {
        return OpcUa_BadOutOfMemory;
    }

    SOPC_Variant* variant;
    for (uint32_t i = 0; i < 5; i++)
    {
        variant = &((*outputArgs)[i]);
        SOPC_Variant_Initialize(variant);
    }

    SOPC_String* SecurityPolicyUri = NULL;
    uint32_t FirstTokenId = 0;
    SOPC_ByteString* Keys = NULL;
    uint32_t NbToken = 0;
    uint32_t TimeToNextKey = 0;
    uint32_t KeyLifetime = 0;

    if (SOPC_GoodGenericStatus == status)
    {
        status = SOPC_SKManager_GetKeys(skManager, requestedStartingTokenId, requestedNbKeys, &SecurityPolicyUri,
                                        &FirstTokenId, &Keys, &NbToken, &TimeToNextKey, &KeyLifetime);
        if (SOPC_GoodGenericStatus != status)
        {
            printf("<Security Key Service: Error in SK Manager when get keys\n");
        }
    }

    bool keysValid = (NULL != Keys && 0 < NbToken && INT32_MAX >= NbToken);

    if (!keysValid)
    {
        printf("<Security Key Service Error: Retrieved Keys are not valid\n");
    }
    if (0 == FirstTokenId && keysValid)
    {
        printf("<Security Key Service Error: First Token id is not valid\n");
    }
    if (0 == TimeToNextKey && keysValid)
    {
        printf("<Security Key Service Error: TimeToNextKey is not valid\n");
    }
    if (0 == KeyLifetime && keysValid)
    {
        printf("<Security Key Service Error: KeyLifetime is not valid\n");
    }

    if (SOPC_GoodGenericStatus == status && keysValid)
    {
        /* SecurityPolicyUri */
        variant = &((*outputArgs)[0]);
        variant->BuiltInTypeId = SOPC_String_Id;
        variant->ArrayType = SOPC_VariantArrayType_SingleValue;
        SOPC_String_Copy(&variant->Value.String, SecurityPolicyUri);
    }

    if (SOPC_GoodGenericStatus == status && keysValid)
    {
        /* FirstTokenId */
        variant = &((*outputArgs)[1]);
        variant->BuiltInTypeId = SOPC_UInt32_Id; /* IntegerId */
        variant->ArrayType = SOPC_VariantArrayType_SingleValue;
        variant->Value.Uint32 = FirstTokenId;
    }

    if (SOPC_GoodGenericStatus == status && keysValid)
    {
        /* Keys */
        variant = &((*outputArgs)[2]);
        variant->BuiltInTypeId = SOPC_ByteString_Id;
        variant->ArrayType = SOPC_VariantArrayType_Array;
        // SigningKey + EncryptingKey + KeyNonce
        variant->Value.Array.Content.BstringArr = SOPC_Calloc(NbToken, sizeof(SOPC_ByteString));
        if (NULL == *outputArgs)
        {
            status = OpcUa_BadOutOfMemory;
        }
        else
        {
            for (uint32_t i = 0; i < NbToken && SOPC_GoodGenericStatus == status; i++)
            {
                SOPC_ByteString_Clear(&variant->Value.Array.Content.BstringArr[i]);
                status = SOPC_ByteString_Copy(&variant->Value.Array.Content.BstringArr[i], &Keys[i]);
            }
        }
        if (SOPC_GoodGenericStatus == status)
        {
            variant->Value.Array.Length = (int32_t) NbToken;
        }
        else
        {
            printf("<Security Key Service Error: Cannot save Keys\n");
        }
    }

    if (SOPC_GoodGenericStatus == status && keysValid)
    {
        /* TimeToNextKey */
        variant = &((*outputArgs)[3]);
        variant->BuiltInTypeId = SOPC_Double_Id; /* Duration */
        variant->ArrayType = SOPC_VariantArrayType_SingleValue;
        variant->Value.Doublev = (double) TimeToNextKey;
    }

    if (SOPC_GoodGenericStatus == status && keysValid)
    {
        /* KeyLifetime */
        variant = &((*outputArgs)[4]);
        variant->BuiltInTypeId = SOPC_Double_Id; /* Duration */
        variant->ArrayType = SOPC_VariantArrayType_SingleValue;
        variant->Value.Doublev = (double) KeyLifetime;
    }

    /* If bad status, delete output */
    if (SOPC_GoodGenericStatus != status)
    {
        for (uint32_t i = 0; i < 5; i++)
        {
            variant = &((*outputArgs)[i]);
            SOPC_Variant_Clear(variant);
        }
        SOPC_Free(*outputArgs);
        *outputArgs = NULL;
        *nbOutputArgs = 0;
    }

    /* Delete Keys */
    for (uint32_t i = 0; i < NbToken; i++)
    {
        SOPC_ByteString_Clear(&Keys[i]);
    }
    SOPC_Free(Keys);
    SOPC_String_Clear(SecurityPolicyUri);
    SOPC_Free(SecurityPolicyUri);

    return status;
}

/*
 * Server callback definition used for address space modification notification
 */
static void Demo_WriteNotificationCallback(const SOPC_CallContext* callContextPtr,
                                           OpcUa_WriteValue* writeValue,
                                           SOPC_StatusCode writeStatus)
{
    const SOPC_User* user = SOPC_CallContext_GetUser(callContextPtr);
    const char* writeSuccess = (SOPC_STATUS_OK == writeStatus ? "success" : "failure");
    char* sNodeId = SOPC_NodeId_ToCString(&writeValue->NodeId);
    SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER, "Write notification (%s) on node '%s' by user '%s'",
                           writeSuccess, sNodeId, SOPC_User_ToCString(user));
    SOPC_Free(sNodeId);
}

/*---------------------------------------------------------------------------
 *                          Server initialization
 *---------------------------------------------------------------------------*/

/* Set the log path and create (or keep existing) directory path built on executable path */
static char* Server_ConfigLogPath(void)
{
    const char* logDirName = "test_sks_server";
    char* underscore = "_";
    char* suffix = sksStrIndex;
    char* logDirPath = NULL;

    size_t logDirPathSize = 2 + strlen(logDirName) + strlen(underscore) + strlen(suffix) +
                            7; // "./" + logDirName + _ + test_name + _logs/ + '\0'
    if (logDirPathSize < 200)
    {
        logDirPath = SOPC_Malloc(logDirPathSize * sizeof(char));
    }
    if (NULL != logDirPath && (int) (logDirPathSize - 1) != snprintf(logDirPath, logDirPathSize, "./%s%s%s_logs/",
                                                                     logDirName, underscore, suffix))
    {
        SOPC_Free(logDirPath);
        logDirPath = NULL;
    }

    return logDirPath;
}

static SOPC_ReturnStatus Server_Initialize(const char* logDirPath)
{
    // Due to issue in certification tool for View Basic 005/015/020 number of chunks shall be the same and at least 12
    SOPC_Common_EncodingConstants encConf = SOPC_Common_GetDefaultEncodingConstants();
    encConf.receive_max_nb_chunks = 12;
    encConf.send_max_nb_chunks = 12;
    bool res = SOPC_Common_SetEncodingConstants(encConf);
    assert(res);

    // Get default log config and set the custom path
    SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
    if (NULL != logDirPath)
    {
        logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = logDirPath;
    }
    else
    {
        logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = "./toolkit_test_server_logs/";
    }
    logConfiguration.logLevel = SOPC_LOG_LEVEL_DEBUG;
    // Initialize the toolkit library and define the log configuration
    SOPC_ReturnStatus status = SOPC_Helper_Initialize(&logConfiguration);
    if (SOPC_STATUS_OK != status)
    {
        printf("<Test_SKS_Server: Failed initializing\n");
    }
    else
    {
        printf("<Test_SKS_Server: initialized\n");
    }
    return status;
}

/*---------------------------------------------------------------------------
 *                             Server configuration
 *---------------------------------------------------------------------------*/

/*----------------------------------------------------
 * Application description and endpoint configuration:
 *---------------------------------------------------*/

/*
 * Configure the cryptographic parameters of the endpoint:
 * - Server certificate and key
 * - Public Key Infrastructure: using a single certificate as Certificate Authority or Trusted Certificate
 */
static SOPC_ReturnStatus Server_SetDefaultCryptographicConfig(void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    SOPC_PKIProvider* pkiProvider = NULL;

#ifdef WITH_STATIC_SECURITY_DATA
    SOPC_SerializedCertificate* serializedCAcert = NULL;
    SOPC_CRLList* serializedCAcrl = NULL;

    /* Load client/server certificates and server key from C source files (no filesystem needed) */
    status = SOPC_HelperConfigServer_SetKeyCertPairFromBytes(sizeof(server_2k_cert), server_2k_cert,
                                                             sizeof(server_2k_key), server_2k_key);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_SerializedCertificate_CreateFromDER(cacert, sizeof(cacert), &serializedCAcert);
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_CRL_CreateOrAddFromDER(cacrl, sizeof(cacrl), &serializedCAcrl);
    }

    /* Create the PKI (Public Key Infrastructure) provider */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_PKIProviderStack_Create(serializedCAcert, serializedCAcrl, &pkiProvider);
    }
    SOPC_KeyManager_SerializedCertificate_Delete(serializedCAcert);
#else // WITH_STATIC_SECURITY_DATA == false
    /* Load client/server certificates and server key from files */
    status = SOPC_HelperConfigServer_SetKeyCertPairFromPath(default_server_cert, default_key_cert);

    /* Create the PKI (Public Key Infrastructure) provider */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_PKIProviderStack_CreateFromPaths(
            default_trusted_root_issuers, default_trusted_intermediate_issuers, default_untrusted_root_issuers,
            default_untrusted_intermediate_issuers, default_issued_certs, default_revoked_certs, &pkiProvider);
    }
#endif

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_HelperConfigServer_SetPKIprovider(pkiProvider);
    }

    if (SOPC_STATUS_OK != status)
    {
        printf("<Test_SKS_Server: Failed loading certificates and key (check paths are valid)\n");
    }
    else
    {
        printf("<Test_SKS_Server: Certificates and key loaded\n");
    }

    return status;
}

/*
 * Default server configuration loader (without XML configuration)
 */
static SOPC_ReturnStatus Server_SetDefaultConfiguration(void)
{
    // Set namespaces
    SOPC_ReturnStatus status = SOPC_HelperConfigServer_SetNamespaces(sizeof(default_app_namespace_uris) / sizeof(char*),
                                                                     default_app_namespace_uris);
    // Set locale ids
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_HelperConfigServer_SetLocaleIds(sizeof(default_locale_ids) / sizeof(char*), default_locale_ids);
    }

    // Set application description of server to be returned by discovery services (GetEndpoints, FindServers)
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_HelperConfigServer_SetApplicationDescription(DEFAULT_APPLICATION_URI, DEFAULT_PRODUCT_URI,
                                                                   "S2OPC toolkit server example", "en-US",
                                                                   OpcUa_ApplicationType_Server);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_HelperConfigServer_AddApplicationNameLocale("S2OPC toolkit: exemple de serveur", "fr-FR");
    }

    /*
     * Create new endpoint in server
     */
    SOPC_Endpoint_Config* ep = NULL;
    if (SOPC_STATUS_OK == status)
    {
        ep = SOPC_HelperConfigServer_CreateEndpoint(sks_server_endpoint_uris[sksServerIndex], true);
        status = NULL == ep ? SOPC_STATUS_OUT_OF_MEMORY : status;
    }

    /*
     * Define the certificates, security policies, security modes and user token policies supported by endpoint
     */
    SOPC_SecurityPolicy* sp;
    if (SOPC_STATUS_OK == status)
    {
        /*
         * Security policy is Basic256Sha256 with anonymous and username (non encrypted) authentication allowed
         */
        sp = SOPC_EndpointConfig_AddSecurityConfig(ep, SOPC_SecurityPolicy_Basic256Sha256);
        if (NULL == sp)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
        else
        {
            status = SOPC_SecurityConfig_SetSecurityModes(
                sp, SOPC_SecurityModeMask_Sign | SOPC_SecurityModeMask_SignAndEncrypt);

            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_SecurityConfig_AddUserTokenPolicy(sp, &SOPC_UserTokenPolicy_Anonymous);
            }
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_SecurityConfig_AddUserTokenPolicy(sp, &SOPC_UserTokenPolicy_UserName_NoneSecurityPolicy);
            }
        }
    }

    /**
     * Define server certificate and PKI provider
     */
    if (SOPC_STATUS_OK == status)
    {
        status = Server_SetDefaultCryptographicConfig();
    }

    return status;
}

/*----------------------------------------
 * Users authentication and authorization:
 *----------------------------------------*/

/* The toolkit demo sks server shall accept:
 *  - anonymous users
 *  - user1:password
 * Then it shall accept username:password, but return "access denied".
 * Otherwise it shall be "identity token rejected".
 */
static SOPC_ReturnStatus authentication_test_sks(SOPC_UserAuthentication_Manager* authn,
                                                 const SOPC_ExtensionObject* token,
                                                 SOPC_UserAuthentication_Status* authenticated)
{
    /* avoid unused parameter compiler warning */
    (void) (authn);

    assert(NULL != token && NULL != authenticated);

    *authenticated = SOPC_USER_AUTHENTICATION_REJECTED_TOKEN;
    assert(SOPC_ExtObjBodyEncoding_Object == token->Encoding);
    if (&OpcUa_UserNameIdentityToken_EncodeableType == token->Body.Object.ObjType)
    {
        OpcUa_UserNameIdentityToken* userToken = token->Body.Object.Value;
        SOPC_String* username = &userToken->UserName;
        if (strcmp(SOPC_String_GetRawCString(username), "user1") == 0)
        {
            SOPC_ByteString* pwd = &userToken->Password;
            if (pwd->Length == strlen("password") && memcmp(pwd->Data, "password", strlen("password")) == 0)
            {
                *authenticated = SOPC_USER_AUTHENTICATION_OK;
            }
        }
    }

    return SOPC_STATUS_OK;
}

static SOPC_ReturnStatus authorization_test_sks(SOPC_UserAuthorization_Manager* authorizationManager,
                                                SOPC_UserAuthorization_OperationType operationType,
                                                const SOPC_NodeId* nodeId,
                                                uint32_t attributeId,
                                                const SOPC_User* pUser,
                                                bool* pbOperationAuthorized)
{
    // We use global user rights only and do not check user rights for a specific node
    (void) (nodeId);
    (void) (attributeId);
    assert(NULL != authorizationManager);
    assert(NULL != pbOperationAuthorized);

    *pbOperationAuthorized = false;

    bool read = true; // Authorize
    bool write = false;
    bool exec = false;

    if (SOPC_User_IsUsername(pUser))
    {
        // Authorize some users to execute methods
        const SOPC_String* username = SOPC_User_GetUsername(pUser);
        if (strcmp(SOPC_String_GetRawCString(username), "user1") == 0)
        {
            read = true;
            exec = true;
        }
    }
    else if (SOPC_User_IsAnonymous(pUser))
    {
        // Anonymous has read only rights
        read = true;
    }

    switch (operationType)
    {
    case SOPC_USER_AUTHORIZATION_OPERATION_READ:
        *pbOperationAuthorized = read;
        break;
    case SOPC_USER_AUTHORIZATION_OPERATION_WRITE:
        *pbOperationAuthorized = write;
        break;
    case SOPC_USER_AUTHORIZATION_OPERATION_EXECUTABLE:
        *pbOperationAuthorized = exec;
        break;
    default:
        assert(false && "Unknown operation type.");
        break;
    }

    return SOPC_STATUS_OK;
}

static const SOPC_UserAuthentication_Functions sks_authentication_functions = {
    .pFuncFree = (SOPC_UserAuthentication_Free_Func) SOPC_Free,
    .pFuncValidateUserIdentity = authentication_test_sks};

static const SOPC_UserAuthorization_Functions sks_authorization_functions = {
    .pFuncFree = (SOPC_UserAuthorization_Free_Func) SOPC_Free,
    .pFuncAuthorizeOperation = authorization_test_sks};

static SOPC_ReturnStatus Server_SetDefaultUserManagementConfig(void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    /* Create authentication and authorization managers which defines users and associated access levels. */
    SOPC_UserAuthorization_Manager* authorizationManager = SOPC_Calloc(1, sizeof(SOPC_UserAuthorization_Manager));
    SOPC_UserAuthentication_Manager* authenticationManager = SOPC_Calloc(1, sizeof(SOPC_UserAuthentication_Manager));
    if (NULL == authenticationManager || NULL == authorizationManager)
    {
        SOPC_UserAuthorization_FreeManager(&authorizationManager);
        SOPC_UserAuthentication_FreeManager(&authenticationManager);
        status = SOPC_STATUS_OUT_OF_MEMORY;
        printf("<Test_SKS_Server: Failed to create the user manager\n");
    }

    if (SOPC_STATUS_OK == status)
    {
        /* Set a users authentication and authorization functions */
        authenticationManager->pFunctions = &sks_authentication_functions;
        authorizationManager->pFunctions = &sks_authorization_functions;
        SOPC_HelperConfigServer_SetUserAuthenticationManager(authenticationManager);
        SOPC_HelperConfigServer_SetUserAuthorizationManager(authorizationManager);
    }

    return status;
}

/*------------------------------
 * Address space configuration :
 *------------------------------*/

static SOPC_ReturnStatus Server_SetDefaultAddressSpace(void)
{
    /* Load embedded default server address space:
     * Use the embedded address space (already defined as C code) loader.
     * The address space C structure shall have been generated prior to compilation.
     * This should be done using the script ./scripts/generate-s2opc-address-space.py
     */

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    SOPC_AddressSpace* addSpace = SOPC_Embedded_AddressSpace_Load();
    status = (NULL != addSpace) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_HelperConfigServer_SetAddressSpace(addSpace);
    }

    if (SOPC_STATUS_OK != status)
    {
        printf("<Test_SKS_Server: Failed to configure the @ space\n");
    }
    else
    {
        printf("<Test_SKS_Server: @ space configured\n");
    }

    return status;
}

/*-------------------------
 * Method call management :
 *-------------------------*/

static SOPC_ReturnStatus Server_InitSKScallMethodService(void)
{
    SOPC_NodeId* methodId;
    SOPC_MethodCallFunc_Ptr methodFunc;
    /* Create and define the method call manager the server will use*/
    SOPC_MethodCallManager* mcm = SOPC_MethodCallManager_Create();
    SOPC_StatusCode status = (NULL != mcm) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_HelperConfigServer_SetMethodCallManager(mcm);
    }

    /* Add methods implementation in the method call manager used */
    if (SOPC_STATUS_OK == status)
    {
        // getSecurityKeys method node
        methodId = SOPC_NodeId_FromCString("i=15215", 7);
        if (NULL != methodId)
        {
            methodIds[nbMethodIds] = methodId;
            nbMethodIds++;
            methodFunc = &SOPC_Method_Func_PublishSubscribe_GetSecurityKeys;
            status = SOPC_MethodCallManager_AddMethod(mcm, methodId, methodFunc, NULL, NULL);
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
    }

    return status;
}

/*---------------------------------------------------------------------------
 *                             Server configuration
 *---------------------------------------------------------------------------*/

static SOPC_ReturnStatus Server_LoadServerConfiguration(void)
{
    /* Retrieve XML configuration file path from environment variables TEST_SERVER_XML_CONFIG,
     * TEST_SERVER_XML_ADDRESS_SPACE and TEST_USERS_XML_CONFIG.
     *
     * In case of success returns the file path otherwise load default configuration.
     */

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    const char* xml_server_config_path = getenv("TEST_SERVER_XML_CONFIG");
    const char* xml_address_space_config_path = getenv("TEST_SERVER_XML_ADDRESS_SPACE");
    const char* xml_users_config_path = getenv("TEST_USERS_XML_CONFIG");

    if (NULL != xml_server_config_path || NULL != xml_address_space_config_path || NULL != xml_users_config_path)
    {
#ifdef WITH_EXPAT
        status = SOPC_HelperConfigServer_ConfigureFromXML(xml_server_config_path, xml_address_space_config_path,
                                                          xml_users_config_path, NULL);
#else
        printf(
            "Error: an XML server configuration file path provided whereas XML library not available (Expat).\n"
            "Do not define environment variables TEST_SERVER_XML_CONFIG, TEST_SERVER_XML_ADDRESS_SPACE and "
            "TEST_USERS_XML_CONFIG.\n"
            "Or compile with XML library available.\n");
        status = SOPC_STATUS_INVALID_PARAMETERS;
#endif
    }

    if (SOPC_STATUS_OK == status && NULL == xml_server_config_path)
    {
        status = Server_SetDefaultConfiguration();
    }

    if (SOPC_STATUS_OK == status && NULL == xml_address_space_config_path)
    {
        status = Server_SetDefaultAddressSpace();
    }

    if (SOPC_STATUS_OK == status && NULL == xml_users_config_path)
    {
        status = Server_SetDefaultUserManagementConfig();
    }

    return status;
}

/*---------------------------------------------------------------------------
 *                         SKS server mode configuration (Slave / Master)
 *---------------------------------------------------------------------------*/
static SOPC_ReturnStatus Config_ConfigureSKSServerMode(int argc, char* argv[])
{
    sksServerIndex = 0;
    sksServerMode = SKS_ServerMode_Indet;

    if (argc < 2)
    {
        /* Server Mode is mandatory */
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    char* serverMode = argv[1];
    if (0 == strcmp(SKS_ARG_MODE_MASTER, serverMode) && argc > 1)
    {
        /* Master address is at first index */
        sksServerMode = SKS_ServerMode_Master;
        sksServerIndex = 0;
        sksStrIndex = "0";
    }
    else if (0 == strcmp(SKS_ARG_MODE_SLAVE, serverMode) && argc > 2)
    {
        sksServerMode = SKS_ServerMode_Slave;

        /* Slave address index is given as second parameter */

        sksStrIndex = argv[2];
        uint8_t val = 0;
        status = SOPC_strtouint8_t(sksStrIndex, &val, 10, '\0');

        /* Check that all characters are valid and in array bounds */
        if (SOPC_STATUS_OK == status && nb_sks_server > val)
        {
            sksServerIndex = val;
        }
        else
        {
            /* Index of slave is not valid */
            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
    }
    else
    {
        /* Server Mode is not valid or Index of slave is missing */
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }

    // Only Master has a restart mode. Find SKS restart keyword in last parameter
    sksRestart = (sksServerMode == SKS_ServerMode_Master && argc > 2 && 0 == strcmp(SKS_ARG_RESTART, argv[argc - 1]));
    return status;
}

/*---------------------------------------------------------------------------
 *                             Server main function
 *---------------------------------------------------------------------------*/

int main(int argc, char* argv[])
{
    // Install signal handler to close the server gracefully when server needs to stop
    signal(SIGINT, Test_StopSignal);
    signal(SIGTERM, Test_StopSignal);

    // Sleep timeout in milliseconds
    const uint32_t sleepTimeout = 500;

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    /* Get the toolkit build information and print it */
    SOPC_Toolkit_Build_Info build_info = SOPC_Helper_GetBuildInfo();
    printf("S2OPC_Common       - Version: %s, SrcCommit: %s, DockerId: %s, BuildDate: %s\n",
           build_info.commonBuildInfo.buildVersion, build_info.commonBuildInfo.buildSrcCommit,
           build_info.commonBuildInfo.buildDockerId, build_info.commonBuildInfo.buildBuildDate);
    printf("S2OPC_ClientServer - Version: %s, SrcCommit: %s, DockerId: %s, BuildDate: %s\n",
           build_info.clientServerBuildInfo.buildVersion, build_info.clientServerBuildInfo.buildSrcCommit,
           build_info.clientServerBuildInfo.buildDockerId, build_info.clientServerBuildInfo.buildBuildDate);

    /* Configure the SKS mode based on command line parameters */
    status = Config_ConfigureSKSServerMode(argc, argv);

    if (SOPC_STATUS_OK == status)
    {
        printf("<Test_SKS_Server: SKS Server %s configured\n", argv[1]);
    }
    else
    {
        printf("<Test_SKS_Server: Error Invalid parameters.\n");
        printf("<Test_SKS_Server: %s SERVER_MODE ADRESS_INDEX.\n", argv[0]);
        printf("<Test_SKS_Server:   - SERVER_MODE should be master or slave.\n");
        printf("<Test_SKS_Server:   - ADRESS_INDEX should setted only for slave. The value should be 1 or 2.\n");

        return (status == SOPC_STATUS_OK) ? 0 : 1;
    }

    /* Configure the server logger:
     * DEBUG traces generated in ./test_sks_server_<sksServerIndex>_logs/ */
    char* logDirPath = Server_ConfigLogPath();

    /* Initialize the server library (start library threads) */
    status = Server_Initialize(logDirPath);

    /* Configuration of:
     * - Server endpoints configuration from XML server configuration file (comply with s2opc_clientserver_config.xsd) :
         - Enpoint URL,
         - Security endpoint properties,
         - Cryptographic parameters,
         - Application description
       - Server address space initial content from XML configuration file (comply with UANodeSet.xsd)
       - User authentication and authorization management from XML configuration file
         (comply with s2opc_clientserver_users_config.xsd)
    */
    if (SOPC_STATUS_OK == status)
    {
        status = Server_LoadServerConfiguration();
    }

    // Define sks implementation of functions called for method call service
    if (SOPC_STATUS_OK == status)
    {
        status = Server_InitSKScallMethodService();
    }

    /* Define address space write notification callback */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_HelperConfigServer_SetWriteNotifCallback(Demo_WriteNotificationCallback);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "Failed to configure the @ space modification notification callback");
        }
    }

    /* Manage client events for SKS client module */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_HelperConfigClient_SetRawClientComEvent(SKS_ComEvent_FctClient);
    }

    if (SOPC_STATUS_OK == status)
    {
        /* Run the server  */
        status = SOPC_ServerHelper_StartServer(Test_ServerStopped_Fct);

        if (SOPC_STATUS_OK != status)
        {
            printf("<Test_SKS_Server: Failed to run the server with error = '%d'\n", status);
        }
        else
        {
            printf("<Test_SKS_Server: Server started\n");
        }
    }
    else
    {
        printf("<Test_SKS_Server: Error during configuration phase, see logs in %s directory for details.\n",
               logDirPath);
    }

    /* Master retrieve Keys from Slave and start Scheduler */
    if (SOPC_STATUS_OK == status)
    {
        status = Server_SKS_Start();
    }

    /* Run the server until notification that endpoint is closed received
     *  or stop server signal detected (Ctrl-C) */
    while (SOPC_STATUS_OK == status && stopServer == 0 && SOPC_Atomic_Int_Get(&endpointClosed) == 0)
    {
        SOPC_Sleep(sleepTimeout);
    }

    /* Asynchronous request to close the endpoint */
    if (SOPC_STATUS_OK == status && SOPC_Atomic_Int_Get(&endpointClosed) == 0)
    {
        SOPC_ServerHelper_StopServer();
    }

    /* Stop and clear SKS related modules */
    Client_Stop();
    SOPC_SKscheduler_StopAndClear(skScheduler);
    SOPC_Free(skScheduler);
    SOPC_SKManager_Clear(skManager);
    SOPC_Free(skManager);
    Client_Teardown();

    /* Clear the server library (stop all library threads) and server configuration */
    SOPC_Helper_Clear();

    if (SOPC_STATUS_OK != status)
    {
        printf("<Test_SKS_Server: Terminating with error status, see logs in %s directory for details.\n", logDirPath);
    }

    // Free the string containing log path
    SOPC_Free(logDirPath);

    return (status == SOPC_STATUS_OK) ? 0 : 1;
}
