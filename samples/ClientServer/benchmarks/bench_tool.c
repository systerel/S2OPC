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

#include <errno.h>
#include <float.h>
#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sopc_askpass.h"
#include "sopc_assert.h"
#include "sopc_common.h"
#include "sopc_crypto_profiles.h"
#include "sopc_date_time.h"
#include "sopc_mem_alloc.h"
#include "sopc_mutexes.h"
#include "sopc_pki_stack.h"
#include "sopc_toolkit_async_api.h"
#include "sopc_toolkit_config.h"
#include "sopc_user_app_itf.h"

static const char* DEFAULT_SERVER_URL = "opc.tcp://localhost:4841";

// Minimum number of measurements to consider a statistic representative.
#define MIN_N_MEASUREMENTS 1000

// Maximum number of measurements after which we give up doing more measurements.
#define MAX_N_MEASUREMENTS 10000

// Relative margin of error threshold below which we consider our sample set is
// representative.
#define REASONABLE_RMOE 10.

typedef enum
{
    BENCH_RUNNING,
    BENCH_FINISHED_OK,
    BENCH_FINISHED_ERROR,
} bench_status_t;

typedef void* bench_func_t(size_t request_size, size_t bench_offset, size_t addspace_size);

// See https://www.itl.nist.gov/div898/handbook/eda/section3/eda3672.htm
static const double T_TABLE[] = {0,     12.706, 4.303, 3.182, 2.776, 2.571, 2.447, 2.365, 2.306, 2.262, 2.228,
                                 2.201, 2.179,  2.16,  2.145, 2.131, 2.12,  2.11,  2.101, 2.093, 2.086, 2.08,
                                 2.074, 2.069,  2.064, 2.06,  2.056, 2.052, 2.048, 2.045, 2.042, 1.96};
#define T_TABLE_LAST_IDX 30
#define T_TABLE_INF_IDX (T_TABLE_LAST_IDX + 1)

struct app_ctx_t
{
    SOPC_Mutex run_mutex;
    SOPC_Condition run_cond;
    size_t address_space_size;
    size_t request_size;
    size_t address_space_offset;
    bench_func_t* bench_func;
    uint32_t session_id;
    uint64_t n_total_requests;
    uint64_t n_sent_requests;
    uint64_t n_measurements;
    uint64_t measurements[MAX_N_MEASUREMENTS];
    bench_status_t status;
    SOPC_DateTime cycle_start_ts;
    double mean;
    double rmoe;
};

static void make_nodeid(char* buf, size_t len, size_t idx)
{
    int n = snprintf(buf, len, "ns=1;s=Objects.%" PRIu64, (uint64_t) idx);
    SOPC_ASSERT(n > 0 && (((size_t) n) < (len - 1)));
}

static void* bench_read_requests(size_t request_size, size_t bench_offset, size_t addspace_size)
{
    SOPC_ASSERT(request_size <= INT32_MAX);

    OpcUa_ReadRequest* req = SOPC_Calloc(1, sizeof(OpcUa_ReadRequest));
    SOPC_ASSERT(req != NULL);

    OpcUa_ReadValueId* req_contents = SOPC_Calloc(request_size, sizeof(OpcUa_ReadValueId));
    SOPC_ASSERT(req_contents != NULL);

    OpcUa_ReadRequest_Initialize(req);

    req->TimestampsToReturn = OpcUa_TimestampsToReturn_Neither;
    req->NoOfNodesToRead = (int32_t) request_size;
    req->NodesToRead = req_contents;

    char buf[1024];

    for (size_t i = 0; i < request_size; ++i)
    {
        OpcUa_ReadValueId* r = &req_contents[i];
        OpcUa_ReadValueId_Initialize(r);

        r->AttributeId = 1; // NodeId
        make_nodeid(buf, sizeof(buf) / sizeof(char), (i + bench_offset) % addspace_size);
        SOPC_NodeId* id = SOPC_NodeId_FromCString(buf);
        SOPC_ASSERT(id != NULL);

        SOPC_ReturnStatus status = SOPC_NodeId_Copy(&r->NodeId, id);
        SOPC_ASSERT(status == SOPC_STATUS_OK);

        SOPC_NodeId_Clear(id);
        SOPC_Free(id);
    }

    return req;
}

