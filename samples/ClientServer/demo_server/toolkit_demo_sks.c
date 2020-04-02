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
#include <errno.h>
#include <inttypes.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h> /* getenv, exit */
#include <string.h>

#include "client.h"
#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"
#include "sopc_atomic.h"
#include "sopc_crypto_profiles.h"
#include "sopc_crypto_provider.h"
#include "sopc_encodeable.h"
#include "sopc_mem_alloc.h"
#include "sopc_pki_stack.h"
#include "sopc_sk_builder.h"
#include "sopc_sk_manager.h"
#include "sopc_sk_ordonnancer.h"
#include "sopc_sk_provider.h"
#include "sopc_time.h"
#include "sopc_toolkit_async_api.h"
#include "sopc_toolkit_config.h"
#include "sopc_user_app_itf.h"

#include "embedded/sopc_addspace_loader.h"
//#include "runtime_variables.h"

#ifdef WITH_EXPAT
#include "xml_expat/sopc_config_loader.h"
#include "xml_expat/sopc_uanodeset_loader.h"
#endif

#ifdef WITH_STATIC_SECURITY_DATA
#include "static_security_data.h"
static SOPC_SerializedCertificate* static_cacert = NULL;
static SOPC_CRLList* static_cacrl = NULL;
#endif

#define DEFAULT_ENDPOINT_URL "opc.tcp://localhost:4841"
#define DEFAULT_APPLICATION_URI "urn:S2OPC:localhost"
#define DEFAULT_PRODUCT_URI "urn:S2OPC:localhost"
#define DEFAULT_PRODUCT_URI_2 "urn:S2OPC:localhost_2"

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

/* Define application namespaces: ns=1 and ns=2 (NULL terminated array) */
static char* default_app_namespace_uris[] = {DEFAULT_PRODUCT_URI, DEFAULT_PRODUCT_URI_2, NULL};
static char* default_locale_ids[] = {"en-US", "fr-FR", NULL};

static char* default_trusted_root_issuers[] = {
    "trusted/cacert.der", /* Demo CA */
#ifdef IS_TEST_SERVER
    "trusted/ctt_ca1TC.der", /* Will be ignored because no CRL associated. Tests 042, 043 */
    "trusted/ctt_ca1T.der",  /* Tests 029, 037 */
#endif
    NULL};
static char* default_trusted_intermediate_issuers[] = {
#ifdef IS_TEST_SERVER
    "trusted/ctt_ca1I_ca2T.der",
#endif
    NULL};
static char* default_issued_certs[] = {
#ifdef IS_TEST_SERVER
    "issued/ctt_appT.der",  /* Test 048 */
    "issued/ctt_appTE.der", /* Test 007 */
    "issued/ctt_appTSha1_1024.der",
    "issued/ctt_appTSha1_2048.der",
    "issued/ctt_appTSha256_2048.der", /* Test 051 */
    "issued/ctt_appTSha256_4096.der", /* Test 052 still fails */
    "issued/ctt_appTSincorrect.der",  /* Test 010 */
    "issued/ctt_appTSip.der",
    "issued/ctt_appTSuri.der",
    "issued/ctt_appTV.der",     /* Test 008 */
    "issued/ctt_ca1I_appT.der", /* Test 044 */
    "issued/ctt_ca1I_appTR.der",
    "issued/ctt_ca1I_ca2T_appT.der",
    "issued/ctt_ca1I_ca2T_appTR.der",
    "issued/ctt_ca1IC_appT.der",
    "issued/ctt_ca1IC_appTR.der",
    "issued/ctt_ca1T_appT.der",
    "issued/ctt_ca1T_appTR.der",
    "issued/ctt_ca1T_ca2U_appT.der",
    "issued/ctt_ca1T_ca2U_appTR.der",
    "issued/ctt_ca1TC_appT.der",
    "issued/ctt_ca1TC_appTR.der",
    "issued/ctt_ca1TC_ca2I_appT.der", /* Test 002 */
    "issued/ctt_ca1TC_ca2I_appTR.der",
    "issued/ctt_ca1U_appT.der", /* Test 046 */
    "issued/ctt_ca1U_appTR.der",
#endif
    NULL};
static char* default_untrusted_root_issuers[] = {
#ifdef IS_TEST_SERVER
    "untrusted/ctt_ca1IC.der", /* Will be ignored because no CRL associated */
    "untrusted/ctt_ca1I.der",  /* Test 044 */
#endif
    NULL};
static char* default_untrusted_intermediate_issuers[] = {
#ifdef IS_TEST_SERVER
    "untrusted/ctt_ca1TC_ca2I.der", /* Test 002 */
#endif
    NULL};
static char* default_revoked_certs[] = {"revoked/cacrl.der",
#ifdef IS_TEST_SERVER
                                        "revoked/revocation_list_ctt_ca1T.crl",
                                        "revoked/revocation_list_ctt_ca1I.crl",
                                        "revoked/revocation_list_ctt_ca1I_ca2T.crl",
                                        "revoked/revocation_list_ctt_ca1TC_ca2I.crl",
#endif
                                        NULL};