static void* bench_write_requests(size_t request_size, size_t bench_offset, size_t addspace_size)
{
    SOPC_ASSERT(request_size <= INT32_MAX);

    OpcUa_WriteRequest* req = SOPC_Calloc(1, sizeof(OpcUa_WriteRequest));
    SOPC_ASSERT(req != NULL);

    OpcUa_WriteValue* req_contents = SOPC_Calloc(request_size, sizeof(OpcUa_WriteValue));
    SOPC_ASSERT(req_contents != NULL);

    OpcUa_WriteRequest_Initialize(req);

    req->NoOfNodesToWrite = (int32_t) request_size;
    req->NodesToWrite = req_contents;

    char buf[1024];

    for (size_t i = 0; i < request_size; ++i)
    {
        OpcUa_WriteValue* r = &req_contents[i];
        OpcUa_WriteValue_Initialize(r);

        make_nodeid(buf, sizeof(buf) / sizeof(char), (i + bench_offset) % addspace_size);
        SOPC_NodeId* id = SOPC_NodeId_FromCString(buf);
        SOPC_ASSERT(id != NULL);

        SOPC_ReturnStatus status = SOPC_NodeId_Copy(&r->NodeId, id);
        SOPC_ASSERT(status == SOPC_STATUS_OK);

        SOPC_NodeId_Clear(id);
        SOPC_Free(id);

        r->AttributeId = 13; // Value
        r->Value.Value.BuiltInTypeId = SOPC_Boolean_Id;
        r->Value.Value.ArrayType = SOPC_VariantArrayType_SingleValue;
        r->Value.Value.Value.Boolean = true;
    }

    return req;
}

static void bench_cycle_start(struct app_ctx_t* ctx)
{
    void* req = ctx->bench_func(ctx->request_size, ctx->address_space_offset, ctx->address_space_size);
    SOPC_ASSERT(req != NULL);

    ctx->address_space_offset = (ctx->address_space_offset + ctx->request_size) % ctx->address_space_size;
    ctx->cycle_start_ts = SOPC_Time_GetCurrentTimeUTC();

    SOPC_ToolkitClient_AsyncSendRequestOnSession(ctx->session_id, req, (uintptr_t) ctx);
}

static double mean(uint64_t* data, uint64_t n)
{
    if (n == 0)
    {
        return 0;
    }

    double acc = 0;

    for (uint64_t i = 0; i < n; ++i)
    {
        double x = acc + ((double) data[i]);
        SOPC_ASSERT(x >= acc); // In case we overflow
        acc = x;
    }

    acc /= ((double) n);

    return acc;
}

static double stddev(uint64_t* data, uint64_t n, double mean)
{
    if (n < 2)
    {
        return 0;
    }

    double var = 0;

    for (uint64_t i = 0; i < n; ++i)
    {
        double d = (double) data[i];
        double x = var + (d - mean) * (d - mean);
        SOPC_ASSERT(x >= var);
        var = x;
    }

    var /= ((double) (n - 1));

    return sqrt(var);
}

static bool bench_cycle_end(struct app_ctx_t* ctx)
{
    if (ctx->n_measurements % 10 == 0)
    {
        printf(".");
        fflush(stdout);
    }

    if (ctx->n_measurements == MAX_N_MEASUREMENTS)
    {
        printf("\n");
        fflush(stdout);
        fprintf(stderr, "Could not reach a stable measure in %d measurements.\n", MAX_N_MEASUREMENTS);
        ctx->status = BENCH_FINISHED_ERROR;
        return false;
    }

    SOPC_DateTime now = SOPC_Time_GetCurrentTimeUTC();
    SOPC_ASSERT(now >= ctx->cycle_start_ts);

    // OPC UA DateTime is 100 nanoseconds
    ctx->measurements[ctx->n_measurements] = 100 * ((uint64_t)(now - ctx->cycle_start_ts));
    ctx->n_measurements++;

    // Standard deviation
    double m = mean(ctx->measurements, ctx->n_measurements);
    double sd = stddev(ctx->measurements, ctx->n_measurements, m);

    // Standard error of the mean
    double sem = sd / sqrt((double) ctx->n_measurements);

    double critical_value;

    if (ctx->n_measurements < 2)
    {
        critical_value = T_TABLE[1];
    }
    else if (ctx->n_measurements <= T_TABLE_INF_IDX)
    {
        critical_value = T_TABLE[ctx->n_measurements - 1];
    }
    else
    {
        critical_value = T_TABLE[T_TABLE_INF_IDX];
    }

    // In percents
    double relative_margin_of_error = (sem * critical_value) / m * 100;

    ctx->mean = m;
    ctx->rmoe = relative_margin_of_error;

    if ((ctx->n_measurements >= MIN_N_MEASUREMENTS) && (ctx->rmoe <= REASONABLE_RMOE))
    {
        printf("\n");
        fflush(stdout);
        ctx->status = BENCH_FINISHED_OK;
        return false;
    }
    else
    {
        return true;
    }
}

static void event_handler(SOPC_App_Com_Event event, uint32_t arg, void* pParam, uintptr_t smCtx)
{
    struct app_ctx_t* ctx = (struct app_ctx_t*) smCtx;
    bool shutdown = false;
    bool more_needed = false;
    OpcUa_ReadResponse* res = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;

    switch (event)
    {
    case SE_ACTIVATED_SESSION:
        fprintf(stderr, "Connected, session activated\n");
        ctx->session_id = arg;
        bench_cycle_start(ctx);
        break;
    case SE_RCV_SESSION_RESPONSE:
        if (arg != ctx->session_id)
        {
            fprintf(stderr, "Received a session response for an unknown session ID %d\n", arg);
            break;
        }

        res = pParam;

        if (res->ResponseHeader.ServiceResult != SOPC_GoodGenericStatus)
        {
            fprintf(stderr, "Response with unexpected code %d\n", res->ResponseHeader.ServiceResult);
            break;
        }

        more_needed = bench_cycle_end(ctx);

        if (more_needed)
        {
            bench_cycle_start(ctx);
        }
        else
        {
            SOPC_ASSERT(ctx->status != BENCH_RUNNING);
            shutdown = true;
        }

        break;
    case SE_SESSION_ACTIVATION_FAILURE:
        fprintf(stderr, "Could not activate session\n");
        ctx->status = BENCH_FINISHED_ERROR;
        shutdown = true;
        break;
    case SE_CLOSED_SESSION:
        fprintf(stderr, "Session closed\n");
        ctx->status = BENCH_FINISHED_ERROR;
        shutdown = true;
        break;
    case SE_SND_REQUEST_FAILED:
        fprintf(stderr, "Request failed\n");
        ctx->status = BENCH_FINISHED_ERROR;
        shutdown = true;
        break;
    default:
        return;
    }

    if (shutdown)
    {
        SOPC_ASSERT(ctx->status != BENCH_RUNNING);
        status = SOPC_Condition_SignalAll(&ctx->run_cond);
        SOPC_ASSERT(SOPC_STATUS_OK == status);
    }
}

static struct
{
    const char* name;
    const char* desc;
    bench_func_t* func;
} BENCH_FUNCS[] = {
    {
        "read",
        "Retrieves NodeIds using a read request",
        bench_read_requests,
    },
    {
        "write",
        "Write the value \"true\" to the nodes using a write request",
        bench_write_requests,
    },
    {NULL, NULL, NULL},
};

static struct
{
    const char* name;
    const char* uri;
    OpcUa_MessageSecurityMode msg_sec_mode;
} SECURITY_POLICIES[] = {
    {"None", SOPC_SecurityPolicy_None_URI, OpcUa_MessageSecurityMode_None},
    {"Basic256", SOPC_SecurityPolicy_Basic256_URI, OpcUa_MessageSecurityMode_Sign},
    {"Basic256Sha256", SOPC_SecurityPolicy_Basic256Sha256_URI, OpcUa_MessageSecurityMode_SignAndEncrypt},
    {NULL, 0, OpcUa_MessageSecurityMode_Invalid}};