static int32_t endpointClosed = 0;
static bool secuActive = true;

volatile sig_atomic_t stopServer = 0;

typedef enum
{
    AS_LOADER_EMBEDDED,
#ifdef WITH_EXPAT
    AS_LOADER_EXPAT,
#endif
} AddressSpaceLoader;

typedef enum
{
    SRV_LOADER_DEFAULT_CONFIG,
#ifdef WITH_EXPAT
    SRV_LOADER_EXPAT_XML_CONFIG,
#endif
} ServerConfigLoader;

#define SHUTDOWN_PHASE_IN_SECONDS 5

/* NodeIds of method for Call Method Service */
SOPC_NodeId* methodIds[1] = {NULL};
uint32_t nbMethodIds = 0;

/* SKS Data  */
SOPC_SKManager* skManager;
SOPC_SKOrdonnancer* skOrdonnancer;
SKS_ServerModeType sksServerMode;
uint64_t sksServerIndex;
bool sksRestart = false;

static SOPC_StatusCode Server_SKS_Configure(SOPC_Server_Config* serverConfig);
static SOPC_StatusCode Server_SKS_Start(void);

/*---------------------------------------------------------------------------
 *                          Callbacks definition
 *---------------------------------------------------------------------------*/

static SOPC_StatusCode SOPC_Method_Func_PublishSubscribe_getSecurityKeys(const SOPC_CallContext* callContextPtr,
                                                                         const SOPC_NodeId* objectId,
                                                                         uint32_t nbInputArgs,
                                                                         const SOPC_Variant* inputArgs,
                                                                         uint32_t* nbOutputArgs,
                                                                         SOPC_Variant** outputArgs,
                                                                         void* param)
{
    (void) objectId; /* Should be "i=15000"*/
    (void) param;    /* Should be NULL */

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

/*
 * Server callback definition used for endpoint communication events
 * (endpoint closed, local service response)
 */
static void Test_ComEvent_FctServer(SOPC_App_Com_Event event, uint32_t idOrStatus, void* param, uintptr_t appContext)
{
    /* avoid unused parameter compiler warning */
    (void) idOrStatus;
    (void) appContext;
    bool debug = false;
    SOPC_EncodeableType* message_type = NULL;
    OpcUa_WriteResponse* write_response = NULL;
    bool ok = false;

    switch (event)
    {
        /* Client application events */
    case SE_SESSION_ACTIVATION_FAILURE:
        if (debug)
        {
            printf(">>Client debug : SE_SESSION_ACTIVATION_FAILURE RECEIVED\n");
            printf(">>Client debug : appContext: %lu\n", appContext);
        }
        if (0 != appContext && appContext == Client_SessionContext)
        {
            SOPC_Atomic_Int_Set((SessionConnectedState*) &scState, (SessionConnectedState) SESSION_CONN_FAILED);
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
        SOPC_Atomic_Int_Set((SessionConnectedState*) &scState, (SessionConnectedState) SESSION_CONN_CONNECTED);
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
    case SE_CLOSED_ENDPOINT:
        printf("<Test_Server_Toolkit: closed endpoint event: OK\n");
        SOPC_Atomic_Int_Set(&endpointClosed, 1);
        break;
    case SE_LOCAL_SERVICE_RESPONSE:
        message_type = *((SOPC_EncodeableType**) param);

        if (message_type != &OpcUa_WriteResponse_EncodeableType)
        {
            return;
        }

        write_response = param;

        ok = (write_response->ResponseHeader.ServiceResult == SOPC_GoodGenericStatus);

        for (int32_t i = 0; i < write_response->NoOfResults; ++i)
        {
            ok &= (write_response->Results[i] == SOPC_GoodGenericStatus);
        }

        if (!ok)
        {
            printf("<Test_Server_Toolkit: Error while updating address space\n");
        }

        break;
    default:
        printf("<Test_Server_Toolkit: unexpected endpoint event %d : NOK\n", event);
    }
}

/*
 * Server callback definition used for address space modification notification
 */
static void Test_AddressSpaceNotif_Fct(const SOPC_CallContext* callCtxPtr,
                                       SOPC_App_AddSpace_Event event,
                                       void* opParam,
                                       SOPC_StatusCode opStatus)
{
    /* avoid unused parameter compiler warning */
    (void) callCtxPtr;
    (void) opParam;
    (void) opStatus;

    if (event != AS_WRITE_EVENT)
    {
        printf("<Test_Server_Toolkit: unexpected address space event %d : NOK\n", event);
    }
}

/*---------------------------------------------------------------------------
 *                          Server initialization
 *---------------------------------------------------------------------------*/

static SOPC_ReturnStatus Server_Initialize(void)
{
    // Initialize the toolkit library and define the communication events callback
    SOPC_ReturnStatus status = SOPC_Toolkit_Initialize(Test_ComEvent_FctServer);
    if (SOPC_STATUS_OK != status)
    {
        printf("<Test_Server_Toolkit: Failed initializing\n");
    }
    else
    {
        printf("<Test_Server_Toolkit: initialized\n");
    }
    return status;
}

/*---------------------------------------------------------------------------
 *                             Server configuration
 *---------------------------------------------------------------------------*/

/*----------------------------------------------------
 * Application description and endpoint configuration:
 *---------------------------------------------------*/

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
    SOPC_SKBuilder* skbAppend;
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
    if (SKS_ServerMode_Master != sksServerMode || false == sksRestart)
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

static SOPC_StatusCode Server_SKS_Start()
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

    /* Init SK Ordonnancer */
    if (SOPC_STATUS_OK == status)
    {
        skOrdonnancer = SOPC_SKOrdonnancer_Create();
        if (NULL == skOrdonnancer)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    /* Init SK Builder and SK Provider */
    SOPC_SKBuilder* skBuilder;
    SOPC_SKProvider* skProvider;
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
        status =
            SOPC_SKOrdonnancer_AddTask(skOrdonnancer, skBuilder, skProvider, skManager, SKS_ORDONANCER_INIT_MSPERIOD);
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_SKOrdonnancer_Start(skOrdonnancer);
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

static SOPC_StatusCode Server_SKS_Configure(SOPC_Server_Config* serverConfig)
{
    SOPC_NodeId* methodId;
    SOPC_MethodCallFunc_Ptr methodFunc;
    serverConfig->mcm = SOPC_MethodCallManager_Create();
    SOPC_StatusCode status = (NULL != serverConfig->mcm) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;

    /* Init Method Call Manager */
    if (SOPC_STATUS_OK == status)
    {
        // getSecurityKeys method node
        methodId = SOPC_NodeId_FromCString("i=15001", 7);
        if (NULL != methodId)
        {
            methodIds[nbMethodIds] = methodId;
            nbMethodIds++;
            methodFunc = &SOPC_Method_Func_PublishSubscribe_getSecurityKeys;
            status = SOPC_MethodCallManager_AddMethod(serverConfig->mcm, methodId, methodFunc, NULL, NULL);
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
    }

    return status;
}

/*
 * Default server configuration loader (without XML configuration)
 */
static bool Server_LoadDefaultConfiguration(SOPC_S2OPC_Config* output_s2opcConfig)
{
    /* Security is mandatory to ensure that PubSub Security key are encrypt */
    assert(secuActive);
    assert(NULL != output_s2opcConfig);
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    // Set namespaces
    output_s2opcConfig->serverConfig.namespaces = default_app_namespace_uris;
    // Set locale ids
    output_s2opcConfig->serverConfig.localeIds = default_locale_ids;

    // Set application description of server to be returned by discovery services (GetEndpoints, FindServers)
    OpcUa_ApplicationDescription* serverDescription = &output_s2opcConfig->serverConfig.serverDescription;
    OpcUa_ApplicationDescription_Initialize(serverDescription);
    SOPC_String_AttachFromCstring(&serverDescription->ApplicationUri, DEFAULT_APPLICATION_URI);
    SOPC_String_AttachFromCstring(&serverDescription->ProductUri, DEFAULT_PRODUCT_URI);
    serverDescription->ApplicationType = OpcUa_ApplicationType_Server;
    SOPC_String_AttachFromCstring(&serverDescription->ApplicationName.defaultText, "S2OPC toolkit server example");
    SOPC_String_AttachFromCstring(&serverDescription->ApplicationName.defaultLocale, "en-US");
    SOPC_LocalizedText appNameFr;
    SOPC_LocalizedText_Initialize(&appNameFr);
    SOPC_String_AttachFromCstring(&appNameFr.defaultText, "S2OPC toolkit: exemple de serveur");
    SOPC_String_AttachFromCstring(&appNameFr.defaultLocale, "fr-FR");

    status = SOPC_LocalizedText_AddOrSetLocale(&serverDescription->ApplicationName,
                                               output_s2opcConfig->serverConfig.localeIds, &appNameFr);
    SOPC_LocalizedText_Clear(&appNameFr);
    if (SOPC_STATUS_OK != status)
    {
        printf("<Test_Server_Toolkit: Error setting second locale application name\n");
    }

    output_s2opcConfig->serverConfig.endpoints = SOPC_Calloc(sizeof(SOPC_Endpoint_Config), 1);

    if (NULL == output_s2opcConfig->serverConfig.endpoints)
    {
        return false;
    }

    output_s2opcConfig->serverConfig.nbEndpoints = 1;
    SOPC_Endpoint_Config* pEpConfig = &output_s2opcConfig->serverConfig.endpoints[0];
    pEpConfig->serverConfigPtr = &output_s2opcConfig->serverConfig;
    assert(nb_sks_server > sksServerIndex);
    pEpConfig->endpointURL = sks_server_endpoint_uris[sksServerIndex];
    pEpConfig->hasDiscoveryEndpoint = true;

    /*
     * Define the certificates, security policies, security modes and user token policies supported by endpoint
     */
    if (secuActive)
    {
        output_s2opcConfig->serverConfig.serverCertPath = "./server_public/server_2k_cert.der";
        output_s2opcConfig->serverConfig.serverKeyPath = "./server_private/server_2k_key.pem";
        output_s2opcConfig->serverConfig.trustedRootIssuersList = default_trusted_root_issuers;
        output_s2opcConfig->serverConfig.trustedIntermediateIssuersList = default_trusted_intermediate_issuers;
        output_s2opcConfig->serverConfig.issuedCertificatesList = default_issued_certs;
        output_s2opcConfig->serverConfig.untrustedRootIssuersList = default_untrusted_root_issuers;
        output_s2opcConfig->serverConfig.untrustedIntermediateIssuersList = default_untrusted_intermediate_issuers;
        output_s2opcConfig->serverConfig.certificateRevocationPathList = default_revoked_certs;

        /*
         * 1st Security policy is Basic256Sha256 with anonymous and username (encrypted) authentication allowed
         */
        SOPC_String_Initialize(&pEpConfig->secuConfigurations[0].securityPolicy);
        status = SOPC_String_AttachFromCstring(&pEpConfig->secuConfigurations[0].securityPolicy,
                                               SOPC_SecurityPolicy_Basic256Sha256_URI);
        pEpConfig->secuConfigurations[0].securityModes = SOPC_SECURITY_MODE_SIGNANDENCRYPT_MASK;
        pEpConfig->secuConfigurations[0].nbOfUserTokenPolicies = 2;
        pEpConfig->secuConfigurations[0].userTokenPolicies[0] = SOPC_UserTokenPolicy_Anonymous;
        pEpConfig->secuConfigurations[0].userTokenPolicies[1] = SOPC_UserTokenPolicy_UserName_NoneSecurityPolicy;

        /*
         * 2nd Security policy is Basic256 with anonymous and username (non encrypted) authentication allowed
         */
        if (SOPC_STATUS_OK == status)
        {
            SOPC_String_Initialize(&pEpConfig->secuConfigurations[1].securityPolicy);
            status = SOPC_String_AttachFromCstring(&pEpConfig->secuConfigurations[1].securityPolicy,
                                                   SOPC_SecurityPolicy_Basic256_URI);
            pEpConfig->secuConfigurations[1].securityModes = SOPC_SECURITY_MODE_SIGNANDENCRYPT_MASK;
            pEpConfig->secuConfigurations[1].nbOfUserTokenPolicies = 2;
            pEpConfig->secuConfigurations[1].userTokenPolicies[0] = SOPC_UserTokenPolicy_Anonymous;
            pEpConfig->secuConfigurations[1].userTokenPolicies[1] = SOPC_UserTokenPolicy_UserName_NoneSecurityPolicy;
        }
    }

    if (secuActive)
    {
        pEpConfig->nbSecuConfigs = 2;
    }

    return SOPC_STATUS_OK == status;
}

#ifdef WITH_EXPAT
static bool load_config_from_file(const char* filename, SOPC_S2OPC_Config* s2opcConfig)
{
    FILE* fd = fopen(filename, "r");

    if (fd == NULL)
    {
        printf("<Test_Server_Toolkit: Error while opening %s: %s\n", filename, strerror(errno));
        return false;
    }

    bool res = SOPC_Config_Parse(fd, s2opcConfig);
    fclose(fd);

    if (res)
    {
        printf("<Test_Server_Toolkit: Loaded configuration from %s\n", filename);
    }
    else
    {
        printf("<Test_Server_Toolkit: Error while parsing XML configuration file\n");
    }

    return res;
}
#endif

static SOPC_ReturnStatus Server_LoadServerConfiguration(SOPC_S2OPC_Config* output_s2opcConfig)
{
    /* Load server endpoints configuration
     * If WITH_EXPAT environment variable defined,
     * retrieve XML file path from environment variable TEST_SERVER_XML_CONFIG.
     * In case of success, use the dynamic server configuration loader from an XML file.
     *
     * Otherwise use an embedded default demo server configuration.
     */
    assert(NULL != output_s2opcConfig);

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    ServerConfigLoader config_loader = SRV_LOADER_DEFAULT_CONFIG;

#ifdef WITH_EXPAT
    const char* xml_file_path = getenv("TEST_SERVER_XML_CONFIG");

    if (xml_file_path != NULL)
    {
        config_loader = SRV_LOADER_EXPAT_XML_CONFIG;
    }
#endif

    bool res = false;
    /* Load the address space using loader */
    switch (config_loader)
    {
    case SRV_LOADER_DEFAULT_CONFIG:
        res = Server_LoadDefaultConfiguration(output_s2opcConfig);
        break;
#ifdef WITH_EXPAT
    case SRV_LOADER_EXPAT_XML_CONFIG:
        res = load_config_from_file(xml_file_path, output_s2opcConfig);
        break;
#endif
    default:
        assert(false);
    }

    /* Check properties on configuration */
    if (res)
    {
        status = SOPC_STATUS_OK;

        // Only 1 endpoint supported in demo server
        if (output_s2opcConfig->serverConfig.nbEndpoints > 1)
        {
            printf("<Test_Server_Toolkit: Error only 1 endpoint supported but %" PRIu8 " in configuration\n",
                   output_s2opcConfig->serverConfig.nbEndpoints);
            status = SOPC_STATUS_INVALID_PARAMETERS;
        }

        if (SOPC_STATUS_OK == status)
        {
            SOPC_Endpoint_Config* epConfig = &output_s2opcConfig->serverConfig.endpoints[0];
            for (int i = 0; i < epConfig->nbSecuConfigs; i++)
            {
                SOPC_SecurityPolicy* secuPolicy = &epConfig->secuConfigurations[i];
                const char* secuPolicyURI = SOPC_String_GetRawCString(&secuPolicy->securityPolicy);
                // Only None, Basic256 and Basic256Sha256 policies supported
                if (0 != strcmp(SOPC_SecurityPolicy_None_URI, secuPolicyURI) &&
                    0 != strcmp(SOPC_SecurityPolicy_Basic256_URI, secuPolicyURI) &&
                    0 != strcmp(SOPC_SecurityPolicy_Basic256Sha256_URI, secuPolicyURI))
                {
                    printf("<Test_Server_Toolkit: Error invalid or unsupported security policy %s in configuration\n",
                           secuPolicyURI);
                    status = SOPC_STATUS_INVALID_PARAMETERS;
                }

                // UserName token type is only supported with security policy None
                for (int j = 0; j < secuPolicy->nbOfUserTokenPolicies; j++)
                {
                    secuPolicyURI = SOPC_String_GetRawCString(&secuPolicy->userTokenPolicies[j].SecurityPolicyUri);
                    if (secuPolicy->userTokenPolicies[j].TokenType == OpcUa_UserTokenType_UserName &&
                        0 != strcmp(SOPC_SecurityPolicy_None_URI, secuPolicyURI))
                    {
                        printf(
                            "<Test_Server_Toolkit: Error invalid or unsupported username user security policy %s in "
                            "configuration\n",
                            secuPolicyURI);
                        status = SOPC_STATUS_INVALID_PARAMETERS;
                    }
                }
            }
        }
    }
    else
    {
        printf("<Test_Server_Toolkit: Failed to load the server configuration\n");
        status = SOPC_STATUS_NOK;
    }

    return status;
}

/*
 * Configure the cryptographic parameters of the endpoint:
 * - Server certificate and key
 * - Public Key Infrastructure: using a single certificate as Certificate Authority or Trusted Certificate
 */
static SOPC_ReturnStatus Server_SetCryptographicConfig(SOPC_Server_Config* serverConfig)
{
    assert(NULL != serverConfig);

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (secuActive)
    {
#ifdef WITH_STATIC_SECURITY_DATA
        /* Load client/server certificates and server key from C source files (no filesystem needed) */
        status = SOPC_KeyManager_SerializedCertificate_CreateFromDER(server_2k_cert, sizeof(server_2k_cert),
                                                                     &serverConfig->serverCertificate);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_KeyManager_SerializedAsymmetricKey_CreateFromData(server_2k_key, sizeof(server_2k_key),
                                                                            &serverConfig->serverKey);
        }

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_KeyManager_SerializedCertificate_CreateFromDER(cacert, sizeof(cacert), &static_cacert);
        }

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_KeyManager_CRL_CreateOrAddFromDER(cacrl, sizeof(cacrl), &static_cacrl);
        }

        /* Create the PKI (Public Key Infrastructure) provider */
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_PKIProviderStack_Create(static_cacert, static_cacrl, &serverConfig->pki);
        }
#else // WITH_STATIC_SECURITY_DATA == false
        /* Load client/server certificates and server key from files */
        status = SOPC_KeyManager_SerializedCertificate_CreateFromFile(serverConfig->serverCertPath,
                                                                      &serverConfig->serverCertificate);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_KeyManager_SerializedAsymmetricKey_CreateFromFile(serverConfig->serverKeyPath,
                                                                            &serverConfig->serverKey);
        }

        /* Create the PKI (Public Key Infrastructure) provider */
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_PKIProviderStack_CreateFromPaths(
                serverConfig->trustedRootIssuersList, serverConfig->trustedIntermediateIssuersList,
                serverConfig->untrustedRootIssuersList, serverConfig->untrustedIntermediateIssuersList,
                serverConfig->issuedCertificatesList, serverConfig->certificateRevocationPathList, &serverConfig->pki);
        }
#endif

        if (SOPC_STATUS_OK != status)
        {
            printf("<Test_Server_Toolkit: Failed loading certificates and key (check paths are valid)\n");
        }
        else
        {
            printf("<Test_Server_Toolkit: Certificates and key loaded\n");
        }
    }

    return status;
}

/*----------------------------------------
 * Users authentication and authorization:
 *----------------------------------------*/

/* The toolkit test servers shall pass the UACTT tests. Hence it shall authenticate
 * (ids and passwords can be changed in the UACTT settings/Server Test/Session):
 *  - anonymous users
 *  - user1:password
 *  - user2:password1
 * Then it shall accept username:password, but return "access denied".
 * Otherwise it shall be "identity token rejected".
 */
static SOPC_ReturnStatus authentication_uactt(SOPC_UserAuthentication_Manager* authn,
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
        else if (strcmp(SOPC_String_GetRawCString(username), "user2") == 0)
        {
            SOPC_ByteString* pwd = &userToken->Password;
            if (pwd->Length == strlen("password1") && memcmp(pwd->Data, "password1", strlen("password1")) == 0)
            {
                *authenticated = SOPC_USER_AUTHENTICATION_OK;
            }
        }
        else if (strcmp(SOPC_String_GetRawCString(username), "username") == 0)
        {
            SOPC_ByteString* pwd = &userToken->Password;
            if (pwd->Length == strlen("password") && memcmp(pwd->Data, "password", strlen("password")) == 0)
            {
                *authenticated = SOPC_USER_AUTHENTICATION_ACCESS_DENIED;
            }
        }
    }

    return SOPC_STATUS_OK;
}