static const char* DEFAULT_KEY_PATH = "client_private/encrypted_client_2k_key.pem";
static const char* DEFAULT_CERT_PATH = "client_public/client_2k_cert.der";
static const char* DEFAULT_SERVER_CERT_PATH = "server_public/server_2k_cert.der";
static const char* DEFAULT_CA_PATH = "./S2OPC_Demo_PKI/trusted/certs/cacert.der";
static const char* DEFAULT_CRL_PATH = "./S2OPC_Demo_PKI/trusted/crl/cacrl.der";

static void usage(char** argv)
{
    printf(
        "Usage: %s BENCH_TYPE SECURITY_POLICY AS_SIZE REQUEST_SIZE\n\n"
        "Benchmarks a type of request against an OPC-UA server.\n\n"
        "Arguments:\n"
        "BENCH_TYPE       The type of benchmark to run (see list below).\n"
        "SECURITY_POLICY  The security policy to use to connect to the server (see list below).\n"
        "AS_SIZE          Number of benchmark nodes in the server address space.\n"
        "REQUEST_SIZE     Number of operations per request (eg. nodes per read request).\n\n"
        "The address space is supposed to hold AS_SIZE variables with a string\n"
        "NodeId following the syntax \"ns=1;s=Objects.IDX\" with IDX varying\n"
        "from 0 to AS_SIZE-1.\n\n"
        "Security policies:\n",
        argv[0]);

    for (size_t i = 0; SECURITY_POLICIES[i].name != NULL; ++i)
    {
        printf("%s\n", SECURITY_POLICIES[i].name);
    }

    printf(
        "\nIf a security policy is defined, the paths to the client key, certificate and to\n"
        "the certificate authority can be set using the environment variables SOPC_KEY, SOPC_CERT\n"
        "SOPC_CA and SOPC_CRL. The default values for those variables are:\n"
        "SOPC_KEY:         %s\n"
        "SOPC_CERT:        %s\n"
        "SOPC_SERVER_CERT: %s\n"
        "SOPC_CA:          %s\n"
        "SOPC_CRL:         %s\n\n"
        "Benchmark types:\n",
        DEFAULT_KEY_PATH, DEFAULT_CERT_PATH, DEFAULT_SERVER_CERT_PATH, DEFAULT_CA_PATH, DEFAULT_CRL_PATH);

    for (size_t i = 0; BENCH_FUNCS[i].name != NULL; ++i)
    {
        printf("%-15s  %s\n", BENCH_FUNCS[i].name, BENCH_FUNCS[i].desc);
    }

    printf(
        "\nThe server address can be set using the SOPC_SERVER_URL environment variable.\n"
        "By default, it is set to %s .\n",
        DEFAULT_SERVER_URL);
}

static bench_func_t* bench_func_by_name(const char* name)
{
    for (size_t i = 0; BENCH_FUNCS[i].name != NULL; ++i)
    {
        if (strcmp(BENCH_FUNCS[i].name, name) == 0)
        {
            return BENCH_FUNCS[i].func;
        }
    }

    return NULL;
}

static bool security_policy_by_name(const char* name, const char** uri, OpcUa_MessageSecurityMode* msg_sec_mode)
{
    for (size_t i = 0; SECURITY_POLICIES[i].name != NULL; ++i)
    {
        if (strcmp(SECURITY_POLICIES[i].name, name) == 0)
        {
            *uri = SECURITY_POLICIES[i].uri;
            *msg_sec_mode = SECURITY_POLICIES[i].msg_sec_mode;
            return true;
        }
    }

    return false;
}

static const char* getenv_default(const char* name, const char* default_value)
{
    const char* val = getenv(name);

    return (val != NULL) ? val : default_value;
}

static bool load_keys(SOPC_KeyCertPair** keyCertPair,
                      SOPC_CertHolder** srvCertHolder,
                      SOPC_CertificateList** ca,
                      SOPC_CRLList** cacrl)
{
    const char* cert_path = getenv_default("SOPC_CERT", DEFAULT_CERT_PATH);
    const char* key_path = getenv_default("SOPC_KEY", DEFAULT_KEY_PATH);
    const char* server_cert_path = getenv_default("SOPC_SERVER_CERT", DEFAULT_SERVER_CERT_PATH);
    const char* ca_path = getenv_default("SOPC_CA", DEFAULT_CA_PATH);
    const char* crl_path = getenv_default("SOPC_CRL", DEFAULT_CRL_PATH);

    char* password = NULL;
    size_t lenPassword = 0;

    bool res = SOPC_AskPass_CustomPromptFromTerminal("Private key password:\n", &password);

    if (res)
    {
        lenPassword = strlen(password);
        if (UINT32_MAX < lenPassword)
        {
            res = false;
        }
    }

    if (!res)
    {
        fprintf(stderr, "Error while retrieve password for private key\n");
    }

    if (SOPC_KeyCertPair_CreateFromPaths(cert_path, key_path, password, keyCertPair) != SOPC_STATUS_OK)
    {
        fprintf(stderr, "Error while loading certificate %s or private key %s from paths\n", server_cert_path,
                key_path);
    }

    if (SOPC_KeyCertPair_CreateCertHolderFromPath(server_cert_path, srvCertHolder) != SOPC_STATUS_OK)
    {
        fprintf(stderr, "Error while loading server certificate from %s\n", ca_path);
    }

    if (SOPC_KeyManager_Certificate_CreateOrAddFromFile(ca_path, ca) != SOPC_STATUS_OK)
    {
        fprintf(stderr, "Error while loading CA certificate from %s\n", ca_path);
    }

    if (SOPC_KeyManager_CRL_CreateOrAddFromFile(crl_path, cacrl) != SOPC_STATUS_OK)
    {
        fprintf(stderr, "Error while loading CA CRL from %s\n", crl_path);
    }

    if (NULL != password)
    {
        SOPC_Free(password);
    }

    if (*keyCertPair == NULL || *srvCertHolder == NULL || *ca == NULL)
    {
        SOPC_KeyCertPair_Delete(keyCertPair);
        SOPC_KeyCertPair_Delete(srvCertHolder);

        SOPC_KeyManager_Certificate_Free(*ca);
        *ca = NULL;

        SOPC_KeyManager_CRL_Free(*cacrl);
        *cacrl = NULL;

        return false;
    }

    return true;
}