static const SOPC_UserAuthentication_Functions authentication_uactt_functions = {
    .pFuncFree = (SOPC_UserAuthentication_Free_Func) SOPC_Free,
    .pFuncValidateUserIdentity = authentication_uactt};

static SOPC_ReturnStatus Server_SetUserManagementConfig(SOPC_Endpoint_Config* pEpConfig,
                                                        SOPC_UserAuthentication_Manager** output_authenticationManager,
                                                        SOPC_UserAuthorization_Manager** output_authorizationManager)
{
    assert(NULL != output_authenticationManager);
    assert(NULL != output_authorizationManager);

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    /* Create an user authorization manager which accepts any user.
     * i.e.: UserAccessLevel right == AccessLevel right for any user for a given node of address space */
    *output_authorizationManager = SOPC_UserAuthorization_CreateManager_AllowAll();
    *output_authenticationManager = SOPC_Calloc(1, sizeof(SOPC_UserAuthentication_Manager));
    if (NULL == *output_authenticationManager || NULL == *output_authorizationManager)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
        printf("<Test_Server_Toolkit: Failed to create the user manager\n");
    }

    if (SOPC_STATUS_OK == status)
    {
        /* Set a user authentication function that complies with UACTT tests expectations */
        (*output_authenticationManager)->pFunctions = &authentication_uactt_functions;
        pEpConfig->authenticationManager = *output_authenticationManager;
        pEpConfig->authorizationManager = *output_authorizationManager;
    }

    return status;
}