int main(int argc, char** argv)
{
    if (argc != 5)
    {
        usage(argv);
        return 1;
    }

    const char* arg_bench_type = argv[1];
    const char* arg_security_policy = argv[2];
    const char* arg_as_size = argv[3];
    const char* arg_request_size = argv[4];

    bench_func_t* bench_func = bench_func_by_name(arg_bench_type);

    if (bench_func == NULL)
    {
        fprintf(stderr, "Unknown benchmark type: %s\n", arg_bench_type);
        return 1;
    }

    const char* security_policy;
    OpcUa_MessageSecurityMode msg_sec_mode;

    if (!security_policy_by_name(arg_security_policy, &security_policy, &msg_sec_mode))
    {
        fprintf(stderr, "Unknown security policy: %s\n", arg_security_policy);
        return 1;
    }

    char* endptr;
    errno = 0;
    uint64_t as_size = strtoul(arg_as_size, &endptr, 10);

    if (arg_as_size[0] == '\0' || *endptr != '\0' || errno != 0 || as_size > SIZE_MAX)
    {
        fprintf(stderr, "AS_SIZE is not a valid integer or is too large.\n");
        return 1;
    }

    uint64_t request_size = strtoul(arg_request_size, &endptr, 10);

    if (arg_request_size[0] == '\0' || *endptr != '\0' || errno != 0 || request_size > SIZE_MAX)
    {
        fprintf(stderr, "REQUEST_SIZE is not a valid integer or is too large.\n");
        return 1;
    }

    struct app_ctx_t ctx;
    memset(&ctx, 0, sizeof(struct app_ctx_t));

    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Initialization(&ctx.run_mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    mutStatus = SOPC_Mutex_Lock(&ctx.run_mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    mutStatus = SOPC_Condition_Init(&ctx.run_cond);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    ctx.address_space_size = (size_t) as_size;
    ctx.request_size = (size_t) request_size;
    ctx.bench_func = bench_func;
    ctx.status = BENCH_RUNNING;

    /* Initialize SOPC_Common */
    SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
    logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = "./bench_tool_logs/";
    logConfiguration.logLevel = SOPC_LOG_LEVEL_DEBUG;
    SOPC_ReturnStatus status = SOPC_Common_Initialize(logConfiguration);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    status = SOPC_Toolkit_Initialize(event_handler);
    SOPC_ASSERT(status == SOPC_STATUS_OK);

    SOPC_SecureChannel_Config scConfig;
    SOPC_Client_Config clientConfig;
    memset(&scConfig, 0, sizeof(SOPC_SecureChannel_Config));
    SOPC_ClientConfig_Initialize(&clientConfig);

    scConfig.isClientSc = true;
    scConfig.clientConfigPtr = &clientConfig;
    scConfig.url = getenv_default("SOPC_SERVER_URL", DEFAULT_SERVER_URL);
    scConfig.reqSecuPolicyUri = security_policy;
    scConfig.msgSecurityMode = msg_sec_mode;
    scConfig.requestedLifetime = 60000;

    SOPC_KeyCertPair* cliKeyCertPair = NULL;
    SOPC_CertHolder* srvCertHolder = NULL;
    SOPC_CertificateList* ca = NULL;
    SOPC_CRLList* cacrl = NULL;
    SOPC_PKIProvider* pki = NULL;

    if (msg_sec_mode != OpcUa_MessageSecurityMode_None)
    {
        bool bRet = load_keys(&cliKeyCertPair, &srvCertHolder, &ca, &cacrl);
        status = SOPC_PKIProvider_CreateFromList(ca, cacrl, NULL, NULL, &pki);
        if (!bRet || SOPC_STATUS_OK != status)
        {
            SOPC_PKIProvider_Free(&pki);
            SOPC_KeyCertPair_Delete(&cliKeyCertPair);
            SOPC_KeyCertPair_Delete(&srvCertHolder);
            SOPC_KeyManager_Certificate_Free(ca);
            SOPC_KeyManager_CRL_Free(cacrl);
            return 1;
        }

        scConfig.peerAppCert = srvCertHolder;
        clientConfig.clientKeyCertPair = cliKeyCertPair;
        clientConfig.clientPKI = pki;
    }

    uint32_t configIdx = SOPC_ToolkitClient_AddSecureChannelConfig(&scConfig);
    SOPC_ASSERT(configIdx != 0);

    printf("Connecting to the server...\n");
    SOPC_EndpointConnectionCfg endpointConnectionCfg = SOPC_EndpointConnectionCfg_CreateClassic(configIdx);
    SOPC_ToolkitClient_AsyncActivateSession_Anonymous(endpointConnectionCfg, NULL, (uintptr_t) &ctx, "anonymous");

    mutStatus = SOPC_Mutex_UnlockAndWaitCond(&ctx.run_cond, &ctx.run_mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    SOPC_Toolkit_Clear();
    SOPC_PKIProvider_Free(&pki);
    SOPC_KeyCertPair_Delete(&cliKeyCertPair);
    SOPC_KeyCertPair_Delete(&srvCertHolder);
    SOPC_KeyManager_Certificate_Free(ca);
    SOPC_KeyManager_CRL_Free(cacrl);

    SOPC_Mutex_Clear(&ctx.run_mutex);
    SOPC_Condition_Clear(&ctx.run_cond);

    if (ctx.status == BENCH_FINISHED_OK)
    {
        printf("Finished benchmarking.\n");
        printf("Address space size: %" PRIu64 "\n", as_size);
        printf("Request size: %" PRIu64 "\n", request_size);
        printf("Number of measurements: %" PRIu64 "\n", ctx.n_measurements);
        printf("Average response time: %.2f us\n", ctx.mean / 1000);
        printf("Relative margin of error: %.2f%%\n", ctx.rmoe);
        return 0;
    }
    else
    {
        fprintf(stderr, "Could not run the benchmark, aborting.\n");
        return 1;
    }
}