/*------------------------------
 * Address space configuration :
 *------------------------------*/

/*
 * XML dynamic loader use for parsing an XML address space defintion file
 */
#ifdef WITH_EXPAT
static SOPC_AddressSpace* load_nodeset_from_file(const char* filename)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_AddressSpace* space = NULL;
    FILE* fd = fopen(filename, "r");

    if (fd == NULL)
    {
        printf("<Test_Server_Toolkit: Error while opening %s: %s\n", filename, strerror(errno));
        status = SOPC_STATUS_NOK;
    }

    if (status == SOPC_STATUS_OK)
    {
        space = SOPC_UANodeSet_Parse(fd);

        if (space == NULL)
        {
            status = SOPC_STATUS_NOK;
        }
    }

    if (status != SOPC_STATUS_OK)
    {
        printf("<Test_Server_Toolkit: Error while parsing XML address space\n");
    }

    if (fd != NULL)
    {
        fclose(fd);
    }

    if (status == SOPC_STATUS_OK)
    {
        printf("<Test_Server_Toolkit: Loaded address space from %s\n", filename);
        return space;
    }
    else
    {
        SOPC_AddressSpace_Delete(space);
        return NULL;
    }
}
#endif

static SOPC_ReturnStatus Server_ConfigureAddressSpace(SOPC_AddressSpace** output_addressSpace)
{
    /* Define server address space loader:
     * If WITH_EXPAT environment variable defined,
     * retrieve XML file path from environment variable TEST_SERVER_XML_ADDRESS_SPACE.
     * In case of success, use the dynamic address space loader from an XML file.
     *
     * Otherwise use the embedded address space (already defined as C code) loader.
     * For this latter case the address space C structure shall have been generated prior to compilation.
     * This should be done using the script ./scripts/generate-s2opc-address-space.py
     */
    assert(NULL != output_addressSpace);

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    AddressSpaceLoader address_space_loader = AS_LOADER_EMBEDDED;

#ifdef WITH_EXPAT
    const char* xml_file_path = getenv("TEST_SERVER_XML_ADDRESS_SPACE");

    if (xml_file_path != NULL)
    {
        address_space_loader = AS_LOADER_EXPAT;
    }
#endif

    /* Load the address space using loader */
    switch (address_space_loader)
    {
    case AS_LOADER_EMBEDDED:
        *output_addressSpace = SOPC_Embedded_AddressSpace_Load();
        status = NULL != *output_addressSpace ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
        break;
#ifdef WITH_EXPAT
    case AS_LOADER_EXPAT:
        *output_addressSpace = load_nodeset_from_file(xml_file_path);
        status = (NULL != *output_addressSpace) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
        break;
#endif
    default:
        assert(false);
    }

    /* Set the loaded address space as the current server address space configuration */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ToolkitServer_SetAddressSpaceConfig(*output_addressSpace);

        if (SOPC_STATUS_OK != status)
        {
            printf("<Test_Server_Toolkit: Failed to configure the @ space\n");
        }
        else
        {
            printf("<Test_Server_Toolkit: @ space configured\n");
        }
    }

    return status;
}

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
    if (0 == strcmp(SKS_ARG_MODE_MASTER, serverMode))
    {
        /* Master address is at first index */
        sksServerMode = SKS_ServerMode_Master;
        sksServerIndex = 0;
    }
    else if (0 == strcmp(SKS_ARG_MODE_SLAVE, serverMode) && 2 < argc)
    {
        sksServerMode = SKS_ServerMode_Slave;

        /* Slave address index is given as second parameter */

        char* endptr;
        char* strIndex = argv[2];
        uint64_t val = strtoul(strIndex, &endptr, 10);

        /* Check that all characters are valid and in array bounds */
        if (endptr == (strIndex + strlen(strIndex)) && 0 < val && nb_sks_server > val)
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

    // Only Master as a restart mode. Find SKS restart keyword in last parameter
    sksRestart = (sksServerMode == SKS_ServerMode_Master && argc > 2 && 0 == strcmp(SKS_ARG_RESTART, argv[argc - 1]));
    return status;
}

/*---------------------------------------------------------------------------
 *                             Server main function
 *---------------------------------------------------------------------------*/

int main(int argc, char* argv[])
{
    (void) argc;
    (void) argv;

    // Install signal handler to close the server gracefully when server needs to stop
    signal(SIGINT, Test_StopSignal);
    signal(SIGTERM, Test_StopSignal);

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    status = Config_ConfigureSKSServerMode(argc, argv);
    if (SOPC_STATUS_OK == status)
    {
        printf("<Test_Server_Toolkit: SKS Server %s configured\n", argv[1]);
    }
    else
    {
        printf("<Test_Server_Toolkit: Error Invalid parameters.\n");
        printf("<Test_Server_Toolkit: toolkit_demo_sks SERVER_MODE ADRESS_INDEX.\n");
        printf("<Test_Server_Toolkit:   - SERVER_MODE should be master or slave.\n");
        printf("<Test_Server_Toolkit:   - ADRESS_INDEX should setted only for slave. The value should be 1 or 2.\n");
    }

    SOPC_S2OPC_Config s2opcConfig;
    SOPC_S2OPC_Config_Initialize(&s2opcConfig);
    SOPC_Server_Config* serverConfig = &s2opcConfig.serverConfig;

    SOPC_Endpoint_Config* epConfig = NULL;
    uint32_t epConfigIdx = 0;

    SOPC_UserAuthentication_Manager* authenticationManager = NULL;
    SOPC_UserAuthorization_Manager* authorizationManager = NULL;

    SOPC_AddressSpace* address_space = NULL;

    // Sleep timeout in milliseconds
    const uint32_t sleepTimeout = 500;

    /* Get the toolkit build information and print it */
    // RuntimeVariables runtime_vars;
    SOPC_Toolkit_Build_Info build_info = SOPC_ToolkitConfig_GetBuildInfo();
    printf("S2OPC_Common       - Version: %s, SrcCommit: %s, DockerId: %s, BuildDate: %s\n",
           build_info.commonBuildInfo.buildVersion, build_info.commonBuildInfo.buildSrcCommit,
           build_info.commonBuildInfo.buildDockerId, build_info.commonBuildInfo.buildBuildDate);
    printf("S2OPC_ClientServer - Version: %s, SrcCommit: %s, DockerId: %s, BuildDate: %s\n",
           build_info.clientServerBuildInfo.buildVersion, build_info.clientServerBuildInfo.buildSrcCommit,
           build_info.clientServerBuildInfo.buildDockerId, build_info.clientServerBuildInfo.buildBuildDate);

    /* Initialize the server library (start library threads)
     * and define communication events callback */

    if (SOPC_STATUS_OK == status)
    {
        status = Server_Initialize();
    }

    /* Configuration of server endpoint:
       - Enpoint URL,
       - Security endpoint properties,
       - Cryptographic parameters,
       - User authentication and authorization management,
       - Application description
    */
    if (SOPC_STATUS_OK == status)
    {
        status = Server_LoadServerConfiguration(&s2opcConfig);

        if (SOPC_STATUS_OK == status)
        {
            epConfig = &serverConfig->endpoints[0];
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        status = Server_SetCryptographicConfig(serverConfig);
    }

    // Define demo implementation of user authentication and authorization
    if (SOPC_STATUS_OK == status)
    {
        status = Server_SetUserManagementConfig(epConfig, &authenticationManager, &authorizationManager);
    }

    // Define demo implementation of functions called for method call service
    if (SOPC_STATUS_OK == status)
    {
        status = Server_SKS_Configure(serverConfig);
    }

    /* Set endpoint configuration: keep endpoint configuration identifier for opening it later */
    if (SOPC_STATUS_OK == status)
    {
        epConfigIdx = SOPC_ToolkitServer_AddEndpointConfig(epConfig);
        status = (epConfigIdx != 0 ? SOPC_STATUS_OK : SOPC_STATUS_NOK);
    }

    /* Configure the address space of the server */
    if (SOPC_STATUS_OK == status)
    {
        /* Address space loaded dynamically from XML file
           or from pre-generated C structure */
        status = Server_ConfigureAddressSpace(&address_space);
    }

    /* Define address space modification notification callback */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ToolkitServer_SetAddressSpaceNotifCb(&Test_AddressSpaceNotif_Fct);
        if (SOPC_STATUS_OK != status)
        {
            printf("<Test_Server_Toolkit: Failed to configure the @ space modification notification callback \n");
        }
        else
        {
            printf("<Test_Server_Toolkit: @ space modification notification callback configured\n");
        }
    }

    /* Finalize the server configuration */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Toolkit_Configured();

        if (SOPC_STATUS_OK != status)
        {
            printf("<Test_Server_Toolkit: Failed to configure the endpoint\n");
        }
        else
        {
            printf("<Test_Server_Toolkit: Endpoint configured\n");
        }
    }

    /* Asynchronous request to open the configured endpoint using endpoint configuration index */
    if (SOPC_STATUS_OK == status)
    {
        printf("<Test_Server_Toolkit: Opening endpoint... \n");
        SOPC_ToolkitServer_AsyncOpenEndpoint(epConfigIdx);
    }

    /*
    // Update server information runtime variables in address space
    if (SOPC_STATUS_OK == status)
    {
        runtime_vars = build_runtime_variables(build_info, serverConfig, "Systerel");

        if (!set_runtime_variables(epConfigIdx, &runtime_vars))
        {
            printf("<Test_Server_Toolkit: Failed to populate Server object\n");
            status = SOPC_STATUS_NOK;
        }
    }
    */

    /* Master retrieve Keys from Slave and start Ordonnancer */
    if (SOPC_STATUS_OK == status)
    {
        status = Server_SKS_Start();
    }

    /* Start Client Side to get Security Keys */

    /* Run the server until notification that endpoint is closed received
     *  or stop server signal detected (Ctrl-C) */
    while (SOPC_STATUS_OK == status && stopServer == 0 && SOPC_Atomic_Int_Get(&endpointClosed) == 0)
    {
        SOPC_Sleep(sleepTimeout);
    }

    /* Asynchronous request to close the endpoint */
    if (SOPC_STATUS_OK == status && SOPC_Atomic_Int_Get(&endpointClosed) == 0)
    {
        SOPC_ToolkitServer_AsyncCloseEndpoint(epConfigIdx);
    }

    /* Wait until endpoint is closed or stop server signal */
    while (SOPC_STATUS_OK == status && stopServer == 0 && SOPC_Atomic_Int_Get(&endpointClosed) == 0)
    {
        // Retrieve received messages on socket
        SOPC_Sleep(sleepTimeout);
    }

    /* Clear the server library (stop all library threads) */
    SOPC_Toolkit_Clear();

    // Check that endpoint closed due to stop signal
    if (SOPC_STATUS_OK == status && stopServer == 0)
    {
        status = SOPC_STATUS_NOK;
    }

    if (SOPC_STATUS_OK == status)
    {
        printf("<Test_Server_Toolkit final result: OK\n");
    }
    else
    {
        printf("<Test_Server_Toolkit final result: NOK with status = '%d'\n", status);
    }

    /* Deallocate all locally created structures: */

    for (uint32_t i = 0; i < nbMethodIds; i++)
    {
        if (NULL != methodIds[i])
        {
            SOPC_NodeId_Clear(methodIds[i]);
            SOPC_Free(methodIds[i]);
        }
    }

    Client_Stop();
    SOPC_SKOrdonnancer_StopAndClear(skOrdonnancer);
    SOPC_Free(skOrdonnancer);
    SOPC_SKManager_Clear(skManager);
    SOPC_Free(skManager);
    Client_Teardown();
    SOPC_AddressSpace_Delete(address_space);
    SOPC_S2OPC_Config_Clear(&s2opcConfig);
#ifdef WITH_STATIC_SECURITY_DATA
    SOPC_KeyManager_SerializedCertificate_Delete(static_cacert);
#endif

    return (status == SOPC_STATUS_OK) ? 0 : 1;
}
